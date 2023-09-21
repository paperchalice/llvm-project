//===-- MMIXInstrFormats.td - MMIX Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD

def InstEnc : GenericEnum {
  let FilterClass = "InstEncEnum";
  let NameField = "Name";
  let ValueField = "Encoding";
}

class InstEncEnum<bits<8> enc> {
  string Name = NAME;
  bits<8> Encoding = enc;
}

class MMIXInst<bits<8> opcode, dag oops, dag iops, string opstr, string argstr>
    : Instruction, InstEncEnum<opcode> {
  field bits<32> Inst; // Instruction encoding.
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;
  let Namespace = "MMIX";
  let Size = 4;
  let AsmString = opstr #"\t" #argstr;

  let OutOperandList = oops;
  let InOperandList = iops;

  let Pattern = [];
  let Inst{31...24} = opcode;

  let HasPositionOrder = true;
}

//===----------------------------------------------------------------------===//
// Operands
//===----------------------------------------------------------------------===//

def SFRAsmOperand : AsmOperandClass {
  let Name = "SpecialRegister";
  let ParserMethod = "parseSFR";
  let PredicateMethod = "isReg";
  let RenderMethod = "addRegOperands";
}

def MemAsmOperand : AsmOperandClass {
  let Name = "JumpDest";
  let PredicateMethod = "isMem";
  let ParserMethod = "parseMemOperand";
  #ifdef GEN_MMIXAL
  let RenderMethod = "addMemOperands";
  #else
  let RenderMethod = "addImmOperands";
  #endif
}

def RoundModeAsmOperand : AsmOperandClass {
  let Name = "RoundMode";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidRoundMode";
  let PredicateMethod = "isRoundMode";
}

//

def SFR : RegisterOperand<SPR> { let ParserMatchClass = SFRAsmOperand; }

def jump_dest : Operand<OtherVT> { let ParserMatchClass = MemAsmOperand; }

def branch_dest : Operand<i16> { let ParserMatchClass = MemAsmOperand; }

class UImmAsmOperand<int width> : AsmOperandClass {
  let Name = "UImm" #width;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
  let PredicateMethod = "isUImm<" #width #">";
}

let OperandType = "OPERAND_IMMEDIATE" in {
  def ui8imm : Operand<i64>,
               ImmLeaf<i64, [{ return isUInt<8>(Imm) && Imm > 0; }]> {
    let ParserMatchClass = UImmAsmOperand<8>;
    let DecoderMethod = "decodeUImmOperand<8>";
  }

  def ui16imm : Operand<i64>,
                ImmLeaf<i64, [{ return isUInt<16>(Imm) && Imm >= 0; }]> {
    let ParserMatchClass = UImmAsmOperand<16>;
    let DecoderMethod = "decodeUImmOperand<16>";
  }

  def ui24imm : Operand<i64>,
                ImmLeaf<i64, [{ return isUInt<24>(Imm) && Imm >= 0; }]> {
    let ParserMatchClass = UImmAsmOperand<24>;
    let DecoderMethod = "decodeUImmOperand<24>";
  }

  def round_mode : Operand<i8>,
                   ImmLeaf<i64, [{ return 5 > Imm && Imm >= 0; }]> {
    let ParserMatchClass = RoundModeAsmOperand;
    let DecoderMethod = "decodeRoundMode";
  }
}

def base_address : Operand<iPTR> {
  let MIOperandInfo = (ops GPR, ui8imm);
  let ParserMatchClass = MemAsmOperand;
}

class Inst_X_Y_Z<bits<8> opcode, dag oops, dag iops, string opstr>
    : MMIXInst<opcode, oops, iops, opstr, "$X,$Y,$Z"> {
  bits<8> X;
  bits<8> Y;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = Y;
  let Inst{7...0} = Z;
}

class Inst_X_YZ<bits<8> opcode, dag oops, dag iops, string opstr>
    : MMIXInst<opcode, oops, iops, opstr, "$X,$YZ"> {
  bits<8> X;
  bits<16> YZ;

  let Inst{23...16} = X;
  let Inst{15...0} = YZ;
}

class Inst_X__Z<bits<8> opcode, dag oops, dag iops, string opstr>
    : MMIXInst<opcode, oops, iops, opstr, "$X,$Z"> {
  bits<8> X;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = 0;
  let Inst{7...0} = Z;
}

class Inst_XYZ<bits<8> opcode, dag oops, dag iops, string opstr>
    : MMIXInst<opcode, oops, iops, opstr, "$XYZ"> {
  bits<24> XYZ;

  let Inst{23...0} = XYZ;
}

class Inst_Z<bits<8> opcode, dag oops, dag iops, string opstr>
    : MMIXInst<opcode, oops, iops, opstr, "$Z"> {
  bits<8> Z;

  let Inst{23...8} = 0;
  let Inst{7...0} = Z;
}

class Inst_r_rr<bits<8> opcode, string opstr = NAME> : Inst_X_Y_Z<opcode,
                                                           (outs GPR
                                                            : $X),
                                                           (ins GPR
                                                            : $Y, GPR
                                                            : $Z),
                                                           opstr>;

class Inst__rrr<bits<8> opcode, string opstr> : Inst_X_Y_Z<opcode, (outs),
                                                           (ins GPR
                                                            : $X, GPR
                                                            : $Y, GPR
                                                            : $Z),
                                                           opstr>;

class Inst_r_ri<bits<8> opcode, string opstr> : Inst_X_Y_Z<opcode,
                                                           (outs GPR
                                                            : $X),
                                                           (ins GPR
                                                            : $Y, ui8imm
                                                            : $Z),
                                                           opstr>;

class Inst_r_ir<bits<8> opcode, string opstr> : Inst_X_Y_Z<opcode,
                                                           (outs GPR
                                                            : $X),
                                                           (ins ui8imm
                                                            : $Y, GPR
                                                            : $Z),
                                                           opstr>;

class Inst_r_ii<bits<8> opcode, string opstr> : Inst_X_Y_Z<opcode,
                                                           (outs GPR
                                                            : $X),
                                                           (ins ui8imm
                                                            : $Y, ui8imm
                                                            : $Z),
                                                           opstr>;

class Inst__rri<bits<8> opcode, string opstr> : Inst_X_Y_Z<opcode, (outs),
                                                           (ins GPR
                                                            : $X, GPR
                                                            : $Y, ui8imm
                                                            : $Z),
                                                           opstr>;

class Inst_r_roundz<bits<8> opcode, string opstr = NAME> : Inst_X_Y_Z<opcode,
                                                               (outs GPR
                                                                : $X),
                                                               (ins round_mode
                                                                : $Y, GPR
                                                                : $Z),
                                                               opstr>;

class Inst_J<bits<8> opcode, dag oops, dag iops, string opstr> : MMIXInst<opcode, oops, iops, opstr, "$X,$YZ"> {
  let isAsmParserOnly = true;
  bits<8> X;
  bits<8> Y;
  bits<8> Z;

  let Inst{23...16} = X;
  let Inst{15...8} = Y;
  let Inst{7...0} = Z;
#ifdef GEN_MMIXAL
  let AsmMatchConverter = "resolveBaseAddress";
#endif
}

multiclass Inst_r_roundzi<bits<8> opcode, string opstr = NAME> {
  let mayRaiseFPException = true, Defs = [rA] in {
    def "" : Inst_X_Y_Z<opcode,
                        (outs GPR
                         : $X),
                        (ins round_mode
                         : $Y, GPR
                         : $Z),
                        opstr>;
    def I : Inst_X_Y_Z<!or(opcode, 1),
                       (outs GPR
                        : $X),
                       (ins round_mode
                        : $Y, ui8imm
                        : $Z),
                       opstr>;
  }
  let EmitPriority = 1 in {
  def:
    InstAlias<opstr #"\t$X,$Z", (!cast<Inst_X_Y_Z>(opstr) GPR
                                 : $X, 0, GPR
                                 : $Z)>;
  def:
    InstAlias<opstr #"\t$X,$Z", (!cast<Inst_X_Y_Z>(opstr #"I") GPR
                                 : $X, 0, ui8imm
                                 : $Z)>;
  }
}

multiclass Inst_trip_trap<bits<8> opcode, string opstr = NAME> {
  let isTrap = true in def "" : Inst_XYZ<opcode, (outs),
                                         (ins ui24imm
                                          : $XYZ),
                                         opstr>;
  let isAsmParserOnly = true in {
    def i8i16 : Inst_X_YZ<opcode, (outs),
                          (ins ui8imm
                           : $X, ui16imm
                           : $YZ),
                          opstr>;
    def ri16 : Inst_X_YZ<opcode, (outs), (ins GPR : $X, ui16imm : $YZ), opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs),
               (ins ui8imm
                : $X, ui8imm
                : $Y, ui8imm
                : $Z),
               opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs), (ins GPR : $X, ui8imm : $Y, ui8imm : $Z), opstr>;
    def iri : Inst_X_Y_Z<opcode, (outs),
                         (ins ui8imm
                          : $X, GPR
                          : $Y, ui8imm
                          : $Z),
                         opstr>;
    def iir : Inst_X_Y_Z<opcode, (outs),
                         (ins ui8imm
                          : $X, ui8imm
                          : $Y, GPR
                          : $Z),
                         opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs), (ins GPR : $X, GPR : $Y, ui8imm : $Z), opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs), (ins GPR : $X, ui8imm : $Y, GPR : $Z), opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs), (ins ui8imm : $X, GPR : $Y, GPR : $Z), opstr>;
  def:
    Inst_X_Y_Z<opcode, (outs), (ins GPR : $X, GPR : $Y, GPR : $Z), opstr>;
  }
def:
  InstAlias<opstr #"$X,$Z", (!cast<Inst_X_Y_Z>(opstr #"iir") ui8imm
                             : $X, 0, GPR
                             : $Z)>;
}

class Inst_r_ui16imm<bits<8> opcode, string opstr = NAME> : Inst_X_YZ<opcode,
                                                               (outs GPR
                                                                : $X),
                                                               (ins GPR
                                                                : $X0, ui16imm
                                                                : $YZ),
                                                               opstr> {
  let Constraints = "$X = $X0";
}

class Inst_jump<bits<8> opcode, string opstr> : Inst_XYZ<opcode, (outs),
                                                         (ins jump_dest
                                                          : $XYZ),
                                                         opstr> {
  let isBranch = true;
}

multiclass Inst_r_rri<bits<8> opcode, bit base = false, string opstr = NAME> {
  def "" : Inst_r_rr<opcode, opstr>;
  def I : Inst_r_ri<!or(opcode, 1), opstr>;

  if base
    then {
      // FIXME: tablegen behavior related
      def J : Inst_J<!or(opcode, 1),
                            (outs GPR
                             : $X),
                            (ins (base_address $Y, $Z): $YZ),
                            opstr>;
    }
}

multiclass Inst_r_iri<bits<8> opcode, string opstr> {
  def "" : Inst_r_ir<opcode, opstr>;
  def I : Inst_r_ii<!or(opcode, 1), opstr>;
}

multiclass Inst_i_rri<bits<8> opcode, bit base = false, string opstr = NAME> {
  def "" : Inst_X_Y_Z<opcode,
                      (outs ui8imm
                       : $X),
                      (ins GPR
                       : $Y, GPR
                       : $Z),
                      opstr>;
  def I : Inst_X_Y_Z<!or(opcode, 1),
                     (outs ui8imm
                      : $X),
                     (ins GPR
                      : $Y, ui8imm
                      : $Z),
                     opstr>;

  if base
    then {
      def J : Inst_J<!or(opcode, 1),
                            (outs GPR
                             : $X),
                            (ins (base_address $Y, $Z)
                             : $YZ),
                            opstr> {
        let isAsmParserOnly = true;
#ifdef GEN_MMIXAL
        let AsmMatchConverter = "resolveBaseAddress";
#endif
      }
    }
}

multiclass Inst__rrri<bits<8> opcode, bit base = false, string opstr = NAME> {
  def "" : Inst__rrr<opcode, opstr>;
  def I : Inst__rri<!or(opcode, 1), opstr>;

  if base
    then {
      def J : Inst_J<!or(opcode, 1),
                            (outs GPR
                             : $X),
                            (ins (base_address $Y, $Z)
                             : $YZ),
                            opstr> {
        let isAsmParserOnly = true;
#ifdef GEN_MMIXAL
        let AsmMatchConverter = "resolveBaseAddress";
#endif
      }
    }
}

multiclass Inst__irri<bits<8> opcode, bit base = false, string opstr = NAME> {
  def "" : Inst_X_Y_Z<opcode, (outs),
                      (ins ui8imm
                       : $X, GPR
                       : $Y, GPR
                       : $Z),
                      opstr>;
  def I : Inst_X_Y_Z<!or(opcode, 1), (outs),
                     (ins ui8imm
                      : $X, GPR
                      : $Y, ui8imm
                      : $Z),
                     opstr>;

  if base
    then {
      def J : Inst_X_YZ<!or(opcode, 1), (outs),
                            (ins ui8imm
                             : $X, (base_address $Y, $Z)
                             : $YZ),
                            opstr> {
        let isAsmParserOnly = true;
      }
    }
}

multiclass Inst__rbranch<bits<8> opcode, string opstr = NAME> {
  let isBranch = true in {
    def "" : Inst_X_YZ<opcode, (outs),
                       (ins GPR
                        : $X, branch_dest
                        : $YZ),
                       opstr>;
    def B : Inst_X_YZ<!or(opcode, 1), (outs),
                      (ins GPR
                       : $X, branch_dest
                       : $YZ),
                      opstr #"B">;
  }
}

multiclass Inst__push<bits<8> opcode, string opstr = NAME> {
  let isBranch = true in {
    def "" : Inst_X_YZ<opcode, (outs GPR:$X),
                       (ins branch_dest
                        : $YZ),
                       opstr>;
    def B : Inst_X_YZ<!or(opcode, 1), (outs GPR:$X),
                      (ins branch_dest
                       : $YZ),
                      opstr #"B">;
  }
}

multiclass Inst__ibranch<bits<8> opcode, string opstr = !substr(NAME, 0, 5)> {
  let isBranch = true in {
    def "" : Inst_X_YZ<opcode, (outs),
                       (ins ui8imm
                        : $X, branch_dest
                        : $YZ),
                       opstr>;
    def B : Inst_X_YZ<!or(opcode, 1), (outs),
                      (ins ui8imm
                       : $X, branch_dest
                       : $YZ),
                      opstr #"B">;
  }
}

multiclass Inst_r_branch<bits<8> opcode, string opstr = NAME> {
  def "" : Inst_X_YZ<opcode, (outs GPR : $X), (ins branch_dest : $YZ), opstr>;
  def B : Inst_X_YZ<!or(opcode, 1),
                    (outs GPR
                     : $X),
                    (ins branch_dest
                     : $YZ),
                    opstr #"B">;
}

multiclass Inst_jmpb<bits<8> opcode, string opstr = NAME> {
  def "" : Inst_jump<opcode, opstr>;
  def B : Inst_jump<!or(opcode, 1), opstr #"B">;
}
#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRFORMATS_TD
