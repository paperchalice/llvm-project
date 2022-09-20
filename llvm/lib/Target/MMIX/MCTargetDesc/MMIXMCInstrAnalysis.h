//===-- MMIXMCInstrAnalysis.h - MMIX MC Instruction Analysis ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX declarations of the MCInstrAnalysis class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRANALYSIS_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRANALYSIS_H

#include "llvm/MC/MCInstrAnalysis.h"

namespace llvm {

class MMIXMCInstrAnalysis : public MCInstrAnalysis {
public:
  MMIXMCInstrAnalysis(const MCInstrInfo *Info);
};

MCInstrAnalysis *createMMIXInstrAnalysis(const MCInstrInfo *Info);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCINSTRANALYSIS_H
