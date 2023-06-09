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

Register MMIXCallLowering::MMIXOutgoingValueHandler::getStackAddress(uint64_t Size, int64_t Offset, MachinePointerInfo &MPO, ISD::ArgFlagsTy Flags) {
  return Register{};
}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToReg(
    Register ValVReg, Register PhysReg, CCValAssign VA) {
  MIRBuilder.buildCopy(PhysReg, extendRegister(ValVReg, VA))
      .addUse(PhysReg, RegState::Implicit);
}

void MMIXCallLowering::MMIXOutgoingValueHandler::assignValueToAddress(Register ValVReg, Register Addr,
  LLT MemTy, MachinePointerInfo &MPO,
  CCValAssign &VA) {}

// MMIXIncomingValueHandler

MMIXCallLowering::MMIXIncomingValueHandler::MMIXIncomingValueHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
    : IncomingValueHandler(MIRBuilder, MRI) {}

Register MMIXCallLowering::MMIXIncomingValueHandler::getStackAddress(uint64_t Size, int64_t Offset,
  MachinePointerInfo &MPO, ISD::ArgFlagsTy Flags) {
  auto &MFI = MIRBuilder.getMF().getFrameInfo();
  // Byval is assumed to be writable memory, but other stack passed arguments
  // are not.
  const bool IsImmutable = !Flags.isByVal();
  unsigned PtrSize = MIRBuilder.getDataLayout().getPointerSizeInBits();
  auto AS = MIRBuilder.getDataLayout().getDefaultGlobalsAddressSpace();

  int FI = MFI.CreateFixedObject(Size, Offset, IsImmutable);
  MPO = MachinePointerInfo::getFixedStack(MIRBuilder.getMF(), FI);
  auto AddrReg = MIRBuilder.buildFrameIndex(LLT::pointer(AS, PtrSize), FI);
  return AddrReg.getReg(0);
}

void MMIXCallLowering::MMIXIncomingValueHandler::assignValueToReg(Register ValVReg, Register PhysReg,
                                  CCValAssign VA) {
  switch (VA.getLocInfo()) {
  case CCValAssign::LocInfo::Full:
    MIRBuilder.buildCopy(ValVReg, PhysReg);
    break;
  case CCValAssign::LocInfo::SExt:
  case CCValAssign::LocInfo::ZExt:
  case CCValAssign::LocInfo::AExt: {
    auto Copy = MIRBuilder.buildCopy(LLT{VA.getLocVT()}, PhysReg);
    MIRBuilder.buildTrunc(ValVReg, Copy);
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

void MMIXCallLowering::MMIXIncomingValueHandler::assignValueToAddress(Register ValVReg, Register Addr,
                                      LLT MemTy, MachinePointerInfo &MPO,
                                      CCValAssign &VA) {
  MachineFunction &MF = MIRBuilder.getMF();
  auto *MMO = MF.getMachineMemOperand(
      MPO, MachineMemOperand::MOLoad | MachineMemOperand::MOInvariant, MemTy,
      inferAlignFromPtrInfo(MF, MPO));
  MIRBuilder.buildLoad(ValVReg, Addr, *MMO);
}

} // namespace llvm
