//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// MMIX specific lowering information.
///
///
//===----------------------------------------------------------------------===//

#include "MMIXTargetLowering.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXRegisterInfo.h"

using namespace llvm;

MMIXTargetLowering::MMIXTargetLowering(const TargetMachine &TM,
                                       const TargetSubtargetInfo &STI)
    : TargetLowering(TM, STI) {
  setStackPointerRegisterToSaveRestore(MMIX::r254);
  addRegisterClass(MVT::i64, &MMIX::GPRRegClass);

  computeRegisterProperties(STI.getRegisterInfo());
}
