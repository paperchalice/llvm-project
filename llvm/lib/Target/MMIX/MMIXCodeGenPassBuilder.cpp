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
#include "MMIXAsmPrinter.h"

#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"

using namespace llvm;

Error MMIXCodeGenPassBuilder::addIRTranslator(PassManagerWrapper &PMW) {
  addMachineFunctionPass(IRTranslatorPass(getOptLevel()), PMW);
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addLegalizeMachineIR(PassManagerWrapper &PMW) {
  addMachineFunctionPass(LegalizerPass(), PMW);
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addRegBankSelect(PassManagerWrapper &PMW) {
  RegBankSelectMode Mode;
  switch (Opt.RegAlloc) {
  default:
  case RegAllocType::Fast:
    Mode = Fast;
    break;
  case RegAllocType::Greedy:
    Mode = Greedy;
    break;
  }
  addMachineFunctionPass(RegBankSelectPass(Mode), PMW);
  return Error::success();
}

Error MMIXCodeGenPassBuilder::addGlobalInstructionSelect(
    PassManagerWrapper &PMW) {
  addMachineFunctionPass(InstructionSelectPass(getOptLevel()), PMW);
  return Error::success();
}

void MMIXCodeGenPassBuilder::addAsmPrinterBegin(PassManagerWrapper &PMW) {
  addModulePass(MMIXAsmPrinterBeginPass(), PMW, /*Force=*/true);
}

void MMIXCodeGenPassBuilder::addAsmPrinter(PassManagerWrapper &PMW) {
  addMachineFunctionPass(MMIXAsmPrinterPass(), PMW);
}

void MMIXCodeGenPassBuilder::addAsmPrinterEnd(PassManagerWrapper &PMW) {
  addModulePass(MMIXAsmPrinterEndPass(), PMW, /*Force=*/true);
}
