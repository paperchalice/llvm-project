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

void MMIXMCCodeEmitter::encodeInstruction(const MCInst &Inst, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  support::endian::Writer W(OS, support::big);
  if (Inst.getOpcode() == MMO::MM) {
    OS.write("\x98\x00\x00\x01", 4);
  }
  uint32_t Bits = getBinaryCodeForInstr(Inst, Fixups, STI);
  if (!Fixups.empty()) {
    if (Fixups[0].getKind() == MCFixupKind::FK_Data_1) {
      Bits |= 1 << 24;
      Fixups.pop_back();
    }
  }
  W.write(Bits);
}

std::uint64_t
MMIXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isExpr()) {
    const auto *Expr = MO.getExpr();
    int64_t Res;
    bool Success = Expr->evaluateAsAbsolute(Res);
    if (Res < 0) {
      Res = -Res;
      Fixups.emplace_back(MCFixup::create(0, MCConstantExpr::create(0, Ctx),
                                          MCFixupKind::FK_Data_1));
    }
    return Success ? Res : 0; // else it is a future reference
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
