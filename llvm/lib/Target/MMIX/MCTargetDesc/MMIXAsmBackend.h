//===-- MMIXAsmBackend.h - MMIX Assembler Backend -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMBACKEND_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMBACKEND_H

#include "MMIXObjectWriter.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"

namespace llvm {

class MCSubtargetInfo;
class MCTargetOptions;

class MMIXAsmBackend : public MCAsmBackend {
public:
  MMIXAsmBackend(const MCSubtargetInfo &STI, const MCRegisterInfo &MRI,
                 const MCTargetOptions &Options);

public:
  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  // we need opportunity to fix lop_fixo
  bool finishLayout() const override;

private:
  const MCSubtargetInfo &STI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMBACKEND_H
