//===-- MMIXInstrInfo.td - Target Description for MMIX ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the MMIX instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "MMIXInstrFormats.td"

// extend
def sextloadi64 : PatFrag<(ops node:$ptr), (sextload node:$ptr)> {
  let IsLoad = true;
  let MemoryVT = i64;
}

def zextloadi64 : PatFrag<(ops node:$ptr), (zextload node:$ptr)> {
  let IsLoad = true;
  let MemoryVT = i64;
}

// a fake truncstore, it is just store i64
def truncstorei64 : PatFrag<(ops node:$val, node:$ptr),
                            (store node:$val, node:$ptr)> {
  let IsStore = true;
  let MemoryVT = i64;
  let IsTruncStore = false;
}

defvar SuffixString = "BWTO";
class SuffixList<int i> {
  string ret = !substr(SuffixString, i, 1);
}
//
// Loading and storing
//

let mayLoad = true, mayStore = false in
multiclass LoadInst<int idx> {
  defm ""#SuffixList<idx>.ret: Inst_r_rri8base<!add(0x80, !mul(idx, 4)), "LD"#SuffixList<idx>.ret>;
  defm ""#SuffixList<idx>.ret#"U": Inst_r_rri8base<!add(0x82, !mul(idx, 4)), "LD"#SuffixList<idx>.ret#"U">;
  // def ""#SuffixList<idx>.ret#"_base": Inst_r_base<!add(0x81, !mul(idx, 4)), "LD"#SuffixList<idx>.ret>;
  // def ""#SuffixList<idx>.ret#"U_base": Inst_r_base<!add(0x83, !mul(idx, 4)), "LD"#SuffixList<idx>.ret>;
}

let mayLoad = false, mayStore = true in
multiclass StoreInst<int idx> {
  defm ""#SuffixList<idx>.ret: Inst_r_rri8<!add(0xA0, !mul(idx, 4)), "ST"#SuffixList<idx>.ret>;
  defm ""#SuffixList<idx>.ret#"U": Inst_r_rri8<!add(0xA2, !mul(idx, 4)), "ST"#SuffixList<idx>.ret#"U">;
}

foreach s = {0...3} in {
  defm LD: LoadInst<s>;
  defm ST: StoreInst<s>;
}
let mayLoad = true, mayStore = false in
defm LDHT: Inst_rrri8<0x92, "LDHT">;

let mayLoad = false, mayStore = true in
def STCO: Inst_X_Y_Z<0xB4, (outs), (ins ui8imm:$X, GPR:$Y, GPR:$Z), "STCO">;
def STCOI: Inst_X_Y_Z<0xB5, (outs), (ins ui8imm:$X, GPR:$Y, ui8imm:$Z), "STCO">;
defm STHT: Inst_rrri8<0xB2, "STHT">;

//
// Adding and subtracting
//

multiclass InstVariant_U<bits<8> opcode, string opstr> {
  defm "": Inst_r_rri8<opcode, opstr>;
  defm U: Inst_r_rri8<!add(opcode, 2), opstr#"U">;
}

let isAdd = true in {
defm ADD: InstVariant_U<0x20, "ADD">;
foreach i = [2, 4, 8, 16] in {
  defm ADD#i#"U": Inst_r_rri8<!add(0x28, !mul(!sub(!logtwo(i), 1), 2)), i#"ADDU">;
}
}
def LDA: MnemonicAlias<"LDA", "ADDU">;
defm SUB: InstVariant_U<0x24, "SUB">;
defm NEG: InstVariant_U<0x34, "NEG">;

//
// Bit fiddling
//
defm AND: Inst_r_rri8<0xC8, "AND">;
defm OR: Inst_r_rri8<0xC0, "OR">;
defm XOR: Inst_r_rri8<0xC6, "XOR">;
defm ANDN: Inst_r_rri8<0xCA, "ANDN">;
defm ORN: Inst_r_rri8<0xC2, "ORN">;
defm NAND: Inst_r_rri8<0xCC, "NAND">;
defm NOR: Inst_r_rri8<0xC4, "NOR">;
defm NXOR: Inst_r_rri8<0xCE, "NXOR">;
defm MUX: Inst_r_rri8<0xD8, "MUX">;

//
// unsigned bytewise and biggerwise operations
//

foreach i = {0...3} in {
defm SuffixList<i>.ret#DIFF: Inst_r_rri8<!add(0xD0, !mul(i, 2)), SuffixList<i>.ret#"DIFF">;
}

//
// Three more pairs of bit-fiddling instructions round out the collection of exotics
//

defm SADD: Inst_r_rri8<0xDA, "SADD">;
defm MOR: Inst_r_rri8<0xDC, "MOR">;
defm MXOR: Inst_r_rri8<0xDE, "MXOR">;

//
// Sixteen "immediate wyde" instructions
//
class PosList<int i> {
  string ret = !cond(!eq(i,  0): "H",
                      !eq(i, 1): "MH",
                      !eq(i, 2): "ML",
                      !eq(i, 3): "L",
                      true: "");
}
foreach i = {0...3} in {
def SET#PosList<i>.ret: Inst_r_i16<!add(0xE0, i), "SET"#PosList<i>.ret> {
  let InOperandList = (ins GPR:$X0, ui16imm:$YZ);
  let Constraints = "$X = $X0";
}
def INC#PosList<i>.ret: Inst_r_i16<!add(0xE4, i), "INC"#PosList<i>.ret>;
def OR#PosList<i>.ret: Inst_r_i16<!add(0xE8, i), "OR"#PosList<i>.ret>;
def ANDN#PosList<i>.ret: Inst_r_i16<!add(0xEC, i), "ANDN"#PosList<i>.ret>;
}

def SET: InstAlias<"SET $X,$Y", (ORI GPR:$X, GPR:$Y, 0)>;

//
// MMIX knows several ways to shift a register left or right by any number of bits
//

defm SL: InstVariant_U<0x38, "SL">;
defm SR: InstVariant_U<0x3C, "SR">;

//
// Comparisons
//

let isCompare = true in
defm CMP: InstVariant_U<0x30, "CMP">;

//
// conditional
//

class CondList<int i>{
  string ret = !cond(
    !eq(i, 0): "N",
    !eq(i, 1): "Z",
    !eq(i, 2): "P",
    !eq(i, 3): "OD",
    !eq(i, 4): "NN",
    !eq(i, 5): "NZ",
    !eq(i, 6): "NP",
    !eq(i, 7): "EV",
    true: ""
  );
}

foreach i = {0...7} in {
defm CS#CondList<i>.ret: Inst_r_rri8<!add(0x60, !mul(i, 2)), "CS"#CondList<i>.ret>;
defm ZS#CondList<i>.ret: Inst_r_rri8<!add(0x70, !mul(i, 2)), "ZS"#CondList<i>.ret>;
}

//
// Branches and jumps
//

// The backward branching is handled by fixup
foreach i = {0...7} in {
defm B#CondList<i>.ret: InstBranchB<!add(0x40, !mul(i, 2)), "B"#CondList<i>.ret>;
defm PB#CondList<i>.ret: InstBranchB<!add(0x50, !mul(i, 2)), "PB"#CondList<i>.ret>;
}

def GETA: Inst_X_YZ<0xF4, (outs GPR:$X), (ins branch_dest:$YZ), "GETA">;
def GETAB: Inst_X_YZ<0xF5, (outs GPR:$X), (ins ui16imm:$YZ), "GETAB">;

def JMP: Inst_XYZ<0xF0, (outs), (ins jump_dest:$XYZ), "JMP">;
def JMPB: Inst_XYZ<0xF1, (outs), (ins ui16imm:$XYZ), "JMPB">;

defm GO: Inst_r_rri8<0x9E, "GO">;


//
// Multiplication and division
//

defm MUL: InstVariant_U<0x18, "MUL">;
defm DIV: InstVariant_U<0x1C, "DIV">;

//
// Floating point computations
//

def FADD: Inst_r_rr<0x04, "FADD">;
def FSUB: Inst_r_rr<0x06, "FSUB">;
def FMUL: Inst_r_rr<0x10, "FMUL">;
def FDIV: Inst_r_rr<0x14, "FDIV">;
def FREM: Inst_r_rr<0x16, "FREM">;

defm FSQRT: Inst_r_RoundMode_r_Alias<0x15, "FSQRT">;
defm FINT: Inst_r_RoundMode_r_Alias<0x17, "FINT">;

multiclass InstFloatingCompare<bits<8> opcode, string opstr>{
  def "":Inst_r_rr<opcode, opstr>;
  def E: Inst_r_rr_rE<!add(opcode, 0x10), opstr#"E">;
}

let isCompare = true in {
defm FCMP: InstFloatingCompare<0x01, "FCMP">;
defm FEQL: InstFloatingCompare<0x03, "FEQL">;
defm FUN: InstFloatingCompare<0x02, "FUN">;
}

//
// single format float
//

let mayLoad = true, mayStore = false in
defm LDSF: Inst_r_rri8<0x90, "LDSF">;
let mayLoad = false, mayStore = true in
defm STSF: Inst_r_rri8<0xB0, "STSF">;

//
// Of course we need to be able to go back and forth between integers and floating point values
//

defm FIX: Inst_r_RoundMode_r_Alias<0x05, "FIX">;
defm FIXU: Inst_r_RoundMode_r_Alias<0x07, "FIXU">;
defm FLOT: InstVariant_U<0x08, "FLOT">;
defm SFLOT: InstVariant_U<0x0C, "SFLOT">;

//
// Subroutine linkage
//

let Defs = [rJ] in {
def PUSHJ: Inst_X_YZ<0xF2, (outs), (ins GPR:$X, branch_dest:$YZ), "PUSHJ">;
defm PUSHGO: Inst_rrri8<0xBE, "PUSHGO">;
}
let Uses = [rJ] in
def POP: Inst_X_YZ<0xF8, (outs), (ins ui8imm:$X, ui16imm:$YZ), "POP">;

//
// System considerations
//

let mayLoad = true, mayStore =false in {
defm LDUNC: Inst_r_rri8<0x96, "LDUNC">;
defm PRELD: Inst_r_rri8<0x9A, "PRELD">;
}
let mayLoad = false, mayStore =true in {
defm STUNC: Inst_r_rri8<0xB6, "STUNC">;
defm PREST: Inst_r_rri8<0xBA, "PREST">;
}
let mayLoad = true, mayStore =true in {
defm SYNCD: Inst_i8rri8<0xB8, "SYNCD">;
defm SYNCID: Inst_i8rri8<0xBC, "SYNCID">;
}

//
// MMIX is designed to work not only on a single processor but also in situations where several processors
// share a common memory
//

let Uses = [rP] in
defm CSWAP: Inst_r_rri8<0x94, "CSWAP">;
def SYNC: Inst_i24imm<0xFC, "SYNC">;

//
// Trips and traps
//

def TRAP: Inst_iii<0x00, "TRAP">;
let isAsmParserOnly = true in {
def TRAP_ii: Inst_X_YZ<0x00, (outs), (ins ui8imm:$X, ui16imm:$YZ), "TRAP">;
def TRAP_i: Inst_XYZ<0x00, (outs), (ins ui24imm:$XYZ), "TRAP">;
}
def : InstAlias<"TRAP", (TRAP 0, 0, 0)>;
def RESUME: Inst_XYZ<0xF9, (outs), (ins ui8imm:$XYZ), "RESUME"> {
  let Inst{24...8} = 0;
}

//
// Special registers
//

def GET: Inst_X__Z<0xFE, (outs GPR:$X), (ins SPR:$Z), "GET">;
def PUT: Inst_X__Z<0xF6, (outs), (ins SPR:$X, ui8imm:$Z), "PUT">;
def PUTI: Inst_X__Z<0xF7, (outs), (ins SPR:$X, GPR:$Z), "PUT">;
def GET_i8: Inst_X__Z<0xFE, (outs GPR:$X), (ins i8imm:$Z), "GET"> {
  let isAsmParserOnly = true;
}

def SAVE: Inst_X__Z<0xFA, (outs), (ins GPR:$X, ui8imm:$Z), "SAVE">;
def UNSAVE: Inst_X__Z<0xFB, (outs), (ins ui8imm:$X, GPR:$Z), "UNSAVE">;

//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//
def :Pat<(sextloadi8 (add GPR:$Y, GPR:$Z)), (LDB GPR:$Y, GPR:$Z)>;
#endif
