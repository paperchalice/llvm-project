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
#include "MMIXRegisterInfo.h"
#include "MMIXTargetLowering.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/Support/Debug.h"

#include <algorithm>

#define DEBUG_TYPE "mmix-call-lowering"

namespace llvm {
MMIXCallLowering::MMIXCallLowering(const TargetLowering &TLI)
    : CallLowering(&TLI) {}

bool MMIXCallLowering::enableBigEndian() const { return true; }

// the calling convention of mmix is somewhat obscure
// it uses register tuples to pass arguments
bool MMIXCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DataLayout &DL = MF.getDataLayout();
  const TargetRegisterInfo *TRI = MRI.getTargetRegisterInfo();

  LLVM_DEBUG(dbgs() << "lower call for function "; Info.Callee.dump());

  MachineInstrBuilder Call;
  if (Info.Callee.isReg())
    Call =
        MIRBuilder.buildInstrNoInsert(MMIX::CALL).addReg(Info.Callee.getReg());
  else
    Call = MIRBuilder.buildInstrNoInsert(MMIX::CALL).add(Info.Callee);

  std::size_t TupleSize = 1;

  // Generate the setup call frame pseudo instruction. This will record the size
  // of the outgoing stack frame once it's known. Usually, all such pseudos can
  // be folded into the prolog/epilog of the function without emitting any
  // additional code.
  auto CallSeqStart = MIRBuilder.buildInstr(MMIX::ADJCALLSTACKDOWN);

  // handle arguments passing
  SmallVector<ArgInfo, 8> InArgs;
  for (auto &OrigArg : Info.OrigArgs) {
    LLVM_DEBUG(dbgs() << "split arg for index " << OrigArg.OrigArgIndex
                      << '\n');
    splitToValueTypes(OrigArg, InArgs, DL, Info.CallConv);
  }

  // determine arguments
  MMIXCallSiteArgValueHandler ArgHandler(MIRBuilder, MRI);
  OutgoingValueAssigner ArgAssigner(CC_MMIX_Knuth);
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(Info.CallConv, Info.IsVarArg, MF, ArgLocs,
                 MF.getFunction().getContext());
  determineAssignments(ArgAssigner, InArgs, CCInfo);
  LLVM_DEBUG(dbgs() << "ArgLocs: " << ArgLocs.size() << '\n');
  for (const auto &ArgLoc : ArgLocs) {
    if (ArgLoc.isRegLoc())
      ++TupleSize;
  }

  // determine return type
  SmallVector<ArgInfo, 8> OutArgs;
  SmallVector<CCValAssign, 16> OutValLocs;
  OutgoingValueAssigner RetAssigner(RetCC_MMIX_Knuth);
  if (!Info.OrigRet.Ty->isVoidTy()) {
    splitToValueTypes(Info.OrigRet, OutArgs, DL, Info.CallConv);
    CCState CCInfo(Info.CallConv, Info.IsVarArg, MF, OutValLocs,
                   MF.getFunction().getContext());

    std::size_t OutTupleSize = 0;
    determineAssignments(RetAssigner, OutArgs, CCInfo);
    LLVM_DEBUG(dbgs() << "OutValLocs: " << OutValLocs.size() << '\n');
    for (const auto &OutValLoc : OutValLocs) {
      if (OutValLoc.isRegLoc())
        ++OutTupleSize;
    }
    TupleSize = std::max(TupleSize, OutTupleSize);
  }

  // construct a implicit register tuple to hold arguments,
  // in MMIXRegisterInfo.td, we set the type of register tuples
  // are `untyped` so set the type here
  assert(TupleSize > 0 && "Invalid arg tuple size!");
  Register ArgReg;
  if (TupleSize > 1) {
    ArgReg = MRI.createVirtualRegister(TRI->getRegClass(TupleSize));
    MRI.setType(ArgReg,
                LLT::fixed_vector(TupleSize, LLT::scalar(TRI->getRegSizeInBits(
                                                 MMIX::GPRRegClass))));
  } else {
    ArgReg = MRI.createVirtualRegister(&MMIX::GPRRegClass);
    MRI.setType(ArgReg, LLT::scalar(TRI->getRegSizeInBits(MMIX::GPRRegClass)));
  }
  MIRBuilder.buildUndef(ArgReg);
  ArgHandler.ArgTupleReg = ArgReg;
  handleAssignments(ArgHandler, InArgs, CCInfo, ArgLocs, MIRBuilder);

  // insert pseudo call instruction
  // %ret_reg_tuple = CALL %arg_reg_tuple, @dest
  auto RetReg = MRI.cloneVirtualRegister(ArgReg);
  auto CallInstr = MIRBuilder.buildInstr(MMIX::CALL, {RetReg}, {ArgReg});
  if (Info.Callee.isReg())
    CallInstr.addReg(Info.Callee.getReg());
  else
    CallInstr.add(Info.Callee);

  // handle return value
  if (!Info.OrigRet.Ty->isVoidTy()) {
    CCState CCRetInfo(Info.CallConv, Info.IsVarArg, MF, OutValLocs,
                      MF.getFunction().getContext());
    MMIXCallSiteRetValueHandler RetHandler(MIRBuilder, MRI);
    RetHandler.RetTuple = RetReg;
    handleAssignments(RetHandler, OutArgs, CCRetInfo, OutValLocs, MIRBuilder);
  }

  // Now that the size of the argument stack region is known, the setup call
  // frame pseudo can be given its arguments.
  auto StackSize = std::max(ArgAssigner.StackSize, RetAssigner.StackSize);
  CallSeqStart.addImm(StackSize).addImm(0);

  // Generate the call frame destroy pseudo with the correct sizes.
  MIRBuilder.buildInstr(MMIX::ADJCALLSTACKUP).addImm(StackSize).addImm(0);
  return true;
}

// FIXME: currently, this method still can't handle structure formal argument,
// in theory, frontend should emit suitable LLVM IR that backend can lowering,
// thus backend can't put struct into registers when it is possible,
// fortunatley, there is no formal ABI for MMIX.
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
  IncomingValueAssigner Assigner(CC_MMIX_Knuth_Callee);
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
  auto Ret = MIRBuilder.buildInstrNoInsert(MMIX::POP);
  MMIXOutgoingValueHandler Handler(MIRBuilder, MRI);
  OutgoingValueAssigner Assigner(RetCC_MMIX_Knuth);
  SmallVector<CCValAssign, 16> ArgLocs;

  CCState CCInfo(F.getCallingConv(), F.isVarArg(), MF, ArgLocs, F.getContext());
  Success = determineAssignments(Assigner, SplitArgs, CCInfo);
  if (!Success)
    return false;
  std::size_t RegCnt = 0;
  for (const auto &ArgLoc : ArgLocs) {
    if (!ArgLoc.isRegLoc())
      break;
    else
      ++RegCnt;
  }
  if (RegCnt > 1)
    Handler.LastRetReg = MMIX::r0 + RegCnt - 1;
  Ret.addImm(RegCnt).addImm(0);
  Success =
      determineAndHandleAssignments(Handler, Assigner, SplitArgs, MIRBuilder,
                                    F.getCallingConv(), F.isVarArg());
  MIRBuilder.insertInstr(Ret);
  return Success;
}

bool MMIXCallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  return true;
}

} // end namespace llvm
