//===-- MMIXFrameLowering.cpp - MMIX Frame Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MMIXFrameLowering.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"

namespace llvm {

MMIXFrameLowering::MMIXFrameLowering(const MMIXSubtarget &STI)
    : TargetFrameLowering(StackGrowsDown,
                          /*StackAlignment=*/Align(8),
                          /*LocalAreaOffset=*/0),
      STI(STI) {}

bool MMIXFrameLowering::hasFP(const MachineFunction &MF) const { return true; }
void MMIXFrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}
void MMIXFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

} // namespace llvm

