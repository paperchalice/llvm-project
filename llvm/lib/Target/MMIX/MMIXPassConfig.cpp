#include "MMIXPassConfig.h"

namespace llvm {

MMIXPassConfig::MMIXPassConfig(MMIXTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {
  TM.setGlobalISelAbort(GlobalISelAbortMode::Enable);
}

bool MMIXPassConfig::addIRTranslator() {
  addPass(new IRTranslator(getOptLevel()));
  return false;
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

} // namespace llvm
