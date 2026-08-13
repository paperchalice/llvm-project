//===-- MMIXObjectWriter.h - MMIX Target Descriptions ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H

#include "llvm/MC/MCELFObjectWriter.h"

#include <memory>

namespace llvm {

class MMIXELFObjectWriter : public MCELFObjectTargetWriter {
public:
  MMIXELFObjectWriter(uint8_t OSABI);
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override;

protected:
private:
};

std::unique_ptr<MCObjectTargetWriter> createMMIXELFObjectWriter(uint8_t OSABI);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXOBJECTWRITER_H
