//===-- MMIXMCAsmInfo.cpp - MMIX Asm properties ---------------------------===//
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

using namespace llvm;

MMIXMCAsmInfoELF::MMIXMCAsmInfoELF(const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  CommentString = "%";
}
