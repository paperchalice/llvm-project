#include "MMIXCallLowering.h"
#include "MMIXRegisterInfo.h"

#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

// MMIXOutgoingValueHandler

MMIXCallLowering::MMIXOutgoingValueHandler::MMIXOutgoingValueHandler(
    MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
    : OutgoingValueHandler(MIRBuilder, MRI) {}

Register MMIXCallLowering::MMIXOutgoingValueHandler::getStackAddress(
    uint64_t Size, int64_t Offset, MachinePointerInfo &MPO,
    ISD::ArgFlagsTy Flags) {
  auto &MF = MIRBuilder.getMF();
  unsigned PtrSize = MIRBuilder.getDataLayout().getPointerSizeInBits();
  auto AS = MIRBuilder.getDataLayout().getAllocaAddrSpace();

  LLT p0 = LLT::pointer(AS, PtrSize);
  LLT s64 = LLT::scalar(PtrSize);

  if (!SPReg)
    SPReg = MIRBuilder.buildCopy(p0, Register(MMIX::r254)).getReg(0);
  auto OffsetReg = MIRBuilder.buildConstant(s64, Offset);
  auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);
  MPO = MachinePointerInfo::getStack(MF, Offset);
  return AddrReg.getReg(0);
}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {
  Register ExtReg = extendRegister(ValVReg, VA);
  if (LastRetReg)
    if (PhysReg == MMIX::r0)
      MIRBuilder.buildCopy(LastRetReg, ExtReg);
    else if (PhysReg == LastRetReg)
      MIRBuilder.buildCopy(MMIX::r0, ExtReg);
    else
      MIRBuilder.buildCopy(PhysReg, ExtReg);
  else
    MIRBuilder.buildCopy(PhysReg, ExtReg);
}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToAddress(
    Register ValVReg, Register Addr, LLT MemTy, MachinePointerInfo &MPO,
    CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOStore, MemTy,
                                      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildStore(ValVReg, Addr, *MMO);
}

// MMIXCallSiteValueHandler

MMIXCallLowering::MMIXCallSiteArgValueHandler::MMIXCallSiteArgValueHandler(
    MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
    : MMIXOutgoingValueHandler(MIRBuilder, MRI), CurrentSubRegIndex(sub1) {}

void MMIXCallLowering::MMIXCallSiteArgValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {
  Register ExtReg = extendRegister(ValVReg, VA);
  auto RegClass = MRI.getRegClass(ArgTupleReg);
  assert(RegClass && "Invalid RegisterClass for argument tuple!");
  auto DstReg = MRI.createVirtualRegister(RegClass);

  if (RegClass == &MMIX::GPRRegClass) {
    MIRBuilder.buildCopy(DstReg, ArgTupleReg);
  } else {
    MIRBuilder.buildInstr(TargetOpcode::INSERT_SUBREG)
        .addReg(DstReg, RegState::Define)
        .addReg(ArgTupleReg)
        .addReg(ExtReg)
        .addImm(CurrentSubRegIndex++);
    ArgTupleReg = DstReg;
  }
}

// MMIXIncomingValueHandler

MMIXCallLowering::MMIXIncomingValueHandler::MMIXIncomingValueHandler(
    MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
    : IncomingValueHandler(MIRBuilder, MRI) {}

Register MMIXCallLowering::MMIXIncomingValueHandler::getStackAddress(
    uint64_t Size, int64_t Offset, MachinePointerInfo &MPO,
    ISD::ArgFlagsTy Flags) {
  auto &MFI = MIRBuilder.getMF().getFrameInfo();
  // Byval is assumed to be writable memory, but other stack passed arguments
  // are not.
  const bool IsImmutable = !Flags.isByVal();
  unsigned PtrSize = MIRBuilder.getDataLayout().getPointerSizeInBits();

  int FI = MFI.CreateFixedObject(Size, Offset, IsImmutable);
  MPO = MachinePointerInfo::getFixedStack(MIRBuilder.getMF(), FI);
  auto AddrReg =
      MIRBuilder.buildFrameIndex(LLT::pointer(MPO.AddrSpace, PtrSize), FI);
  return AddrReg.getReg(0);
}

void MMIXCallLowering::MMIXIncomingValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {
  Register ExtReg = extendRegister(ValVReg, VA);
  MIRBuilder.buildCopy(ExtReg, PhysReg);
  // mark PhysReg used
  MRI.addLiveIn(PhysReg);
  MIRBuilder.getMBB().addLiveIn(PhysReg);
}

void MMIXCallLowering::MMIXIncomingValueHandler::assignValueToAddress(
    Register ValVReg, Register Addr, LLT MemTy, MachinePointerInfo &MPO,
    CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(
      MPO, MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, MemTy,
      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
}

MMIXCallLowering::MMIXCallSiteRetValueHandler::MMIXCallSiteRetValueHandler(
    MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
    : MMIXIncomingValueHandler(MIRBuilder, MRI), CurrentSubRegIndex(sub0) {}

void MMIXCallLowering::MMIXCallSiteRetValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {
  auto *RegClass = MRI.getRegClass(RetTuple);
  assert(RegClass && "Invalid reg class for return values!");
  if (RegClass == &MMIX::GPRRegClass) {
    MIRBuilder.buildCopy(ValVReg, RetTuple);
  } else {
    MIRBuilder.buildInstr(TargetOpcode::EXTRACT_SUBREG)
        .addReg(ValVReg, RegState::Define)
        .addReg(RetTuple)
        .addImm(CurrentSubRegIndex++);
  }
}

void llvm::MMIXCallLowering::MMIXCallSiteRetValueHandler::assignValueToAddress(
    Register ValVReg, Register Addr, LLT MemTy, MachinePointerInfo &MPO,
    CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOLoad, MemTy,
                                      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
}
