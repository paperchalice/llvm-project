//===- MMIXInstPrinter.h - Convert MMIX MCInst to assembly syntax -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXINSTPRINTER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXINSTPRINTER_H

#include "llvm/ADT//Triple.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInst.h"

namespace llvm {

class MMIXInstPrinter : public MCInstPrinter {
public:
MMIXInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                     const MCRegisterInfo &MRI);

std::pair<const char *, uint64_t> getMnemonic(const MCInst *MI) override;

void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                         const MCSubtargetInfo &STI, raw_ostream &OS) override;
public:
static const char *getRegisterName(unsigned RegNo);
void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
bool printAliasInstr(const MCInst *MI, uint64_t Address, raw_ostream &O);
void printOperand(const MCInst *MI, unsigned OpNo, 
                    raw_ostream &O);
};

MCInstPrinter *createMMIXMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXINSTPRINTER_H
