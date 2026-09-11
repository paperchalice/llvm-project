//===- MMIXAsmPrinter.h - MMIX LLVM assembly writer -----------------------===//
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

#ifndef LLVM_LIB_TARGET_MMIX_MMIXASMPRINTER_H
#define LLVM_LIB_TARGET_MMIX_MMIXASMPRINTER_H

#include "MMIXMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class LLVM_LIBRARY_VISIBILITY MMIXAsmPrinter : public AsmPrinter {
public:
  void emitInstruction(const MachineInstr *MI) override;

public:
  MMIXAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), InstLower(OutContext, *this) {}

private:
  MMIXMCInstLower InstLower;
};

class MMIXAsmPrinterBeginPass
    : public RequiredPassInfoMixin<MMIXAsmPrinterBeginPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

class MMIXAsmPrinterPass : public RequiredPassInfoMixin<MMIXAsmPrinterPass> {
public:
  PreservedAnalyses run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM);
  // AsmPrinter needs to run regardless of optimization level.
};

class MMIXAsmPrinterEndPass
    : public RequiredPassInfoMixin<MMIXAsmPrinterEndPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXASMPRINTER_H
