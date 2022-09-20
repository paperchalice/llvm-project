//===-- MMIXRegisterBankInfo.h ---------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKINFO_H

#include "llvm/CodeGen/GlobalISel/RegisterBankInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "MMIXGenRegisterBank.inc"

namespace llvm {

class TargetRegisterInfo;

class MMIXGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "MMIXGenRegisterBank.inc"
};

/// This class provides the information for the target register banks.
class MMIXRegisterBankInfo final : public MMIXGenRegisterBankInfo {
public:
  MMIXRegisterBankInfo(const TargetRegisterInfo &TRI);

public:
  const InstructionMapping &
  getInstrMapping(const MachineInstr &MI) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKINFO_H
