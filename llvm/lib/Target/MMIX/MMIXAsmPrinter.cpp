//===- MMIXAsmPrinter.cpp - MMIX LLVM assembly writer ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the MMIX LLVM MC style assembly language.
//
//===----------------------------------------------------------------------===//

#include "MMIXAsmPrinter.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/CodeGen/AsmPrinterAnalysis.h"
#include "llvm/CodeGen/MachinePassManager.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

#define DEBUG_TYPE "mmix-asm-printer"

using namespace llvm;

void MMIXAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst LoweredInst;
  InstLower.lower(*MI, LoweredInst);
  EmitToStreamer(*OutStreamer, LoweredInst);
}

/// pass registry

// Force static initialization.
extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeMMIXAsmPrinter() {
  RegisterAsmPrinter<MMIXAsmPrinter> AP0(getTheMMIXTarget());
}

PreservedAnalyses MMIXAsmPrinterBeginPass::run(Module &M,
                                               ModuleAnalysisManager &MAM) {
  // Force the computation of SDPI so that it is available for the
  // actual pass, where it cannot be explicitly requested.
  MAM.getResult<StaticDataProfileInfoAnalysis>(M);
  auto &AsmPrinter = MAM.getResult<AsmPrinterAnalysis>(M).getPrinter();
  setupModuleAsmPrinter(M, MAM, AsmPrinter);
  AsmPrinter.doInitialization(M);
  return PreservedAnalyses::all();
}

PreservedAnalyses
MMIXAsmPrinterPass::run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM) {
  auto &AsmPrinter =
      MFAM.getResult<ModuleAnalysisManagerMachineFunctionProxy>(MF)
          .getCachedResult<AsmPrinterAnalysis>(*MF.getFunction().getParent())
          ->getPrinter();
  setupMachineFunctionAsmPrinter(MFAM, MF, AsmPrinter);
  AsmPrinter.runOnMachineFunction(MF);
  return PreservedAnalyses::all();
}

PreservedAnalyses MMIXAsmPrinterEndPass::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  auto &AsmPrinter = MAM.getCachedResult<AsmPrinterAnalysis>(M)->getPrinter();
  setupModuleAsmPrinter(M, MAM, AsmPrinter);
  AsmPrinter.doFinalization(M);
  return PreservedAnalyses::all();
}
