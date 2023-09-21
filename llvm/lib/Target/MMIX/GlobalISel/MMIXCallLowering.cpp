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

bool MMIXCallLowering::enableBigEndian() const { return true; }

bool MMIXCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = MF.getDataLayout();
  // const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();

  SmallVector<ArgInfo, 8> OutArgs;
  for (auto &OrigArg : Info.OrigArgs) {
    splitToValueTypes(OrigArg, OutArgs, DL, Info.CallConv);
  }

  MMIXOutgoingValueHandler Handler(MIRBuilder, MRI);
  OutgoingValueAssigner Assigner(CC_MMIX_GNU);
  bool Success = determineAndHandleAssignments(
      Handler, Assigner, OutArgs, MIRBuilder, Info.CallConv, Info.IsVarArg);
  if (!Success) {
    return false;
  }

  if (Info.Callee.isReg()) {
    Info.Callee.getReg();
    MIRBuilder.buildInstr(MMIX::PUSHGOI)
        .addReg(MMIX::r15)
        .addReg(Info.Callee.getReg())
        .addImm(0);
  } else
    MIRBuilder.buildInstr(MMIX::PUSHJ).addReg(MMIX::r15).add(Info.Callee);
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
  LLVM_DEBUG(dbgs() << "lower formal arguments for function: " << F.getName()
                    << '\n');
  for (auto &Arg : F.args()) {
    LLVM_DEBUG(dbgs() << "handle formal arg:");
    LLVM_DEBUG(Arg.dump());
    const auto ArgNo = Arg.getArgNo();
    ArgInfo OrigArgInfo(VRegs[ArgNo], Arg, ArgNo);
    setArgFlags(OrigArgInfo, AttributeList::FirstArgIndex + ArgNo, DL, F);
    splitToValueTypes(OrigArgInfo, SplitArgs, DL, F.getCallingConv());
  }

  MMIXIncomingValueHandler Handler(MIRBuilder, MRI);
  IncomingValueAssigner Assigner(CC_MMIX_Callee);
  return determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder,
                                       F.getCallingConv(), F.isVarArg());
}

bool MMIXCallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val, ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI) const {
  if (Val == nullptr) {
    // nothing to lower
    assert(VRegs.empty() && "Return value but have VRegs!");
    MIRBuilder.buildInstr(MMIX::POP).addImm(0).addImm(0);
    return true;
  }

  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = MF.getDataLayout();
  const MMIXTargetLowering &TLI = *getTLI<MMIXTargetLowering>();

  SmallVector<EVT, 8> SplitEVTs;
  LLVM_DEBUG(dbgs() << "lower return for val: ");
  LLVM_DEBUG(Val->dump());
  // Compute the primitive types that underlying the Val
  ComputeValueVTs(TLI, DL, Val->getType(), SplitEVTs);
  assert(VRegs.size() == SplitEVTs.size() &&
         "For each split Type there should be exactly one VReg.");

  SmallVector<ArgInfo, 8> SplitArgs;
  for (const auto &[VReg, ValueEVT] : zip(VRegs, SplitEVTs)) {
    SplitArgs.emplace_back(VReg, ValueEVT.getTypeForEVT(Val->getContext()), 0);
    setArgFlags(SplitArgs.back(), AttributeList::ReturnIndex, DL, F);
  }

  bool Success = true;
  MMIXOutgoingValueHandler Handler(MIRBuilder, MRI);
  OutgoingValueAssigner Assigner(RetCC_MMIX_GNU);
  Success =
      determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder,
                                    F.getCallingConv(), F.isVarArg());

  auto ImplicitR0 =
      MachineOperand::CreateReg(MMIX::r0, /*isDef=*/false, /*isImp=*/true);
  MIRBuilder.buildInstr(MMIX::POP).addImm(1).addImm(0).add(ImplicitR0);
  return Success;
}

bool MMIXCallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  return true;
}

} // end namespace llvm
