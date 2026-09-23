//===-- MMIXInstrInfo.cpp - MMIX Instruction Information ------------------===//
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
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXSubtarget.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "MMIXGenInstrInfo.inc"

using namespace llvm;

MMIXInstrInfo::MMIXInstrInfo(const MMIXSubtarget &STI)
    : MMIXGenInstrInfo(STI, RI, /*CFSetupOpcode=*/MMIX::ADJCALLSTACKDOWN,
                       /*CFDestroyOpcode=*/MMIX::ADJCALLSTACKUP,
                       /*CatchRetOpcode=*/~0u,
                       /*ReturnOpcode=*/MMIX::POP),
      STI(STI), RI() {}

void MMIXInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  dbgs() << "TODO: implement MMIXInstrInfo::storeRegToStackSlot";
}

void MMIXInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, Register DestReg,
                                Register SrcReg, bool KillSrc,
                                bool RenamableDest, bool RenamableSrc) const {
  auto MIB = MachineIRBuilder(*MI);
  constexpr uint64_t Zero = 0;
  MIB.buildInstr(MMIX::ORI, {DestReg}, {SrcReg, Zero});
}

bool MMIXInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  SmallVector<MachineInstr *, 4> BranchInstrs;
  for (auto &Terminator : llvm::make_filter_range(
           MBB.terminators(), [](MachineInstr &MI) { return MI.isBranch(); }))
    BranchInstrs.push_back(&Terminator);

  size_t TerminatorCnt = BranchInstrs.size();
  // case 1
  if (TerminatorCnt == 0)
    return false;

  // case 2
  if (TerminatorCnt == 1 && BranchInstrs[0]->isUnconditionalBranch()) {
    MachineInstr &TermInstr = *BranchInstrs[0];
    for (auto &MO : TermInstr.operands()) {
      if (!MO.isMBB())
        continue;
      TBB = MO.getMBB();
      return false;
    }
    return true;
  }

  // TODO: handle case 3 and 4
  return true;
}

unsigned MMIXInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  unsigned RemovedCnt = 0;
  for (auto &BranchInst : llvm::make_early_inc_range(llvm::make_filter_range(
           MBB, [](MachineInstr &MI) { return MI.isBranch(); }))) {
    BranchInst.removeFromParent();
    ++RemovedCnt;
  }

  if (BytesRemoved)
    *BytesRemoved = 4 * RemovedCnt;
  return RemovedCnt;
}
