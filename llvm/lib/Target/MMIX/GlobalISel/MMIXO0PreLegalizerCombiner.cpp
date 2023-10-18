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

#define DEBUG_TYPE "mmix-o0-prelegalizer-combiner"

using namespace llvm;

namespace {

#define GET_GICOMBINER_TYPES
#include "MMIXGenO0PreLegalizerCombiner.inc"
#undef GET_GICOMBINER_TYPES

class MMIXO0PreLegalizerCombinerImpl : public Combiner {
protected:
  // TODO: Make CombinerHelper methods const.
  mutable CombinerHelper Helper;
  const MMIXO0PreLegalizerCombinerImplRuleConfig &RuleConfig;

public:
  MMIXO0PreLegalizerCombinerImpl(
      MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
      GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
      const MMIXO0PreLegalizerCombinerImplRuleConfig &RuleConfig,
      const MMIXSubtarget &STI);
  bool tryCombineAllImpl(MachineInstr &I) const;
  static const char *getName();

public:
  bool tryCombineAll(MachineInstr &I) const override;

private:
#define GET_GICOMBINER_CLASS_MEMBERS
#include "MMIXGenO0PreLegalizerCombiner.inc"
#undef GET_GICOMBINER_CLASS_MEMBERS
};

#define GET_GICOMBINER_IMPL
#include "MMIXGenO0PreLegalizerCombiner.inc"
#undef GET_GICOMBINER_IMPL

MMIXO0PreLegalizerCombinerImpl::MMIXO0PreLegalizerCombinerImpl(
    MachineFunction &MF, CombinerInfo &CInfo, const TargetPassConfig *TPC,
    GISelKnownBits &KB, GISelCSEInfo *CSEInfo,
    const MMIXO0PreLegalizerCombinerImplRuleConfig &RuleConfig,
    const MMIXSubtarget &STI)
    : Combiner(MF, CInfo, TPC, &KB, CSEInfo),
      Helper(Observer, B, /*IsPreLegalize*/ true, &KB), RuleConfig(RuleConfig),
#define GET_GICOMBINER_CONSTRUCTOR_INITS
#include "MMIXGenPreLegalizerCombiner.inc"
#undef GET_GICOMBINER_CONSTRUCTOR_INITS
{
}

const char *MMIXO0PreLegalizerCombinerImpl::getName() { return DEBUG_TYPE; }

bool MMIXO0PreLegalizerCombinerImpl::tryCombineAll(MachineInstr &I) const {
  return tryCombineAllImpl(I);
}

} // namespace

// Pass boilerplate
// ================

// TODO: migrate to new pass manager
class MMIXO0PreLegalizerCombiner : public MachineFunctionPass {
public:
  static char ID;
  MMIXO0PreLegalizerCombiner();
  StringRef getPassName() const override { return "MMIXO0PreLegalizerCombiner"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  MMIXO0PreLegalizerCombinerImplRuleConfig RuleConfig;
};

char MMIXO0PreLegalizerCombiner::ID = 0;

MMIXO0PreLegalizerCombiner::MMIXO0PreLegalizerCombiner() : MachineFunctionPass(ID) {
  initializeMMIXO0PreLegalizerCombinerPass(*PassRegistry::getPassRegistry());

  if (!RuleConfig.parseCommandLineOption())
    report_fatal_error("Invalid rule identifier");
}

bool MMIXO0PreLegalizerCombiner::runOnMachineFunction(MachineFunction &MF) {
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
  MMIXO0PreLegalizerCombinerImpl Impl(MF, CInfo, &TPC, *KB,
                                    /*CSEInfo*/ nullptr, RuleConfig, ST);
  return Impl.combineMachineInstrs();
}

void MMIXO0PreLegalizerCombiner::getAnalysisUsage(AnalysisUsage &AU) const {
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

INITIALIZE_PASS_BEGIN(MMIXO0PreLegalizerCombiner, DEBUG_TYPE, "Combine MMIX machine instrs",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(GISelKnownBitsAnalysis)
INITIALIZE_PASS_END(MMIXO0PreLegalizerCombiner, DEBUG_TYPE, "Combine MMIX machine instrs",
                    false, false)

FunctionPass * ::llvm::createMMIXO0PreLegalizerCombiner() { return new MMIXO0PreLegalizerCombiner(); }
