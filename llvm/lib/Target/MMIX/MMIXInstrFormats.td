//===-- MMIXInstrFormats.td - MMIX Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD

class MMIXInst<bits<8> opcode, dag oops, dag iops, string opstr, string argstr,
             list<dag> patterns>
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

  let Pattern = patterns;
  let Inst{31...24} = opcode;

}

class MMIXInstX_Y_Z<bits<8> opcode, dag iops, string opstr,
             list<dag> patterns>: MMIXInst<opcode,
  (outs GPR:$X), iops, opstr, "$X, $Y, $Z", patterns> {
  bits<8> X;
  bits<8> Y;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = Y;
  let Inst{7...0} = Z;
}

class InstX_YZ<bits<8> opcode, string opstr,
             list<dag> patterns>: MMIXInst<opcode,
  (outs GPR:$X), (ins i16imm:$YZ), opstr, "$X, $YZ", patterns> {
  bits<8> X;
  bits<16> YZ;

  let Inst{23...16} = X;
  let Inst{15...0} = YZ;
}

class InstX_Y_Z<bits<8> opcode, string opstr, list<dag> patterns>:
  MMIXInstX_Y_Z<opcode, (ins GPR:$Y, GPR:$Z), opstr, patterns> {}

class InstX_Y_ZI<bits<8> opcode, string opstr, list<dag> patterns>:
  MMIXInstX_Y_Z<opcode, (ins GPR:$Y, i8imm:$Z), opstr, patterns> {}

multiclass InstVariantI<bits<8> opcode, string opstr, list<dag> patterns1, list<dag> patterns2> {
  def "": InstX_Y_Z<opcode, opstr, patterns1>;
  def I: InstX_Y_ZI<!or(opcode, 1), opstr # "I", patterns2>;
}

multiclass InstVariantUI<bits<8> opcode, string opstr,
  list<dag> patterns1, list<dag> patterns2,
  list<dag> patterns3, list<dag> patterns4> {
  defm "": InstVariantI<opcode, opstr, patterns1, patterns2>;
  defm "U": InstVariantI<!add(opcode, 2), opstr, patterns3, patterns4>;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
