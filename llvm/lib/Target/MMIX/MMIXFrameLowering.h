//===-- MMIXFrameLowering.h - Define frame lowering for MMIX -*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements MMIX-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXFRAMELOWERING_H
#define LLVM_LIB_TARGET_MMIX_MMIXFRAMELOWERING_H

#include "MMIXInstrInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class MMIXSubtarget;
class MMIXFrameLowering : public TargetFrameLowering {
public:
  explicit MMIXFrameLowering(const MMIXSubtarget &STI);

public:
  bool hasFP(const MachineFunction &MF) const override;
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

private:
  const MMIXSubtarget &STI;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXFRAMELOWERING_H
