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

#include "MMIXCodeGenPassBuilder.h"

#include "llvm/CodeGen/GlobalISel/IRTranslator.h"

using namespace llvm;

Error MMIXCodeGenPassBuilder::addIRTranslator(PassManagerWrapper &PMW) {
  addMachineFunctionPass(IRTranslatorPass(getOptLevel()), PMW);
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addLegalizeMachineIR(PassManagerWrapper &PMW) {
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addRegBankSelect(PassManagerWrapper &PMW) {
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addGlobalInstructionSelect(
    PassManagerWrapper &PMW) {
  return Error::success();
}

void MMIXCodeGenPassBuilder::addAsmPrinter(PassManagerWrapper &PMW) {}
