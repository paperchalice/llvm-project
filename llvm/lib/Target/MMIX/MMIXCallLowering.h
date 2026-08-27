//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Call lowering for GlobalISel
/// Handle calling conventions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCALLLOWERING_H
#define LLVM_LIB_TARGET_MMIX_MMIXCALLLOWERING_H

#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

class LLVM_LIBRARY_VISIBILITY MMIXCallLowering : public CallLowering {
public:
  using CallLowering::CallLowering;
  // interface
public:
  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;

  /**
   * @brief Lower outgoing return values, described by Val, into the specified
   * virtual registers VRegs.
   *
   * @param MIRBuilder
   * @param Val
   * @param VRegs
   * @param FLI
   * @param SwiftErrorVReg
   * @return true
   * @return false
   */
  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs, FunctionLoweringInfo &FLI,
                   Register SwiftErrorVReg) const override;

  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;

  bool enableBigEndian() const override { return true; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLLOWERING_H
