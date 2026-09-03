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
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXCallingConvention.h"
#include "MMIXSubtarget.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"

using namespace llvm;

namespace {

static Register createVirtualGPR(MachineIRBuilder &MIRBuilder) {
  return MIRBuilder.getMRI()->createVirtualRegister(
      &getMMIXMCRegisterClass(MMIX::GPRRegClassID));
}

static const MMIXRegisterInfo &getTRI(MachineIRBuilder &MIRBuilder) {
  return *static_cast<const MMIXRegisterInfo *>(
      MIRBuilder.getMF().getSubtarget().getRegisterInfo());
}

struct MMIXOutgoingValueHandler : public CallLowering::OutgoingValueHandler {
  using CallLowering::OutgoingValueHandler::OutgoingValueHandler;

  Register getStackAddress(uint64_t MemSize, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override {
    LLT p0 = LLT::pointer(0, 64);
    LLT SType = LLT::scalar(64);
    Register StackReg = MMIX::r254;
    auto SPReg = MIRBuilder.buildCopy(p0, StackReg).getReg(0);
    auto OffsetReg = MIRBuilder.buildConstant(SType, Offset);
    auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);
    MPO = MachinePointerInfo::getStack(MIRBuilder.getMF(), Offset);
    return AddrReg.getReg(0);
  }

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA, ISD::ArgFlagsTy Flags) override {
    Register ExtReg = extendRegister(ValVReg, VA);
    MachineRegisterInfo &MRI = *MIRBuilder.getMRI();
    MRI.setRegClass(ExtReg, &getMMIXMCRegisterClass(MMIX::GPRRegClassID));
    MIRBuilder.buildCopy(PhysReg, ExtReg);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();
    Register ExtReg = extendRegister(ValVReg, VA);

    auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOStore, MemTy,
                                        inferAlignFromPtrInfo(MF, MPO));
    MIRBuilder.buildStore(ExtReg, Addr, *MMO);
  }
};

struct MMIXArgHandler : MMIXOutgoingValueHandler {
  using MMIXOutgoingValueHandler::MMIXOutgoingValueHandler;

  SmallVector<Register, 16> ArgVRegs;

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA, ISD::ArgFlagsTy Flags) override {
    Register DstReg = createVirtualGPR(MIRBuilder);
    ArgVRegs.push_back(DstReg);
    MMIXOutgoingValueHandler::assignValueToReg(ValVReg, DstReg, VA, Flags);
  }
};

struct MMIXIncomingValueHandler : public CallLowering::IncomingValueHandler {
  using CallLowering::IncomingValueHandler::IncomingValueHandler;

  Register getStackAddress(uint64_t MemSize, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override {
    LLT p0 = LLT::pointer(0, 64);
    LLT SType = LLT::scalar(64);
    Register StackReg = MMIX::r254;
    MIRBuilder.getMRI()->addLiveIn(StackReg);
    MIRBuilder.getMBB().addLiveIn(StackReg);
    auto SPReg = MIRBuilder.buildCopy(p0, StackReg).getReg(0);
    auto OffsetReg = MIRBuilder.buildConstant(SType, Offset);
    auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);
    MPO = MachinePointerInfo::getStack(MIRBuilder.getMF(), Offset);
    return AddrReg.getReg(0);
  }

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA, ISD::ArgFlagsTy Flags) override {
    MIRBuilder.getMRI()->addLiveIn(PhysReg);
    MIRBuilder.getMBB().addLiveIn(PhysReg);
    IncomingValueHandler::assignValueToReg(ValVReg, PhysReg, VA);
  }

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override {
    MachineFunction &MF = MIRBuilder.getMF();
    auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOLoad, MemTy,
                                        inferAlignFromPtrInfo(MF, MPO));
    MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
  }
};

struct MMIXReturnValueHandler : MMIXIncomingValueHandler {
  using MMIXIncomingValueHandler::MMIXIncomingValueHandler;

  SmallVector<Register, 2> RetVRegs;

  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA, ISD::ArgFlagsTy Flags) override {
    assert(!RetVRegs.empty() && "expect vregs to store return values");
    IncomingValueHandler::assignValueToReg(ValVReg, RetVRegs.front(), VA);
    RetVRegs.erase(RetVRegs.begin());
  }
};

} // namespace

bool MMIXCallLowering::canLowerReturn(MachineFunction &MF,
                                      CallingConv::ID CallConv,
                                      SmallVectorImpl<BaseArgInfo> &Outs,
                                      bool IsVarArg) const {
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs,
                 MF.getFunction().getContext());

  return checkReturn(CCInfo, Outs, RetCC_MMIX);
}

bool MMIXCallLowering::lowerReturn(MachineIRBuilder &MIRBuilder,
                                   const Value *Val, ArrayRef<Register> VRegs,
                                   FunctionLoweringInfo &FLI,
                                   Register SwiftErrorVReg) const {
  assert(((Val && !VRegs.empty()) || (!Val && VRegs.empty())) &&
         "Return value without a vreg");
  if (!FLI.CanLowerReturn)
    return false;
  MachineFunction &MF = MIRBuilder.getMF();
  const Function &F = MF.getFunction();
  const DataLayout &DL = F.getDataLayout();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto MIB = MIRBuilder.buildInstrNoInsert(MMIX::POP);
  int64_t X = 0; // the X field of POP
  bool Success = false;
  if (!VRegs.empty()) {
    SmallVector<ArgInfo, 8> SplitArgs;
    ArgInfo OrigArg{VRegs, Val->getType(), 0};
    setArgFlags(OrigArg, AttributeList::ReturnIndex, DL, F);
    splitToValueTypes(OrigArg, SplitArgs, DL, F.getCallingConv());
    OutgoingValueAssigner ValAssigner(RetCC_MMIX);
    MMIXOutgoingValueHandler ValHandler(MIRBuilder, MRI);

    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(F.getCallingConv(), F.isVarArg(), MF, ArgLocs,
                   F.getContext());
    if (!determineAssignments(ValAssigner, SplitArgs, CCInfo))
      return false;
    // The main return value should be the last register, swap them.
    if (ArgLocs.size() > 1 && ArgLocs[0].isRegLoc()) {
      CCValAssign *LastReg = &ArgLocs[0];
      // find last part returned by register
      for (auto &ArgLoc : ArgLocs) {
        if (ArgLoc.isRegLoc())
          LastReg = &ArgLoc;
      }
      std::swap(ArgLocs[0], *LastReg);
    }
    Success =
        handleAssignments(ValHandler, SplitArgs, CCInfo, ArgLocs, MIRBuilder);
    X = std::min<int64_t>(2, ArgLocs.size());
  } else {
    Success = true;
  }
  MIB.addImm(X).addImm(0);
  // the 1st operand of canonical POP instruction is immediate,
  // which indicate the number of registers to pop, which is implicit use
  for (int I = 0; I != X; ++I)
    MIB.addUse(MMIX::r0 + I, RegState::Implicit);
  if (Success)
    MIRBuilder.insertInstr(MIB);
  return Success;
}

bool MMIXCallLowering::lowerFormalArguments(MachineIRBuilder &MIRBuilder,
                                            const Function &F,
                                            ArrayRef<ArrayRef<Register>> VRegs,
                                            FunctionLoweringInfo &FLI) const {
  if (VRegs.empty())
    return true;

  MachineFunction &MF = MIRBuilder.getMF();
  const DataLayout &DL = MF.getDataLayout();

  OutgoingValueAssigner ValAssigner(CC_MMIX_Callee);
  MMIXIncomingValueHandler ValHandler(MIRBuilder, MF.getRegInfo());

  SmallVector<ArgInfo, 8> SplitArgInfos;
  for (auto &&[Idx, Arg] : llvm::enumerate(F.args())) {
    ArgInfo OrigArgInfo(VRegs[Idx], Arg.getType(), Idx);

    setArgFlags(OrigArgInfo, Idx + AttributeList::FirstArgIndex, DL, F);
    splitToValueTypes(OrigArgInfo, SplitArgInfos, DL, F.getCallingConv());
  }

  if (!determineAndHandleAssignments(ValHandler, ValAssigner, SplitArgInfos,
                                     MIRBuilder, F.getCallingConv(),
                                     F.isVarArg()))
    return false;

  return true;
}

bool MMIXCallLowering::lowerCall(MachineIRBuilder &MIRBuilder,
                                 CallLoweringInfo &Info) const {
  MachineFunction &MF = MIRBuilder.getMF();
  Function &F = MF.getFunction();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const auto &STI = MF.getSubtarget<MMIXSubtarget>();
  auto &DL = F.getDataLayout();
  const TargetInstrInfo &TII = *STI.getInstrInfo();

  SmallVector<ArgInfo, 8> OutArgs;
  for (auto &OrigArg : Info.OrigArgs)
    splitToValueTypes(OrigArg, OutArgs, DL, Info.CallConv);

  unsigned AdjStackDown = TII.getCallFrameSetupOpcode();
  auto CallSeqStart = MIRBuilder.buildInstr(AdjStackDown);

  // handle arguments
  OutgoingValueAssigner ArgAssigner(CC_MMIX_Caller);
  MMIXArgHandler ArgHandler(MIRBuilder, MRI);
  if (!determineAndHandleAssignments(ArgHandler, ArgAssigner, OutArgs,
                                     MIRBuilder, Info.CallConv, Info.IsVarArg))
    return false;

  MachineInstrBuilder MIB = MIRBuilder.buildInstrNoInsert(MMIX::MP_CALL);
  MIRBuilder.insertInstr(MIB);

  SmallVector<Register, 2> RetDefVRegs;

  // handle return value
  if (!Info.OrigRet.Ty->isVoidTy()) {
    SmallVector<ArgInfo, 8> RetArgs;
    splitToValueTypes(Info.OrigRet, RetArgs, DL, Info.CallConv);

    OutgoingValueAssigner Assigner(RetCC_MMIX);
    MMIXReturnValueHandler Handler(MIRBuilder, MRI);
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(F.getCallingConv(), F.isVarArg(), MF, ArgLocs,
                   F.getContext());
    if (!determineAssignments(Assigner, RetArgs, CCInfo))
      return false;

    unsigned InRegRetCnt = llvm::count_if(
        ArgLocs, [](const CCValAssign &ArgLoc) { return ArgLoc.isRegLoc(); });
    auto RetRegCnt = std::min<unsigned>(getTRI(MIRBuilder).getReturnThreshold(),
                                        InRegRetCnt);
    for (unsigned I = 0; I != RetRegCnt; ++I) {
      Register RetVReg = createVirtualGPR(MIRBuilder);
      RetDefVRegs.push_back(RetVReg);
    }
    Handler.RetVRegs = RetDefVRegs;

    if (!handleAssignments(Handler, RetArgs, CCInfo, ArgLocs, MIRBuilder))
      return false;
  } else {
    // prepare a hole register when pop 0 register
    Register HoleReg = createVirtualGPR(MIRBuilder);
    RetDefVRegs.push_back(HoleReg);
  }

  // construct call
  for (auto &Reg : RetDefVRegs)
    MIB.addDef(Reg);
  MIB.add(Info.Callee);
  for (auto &Reg : ArgHandler.ArgVRegs)
    MIB.addUse(Reg);

  // tie arg and output operand
  {
    size_t DefSize = RetDefVRegs.size();
    size_t E = std::min(DefSize, ArgHandler.ArgVRegs.size());
    for (size_t I = 0; I != E; ++I)
      MIB->tieOperands(I, 1 + DefSize + I);
  }

  CallSeqStart.addImm(ArgAssigner.StackSize).addImm(0);
  unsigned AdjStackUp = TII.getCallFrameDestroyOpcode();
  MIRBuilder.buildInstr(AdjStackUp).addImm(ArgAssigner.StackSize).addImm(0);

  return true;
}
