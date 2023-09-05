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

defvar rel_addr_bit = 0x1;
defvar immed_bit = 0x2;
defvar zar_bit = 0x4;
defvar zr_bit = 0x8;
defvar yar_bit = 0x10;
defvar yr_bit = 0x20;
defvar xar_bit = 0x40;
defvar xr_bit = 0x80;
defvar yzar_bit = 0x100;
defvar yzr_bit = 0x200;
defvar xyzar_bit = 0x400;
defvar xyzr_bit = 0x800;
defvar one_arg_bit = 0x1000;
defvar two_arg_bit = 0x2000;
defvar three_arg_bit = 0x4000;
defvar many_arg_bit = 0x8000;
defvar align_bits = 0x30000;
defvar no_label_bit = 0x40000;
defvar mem_bit = 0x80000;
defvar spec_bit = 0x100000;


class OpSpec<string name, bits<8> op_code, int spec> {
  string Name = name;
  bits<8> OpCode = op_code;
  int SpecBits = spec;
}

defvar OpInitTable = [
  OpSpec<"TRAP", 0x0, 0x27554>,
  OpSpec<"FCMP", 0x1, 0x240A8>,
  OpSpec<"FUN", 0x2, 0x240A8>,
  OpSpec<"FEQL", 0x3, 0x240A8>,
  OpSpec<"FADD", 0x4, 0x240A8>,
  OpSpec<"FIX", 0x5, 0x26288>,
  OpSpec<"FSUB", 0x6, 0x240A8>,
  OpSpec<"FIXU", 0x7, 0x26288>,
  OpSpec<"FLOT", 0x8, 0x26282>,
  OpSpec<"FLOTU", 0xA, 0x26282>,
  OpSpec<"SFLOT", 0xC, 0x26282>,
  OpSpec<"SFLOTU", 0xE, 0x26282>,
  OpSpec<"FMUL", 0x10, 0x240A8>,
  OpSpec<"FCMPE", 0x11, 0x240A8>,
  OpSpec<"FUNE", 0x12, 0x240A8>,
  OpSpec<"FEQLE", 0x13, 0x240A8>,
  OpSpec<"FDIV", 0x14, 0x240A8>,
  OpSpec<"FSQRT", 0x15, 0x26288>,
  OpSpec<"FREM", 0x16, 0x240A8>,
  OpSpec<"FINT", 0x17, 0x26288>,
  OpSpec<"MUL", 0x18, 0x240A2>,
  OpSpec<"MULU", 0x1A, 0x240A2>,
  OpSpec<"DIV", 0x1C, 0x240A2>,
  OpSpec<"DIVU", 0x1E, 0x240A2>,
  OpSpec<"ADD", 0x20, 0x240A2>,
  OpSpec<"ADDU", 0x22, 0x240A2>,
  OpSpec<"SUB", 0x24, 0x240A2>,
  OpSpec<"SUBU", 0x26, 0x240A2>,
  OpSpec<"2ADDU", 0x28, 0x240A2>,
  OpSpec<"4ADDU", 0x2A, 0x240A2>,
  OpSpec<"8ADDU", 0x2C, 0x240A2>,
  OpSpec<"16ADDU", 0x2E, 0x240A2>,
  OpSpec<"CMP", 0x30, 0x240A2>,
  OpSpec<"CMPU", 0x32, 0x240A2>,
  OpSpec<"NEG", 0x34, 0x26082>,
  OpSpec<"NEGU", 0x36, 0x26082>,
  OpSpec<"SL", 0x38, 0x240A2>,
  OpSpec<"SLU", 0x3A, 0x240A2>,
  OpSpec<"SR", 0x3C, 0x240A2>,
  OpSpec<"SRU", 0x3E, 0x240A2>,
  OpSpec<"BN", 0x40, 0x22081>,
  OpSpec<"BZ", 0x42, 0x22081>,
  OpSpec<"BP", 0x44, 0x22081>,
  OpSpec<"BOD", 0x46, 0x22081>,
  OpSpec<"BNN", 0x48, 0x22081>,
  OpSpec<"BNZ", 0x4A, 0x22081>,
  OpSpec<"BNP", 0x4C, 0x22081>,
  OpSpec<"BEV", 0x4E, 0x22081>,
  OpSpec<"PBN", 0x50, 0x22081>,
  OpSpec<"PBZ", 0x52, 0x22081>,
  OpSpec<"PBP", 0x54, 0x22081>,
  OpSpec<"PBOD", 0x56, 0x22081>,
  OpSpec<"PBNN", 0x58, 0x22081>,
  OpSpec<"PBNZ", 0x5A, 0x22081>,
  OpSpec<"PBNP", 0x5C, 0x22081>,
  OpSpec<"PBEV", 0x5E, 0x22081>,
  OpSpec<"CSN", 0x60, 0x240A2>,
  OpSpec<"CSZ", 0x62, 0x240A2>,
  OpSpec<"CSP", 0x64, 0x240A2>,
  OpSpec<"CSOD", 0x66, 0x240A2>,
  OpSpec<"CSNN", 0x68, 0x240A2>,
  OpSpec<"CSNZ", 0x6A, 0x240A2>,
  OpSpec<"CSNP", 0x6C, 0x240A2>,
  OpSpec<"CSEV", 0x6E, 0x240A2>,
  OpSpec<"ZSN", 0x70, 0x240A2>,
  OpSpec<"ZSZ", 0x72, 0x240A2>,
  OpSpec<"ZSP", 0x74, 0x240A2>,
  OpSpec<"ZSOD", 0x76, 0x240A2>,
  OpSpec<"ZSNN", 0x78, 0x240A2>,
  OpSpec<"ZSNZ", 0x7A, 0x240A2>,
  OpSpec<"ZSNP", 0x7C, 0x240A2>,
  OpSpec<"ZSEV", 0x7E, 0x240A2>,
  OpSpec<"LDB", 0x80, 0xA60A2>,
  OpSpec<"LDBU", 0x82, 0xA60A2>,
  OpSpec<"LDW", 0x84, 0xA60A2>,
  OpSpec<"LDWU", 0x86, 0xA60A2>,
  OpSpec<"LDT", 0x88, 0xA60A2>,
  OpSpec<"LDTU", 0x8A, 0xA60A2>,
  OpSpec<"LDO", 0x8C, 0xA60A2>,
  OpSpec<"LDOU", 0x8E, 0xA60A2>,
  OpSpec<"LDSF", 0x90, 0xA60A2>,
  OpSpec<"LDHT", 0x92, 0xA60A2>,
  OpSpec<"CSWAP", 0x94, 0xA60A2>,
  OpSpec<"LDUNC", 0x96, 0xA60A2>,
  OpSpec<"LDVTS", 0x98, 0xA60A2>,
  OpSpec<"PRELD", 0x9A, 0xA6022>,
  OpSpec<"PREGO", 0x9C, 0xA6022>,
  OpSpec<"GO", 0x9E, 0xA60A2>,
  OpSpec<"STB", 0xA0, 0xA60A2>,
  OpSpec<"STBU", 0xA2, 0xA60A2>,
  OpSpec<"STW", 0xA4, 0xA60A2>,
  OpSpec<"STWU", 0xA6, 0xA60A2>,
  OpSpec<"STT", 0xA8, 0xA60A2>,
  OpSpec<"STTU", 0xAA, 0xA60A2>,
  OpSpec<"STO", 0xAC, 0xA60A2>,
  OpSpec<"STOU", 0xAE, 0xA60A2>,
  OpSpec<"STSF", 0xB0, 0xA60A2>,
  OpSpec<"STHT", 0xB2, 0xA60A2>,
  OpSpec<"STCO", 0xB4, 0xA6022>,
  OpSpec<"STUNC", 0xB6, 0xA60A2>,
  OpSpec<"SYNCD", 0xB8, 0xA6022>,
  OpSpec<"PREST", 0xBA, 0xA6022>,
  OpSpec<"SYNCID", 0xBC, 0xA6022>,
  OpSpec<"PUSHGO", 0xBE, 0xA6062>,
  OpSpec<"OR", 0xC0, 0x240A2>,
  OpSpec<"ORN", 0xC2, 0x240A2>,
  OpSpec<"NOR", 0xC4, 0x240A2>,
  OpSpec<"XOR", 0xC6, 0x240A2>,
  OpSpec<"AND", 0xC8, 0x240A2>,
  OpSpec<"ANDN", 0xCA, 0x240A2>,
  OpSpec<"NAND", 0xCC, 0x240A2>,
  OpSpec<"NXOR", 0xCE, 0x240A2>,
  OpSpec<"BDIF", 0xD0, 0x240A2>,
  OpSpec<"WDIF", 0xD2, 0x240A2>,
  OpSpec<"TDIF", 0xD4, 0x240A2>,
  OpSpec<"ODIF", 0xD6, 0x240A2>,
  OpSpec<"MUX", 0xD8, 0x240A2>,
  OpSpec<"SADD", 0xDA, 0x240A2>,
  OpSpec<"MOR", 0xDC, 0x240A2>,
  OpSpec<"MXOR", 0xDE, 0x240A2>,
  OpSpec<"SETH", 0xE0, 0x22080>,
  OpSpec<"SETMH", 0xE1, 0x22080>,
  OpSpec<"SETML", 0xE2, 0x22080>,
  OpSpec<"SETL", 0xE3, 0x22080>,
  OpSpec<"INCH", 0xE4, 0x22080>,
  OpSpec<"INCMH", 0xE5, 0x22080>,
  OpSpec<"INCML", 0xE6, 0x22080>,
  OpSpec<"INCL", 0xE7, 0x22080>,
  OpSpec<"ORH", 0xE8, 0x22080>,
  OpSpec<"ORMH", 0xE9, 0x22080>,
  OpSpec<"ORML", 0xEA, 0x22080>,
  OpSpec<"ORL", 0xEB, 0x22080>,
  OpSpec<"ANDNH", 0xEC, 0x22080>,
  OpSpec<"ANDNMH", 0xED, 0x22080>,
  OpSpec<"ANDNML", 0xEE, 0x22080>,
  OpSpec<"ANDNL", 0xEF, 0x22080>,
  OpSpec<"JMP", 0xF0, 0x21001>,
  OpSpec<"PUSHJ", 0xF2, 0x22041>,
  OpSpec<"GETA", 0xF4, 0x22081>,
  OpSpec<"PUT", 0xF6, 0x22002>,
  OpSpec<"POP", 0xF8, 0x23000>,
  OpSpec<"RESUME", 0xF9, 0x21000>,
  OpSpec<"SAVE", 0xFA, 0x22080>,
  OpSpec<"UNSAVE", 0xFB, 0x23A00>,
  OpSpec<"SYNC", 0xFC, 0x21000>,
  OpSpec<"SWYM", 0xFD, 0x27554>,
  OpSpec<"GET", 0xFE, 0x22080>,
  OpSpec<"TRIP", 0xFF, 0x27554>,
];

//
// Loading and storing
//

let mayLoad = true, mayStore = false in
multiclass LoadInst<int idx> {
  defm ""#SuffixList<idx>.ret: Inst_r_rri8base<!add(0x80, !mul(idx, 4)), "LD"#SuffixList<idx>.ret>;
  defm ""#SuffixList<idx>.ret#"U": Inst_r_rri8base<!add(0x82, !mul(idx, 4)), "LD"#SuffixList<idx>.ret#"U">;
}

let mayLoad = false, mayStore = true in
multiclass StoreInst<int idx> {
  let Defs = [rA] in
  defm ""#SuffixList<idx>.ret: Inst_rrri8base<!add(0xA0, !mul(idx, 4)), "ST"#SuffixList<idx>.ret>;
  defm ""#SuffixList<idx>.ret#"U": Inst_rrri8base<!add(0xA2, !mul(idx, 4)), "ST"#SuffixList<idx>.ret#"U">;
}

/// another obscure definition
foreach s = {0...3} in {
  defm LD: LoadInst<s>;
  defm ST: StoreInst<s>;
}

let mayLoad = true, mayStore = false in
defm LDHT  : Inst_r_rri8base<0x92, "LDHT">;

let mayLoad = false, mayStore = true in
defm STHT : Inst_rrri8base<0xB2, "STHT">;

let mayLoad = false, mayStore = true in
def STCO: Inst_X_Y_Z<0xB4, (outs), (ins ui8imm:$X, GPR:$Y, GPR:$Z), "STCO">;
def STCOI: Inst_X_Y_Z<0xB5, (outs), (ins ui8imm:$X, GPR:$Y, ui8imm:$Z), "STCO">;
let isAsmParserOnly = true, AsmMatchConverter = "resolveBaseAddress" in {
  def STCO_base : Inst_X_YZ<0xB4, (outs), (ins ui8imm:$X, base_address:$YZ), "STCO">;
}

//
// Adding and subtracting
//

multiclass InstVariant_U<bits<8> opcode, string opstr> {
  let Defs = [rA] in
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
def NEG_base : Inst_r_base<0x34, "NEG">;
def NEGU_base : Inst_r_base<0x36, "NEGU">;

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
defm SuffixList<i>.ret#DIF: Inst_r_rri8<!add(0xD0, !mul(i, 2)), SuffixList<i>.ret#"DIF">;
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

defm GO: Inst_r_rri8base<0x9E, "GO">;


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
def : InstAlias<"POP", (POP 0, 0)>;
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

defm PREGO: Inst_i8rri8<0x9C, "PREGO">;

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
def TRAP_iri: Inst_X_Y_Z<0x00, (outs), (ins ui8imm:$X, GPR:$Y, ui8imm:$Z), "TRAP">;
}
def : InstAlias<"TRAP", (TRAP 0, 0, 0)>;
def : InstAlias<"TRAP $X, $Y", (TRAP_iri ui8imm:$X, GPR:$Y, 0)>;
def RESUME: Inst_XYZ<0xF9, (outs), (ins ui24imm:$XYZ), "RESUME"> {
  let Inst{24...8} = 0;
}
def : InstAlias<"RESUME", (RESUME 0)>;

def TRIP: Inst_iii <0xFF, "TRIP">;
let isAsmParserOnly = true in {
def TRIP_ii: Inst_X_YZ<0xFF, (outs), (ins ui8imm:$X, ui16imm:$YZ), "TRIP">;
def TRIP_i: Inst_XYZ<0xFF, (outs), (ins ui24imm:$XYZ), "TRIP">;
def TRIP_iri: Inst_X_Y_Z<0xFF, (outs), (ins ui8imm:$X, GPR:$Y, ui8imm:$Z), "TRIP">;
def TRIP_irr: Inst_X_Y_Z<0xFF, (outs), (ins ui8imm:$X, GPR:$Y, GPR:$Z), "TRIP">;
}
def : InstAlias<"TRIP", (TRIP 0, 0, 0)>;
def : InstAlias<"TRIP $X, $Y", (TRIP_iri ui8imm:$X, GPR:$Y, 0)>;
//
// Special registers
//

def GET: Inst_X__Z<0xFE, (outs GPR:$X), (ins SPR:$Z), "GET">;
def PUT: Inst_X__Z<0xF6, (outs), (ins ui8imm:$X, ui8imm:$Z), "PUT">;
def PUTI: Inst_X__Z<0xF7, (outs), (ins ui8imm:$X, GPR:$Z), "PUT">;
def GET_i8: Inst_X__Z<0xFE, (outs GPR:$X), (ins i8imm:$Z), "GET"> {
  let isAsmParserOnly = true;
}

def SAVE: Inst_X__Z<0xFA, (outs), (ins GPR:$X, ui8imm:$Z), "SAVE">;
def : InstAlias<"SAVE $X", (SAVE GPR:$X, 0)>;
def UNSAVE: Inst_X__Z<0xFB, (outs), (ins ui8imm:$X, GPR:$Z), "UNSAVE">;
def : InstAlias<"UNSAVE $Y", (UNSAVE 0, GPR:$Y)>;

//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//

// load and store
def :Pat<(sextloadi8 (add i64:$Y, i64:$Z)), (i64 (LDB i64:$Y, i64:$Z))>;
def :Pat<(sextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi8 (add i64:$Y, i64:$Z)), (i64 (LDBU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBUI i64:$Y, i64:$Z))>;

def :Pat<(sextloadi16 (add i64:$Y, i64:$Z)), (i64 (LDW i64:$Y, i64:$Z))>;
def :Pat<(sextloadi16 (add i64:$Y, ui8imm:$Z)), (i64 (LDWI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi16 (add i64:$Y, i64:$Z)), (i64 (LDWU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi16 (add i64:$Y, ui8imm:$Z)), (i64 (LDWUI i64:$Y, i64:$Z))>;

def :Pat<(sextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDT i64:$Y, i64:$Z))>;
def :Pat<(sextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDTU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTUI i64:$Y, i64:$Z))>;

def :Pat<(sextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDO i64:$Y, i64:$Z))>;
def :Pat<(sextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDOU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;

def :Pat<(shl (sextloadi32 (add i64:$Y, i64:$Z)), (i8 32)), (i64 (LDHT i64:$Y, i64:$Z))>;
def :Pat<(shl (sextloadi32 (add i64:$Y, ui8imm:$Z)), (i8 32)), (i64 (LDHTI i64:$Y, i64:$Z))>;


def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STB i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBI i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STBU i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STW i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWI i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STWU i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STT i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTI i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STTU i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTUI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STO i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STOU i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;

def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, i64:$Z)), (STCO ui8imm:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, ui8imm:$Z)), (STCO ui8imm:$X, i64:$Y, ui8imm:$Z)>;

def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, i64:$Z)), (STHT i64:$X, i64:$Y, i64:$Z)>;
def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, ui8imm:$Z)), (STHT i64:$X, i64:$Y, ui8imm:$Z)>;


// add and sub
def :Pat<(add i64:$Y, i64:$Z), (ADD i64:$Y, i64:$Z)>;
def :Pat<(add i64:$Y, ui8imm:$Z), (ADDI i64:$Y, ui8imm:$Z)>;
def :Pat<(add i64:$Y, i64:$Z), (ADDU i64:$Y, i64:$Z)>;
def :Pat<(add i64:$Y, ui8imm:$Z), (ADDUI i64:$Y, ui8imm:$Z)>;
def :Pat<(add (mul i64:$Y, 2), i64:$Z), (ADD2U i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 4), i64:$Z), (ADD4U i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 8), i64:$Z), (ADD8U i64:$Y, i64:$Z)>;
def :Pat<(add (mul i64:$Y, 16), i64:$Z), (ADD16U i64:$Y, i64:$Z)>;

def :Pat<(sub i64:$Y, i64:$Z), (SUB i64:$Y, i64:$Z)>;
def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBI i64:$Y, ui8imm:$Z)>;
def :Pat<(sub i64:$Y, i64:$Z), (SUB i64:$Y, i64:$Z)>;
def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBI i64:$Y, ui8imm:$Z)>;
def :Pat<(sub i64:$Y, i64:$Z), (SUBU i64:$Y, i64:$Z)>;
def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBUI i64:$Y, ui8imm:$Z)>;

def :Pat<(sub ui8imm:$Y, i64:$Z), (NEG ui8imm:$Y, i64:$Z)>;
def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGI ui8imm:$Y, ui8imm:$Z)>;
def :Pat<(sub ui8imm:$Y, i64:$Z), (NEGU ui8imm:$Y, i64:$Z)>;
def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGUI ui8imm:$Y, ui8imm:$Z)>;

// bit fiddling
def :Pat<(and i64:$Y, i64:$Z), (AND i64:$Y, i64:$Z)>;
def :Pat<(and i64:$Y, ui8imm:$Z), (ANDI i64:$Y, i64:$Z)>;

def :Pat<(or i64:$Y, i64:$Z), (OR i64:$Y, i64:$Z)>;
def :Pat<(or i64:$Y, ui8imm:$Z), (ORI i64:$Y, i64:$Z)>;

def :Pat<(xor i64:$Y, i64:$Z), (XOR i64:$Y, i64:$Z)>;
def :Pat<(xor i64:$Y, ui8imm:$Z), (XORI i64:$Y, i64:$Z)>;

def :Pat<(and i64:$Y, (not i64:$Z)), (ANDN i64:$Y, i64:$Z)>;
def :Pat<(and i64:$Y, (not ui8imm:$Z)), (ANDNI i64:$Y, ui8imm:$Z)>;

def :Pat<(or i64:$Y, (not i64:$Z)), (ORN i64:$Y, i64:$Z)>;
def :Pat<(or i64:$Y, (not ui8imm:$Z)), (ORNI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (and i64:$Y, i64:$Z)), (NAND i64:$Y, i64:$Z)>;
def :Pat<(not (and i64:$Y, ui8imm:$Z)), (NANDI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (or i64:$Y, i64:$Z)), (NOR i64:$Y, i64:$Z)>;
def :Pat<(not (or i64:$Y, ui8imm:$Z)), (NORI i64:$Y, ui8imm:$Z)>;

def :Pat<(not (xor i64:$Y, i64:$Z)), (NXOR i64:$Y, i64:$Z)>;
def :Pat<(not (xor i64:$Y, ui8imm:$Z)), (NXORI i64:$Y, ui8imm:$Z)>;

// // mux

def :Pat<(usubsat v8i8:$Y, v8i8:$Z), (BDIF v8i8:$Y, v8i8:$Z)>;
#endif
