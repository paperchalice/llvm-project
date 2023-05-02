//===-- MMIXAsmParser.cpp - Parse MMIX assembly to MCInst instructions --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXAsmParser.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "MMIXInstrInfo.h"
#include "MMIXOperand.h"
#include "MMIXRegisterInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"

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
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "mmix-asm-parser"

#define GET_REGISTER_MATCHER
// #define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
// #define GET_MNEMONIC_SPELL_CHECKER
#define GET_MNEMONIC_CHECKER
#include "MMIXGenAsmMatcher.inc"

// return 0(false) means success;

// ctor
MMIXAsmParser::MMIXAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII,
                             const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII) {}

// interface implementation
bool MMIXAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (MatchResult) {
  default:
    return true;
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  }
  return false;
}

bool MMIXAsmParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  OperandMatchResultTy MatchResult = tryParseRegister(RegNo, StartLoc, EndLoc);
  return MatchResult != OperandMatchResultTy::MatchOperand_Success;
}

OperandMatchResultTy MMIXAsmParser::tryParseRegister(MCRegister &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {

  auto RegTok = getTok();
  Lex();                                         // eat synthesis register token
  RegNo = MatchRegisterName(RegTok.getString()); // impossible to fail
  return MatchOperand_Success;
}

bool MMIXAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  // First token is mnemonic
  if (!MMIXCheckMnemonic(Name, getAvailableFeatures(), 0)) {
    return true; // not a mnemonic, might be a label
  }

  // eat the mnemonic
  Operands.push_back(MMIXOperand::createMnemonic(Name, NameLoc));

  auto CurTok = getTok();

  while (getTok().isNot(AsmToken::EndOfStatement)) {
    if (getTok().is(AsmToken::Comma)) {
      // eat comma
      Lex();
    }

    // operand is always expression
    const MCExpr *Expr = nullptr;
    auto StartLoc = getTok().getLoc();
    bool HasError = getParser().parseExpression(Expr);
    auto EndLoc = getTok().getLoc();

    if (HasError)
      return true;
    else
      Operands.emplace_back(
          MMIXOperand::createExpression(Expr, StartLoc, EndLoc));
  }
  return false;
}

bool MMIXAsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {
  return false;
}

OperandMatchResultTy MMIXAsmParser::tryParseJumpDestOperand(
    OperandVector &Operands) {
  return MatchOperand_Success;
}

OperandMatchResultTy MMIXAsmParser::tryParseBranchDestOperand(
    OperandVector &Operands) {
  return MatchOperand_Success;
}

void llvm::MMIXAsmParser::resolveBaseAddress(MCInst &Inst,
                                             const OperandVector &Operands) {}

const MCExpr *MMIXAsmParser::createTargetUnaryExpr(
    const MCExpr *E, AsmToken::TokenKind OperatorToken, MCContext &Ctx) {
  return MMIXMCExpr::create(E, MMIXMCExpr::VK_MMIX_REG_EXPR, Ctx);
}

bool MMIXAsmParser::ParseDirective(AsmToken DirectiveID) { return false; }

/// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmParser() {
  RegisterMCAsmParser<MMIXAsmParser> AP1(getTheMMIXTarget());
}
