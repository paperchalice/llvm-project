//===-- MMIXMCInstrInfo.cpp - MMIX MC Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the MCInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MMIXMCInstrInfo.h"
#include "MMIXRegisterInfo.h"

#define GET_INSTRINFO_MC_DESC
#include "MMIXGenInstrInfo.inc"

namespace llvm {

MCInstrInfo *createMMIXMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMMIXMCInstrInfo(X);
  return X;
}

} // namespace llvm
