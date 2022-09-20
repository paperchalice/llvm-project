//===-- MMIXMCInstrAnalysis.cpp - MMIX MC Instruction Analysis ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the MCInstrAnalysis class.
//
//===----------------------------------------------------------------------===//

#include "MMIXMCInstrAnalysis.h"

namespace llvm {

MMIXMCInstrAnalysis::MMIXMCInstrAnalysis(const MCInstrInfo *Info): MCInstrAnalysis(Info) {}

MCInstrAnalysis *createMMIXInstrAnalysis(const MCInstrInfo *Info) {
  return new MMIXMCInstrAnalysis(Info);
}

} // namespace llvm
