//=== MMIXCallingConvention.cpp - MMIX CC entry points --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file define the entry points for MMIX calling convention analysis.
///
//===----------------------------------------------------------------------===//

#include "MMIXCallingConvention.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXRegisterInfo.h"
#include "MMIXSubtarget.h"

#include <array>

using namespace llvm;

static ArrayRef<MCPhysReg> getGPRList(size_t Len) {
  static MCPhysReg *GPRs = []() {
    static MCPhysReg RegList[256];
    for (int I = 0; I != 256; ++I)
      RegList[I] = MMIX::r0 + I;
    return RegList;
  }();
  return ArrayRef<MCPhysReg>(GPRs, Len);
}

// return true when success
template <auto Ptr>
static bool assignToRegCustom(unsigned int ValNo, MVT ValVT, MVT,
                              CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {
  const auto *TRI = static_cast<const MMIXRegisterInfo *>(
      State.getMachineFunction().getSubtarget().getRegisterInfo());

  if (MCRegister Reg = State.AllocateReg(getGPRList((TRI->*Ptr)()))) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, MVT::i64, LocInfo));
    return true;
  }
  return false;
}

// TableGen provides definitions of the calling convention analysis entry
// points.
#define GET_CALLING_CONV_IMPL
#include "MMIXGenCallingConv.inc"
