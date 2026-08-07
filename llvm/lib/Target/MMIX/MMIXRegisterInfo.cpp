//===-- MMIXRegisterInfo.cpp - MMIX Register Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "MMIXRegisterInfo.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXFrameLowering.h"
#include "MMIXSubtarget.h"

#define GET_REGINFO_TARGET_DESC
#include "MMIXGenRegisterInfo.inc"

using namespace llvm;

MMIXRegisterInfo::MMIXRegisterInfo() : MMIXGenRegisterInfo(MMIX::r0) {}

const MCPhysReg *
MMIXRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedRegs[] = {0};
  return CalleeSavedRegs;
}

BitVector MMIXRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  return Reserved;
}

bool MMIXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  return false;
}

Register MMIXRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return MMIX::r253;
}
