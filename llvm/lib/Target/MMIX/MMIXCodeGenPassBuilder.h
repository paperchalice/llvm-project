//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Build CodeGen pass pipeline for MMIX
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H
#define LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H

#include "llvm/Passes/CodeGenPassBuilder.h"

namespace llvm {

class LLVM_LIBRARY_VISIBILITY MMIXCodeGenPassBuilder
    : public CodeGenPassBuilder {

public:
  using CodeGenPassBuilder::CodeGenPassBuilder;

public:
  Error addIRTranslator(PassManagerWrapper &PMW) override;
  Error addLegalizeMachineIR(PassManagerWrapper &PMW) override;
  Error addRegBankSelect(PassManagerWrapper &PMW) override;
  Error addGlobalInstructionSelect(PassManagerWrapper &PMW) override;
  void addAsmPrinter(PassManagerWrapper &PMW) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXCODEGENPASSBUILDER_H
