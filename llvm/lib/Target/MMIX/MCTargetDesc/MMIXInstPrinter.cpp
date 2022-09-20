//==-- MMIXInstPrinter.cpp - Convert MMIX MCInst to assembly syntax --==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an MMIX MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "MMIXInstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "MMIXGenAsmWriter.inc"


MMIXInstPrinter::MMIXInstPrinter(const MCAsmInfo &MAI,
                                       const MCInstrInfo &MII,
                                       const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void MMIXInstPrinter::printOperand(const MCInst *MI, unsigned OpNo, 
                    raw_ostream &O) {
  const auto Operand = MI->getOperand(OpNo);
  if(Operand.isReg()) {
    O << getRegisterName(Operand.getReg());
  }
  if(Operand.isImm()) {
    O << Operand.getImm();
  }
  if(Operand.isExpr()) {
    O << Operand.getExpr();
  }
}

void MMIXInstPrinter::printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                         const MCSubtargetInfo &STI, raw_ostream &OS) {
  printInstruction(MI, Address, OS);
}

MCInstPrinter *createMMIXMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0) {
    return new MMIXInstPrinter(MAI, MII, MRI);
  }

  return nullptr;
}

}


