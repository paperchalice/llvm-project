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

let isTrap = true, hasSideEffects = true in {
def TRAP : Inst_X_Y_Z<0x00, (outs), (ins u8imm:$X, trap_y:$Y, trap_z:$Z)>;
let isAsmParserOnly = true in {
def : Inst_X_YZ<0x00, (outs), (ins u8imm:$X, u16imm:$YZ), "TRAP">;
def : Inst_XYZ<0x00, (outs), (ins u24imm:$XYZ), "TRAP">;
}
}

def FCMP : Inst_X_Y_Z<0x01, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isCompare = true;
}

def FUN : Inst_X_Y_Z<0x02, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isCompare = true;
}

def FEQL : Inst_X_Y_Z<0x03, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isCompare = true;
}

def FADD : Inst_X_Y_Z<0x04, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isAdd = true;
}

def FIX : Inst_X_Y_Z<0x05, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
}
def : InstAlias<"FIX\t$X,$Z", (FIX GPR:$X, 0, GPR:$Z)>;

def FSUB : Inst_X_Y_Z<0x06, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;

def FIXU : Inst_X_Y_Z<0x07, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def : InstAlias<"FIXU\t$X,$Z", (FIXU GPR:$X, 0, GPR:$Z)>;

def FLOT : Inst_X_Y_Z<0x08, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def FLOTI : Inst_X_Y_Z<0x09, (outs GPR:$X), (ins rounding_mode:$Y, i8imm:$Z)>;
def : InstAlias<"FLOT\t$X,$Z", (FLOT GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"FLOT\t$X,$Z", (FLOTI GPR:$X, 0, i8imm:$Z)>;
def : InstAlias<"FLOT\t$X,$Y,$Z", (FLOTI GPR:$X, rounding_mode:$Y, i8imm:$Z)>;

def FLOTU : Inst_X_Y_Z<0x0A, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def FLOTUI : Inst_X_Y_Z<0x0B, (outs GPR:$X), (ins rounding_mode:$Y, i8imm:$Z)>;
def : InstAlias<"FLOTU\t$X,$Z", (FLOTU GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"FLOTU\t$X,$Z", (FLOTUI GPR:$X, 0, i8imm:$Z)>;
def : InstAlias<"FLOTU\t$X,$Y,$Z", (FLOTUI GPR:$X, rounding_mode:$Y, i8imm:$Z)>;

def SFLOT : Inst_X_Y_Z<0x0C, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def SFLOTI : Inst_X_Y_Z<0x0D, (outs GPR:$X), (ins rounding_mode:$Y, i8imm:$Z)>;
def : InstAlias<"SFLOT\t$X,$Z", (SFLOT GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"SFLOT\t$X,$Z", (SFLOTI GPR:$X, 0, i8imm:$Z)>;
def : InstAlias<"SFLOT\t$X,$Y,$Z", (SFLOTI GPR:$X, rounding_mode:$Y, i8imm:$Z)>;

def SFLOTU : Inst_X_Y_Z<0x0E, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def SFLOTUI : Inst_X_Y_Z<0x0F, (outs GPR:$X), (ins rounding_mode:$Y, i8imm:$Z)>;
def : InstAlias<"SFLOTU\t$X,$Z", (SFLOTU GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"SFLOTU\t$X,$Z", (SFLOTUI GPR:$X, 0, i8imm:$Z)>;
def : InstAlias<"SFLOTU\t$X,$Y,$Z", (SFLOTUI GPR:$X, rounding_mode:$Y, i8imm:$Z)>;

def FMUL : Inst_X_Y_Z<0x10, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
}

def FCMPE : Inst_X_Y_Z<0x11, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Uses = [rE];
  let isCompare = true;
  let mayRaiseFPException = true;
}

def FUNE : Inst_X_Y_Z<0x12, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Uses = [rE];
  let isCompare = true;
  let mayRaiseFPException = true;
}

def FEQLE : Inst_X_Y_Z<0x13, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Uses = [rE];
  let isCompare = true;
  let mayRaiseFPException = true;
}

let mayRaiseFPException = true in
def FDIV : Inst_X_Y_Z<0x14, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;

let mayRaiseFPException = true in
def FSQRT : Inst_X_Y_Z<0x15, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;

let mayRaiseFPException = true in
def FREM : Inst_X_Y_Z<0x16, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;

def FINT : Inst_X_Y_Z<0x17, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def : InstAlias<"FINT\t$X,$Z", (FINT GPR:$X, 0, GPR:$Z)>;

def MUL : Inst_X_Y_Z<0x18, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def MULI : Inst_X_Y_Z<0x19, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"MUL\t$X,$Y,$Z", (MULI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rH] in {
def MULU : Inst_X_Y_Z<0x1A, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def MULUI : Inst_X_Y_Z<0x1B, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"MULU\t$X,$Y,$Z", (MULUI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rR] in {
def DIV : Inst_X_Y_Z<0x1C, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def DIVI : Inst_X_Y_Z<0x1D, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"DIV\t$X,$Y,$Z", (DIVI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rR], Uses=[rD] in {
def DIVU : Inst_X_Y_Z<0x1E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def DIVUI : Inst_X_Y_Z<0x1F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"DIVU\t$X,$Y,$Z", (DIVUI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isAdd = true in {
def ADD : Inst_X_Y_Z<0x20, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADDI : Inst_X_Y_Z<0x21, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"ADD\t$X,$Y,$Z", (ADDI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isAdd = true in {
def ADDU : Inst_X_Y_Z<0x22, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADDUI : Inst_X_Y_Z<0x23, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"ADDU\t$X,$Y,$Z", (ADDUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SUB : Inst_X_Y_Z<0x24, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SUBI : Inst_X_Y_Z<0x25, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SUB\t$X,$Y,$Z", (SUBI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SUBU : Inst_X_Y_Z<0x26, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SUBUI : Inst_X_Y_Z<0x27, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SUBU\t$X,$Y,$Z", (SUBUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD2U : Inst_X_Y_Z<0x28, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADD2UI : Inst_X_Y_Z<0x29, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ADD2U\t$X,$Y,$Z", (ADD2UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD4U : Inst_X_Y_Z<0x2A, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADD4UI : Inst_X_Y_Z<0x2B, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ADD4U\t$X,$Y,$Z", (ADD4UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD8U : Inst_X_Y_Z<0x2C, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADD8UI : Inst_X_Y_Z<0x2D, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ADD8U\t$X,$Y,$Z", (ADD8UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD16U : Inst_X_Y_Z<0x2E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ADD16UI : Inst_X_Y_Z<0x2F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ADD16U\t$X,$Y,$Z", (ADD16UI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isCompare = true in {
def CMP : Inst_X_Y_Z<0x30, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def CMPI : Inst_X_Y_Z<0x31, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"CMP\t$X,$Y,$Z", (CMPI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isCompare = true in {
def CMPU : Inst_X_Y_Z<0x32, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def CMPUI : Inst_X_Y_Z<0x33, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"CMPU\t$X,$Y,$Z", (CMPUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NEG : Inst_X_Y_Z<0x34, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def NEGI : Inst_X_Y_Z<0x35, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"NEG\t$X,$Y,$Z", (NEGI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NEGU : Inst_X_Y_Z<0x36, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def NEGUI : Inst_X_Y_Z<0x37, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"NEGU\t$X,$Y,$Z", (NEGUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SL : Inst_X_Y_Z<0x38, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SLI : Inst_X_Y_Z<0x39, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SL\t$X,$Y,$Z", (SLI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SLU : Inst_X_Y_Z<0x3A, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SLUI : Inst_X_Y_Z<0x3B, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SLU\t$X,$Y,$Z", (SLUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SR : Inst_X_Y_Z<0x3C, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SRI : Inst_X_Y_Z<0x3D, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SR\t$X,$Y,$Z", (SRI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SRU : Inst_X_Y_Z<0x3E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SRUI : Inst_X_Y_Z<0x3F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SRU\t$X,$Y,$Z", (SRUI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(CondList)) in {
  defvar c = CondList[i];
  defvar opc = !add(0x40, !mul(2, i));
  def B#c : InstB<opc>;
  def PB#c : InstB<!add(0x10, opc)>;

  // extension aliases BNI which accept immediate
  let isAsmParserOnly = true, InOperandList = (ins GPR:$X, s17imm:$YZ) in {
    def : InstB<opc, "B"#c#"I">;
    def : InstB<!add(0x10, opc), "PB"#c#"I">;
  }
}

foreach i = !range(!size(CondList)) in {
  defvar c = CondList[i];
  defvar opc = !add(0x60, !mul(2, i));
  let isSelect = true in {
    def CS#c : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
    def CS#c#I : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  }
  defvar csi = !cast<Inst_X_Y_Z>("CS"#c#I);
  def : InstAlias<"CS"#c#"\t$X,$Y,$Z", (csi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

foreach i = !range(!size(CondList)) in {
  defvar c = CondList[i];
  defvar opc = !add(0x70, !mul(2, i));
  let isSelect = true in {
    def ZS#c : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
    def ZS#c#I : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  }
  defvar zsi = !cast<Inst_X_Y_Z>("ZS"#c#I);
  def : InstAlias<"ZS"#c#"\t$X,$Y,$Z", (zsi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0x80, !mul(i, 4));
  let mayLoad = true in {
  def LD#s : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
  def LD#s#I : Inst_X_Y_Z<!add(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  def LD#s#U : Inst_X_Y_Z<!add(opc, 2), (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
  def LD#s#UI : Inst_X_Y_Z<!add(opc, 3), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  }
  def : InstAlias<"LD"#s#"\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("LD"#s#I) GPR:$X, GPR:$Y, u8imm:$Z)>;
  def : InstAlias<"LD"#s#"U\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("LD"#s#"UI") GPR:$X, GPR:$Y, u8imm:$Z)>;
}
let mayLoad = true in {
def LDSF : Inst_X_Y_Z<0x90, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDSFI : Inst_X_Y_Z<0x91, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDSF\t$X,$Y,$Z", (LDSFI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDHT : Inst_X_Y_Z<0x92, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDHTI : Inst_X_Y_Z<0x93, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDHT\t$X,$Y,$Z", (LDHTI GPR:$X, GPR:$Y, u8imm:$Z)>;

def : MnemonicAlias<"LDA", "ADDU">;

def CSWAP : Inst_X_Y_Z<0x94, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def CSWAPI : Inst_X_Y_Z<0x95, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"CSWAP\t$X,$Y,$Z", (CSWAPI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDUNC : Inst_X_Y_Z<0x96, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDUNCI : Inst_X_Y_Z<0x97, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDUNC\t$X,$Y,$Z", (LDUNCI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDVTS : Inst_X_Y_Z<0x98, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDVTSI : Inst_X_Y_Z<0x99, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDVTS\t$X,$Y,$Z", (LDVTSI GPR:$X, GPR:$Y, u8imm:$Z)>;

def PRELD : Inst_X_Y_Z<0x9A, (outs u8imm:$X), (ins GPR:$Y, GPR:$Z)>;
def PRELDI : Inst_X_Y_Z<0x9B, (outs u8imm:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PRELD\t$X,$Y,$Z", (PRELDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

def PREGO : Inst_X_Y_Z<0x9C, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def PREGOI : Inst_X_Y_Z<0x9D, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PREGO\t$X,$Y,$Z", (PREGOI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let isCall = true, isIndirectBranch = true in {
def GO : Inst_X_Y_Z<0x9E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def GOI : Inst_X_Y_Z<0x9F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"GO\t$X,$Y,$Z", (GOI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0xA0, !mul(i, 4));
  let mayStore = true in {
  def ST#s : Inst_X_Y_Z<opc,
                        (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
  defvar st = !cast<Inst_X_Y_Z>("ST"#s);
  def ST#s#I : Inst_X_Y_Z<!add(opc, 1),
                            (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
  def ST#s#U : Inst_X_Y_Z<!add(opc, 2),
                        (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
  def ST#s#UI : Inst_X_Y_Z<!add(opc, 3),
                        (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
  }
  def : InstAlias<"ST"#s#"\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("ST"#s#"I") GPR:$X, GPR:$Y, u8imm:$Z)>;
  def : InstAlias<"ST"#s#"U\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("ST"#s#"UI") GPR:$X, GPR:$Y, u8imm:$Z)>;
}

let mayStore = true in {
def STSF : Inst_X_Y_Z<0xB0, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STSFI : Inst_X_Y_Z<0xB1, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STSF\t$X,$Y,$Z", (STSFI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STHT : Inst_X_Y_Z<0xB2, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STHTI : Inst_X_Y_Z<0xB3, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STHT\t$X,$Y,$Z", (STHTI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STCO : Inst_X_Y_Z<0xB4, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def STCOI : Inst_X_Y_Z<0xB5, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STCO\t$X,$Y,$Z", (STCOI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STUNC : Inst_X_Y_Z<0xB6, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STUNCI : Inst_X_Y_Z<0xB7, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STUNC\t$X,$Y,$Z", (STUNCI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def SYNCD : Inst_X_Y_Z<0xB8, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def SYNCDI : Inst_X_Y_Z<0xB9, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"SYNCD\t$X,$Y,$Z", (SYNCDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

def PREST : Inst_X_Y_Z<0xBA, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def PRESTI : Inst_X_Y_Z<0xBB, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PREST\t$X,$Y,$Z", (PRESTI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def SYNCID : Inst_X_Y_Z<0xBC, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def SYNCIDI : Inst_X_Y_Z<0xBD, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"SYNCID\t$X,$Y,$Z", (SYNCIDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let isCall = true in {
def PUSHGO : Inst_X_Y_Z<0xBE, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def PUSHGOI : Inst_X_Y_Z<0xBF, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
let isAsmParserOnly = true in {
def : Inst_X_Y_Z<0xBE, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z), "PUSHGO">;
def PUSHGOIIdiosyncratic: Inst_X_Y_Z<0xBF, (outs),
  (ins u8imm:$X, GPR:$Y, u8imm:$Z), "PUSHGOI">;
}
}
def : InstAlias<"PUSHGO\t$X,$Y,$Z", (PUSHGOI GPR:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PUSHGO\t$X,$Y,$Z", (PUSHGOIIdiosyncratic u8imm:$X, GPR:$Y, u8imm:$Z)>;

def OR : Inst_X_Y_Z<0xC0, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ORI : Inst_X_Y_Z<0xC1, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"OR\t$X,$Y,$Z", (ORI GPR:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SET\t$X,$Y", (ORI GPR:$X, GPR:$Y, 0), 2>;

def ORN : Inst_X_Y_Z<0xC2, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ORNI : Inst_X_Y_Z<0xC3, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ORN\t$X,$Y,$Z", (ORNI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NOR : Inst_X_Y_Z<0xC4, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def NORI : Inst_X_Y_Z<0xC5, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"NOR\t$X,$Y,$Z", (NORI GPR:$X, GPR:$Y, u8imm:$Z)>;

def XOR : Inst_X_Y_Z<0xC6, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def XORI : Inst_X_Y_Z<0xC7, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"XOR\t$X,$Y,$Z", (XORI GPR:$X, GPR:$Y, u8imm:$Z)>;

def AND : Inst_X_Y_Z<0xC8, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ANDI : Inst_X_Y_Z<0xC9, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"AND\t$X,$Y,$Z", (ANDI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ANDN : Inst_X_Y_Z<0xCA, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def ANDNI : Inst_X_Y_Z<0xCB, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"ANDN\t$X,$Y,$Z", (ANDNI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NAND : Inst_X_Y_Z<0xCC, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def NANDI : Inst_X_Y_Z<0xCD, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"NAND\t$X,$Y,$Z", (NANDI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NXOR : Inst_X_Y_Z<0xCE, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def NXORI : Inst_X_Y_Z<0xCF, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"NXOR\t$X,$Y,$Z", (NXORI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0xD0, !mul(i, 2));
  def s#DIF : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
  def s#DIFI : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  defvar difi = !cast<Inst_X_Y_Z>(s#DIFI);
  def : InstAlias<i#"DIF\t$X,$Y,$Z", (difi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

def MUX : Inst_X_Y_Z<0xD8, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def MUXI : Inst_X_Y_Z<0xD9, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"MUX\t$X,$Y,$Z", (MUXI GPR:$X, GPR:$Y, u8imm:$Z)>;

def SADD : Inst_X_Y_Z<0xDA, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def SADDI : Inst_X_Y_Z<0xDB, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SADD\t$X,$Y,$Z", (SADDI GPR:$X, GPR:$Y, u8imm:$Z)>;

def MOR : Inst_X_Y_Z<0xDC, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def MORI : Inst_X_Y_Z<0xDD, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"MOR\t$X,$Y,$Z", (MORI GPR:$X, GPR:$Y, u8imm:$Z)>;

def MXOR : Inst_X_Y_Z<0xDE, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def MXORI : Inst_X_Y_Z<0xDF, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"MXOR\t$X,$Y,$Z", (MXORI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(RegPosList)) in {
  defvar pos = RegPosList[i];
  def SET#pos : Inst_X_YZ<!add(0xE0, i), (outs GPR:$X), (ins u16imm:$YZ)>;
  def INC#pos : Inst_X_YZ<!add(0xE4, i), (outs GPR:$X), (ins u16imm:$YZ)>;
  def OR#pos : Inst_X_YZ<!add(0xE8, i), (outs GPR:$X), (ins u16imm:$YZ)>;
  def ANDN#pos : Inst_X_YZ<!add(0xEC, i), (outs GPR:$X), (ins u16imm:$YZ)>;
}
def : InstAlias<"SET\t$X,$YZ", (SETL GPR:$X, u16imm:$YZ), 2>;

def JMP : InstJMP;
def : InstJMP<"JMPI"> {
  let isAsmParserOnly = true;
  let InOperandList = (ins s25imm:$XYZ);
}

let isCall = true in {
def PUSHJ : Inst_X_YZ<0xF2, (outs), (ins GPR:$X, b_dst:$YZ)>;
def PUSHJB : Inst_X_YZ<0xF3, (outs), (ins GPR:$X, b_dst:$YZ)> {
  let isCodeGenOnly = true;
}
let isAsmParserOnly = true in
def : Inst_X_YZ<0xF2, (outs), (ins u8imm:$X, b_dst:$YZ), "PUSHJ">;
}

def GETA : Inst_X_YZ<0xF4, (outs GPR:$X), (ins b_dst:$YZ)>;
def GETAB : Inst_X_YZ<0xF5, (outs GPR:$X), (ins b_dst:$YZ)> {
  let isCodeGenOnly = true;
}

class PutInst<bit isI, bit isIdiosyncratic>
  : MMIXInst<0xF6,
      !con((outs), !if(isIdiosyncratic, (outs), (outs SPR:$X))),
      !con((ins), !if(isIdiosyncratic, (ins spr_imm:$X), (ins)),
                      !if(isI, (ins u8imm:$Z), (ins GPR:$Z))),
            "PUT"#!if(isI, "I", ""), "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{24} = isI;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;

  let isAsmParserOnly = isIdiosyncratic;
}
def PUT : PutInst<false, false>;
def PUTI : PutInst<true, false>;
def : PutInst<false, true>;
def : PutInst<true, true>;
def : InstAlias<"PUT\t$X,$Z", (PUTI SPR:$X, u8imm:$Z)>;

let isReturn = true in
def POP : Inst_X_YZ<0xF8, (outs), (ins u8imm:$X, u16imm:$YZ)>;

def RESUME : MMIXInst<0xF9, (outs), (ins u8imm:$Z), "RESUME", "$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"RESUME", (RESUME 0)>;

def SAVE : MMIXInst<0xFA, (outs GPR:$X), (ins u8imm:$Z), "SAVE", "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;
}

def UNSAVE : MMIXInst<0xFB, (outs), (ins u8imm:$X, GPR:$Z), "UNSAVE", "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;
}
def : InstAlias<"UNSAVE\t$Z", (UNSAVE 0, GPR:$Z)>;

def SYNC : Inst_XYZ<0xFC, (outs), (ins u24imm:$XYZ)>;

def SWYM : Inst_X_Y_Z<0xFD, (outs), (ins u8imm:$X, u8imm:$Y, u8imm:$Z)>;
let isAsmParserOnly = true in {
def : Inst_X_YZ<0xFD, (outs), (ins u8imm:$X, u16imm:$YZ), "SWYM">;
def : Inst_XYZ<0xFD, (outs), (ins u24imm:$XYZ), "SWYM">;
}
def : InstAlias<"NOP", (SWYM 0, 0, 0), 0>;

class GetInst<bit isIdiosyncratic = false>
  : MMIXInst<0xFE,(outs GPR:$X),
    !if(isIdiosyncratic, (ins spr_imm:$Z), (ins SPR:$Z)), "GET", "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;

  let isAsmParserOnly = isIdiosyncratic;
}
def GET : GetInst;
def : GetInst<true>;

let isTrap = true in
def TRIP : Inst_X_Y_Z<0xFF, (outs), (ins u8imm:$X, u8imm:$Y, u8imm:$Z)>;
let isAsmParserOnly = true in {
def : Inst_X_YZ<0xFF, (outs), (ins u8imm:$X, u16imm:$YZ), "TRIP">;
def : Inst_XYZ<0xFF, (outs), (ins u24imm:$XYZ), "TRIP">;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD
