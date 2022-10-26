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

/*
* Byte s(M[$Y + $Z]) or s(M[$Y + Z]) is loaded into register X as a signed number between -128 and +127,
  inclusive.
*/

class getSuffix<int size> {
  string Ret = !cond(!eq(size,  8): "B",
                  !eq(size,  16): "W",
                  !eq(size,  32): "T",
                  !eq(size,  64): "O",
                  true: "");
}

class catOp<string s1, int s2> {
  SDPatternOperator Ret = !cast<SDPatternOperator>(s1 # s2);
}

// Loading and storing
multiclass LoadInst<bits<8> opcode, string opstr,
  SDPatternOperator op1, SDPatternOperator op2> {
  defm "": InstVariantUI<opcode, opstr,
  [(set i64:$X, (op1 (add i64:$Y, i64:$Z)))],
  [(set i64:$X, (op1 (add i64:$Y, (zext i8imm:$Z))))],
  [(set i64:$X, (op2 (add i64:$Y, i64:$Z)))],
  [(set i64:$X, (op2 (add i64:$Y, (zext i8imm:$Z))))]
  >;
}

multiclass StoreInst<bits<8> opcode, string opstr,
  SDPatternOperator op> {
  defm "": InstVariantUI<opcode, opstr,
  [(op i64:$X, (add i64:$Y, i64:$Z))],
  [(op i64:$X, (add i64:$Y, (zext i8imm:$Z)))],
  [(op i64:$X, (add i64:$Y, i64:$Z))],
  [(op i64:$X, (add i64:$Y, (zext i8imm:$Z)))]
  >;
}

foreach i = {3...6} in {
  defvar Width = !shl(1, i);
  defvar s = getSuffix<Width>.Ret;
  defvar LDOpcode = !add(0x80, !mul(!sub(i, 3), 4));

  defm LD # s: LoadInst<LDOpcode, "LD" # s,
  catOp<"sextloadi", Width>.Ret,
  catOp<"zextloadi", Width>.Ret
  >;

  defvar STOpcode = !add(0xA0, !mul(!sub(i, 3), 4));
  defm ST # s:StoreInst<STOpcode, "ST" # s, catOp<"truncstorei", Width>.Ret>;
}

defm LDHT: InstVariantI<0x92, "LDHT",
  [(set GPR:$X, (shl (i64 (extloadi32 (add i64:$Y, i64:$Z))), (i64 32)))],
  [(set GPR:$X, (shl (i64 (extloadi32 (add i64:$Y, (zext i8imm:$Z)))), (i64 32)))]>;
defm STHT: InstVariantI<0xB2, "STHT",
  [(truncstorei32 (srl i64:$X, (i64 32)), (add i64:$Y, i64:$Z))],
  [(truncstorei32 (srl i64:$X, (i64 32)), (add i64:$Y, (zext i8imm:$Z)))]>;

// Adding and subtracting
multiclass ArithInst<bits<8> opcode, string opstr, SDPatternOperator op> {
  defm "": InstVariantUI<opcode, opstr,
  [(set i64:$X, (op i64:$Y, i64:$Z))],
  [(set i64:$X, (op i64:$Y, (sext i8imm:$Z)))],
  [(set i64:$X, (op i64:$Y, i64:$Z))],
  [(set i64:$X, (op i64:$Y, (zext i8imm:$Z)))]>;
}
defm ADD: ArithInst<0x20, "ADD", add>;
foreach i = {1...4} in {
  defvar Width = !shl(1, i);
  defvar Opcode = !add(0x28, !mul(!sub(i, 1), 2));
  defm ADDU # Width: InstVariantI<Opcode, "ADDU" # Width,
  [(set i64:$X, (add (mul i64:$Y, Width), i64:$Z))],
  [(set i64:$X, (add (mul i64:$Y, Width), (zext i8imm:$Z)))]>;
}
defm SUB: ArithInst<0x24, "SUB", sub>;
def NEG: MMIXInst<0x34, (outs GPR:$X), (ins i8imm:$Y, GPR:$Z), "NEG", "$X, $Y, $Z",
[(set i64:$X, (sub (sext i8imm:$Y), i64:$Z))]>;
def NEGI: MMIXInst<0x35, (outs GPR:$X), (ins i8imm:$Y, i8imm:$Z), "NEG", "$X, $Y, $Z",
[(set i64:$X, (sub (sext i8imm:$Y), (sext i8imm:$Z)))]>;
def NEGU: MMIXInst<0x36, (outs GPR:$X), (ins i8imm:$Y, GPR:$Z), "NEG", "$X, $Y, $Z",
[(set i64:$X, (sub (zext i8imm:$Y), i64:$Z))]>;
def NEGUI: MMIXInst<0x37, (outs GPR:$X), (ins i8imm:$Y, i8imm:$Z), "NEG", "$X, $Y, $Z",
[(set i64:$X, (sub (zext i8imm:$Y), (zext i8imm:$Z)))]>;

// Bit fiddling
multiclass InstX<bits<8> opcode, string opstr, SDPatternOperator op> {
  defm "": InstVariantI<opcode, opstr,
    [(set i64:$X, (op i64:$Y, i64:$Z))],
    [(set i64:$X, (op i64:$Y, (zext i8imm:$Z)))]>;
}
multiclass InstX2<bits<8> opcode, string opstr, SDPatternOperator op1, SDPatternOperator op2> {
  defm "": InstVariantI<opcode, opstr,
    [(set i64:$X, (op1 (op2 i64:$Y, i64:$Z)))],
    [(set i64:$X, (op1 (op2 i64:$Y, (zext i8imm:$Z))))]>;
}
defm AND: InstX<0xC8, "AND", and>;
defm OR: InstX<0xC0, "OR", or>;
defm XOR: InstX<0xC6, "XOR", xor>;
defm ANDN: InstVariantI<0xCA, "ANDN",
    [(set i64:$X, (and i64:$Y, (not i64:$Z)))],
    [(set i64:$X, (and i64:$Y, (zext (xor i8imm:$Z, (i8 -1)))))]>;
defm ORN: InstVariantI<0xC2, "ORN",
    [(set i64:$X, (or i64:$Y, (not i64:$Z)))],
    [(set i64:$X, (or i64:$Y, (zext (xor i8imm:$Z, (i8 -1)))))]>;
defm NAND: InstX2<0xCC, "NAND", not, and>;
defm NOR: InstX2<0xC4, "NOR", not, or>;
defm NXOR: InstX2<0xCE, "NXOR", not, xor>;
let Uses = [rM] in
defm MUX: InstVariantI<0xD8, "MUX",
  [(set i64:$X, (or (and i64:$Y, rM), (and i64:$Z, rM)))],
  [(set i64:$X, (or (and i64:$Y, rM), (and (zext i8imm:$Z), rM)))]>;
foreach i = {3} in {
  defvar Width = !shl(1, i);
  defvar Suffix = getSuffix<Width>.Ret;
  defvar Opcode = !add(0xD0, !mul(2, !sub(i, 3)));
  defm Suffix # "DIF":InstVariantI<Opcode, Suffix # "DIF",
  [(set v8i8:$X, (usubsat (v8i8 (bitconvert i64:$Y)), (v8i8 (bitconvert i64:$Z))))],
  [(set v8i8:$X, (usubsat (v8i8 (bitconvert i64:$Y)), (v8i8 (bitconvert (zext i8imm:$Z)))))]>;
}

defm SADD: InstVariantI<0xDA, "SADD",
  [(set i64:$X, (ctpop (and i64:$Y, i64:$Z)))],
  [(set i64:$X, (ctpop (and i64:$Y, (zext i8imm:$Z))))]>;
defm MOR: InstVariantI<0xDC, "MOR",
  [],
  []>;
defm MXOR: InstVariantI<0xDE, "MXOR",
  [],
  []>;

// Wyde instructions
class getPos<int i> {
  string Ret = !cond(!eq(i,  0): "L",
                  !eq(i,  1): "ML",
                  !eq(i,  2): "MH",
                  !eq(i,  3): "H",
                  true: "");
}
foreach i = {0...3} in {
  defvar SetOpcode = !add(0xE0, !sub(3, i));
  defvar Suffix = getPos<i>.Ret;
  def SET # Suffix: InstX_YZ<SetOpcode, "SET" # Suffix,
  [(set i64:$X, (shl (zext i16imm:$YZ), (i64 !mul(i, 16))))]>;

  defvar IncOpcode = !add(0xE4, !sub(3, i));
  def INC # Suffix: MMIXInst<IncOpcode,
  (outs), (ins GPR:$X, i16imm:$YZ), "INC" # Suffix, "$X, $YZ",
  []>;
  
  defvar OrOpcode = !add(0xE8, !sub(3, i));
  def OR # Suffix: MMIXInst<OrOpcode,
  (outs), (ins GPR:$X, i16imm:$YZ), "OR" # Suffix, "$X, $YZ",
  []>;

  defvar AndOpcode = !add(0xEC, !sub(3, i));
  def AND # Suffix: MMIXInst<AndOpcode,
  (outs), (ins GPR:$X, i16imm:$YZ), "AND" # Suffix, "$X, $YZ",
  []>;
}

// shift
defm SL: ArithInst<0x38, "SL", shl>;
defm SR: InstVariantI<0x3C, "SR",
[(set i64:$X, (sra i64:$Y, i64:$Z))],
[(set i64:$X, (sra i64:$Y, i8imm:$Z))]>;
defm SRU: InstVariantI<0x3E, "SRU",
[(set i64:$X, (sra i64:$Y, i64:$Z))],
[(set i64:$X, (sra i64:$Y, i8imm:$Z))]>;

// Comparisons
defm CMP: InstVariantUI<0x30, "CMP", [], [], [], []>;

defm CSN: InstVariantI<0x60, "CSN", [], []>;
defm CSZ: InstVariantI<0x62, "CSZ", [], []>;
defm CSP: InstVariantI<0x64, "CSP", [], []>;
defm CSOD: InstVariantI<0x66, "CSOD", [], []>;
defm CSNN: InstVariantI<0x68, "CSNN", [], []>;
defm CSNZ: InstVariantI<0x6A, "CSNZ", [], []>;
defm CSNP: InstVariantI<0x6C, "CSNP", [], []>;
defm CSEV: InstVariantI<0x6E, "CSEV", [], []>;

defm ZSN: InstVariantI<0x70, "ZSN", [], []>;
defm ZSZ: InstVariantI<0x72, "ZSZ", [], []>;
defm ZSP: InstVariantI<0x74, "ZSP", [], []>;
defm ZSOD: InstVariantI<0x76, "ZSOD", [], []>;
defm ZSNN: InstVariantI<0x78, "ZSNN", [], []>;
defm ZSNZ: InstVariantI<0x7A, "ZSNZ", [], []>;
defm ZSNP: InstVariantI<0x7C, "ZSNP", [], []>;
defm ZSEV: InstVariantI<0x7E, "ZSEV", [], []>;

// Branches and jumps
multiclass InstVariantB<bits<8> opcode, string opstr,
             list<dag> patterns> {
  def "": InstX_YZ<opcode, opstr, patterns>;
  def B: InstX_YZ<!add(opcode, 1), opstr # "B", patterns>;
}

defm BN:  InstVariantB<0x40, "BN",  []>;
defm BZ:  InstVariantB<0x42, "BZ",  []>;
defm BP:  InstVariantB<0x44, "BP",  []>;
defm BOD: InstVariantB<0x46, "BOD", []>;
defm BNN: InstVariantB<0x48, "BNN", []>;
defm BNZ: InstVariantB<0x4A, "BNZ", []>;
defm BNP: InstVariantB<0x4C, "BNP", []>;
defm BEV: InstVariantB<0x4E, "BEV", []>;

defm PBN:  InstVariantB<0x50, "PBN",  []>;
defm PBZ:  InstVariantB<0x52, "PBZ",  []>;
defm PBP:  InstVariantB<0x54, "PBP",  []>;
defm PBOD: InstVariantB<0x56, "PBOD", []>;
defm PBNN: InstVariantB<0x58, "PBNN", []>;
defm PBNZ: InstVariantB<0x5A, "PBNZ", []>;
defm PBNP: InstVariantB<0x5C, "PBNP", []>;
defm PBEV: InstVariantB<0x5E, "PBEV", []>;

def GETA: InstVariantB<0xF4, "GETA", []>;
#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD
