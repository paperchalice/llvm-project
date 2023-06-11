//===-- MMIX.td - Describe the MMIX Target Machine -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIX_TD
#define LLVM_LIB_TARGET_MMIX_MMIX_TD

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Registers, calling conventions.
//===----------------------------------------------------------------------===//
include "MMIXRegisterInfo.td"
include "GlobalISel/MMIXRegisterBanks.td"
include "MMIXSystemOperands.td"
include "MMIXCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//
include "MMIXInstrInfo.td"
include "MMIXInstrGISel.td"
include "MMIXCombine.td"
//===----------------------------------------------------------------------===//
// MMIX processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Define the MMIX target.
//===----------------------------------------------------------------------===//

def MMIXInstrInfo : InstrInfo {
  
}

#ifndef GEN_MMIXAL
def MMIX : Target {
  let InstructionSet = MMIXInstrInfo;
}
#else
def MMIXAL : Target {
  let InstructionSet = MMIXInstrInfo;
}
#endif

#endif // LLVM_LIB_TARGET_MMIX_MMIX_TD
