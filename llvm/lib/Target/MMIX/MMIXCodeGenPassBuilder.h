#ifndef LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H
#define LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H

#include "llvm/CodeGen/CodeGenPassBuilder.h"

namespace llvm {

class MMIXTargetMachine;

struct MMIXCodeGenPassBuilder
    : public CodeGenPassBuilder<MMIXCodeGenPassBuilder> {
  MMIXCodeGenPassBuilder(MMIXTargetMachine &TM,
                         CGPassBuilderOption Opt = CGPassBuilderOption(),
                         PassInstrumentationCallbacks *PIC = nullptr);

public: // GlobalIsel
  void addIRPasses(AddIRPass &AddPass) const;
  Error addIRTranslator(AddMachinePass &AddPass) const;
  // void addPreLegalizeMachineIR(AddMachinePass &AddPass) const
  Error addLegalizeMachineIR(AddMachinePass &AddPass) const;
  // void addPreRegBankSelect(AddMachinePass &AddPass) const;
  Error addRegBankSelect(AddMachinePass &AddPass) const;
  // void addPreGlobalInstructionSelect(AddMachinePass &AddPass) const;
  Error addGlobalInstructionSelect(AddMachinePass &AddPass) const;

public: // MC
  void addAsmPrinter(AddMachinePass &AddPass, CreateMCStreamer CallBack) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H
