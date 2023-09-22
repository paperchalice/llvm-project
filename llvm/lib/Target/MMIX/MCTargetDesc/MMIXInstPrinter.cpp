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
#include "MCTargetDesc/MMIXMCExpr.h"

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

MMIXInstPrinter::MMIXInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                                 const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void MMIXInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O) {
  const auto Operand = MI->getOperand(OpNo);
  if (Operand.isReg()) {
    auto RegName = getRegisterName(Operand.getReg());
    if (std::isdigit(RegName[0]))
      O << '$' << RegName;
    else
      O << RegName;
  }

  if (Operand.isImm())
    O << Operand.getImm();

  if (Operand.isExpr()) {
    if (auto E = dyn_cast<MMIXMCExpr>(Operand.getExpr())) {
      std::int64_t Res = 0;
      E->evaluateAsAbsolute(Res);
      switch (E->getKind()) {
      case MMIXMCExpr::VK_ROUND_MODE:
        switch (Res) {
        case 0:
          O << "ROUND_Current";
          break;
        case 1:
          O << "ROUND_OFF";
          break;
        case 2:
          O << "ROUND_UP";
          break;
        case 3:
          O << "ROUND_DOWN";
          break;
        case 4:
          O << "ROUND_NEAR";
          break;
        default:
          break;
        }
        break;
      case MMIXMCExpr::VK_MMIX_PC_REL_JMP:
      case MMIXMCExpr::VK_MMIX_PC_REL_BR:
        O << dyn_cast<MCSymbolRefExpr>(E->getExpr())->getSymbol().getName();
        break;
      default:
        break;
      }
    }
  }
}

void MMIXInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &OS) {
  if (!printAliasInstr(MI, Address, OS)) {
    printInstruction(MI, Address, OS);
  }
}

MCInstPrinter *createMMIXMCInstPrinter(const Triple &T, unsigned SyntaxVariant,
                                       const MCAsmInfo &MAI,
                                       const MCInstrInfo &MII,
                                       const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0) {
    return new MMIXInstPrinter(MAI, MII, MRI);
  }

  return nullptr;
}

} // namespace llvm
