#include "MMIXCodeGenPassBuilder.h"
#include "MMIXTargetMachine.h"

namespace llvm {

MMIXCodeGenPassBuilder::MMIXCodeGenPassBuilder(
    MMIXTargetMachine &TM, CGPassBuilderOption Opt,
    PassInstrumentationCallbacks *PIC)
    : CodeGenPassBuilder<MMIXCodeGenPassBuilder>(TM, Opt, PIC) {
  // Target-specific `CGPassBuilderOption` could be overridden here.
}

// GlobalIsel API
void MMIXCodeGenPassBuilder::addIRPasses(AddIRPass &AddPass) const {}

Error MMIXCodeGenPassBuilder::addIRTranslator(AddMachinePass &AddPass) const {
  AddPass(IRTranslatorPass());
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addLegalizeMachineIR(
    AddMachinePass &AddPass) const {
  AddPass(LegalizerPass());
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addRegBankSelect(AddMachinePass &AddPass) const {
  AddPass(RegBankSelectPass());
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addGlobalInstructionSelect(
    AddMachinePass &AddPass) const {
  AddPass(InstructionSelectPass());
  return Error::success();
}

void MMIXCodeGenPassBuilder::addAsmPrinter(AddMachinePass &AddPass,
                                           CreateMCStreamer CallBack) const {}

} // namespace llvm
