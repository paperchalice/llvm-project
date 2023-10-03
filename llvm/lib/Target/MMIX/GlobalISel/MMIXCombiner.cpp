#include "MMIXCombiner.h"
#include "MMIX.h"
#include "MMIXSubtarget.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/Combiner.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
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

namespace {

#define GET_GICOMBINER_TYPES
#include "MMIXGenGICombiner.inc"
#undef GET_GICOMBINER_TYPES

class MMIXCombinerImpl : public Combiner {
protected:
  // TODO: Make CombinerHelper methods const.
  mutable CombinerHelper Helper;
  const MMIXCombinerImplRuleConfig &RuleConfig;

public:
  MMIXCombinerImpl(MachineFunction &MF, CombinerInfo &CInfo,
                   const TargetPassConfig *TPC, GISelKnownBits &KB,
                   GISelCSEInfo *CSEInfo,
                   const MMIXCombinerImplRuleConfig &RuleConfig,
                   const MMIXSubtarget &STI);
  bool tryCombineAllImpl(MachineInstr &I) const;
  static const char *getName();

public:
  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "MMIXGenGICombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "MMIXGenGICombiner.inc"
#undef GET_GICOMBINER_IMPL

MMIXCombinerImpl::MMIXCombinerImpl(MachineFunction &MF, CombinerInfo &CInfo,
                                   const TargetPassConfig *TPC,
                                   GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
                                   const MMIXCombinerImplRuleConfig &RuleConfig,
                                   const MMIXSubtarget &STI)
    : Combiner(MF, CInfo, TPC, &KB, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ true, &KB), RuleConfig(RuleConfig),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "MMIXGenGICombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

const char *MMIXCombinerImpl::getName() { return "MMIXCombiner"; }

bool MMIXCombinerImpl::tryCombineAll(MachineInstr &I) const {
  return tryCombineAllImpl(I);
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

private:
  MMIXCombinerImplRuleConfig RuleConfig;
};

char MMIXCombiner::ID = 0;

MMIXCombiner::MMIXCombiner() : MachineFunctionPass(ID) {
  initializeMMIXCombinerPass(*PassRegistry::getPassRegistry());

  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool MMIXCombiner::runOnMachineFunction(MachineFunction &MF) {
  if (MF.getProperties().hasProperty(
          MachineFunctionProperties::Property::FailedISel)) {
    return false;
  }
  auto &TPC = getAnalysis<TargetPassConfig>();

  const Function &F = MF.getFunction();
  GISelKnownBits *KB = &getAnalysis<GISelKnownBitsAnalysis>().get(MF);

  const auto &ST = MF.getSubtarget<MMIXSubtarget>();

  CombinerInfo CInfo(/*AllowIllegalOps*/ true, /*ShouldLegalizeIllegal*/ false,
                     /*LegalizerInfo*/ nullptr, /*EnableOpt*/ false,
                     F.hasOptSize(), F.hasMinSize());
  MMIXCombinerImpl Impl(MF, CInfo, &TPC, *KB,
                        /*CSEInfo*/ nullptr, RuleConfig, ST);    
  return Impl.combineMachineInstrs();
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

FunctionPass * ::llvm::createMMIXCombiner() { return new MMIXCombiner(); }
