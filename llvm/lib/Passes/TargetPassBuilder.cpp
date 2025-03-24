//===- Parsing, selection, and construction of pass pipelines -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Passes/TargetPassBuilder.h"
#include "llvm/CodeGen/CallBrPrepare.h"
#include "llvm/CodeGen/CodeGenPrepare.h"
#include "llvm/CodeGen/DwarfEHPrepare.h"
#include "llvm/CodeGen/ExpandLargeDivRem.h"
#include "llvm/CodeGen/ExpandLargeFpConvert.h"
#include "llvm/CodeGen/ExpandMemCmp.h"
#include "llvm/CodeGen/ExpandReductions.h"
#include "llvm/CodeGen/FinalizeISel.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/GlobalMergeFunctions.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/LocalStackSlotAllocation.h"
#include "llvm/CodeGen/LowerEmuTLS.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachinePassManager.h"
#include "llvm/CodeGen/PreISelIntrinsicLowering.h"
#include "llvm/CodeGen/ReplaceWithVeclib.h"
#include "llvm/CodeGen/SafeStack.h"
#include "llvm/CodeGen/SelectOptimize.h"
#include "llvm/CodeGen/ShadowStackGCLowering.h"
#include "llvm/CodeGen/SjLjEHPrepare.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/UnreachableBlockElim.h"
#include "llvm/CodeGen/WasmEHPrepare.h"
#include "llvm/CodeGen/WinEHPrepare.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Passes/CodeGenPassBuilder.h" // Dummy passes only!
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/ObjCARC.h"
#include "llvm/Transforms/Scalar/ConstantHoisting.h"
#include "llvm/Transforms/Scalar/LoopStrengthReduce.h"
#include "llvm/Transforms/Scalar/LoopTermFold.h"
#include "llvm/Transforms/Scalar/MergeICmps.h"
#include "llvm/Transforms/Scalar/PartiallyInlineLibCalls.h"
#include "llvm/Transforms/Scalar/ScalarizeMaskedMemIntrin.h"
#include "llvm/Transforms/Utils/CanonicalizeFreezeInLoops.h"
#include "llvm/Transforms/Utils/LowerGlobalDtors.h"
#include "llvm/Transforms/Utils/LowerInvoke.h"

using namespace llvm;

TargetPassBuilder::TargetPassBuilder(PassBuilder &PB) : PB(PB) {
  BeforeAddingCallbacks.push_back(
      [this](StringRef Name) { return !DisabedPasses.contains(Name); });

  CodeGenOptLevel OptLevel = PB.TM->getOptLevel();
  if (OptLevel != CodeGenOptLevel::None && !CGPBO.DisableCGP)
    disablePass<CodeGenPreparePass>();
}

ModulePassManager TargetPassBuilder::buildPipeline() {
  buildCodeGenIRPipeline();

  return ModulePassManager();
}

void TargetPassBuilder::buildCodeGenIRPipeline() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();
  assert(TM && "Must have a valid TargetMachine!");
  if (TM->useEmulatedTLS())
    addModulePass(LowerEmuTLSPass());
  addModulePass(PreISelIntrinsicLoweringPass(TM));
  addFunctionPass(ExpandLargeDivRemPass(TM), ExpandLargeFpConvertPass(TM));

  addISelIRPasses();

  addFunctionPass(CodeGenPreparePass(PB.TM));
  buildExceptionHandlingPipeline();
  buildISelPreparePipeline();
}

void TargetPassBuilder::buildExceptionHandlingPipeline() {
  TargetMachine *TM = PB.TM;
  const MCAsmInfo *MCAI = TM->getMCAsmInfo();

  switch (MCAI->getExceptionHandlingType()) {
  case ExceptionHandling::SjLj:
    // SjLj piggy-backs on dwarf for this bit. The cleanups done apply to both
    // Dwarf EH prepare needs to be run after SjLj prepare. Otherwise,
    // catch info can get misplaced when a selector ends up more than one block
    // removed from the parent invoke(s). This could happen when a landing
    // pad is shared by multiple invokes and is also a target of a normal
    // edge from elsewhere.
    addFunctionPass(SjLjEHPreparePass(TM));
    [[fallthrough]];
  case ExceptionHandling::DwarfCFI:
  case ExceptionHandling::ARM:
  case ExceptionHandling::AIX:
  case ExceptionHandling::ZOS:
    addFunctionPass(DwarfEHPreparePass(TM));
    break;
  case ExceptionHandling::WinEH:
    // We support using both GCC-style and MSVC-style exceptions on Windows, so
    // add both preparation passes. Each pass will only actually run if it
    // recognizes the personality function.
    addFunctionPass(WinEHPreparePass(), DwarfEHPreparePass(TM));
    break;
  case ExceptionHandling::Wasm:
    // Wasm EH uses Windows EH instructions, but it does not need to demote PHIs
    // on catchpads and cleanuppads because it does not outline them into
    // funclets. Catchswitch blocks are not lowered in SelectionDAG, so we
    // should remove PHIs there.
    addFunctionPass(WinEHPreparePass(/*DemoteCatchSwitchPHIOnly=*/true),
                    WasmEHPreparePass());
    break;
  case ExceptionHandling::None:
    addFunctionPass(
        LowerInvokePass(),
        // The lower invoke pass may create unreachable code. Remove it.
        UnreachableBlockElimPass());
    break;
  }
}

void TargetPassBuilder::buildISelPreparePipeline() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();

  if (OptLevel != CodeGenOptLevel::None)
    addFunctionPass(ObjCARCContractPass());

  addFunctionPass(CallBrPreparePass());

  // Add both the safe stack and the stack protection passes: each of them will
  // only protect functions that have corresponding attributes.
  addFunctionPass(SafeStackPass(TM), StackProtectorPass(TM));

  // All passes which modify the LLVM IR are now complete; run the verifier
  // to ensure that the IR is valid.
  if (!CGPBO.DisableVerify)
    addFunctionPass(VerifierPass());
}

void TargetPassBuilder::addISelIRPasses() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();

  // Run loop strength reduction before anything else.
  if (TM->getOptLevel() == CodeGenOptLevel::None) {
    // Basic AliasAnalysis support.
    // Add TypeBasedAliasAnalysis before BasicAliasAnalysis so that
    // BasicAliasAnalysis wins if they disagree. This is intended to help
    // support "obvious" type-punning idioms.
    addFunctionPass(RequireAnalysisPass<TypeBasedAA, Function>(),
                    RequireAnalysisPass<ScopedNoAliasAA, Function>(),
                    RequireAnalysisPass<BasicAA, Function>());

    if (!CGPBO.DisableLSR) {
      addLoopPass(CanonicalizeFreezeInLoopsPass(), LoopStrengthReducePass());
      if (CGPBO.EnableLoopTermFold)
        addLoopPass(LoopTermFoldPass());
    }

    // The MergeICmpsPass tries to create memcmp calls by grouping sequences
    // of loads and compares. ExpandMemCmpPass then tries to expand those
    // calls into optimally-sized loads and compares. The transforms are
    // enabled by a target lowering hook.
    if (!CGPBO.DisableMergeICmps)
      addFunctionPass(MergeICmpsPass());
    addFunctionPass(ExpandMemCmpPass(TM));
  }

  // Run GC lowering passes for builtin collectors
  addFunctionPass(GCLoweringPass());
  addModulePass(ShadowStackGCLoweringPass());
  // PB.invokeGCLoweringEPCallbacks();

  if (TM->getTargetTriple().isOSBinFormatMachO() &&
      !CGPBO.DisableAtExitBasedGlobalDtorLowering)
    addModulePass(LowerGlobalDtorsPass());

  // Make sure that no unreachable blocks are instruction selected.
  addFunctionPass(UnreachableBlockElimPass());

  if (OptLevel != CodeGenOptLevel::None) {
    if (!CGPBO.DisableConstantHoisting)
      addFunctionPass(ConstantHoistingPass());
    if (!CGPBO.DisableReplaceWithVecLib)
      addFunctionPass(ReplaceWithVeclib());
    if (!CGPBO.DisablePartialLibcallInlining)
      addFunctionPass(PartiallyInlineLibCallsPass());
  }

  // Instrument function entry after all inlining.
  addFunctionPass(EntryExitInstrumenterPass(/*PostInlining=*/true));

  // Add scalarization of target's unsupported masked memory intrinsics pass.
  // the unsupported intrinsic will be replaced with a chain of basic blocks,
  // that stores/loads element one-by-one if the appropriate mask bit is set.
  addFunctionPass(ScalarizeMaskedMemIntrinPass());

  // Expand reduction intrinsics into shuffle sequences if the target wants
  // to. Allow disabling it for testing purposes.
  if (!CGPBO.DisableExpandReductions)
    addFunctionPass(ExpandReductionsPass());

  // Convert conditional moves to conditional jumps when profitable.
  if (OptLevel != CodeGenOptLevel::None && !CGPBO.DisableSelectOptimize)
    addFunctionPass(SelectOptimizePass(TM));

  if (CGPBO.EnableGlobalMergeFunc)
    addModulePass(GlobalMergeFuncPass());
}

void TargetPassBuilder::buildCoreCodeGenPipeline() {}

void TargetPassBuilder::anchor() {}
