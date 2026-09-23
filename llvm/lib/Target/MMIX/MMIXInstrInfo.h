//===-- MMIXInstrInfo.h - MMIX Instruction Information --------------------===//
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

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H

#include "MMIXRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "MMIXGenInstrInfo.inc"

namespace llvm {

class MMIXSubtarget;

class MMIXInstrInfo : public MMIXGenInstrInfo {
public:
  explicit MMIXInstrInfo(const MMIXSubtarget &STI);

  const MMIXRegisterInfo &getRegisterInfo() const { return RI; }

public:
  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

private:
  const MMIXSubtarget &STI;
  MMIXRegisterInfo RI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H
