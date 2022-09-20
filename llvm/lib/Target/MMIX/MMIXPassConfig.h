#ifndef LLVM_LIB_TARGET_MMIX_MMIXPASSCONFIG_H
#define LLVM_LIB_TARGET_MMIX_MMIXPASSCONFIG_H

#include "MMIXTargetMachine.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"

namespace llvm {

class MMIXPassConfig : public TargetPassConfig {
public:
  MMIXPassConfig(MMIXTargetMachine &TM, PassManagerBase &PM);

public:
 // GlobalISel API
  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXPASSCONFIG_H
