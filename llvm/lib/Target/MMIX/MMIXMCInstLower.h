//===-- MMIXMCInstLower.h - Lower MachineInstr to MCInst ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H
#define LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H

#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {

class AsmPrinter;
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;

class LLVM_LIBRARY_VISIBILITY MMIXMCInstLower {
public:
  MMIXMCInstLower(MCContext &ctx, AsmPrinter &printer);

public:
  void lower(const MachineInstr &MI, MCInst &OutMI) const;
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;

private:
  MCContext &Ctx;
  AsmPrinter &Printer;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H
