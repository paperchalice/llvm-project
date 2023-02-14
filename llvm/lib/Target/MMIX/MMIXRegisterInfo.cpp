//===-- MMIXRegisterInfo.cpp - MMIX Register Information ------*- C++ -*-===//
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

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "MMIXGenRegisterInfo.inc"

using namespace llvm;

MMIXRegisterInfo::MMIXRegisterInfo()
    : MMIXGenRegisterInfo(MMIX::r0) {
}

const uint32_t *
MMIXRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const {
  return CSR_MMIX_RegMask;
}

const MCPhysReg *
MMIXRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_MMIX_SaveList;
}

BitVector MMIXRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  return Reserved;
}

bool MMIXRegisterInfo::isAsmClobberable(const MachineFunction &MF,
                                        MCRegister PhysReg) const {
  return false;
}

bool MMIXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const { return true; }

Register
MMIXRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return {};
}