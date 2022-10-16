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
#include "GlobalISel/MMIXCallLowering.h"
#include "GlobalISel/MMIXLegalizerInfo.h"
#include "GlobalISel/MMIXRegisterBankInfo.h"
#include "GlobalISel/MMIXInstructionSelector.h"
#include "MMIXRegisterInfo.h"

#include "llvm/MC/TargetRegistry.h"
#include <memory>

#define DEBUG_TYPE "mmix-subtarget"

namespace llvm {

MMIXSubtarget::MMIXSubtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                             StringRef FS, StringRef ABIName,
                             const MMIXTargetMachine &TM)
    : MMIXGenSubtargetInfo(TT, CPU, TuneCPU, FS),
      TLInfo(TM, *this), FrameLowering(*this), InstrInfo(*this), RegInfo(),
      CallLoweringInfo(new MMIXCallLowering(*getTargetLowering())),
      Legalizer(new MMIXLegalizerInfo(*this)),
      RegBankInfo(new MMIXRegisterBankInfo(*getRegisterInfo())),
      InstSelector(new MMIXInstructionSelector(TM, *this, *RegBankInfo)) {}


const MMIXInstrInfo *MMIXSubtarget::getInstrInfo() const { return &InstrInfo; }
const MMIXRegisterInfo *MMIXSubtarget::getRegisterInfo() const {
  return &RegInfo;
}
const TargetLowering *MMIXSubtarget::getTargetLowering() const {
  return &TLInfo;
}
const MMIXFrameLowering *MMIXSubtarget::getFrameLowering() const {
  return &FrameLowering;
}

const CallLowering *MMIXSubtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}

InstructionSelector *MMIXSubtarget::getInstructionSelector() const {
  return InstSelector.get();
}

const LegalizerInfo *MMIXSubtarget::getLegalizerInfo() const {
  return Legalizer.get();
}

const RegisterBankInfo *MMIXSubtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}

} // namespace llvm

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MMIXGenSubtargetInfo.inc"

