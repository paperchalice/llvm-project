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
#include "MMIXFixupKinds.h"
#include "MMIXInstrInfo.h"
#include "MMIXMCExpr.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"

namespace llvm {

#include "MMIXGenMCCodeEmitter.inc"

MMIXMCCodeEmitter::MMIXMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
    : MCII(MCII), Ctx(Ctx) {}

void MMIXMCCodeEmitter::encodeInstruction(const MCInst &Inst,
                                          SmallVectorImpl<char> &CB,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  if (Inst.getOpcode() == MMO::MM) {
    CB.append({'\x98', '\x00', '\x00', '\x01'});
  }
  uint32_t Bits = getBinaryCodeForInstr(Inst, Fixups, STI);
  if (!Fixups.empty()) {
    assert(Fixups.size() == 1 && "only one fixup per instruction");
    if (Fixups[0].getKind() == MCFixupKind::FK_Data_1) {
      Bits |= 1 << 24;
      Fixups.clear();
    }
  }
  switch (Inst.getFlags()) {
  default:
    break;
  case MMIX::BASE_ADDRESS_ADJUST:
    Bits &= ~(1 << 24);
    break;
  case MMIX::DONT_EMIT:
    Bits = 0xC0000000;
    break;
  }
  support::endian::write<uint32_t>(CB, Bits, support::big);
}

std::uint64_t
MMIXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isExpr()) {
    const auto *Expr = MO.getExpr();
    int64_t Res = 0;
    bool Success = Expr->evaluateAsAbsolute(Res);
    if (Success) {
      if (Res < 0) {
        Fixups.emplace_back(MCFixup::create(0, MCConstantExpr::create(0, Ctx),
                                            MCFixupKind::FK_Data_1));
      }
      return Res;
    } else {
      // Fixups.emplace_back(MCFixup::create(0, MCConstantExpr::create(Res, Ctx),
      //                                     MCFixupKind(MMIX::FK_JMP)));
      return 0;
    }
  }
  if (MO.isImm()) {
    return MO.getImm();
  }

  if (MO.isReg()) {
    auto Value = Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
    return Value;
  }
  llvm_unreachable("Unhandled expression!");
}

MCCodeEmitter *createMMIXMCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx) {
  return new MMIXMCCodeEmitter(MCII, Ctx);
}

} // namespace llvm
