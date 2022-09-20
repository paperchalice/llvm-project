//===-- MMIXRegisterBankInfo.cpp -------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//


#include "MMIXRegisterBankInfo.h"
#include "MMIXRegisterInfo.h"

#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "llvm/CodeGen/GlobalISel/RegisterBank.h"
#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_TARGET_REGBANK_IMPL
#include "MMIXGenRegisterBank.inc"

using namespace llvm;

MMIXRegisterBankInfo::MMIXRegisterBankInfo(const TargetRegisterInfo &TRI)
    : MMIXGenRegisterBankInfo() {}

const RegisterBankInfo::InstructionMapping &
MMIXRegisterBankInfo::getInstrMapping(const MachineInstr &MI) const {
  const RegisterBankInfo::InstructionMapping &Mapping = getInstrMappingImpl(MI);
  if (Mapping.isValid())
    return Mapping;

  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();

  unsigned NumOperands = MI.getNumOperands();
  SmallVector<const RegisterBankInfo::ValueMapping *, 8> ValMappings(
      NumOperands);
  for (unsigned Idx = 0; Idx < NumOperands; ++Idx) {
    if (MI.getOperand(Idx).isReg()) {
      LLT Ty = MRI.getType(MI.getOperand(Idx).getReg());
      auto Size = Ty.getSizeInBits();
      ValMappings[Idx] = &getValueMapping(0, Size, MMIX::GPRBank);
    }
  }

  return getInstructionMapping(DefaultMappingID, 0,
                               getOperandsMapping(ValMappings), NumOperands);
}
