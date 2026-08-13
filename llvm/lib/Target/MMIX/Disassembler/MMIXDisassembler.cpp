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
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "TargetInfo/MMIXTargetInfo.h"
#include "Utils/MMIXBaseInfo.h"

#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"

#include <array>

using namespace llvm;
using namespace llvm::MCD;
using DecodeStatus = MCDisassembler::DecodeStatus;

#define DEBUG_TYPE "mmix-disassembler"

template <int ClassID>
static DecodeStatus DecodeMMIXRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder);

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

static MCDisassembler *createMMIXDisassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new MMIXDisassembler(STI, Ctx, T.createMCInstrInfo());
}

template <int ClassID>
DecodeStatus DecodeMMIXRegisterClass(MCInst &Inst, uint64_t RegNo,
                                     uint64_t Address, const void *Decoder) {
  const MCRegisterClass &RClass = getMMIXMCRegisterClass(ClassID);
  if (RegNo >= RClass.getNumRegs())
    return DecodeStatus::Fail;
  bool IsSPR = ClassID == MMIX::SPRRegClassID;
  MCRegister Reg =
      IsSPR ? MMIX::getSPRFromEnc(RegNo) : RClass.getRegister(RegNo);
  Inst.addOperand(MCOperand::createReg(Reg));
  return DecodeStatus::Success;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheMMIXTarget(),
                                         createMMIXDisassembler);
}
