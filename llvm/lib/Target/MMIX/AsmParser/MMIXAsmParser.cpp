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
#define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
#define GET_MNEMONIC_CHECKER
#include "MMIXGenAsmMatcher.inc"

bool MMIXAsmParser::evaluateOperandExpr(const MCExpr *Expr, int64_t &Res,
                                        bool &IsRegister) {
  if (dyn_cast<MCTargetExpr>(Expr)) {
    IsRegister = true;
    return Expr->evaluateAsAbsolute(Res);
  }

  if (auto *BinExpr = dyn_cast<MCBinaryExpr>(Expr)) {
    bool IsRegLHS, IsRegRHS;
    int64_t ResLHS, ResRHS;
    auto EvalResL = evaluateOperandExpr(BinExpr->getLHS(), ResLHS, IsRegLHS);
    auto EvalResR = evaluateOperandExpr(BinExpr->getRHS(), ResRHS, IsRegRHS);
    if (EvalResL && EvalResR)
      return true;

    switch (BinExpr->getOpcode()) {
    default:
      return BinExpr->evaluateAsAbsolute(Res);
    case MCBinaryExpr::Add:
      Res = ResLHS + ResRHS;
      if (IsRegLHS && IsRegRHS)
        return Error(getTok().getLoc(), "cannot add two register numbers!");
      IsRegister = IsRegLHS || IsRegRHS;
      return false;
    case MCBinaryExpr::Sub:
      Res = ResLHS - ResRHS;
      if (IsRegLHS && IsRegRHS)
        IsRegister = false;
      else if (IsRegLHS && !IsRegRHS)
        IsRegister = true;
      else if (!IsRegLHS && IsRegRHS)
        return Error(getTok().getLoc(),
                     "cannot subtract register number from pure value!");
      else
        IsRegister = false;
      return false;
    case MCBinaryExpr::Div:
      if (BinExpr->getLoc().getPointer()[1] == '/') {
        APInt L(128, static_cast<uint64_t>(ResLHS));
        Res = L.udiv(static_cast<uint64_t>(ResRHS)).urem(1 << 24);
      } else {
        return BinExpr->evaluateAsAbsolute(Res);
      }
    }
  }
  return Expr->evaluateAsAbsolute(Res);
}

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
  StartLoc = getTok().getLoc();
  EndLoc = getTok().getEndLoc();
  if (getTok().getKind() == AsmToken::TokenKind::Identifier) {
    RegNo = MatchRegisterName(getTok().getIdentifier());
    LLVM_DEBUG(dbgs() << "parse token: "; getTok().dump(dbgs()));
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

bool MMIXAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  // First token is mnemonic
  if (!MMIXCheckMnemonic(Name, getAvailableFeatures(), 0)) {
    return true; // not a mnemonic, might be a label
  }
  Operands.push_back(MMIXOperand::createMnemonic(Name, NameLoc));

  auto CurTok = getTok();
  bool IsStrictMode = CurTok.is(AsmToken::Space);
  if (IsStrictMode) {
    Lex(); // eat the space
  }

  while (getTok().isNot(AsmToken::EndOfStatement)) {
    const MCExpr *Val = nullptr;
    int64_t Res = 0;
    bool IsRegister = false;
    getParser().parseExpression(Val);
    if (evaluateOperandExpr(Val, Res, IsRegister)) {
      return true;
    } else {
      getLexer().LexUntilEndOfStatement();
    }
  }

  return false;
}

// primary expression:
// <primary expression> -> <constant> | <symbol> | <local operand> | @ |
//                         <(expression)> | <unary operator><primary expression>
// <unary operator> -> + | - | ~ | $ | &
bool MMIXAsmParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc) {
  SMLoc FirstTokenLoc = getLexer().getLoc();

  auto CurTok = getTok();
  switch (CurTok.getKind()) {
  case AsmToken::Integer: {
    if (CurTok.getString()[0] != '#') {
      auto Suffix = getLexer().peekTok(false);
      // check local symbol
      if (Suffix.getString() == "F") {
        // not support
        return true;
      } else if (Suffix.getString() == "B") {
        // not support
        // local symbol
        Lex();
        Lex();
        return false;
      }
    }
  }
    Res = MCConstantExpr::create(getTok().getIntVal(), getContext());
    Lex();
    return false;
  case AsmToken::Identifier: {
    StringRef SymName;
    if (getParser().parseIdentifier(SymName)) {
      return Error(getTok().getLoc(), "Expect Identifier");
    } else {
      Res = MCSymbolRefExpr::create(getContext().getOrCreateSymbol(SymName),
                                    getContext());
    }
  } break;
  case AsmToken::At: {
    MCSymbol *Sym = getContext().createTempSymbol();
    Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
  }
    Lex(); // eat '@'
    return false;
  case AsmToken::LParen:
    Lex(); // Eat the '('.
    return getParser().parseParenExpression(Res, EndLoc);
  case AsmToken::Plus: // do nothing
    Lex();
    return parsePrimaryExpr(Res, EndLoc);
  case AsmToken::Minus: // subtract from zero
    Lex();
    {
      if (parsePrimaryExpr(Res, EndLoc)) {
        return true;
      } else {
        Res = MCUnaryExpr::createMinus(Res, getContext(), FirstTokenLoc);
        return false;
      }
    }
  case AsmToken::Tilde: // complement the bits
    Lex();              // eat ~
    {
      if (parsePrimaryExpr(Res, EndLoc)) {
        return true;
      } else {
        if (dyn_cast<MCSymbolRefExpr>(Res)) {
        }
        Res = MCUnaryExpr::createNot(Res, getContext(), FirstTokenLoc);
        return false;
      }
    }
    break;
  case AsmToken::Dollar: // change from pure value to register number
    // here leverage the target MCExpr to handle expressions which involve
    // registers
    Lex(); // eat $
    if (parsePrimaryExpr(Res, EndLoc)) {
      return true;
    }
    Res = MMIXMCExpr::create(Res, getContext());
    return false;
  case AsmToken::Amp: // take the serial number
    Lex();            // eat &
    {
      std::int64_t SerialNumber = 1;
      Res = MCConstantExpr::create(1, getContext());
      return false;
    }
    break;
  default:
    break;
  }
  return true;
}

bool MMIXAsmParser::ParseDirective(AsmToken DirectiveID) { return false; }

/// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmParser() {
  RegisterMCAsmParser<MMIXAsmParser> AP1(getTheMMIXTarget());
}
