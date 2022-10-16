//=- MMIX/MMIXMCCodeEmitter.cpp - Convert MMIX code to machine code-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MMIXMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MMIXMCCodeEmitter.h"
#include "MMIXInstrInfo.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"

namespace llvm {

#include "MMIXGenMCCodeEmitter.inc"

MMIXMCCodeEmitter::MMIXMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : MCII(MCII), Ctx(Ctx) {}

void MMIXMCCodeEmitter::encodeInstruction(const MCInst &Inst, raw_ostream &OS,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const {
  uint32_t Bits = getBinaryCodeForInstr(Inst, Fixups, STI);
  support::endian::write(
      OS, Bits,
      STI.getTargetTriple().isLittleEndian() ? support::little : support::big);
}


unsigned
MMIXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());
  llvm_unreachable("Unhandled expression!");
  return 0;
}


MCCodeEmitter *createMMIXMCCodeEmitter(const MCInstrInfo &MCII,
                                                MCContext &Ctx) {
  return new MMIXMCCodeEmitter(MCII, Ctx);
}


} // namespace llvm;
