//===-- MMIXMCAsmInfo.h - MMIX Asm classes -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MMIXMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCASMINFO_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCASMINFO_H

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

class MCStreamer;
class Target;
class Triple;

struct MMIXMCAsmInfoELF : public MCAsmInfoELF {
  explicit MMIXMCAsmInfoELF(const Triple &T);
};

MCAsmInfo *createMMIXMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCASMINFO_H
