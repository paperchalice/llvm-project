//===-- MMIXMCExpr.cpp - MMIX specific MC expression classes --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXMCExpr.h"
#include "MMIXMCTargetDesc.h"

#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

bool MMIXMCExpr::classof(const MCExpr *E) {
  return E->getKind() == MCExpr::Target;
}

const MMIXMCExpr *MMIXMCExpr::createRegExpr(const MCExpr *Expr,
                                            MCContext &Ctx) {
  auto *E = new (Ctx) MMIXMCExpr();
  E->Content = Expr;
  return E;
}

const MMIXMCExpr *MMIXMCExpr::createFracDivExpr(const MCExpr *LHS,
                                                const MCExpr *RHS,
                                                MCContext &Ctx) {
  auto *E = new (Ctx) MMIXMCExpr();
  E->Content = FracDivType{LHS, RHS};
  return E;
}

bool MMIXMCExpr::isRegExpr() const { return Content.index() == 0; }
bool MMIXMCExpr::isFracDiv() const { return Content.index() == 1; }

MCRegister MMIXMCExpr::getMCReg() const {
  assert(isRegExpr() && "Not a reg!");
  const MCRegisterClass &GPRClass = getMMIXMCRegisterClass(MMIX::GPRRegClassID);
  const auto *Expr = std::get<const MCExpr *>(Content);
  int64_t Val = 0;
  if (Expr->evaluateAsAbsolute(Val))
    return GPRClass.getRegister(Val);
  else
    return MCRegister();
}

void MMIXMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  if (isRegExpr()) {
    OS << '$';
    const auto *Expr = std::get<const MCExpr *>(Content);
    if (const auto *C = dyn_cast<MCConstantExpr>(Expr)) {
      OS << static_cast<unsigned>(C->getValue());
    } else {
      OS << '(';
      MAI->printExpr(OS, *Expr);
      OS << ')';
    }
    return;
  }
  if (isFracDiv()) {
    const auto &Expr = std::get<FracDivType>(Content);
    MAI->printExpr(OS, *std::get<0>(Expr));
    OS << "//";
    MAI->printExpr(OS, *std::get<1>(Expr));
  }
}

bool MMIXMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAssembler *Asm) const {
  if (isRegExpr())
    return std::get<const MCExpr *>(Content)->evaluateAsRelocatable(Res, Asm);
  if (isFracDiv()) {
    const auto &FracDivExpr = std::get<FracDivType>(Content);
    int64_t LHSSVal, RHSSVal;
    if (!std::get<0>(FracDivExpr)->evaluateAsAbsolute(LHSSVal) ||
        !std::get<1>(FracDivExpr)->evaluateAsAbsolute(RHSSVal))
      return false;
    uint64_t LHSVal = LHSSVal, RHSVal = RHSSVal;
    auto Val = (APInt(128, LHSVal) << 64).udiv(RHSVal) &
               std::numeric_limits<uint64_t>::max();
    Res = MCValue::get(Val.getZExtValue());
    return true;
  }

  return false;
}

void MMIXMCExpr::visitUsedExpr(MCStreamer &Streamer) const {}

MCFragment *MMIXMCExpr::findAssociatedFragment() const { return nullptr; }
