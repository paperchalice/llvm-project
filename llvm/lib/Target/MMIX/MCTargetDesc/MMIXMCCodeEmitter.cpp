//=- MMIX/MMIXMCCodeEmitter.cpp - Convert MMIX code to machine code----------=//
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
#include "MMIXFixupKinds.h"
#include "MMIXInstrInfo.h"
#include "MMIXMCExpr.h"
#include "MMIXMCTargetDesc.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#include "MMIXGenMCCodeEmitter.inc"

void MMIXMCCodeEmitter::encodeInstruction(const MCInst &Inst,
                                          SmallVectorImpl<char> &CB,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  if (Inst.getOpcode() == 0) {
    assert(Inst.getNumOperands() == 1 &&
           "this is a dummy instruction! for fixo!");
    return;
  }

  uint32_t Bits = getBinaryCodeForInstr(Inst, Fixups, STI);
  support::endian::write<uint32_t>(CB, Bits, endianness::big);
}

std::uint64_t
MMIXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isExpr()) {
    int FixupKind = MI.getOpcode() == MMIX::JMP
                        ? static_cast<int>(MMIX::fixup_MMIX_jmp)
                        : MCFixup::getDataKindForSize(2);
    Fixups.push_back(MCFixup::create(/*Offset=*/0, MO.getExpr(), FixupKind,
                                     /*PCRel=*/true));
    return 0;
  }
  if (MO.isImm())
    return MO.getImm();
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  llvm_unreachable("Unhandled expression!");
}
