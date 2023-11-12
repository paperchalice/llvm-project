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
#include "MMIXFixupKinds.h"
#include "MMIXObjectWriter.h"
#include "llvm/ADT/APInt.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

const MCFixupKindInfo &
llvm::MMIXAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  static MCFixupKindInfo Infos[] = {
#define MMIX_FIXUP_X(A0, A1, A2, A3) {#A0, A1, A2, A3},
#include "MMIXFixupX.def"
  };
  if (Kind >= MCFixupKind::FirstTargetFixupKind &&
      (MMIX::Fixups)Kind < MMIX::LastTargetFixupKind) {
    return Infos[Kind - MCFixupKind::FirstTargetFixupKind];
  } else
    return MCAsmBackend::getFixupKindInfo(Kind);
}

MMIXAsmBackend::MMIXAsmBackend(const MCSubtargetInfo &STI,
                               const MCRegisterInfo &MRI,
                               const MCTargetOptions &Options)
    : MCAsmBackend(endianness::big), STI(STI) {}

unsigned MMIXAsmBackend::getNumFixupKinds() const {
  return MMIX::NumTargetFixupKinds;
}

void MMIXAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target,
                                MutableArrayRef<char> Data, uint64_t Value,
                                bool IsResolved,
                                const MCSubtargetInfo *STI) const {
  if (!Asm.getRelaxAll())
    return;
  std::int64_t Res;
  Fixup.getValue()->evaluateAsAbsolute(Res);
  auto Dest = Data.begin() + Fixup.getOffset();
  if (Res < 0)
    Dest[0] += 1;
  switch (static_cast<unsigned>(Fixup.getKind())) {
  case FK_Data_8:
    support::endian::write64be(Dest, Value);
    break;
  case MMIX::fixup_MMIX_jump:
    Dest[1] = Res >> 16;
    [[fallthrough]];
  case MMIX::fixup_MMIX_branch:
    Dest[2] = Res >> 8;
    Dest[3] = Res;
    break;
  default:
    break;
  }
}

bool MMIXAsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                          const MCRelaxableFragment *DF,
                                          const MCAsmLayout &Layout) const {
  return false;
}

bool MMIXAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                  const MCSubtargetInfo *STI) const {
  char NOP[] = {'\xc0', '\x00', '\x00', '\x00'};
  for (uint64_t I = 0; I != Count; ++I) {
    OS.write(NOP, sizeof(NOP));
  }
  return true;
}

void MMIXAsmBackend::finishLayout(MCAssembler const &Asm,
                                  MCAsmLayout &Layout) const {
  auto &Sections = Layout.getSectionOrder();
  for (auto *Sec : Sections) {
    auto &Fragments = Sec->getFragmentList();
    bool ShouldInvalidate = false;
    for (auto &F : Fragments) {
      if (auto *DF = dyn_cast<MCDataFragment>(&F)) {
        for (auto Fixup : DF->getFixups()) {
          if (Fixup.getKind() == FK_Data_8) {
            auto *Expr = Fixup.getValue();
            std::int64_t Res = 0;
            Expr->evaluateAsAbsolute(Res);
            auto &Content = DF->getContents();
            auto Start = Content.begin() + Fixup.getOffset();
            assert(Start + 16 < Content.end() &&
                   "there must be 16 bytes here!");
            if (Hi_32(Res) >> 24 != MMO::MM) {
              Start = Content.erase(Start, Start + 4) + 4;
            }
            if (Lo_32(Res) >> 24 != MMO::MM) {
              Content.erase(Start, Start + 4);
            }
            ShouldInvalidate = true;
          }
        }
      }
      if (ShouldInvalidate)
        Layout.invalidateFragmentsFrom(&F);
    }
  }
}

std::unique_ptr<MCObjectTargetWriter>
MMIXAsmBackend::createObjectTargetWriter() const {
  auto Format = STI.getTargetTriple().getObjectFormat();
  switch (Format) {
  case Triple::ObjectFormatType::ELF:
    return createMMIXELFObjectWriter(true, 0);

  default:
    return nullptr;
  }
}

MCAsmBackend *llvm::createMMIXAsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new MMIXAsmBackend(STI, MRI, Options);
}
