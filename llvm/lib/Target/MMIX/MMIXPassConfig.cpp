#include "MMIXPassConfig.h"
#include "GlobalISel/MMIXCombiner.h"

namespace llvm {}

using namespace llvm;

MMIXPassConfig::MMIXPassConfig(MMIXTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {
  TM.setGlobalISelAbort(GlobalISelAbortMode::Enable);
}

bool MMIXPassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
}

void MMIXPassConfig::addPreLegalizeMachineIR() {
  switch (getOptLevel()) {
  case CodeGenOptLevel::None:
    addPass(createMMIXO0PreLegalizerCombiner());
    break;
  default:
    addPass(createMMIXPreLegalizerCombiner());
    break;
  }
}

bool MMIXPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

bool MMIXPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool MMIXPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

// TODO: use custom register allocator to handle DEK calling convention
FunctionPass *MMIXPassConfig::createTargetRegisterAllocator(bool) {
  return createGreedyRegisterAllocator();
}
