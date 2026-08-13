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

class Inst_X_Y_Z<bits<8> opcode, dag oops, dag iops, string opstr=NAME>
    : MMIXInst<opcode, oops, iops, opstr, "$X,$Y,$Z"> {
  bits<8> X;
  bits<8> Y;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = Y;
  let Inst{7...0} = Z;
}

class Inst_X_YZ<bits<8> opcode, dag oops, dag iops, string opstr=NAME>
    : MMIXInst<opcode, oops, iops, opstr, "$X,$YZ"> {
  bits<8> X;
  bits<16> YZ;

  let Inst{23...16} = X;
  let Inst{15...0} = YZ;
}

class Inst_XYZ<bits<8> opcode, dag oops, dag iops, string opstr=NAME>
    : MMIXInst<opcode, oops, iops, opstr, "$XYZ"> {
  bits<24> XYZ;

  let Inst{23...0} = XYZ;
}

// MMIX specific operands types.

foreach j = [17, 24, 25] in {
  def i#j : VTInt<j>;
}

class UImmAsmOperand<int N = -1> : AsmOperandClass {
  let Name = "UImm"# !if(!lt(N, 0), "", !cast<string>(N));
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isUImm<"#N#">";
}

class SImmAsmOperand<int N = -1> : AsmOperandClass {
  let Name = "SImm"# !if(!lt(N, 0), "", !cast<string>(N));
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isSImm<"#N#">";
}

let OperandType = "OPERAND_IMMEDIATE" in
{
def i17imm : Operand<i32>;
def i24imm : Operand<i32>;
def i25imm : Operand<i32>;

foreach j = !range(1,4) in {
  defvar N = !mul(8, j);
  let ParserMatchClass = UImmAsmOperand<N> in
  def u#N#imm : Operand<!cast<VTInt>("i"#N)>;
}
let ParserMatchClass = SImmAsmOperand<17> in
def s17imm : Operand<i32>;
let ParserMatchClass = SImmAsmOperand<25> in
def s25imm : Operand<i32>;
}

def RoundingModeOperand : AsmOperandClass {
  let Name = "RoundingMode";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidRoundingMode";
}
def rounding_mode: Operand<i8> {
  let ParserMatchClass = RoundingModeOperand;
  let PrintMethod = "printRoundingModeOperand";
}

def BranchDestOperand : AsmOperandClass {
  let Name = "BranchDest";
  let ParserMethod = "parseBranchDest";
}
def b_dst : Operand<i32> {
  let ParserMatchClass = BranchDestOperand;
  let PrintMethod = "printBranchDestOperand";
}
def jmp_dst : Operand<i32> {
  let ParserMatchClass = BranchDestOperand;
  let PrintMethod = "printJmpDestOperand";
}

def TrapYOperand : AsmOperandClass {
  let Name = "TrapY";
  let PredicateMethod = "isUImm";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidTrapY";
}
def trap_y : Operand<i8> {
  let ParserMatchClass = TrapYOperand;
  let PrintMethod = "printTrapYOperand";
}

def trap_z : Operand<i8> {
  let ParserMatchClass = UImmAsmOperand<8>;
  let PrintMethod = "printTrapZOperand";
}

def spr_imm : Operand<i8> {
  let ParserMatchClass = UImmAsmOperand<5>;
  let PrintMethod = "printSPRImmOperand";
}

// special inst class

// Here we use BN as example.
// The canonical syntax of BN[B] is
// BN $X,label
// mmixal will adjust BN to BNB if it
// is a backward branch, i.e. the YZ is a signed operand,
// so we can think BN is an instruction with only 7 bits opcode
// and YZ is a 25 bits operand, the HSB is encoded into the Inst{24}.
class InstB<bits<8> opcode, string opstr = NAME> :
  MMIXInst<opcode, (outs), (ins GPR:$X, b_dst:$YZ), opstr, "$X,$YZ"> {
  bits<8> X;
  bits<17> YZ;

  let Inst{31...25} = !srl(opcode, 1);
  let Inst{24} = YZ{16};
  let Inst{23...16} = X;
  let Inst{15...0} = YZ{15...0};

  let isBranch = true;
}

class InstJMP<string opstr = NAME> :
  MMIXInst<0xF0, (outs), (ins jmp_dst:$XYZ), opstr, "$XYZ"> {
  bits<25> XYZ;

  let Inst{31...25} = !srl(0xF0, 1);
  let Inst{24} = XYZ{24};
  let Inst{23...0} = XYZ{23...0};

  let isBranch = true;
}

// helper records

defvar SizeSuffixList = ["B", "W", "T", "O"];

defvar PosCondList = ["N", "Z", "P", "OD"];
defvar NegCondList = !foreach(c, PosCondList, !if(!ne(c, "OD"), "N"#c, "EV"));
defvar CondList = !listconcat(PosCondList, NegCondList);

defvar RegPosList = ["H", "MH", "ML", "L"];

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
