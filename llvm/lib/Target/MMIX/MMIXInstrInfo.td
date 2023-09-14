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
def sextloadi64 : PatFrag<(ops node : $ptr), (sextload node : $ptr)> {
  let IsLoad = true;
  let MemoryVT = i64;
}

def zextloadi64 : PatFrag<(ops node : $ptr), (zextload node : $ptr)> {
  let IsLoad = true;
  let MemoryVT = i64;
}

// a fake truncstore, it is just store i64
def truncstorei64 : PatFrag<(ops node
                             : $val, node
                             : $ptr),
                            (store node
                             : $val, node
                             : $ptr)> {
  let IsStore = true;
  let MemoryVT = i64;
  let IsTruncStore = false;
}

//===----------------------------------------------------------------------===//
// Instruction Definition
//===----------------------------------------------------------------------===//

defm TRAP : Inst_trip_trap<0x00>;
def : InstAlias<"TRAP", (TRAP 0)>;

def FCMP : Inst_r_rr<0x01> {
  let isCompare = true;
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FUN : Inst_r_rr<0x02>;

def FEQL : Inst_r_rr<0x03> { let isCompare = true; }

def FADD : Inst_r_rr<0x04> {
  let isAdd = true;
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FIX : Inst_r_roundz<0x05, "FIX"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}
let EmitPriority = 1 in def : InstAlias<"FIX\t$X,$Z", (FIX GPR
                                                       : $X, 0, GPR
                                                       : $Z)>;

def FSUB : Inst_r_rr<0x06>;

def FIXU : Inst_r_roundz<0x07>;

defm FLOT : Inst_r_roundzi<0x08>;
defm FLOTU : Inst_r_roundzi<0x0A>;
defm SFLOT : Inst_r_roundzi<0x0C>;
defm SFLOTU : Inst_r_roundzi<0x0E>;

def FMUL : Inst_r_rr<0x10> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

let Uses = [rE], Defs = [rA] in {
  def FCMPE : Inst_r_rr<0x11>;
  def FUNE : Inst_r_rr<0x12> { let Defs = []; }
  def FEQLE : Inst_r_rr<0x13>;
}

def FDIV : Inst_r_rr<0x14> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FSQRT : Inst_X_Y_Z<0x15,
                       (outs GPR
                        : $X),
                       (ins round_mode
                        : $Y, GPR
                        : $Z),
                       "FSQRT"> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}
def : InstAlias<"FSQRT\t$X,$Z", (FSQRT GPR : $X, 0, GPR : $Z)>;

def FREM : Inst_r_rr<0x16> {
  let mayRaiseFPException = true;
  let Defs = [rA];
}

def FINT : Inst_r_roundz<0x17>;
def : InstAlias<"FINT\t$X,$Z", (FINT GPR : $X, 0, GPR : $Z)>;

let Defs = [rA] in defm MUL : Inst_r_rri<0x18>;

let Defs = [rH] in defm MULU : Inst_r_rri<0x1A>;

let Defs = [rA, rR] in defm DIV : Inst_r_rri<0x1C>;

let Defs = [rR], Uses = [rD] in defm DIVU : Inst_r_rri<0x1E>;

let isAdd = true, Defs = [rA] in defm ADD : Inst_r_rri<0x20>;

let isAdd = true in defm ADDU : Inst_r_rri<0x22>;

let isAsmParserOnly = true in defm LDA : Inst_r_rri<0x22, true>;

let Defs = [rA] in defm SUB : Inst_r_rri<0x24>;

defm SUBU : Inst_r_rri<0x26>;

defm I2ADDU : Inst_r_rri<0x28>;
def : MnemonicAlias<"ADD2U", "2ADDU">;

defm I4ADDU : Inst_r_rri<0x2A>;
def : MnemonicAlias<"ADDU4", "ADD4U">;

defm I8ADDU : Inst_r_rri<0x2C>;
def : MnemonicAlias<"ADDU8", "ADD8U">;

defm I16ADDU : Inst_r_rri<0x2E>;
def : MnemonicAlias<"ADDU16", "ADD16U">;

let isCompare = true in defm CMP : Inst_r_rri<0x30>;

let isCompare = true in defm CMPU : Inst_r_rri<0x32>;

let Defs = [rA] in defm NEG : Inst_r_iri<0x34, "NEG">;
def : InstAlias<"NEG\t$X,$Y", (NEG GPR : $X, 0, GPR : $Y)>;
def : InstAlias<"NEG\t$X,$Y", (NEGI GPR : $X, 0, ui8imm : $Y)>;

defm NEGU : Inst_r_iri<0x36, "NEGU">;
def : InstAlias<"NEGU\t$X,$Y", (NEGU GPR : $X, 0, GPR : $Y)>;
def : InstAlias<"NEGU\t$X,$Y", (NEGUI GPR : $X, 0, ui8imm : $Y)>;

let Defs = [rA] in defm SL : Inst_r_rri<0x38>;

defm SLU : Inst_r_rri<0x3A>;

defm SR : Inst_r_rri<0x3C>;

defm SRU : Inst_r_rri<0x3E>;

defm BN : Inst__rbranch<0x40>;

defm BZ : Inst__rbranch<0x42>;

defm BP : Inst__rbranch<0x44>;

defm BOD : Inst__rbranch<0x46>;

defm BNN : Inst__rbranch<0x48>;

defm BNZ : Inst__rbranch<0x4A>;

defm BNP : Inst__rbranch<0x4C>;

defm BEV : Inst__rbranch<0x4E>;

defm PBN : Inst__rbranch<0x50>;

defm PBZ : Inst__rbranch<0x52>;

defm PBP : Inst__rbranch<0x54>;

defm PBOD : Inst__rbranch<0x56>;

defm PBNN : Inst__rbranch<0x58>;

defm PBNZ : Inst__rbranch<0x5A>;

defm PBNP : Inst__rbranch<0x5C>;

defm PBEV : Inst__rbranch<0x5E>;

defm CSN : Inst_r_rri<0x60>;

defm CSZ : Inst_r_rri<0x62>;

defm CSP : Inst_r_rri<0x64>;

defm CSOD : Inst_r_rri<0x66>;

defm CSNN : Inst_r_rri<0x68>;

defm CSNZ : Inst_r_rri<0x6A>;

defm CSNP : Inst_r_rri<0x6C>;

defm CSEV : Inst_r_rri<0x6E>;

defm ZSN : Inst_r_rri<0x70>;

defm ZSZ : Inst_r_rri<0x72>;

defm ZSP : Inst_r_rri<0x74>;

defm ZSOD : Inst_r_rri<0x76>;

defm ZSNN : Inst_r_rri<0x78>;

defm ZSNZ : Inst_r_rri<0x7A>;

defm ZSNP : Inst_r_rri<0x7C>;

defm ZSEV : Inst_r_rri<0x7E>;

let mayLoad = true in {
  defm LDB : Inst_r_rri<0x80, true>;
  defm LDBU : Inst_r_rri<0x82, true>;
  defm LDW : Inst_r_rri<0x84, true>;
  defm LDWU : Inst_r_rri<0x86, true>;
  defm LDT : Inst_r_rri<0x88, true>;
  defm LDTU : Inst_r_rri<0x8A, true>;
  defm LDO : Inst_r_rri<0x8C, true>;
  defm LDOU : Inst_r_rri<0x8E, true>;

  defm LDSF : Inst_r_rri<0x90, true>;
  defm LDHT : Inst_r_rri<0x92, true>;
}

let isCompare = true,
    Uses = [rP] in defm CSWAP : Inst_r_rri<0x94, true>;

let mayLoad = true in {
  defm LDUNC : Inst_r_rri<0x96, true>;
  defm LDVTS : Inst_r_rri<0x98, true>;
  defm PRELD : Inst_i_rri<0x9A, true>;
  defm PREGO : Inst_i_rri<0x9C, true>;
}

let isBranch = true in defm GO : Inst_r_rri<0x9E, true>;

let mayStore = true in {
  defm STB : Inst__rrri<0xA0, true>;
  defm STBU : Inst__rrri<0xA2, true>;
  defm STW : Inst__rrri<0xA4, true>;
  defm STWU : Inst__rrri<0xA6, true>;
  defm STT : Inst__rrri<0xA8, true>;
  defm STTU : Inst__rrri<0xAA, true>;
  defm STO : Inst__rrri<0xAC, true>;
  defm STOU : Inst__rrri<0xAE, true>;
  defm STSF : Inst__rrri<0xB0, true>;
  defm STHT : Inst__rrri<0xB2, true>;
  defm STCO : Inst__irri<0xB4, true>;
  defm STUNC : Inst__rrri<0xB6, true>;
}

defm SYNCD : Inst__irri<0xB8, true>;

let mayStore = true in defm PREST : Inst__irri<0xBA, true>;

defm SYNCID : Inst__irri<0xBC, true>;

let isBranch = true in defm PUSHGO : Inst__rrri<0xBE, true>;

defm OR : Inst_r_rri<0xC0>;
let EmitPriority = 1 in def : InstAlias<"SET\t$X,$Y", (ORI GPR
                                                       : $X, GPR
                                                       : $Y, 0)>;

defm ORN : Inst_r_rri<0xC2>;

defm NOR : Inst_r_rri<0xC4>;

defm XOR : Inst_r_rri<0xC6>;

defm AND : Inst_r_rri<0xC8>;

defm ANDN : Inst_r_rri<0xCA>;

defm NAND : Inst_r_rri<0xCC>;

defm NXOR : Inst_r_rri<0xCE>;

defm BDIF : Inst_r_rri<0xD0>;

defm WDIF : Inst_r_rri<0xD2>;

defm TDIF : Inst_r_rri<0xD4>;

defm ODIF : Inst_r_rri<0xD6>;

defm MUX : Inst_r_rri<0xD8>;

defm SADD : Inst_r_rri<0xDA>;

defm MOR : Inst_r_rri<0xDC>;

defm MXOR : Inst_r_rri<0xDE>;

def SETH : Inst_r_ui16imm<0xE0>;

def SETMH : Inst_r_ui16imm<0xE1>;

def SETML : Inst_r_ui16imm<0xE2>;

def SETL : Inst_r_ui16imm<0xE3>;

def INCH : Inst_r_ui16imm<0xE4>;

def INCMH : Inst_r_ui16imm<0xE5>;

def INCML : Inst_r_ui16imm<0xE6>;

def INCL : Inst_r_ui16imm<0xE7>;

def ORH : Inst_r_ui16imm<0xE8>;

def ORMH : Inst_r_ui16imm<0xE9>;

def ORML : Inst_r_ui16imm<0xEA>;

def ORL : Inst_r_ui16imm<0xEB>;

def ANDNH : Inst_r_ui16imm<0xEC>;

def ANDNMH : Inst_r_ui16imm<0xED>;

def ANDNML : Inst_r_ui16imm<0xEE>;

def ANDNL : Inst_r_ui16imm<0xEF>;

defm JMP : Inst_jmpb<0xF0>;

let isCall = true in defm PUSHJ : Inst__rbranch<0xF2>;
let isAsmParserOnly = true in defm PUSHJ_imm: Inst__ibranch<0xF2>;

defm GETA : Inst_r_branch<0xF4>;

def PUT : Inst_X__Z<0xF6, (outs), (ins SFR : $X, GPR : $Z), "PUT">;
def PUTI : Inst_X__Z<0xF7, (outs), (ins SFR : $X, ui8imm : $Z), "PUT">;

let isReturn = true in def POP : Inst_X_YZ<0xF8, (outs),
                                           (ins ui8imm
                                            : $X, ui16imm
                                            : $YZ),
                                           "POP">;
def : InstAlias<"POP", (POP 0, 0)>;

def RESUME : MMIXInst<0xF9, (outs), (ins ui8imm : $Z), "RESUME", "$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"RESUME", (RESUME 0)>;

def SAVE : MMIXInst<0xFA, (outs GPR : $X), (ins ui8imm : $Z), "SAVE", "$X,$Z"> {
  bits<8> X;

  let Inst{23...16} = X;
  let Inst{15...0} = 0;
}

def UNSAVE : MMIXInst<0xFB, (outs),
                      (ins ui8imm
                       : $X, GPR
                       : $Z),
                      "UNSAVE", "$X,$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"UNSAVE\t$Z", (UNSAVE 0, GPR : $Z)>;

def SYNC : Inst_XYZ<0xFC, (outs), (ins ui24imm : $XYZ), "SYNC">;
def : InstAlias<"SYNC", (SYNC 0)>;

def SWYM : Inst_X_Y_Z<0xFD, (outs),
                      (ins ui8imm
                       : $X, ui8imm
                       : $Y, ui8imm
                       : $Z),
                      "SWYM">;
def : InstAlias<"SWYM", (SWYM 0, 0, 0)>;
let isAsmParserOnly = true in def : Inst_X_YZ<0xFD, (outs),
                                              (ins ui8imm
                                               : $X, ui16imm
                                               : $YZ),
                                              "SWYM">;

def GET : Inst_X__Z<0xFE, (outs GPR : $X), (ins SFR : $Z), "GET">;

defm TRIP : Inst_trip_trap<0xFF>;
def : InstAlias<"TRIP", (TRIP 0)>;
//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//

// load and store
// def :Pat<(sextloadi8 (add i64:$Y, i64:$Z)), (LDB i64:$Y, i64:$Z)>;
// def :Pat<(sextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi8 (add i64:$Y, i64:$Z)), (i64 (LDBU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi8 (add i64:$Y, ui8imm:$Z)), (i64 (LDBUI i64:$Y, i64:$Z))>;

def : Pat<(sextloadi16(add i64 : $Y, i64 : $Z)), (i64(LDW i64 : $Y, i64 : $Z))>;
def : Pat<(sextloadi16(add i64
                       : $Y, ui8imm
                       : $Z)),
          (i64(LDWI i64
               : $Y, i64
               : $Z))>;
def : Pat<(zextloadi16(add i64
                       : $Y, i64
                       : $Z)),
          (i64(LDWU i64
               : $Y, i64
               : $Z))>;
def : Pat<(zextloadi16(add i64
                       : $Y, ui8imm
                       : $Z)),
          (i64(LDWUI i64
               : $Y, i64
               : $Z))>;

// def :Pat<(sextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDT i64:$Y, i64:$Z))>;
// def :Pat<(sextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi32 (add i64:$Y, i64:$Z)), (i64 (LDTU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi32 (add i64:$Y, ui8imm:$Z)), (i64 (LDTUI i64:$Y,
// i64:$Z))>;

// def :Pat<(sextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDO i64:$Y, i64:$Z))>;
// def :Pat<(sextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi64 (add i64:$Y, i64:$Z)), (i64 (LDOU i64:$Y, i64:$Z))>;
// def :Pat<(zextloadi64 (add i64:$Y, ui8imm:$Z)), (i64 (LDOI i64:$Y, i64:$Z))>;

// def :Pat<(shl (sextloadi32 (add i64:$Y, i64:$Z)), (i8 32)), (i64 (LDHT
// i64:$Y, i64:$Z))>; def :Pat<(shl (sextloadi32 (add i64:$Y, ui8imm:$Z)), (i8
// 32)), (i64 (LDHTI i64:$Y, i64:$Z))>;

// def :Pat<(truncstorei8 i64:$X, (add i64:$Y, i64:$Z)), (STB i64:$X, i64:$Y,
// i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add i64:$Y, ui8imm:$Z)), (STBI
// i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add i64:$Y,
// i64:$Z)), (STBU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei8 i64:$X, (add
// i64:$Y, ui8imm:$Z)), (STBUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei16 i64:$X, (add i64:$Y, i64:$Z)), (STW i64:$X, i64:$Y,
// i64:$Z)>; def :Pat<(truncstorei16 i64:$X, (add i64:$Y, ui8imm:$Z)), (STWI
// i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei16 i64:$X, (add i64:$Y,
// i64:$Z)), (STWU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei16 i64:$X,
// (add i64:$Y, ui8imm:$Z)), (STWUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei32 i64:$X, (add i64:$Y, i64:$Z)), (STT i64:$X, i64:$Y,
// i64:$Z)>; def :Pat<(truncstorei32 i64:$X, (add i64:$Y, ui8imm:$Z)), (STTI
// i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 i64:$X, (add i64:$Y,
// i64:$Z)), (STTU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 i64:$X,
// (add i64:$Y, ui8imm:$Z)), (STTUI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei64 i64:$X, (add i64:$Y, i64:$Z)), (STO i64:$X, i64:$Y,
// i64:$Z)>; def :Pat<(truncstorei64 i64:$X, (add i64:$Y, ui8imm:$Z)), (STOI
// i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 i64:$X, (add i64:$Y,
// i64:$Z)), (STOU i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 i64:$X,
// (add i64:$Y, ui8imm:$Z)), (STOI i64:$X, i64:$Y, i64:$Z)>;

// def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y, i64:$Z)), (STCO ui8imm:$X,
// i64:$Y, i64:$Z)>; def :Pat<(truncstorei64 ui8imm:$X, (add i64:$Y,
// ui8imm:$Z)), (STCO ui8imm:$X, i64:$Y, ui8imm:$Z)>;

// def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add i64:$Y, i64:$Z)), (STHT
// i64:$X, i64:$Y, i64:$Z)>; def :Pat<(truncstorei32 (srl i64:$X, (i8 32)), (add
// i64:$Y, ui8imm:$Z)), (STHT i64:$X, i64:$Y, ui8imm:$Z)>;

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
