//===-- MMIXMCInstrInfo.h - MMIX MC Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX declarations of the MCInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRINFO_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRINFO_H

#include "llvm/MC/MCInstrInfo.h"

namespace llvm {

MCInstrInfo *createMMIXMCInstrInfo();

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRINFO_H
