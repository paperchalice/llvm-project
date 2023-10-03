#include "MMIXCallLowering.h"
#include "MMIXRegisterInfo.h"

#include "llvm/CodeGen/Analysis.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/Support/Debug.h"

namespace llvm {

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

  auto SPReg = MIRBuilder.buildCopy(p0, Register(MMIX::r254)).getReg(0);
  auto OffsetReg = MIRBuilder.buildConstant(s64, Offset);
  auto AddrReg = MIRBuilder.buildPtrAdd(p0, SPReg, OffsetReg);
  MPO = MachinePointerInfo::getStack(MF, Offset);
  return AddrReg.getReg(0);
}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToAddress(
    Register ValVReg, Register Addr, LLT MemTy, MachinePointerInfo &MPO,
    CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(MPO, MachineMemOperand::MOStore, MemTy,
                                      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildStore(ValVReg, Addr, *MMO);
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
  switch (VA.getLocInfo()) {
  case CCValAssign::LocInfo::Full:
    MIRBuilder.buildCopy(ValVReg, PhysReg);
    break;
  case CCValAssign::LocInfo::SExt:
  case CCValAssign::LocInfo::ZExt:
  case CCValAssign::LocInfo::AExt: {
    // auto Ext = MIRBuilder.buildAnyExt(LLT{VA.getValVT()}, ValVReg);
    MIRBuilder.buildCopy(PhysReg, ValVReg);
  } break;
  case CCValAssign::LocInfo::SExtUpper:
  case CCValAssign::LocInfo::ZExtUpper:
  case CCValAssign::LocInfo::AExtUpper:
  case CCValAssign::LocInfo::BCvt:
  case CCValAssign::LocInfo::Trunc:
  case CCValAssign::LocInfo::VExt:
  case CCValAssign::LocInfo::FPExt:
  case CCValAssign::LocInfo::Indirect:
    break;
  }
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

} // namespace llvm
