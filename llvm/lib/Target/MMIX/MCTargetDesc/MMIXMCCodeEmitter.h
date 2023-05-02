//=- MMIX/MMIXMCCodeEmitter.cpp - Convert MMIX code to machine code-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the MMIXMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCCODEEMITTER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCCODEEMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"

namespace llvm {

class MMIXMCCodeEmitter : public MCCodeEmitter {
public:
MMIXMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx);

public:
void encodeInstruction(const MCInst &Inst, raw_ostream &OS,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const override;
  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;
  // getMachineOpValue - Return binary encoding of operand. If the machin
  // operand requires relocation, record the relocation and return zero.
  std::uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
private:
const MCInstrInfo &MCII;
MCContext &Ctx;
};

MCCodeEmitter *
createMMIXMCCodeEmitter(const MCInstrInfo &MCII,
                                const MCRegisterInfo &MRI,
                                MCContext &Ctx);

}

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCCODEEMITTER_H
