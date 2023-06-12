#include "MMIXCombiner.h"
#include "MMIX.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GISelChangeObserver.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegionInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetPassConfig.h"

#define DEBUG_TYPE "mmix-combiner"

using namespace llvm;

class MMIXCombinerHelperState {
protected:
  CombinerHelper &Helper;

public:
  MMIXCombinerHelperState(CombinerHelper &Helper) : Helper(Helper) {}
};

namespace {

#define MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
#include "MMIXGenGICombiner.inc"
#undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_H

class MMIXCombinerInfo : public CombinerInfo {
  GISelKnownBits *KB;
  MachineDominatorTree *MDT;
  MMIXGenCombinerHelperRuleConfig GeneratedRuleCfg;

public:
  MMIXCombinerInfo(bool EnableOpt, bool OptSize, bool MinSize,
                   GISelKnownBits *KB, MachineDominatorTree *MDT)
      : CombinerInfo(/*AllowIllegalOps*/ true,
                     /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, EnableOpt, OptSize, MinSize),
        KB(KB), MDT(MDT) {
    if (!GeneratedRuleCfg.parseCommandLineOption())
      report_fatal_error("Invalid rule identifier");
  }
  bool combine(GISelChangeObserver &Observer, MachineInstr &MI,
               MachineIRBuilder &B) const override;
};

#define MMIXCOMBINERHELPER_GENCOMBINERHELPER_CPP
#include "MMIXGenGICombiner.inc"
#undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_CPP

bool MMIXCombinerInfo::combine(GISelChangeObserver &Observer, MachineInstr &MI,
                               MachineIRBuilder &B) const {
  const LegalizerInfo *LI = MI.getMF()->getSubtarget().getLegalizerInfo();
  bool IsPreLegalize = !MI.getMF()->getProperties().hasProperty(
      MachineFunctionProperties::Property::Legalized);
  CombinerHelper Helper(Observer, B, IsPreLegalize, KB, MDT, LI);
  MMIXGenCombinerHelper Generated(GeneratedRuleCfg, Helper);
  return Generated.tryCombineAll(Observer, MI, B, Helper);
}

} // namespace

// Pass boilerplate
// ================

// TODO: migrate to new pass manager
class MMIXCombiner : public MachineFunctionPass {
public:
  static char ID;
  MMIXCombiner();
  StringRef getPassName() const override { return "MMIXCombiner"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

char MMIXCombiner::ID = 0;

MMIXCombiner::MMIXCombiner() : MachineFunctionPass(ID) {
  initializeMMIXCombinerPass(*PassRegistry::getPassRegistry());
}

bool MMIXCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel)) {
    return false;
  }
  auto *TPC = &getAnalysis<TargetPassConfig>();

  // Enable CSE.
  GISelCSEAnalysisWrapper &Wrapper =
      getAnalysis<GISelCSEAnalysisWrapperPass>().getCSEWrapper();
  auto *CSEInfo = &Wrapper.get(TPC->getCSEConfig());

  const Function &F = MF.getFunction();
  bool EnableOpt =
      MF.getTarget().getOptLevel() != CodeGenOpt::None && !skipFunction(F);
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);
  MachineDominatorTree *MDT = &getAnalysis<MachineDominatorTree>();
  MMIXCombinerInfo PCInfo(EnableOpt, F.hasOptSize(), F.hasMinSize(), KB, MDT);
  Combiner C(PCInfo, TPC);
  return C.combineMachineInstrs(MF, CSEInfo);
}

void MMIXCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetPassConfig>();
  AU.setPreservesCFG();
  getSelectionDAGFallbackAnalysisUsage(AU);
  AU.addRequired<GISelKnownBitsAnalysis>();
  AU.addPreserved<GISelKnownBitsAnalysis>();
  AU.addRequired<MachineDominatorTree>();
  AU.addPreserved<MachineDominatorTree>();
  AU.addRequired<GISelCSEAnalysisWrapperPass>();
  AU.addPreserved<GISelCSEAnalysisWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

INITIALIZE_PASS_BEGIN(MMIXCombiner, DEBUG_TYPE, "Combine MMIX machine instrs",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_END(MMIXCombiner, DEBUG_TYPE, "Combine MMIX machine instrs",
                    false, false)

FunctionPass *::llvm::createMMIXCombiner() { return new MMIXCombiner; }
