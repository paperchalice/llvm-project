//===-- MMIXSubtarget.cpp - MMIX Subtarget Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MMIX specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "MMIXSubtarget.h"

#define DEBUG_TYPE "mmix-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MMIXGenSubtargetInfo.inc"

using namespace llvm;

MMIXSubtarget::MMIXSubtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                             StringRef FS, const MMIXTargetMachine &TM)
    : MMIXGenSubtargetInfo(TT, CPU, TuneCPU, FS), FrameLowering(*this),
      InstrInfo(initSubtargetDependencies(CPU, FS)) {}

MMIXSubtarget &MMIXSubtarget::initSubtargetDependencies(StringRef CPU,
                                                        StringRef FS) {
  ParseSubtargetFeatures(CPU, /*TuneCPU=*/CPU, FS);
  return *this;
}
