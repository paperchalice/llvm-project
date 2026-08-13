//===-- MMIXMCTargetDesc.h - MMIX Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides MMIX specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H

// Defines symbolic names for MMIX registers. This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "MMIXGenRegisterInfo.inc"

// Defines symbolic names for LoongArch instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "MMIXGenInstrInfo.inc"

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H
