//===-- MMIXInstrFormats.td - MMIX Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD

include "MMIXInstrFormats.td"

class MMIXInst<bits<8> opcode, dag oops, dag iops, string opstr, string argstr>
    : Instruction {
  bits<32> Inst; // Instruction encoding.
  let Namespace = "MMIX";
  let Size = 4;
  let AsmString = opstr #"\t" #argstr;

  let OutOperandList = oops;
  let InOperandList = iops;

  let Pattern = [];
  let Inst{31...24} = opcode;

  let HasPositionOrder = true;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
