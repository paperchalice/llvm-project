//===-- MMIXMCInstLower.h - Lower MachineInstr to MCInst ---------*- C++
//-*-===//
//
// Part of LLVM, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H
#define LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCContext.h"

namespace llvm {

class MMIXMCInstLower {
  MCContext &Ctx;
  const AsmPrinter &AP;

public:
  MMIXMCInstLower(MCContext &Ctx, const AsmPrinter &AP);

  void lower(const MachineInstr *MI, MCInst &OutMI);
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXMCINSTLOWER_H
