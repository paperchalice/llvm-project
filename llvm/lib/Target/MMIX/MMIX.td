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
include "MMIXCallingConvention.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//
include "MMIXInstrInfo.td"

//===----------------------------------------------------------------------===//
// MMIX processors supported.
//===----------------------------------------------------------------------===//
include "MMIXSchedDefault.td"

def : ProcessorModel<"generic", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Define the MMIX target.
//===----------------------------------------------------------------------===//

def MMIXInstrInfo : InstrInfo;

def MMIXALParser : AsmParser {
  let AsmParserClassName = "ALParser";
}

def MMIX : Target {
  let InstructionSet = MMIXInstrInfo;
  let AssemblyParsers = [DefaultAsmParser, MMIXALParser];
}

#endif
