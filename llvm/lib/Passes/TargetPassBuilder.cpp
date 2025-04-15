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
#include "llvm/CodeGen/ExpandFp.h"
#include "llvm/CodeGen/ExpandLargeDivRem.h"
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
#include <stack>

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

  return llvm::ModulePassManager();
}

void TargetPassBuilder::buildCodeGenIRPipeline() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();
  assert(TM && "Must have a valid TargetMachine!");

  ModulePassManager MPM;
  if (TM->useEmulatedTLS())
    MPM.addPass(LowerEmuTLSPass());
  MPM.addPass(PreISelIntrinsicLoweringPass(TM));
  FunctionPassManager FPM;
  FPM.addPass(ExpandLargeDivRemPass(TM));
  FPM.addPass(ExpandFpPass(TM));

  // Run loop strength reduction before anything else.
  if (TM->getOptLevel() == CodeGenOptLevel::None) {
    // Basic AliasAnalysis support.
    // Add TypeBasedAliasAnalysis before BasicAliasAnalysis so that
    // BasicAliasAnalysis wins if they disagree. This is intended to help
    // support "obvious" type-punning idioms.
    FPM.addPass(RequireAnalysisPass<TypeBasedAA, Function>());
    FPM.addPass(RequireAnalysisPass<ScopedNoAliasAA, Function>());
    FPM.addPass(RequireAnalysisPass<BasicAA, Function>());

    if (!CGPBO.DisableLSR) {
      LoopPassManager LPM;
      LPM.addPass(CanonicalizeFreezeInLoopsPass());
      LPM.addPass(LoopStrengthReducePass());
      if (CGPBO.EnableLoopTermFold)
        LPM.addPass(LoopTermFoldPass());
      FPM.addPass(createFunctionToLoopPassAdaptor(std::move(LPM)));
    }

    // The MergeICmpsPass tries to create memcmp calls by grouping sequences
    // of loads and compares. ExpandMemCmpPass then tries to expand those
    // calls into optimally-sized loads and compares. The transforms are
    // enabled by a target lowering hook.
    if (!CGPBO.DisableMergeICmps)
      FPM.addPass(MergeICmpsPass());
    FPM.addPass(ExpandMemCmpPass(TM));
  }

  // Run GC lowering passes for builtin collectors
  FPM.addPass(GCLoweringPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  FPM = FunctionPassManager();
  MPM.addPass(ShadowStackGCLoweringPass());
  // PB.invokeGCLoweringEPCallbacks();

  if (TM->getTargetTriple().isOSBinFormatMachO() &&
      !CGPBO.DisableAtExitBasedGlobalDtorLowering) {
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    FPM = FunctionPassManager();
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(LowerGlobalDtorsPass());
  }

  // Make sure that no unreachable blocks are instruction selected.
  FPM.addPass(UnreachableBlockElimPass());

  if (OptLevel != CodeGenOptLevel::None) {
    if (!CGPBO.DisableConstantHoisting)
      FPM.addPass(ConstantHoistingPass());
    if (!CGPBO.DisableReplaceWithVecLib)
      FPM.addPass(ReplaceWithVeclib());
    if (!CGPBO.DisablePartialLibcallInlining)
      FPM.addPass(PartiallyInlineLibCallsPass());
  }

  // Instrument function entry after all inlining.
  FPM.addPass(EntryExitInstrumenterPass(/*PostInlining=*/true));

  // Add scalarization of target's unsupported masked memory intrinsics pass.
  // the unsupported intrinsic will be replaced with a chain of basic blocks,
  // that stores/loads element one-by-one if the appropriate mask bit is set.
  FPM.addPass(ScalarizeMaskedMemIntrinPass());

  // Expand reduction intrinsics into shuffle sequences if the target wants
  // to. Allow disabling it for testing purposes.
  if (!CGPBO.DisableExpandReductions)
    FPM.addPass(ExpandReductionsPass());

  // Convert conditional moves to conditional jumps when profitable.
  if (OptLevel != CodeGenOptLevel::None && !CGPBO.DisableSelectOptimize)
    FPM.addPass(SelectOptimizePass(TM));

  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  if (CGPBO.EnableGlobalMergeFunc)
    MPM.addPass(GlobalMergeFuncPass());

  FPM.addPass(CodeGenPreparePass(PB.TM));
  FPM.addPass(buildExceptionHandlingPipeline());

  // Add common passes that perform LLVM IR to IR transforms in preparation for
  // instruction selection.

  if (OptLevel != CodeGenOptLevel::None)
    FPM.addPass(ObjCARCContractPass());

  FPM.addPass(CallBrPreparePass());

  // Add both the safe stack and the stack protection passes: each of them will
  // only protect functions that have corresponding attributes.
  FPM.addPass(SafeStackPass(TM));
  FPM.addPass(StackProtectorPass(TM));

  // All passes which modify the LLVM IR are now complete; run the verifier
  // to ensure that the IR is valid.
  if (!CGPBO.DisableVerify)
    FPM.addPass(VerifierPass());

  buildISelPipeline();
}

void TargetPassBuilder::buildISelPipeline() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();

  // Add core instruction selection passes.
  MachineFunctionPassManager MFPM;
  // Enable FastISel with -fast-isel, but allow that to be overridden.
  TM->setO0WantsFastISel(CGPBO.EnableFastISelOption.value_or(true));

  // Determine an instruction selector.
  enum class SelectorType { SelectionDAG, FastISel, GlobalISel };
  SelectorType Selector;

  if (CGPBO.EnableFastISelOption.value_or(false))
    Selector = SelectorType::FastISel;
  else if (CGPBO.EnableGlobalISelOption.value_or(false) ||
           (TM->Options.EnableGlobalISel &&
            CGPBO.EnableGlobalISelOption.value_or(true)))
    Selector = SelectorType::GlobalISel;
  else if (TM->getOptLevel() == CodeGenOptLevel::None &&
           TM->getO0WantsFastISel())
    Selector = SelectorType::FastISel;
  else
    Selector = SelectorType::SelectionDAG;

  // Set consistently TM->Options.EnableFastISel and EnableGlobalISel.
  if (Selector == SelectorType::FastISel) {
    TM->setFastISel(true);
    TM->setGlobalISel(false);
  } else if (Selector == SelectorType::GlobalISel) {
    TM->setFastISel(false);
    TM->setGlobalISel(true);
  }

  // FIXME: Currently debugify is not support in new pass manager,
  // because it inserts module passes between each pass.

  // Add instruction selector passes for global isel if enabled.
  if (Selector == SelectorType::GlobalISel) {
    MFPM.addPass(IRTranslatorPass());

    // addPreLegalizeMachineIR();

    // addLegalizeMachineIR()

    // Before running the register bank selector, ask the target if it
    // wants to run some passes.
    // addPreRegBankSelect();

    MFPM.addPass(RegBankSelectPass());

    // addPreGlobalInstructionSelect();

    MFPM.addPass(InstructionSelectPass(OptLevel));
  }

  // Pass to reset the MachineFunction if the ISel failed. Outside of the
  // above if so that the verifier is not added to it.
  MFPM.addPass(ResetMachineFunctionPass());

  // Run the SDAG InstSelector, providing a fallback path when we do not want
  // to abort on not-yet-supported input.
  if (Selector != SelectorType::GlobalISel ||
      TM->Options.GlobalISelAbort != GlobalISelAbortMode::Enable)
    (void)PB; // addInstSelector();

  // Expand pseudo-instructions emitted by ISel. Don't run the verifier
  // before FinalizeISel.
  MFPM.addPass(FinalizeISelPass());
}

void TargetPassBuilder::buildCodeGenMIRPipeline() {
  TargetMachine *TM = PB.TM;
  CodeGenOptLevel OptLevel = TM->getOptLevel();

  MachineFunctionPassManager MFPM;
  if (OptLevel != CodeGenOptLevel::None) {
    /// Add passes that optimize machine instructions in SSA form.

    // Pre-ra tail duplication.
    MFPM.addPass(EarlyTailDuplicatePass());

    // Optimize PHIs before DCE: removing dead PHI cycles may make more
    // instructions dead.
    MFPM.addPass(OptimizePHIsPass());

    // This pass merges large allocas. StackSlotColoring is a different pass
    // which merges spill slots.
    MFPM.addPass(StackColoringPass());

    // If the target requests it, assign local variables to stack slots relative
    // to one another and simplify frame index references where possible.
    MFPM.addPass(LocalStackSlotAllocationPass());

    // With optimization, dead code should already be eliminated. However
    // there is one known exception: lowered code for arguments that are only
    // used by tail calls, where the tail calls reuse the incoming stack
    // arguments directly (see t11 in test/CodeGen/X86/sibcall.ll).
    MFPM.addPass(DeadMachineInstructionElimPass());

    // Allow targets to insert passes that improve instruction level
    // parallelism, like if-conversion. Such passes will typically need
    // dominator trees and loop info, just like LICM and CSE below.
    // MFPM.addILPOpts();

    MFPM.addPass(EarlyMachineLICMPass());
    MFPM.addPass(MachineCSEPass());

    MFPM.addPass(MachineSinkingPass(CGPBO.EnableSinkAndFold));

    MFPM.addPass(PeepholeOptimizerPass());
    // Clean-up the dead code that may have been generated by peephole
    // rewriting.
    MFPM.addPass(DeadMachineInstructionElimPass());
  } else {
    // If the target requests it, assign local variables to stack slots relative
    // to one another and simplify frame index references where possible.
    MFPM.addPass(LocalStackSlotAllocationPass());
  }
}

TargetPassBuilder::FunctionPassManager
TargetPassBuilder::buildExceptionHandlingPipeline() {
  TargetMachine *TM = PB.TM;
  const MCAsmInfo *MCAI = TM->getMCAsmInfo();

  FunctionPassManager FPM;
  switch (MCAI->getExceptionHandlingType()) {
  case ExceptionHandling::SjLj:
    // SjLj piggy-backs on dwarf for this bit. The cleanups done apply to both
    // Dwarf EH prepare needs to be run after SjLj prepare. Otherwise,
    // catch info can get misplaced when a selector ends up more than one block
    // removed from the parent invoke(s). This could happen when a landing
    // pad is shared by multiple invokes and is also a target of a normal
    // edge from elsewhere.
    FPM.addPass(SjLjEHPreparePass(TM));
    [[fallthrough]];
  case ExceptionHandling::DwarfCFI:
  case ExceptionHandling::ARM:
  case ExceptionHandling::AIX:
  case ExceptionHandling::ZOS:
    FPM.addPass(DwarfEHPreparePass(TM));
    break;
  case ExceptionHandling::WinEH:
    // We support using both GCC-style and MSVC-style exceptions on Windows, so
    // add both preparation passes. Each pass will only actually run if it
    // recognizes the personality function.
    FPM.addPass(WinEHPreparePass());
    FPM.addPass(DwarfEHPreparePass(TM));
    break;
  case ExceptionHandling::Wasm:
    // Wasm EH uses Windows EH instructions, but it does not need to demote PHIs
    // on catchpads and cleanuppads because it does not outline them into
    // funclets. Catchswitch blocks are not lowered in SelectionDAG, so we
    // should remove PHIs there.
    FPM.addPass(WinEHPreparePass(/*DemoteCatchSwitchPHIOnly=*/true));
    FPM.addPass(WasmEHPreparePass());
    break;
  case ExceptionHandling::None:
    FPM.addPass(LowerInvokePass());
    // The lower invoke pass may create unreachable code. Remove it.
    FPM.addPass(UnreachableBlockElimPass());
    break;
  }
  return FPM;
}

void TargetPassBuilder::buildCoreCodeGenPipeline() {
  buildCodeGenIRPipeline();
  buildISelPipeline();
  buildCodeGenMIRPipeline();
}

ModulePassManager
TargetPassBuilder::constructRealPassManager(ModulePassManager &&MPMW) {
  using llvm::createFunctionToLoopPassAdaptor,
      llvm::createFunctionToMachineFunctionPassAdaptor,
      llvm::createModuleToFunctionPassAdaptor;
  using llvm::ModulePassManager, llvm::FunctionPassManager,
      llvm::LoopPassManager, llvm::MachineFunctionPassManager;

  ModulePassManager MPM;
  FunctionPassManager FPM;
  LoopPassManager LPM;
  MachineFunctionPassManager MFPM;

  std::stack<size_t> S({0});
  for (auto &P : MPMW.Passes) {
    // StringRef Name = P.first;
    auto &PMVar = P.second;
    std::visit(
        [&](auto &&PM) {
          size_t VarIdx = PMVar.index();
          while (VarIdx < S.top()) {
            switch (S.top()) {
            case 3:
              if (MFPM.isEmpty())
                FPM.addPass(createFunctionToMachineFunctionPassAdaptor(
                    std::move(MFPM)));
              MFPM = llvm::MachineFunctionPassManager();
              S.pop();
              break;
            case 2:
              if (!LPM.isEmpty())
                FPM.addPass(createFunctionToLoopPassAdaptor(
                    std::move(LPM), /*UseMemorySSA=*/true));
              LPM = llvm::LoopPassManager();
              S.pop();
              break;
            case 1:
              if (!FPM.isEmpty())
                MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
              FPM = llvm::FunctionPassManager();
              S.pop();
              break;
            case 0:
              break;
            default:
              llvm_unreachable("");
            }
            if (VarIdx > S.top())
              S.push(VarIdx);
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(PM)>,
                                         ModulePassManager>)
              MPM.addPass(std::move(PM));
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(PM)>,
                                         FunctionPassManager>)
              FPM.addPass(std::move(PM));
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(PM)>,
                                         LoopPassManager>)
              LPM.addPass(std::move(PM));
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(PM)>,
                                         MachineFunctionPassManager>)
              MFPM.addPass(std::move(PM));
          }
        },
        PMVar);
  }

  if (!MFPM.isEmpty())
    FPM.addPass(createFunctionToMachineFunctionPassAdaptor(std::move(MFPM)));
  if (!LPM.isEmpty())
    FPM.addPass(
        createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true));
  if (!FPM.isEmpty())
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

  return MPM;
}

void TargetPassBuilder::anchor() {}
