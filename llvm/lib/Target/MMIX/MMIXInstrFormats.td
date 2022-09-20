//===-- MMIXInstrFormats.td - MMIX Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD

class MMIXInst<bits<32> opcode, dag oops, dag iops, string opstr, string argstr,
             list<dag> pattern>
    : Instruction {
  field bits<32> Inst; // Instruction encoding.
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;
  let Namespace = "MMIX";
  let Size = 4;
  let AsmString = opstr # "\t" # argstr;

  dag OutOperandList = oops;
  dag InOperandList = iops;

  let Pattern = pattern;
  let Inst = opcode;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
