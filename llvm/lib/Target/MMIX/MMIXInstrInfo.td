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


//===----------------------------------------------------------------------===//
// Instruction Definition
//===----------------------------------------------------------------------===//

defm TRAP : Inst_trip_trap<0x00, "TRAP">;
def : InstAlias<"TRAP", (TRAP 0)>;

def FCMP : Inst_r_rr<0x01, "FCMP"> {
  let isCompare = true;
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FUN : Inst_r_rr<0x02, "FUN">;

def FEQL   : Inst_r_rr<0x03, "FEQL"> {
  let isCompare = true;
}

def FADD : Inst_r_rr<0x04, "FADD"> {
  let isAdd = true;
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FIX : Inst_r_roundz<0x05, "FIX"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}
let EmitPriority = 1 in
  def : InstAlias<"FIX\t$X,$Z", (FIX GPR:$X, 0, GPR:$Z)>;

def FSUB : Inst_r_rr<0x06, "FSUB">;

def FIXU : Inst_r_roundz<0x07, "FIXU">;

defm FLOT : Inst_r_roundzi<0x08, "FLOT">;
defm FLOTU : Inst_r_roundzi<0x0A, "FLOTU">;
defm SFLOT : Inst_r_roundzi<0x0C, "SFLOT">;
defm SFLOTU : Inst_r_roundzi<0x0E, "SFLOTU">;

def FMUL : Inst_r_rr<0x10, "FMUL"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

let Uses = [rE], Defs = [rA] in {
  def FCMPE : Inst_r_rr<0x11, "FCMPE">;
  def FUNE : Inst_r_rr<0x12, "FUNE"> { let Defs = []; }
  def FEQLE : Inst_r_rr<0x13, "FEQLE">;
}

def FDIV : Inst_r_rr<0x14, "FDIV"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FSQRT : Inst_X_Y_Z<0x15, (outs GPR:$X), (ins round_mode:$Y, GPR:$Z), "FSQRT"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}
def : InstAlias<"FSQRT\t$X,$Z", (FSQRT GPR:$X, 0, GPR:$Z)>;

def FREM : Inst_r_rr<0x16, "FREM"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FINT : Inst_r_roundz<0x17, "FINT">;
def : InstAlias<"FINT\t$X,$Z", (FINT GPR:$X, 0, GPR:$Z)>;

let Defs = [rA] in
  defm MUL : Inst_r_rri<0x18, "MUL">;

let Defs = [rH] in
defm MULU : Inst_r_rri<0x1A, "MULU">;

let Defs = [rA, rR] in
  defm DIV : Inst_r_rri<0x1C, "DIV">;

let Defs = [rR], Uses = [rD] in
  defm DIVU : Inst_r_rri<0x1E, "DIVU">;

let isAdd = true, Defs = [rA] in
  defm ADD : Inst_r_rri<0x20, "ADD">;

let isAdd = true in
  defm ADDU : Inst_r_rri<0x22, "ADDU">;

let isAsmParserOnly = true in
  defm LDA: Inst_r_rri<0x22, "LDA", true>;

let Defs = [rA] in
  defm SUB : Inst_r_rri<0x24, "SUB">;

defm SUBU : Inst_r_rri<0x26, "ADDU">;

defm I2ADDU : Inst_r_rri<0x28, "2ADDU">;
def : MnemonicAlias<"ADD2U", "2ADDU">;

defm I4ADDU : Inst_r_rri<0x2A, "4ADDU">;
def : MnemonicAlias<"ADDU4", "ADD4U">;

defm I8ADDU : Inst_r_rri<0x2C, "8ADDU">;
def : MnemonicAlias<"ADDU8", "ADD8U">;

defm I16ADDU : Inst_r_rri<0x2E, "16ADDU">;
def : MnemonicAlias<"ADDU16", "ADD16U">;

let isCompare = true in
  defm CMP : Inst_r_rri<0x30, "CMP">;

let isCompare = true in
  defm CMPU : Inst_r_rri<0x32, "CMPU">;

let Defs = [rA] in
  defm NEG : Inst_r_iri<0x34, "NEG">;
def : InstAlias<"NEG\t$X,$Y", (NEG GPR:$X, 0, GPR:$Y)>;
def : InstAlias<"NEG\t$X,$Y", (NEGI GPR:$X, 0, ui8imm:$Y)>;

defm NEGU : Inst_r_iri<0x36, "NEGU">;
def : InstAlias<"NEGU\t$X,$Y", (NEGU GPR:$X, 0, GPR:$Y)>;
def : InstAlias<"NEGU\t$X,$Y", (NEGUI GPR:$X, 0, ui8imm:$Y)>;

let Defs = [rA] in
  defm SL : Inst_r_rri<0x38, "SL">;

defm SLU : Inst_r_rri<0x3A, "SLU">;

defm SR : Inst_r_rri<0x3C, "SR">;

defm SRU : Inst_r_rri<0x3E, "SRU">;

defm BN : Inst__rbranch<0x40, "BN">;

defm BZ : Inst__rbranch<0x42, "BZ">;

defm BP : Inst__rbranch<0x44, "BP">;

defm BOD : Inst__rbranch<0x46, "BOD">;

defm BNN : Inst__rbranch<0x48, "BNN">;

defm BNZ : Inst__rbranch<0x4A, "BNZ">;

defm BNP : Inst__rbranch<0x4C, "BNP">;

defm BEV : Inst__rbranch<0x4E, "BEV">;

defm PBN : Inst__rbranch<0x50, "PBN">;

defm PBZ : Inst__rbranch<0x52, "PBZ">;

defm PBP : Inst__rbranch<0x54, "PBP">;

defm PBOD : Inst__rbranch<0x56, "PBOD">;

defm PBNN : Inst__rbranch<0x58, "PBNN">;

defm PBNZ : Inst__rbranch<0x5A, "PBNZ">;

defm PBNP : Inst__rbranch<0x5C, "PBNP">;

defm PBEV : Inst__rbranch<0x5E, "PBEV">;

defm CSN : Inst_r_rri<0x60, "CSN">;

defm CSZ : Inst_r_rri<0x62, "CSZ">;

defm CSP : Inst_r_rri<0x64, "CSP">;

defm CSOD : Inst_r_rri<0x66, "CSOD">;

defm CSNN : Inst_r_rri<0x68, "CSNN">;

defm CSNZ : Inst_r_rri<0x6A, "CSNZ">;

defm CSNP : Inst_r_rri<0x6C, "CSNP">;

defm CSEV : Inst_r_rri<0x6E, "CSEV">;

defm ZSN : Inst_r_rri<0x70, "ZSN">;

defm ZSZ : Inst_r_rri<0x72, "ZSZ">;

defm ZSP : Inst_r_rri<0x74, "ZSP">;

defm ZSOD : Inst_r_rri<0x76, "ZSOD">;

defm ZSNN : Inst_r_rri<0x78, "ZSNN">;

defm ZSNZ : Inst_r_rri<0x7A, "ZSNZ">;

defm ZSNP : Inst_r_rri<0x7C, "ZSNP">;

defm ZSEV : Inst_r_rri<0x7E, "ZSEV">;

let mayLoad = true in {
  defm LDB : Inst_r_rri<0x80, "LDB", true>;
  defm LDBU : Inst_r_rri<0x82, "LDBU", true>;
  defm LDW : Inst_r_rri<0x84, "LDW", true>;
  defm LDWU : Inst_r_rri<0x86, "LDWU", true>;
  defm LDT : Inst_r_rri<0x88, "LDT", true>;
  defm LDTU : Inst_r_rri<0x8A, "LDTU", true>;
  defm LDO : Inst_r_rri<0x8C, "LDO", true>;
  defm LDOU : Inst_r_rri<0x8E, "LDOU", true>;

  defm LDSF : Inst_r_rri<0x90, "LDSF", true>;
  defm LDHT : Inst_r_rri<0x92, "LDHT", true>;
}

let isCompare = true, Uses = [rP] in
  defm CSWAP : Inst_r_rri<0x94, "CSWAP", true>;

let mayLoad = true in {
  defm LDUNC : Inst_r_rri<0x96, "LDUNC", true>;
  defm LDVTS : Inst_r_rri<0x98, "LDVTS", true>;
  defm PRELD : Inst_i_rri<0x9A, "PRELD", true>;
  defm PREGO : Inst_i_rri<0x9C, "PREGO", true>;
}

let isBranch = true in
  defm GO : Inst_r_rri<0x9E, "GO", true>;

let mayStore = true in {
  defm STB : Inst__rrri<0xA0, "STB", true>;
  defm STBU : Inst__rrri<0xA2, "STBU", true>;
  defm STW : Inst__rrri<0xA4, "STW", true>;
  defm STWU : Inst__rrri<0xA6, "STWU", true>;
  defm STT : Inst__rrri<0xA8, "STT", true>;
  defm STTU : Inst__rrri<0xAA, "STTU", true>;
  defm STO : Inst__rrri<0xAC, "STO", true>;
  defm STOU : Inst__rrri<0xAE, "STOU", true>;
  defm STSF : Inst__rrri<0xB0, "STSF", true>;
  defm STHT : Inst__rrri<0xB2, "STHT", true>;
  defm STCO : Inst__irri<0xB4, "STCO", true>;
  defm STUNC : Inst__rrri<0xB6, "STUNC", true>;
}

defm SYNCD : Inst__irri<0xB8, "SYNCD", true>;

let mayStore = true in
  defm PREST : Inst__irri<0xBA, "PREST", true>;

defm SYNCID : Inst__irri<0xBC, "SYNCID", true>;

let isBranch = true in
  defm PUSHGO : Inst__rrri<0xBE, "PUSHGO", true>;

defm OR : Inst_r_rri<0xC0, "OR">;
let EmitPriority = 1 in
  def : InstAlias<"SET\t$X,$Y", (ORI GPR:$X, GPR:$Y, 0)>;

defm ORN : Inst_r_rri<0xC2, "ORN">;

defm NOR : Inst_r_rri<0xC4, "NOR">;

defm XOR : Inst_r_rri<0xC6, "XOR">;

defm AND : Inst_r_rri<0xC8, "AND">;

defm ANDN : Inst_r_rri<0xCA, "ANDN">;

defm NAND : Inst_r_rri<0xCC, "NAND">;

defm NXOR : Inst_r_rri<0xCE, "NXOR">;

defm BDIF : Inst_r_rri<0xD0, "BDIF">;

defm WDIF : Inst_r_rri<0xD2, "WDIF">;

defm TDIF : Inst_r_rri<0xD4, "TDIF">;

defm ODIF : Inst_r_rri<0xD6, "ODIF">;

defm MUX : Inst_r_rri<0xD8, "MUX">;

defm SADD : Inst_r_rri<0xDA, "SADD">;

defm MOR : Inst_r_rri<0xDC, "MOR">;

defm MXOR : Inst_r_rri<0xDE, "MXOR">;

def SETH : Inst_r_ui16imm<0xE0, "SETH">;

def SETMH : Inst_r_ui16imm<0xE1, "SETMH">;

def SETML : Inst_r_ui16imm<0xE2, "SETML">;

def SETL : Inst_r_ui16imm<0xE3, "SETL">;

def INCH : Inst_r_ui16imm<0xE4, "INCH">;

def INCMH : Inst_r_ui16imm<0xE5, "INCMH">;

def INCML : Inst_r_ui16imm<0xE6, "INCML">;

def INCL : Inst_r_ui16imm<0xE7, "INCL">;

def ORH : Inst_r_ui16imm<0xE8, "ORH">;

def ORMH : Inst_r_ui16imm<0xE9, "ORMH">;

def ORML : Inst_r_ui16imm<0xEA, "ORML">;

def ORL : Inst_r_ui16imm<0xEB, "ORL">;

def ANDNH : Inst_r_ui16imm<0xEC, "ANDNH">;

def ANDNMH : Inst_r_ui16imm<0xED, "ANDNMH">;

def ANDNML : Inst_r_ui16imm<0xEE, "ANDNML">;

def ANDNL : Inst_r_ui16imm<0xEF, "ANDNL">;

defm JMP : Inst_jmpb<0xF0, "JMP">;

let isCall = true in
  defm PUSHJ : Inst__rbranch<0xF2, "PUSHJ">;
let isAsmParserOnly = true in
  defm : Inst__ibranch<0xF2, "PUSHJ">;

defm GETA : Inst_r_branch<0xF4, "GETA">;

def PUT : Inst_X__Z<0xF6, (outs), (ins SFR:$X, GPR:$Z), "PUT">;
def PUTI : Inst_X__Z<0xF7, (outs), (ins SFR:$X, ui8imm:$Z), "PUT">;

let isReturn = true in
  def POP : Inst_X_YZ<0xF8, (outs), (ins ui8imm:$X, ui16imm:$YZ), "POP">;
def : InstAlias<"POP", (POP 0, 0)>;

def RESUME : MMIXInst<0xF9, (outs), (ins ui8imm:$Z), "RESUME", "$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"RESUME", (RESUME 0)>;

def SAVE : MMIXInst<0xFA,
  (outs GPR:$X), (ins ui8imm:$Z), "SAVE", "$X,$Z"> {
  bits<8> X;

  let Inst{23...16} = X;
  let Inst{15...0} = 0;
}

def UNSAVE : MMIXInst<0xFB,
  (outs), (ins ui8imm:$X, GPR:$Z), "UNSAVE", "$X,$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"UNSAVE\t$Z", (UNSAVE 0, GPR:$Z)>;

def SYNC : Inst_XYZ<0xFC, (outs), (ins ui24imm:$XYZ), "SYNC">;
def : InstAlias<"SYNC", (SYNC 0)>;

def SWYM : Inst_X_Y_Z<0xFD, (outs), (ins ui8imm:$X, ui8imm:$Y, ui8imm:$Z), "SWYM">;
def : InstAlias<"SWYM", (SWYM 0, 0, 0)>;
let isAsmParserOnly = true in
  def : Inst_X_YZ<0xFD, (outs), (ins ui8imm:$X, ui16imm:$YZ), "SWYM">;

def GET : Inst_X__Z<0xFE, (outs GPR:$X), (ins SFR:$Z), "GET">;

defm TRIP : Inst_trip_trap<0xFF, "TRIP">;
def : InstAlias<"TRIP", (TRIP 0)>;
//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//

// load and store
// def :Pat<(sextloadi8 (add i64:$Y, i64:$Z)), (LDB i64:$Y, i64:$Z)>;
// def :Pat<(sextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi8 (add i64:$Y, i64:$Z)), (i64 (LDBU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBUI i64:$Y, i64:$Z))>;

def :Pat<(sextloadi16 (add i64:$Y, i64:$Z)), (i64 (LDW i64:$Y, i64:$Z))>;
def :Pat<(sextloadi16 (add i64:$Y, ui8imm:$Z)), (i64 (LDWI i64:$Y, i64:$Z))>;
def :Pat<(zextloadi16 (add i64:$Y, i64:$Z)), (i64 (LDWU i64:$Y, i64:$Z))>;
def :Pat<(zextloadi16 (add i64:$Y, ui8imm:$Z)), (i64 (LDWUI i64:$Y, i64:$Z))>;

// def :Pat<(sextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDT i64:$Y, i64:$Z))>;
// def :Pat<(sextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDTU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTUI i64:$Y, i64:$Z))>;

// def :Pat<(sextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDO i64:$Y, i64:$Z))>;
// def :Pat<(sextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDOU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;

// def :Pat<(shl (sextloadi32 (add i64:$Y, i64:$Z)), (i8 32)), (i64 (LDHT i64:$Y, i64:$Z))>;
// def :Pat<(shl (sextloadi32 (add i64:$Y, ui8imm:$Z)), (i8 32)), (i64 (LDHTI i64:$Y, i64:$Z))>;


// def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STB i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBI i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STBU i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STW i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWI i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STWU i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STT i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTI i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STTU i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STO i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STOU i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, i64:$Z)), (STCO ui8imm:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, ui8imm:$Z)), (STCO ui8imm:$X, i64:$Y, ui8imm:$Z)>;

// def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, i64:$Z)), (STHT i64:$X, i64:$Y, i64:$Z)>;
// def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, ui8imm:$Z)), (STHT i64:$X, i64:$Y, ui8imm:$Z)>;


// // add and sub
// def :Pat<(add i64:$Y, i64:$Z), (ADD i64:$Y, i64:$Z)>;
// def :Pat<(add i64:$Y, ui8imm:$Z), (ADDI i64:$Y, ui8imm:$Z)>;
// def :Pat<(add i64:$Y, i64:$Z), (ADDU i64:$Y, i64:$Z)>;
// def :Pat<(add i64:$Y, ui8imm:$Z), (ADDUI i64:$Y, ui8imm:$Z)>;
// def :Pat<(add (mul i64:$Y, 2), i64:$Z), (I_2ADDU i64:$Y, i64:$Z)>;
// def :Pat<(add (mul i64:$Y, 4), i64:$Z), (I_4ADDU i64:$Y, i64:$Z)>;
// def :Pat<(add (mul i64:$Y, 8), i64:$Z), (I_8ADDU i64:$Y, i64:$Z)>;
// def :Pat<(add (mul i64:$Y, 16), i64:$Z), (I_16ADDU i64:$Y, i64:$Z)>;

// def :Pat<(sub i64:$Y, i64:$Z), (SUB i64:$Y, i64:$Z)>;
// def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBI i64:$Y, ui8imm:$Z)>;
// def :Pat<(sub i64:$Y, i64:$Z), (SUB i64:$Y, i64:$Z)>;
// def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBI i64:$Y, ui8imm:$Z)>;
// def :Pat<(sub i64:$Y, i64:$Z), (SUBU i64:$Y, i64:$Z)>;
// def :Pat<(sub i64:$Y, ui8imm:$Z), (SUBUI i64:$Y, ui8imm:$Z)>;

// def :Pat<(sub ui8imm:$Y, i64:$Z), (NEG ui8imm:$Y, i64:$Z)>;
// def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGI ui8imm:$Y, ui8imm:$Z)>;
// def :Pat<(sub ui8imm:$Y, i64:$Z), (NEGU ui8imm:$Y, i64:$Z)>;
// def :Pat<(sub ui8imm:$Y, ui8imm:$Z), (NEGUI ui8imm:$Y, ui8imm:$Z)>;

// // bit fiddling
// def :Pat<(and i64:$Y, i64:$Z), (AND i64:$Y, i64:$Z)>;
// def :Pat<(and i64:$Y, ui8imm:$Z), (ANDI i64:$Y, i64:$Z)>;

// def :Pat<(or i64:$Y, i64:$Z), (OR i64:$Y, i64:$Z)>;
// def :Pat<(or i64:$Y, ui8imm:$Z), (ORI i64:$Y, i64:$Z)>;

// def :Pat<(xor i64:$Y, i64:$Z), (XOR i64:$Y, i64:$Z)>;
// def :Pat<(xor i64:$Y, ui8imm:$Z), (XORI i64:$Y, i64:$Z)>;

// def :Pat<(and i64:$Y, (not i64:$Z)), (ANDN i64:$Y, i64:$Z)>;
// def :Pat<(and i64:$Y, (not ui8imm:$Z)), (ANDNI i64:$Y, ui8imm:$Z)>;

// def :Pat<(or i64:$Y, (not i64:$Z)), (ORN i64:$Y, i64:$Z)>;
// def :Pat<(or i64:$Y, (not ui8imm:$Z)), (ORNI i64:$Y, ui8imm:$Z)>;

// def :Pat<(not (and i64:$Y, i64:$Z)), (NAND i64:$Y, i64:$Z)>;
// def :Pat<(not (and i64:$Y, ui8imm:$Z)), (NANDI i64:$Y, ui8imm:$Z)>;

// def :Pat<(not (or i64:$Y, i64:$Z)), (NOR i64:$Y, i64:$Z)>;
// def :Pat<(not (or i64:$Y, ui8imm:$Z)), (NORI i64:$Y, ui8imm:$Z)>;

// def :Pat<(not (xor i64:$Y, i64:$Z)), (NXOR i64:$Y, i64:$Z)>;
// def :Pat<(not (xor i64:$Y, ui8imm:$Z)), (NXORI i64:$Y, ui8imm:$Z)>;

// mux

// def :Pat<(usubsat v8i8:$Y, v8i8:$Z), (BDIF v8i8:$Y, v8i8:$Z)>;
#endif
