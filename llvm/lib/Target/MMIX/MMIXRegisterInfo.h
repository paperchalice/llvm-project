//==- MMIXRegisterInfo.h - MMIX Register Information Impl --*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_H
#define LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_H

#include "MMIXFrameLowering.h"

#include "llvm/CodeGen/TargetRegisterInfo.h"

// Defines symbolic names for MMIX registers.
#define GET_REGINFO_ENUM
#include "MMIXGenRegisterInfo.inc"

#define GET_REGINFO_HEADER
#include "MMIXGenRegisterInfo.inc"

namespace llvm {

namespace MMIX {

constexpr std::array SPRDecodeTable = {
    MMIX::rB,  MMIX::rD,  MMIX::rE,  MMIX::rH,  MMIX::rJ, MMIX::rM, MMIX::rR,
    MMIX::rBB, MMIX::rC,  MMIX::rN,  MMIX::rO,  MMIX::rS, MMIX::rI, MMIX::rT,
    MMIX::rTT, MMIX::rK,  MMIX::rQ,  MMIX::rU,  MMIX::rV, MMIX::rG, MMIX::rL,
    MMIX::rA,  MMIX::rF,  MMIX::rP,  MMIX::rW,  MMIX::rX, MMIX::rY, MMIX::rZ,
    MMIX::rWW, MMIX::rXX, MMIX::rYY, MMIX::rZZ,
};

}

class MMIXRegisterInfo final : public MMIXGenRegisterInfo {
public:
  MMIXRegisterInfo();

public:
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool isAsmClobberable(const MachineFunction &MF,
                        MCRegister PhysReg) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

private:
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_H
