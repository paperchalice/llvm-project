//===-- MMIXCallLowering.h - Call lowering ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file describes how to lower LLVM calls to machine code calls.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCALLLOWERING_H
#define LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCALLLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/ValueTypes.h"

namespace llvm {

class MMIXCallLowering : public CallLowering {
public:
  MMIXCallLowering(const TargetLowering &TLI);

public: // CallLowering
  bool enableBigEndian() const override;
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;
  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;
  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs,
                   FunctionLoweringInfo &FLI) const override;

public:
  struct MMIXOutgoingValueHandler : public OutgoingValueHandler {
    MMIXOutgoingValueHandler(MachineIRBuilder &MIRBuilder,
                             MachineRegisterInfo &MRI);

  public:
    Register getStackAddress(uint64_t Size, int64_t Offset,
                             MachinePointerInfo &MPO,
                             ISD::ArgFlagsTy Flags) override;
    void assignValueToReg(Register ValVReg, Register PhysReg,
                          CCValAssign VA) override;
    void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                              MachinePointerInfo &MPO,
                              CCValAssign &VA) override;
  };

  struct MMIXIncomingValueHandler : public IncomingValueHandler {
    MMIXIncomingValueHandler(MachineIRBuilder &MIRBuilder,
                             MachineRegisterInfo &MRI);

  public:
    Register getStackAddress(uint64_t Size, int64_t Offset,
                             MachinePointerInfo &MPO,
                             ISD::ArgFlagsTy Flags) override;
    void assignValueToReg(Register ValVReg, Register PhysReg,
                          CCValAssign VA) override;
    void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                              MachinePointerInfo &MPO,
                              CCValAssign &VA) override;
  };
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCALLLOWERING_H
