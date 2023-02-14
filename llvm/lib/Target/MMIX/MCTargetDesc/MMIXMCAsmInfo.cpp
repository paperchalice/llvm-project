//===-- MMIXMCAsmInfo.cpp - MMIX Asm properties -------------------------===//
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

#include "MMIXMCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/CommandLine.h"

namespace llvm {

MMIXMCAsmInfoELF::MMIXMCAsmInfoELF(const Triple &T) {

}

MCAsmInfo *createMMIXMCAsmInfo(const MCRegisterInfo &MRI, const Triple &TT,
                      const MCTargetOptions &Options) {
  return new MMIXMCAsmInfoELF(TT);
}

} // namespace llvm
