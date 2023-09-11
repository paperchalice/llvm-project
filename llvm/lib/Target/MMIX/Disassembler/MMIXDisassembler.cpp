//===-- MMIXDisassembler.cpp - Disassembler for MMIX --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MMIXDisassembler class.
//
//===----------------------------------------------------------------------===//

#include "MMIXDisassembler.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "MMIXRegisterInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

#include <array>

using namespace llvm;
using DecodeStatus = MCDisassembler::DecodeStatus;

#define DEBUG_TYPE "mmix-disassembler"

namespace {

std::array SPRDecodeTable = {
    MMIX::rB,  MMIX::rD,  MMIX::rE,  MMIX::rH,  MMIX::rJ, MMIX::rM, MMIX::rR,
    MMIX::rBB, MMIX::rC,  MMIX::rN,  MMIX::rO,  MMIX::rS, MMIX::rI, MMIX::rT,
    MMIX::rTT, MMIX::rK,  MMIX::rQ,  MMIX::rU,  MMIX::rV, MMIX::rG, MMIX::rL,
    MMIX::rA,  MMIX::rF,  MMIX::rP,  MMIX::rW,  MMIX::rX, MMIX::rY, MMIX::rZ,
    MMIX::rWW, MMIX::rXX, MMIX::rYY, MMIX::rZZ,
};

DecodeStatus DecodeSPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder);
DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder);
template <std::size_t Width>
DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t RegNo, uint64_t Address,
                               const void *Decoder);
DecodeStatus decodeRoundMode(MCInst &Inst, uint64_t RegNo, uint64_t Address,
                             const void *Decoder);
} // end namespace

#include "MMIXGenDisassemblerTables.inc"

MMIXDisassembler::MMIXDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                                   MCInstrInfo const *MCII)
    : MCDisassembler(STI, Ctx), MCII(MCII) {}

MCDisassembler::DecodeStatus
MMIXDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                 ArrayRef<uint8_t> Bytes, uint64_t Address,
                                 raw_ostream &CStream) const {
  // Instruction size in byte, in MMIX, always 4 bytes
  Size = 4;
  // `Insn` is the encoded instruction value, e.g.
  // `ADD $0,$1,$2`, then the bytes in `Bytes` are
  // `0x20 0x00 0x01 0x02`, then read these bytes as
  // `std::uint32_t` in big endian, we get the `Insn`
  // then `decodeInstruction` will use these bits
  // decode the information
  std::uint32_t Insn = support::endian::read32be(Bytes.data());
  DecodeStatus Result =
      decodeInstruction(DecoderTable32, Instr, Insn, Address, this, STI);
  return Result;
}

namespace {

MCDisassembler *createMMIXDisassembler(const Target &T,
                                       const MCSubtargetInfo &STI,
                                       MCContext &Ctx) {
  return new MMIXDisassembler(STI, Ctx, T.createMCInstrInfo());
}

DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder) {
  MCRegister Reg = MMIX::r0 + RegNo;
  Inst.addOperand(MCOperand::createReg(Reg));
  return DecodeStatus::Success;
}

DecodeStatus DecodeSPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder) {
  if (RegNo < SPRDecodeTable.size()) {
    Inst.addOperand(MCOperand::createReg(SPRDecodeTable[RegNo]));
    return DecodeStatus::Success;
  }
  return DecodeStatus::Fail;
}

template <std::size_t Width>
DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm, uint64_t Address,
                               const void *Decoder) {
  std::uint64_t Mask = (1UL << Width) - 1;
  auto Val = Mask & Imm;
  Inst.addOperand(MCOperand::createImm(Val));
  return DecodeStatus::Success;
}

DecodeStatus decodeRoundMode(MCInst &Inst, uint64_t Mode, uint64_t Address,
                             const void *Decoder) {
  if (Mode == 0) {
    Inst.addOperand(MCOperand::createImm(0));
    return DecodeStatus::Success;
  }

  auto &Ctx = static_cast<const MCDisassembler *>(Decoder)->getContext();
  auto E = MCConstantExpr::create(Mode, Ctx);
  MMIXMCExpr::create(E, 0, MMIXMCExpr::VK_ROUND_MODE, Ctx);
  Inst.addOperand(MCOperand::createExpr(E));
  return DecodeStatus::Success;
}

} // end namespace

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheMMIXTarget(),
                                         createMMIXDisassembler);
}
