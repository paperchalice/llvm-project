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
#include "MMIXRegisterInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/CodeGen/Register.h"
#include "llvm/Support/Endian.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/MC/MCDecoderOps.h"

using namespace llvm;
using DecodeStatus = MCDisassembler::DecodeStatus;

#define DEBUG_TYPE "mmix-disassembler"



namespace {
DecodeStatus DecodeSPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder);
DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder);
template <std::size_t Width>
DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder);
} // end namespace

#include "MMIXGenDisassemblerTables.inc"

MMIXDisassembler::MMIXDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                                   MCInstrInfo const *MCII)
    : MCDisassembler(STI, Ctx), MCII(MCII) {}

MCDisassembler::DecodeStatus MMIXDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                            ArrayRef<uint8_t> Bytes, uint64_t Address,
                            raw_ostream &CStream) const {
  uint32_t Insn = 0;
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
  auto RegInfo = static_cast<const MCDisassembler *>(Decoder)->getContext().getRegisterInfo();
  RegInfo->getRegClass(MMIX::GPRRegClassID);
  MCRegister Reg = MMIX::r0 + RegNo;
  Inst.addOperand(MCOperand::createReg(Reg));
  return DecodeStatus::Success;
}

DecodeStatus DecodeSPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder) {
  return DecodeStatus::Success;
}

template <std::size_t Width>
DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t RegNo,
                                    uint64_t Address, const void *Decoder) {
  return DecodeStatus::Success;
}

} // end namespace

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheMMIXTarget(),
                                         createMMIXDisassembler);
}
