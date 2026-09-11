//==-- MMIXMCInstLower.cpp - Convert MMIX MachineInstr to an MCInst --------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file contains code to lower MMIX MachineInstrs to their corresponding
/// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "MMIXMCInstLower.h"

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Constants.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

MMIXMCInstLower::MMIXMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
    : Ctx(Ctx), Printer(Printer) {}

void MMIXMCInstLower::lower(const MachineInstr &MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI.getOpcode());

  for (const MachineOperand &MO : MI.operands()) {
    MCOperand MCOp;
    if (lowerOperand(MO, MCOp))
      OutMI.addOperand(MCOp);
  }
}

bool MMIXMCInstLower::lowerOperand(const MachineOperand &MO,
                                   MCOperand &MCOp) const {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_CImmediate:
    MCOp = MCOperand::createImm(MO.getCImm()->getZExtValue());
    break;
  case MachineOperand::MO_FPImmediate:
    break;
  default:
    return false;
  }
  return true;
}
