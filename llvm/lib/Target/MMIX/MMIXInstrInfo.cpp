//===-- MMIXInstrInfo.cpp - MMIX Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MMIXInstrInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "mmix-instrinfo"

using namespace llvm;

MMIXInstrInfo::MMIXInstrInfo(MMIXSubtarget &STI) : STI(STI) {}

void MMIXInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {
  BuildMI(MBB, MI, DL, get(MMIX::ORI), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(0);
}

void MMIXInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg) const {
  MachineFunction &MF = *MBB.getParent();
  MachineIRBuilder Builder(MBB, MI);
  Builder.buildInstr(MMIX::STOI, {},
                     {SrcReg, TRI->getFrameRegister(MF),
                      static_cast<std::uint64_t>(FrameIndex)});
  outs() << "TODO: implement " << __func__ << "\n";
}

void MMIXInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  MachineFunction &MF = *MBB.getParent();
  MachineIRBuilder Builder(MBB, MI);
  auto Instr = Builder.buildInstrNoInsert(MMIX::LDO)
                   .addReg(DestReg)
                   .addReg(TRI->getFrameRegister(MF))
                   .addImm(FrameIndex);
  MBB.insert(MI, Instr.getInstr());
  outs() << "TODO: implement " << __func__ << "\n";
}
#define GET_INSTRINFO_CTOR_DTOR
#include "MMIXGenInstrInfo.inc"
