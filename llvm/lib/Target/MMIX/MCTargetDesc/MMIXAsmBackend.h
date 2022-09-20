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
#include "MMIXFixupKinds.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

class MMIXAsmBackend : public MCAsmBackend {
public:
MMIXAsmBackend(const MCSubtargetInfo &STI,
                       const MCRegisterInfo &MRI,
                       const MCTargetOptions &Options);
public:
std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

unsigned getNumFixupKinds() const override;

void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                          const MCValue &Target, MutableArrayRef<char> Data,
                          uint64_t Value, bool IsResolved,
                          const MCSubtargetInfo *STI) const override;

bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                    const MCRelaxableFragment *DF,
                                    const MCAsmLayout &Layout) const override;

bool writeNopData(raw_ostream &OS, uint64_t Count, const MCSubtargetInfo *STI) const override;

private:
  const MCSubtargetInfo &STI;
};

MCAsmBackend *createMMIXAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options);

}

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMBACKEND_H
