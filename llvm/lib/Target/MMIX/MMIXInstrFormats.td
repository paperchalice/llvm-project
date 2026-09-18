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
  def u#N#imm : Operand<i64>, ImmLeaf<i64, "return isUInt<"#N#">(Imm);">;
}
foreach j = [v8i8, v4i16, v2i32] in {
def u8imm_#j : Operand<j>, ImmLeaf<j, "return isUInt<8>(Imm);">;
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

def RelAddrOperand : AsmOperandClass {
  let Name = "Mem";
  let ParserMethod = "parseRelAddr";
}
def rel_addr : Operand<OtherVT> {
  let ParserMatchClass = RelAddrOperand;
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
class InstB_X_YZ<bits<8> opcode, dag oops, dag iops, string opstr=NAME> :
  MMIXInst<opcode, oops, iops, opstr, "$X,$YZ"> {
  bits<8> X;
  bits<17> YZ;

  let Inst{31...25} = !srl(opcode, 1);
  let Inst{24} = YZ{16};
  let Inst{23...16} = X;
  let Inst{15...0} = YZ{15...0};
}

class InstB_XYZ<bits<8> opcode, dag oops, dag iops, string opstr=NAME> :
  MMIXInst<opcode, oops, iops, opstr, "$XYZ"> {
  bits<25> XYZ;

  let Inst{31...25} = !srl(opcode, 1);
  let Inst{24} = XYZ{24};
  let Inst{23...0} = XYZ{23...0};
}

class InstB<bits<8> opcode, string opstr = NAME> :
  InstB_X_YZ<opcode, (outs), (ins GPR:$X, rel_addr:$YZ), opstr> {
  bits<8> X;
  bits<17> YZ;

  let Inst{31...25} = !srl(opcode, 1);
  let Inst{24} = YZ{16};
  let Inst{23...16} = X;
  let Inst{15...0} = YZ{15...0};

  let isBranch = true;
  let isTerminator = true;
}

class InstJMP<string opstr = NAME> :
  InstB_XYZ<0xF0, (outs), (ins rel_addr:$XYZ), opstr> {
  let isBranch = true;
  let isTerminator = true;
}

// helper records

defvar SizeSuffixList = ["B", "W", "T", "O"];
defvar SizeVTIntList = [i8, i16, i32, i64];

defvar PosCondList = ["N", "Z", "P", "OD"];
defvar NegCondList = !foreach(c, PosCondList, !if(!ne(c, "OD"), "N"#c, "EV"));
defvar PosCondDagList = [
  (setlt i64:$Y, 0), (seteq i64:$Y, 0), (setgt i64:$Y, 0),
  (seteq (and i64:$Y, 1), 1),
  (setge i64:$Y, 0), (setne i64:$Y, 0), (setle i64:$Y, 0),
  (seteq (and i64:$Y, 1), 0)
];
defvar I64PosCondDagList = !foreach(dg, PosCondDagList, (i64 dg));
defvar CondList = !listconcat(PosCondList, NegCondList);

defvar RegPosList = ["H", "MH", "ML", "L"];

// inst prototypes

multiclass BinOp<bits<8> opcode, list<dag> patops=[]> {
  def "" : Inst_X_Y_Z<opcode, (outs GPR:$X), (ins GPR:$Y, GPR:$Z), NAME> {
    let Pattern = !foreach(d, patops, (set i64:$X, (!getdagop(d) i64:$Y, i64:$Z)));
  }
  def I : Inst_X_Y_Z<!or(opcode, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z), NAME#"I"> {
    let Pattern = !foreach(d, patops, (set i64:$X, (!getdagop(d) i64:$Y, u8imm:$Z)));
  }
}

// patfrags

class op_nxw<dag op> : PatFrag<(ops node:$src0, node:$src1), (!getdagop(op) node:$src0, node:$src1)> {
  let GISelPredicateCode = [{return MI.getFlag(MachineInstr::NoUWrap) || MI.getFlag(MachineInstr::NoSWrap);}];
}

class OpNPat<dag op> : PatFrag<(ops node:$src0, node:$src1), (!getdagop(op) node:$src0, (not node:$src1))>;
class NOpPat<dag op> : PatFrag<(ops node:$src0, node:$src1), (not (!getdagop(op) node:$src0, node:$src1))>;
foreach i=[or, and, xor] in {
  def i#"n" : OpNPat<(i)>;
  def "n"#i : NOpPat<(i)>;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
