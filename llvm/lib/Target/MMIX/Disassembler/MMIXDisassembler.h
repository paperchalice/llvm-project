//===-- MMIXDisassembler.h - Disassembler for MMIX --------------------===//
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

#ifndef  LLVM_LIB_TARGET_MMIX_ASM_DISASSEMBLER_MMIXDISASSEMBLER_H
#define  LLVM_LIB_TARGET_MMIX_ASM_DISASSEMBLER_MMIXDISASSEMBLER_H

#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include <memory>

namespace llvm {

class  MMIXDisassembler : public MCDisassembler {
  std::unique_ptr<MCInstrInfo const> const MCII;

public:
  MMIXDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                    MCInstrInfo const *MCII);

  MCDisassembler::DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_DISASSEMBLER_MMIXDISASSEMBLER_H
