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
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

#define DEBUG_TYPE "mmix-instrinfo"

using namespace llvm;

MMIXInstrInfo::MMIXInstrInfo(MMIXSubtarget &STI)
    : MMIXGenInstrInfo(MMIX::ADJCALLSTACKDOWN, MMIX::ADJCALLSTACKUP, ~0u,
                       MMIX::POP),
      STI(STI) {}

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
  outs() << "TODO: implement " << __func__ << "\n";
  outs() << "store reg: " << SrcReg.id() << " to index " << FrameIndex << '\n';
  auto &MF = *MBB.getParent();
  auto &MFI = MF.getFrameInfo();

  auto PtrInfo = MachinePointerInfo::getFixedStack(MF, FrameIndex);
  auto *MMO = MF.getMachineMemOperand(PtrInfo, MachineMemOperand::MOStore,
                                      MFI.getObjectSize(FrameIndex),
                                      MFI.getObjectAlign(FrameIndex));

  MachineIRBuilder Builder(MBB, MI);
  if (RC->getID() == MMIX::SPRRegClassID) {
    Builder.buildInstr(MMIX::GET).addDef(MMIX::r16).addReg(SrcReg);
    Builder.buildInstr(MMIX::STOU)
        .addReg(MMIX::r16, RegState::Kill)
        .addReg(MMIX::r254)
        .addFrameIndex(FrameIndex)
        .addMemOperand(MMO);
  } else {
    assert(RC->getID() == MMIX::GPRRegClassID && "uknown register class ID!");
    Builder.buildInstr(MMIX::STOU)
        .addReg(SrcReg)
        .addFrameIndex(FrameIndex)
        .addMemOperand(MMO);
  }
}

void MMIXInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  outs() << "TODO: implement " << __func__ << "\n";
  outs() << "load reg: " << DestReg.id() << " to index " << FrameIndex << '\n';
}
#define GET_INSTRINFO_CTOR_DTOR
#include "MMIXGenInstrInfo.inc"
