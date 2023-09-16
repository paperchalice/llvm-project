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
#include "MMIXMCInstrInfo.h"
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
  if (Inst.getOpcode() == 0) {
    assert(Inst.getNumOperands() == 1 &&
           "this is a dummy instruction! for fixo!");
    auto Expr = Inst.getOperand(0).getExpr();
    MCFixup Fixup = MCFixup::create(0, Expr, FK_Data_8);
    Fixups.emplace_back(Fixup);
    return;
  }

  if (Inst.getOpcode() == MMO::MM) {
    CB.append({'\x98', '\x00', '\x00', '\x01'});
  }
  uint32_t Bits = getBinaryCodeForInstr(Inst, Fixups, STI);
  support::endian::write<uint32_t>(CB, Bits, support::big);
}

std::uint64_t
MMIXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isExpr()) {
    const auto *E = dyn_cast<MMIXMCExpr>(MO.getExpr());
    if (E && E->shouldEmitFixup()) {
      const bool IsJMP = MI.getNumOperands() == 1;
      auto FK = IsJMP ? MMIX::fixup_MMIX_jump : MMIX::fixup_MMIX_branch;
      Fixups.push_back(MCFixup::create(0, E, static_cast<MCFixupKind>(FK)));
    }
    return 0;
  }
  if (MO.isImm())
    return MO.getImm();
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  llvm_unreachable("Unhandled expression!");
}

MCCodeEmitter *createMMIXMCCodeEmitter(const MCInstrInfo &MCII,
                                       MCContext &Ctx) {
  return new MMIXMCCodeEmitter(MCII, Ctx);
}

} // namespace llvm
