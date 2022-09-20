//===-- MMIXAsmBackend.cpp - MMIX Assembler Backend ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the AVRAsmBackend class.
//
//===----------------------------------------------------------------------===//
#include "MMIXAsmBackend.h"
#include "MMIXObjectWriter.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

MMIXAsmBackend::MMIXAsmBackend(const MCSubtargetInfo &STI,
                               const MCRegisterInfo &MRI,
                               const MCTargetOptions &Options)
  : MCAsmBackend(support::little), STI(STI) {

}

unsigned MMIXAsmBackend::getNumFixupKinds() const { return MMIX::NumTargetFixupKinds; }

void MMIXAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target,
                                MutableArrayRef<char> Data, uint64_t Value,
                                bool IsResolved,
                                const MCSubtargetInfo *STI) const {}

bool MMIXAsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                          const MCRelaxableFragment *DF,
                                          const MCAsmLayout &Layout) const {
  return true;
}

bool MMIXAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count, const MCSubtargetInfo *STI) const {
  return true;
}

std::unique_ptr<MCObjectTargetWriter> MMIXAsmBackend::createObjectTargetWriter() const {
  auto Format = STI.getTargetTriple().getObjectFormat();
  switch(Format) {


  case Triple::ObjectFormatType::ELF:
    return createMMIXELFObjectWriter(true, 0);


  default:
    return nullptr;
  }
}

MCAsmBackend *createMMIXAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options) {
  return new MMIXAsmBackend(STI, MRI, Options);
}

} // namespace llvm
