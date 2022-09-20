//===-- MMIXAsmParser.cpp - Parse MMIX assembly to MCInst instructions --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/MMIXTargetInfo.h"
#include "MMIXAsmParser.h"
#include "MMIXRegisterInfo.h"
#include "MMIXInstrInfo.h"
#include "MMIXOperand.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "mmix-asm-parser"

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "MMIXGenAsmMatcher.inc"

// return 0(false) means success;

// ctor
MMIXAsmParser::MMIXAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII,
                             const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII) {

}

// interface implementation
bool MMIXAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                        OperandVector &Operands, MCStreamer &Out,
                        uint64_t &ErrorInfo,
                        bool MatchingInlineAsm) {
  return false;
}

bool MMIXAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  OperandMatchResultTy MatchResult = tryParseRegister(RegNo, StartLoc, EndLoc);
  return MatchResult != OperandMatchResultTy::MatchOperand_Success;
}

OperandMatchResultTy MMIXAsmParser::tryParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) {
  StartLoc = getTok().getLoc();
  EndLoc = getTok().getEndLoc();
  if (getTok().getKind() == AsmToken::TokenKind::Identifier) {
    RegNo = MatchRegisterName(getTok().getIdentifier());
    LLVM_DEBUG(dbgs() << "parse token: ";
                    getTok().dump(dbgs()));
  } else {
    RegNo = 0;
  }
  if (RegNo) {
    Lex(); // consume the register token
    return OperandMatchResultTy::MatchOperand_Success;
  } else {
    return OperandMatchResultTy::MatchOperand_NoMatch;
  }
}

bool MMIXAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc,
                      OperandVector &Operands) {
  return false;
}

bool MMIXAsmParser::ParseDirective(AsmToken DirectiveID) { return false; }

/// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmParser() {
  RegisterMCAsmParser<MMIXAsmParser> AP1(getTheMMIXTarget());
}
