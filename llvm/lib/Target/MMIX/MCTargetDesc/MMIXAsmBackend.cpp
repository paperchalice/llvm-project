//===-- MMIXAsmBackend.cpp - MMIX Assembler Backend ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MMIXAsmBackend class.
//
//===----------------------------------------------------------------------===//

#include "MMIXAsmBackend.h"
#include "MMIXFixupKinds.h"
#include "MMIXMCTargetDesc.h"

#include "llvm/Support/Endian.h"

#include <array>
#include <cmath>

using namespace llvm;

MMIXAsmBackend::MMIXAsmBackend(const MCSubtargetInfo &STI,
                               const MCRegisterInfo &MRI,
                               const MCTargetOptions &Options)
    : MCAsmBackend(endianness::big), STI(STI) {}

MCFixupKindInfo
MMIXAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  // Name                           Offset (bits) Size (bits)     Flags
  const static std::array<MCFixupKindInfo, MMIX::NumTargetFixupKinds> Infos = {
      {"fixup_MMIX_jmp", 0, 32, 0},
  };
  assert(unsigned(Kind - FirstTargetFixupKind) < MMIX::NumTargetFixupKinds &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

void MMIXAsmBackend::applyFixup(const MCFragment &, const MCFixup &Fixup,
                                const MCValue &Target, uint8_t *Data,
                                uint64_t UValue, bool IsResolved) {
  std::int64_t Value = UValue;
  Value /= 4;
  bool IsJmp = Fixup.getKind() == MMIX::fixup_MMIX_jmp;
  Data[0] |= Value < 0;
  Value = std::abs(Value);

  if (IsJmp) {
    Value |= Data[0] << 24;
    support::endian::write<uint32_t>(Data, Value, endianness::big);
  } else {
    support::endian::write<uint16_t>(Data + 2, Value, endianness::big);
  }
}

bool MMIXAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                  const MCSubtargetInfo *STI) const {
  // SWYM 0,0,0
  OS.write("\xC0\0\0\0", 4);
  return true;
}

bool MMIXAsmBackend::finishLayout() const { return false; }

std::unique_ptr<MCObjectTargetWriter>
MMIXAsmBackend::createObjectTargetWriter() const {
  auto Format = STI.getTargetTriple().getObjectFormat();
  switch (Format) {
  case Triple::ObjectFormatType::ELF:
    return createMMIXELFObjectWriter(0);

    // TODO: Use MMO if possible in future.
    // case Triple::ObjectFormatType::MMO:

  default:
    return nullptr;
  }
}
