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
  let mayRaiseFPException = true;
}

def FUN : Inst_X_Y_Z<0x02, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isCompare = true;
  let Pattern = [(set i64:$X, (setuo f64:$Y, f64:$Z))];
}

def FEQL : Inst_X_Y_Z<0x03, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isCompare = true;
}

def FADD : Inst_X_Y_Z<0x04, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let isAdd = true;
  let mayRaiseFPException = true;
  let Pattern = [(set f64:$X, (fadd f64:$Y, f64:$Z))];
}

def FIX : Inst_X_Y_Z<0x05, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
}
def : InstAlias<"FIX\t$X,$Z", (FIX GPR:$X, 0, GPR:$Z)>;

def FSUB : Inst_X_Y_Z<0x06, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
  let Pattern = [(set f64:$X, (fsub f64:$Y, f64:$Z))];
}

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
  let Pattern = [(set f64:$X, (fmul f64:$Y, f64:$Z))];
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
  let Pattern = [(set i64:$X, (setoeq f64:$Y, f64:$Z))];
}

def FDIV : Inst_X_Y_Z<0x14, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
  let Pattern = [(set f64:$X, (fdiv f64:$Y, f64:$Z))];
}

let mayRaiseFPException = true in
def FSQRT : Inst_X_Y_Z<0x15, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def : InstAlias<"FSQRT\t$X,$Z", (FSQRT GPR:$X, 0, GPR:$Z)>;

def FREM : Inst_X_Y_Z<0x16, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let mayRaiseFPException = true;
  let Pattern = [(set f64:$X, (frem f64:$Y, f64:$Z))];
}

def FINT : Inst_X_Y_Z<0x17, (outs GPR:$X), (ins rounding_mode:$Y, GPR:$Z)>;
def : InstAlias<"FINT\t$X,$Z", (FINT GPR:$X, 0, GPR:$Z)>;

let isCommutable = true in
defm MUL : BinOp<0x18, [(mul)]>;
def : InstAlias<"MUL\t$X,$Y,$Z", (MULI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rH], isCommutable = true in
defm MULU : BinOp<0x1A, [(op_nxw<(mul)>)]>;
def : InstAlias<"MULU\t$X,$Y,$Z", (MULUI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rR] in
defm DIV : BinOp<0x1C, [(sdiv)]>;
def : InstAlias<"DIV\t$X,$Y,$Z", (DIVI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs = [rR], Uses=[rD] in
defm DIVU : BinOp<0x1E, [(udiv)]>;
def : InstAlias<"DIVU\t$X,$Y,$Z", (DIVUI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs=[rA], isAdd = true, isCommutable = true in
defm ADD : BinOp<0x20, [(add)]>;
def : InstAlias<"ADD\t$X,$Y,$Z", (ADDI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isAdd = true, isCommutable = true in
defm ADDU : BinOp<0x22, [(op_nxw<(add)>)]>;
def : InstAlias<"ADDU\t$X,$Y,$Z", (ADDUI GPR:$X, GPR:$Y, u8imm:$Z)>;

let Defs=[rA] in
defm SUB : BinOp<0x24, [(sub)]>;
def : InstAlias<"SUB\t$X,$Y,$Z", (SUBI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm SUBU : BinOp<0x26, [(op_nxw<(sub)>)]>;
def : InstAlias<"SUBU\t$X,$Y,$Z", (SUBUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD2U : Inst_X_Y_Z<0x28, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 2), i64:$Z))];
}
def ADD2UI : Inst_X_Y_Z<0x29, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 2), (i64 u8imm:$Z)))];
}
def : InstAlias<"ADD2U\t$X,$Y,$Z", (ADD2UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD4U : Inst_X_Y_Z<0x2A, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 4), i64:$Z))];
}
def ADD4UI : Inst_X_Y_Z<0x2B, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 4), (i64 u8imm:$Z)))];
}
def : InstAlias<"ADD4U\t$X,$Y,$Z", (ADD4UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD8U : Inst_X_Y_Z<0x2C, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 8), i64:$Z))];
}
def ADD8UI : Inst_X_Y_Z<0x2D, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 8), (i64 u8imm:$Z)))];
}
def : InstAlias<"ADD8U\t$X,$Y,$Z", (ADD8UI GPR:$X, GPR:$Y, u8imm:$Z)>;

def ADD16U : Inst_X_Y_Z<0x2E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 16), i64:$Z))];
}
def ADD16UI : Inst_X_Y_Z<0x2F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (add (mul i64:$Y, 16), (i64 u8imm:$Z)))];
}
def : InstAlias<"ADD16U\t$X,$Y,$Z", (ADD16UI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isCompare = true in {
def CMP : Inst_X_Y_Z<0x30, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (scmp i64:$Y, i64:$Z))];
}
def CMPI : Inst_X_Y_Z<0x31, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (scmp i64:$Y, u8imm:$Z))];
}
}
def : InstAlias<"CMP\t$X,$Y,$Z", (CMPI GPR:$X, GPR:$Y, u8imm:$Z)>;

let isCompare = true in {
def CMPU : Inst_X_Y_Z<0x32, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (ucmp i64:$Y, i64:$Z))];
}
def CMPUI : Inst_X_Y_Z<0x33, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (ucmp i64:$Y, u8imm:$Z))];
}
}
def : InstAlias<"CMPU\t$X,$Y,$Z", (CMPUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def NEG : Inst_X_Y_Z<0x34, (outs GPR:$X), (ins u8imm:$Y, GPR:$Z)>;
def NEGI : Inst_X_Y_Z<0x35, (outs GPR:$X), (ins u8imm:$Y, u8imm:$Z)>;
def : InstAlias<"NEG\t$X,$Z", (NEG GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"NEG\t$X,$Z", (NEGI GPR:$X, 0, u8imm:$Z)>;
def : InstAlias<"NEG\t$X,$Y,$Z", (NEGI GPR:$X, u8imm:$Y, u8imm:$Z)>;

def NEGU : Inst_X_Y_Z<0x36, (outs GPR:$X), (ins u8imm:$Y, GPR:$Z)> {
  let Pattern = [(set i64:$X, (sub (i64 u8imm:$Y), i64:$Z))];
}
def NEGUI : Inst_X_Y_Z<0x37, (outs GPR:$X), (ins u8imm:$Y, u8imm:$Z)> {
  let Pattern = [(set i64:$X, (sub (i64 u8imm:$Y), (i64 u8imm:$Z)))];
}
def : InstAlias<"NEGU\t$X,$Z", (NEGU GPR:$X, 0, GPR:$Z)>;
def : InstAlias<"NEGU\t$X,$Z", (NEGUI GPR:$X, 0, u8imm:$Z)>;
def : InstAlias<"NEGU\t$X,$Y,$Z", (NEGUI GPR:$X, u8imm:$Y, u8imm:$Z)>;

defm SL : BinOp<0x38, [(shl)]>;
def : InstAlias<"SL\t$X,$Y,$Z", (SLI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm SLU : BinOp<0x3A, [(op_nxw<(shl)>)]>;
def : InstAlias<"SLU\t$X,$Y,$Z", (SLUI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm SR : BinOp<0x3C, [(sra)]>;
def : InstAlias<"SR\t$X,$Y,$Z", (SRI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm SRU : BinOp<0x3E, [(srl)]>;
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
  let isSelect = true, Constraints="$X = $X0" in {
    def CS#c : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z, GPR:$X0)>;
    def CS#c#I : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z, GPR:$X0)>;
  }
  defvar csi = !cast<Inst_X_Y_Z>("CS"#c#I);
  def : InstAlias<"CS"#c#"\t$X,$Y,$Z", (csi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

foreach i = !range(!size(CondList)) in {
  defvar c = CondList[i];
  defvar opc = !add(0x70, !mul(2, i));
  let isSelect = true in {
    def ZS#c : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
      let Pattern = [(set i64:$X, (select I64PosCondDagList[i], i64:$Z, 0))];
    }
    def ZS#c#I : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)> {
      let Pattern = [(set i64:$X, (select I64PosCondDagList[i], u8imm:$Z, 0))];
    }
  }
  defvar zsi = !cast<Inst_X_Y_Z>("ZS"#c#I);
  def : InstAlias<"ZS"#c#"\t$X,$Y,$Z", (zsi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0x80, !mul(i, 4));
  defvar width = !mul(!shl(1, i), 8);
  let mayLoad = true in {
  def LD#s : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
  def LD#s#I : Inst_X_Y_Z<!add(opc, 1), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  def LD#s#U : Inst_X_Y_Z<!add(opc, 2), (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
  def LD#s#UI : Inst_X_Y_Z<!add(opc, 3), (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
  }
  def : InstAlias<"LD"#s#"\t$X,$Y", (!cast<Inst_X_Y_Z>("LD"#s#I) GPR:$X, GPR:$Y, 0)>;
  def : InstAlias<"LD"#s#"U\t$X,$Y", (!cast<Inst_X_Y_Z>("LD"#s#"UI") GPR:$X, GPR:$Y, 0)>;
  def : InstAlias<"LD"#s#"\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("LD"#s#I) GPR:$X, GPR:$Y, u8imm:$Z)>;
  def : InstAlias<"LD"#s#"U\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("LD"#s#"UI") GPR:$X, GPR:$Y, u8imm:$Z)>;
}
let mayLoad = true in {
def LDSF : Inst_X_Y_Z<0x90, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDSFI : Inst_X_Y_Z<0x91, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDSF\t$X,$Y", (LDSFI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"LDSF\t$X,$Y,$Z", (LDSFI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDHT : Inst_X_Y_Z<0x92, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDHTI : Inst_X_Y_Z<0x93, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDHT\t$X,$Y", (LDHTI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"LDHT\t$X,$Y,$Z", (LDHTI GPR:$X, GPR:$Y, u8imm:$Z)>;

def : InstAlias<"LDA\t$X,$Y",    (ADDUI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"LDA\t$X,$Y,$Z", (ADDU GPR:$X, GPR:$Y, GPR:$Z)>;
def : InstAlias<"LDA\t$X,$Y,$Z", (ADDUI GPR:$X, GPR:$Y, u8imm:$Z)>;

def CSWAP : Inst_X_Y_Z<0x94, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def CSWAPI : Inst_X_Y_Z<0x95, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"CSWAP\t$X,$Y", (CSWAPI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"CSWAP\t$X,$Y,$Z", (CSWAPI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDUNC : Inst_X_Y_Z<0x96, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDUNCI : Inst_X_Y_Z<0x97, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDUNC\t$X,$Y", (LDUNCI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"LDUNC\t$X,$Y,$Z", (LDUNCI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def LDVTS : Inst_X_Y_Z<0x98, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def LDVTSI : Inst_X_Y_Z<0x99, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"LDVTS\t$X,$Y", (LDVTSI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"LDVTS\t$X,$Y,$Z", (LDVTSI GPR:$X, GPR:$Y, u8imm:$Z)>;

def PRELD : Inst_X_Y_Z<0x9A, (outs u8imm:$X), (ins GPR:$Y, GPR:$Z)>;
def PRELDI : Inst_X_Y_Z<0x9B, (outs u8imm:$X), (ins GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PRELD\t$X,$Y", (PRELDI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"PRELD\t$X,$Y,$Z", (PRELDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

def PREGO : Inst_X_Y_Z<0x9C, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def PREGOI : Inst_X_Y_Z<0x9D, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PREGO\t$X,$Y", (PREGOI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"PREGO\t$X,$Y,$Z", (PREGOI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let isCall = true, isIndirectBranch = true in {
def GO : Inst_X_Y_Z<0x9E, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)>;
def GOI : Inst_X_Y_Z<0x9F, (outs GPR:$X), (ins GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"GO\t$X,$Y", (GOI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"GO\t$X,$Y,$Z", (GOI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0xA0, !mul(i, 4));
  defvar width = !mul(!shl(1, i), 8);
  let mayStore = true in {
  def ST#s : Inst_X_Y_Z<opc,
                        (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
  def ST#s#I : Inst_X_Y_Z<!add(opc, 1),
                            (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
  def ST#s#U : Inst_X_Y_Z<!add(opc, 2),
                        (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)> {
    let Pattern = [(st !cast<VTInt>("i"#width):$X, (iPTR (add i64:$Y, i64:$Z)))];
  }
  def ST#s#UI : Inst_X_Y_Z<!add(opc, 3),
                        (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)> {
    let Pattern = [(st !cast<VTInt>("i"#width):$X, (iPTR (add i64:$Y, (i64 u8imm:$Z))))];
  }
  }
  def : InstAlias<"ST"#s#"\t$X,$Y", (!cast<Inst_X_Y_Z>("ST"#s#"I") GPR:$X, GPR:$Y, 0)>;
  def : InstAlias<"ST"#s#"U\t$X,$Y", (!cast<Inst_X_Y_Z>("ST"#s#"UI") GPR:$X, GPR:$Y, 0)>;
  def : InstAlias<"ST"#s#"\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("ST"#s#"I") GPR:$X, GPR:$Y, u8imm:$Z)>;
  def : InstAlias<"ST"#s#"U\t$X,$Y,$Z", (!cast<Inst_X_Y_Z>("ST"#s#"UI") GPR:$X, GPR:$Y, u8imm:$Z)>;
}

let mayStore = true in {
def STSF : Inst_X_Y_Z<0xB0, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STSFI : Inst_X_Y_Z<0xB1, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STSF\t$X,$Y", (STSFI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"STSF\t$X,$Y,$Z", (STSFI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STHT : Inst_X_Y_Z<0xB2, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STHTI : Inst_X_Y_Z<0xB3, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STHT\t$X,$Y", (STHTI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"STHT\t$X,$Y,$Z", (STHTI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STCO : Inst_X_Y_Z<0xB4, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)> {
  let Pattern = [(truncstorei8 u8imm:$X, (ptradd iPTR:$Y, i64:$Z))];
}
def STCOI : Inst_X_Y_Z<0xB5, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)> {
  let Pattern = [(truncstorei8 u8imm:$X, (ptradd iPTR:$Y, u8imm:$Z))];
}
}
def : InstAlias<"STCO\t$X,$Y", (STCOI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"STCO\t$X,$Y,$Z", (STCOI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let mayStore = true in {
def STUNC : Inst_X_Y_Z<0xB6, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z)>;
def STUNCI : Inst_X_Y_Z<0xB7, (outs), (ins GPR:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"STUNC\t$X,$Y", (STUNCI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"STUNC\t$X,$Y,$Z", (STUNCI GPR:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def SYNCD : Inst_X_Y_Z<0xB8, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def SYNCDI : Inst_X_Y_Z<0xB9, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"SYNCD\t$X,$Y", (SYNCDI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"SYNCD\t$X,$Y,$Z", (SYNCDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

def PREST : Inst_X_Y_Z<0xBA, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def PRESTI : Inst_X_Y_Z<0xBB, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PREST\t$X,$Y", (PRESTI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"PREST\t$X,$Y,$Z", (PRESTI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let mayLoad = true in {
def SYNCID : Inst_X_Y_Z<0xBC, (outs), (ins u8imm:$X, GPR:$Y, GPR:$Z)>;
def SYNCIDI : Inst_X_Y_Z<0xBD, (outs), (ins u8imm:$X, GPR:$Y, u8imm:$Z)>;
}
def : InstAlias<"SYNCID\t$X,$Y", (SYNCIDI u8imm:$X, GPR:$Y, 0)>;
def : InstAlias<"SYNCID\t$X,$Y,$Z", (SYNCIDI u8imm:$X, GPR:$Y, u8imm:$Z)>;

let isCall = true in {
defm PUSHGO : BinOp<0xBE>;
let isAsmParserOnly = true in {
def : Inst_X_Y_Z<0xBE, (outs), (ins GPR:$X, GPR:$Y, GPR:$Z), "PUSHGO">;
def PUSHGOIIdiosyncratic: Inst_X_Y_Z<0xBF, (outs),
  (ins u8imm:$X, GPR:$Y, u8imm:$Z), "PUSHGOI">;
}
}
def : InstAlias<"PUSHGO\t$X,$Y", (PUSHGOI GPR:$X, GPR:$Y, 0)>;
def : InstAlias<"PUSHGO\t$X,$Y,$Z", (PUSHGOI GPR:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"PUSHGO\t$X,$Y,$Z", (PUSHGOIIdiosyncratic u8imm:$X, GPR:$Y, u8imm:$Z)>;

defm OR : BinOp<0xC0, [(or)]>;
def : InstAlias<"OR\t$X,$Y,$Z", (ORI GPR:$X, GPR:$Y, u8imm:$Z)>;
def : InstAlias<"SET\t$X,$Y", (ORI GPR:$X, GPR:$Y, 0), 2>;

defm ORN : BinOp<0xC2, [(orn)]>;
def : InstAlias<"ORN\t$X,$Y,$Z", (ORNI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm NOR : BinOp<0xC4, [(nor)]>;
def : InstAlias<"NOR\t$X,$Y,$Z", (NORI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm XOR : BinOp<0xC6, [(xor)]>;
def : InstAlias<"XOR\t$X,$Y,$Z", (XORI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm AND : BinOp<0xC8, [(and)]>;
def : InstAlias<"AND\t$X,$Y,$Z", (ANDI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm ANDN : BinOp<0xCA, [(andn)]>;
def : InstAlias<"ANDN\t$X,$Y,$Z", (ANDNI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm NAND : BinOp<0xCC, [(nand)]>;
def : InstAlias<"NAND\t$X,$Y,$Z", (NANDI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm NXOR : BinOp<0xCE, [(nxor)]>;
def : InstAlias<"NXOR\t$X,$Y,$Z", (NXORI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(SizeSuffixList)) in {
  defvar s = SizeSuffixList[i];
  defvar opc = !add(0xD0, !mul(i, 2));
  defvar tys = [v8i8, v4i16, v2i32, i64];
  defvar imms = [u8imm_v8i8, u8imm_v4i16, u8imm_v2i32, u8imm];
  def s#DIF : Inst_X_Y_Z<opc, (outs GPR:$X), (ins GPR:$Y, GPR:$Z)> {
    let Pattern = [(set tys[i]:$X, (usubsat tys[i]:$Y, tys[i]:$Z))];
  }
  def s#DIFI : Inst_X_Y_Z<!or(opc, 1), (outs GPR:$X), (ins GPR:$Y, imms[i]:$Z)> {
    let Pattern = [(set tys[i]:$X, (usubsat tys[i]:$Y, imms[i]:$Z))];
  }
  defvar difi = !cast<Inst_X_Y_Z>(s#DIFI);
  def : InstAlias<i#"DIF\t$X,$Y,$Z", (difi GPR:$X, GPR:$Y, u8imm:$Z)>;
}

let Uses = [rM] in
defm MUX : BinOp<0xD8>;
def : InstAlias<"MUX\t$X,$Y,$Z", (MUXI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm SADD : BinOp<0xDA>;
def : InstAlias<"SADD\t$X,$Y,$Z", (SADDI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm MOR : BinOp<0xDC>;
def : InstAlias<"MOR\t$X,$Y,$Z", (MORI GPR:$X, GPR:$Y, u8imm:$Z)>;

defm MXOR : BinOp<0xDE>;
def : InstAlias<"MXOR\t$X,$Y,$Z", (MXORI GPR:$X, GPR:$Y, u8imm:$Z)>;

foreach i = !range(!size(RegPosList)) in {
  defvar pos = RegPosList[i];
  def SET#pos : Inst_X_YZ<!add(0xE0, i), (outs GPR:$X), (ins u16imm:$YZ)>;
  let Constraints = "$X = $X0" in {
    def INC#pos : Inst_X_YZ<!add(0xE4, i), (outs GPR:$X), (ins GPR:$X0, u16imm:$YZ)>;
    def OR#pos : Inst_X_YZ<!add(0xE8, i), (outs GPR:$X), (ins GPR:$X0, u16imm:$YZ)>;
    def ANDN#pos : Inst_X_YZ<!add(0xEC, i), (outs GPR:$X), (ins GPR:$X0, u16imm:$YZ)>;
  }
}
def : InstAlias<"SET\t$X,$YZ", (SETL GPR:$X, u16imm:$YZ), 2>;

def JMP : InstJMP {
  let isBranch = 1;
  let isTerminator = 1;
  let Pattern = [(br bb:$XYZ)];
}

let isCall = true in {
def PUSHJ : InstB_X_YZ<0xF2, (outs), (ins GPR:$X, rel_addr:$YZ)>;
let isAsmParserOnly = true in
def : Inst_X_YZ<0xF2, (outs), (ins u8imm:$X, rel_addr:$YZ), "PUSHJ">;
}

def GETA : InstB_X_YZ<0xF4, (outs GPR:$X), (ins rel_addr:$YZ)>;

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

include "MMIXInstrCompiler.td"

//===----------------------------------------------------------------------===//
// Instruction Patterns
//===----------------------------------------------------------------------===//

def : Pat<(i64 (ctpop i64:$Y)), (SADDI GPR:$Y, 0)>;
def : Pat<(i64 (ctpop (and i64:$Y, i64:$mask))), (SADD GPR:$Y, (NORI GPR:$mask, 0))>;

def : Pat<(f64 (fsqrt f64:$Z)), (FSQRT 0, GPR:$Z)>;

def : Pat<(i64 (bitconvert v8i8:$src)), (i64 GPR:$src)>;
def : Pat<(v8i8 (bitconvert i64:$src)), (v8i8 GPR:$src)>;

foreach i = !range(!size(SizeSuffixList)) in {
  defvar SizeSuffix = SizeSuffixList[i];
  defvar SizeVTInt = SizeVTIntList[i];
  defvar LDXI = !cast<Inst_X_Y_Z>("LD"#SizeSuffix#"I");
  defvar LDX = !cast<Inst_X_Y_Z>("LD"#SizeSuffix);
  defvar LDXUI = !cast<Inst_X_Y_Z>("LD"#SizeSuffix#"UI");
  defvar LDXU = !cast<Inst_X_Y_Z>("LD"#SizeSuffix#"U");
  defvar STXUI = !cast<Inst_X_Y_Z>("ST"#SizeSuffix#"UI");
  defvar STXU = !cast<Inst_X_Y_Z>("ST"#SizeSuffix#"U");

  // use signed just because it has a shorter mnemonic...
  // the cost of signed and unsigned load is equal
  // load with index
  defvar XExtLoads = ["extload", "sextload", "zextload"];
  defvar LDXXI = [LDXI, LDXI, LDXUI];
  defvar LDXX = [LDX, LDX, LDXU];
  foreach j = !range(!size(XExtLoads)) in {
    defvar xextload = !cast<PatFrag>(XExtLoads[j]#SizeVTInt);
    def : Pat<(i64 (xextload iPTR:$Y)), (LDXXI[j] GPR:$Y, 0)>;
    def : Pat<(i64 (xextload (ptradd iPTR:$Y, u8imm:$offset))),
              (LDXXI[j] GPR:$Y, u8imm:$offset)>;
    def : Pat<(i64 (xextload (ptradd iPTR:$Y, i64:$offset))),
              (LDXX[j] GPR:$Y, GPR:$offset)>;
  }

  def : Pat<(!cast<PatFrag>(truncstore#SizeVTInt) i64:$X, (ptradd iPTR:$Y, u8imm:$offset)),
            (STXUI GPR:$X, GPR:$Y, u8imm:$offset)>;
  def : Pat<(!cast<PatFrag>(truncstore#SizeVTInt) i64:$X, (ptradd iPTR:$Y, i64:$offset)),
            (STXU GPR:$X, GPR:$Y, GPR:$offset)>;
}
// LDO and LDOU have the same effect, but they have different opcodes
def : Pat<(i64 (load iPTR:$Y)), (LDOI GPR:$Y, 0)>;
def : Pat<(i64 (load (ptradd iPTR:$Y, u8imm:$offset))), (LDOI GPR:$Y, u8imm:$offset)>;
def : Pat<(store i64:$X, iPTR:$Y), (STOI GPR:$X, GPR:$Y, 0)>;

def : Pat<(ptradd iPTR:$ptr, u8imm:$addend), (ADDU GPR:$ptr, u8imm:$addend)>;
def : Pat<(ptradd iPTR:$ptr, i64:$addend), (ADDUI GPR:$ptr, GPR:$addend)>;

defvar ICMPConds = ["ugt", "uge", "ult", "ule", "eq", "gt", "ge", "lt", "le", "ne"];
defvar CMPConds = ["P", "NN", "N", "NP", "Z", "P", "NN", "N", "NP", "NZ"];
def : Pat<(brcond i64:$X, bb:$YZ), (BP GPR:$X, bb:$YZ)>;
def : Pat<(select i64:$Y, u8imm:$Z, 0), (ZSPI GPR:$Y, u8imm:$Z)>;
def : Pat<(select i64:$Y, i64:$Z, 0), (ZSP GPR:$Y, GPR:$Z)>;
def : Pat<(select i64:$Y, u8imm:$t, i64:$f), (CSNZI GPR:$Y, u8imm:$t, GPR:$f)>;
def : Pat<(select i64:$Y, i64:$t, i64:$f), (CSNZ GPR:$Y, GPR:$t, GPR:$f)>;
foreach i = !range(!size(ICMPConds)) in {
  defvar BCond = CMPConds[i];
  defvar BInst = !cast<InstB>("B"#BCond);
  defvar SetCond = ICMPConds[i];
  defvar CMPInst = !cast<Inst_X_Y_Z>(!if(!match(SetCond, "u"), "CMPU", "CMP"));
  defvar SetInst = !cast<PatFrag>("set"#SetCond);
  defvar ZSInst = !cast<Inst_X_Y_Z>("ZS"#BCond#"I");
  defvar CSInst = !cast<Inst_X_Y_Z>("CS"#BCond);
  defvar CSIInst = !cast<Inst_X_Y_Z>("CS"#BCond#"I");

  def : Pat<(brcond (i64 (SetInst i64:$lhs, i64:$rhs)), bb:$YZ),
            (BInst (CMPInst GPR:$lhs, GPR:$rhs), bb:$YZ)>;
  def : Pat<(i64 (SetInst i64:$lhs, i64:$rhs)), (ZSInst (CMPInst GPR:$lhs, GPR:$rhs), 1)>;
  def : Pat<(select (i64 (SetInst i64:$lhs, i64:$rhs)), i64:$t, i64:$f),
            (CSInst (CMPInst GPR:$lhs, GPR:$rhs), GPR:$t, GPR:$f)>;
  def : Pat<(select (i64 (SetInst i64:$lhs, i64:$rhs)), u8imm:$t, i64:$f),
            (CSIInst (CMPInst GPR:$lhs, GPR:$rhs), u8imm:$t, GPR:$f)>;
}

// floating points
def : Pat<(i64 (setueq f64:$Y, f64:$Z)), (OR (FUN GPR:$Y, GPR:$Z), (FEQL GPR:$Y, GPR:$Z))>;
def : Pat<(i64 (setune f64:$Y, f64:$Z)), (OR (FUN GPR:$Y, GPR:$Z), (XORI (FEQL GPR:$Y, GPR:$Z), 1))>;
def : Pat<(i64 (seto f64:$Y, f64:$Z)), (XORI (FUN GPR:$Y, GPR:$Z), 1)>;

include "MMIXInstrGISel.td"

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD
