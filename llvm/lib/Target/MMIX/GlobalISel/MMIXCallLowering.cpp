//===-- MMIXCallLowering.cpp - Call lowering -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file implements the lowering of LLVM calls to machine code calls for
/// GlobalISel.
//
//===----------------------------------------------------------------------===//

#include "MMIXCallLowering.h"
#include "MMIXCallingConv.h"
#include "MMIXInstrInfo.h"
#include "MMIXTargetLowering.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "mmix-call-lowering"

namespace llvm {
MMIXCallLowering::MMIXCallLowering(const TargetLowering &TLI)
    : CallLowering(&TLI) {}

bool MMIXCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                         CallLoweringInfo &Info) const {
  
  return true;
}

bool MMIXCallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                            const Function &F,
                                            ArrayRef<ArrayRef<Register>> VRegs,
                                            FunctionLoweringInfo &FLI) const {
  MachineFunction &MF = MIRBuilder.getMF();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = MF.getDataLayout();

  SmallVector<ArgInfo, 8> SplitArgs;
  for (auto &Arg : F.args()) {
    const auto ArgNo = Arg.getArgNo();

    ArgInfo OrigArgInfo{VRegs[ArgNo], Arg, ArgNo};
    setArgFlags(OrigArgInfo, AttributeList::FirstArgIndex + ArgNo, DL, F);
    splitToValueTypes(OrigArgInfo, SplitArgs, DL, F.getCallingConv());

    // TODO: construct IncomingArgs here, using OrigArgInfo
  }

  MMIXIncomingValueHandler Handler(MIRBuilder, MRI);
  IncomingValueAssigner Assigner(CC_MMIX);
  return determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder,
                                       F.getCallingConv(), F.isVarArg());
}

bool MMIXCallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val, ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI) const {
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = MF.getDataLayout();
  const MMIXTargetLowering &TLI = *getTLI<MMIXTargetLowering>();

  SmallVector<EVT, 8> SplitEVTs;
  // Compute the primitive types that underlying the Val
  ComputeValueVTs(TLI, DL, Val->getType(), SplitEVTs);
  assert(VRegs.size() == SplitEVTs.size() &&
         "For each split Type there should be exactly one VReg.");

  SmallVector<ArgInfo, 8> SplitArgs;
  for (unsigned i = 0; i < SplitEVTs.size(); ++i) {
    // TODO: construct return value here
  }

  bool Success = true;
  MMIXOutgoingValueHandler Handler(MIRBuilder, MRI);
  OutgoingValueAssigner Assigner(RetCC_MMIX);
  Success = determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder, F.getCallingConv(), F.isVarArg());

  // TODO: insert return instruction
  return Success;
}

bool MMIXCallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  return true;
}

} // end namespace llvm
