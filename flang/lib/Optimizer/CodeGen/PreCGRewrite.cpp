//===-- PreCGRewrite.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.llvm.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "flang/Optimizer/CodeGen/CodeGen.h"

#include "flang/Optimizer/Builder/Todo.h" // remove when TODO's are done
#include "flang/Optimizer/Dialect/FIRCG/CGOps.h"
#include "flang/Optimizer/Dialect/FIRDialect.h"
#include "flang/Optimizer/Dialect/FIROps.h"
#include "flang/Optimizer/Dialect/FIRType.h"
#include "flang/Optimizer/Dialect/Support/FIRContext.h"
#include "mlir/IR/Iterators.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Debug.h"

namespace fir {
#define GEN_PASS_DEF_CODEGENREWRITE
#include "flang/Optimizer/CodeGen/CGPasses.h.inc"
} // namespace fir

//===----------------------------------------------------------------------===//
// Codegen rewrite: rewriting of subgraphs of ops
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "flang-codegen-rewrite"

static void populateShape(llvm::SmallVectorImpl<mlir::Value> &vec,
                          fir::ShapeOp shape) {
  vec.append(shape.getExtents().begin(), shape.getExtents().end());
}

// Operands of fir.shape_shift split into two vectors.
static void populateShapeAndShift(llvm::SmallVectorImpl<mlir::Value> &shapeVec,
                                  llvm::SmallVectorImpl<mlir::Value> &shiftVec,
                                  fir::ShapeShiftOp shift) {
  for (auto i = shift.getPairs().begin(), endIter = shift.getPairs().end();
       i != endIter;) {
    shiftVec.push_back(*i++);
    shapeVec.push_back(*i++);
  }
}

static void populateShift(llvm::SmallVectorImpl<mlir::Value> &vec,
                          fir::ShiftOp shift) {
  vec.append(shift.getOrigins().begin(), shift.getOrigins().end());
}

namespace {

/// Convert fir.embox to the extended form where necessary.
///
/// The embox operation can take arguments that specify multidimensional array
/// properties at runtime. These properties may be shared between distinct
/// objects that have the same properties. Before we lower these small DAGs to
/// LLVM-IR, we gather all the information into a single extended operation. For
/// example,
/// ```
/// %1 = fir.shape_shift %4, %5 : (index, index) -> !fir.shapeshift<1>
/// %2 = fir.slice %6, %7, %8 : (index, index, index) -> !fir.slice<1>
/// %3 = fir.embox %0 (%1) [%2] : (!fir.ref<!fir.array<?xi32>>,
/// !fir.shapeshift<1>, !fir.slice<1>) -> !fir.box<!fir.array<?xi32>>
/// ```
/// can be rewritten as
/// ```
/// %1 = fircg.ext_embox %0(%5) origin %4[%6, %7, %8] :
/// (!fir.ref<!fir.array<?xi32>>, index, index, index, index, index) ->
/// !fir.box<!fir.array<?xi32>>
/// ```
class EmboxConversion : public mlir::OpRewritePattern<fir::EmboxOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  llvm::LogicalResult
  matchAndRewrite(fir::EmboxOp embox,
                  mlir::PatternRewriter &rewriter) const override {
    // If the embox does not include a shape, then do not convert it
    if (auto shapeVal = embox.getShape())
      return rewriteDynamicShape(embox, rewriter, shapeVal);
    if (mlir::isa<fir::ClassType>(embox.getType()))
      TODO(embox.getLoc(), "embox conversion for fir.class type");
    if (auto boxTy = mlir::dyn_cast<fir::BoxType>(embox.getType()))
      if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(boxTy.getEleTy()))
        if (!seqTy.hasDynamicExtents())
          return rewriteStaticShape(embox, rewriter, seqTy);
    return mlir::failure();
  }

  llvm::LogicalResult rewriteStaticShape(fir::EmboxOp embox,
                                         mlir::PatternRewriter &rewriter,
                                         fir::SequenceType seqTy) const {
    auto loc = embox.getLoc();
    llvm::SmallVector<mlir::Value> shapeOpers;
    auto idxTy = rewriter.getIndexType();
    for (auto ext : seqTy.getShape()) {
      auto iAttr = rewriter.getIndexAttr(ext);
      auto extVal =
          mlir::arith::ConstantOp::create(rewriter, loc, idxTy, iAttr);
      shapeOpers.push_back(extVal);
    }
    auto xbox = fir::cg::XEmboxOp::create(
        rewriter, loc, embox.getType(), embox.getMemref(), shapeOpers,
        mlir::ValueRange{}, mlir::ValueRange{}, mlir::ValueRange{},
        mlir::ValueRange{}, embox.getTypeparams(), embox.getSourceBox(),
        embox.getAllocatorIdxAttr());
    LLVM_DEBUG(llvm::dbgs() << "rewriting " << embox << " to " << xbox << '\n');
    rewriter.replaceOp(embox, xbox.getOperation()->getResults());
    return mlir::success();
  }

  llvm::LogicalResult rewriteDynamicShape(fir::EmboxOp embox,
                                          mlir::PatternRewriter &rewriter,
                                          mlir::Value shapeVal) const {
    auto loc = embox.getLoc();
    llvm::SmallVector<mlir::Value> shapeOpers;
    llvm::SmallVector<mlir::Value> shiftOpers;
    if (auto shapeOp = mlir::dyn_cast<fir::ShapeOp>(shapeVal.getDefiningOp())) {
      populateShape(shapeOpers, shapeOp);
    } else {
      auto shiftOp =
          mlir::dyn_cast<fir::ShapeShiftOp>(shapeVal.getDefiningOp());
      assert(shiftOp && "shape is neither fir.shape nor fir.shape_shift");
      populateShapeAndShift(shapeOpers, shiftOpers, shiftOp);
    }
    llvm::SmallVector<mlir::Value> sliceOpers;
    llvm::SmallVector<mlir::Value> subcompOpers;
    llvm::SmallVector<mlir::Value> substrOpers;
    if (auto s = embox.getSlice())
      if (auto sliceOp =
              mlir::dyn_cast_or_null<fir::SliceOp>(s.getDefiningOp())) {
        sliceOpers.assign(sliceOp.getTriples().begin(),
                          sliceOp.getTriples().end());
        subcompOpers.assign(sliceOp.getFields().begin(),
                            sliceOp.getFields().end());
        substrOpers.assign(sliceOp.getSubstr().begin(),
                           sliceOp.getSubstr().end());
      }
    auto xbox = fir::cg::XEmboxOp::create(
        rewriter, loc, embox.getType(), embox.getMemref(), shapeOpers,
        shiftOpers, sliceOpers, subcompOpers, substrOpers,
        embox.getTypeparams(), embox.getSourceBox(),
        embox.getAllocatorIdxAttr());
    LLVM_DEBUG(llvm::dbgs() << "rewriting " << embox << " to " << xbox << '\n');
    rewriter.replaceOp(embox, xbox.getOperation()->getResults());
    return mlir::success();
  }
};

/// Convert fir.rebox to the extended form where necessary.
///
/// For example,
/// ```
/// %5 = fir.rebox %3(%1) : (!fir.box<!fir.array<?xi32>>, !fir.shapeshift<1>) ->
/// !fir.box<!fir.array<?xi32>>
/// ```
/// converted to
/// ```
/// %5 = fircg.ext_rebox %3(%13) origin %12 : (!fir.box<!fir.array<?xi32>>,
/// index, index) -> !fir.box<!fir.array<?xi32>>
/// ```
class ReboxConversion : public mlir::OpRewritePattern<fir::ReboxOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  llvm::LogicalResult
  matchAndRewrite(fir::ReboxOp rebox,
                  mlir::PatternRewriter &rewriter) const override {
    auto loc = rebox.getLoc();
    llvm::SmallVector<mlir::Value> shapeOpers;
    llvm::SmallVector<mlir::Value> shiftOpers;
    if (auto shapeVal = rebox.getShape()) {
      if (auto shapeOp = mlir::dyn_cast<fir::ShapeOp>(shapeVal.getDefiningOp()))
        populateShape(shapeOpers, shapeOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShapeShiftOp>(shapeVal.getDefiningOp()))
        populateShapeAndShift(shapeOpers, shiftOpers, shiftOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShiftOp>(shapeVal.getDefiningOp()))
        populateShift(shiftOpers, shiftOp);
      else
        return mlir::failure();
    }
    llvm::SmallVector<mlir::Value> sliceOpers;
    llvm::SmallVector<mlir::Value> subcompOpers;
    llvm::SmallVector<mlir::Value> substrOpers;
    if (auto s = rebox.getSlice())
      if (auto sliceOp =
              mlir::dyn_cast_or_null<fir::SliceOp>(s.getDefiningOp())) {
        sliceOpers.append(sliceOp.getTriples().begin(),
                          sliceOp.getTriples().end());
        subcompOpers.append(sliceOp.getFields().begin(),
                            sliceOp.getFields().end());
        substrOpers.append(sliceOp.getSubstr().begin(),
                           sliceOp.getSubstr().end());
      }

    auto xRebox = fir::cg::XReboxOp::create(
        rewriter, loc, rebox.getType(), rebox.getBox(), shapeOpers, shiftOpers,
        sliceOpers, subcompOpers, substrOpers);
    LLVM_DEBUG(llvm::dbgs()
               << "rewriting " << rebox << " to " << xRebox << '\n');
    rewriter.replaceOp(rebox, xRebox.getOperation()->getResults());
    return mlir::success();
  }
};

/// Convert all fir.array_coor to the extended form.
///
/// For example,
/// ```
///  %4 = fir.array_coor %addr (%1) [%2] %0 : (!fir.ref<!fir.array<?xi32>>,
///  !fir.shapeshift<1>, !fir.slice<1>, index) -> !fir.ref<i32>
/// ```
/// converted to
/// ```
/// %40 = fircg.ext_array_coor %addr(%9) origin %8[%4, %5, %6<%39> :
/// (!fir.ref<!fir.array<?xi32>>, index, index, index, index, index, index) ->
/// !fir.ref<i32>
/// ```
class ArrayCoorConversion : public mlir::OpRewritePattern<fir::ArrayCoorOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  llvm::LogicalResult
  matchAndRewrite(fir::ArrayCoorOp arrCoor,
                  mlir::PatternRewriter &rewriter) const override {
    auto loc = arrCoor.getLoc();
    llvm::SmallVector<mlir::Value> shapeOpers;
    llvm::SmallVector<mlir::Value> shiftOpers;
    if (auto shapeVal = arrCoor.getShape()) {
      if (auto shapeOp = mlir::dyn_cast<fir::ShapeOp>(shapeVal.getDefiningOp()))
        populateShape(shapeOpers, shapeOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShapeShiftOp>(shapeVal.getDefiningOp()))
        populateShapeAndShift(shapeOpers, shiftOpers, shiftOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShiftOp>(shapeVal.getDefiningOp()))
        populateShift(shiftOpers, shiftOp);
      else
        return mlir::failure();
    }
    llvm::SmallVector<mlir::Value> sliceOpers;
    llvm::SmallVector<mlir::Value> subcompOpers;
    if (auto s = arrCoor.getSlice())
      if (auto sliceOp =
              mlir::dyn_cast_or_null<fir::SliceOp>(s.getDefiningOp())) {
        sliceOpers.append(sliceOp.getTriples().begin(),
                          sliceOp.getTriples().end());
        subcompOpers.append(sliceOp.getFields().begin(),
                            sliceOp.getFields().end());
        assert(sliceOp.getSubstr().empty() &&
               "Don't allow substring operations on array_coor. This "
               "restriction may be lifted in the future.");
      }
    auto xArrCoor = fir::cg::XArrayCoorOp::create(
        rewriter, loc, arrCoor.getType(), arrCoor.getMemref(), shapeOpers,
        shiftOpers, sliceOpers, subcompOpers, arrCoor.getIndices(),
        arrCoor.getTypeparams());
    LLVM_DEBUG(llvm::dbgs()
               << "rewriting " << arrCoor << " to " << xArrCoor << '\n');
    rewriter.replaceOp(arrCoor, xArrCoor.getOperation()->getResults());
    return mlir::success();
  }
};

class DeclareOpConversion : public mlir::OpRewritePattern<fir::DeclareOp> {
  bool preserveDeclare;

public:
  using OpRewritePattern::OpRewritePattern;
  DeclareOpConversion(mlir::MLIRContext *ctx, bool preserveDecl)
      : OpRewritePattern(ctx), preserveDeclare(preserveDecl) {}

  llvm::LogicalResult
  matchAndRewrite(fir::DeclareOp declareOp,
                  mlir::PatternRewriter &rewriter) const override {
    if (!preserveDeclare) {
      rewriter.replaceOp(declareOp, declareOp.getMemref());
      return mlir::success();
    }
    auto loc = declareOp.getLoc();
    llvm::SmallVector<mlir::Value> shapeOpers;
    llvm::SmallVector<mlir::Value> shiftOpers;
    if (auto shapeVal = declareOp.getShape()) {
      if (auto shapeOp = mlir::dyn_cast<fir::ShapeOp>(shapeVal.getDefiningOp()))
        populateShape(shapeOpers, shapeOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShapeShiftOp>(shapeVal.getDefiningOp()))
        populateShapeAndShift(shapeOpers, shiftOpers, shiftOp);
      else if (auto shiftOp =
                   mlir::dyn_cast<fir::ShiftOp>(shapeVal.getDefiningOp()))
        populateShift(shiftOpers, shiftOp);
      else
        return mlir::failure();
    }
    // FIXME: Add FortranAttrs and CudaAttrs
    auto xDeclOp = fir::cg::XDeclareOp::create(
        rewriter, loc, declareOp.getType(), declareOp.getMemref(), shapeOpers,
        shiftOpers, declareOp.getTypeparams(), declareOp.getDummyScope(),
        declareOp.getUniqName());
    LLVM_DEBUG(llvm::dbgs()
               << "rewriting " << declareOp << " to " << xDeclOp << '\n');
    rewriter.replaceOp(declareOp, xDeclOp.getOperation()->getResults());
    return mlir::success();
  }
};

class DummyScopeOpConversion
    : public mlir::OpRewritePattern<fir::DummyScopeOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  llvm::LogicalResult
  matchAndRewrite(fir::DummyScopeOp dummyScopeOp,
                  mlir::PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<fir::UndefOp>(dummyScopeOp,
                                              dummyScopeOp.getType());
    return mlir::success();
  }
};

/// Simple DCE to erase fir.shape/shift/slice/unused shape operands after this
/// pass (fir.shape and like have no codegen).
/// mlir::RegionDCE is expensive and requires running
/// mlir::eraseUnreachableBlocks. It does things that are not needed here, like
/// removing unused block arguments. fir.shape/shift/slice cannot be block
/// arguments.
/// This helper does a naive backward walk of the IR. It is not even guaranteed
/// to walk blocks according to backward dominance, but that is good enough for
/// what is done here, fir.shape/shift/slice have no usages anymore. The
/// backward walk allows getting rid of most of the unused operands, it is not a
/// problem to leave some in the weird cases.
static void simpleDCE(mlir::RewriterBase &rewriter, mlir::Operation *op) {
  op->walk<mlir::WalkOrder::PostOrder, mlir::ReverseIterator>(
      [&](mlir::Operation *subOp) {
        if (mlir::isOpTriviallyDead(subOp))
          rewriter.eraseOp(subOp);
      });
}

class CodeGenRewrite : public fir::impl::CodeGenRewriteBase<CodeGenRewrite> {
public:
  using CodeGenRewriteBase<CodeGenRewrite>::CodeGenRewriteBase;

  void runOnOperation() override final {
    mlir::ModuleOp mod = getOperation();

    auto &context = getContext();
    mlir::ConversionTarget target(context);
    target.addLegalDialect<mlir::arith::ArithDialect, fir::FIROpsDialect,
                           fir::FIRCodeGenDialect, mlir::func::FuncDialect>();
    target.addIllegalOp<fir::ArrayCoorOp>();
    target.addIllegalOp<fir::ReboxOp>();
    target.addIllegalOp<fir::DeclareOp>();
    target.addIllegalOp<fir::DummyScopeOp>();
    target.addDynamicallyLegalOp<fir::EmboxOp>([](fir::EmboxOp embox) {
      return !(embox.getShape() ||
               mlir::isa<fir::SequenceType>(
                   mlir::cast<fir::BaseBoxType>(embox.getType()).getEleTy()));
    });
    mlir::RewritePatternSet patterns(&context);
    fir::populatePreCGRewritePatterns(patterns, preserveDeclare);
    if (mlir::failed(
            mlir::applyPartialConversion(mod, target, std::move(patterns)))) {
      mlir::emitError(mlir::UnknownLoc::get(&context),
                      "error in running the pre-codegen conversions");
      signalPassFailure();
      return;
    }
    // Erase any residual (fir.shape, fir.slice...).
    mlir::IRRewriter rewriter(&context);
    simpleDCE(rewriter, mod.getOperation());
  }
};

} // namespace

void fir::populatePreCGRewritePatterns(mlir::RewritePatternSet &patterns,
                                       bool preserveDeclare) {
  patterns.insert<EmboxConversion, ArrayCoorConversion, ReboxConversion,
                  DummyScopeOpConversion>(patterns.getContext());
  patterns.add<DeclareOpConversion>(patterns.getContext(), preserveDeclare);
}
