//===-- MMIXInstrFormats.td - MMIX Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD

class MMIXInst<bits<8> opcode, dag oops, dag iops, string opstr, string argstr>
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

  let OutOperandList = oops;
  let InOperandList = iops;

  let Pattern = [];
  let Inst{31...24} = opcode;

}

//===----------------------------------------------------------------------===//
// Operands
//===----------------------------------------------------------------------===//

def JumpDestAsmOperand : AsmOperandClass {
  let Name = "JumpDest";
  let ParserMethod = "tryParseJumpDestOperand";
  let RenderMethod = "addExprOperands";
}

def BranchDestAsmOperand : AsmOperandClass {
  let Name = "BranchDest";
  let ParserMethod = "tryParseBranchDestOperand";
  let RenderMethod = "addExprOperands";
}

def RoundModeAsmOperand: AsmOperandClass {
  let Name = "RoundMode";
  let RenderMethod = "addExprOperands";
  let DiagnosticType = "InvalidRoundMode";
}

def jump_dest : Operand<OtherVT> {
  let ParserMatchClass = JumpDestAsmOperand;
}

def branch_dest: Operand<i16> {
  let ParserMatchClass = BranchDestAsmOperand;
}

def round_mode: Operand<i8> {

}

def base_address: Operand<i16> {
}

class UImmAsmOperand<int width>
    : AsmOperandClass {
  let Name = "UImm" # width;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

def ui8imm : Operand<i64>, ImmLeaf<i64, [{return isUInt<8>(Imm) && Imm > 0;}]> {
  let ParserMatchClass = UImmAsmOperand<8>;
  let DecoderMethod = "decodeUImmOperand<8>";
}

def ui16imm : Operand<i64>, ImmLeaf<i64, [{return isUInt<16>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<16>;
  let DecoderMethod = "decodeUImmOperand<16>";
}

def ui24imm : Operand<i64>, ImmLeaf<i64, [{return isUInt<24>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<24>;
  let DecoderMethod = "decodeUImmOperand<24>";
}

class Inst_X_Y_Z<bits<8> opcode, dag oops, dag iops, string opstr>: MMIXInst<opcode,
  oops, iops, opstr, "$X,$Y,$Z"> {
  bits<8> X;
  bits<8> Y;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = Y;
  let Inst{7...0} = Z;
}

class Inst_X_YZ<bits<8> opcode, dag oops, dag iops, string opstr>: MMIXInst<opcode,
  oops, iops, opstr, "$X,$YZ"> {
  bits<8> X;
  bits<16> YZ;

  let Inst{23...16} = X;
  let Inst{15...0} = YZ;
}

class Inst_X__Z<bits<8> opcode, dag oops, dag iops, string opstr>: MMIXInst<opcode,
  oops, iops, opstr, "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;
}

class Inst_XYZ<bits<8> opcode, dag oops, dag iops, string opstr>:MMIXInst<opcode,
  oops, iops, opstr, "$XYZ"> {
  bits<24> XYZ;

  let Inst{23...0} = XYZ;
}

class Inst_r_rr<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs GPR:$X), (ins GPR:$Y, GPR:$Z), opstr> {
}

class Inst_rrr<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z), opstr> {
}

class Inst_r_ri8<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs GPR:$X), (ins GPR:$Y, ui8imm:$Z), opstr> {
}

class Inst_rri8<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs), (ins GPR:$X, GPR:$Y, ui8imm:$Z), opstr> {
}

class Inst_r_base<bits<8> opcode, string opstr>:
  Inst_X_YZ<opcode, (outs GPR:$X), (ins base_address:$YZ), opstr> {
  let isAsmParserOnly = true;
  let AsmMatchConverter = "resolveBaseAddress";
}

multiclass Inst_r_rri8<bits<8> opcode, string opstr> {
  def "": Inst_r_rr<opcode, opstr>;
  def I: Inst_r_ri8<!or(opcode, 1), opstr>;
}

multiclass Inst_r_rri8base<bits<8> opcode, string opstr> {
  defm "": Inst_r_rri8<opcode, opstr>;
  def I_base: Inst_r_base<!or(opcode, 1), opstr>;
}

multiclass Inst_rrri8<bits<8> opcode, string opstr> {
  def "": Inst_rrr<opcode, opstr>;
  def I: Inst_rri8<!or(opcode, 1), opstr>;
}

class Inst_r_i16<bits<8> opcode, string opstr>:
  Inst_X_YZ<opcode, (outs GPR:$X), (ins ui16imm:$YZ), opstr> {

}

class Inst_r_RoundMode_r<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs GPR:$X), (ins round_mode:$Y, GPR:$Z), opstr> {
  
}

multiclass Inst_r_RoundMode_r_Alias<bits<8> opcode, string opstr> {
  def "": Inst_r_RoundMode_r<opcode, opstr>;
  def : InstAlias<opstr # "\t$X,$Z", (!cast<Inst_r_RoundMode_r>(opstr) GPR:$X, 0, GPR:$Z)>;
}

class Inst_r_0r<bits<8> opcode, string opstr>:
  Inst_X__Z<opcode, (outs GPR:$X), (ins GPR:$Z), opstr> {

}

class InstBranch<bits<8> opcode, string opstr>:
  Inst_X_YZ<opcode, (outs), (ins GPR:$X, branch_dest:$YZ), opstr> {
  let isBranch = true;
}

multiclass InstBranchB<bits<8> opcode, string opstr> {
  def "": InstBranch<opcode, opstr>;
  def B: Inst_X_YZ<!or(opcode, 1), (outs), (ins GPR:$X, ui16imm:$YZ), opstr#"B">;
}

class Inst_r_rr_rE<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs GPR:$X), (ins GPR:$Y, GPR:$Z), opstr> {
  let Uses = [rE];
}

class Inst_i8rr<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs), (ins ui8imm:$X, GPR:$Y, GPR:$Z), opstr> {

}
class Inst_i8ri8<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs), (ins ui8imm:$X, GPR:$Y, ui8imm:$Z), opstr> {

}

multiclass Inst_i8rri8<bits<8> opcode, string opstr> {
  def "": Inst_i8rr<opcode, opstr>;
  def I: Inst_i8ri8<!or(opcode, 1), opstr>;
}

class Inst_i24imm<bits<8> opcode, string opstr>:
  Inst_XYZ<opcode, (outs), (ins ui24imm:$XYZ), opstr> {

}

class Inst_iii<bits<8> opcode, string opstr>:
  Inst_X_Y_Z<opcode, (outs), (ins ui8imm:$X, ui8imm:$Y, ui8imm:$Z), opstr> {

}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
