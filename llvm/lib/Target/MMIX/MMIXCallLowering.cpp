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

#include "MMIXCallLowering.h"

using namespace llvm;

bool MMIXCallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  return false;
}

bool MMIXCallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val, ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI,
                                   Register SwiftErrorVReg) const {
  return false;
}

bool MMIXCallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                            const Function &F,
                                            ArrayRef<ArrayRef<Register>> VRegs,
                                            FunctionLoweringInfo &FLI) const {
  return false;
}

bool MMIXCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  return false;
}
