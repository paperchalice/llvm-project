//===-- MMIXInstrInfo.h - MMIX Instruction Information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H

#include "MMIXRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_ENUM
#include "MMIXGenInstrInfo.inc"

#define GET_INSTRINFO_HEADER
#include "MMIXGenInstrInfo.inc"

namespace llvm {

class MMIXSubtarget;

class MMIXInstrInfo : public MMIXGenInstrInfo {
public:
explicit MMIXInstrInfo(MMIXSubtarget &STI);
private:
  const MMIXSubtarget &STI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_H
