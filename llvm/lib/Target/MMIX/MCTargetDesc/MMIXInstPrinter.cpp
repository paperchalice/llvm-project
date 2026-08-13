//==-- MMIXInstPrinter.cpp - Convert MMIX MCInst to assembly syntax --------==//
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
#include "MMIXMCTargetDesc.h"
#include "Utils/MMIXBaseInfo.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/MathExtras.h"

#include <array>

using namespace llvm;

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "MMIXGenAsmWriter.inc"

void MMIXInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << getRegisterName(Reg);
}

void MMIXInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &OS) {
  if (!printAliasInstr(MI, Address, OS))
    printInstruction(MI, Address, OS);
  printAnnotation(OS, Annot);
}

template <unsigned N, bool isS = true> static int getBranchImm(int64_t Imm) {
  if constexpr (isS)
    return SignExtend32<N>(Imm);
  else
    return Imm;
}

void MMIXInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
  } else if (Op.isImm()) {
    O << Op.getImm();
  } else if (Op.isExpr()) {
    const MCExpr *Expr = Op.getExpr();
    MAI.printExpr(O, *Expr);
  }
}

void MMIXInstPrinter::printBranchDestOperand(const MCInst *MI, unsigned OpNo,
                                             raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    int Imm = getBranchImm<17>(Op.getImm());
    O << '.';
    if (Imm == 0)
      return;
    O << (Imm > 0 ? '+' : '-') << std::abs(Imm) * 4;
    return;
  }
  const MCExpr *Expr = Op.getExpr();
  MAI.printExpr(O, *Expr);
}

void MMIXInstPrinter::printJmpDestOperand(const MCInst *MI, unsigned OpNo,
                                          raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    int Imm = getBranchImm<25>(Op.getImm());
    O << '.';
    if (Imm == 0)
      return;
    O << (Imm > 0 ? '+' : '-') << std::abs(Imm) * 4;
    return;
  }
  const MCExpr *Expr = Op.getExpr();
  MAI.printExpr(O, *Expr);
}

void MMIXInstPrinter::printRoundingModeOperand(const MCInst *MI, unsigned OpNo,
                                               raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  unsigned Imm = Op.getImm();

  std::array Modes = {
      "ROUND_CURRENT", "ROUND_OFF", "ROUND_UP", "ROUND_DOWN", "ROUND_NEAR",
  };
  if (Imm > Modes.size())
    O << Imm;
  else
    O << Modes[Imm];
}

static std::array TrapFuncPreDefs = {
    "Halt",   "Fopen", "Fclose", "Fread", "Fgets", "Fgetws",
    "Fwrite", "Fputs", "Fputws", "Fseek", "Ftell",
};

void MMIXInstPrinter::printTrapYOperand(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  unsigned Imm = Op.getImm();

  if (Imm > TrapFuncPreDefs.size())
    O << Imm;
  else
    O << TrapFuncPreDefs[Imm];
}

void MMIXInstPrinter::printTrapZOperand(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  const MCOperand &FOp = MI->getOperand(OpNo - 1);
  unsigned Imm = Op.getImm();
  unsigned FImm = FOp.getImm();

  if (FImm < TrapFuncPreDefs.size()) {
    StringRef FuncName = TrapFuncPreDefs[FImm];
    static std::array PreDefHandles = {"StdIn", "StdOut", "StdErr"};
    if (FuncName.starts_with('F') && Imm < PreDefHandles.size()) {
      O << PreDefHandles[Imm];
      return;
    }
  }
  O << Imm;
}

void MMIXInstPrinter::printSPRImmOperand(const MCInst *MI, unsigned OpNo,
                                         raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  unsigned Imm = Op.getImm();
  const MCRegisterClass &SPRClass = getMMIXMCRegisterClass(MMIX::SPRRegClassID);
  if (Imm >= SPRClass.getNumRegs()) {
    O << Imm;
    return;
  }
  MCRegister Reg = MMIX::getSPRFromEnc(Imm);
  O << getRegisterName(Reg);
}
