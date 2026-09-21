//===-- ALParser.cpp - Parse MMIX assembly language -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXALParser.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;
using namespace llvm::MMIX;

ALParser::ALParser(MCContext &Ctx, MCStreamer &Out, SourceMgr &SrcMgr,
                   const MCAsmInfo &MAI)
    : MCAsmParser(Ctx, Out, SrcMgr, MAI) {
  Lexer.setBuffer(SrcMgr.getMemoryBuffer(SrcMgr.getMainFileID())->getBuffer());
}

const ALToken &ALParser::lex() {
  if (Lexer.getTok().is(ALToken::Error))
    Error(Lexer.getErrLoc(), Lexer.getErr());

  // if it's a end of statement with a comment in it
  if (getTok().is(ALToken::EndOfStatement)) {
    // if this is a line comment output it.
    if (!getTok().getString().empty() && getTok().getString().front() != '\n' &&
        getTok().getString().front() != '\r' && MAI.preserveAsmComments())
      Out.addExplicitComment(Twine(getTok().getString()));
  }

  const ALToken &Tok = Lexer.Lex();

  return Tok;
}

void ALParser::Note(SMLoc L, const Twine &Msg, SMRange Range) {
  printPendingErrors();
  printMessage(L, SourceMgr::DK_Note, Msg, Range);
}
bool ALParser::Warning(SMLoc L, const Twine &Msg, SMRange Range) {
  if (getTargetParser().getTargetOptions().MCNoWarn)
    return false;
  if (getTargetParser().getTargetOptions().MCFatalWarnings)
    return Error(L, Msg, Range);
  printMessage(L, SourceMgr::DK_Warning, Msg, Range);
  return false;
}
bool ALParser::printError(SMLoc L, const Twine &Msg, SMRange Range) {
  HadError = true;
  printMessage(L, SourceMgr::DK_Error, Msg, Range);
  return true;
}

bool ALParser::parseIdentifier(StringRef &Res) {
  auto CurTok = getTok();
  if (CurTok.isNot(ALToken::Identifier))
    return true;

  auto TokStr = CurTok.getString();
  if (isdigit(TokStr[0])) {
    auto Suffix = TokStr.drop_while(isdigit);
    if (Suffix.size() != 1) {
      Error(CurTok.getLoc(), "invalid local label!");
      return true;
    }
    switch (Suffix[0]) {
    case 'B':
    case 'F':
    case 'H':
      break;
    default:
      Error(CurTok.getLoc(), "invalid local label!");
      return true;
      break;
    }
  }
  Res = CurTok.getString();
  Lex();
  return false;
}

bool ALParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  Res = nullptr;

  // if (parsePrimaryExpr(Res, EndLoc, nullptr) || parseBinOpRHS(1, Res,
  // EndLoc))
  //   return true;
  return false;
}

bool ALParser::parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  if (parseExpression(Res, EndLoc)) {
    return false;
  }
  return parseRParen();
}

// primary expression:
// <primary expression> -> <constant> | <symbol> | <local operand> | @ |
//                         <(expression)> | <unary operator><primary
//                         expression>
// <unary operator> -> + | - | ~ | $ | &
bool ALParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                                AsmTypeInfo *TypeInfo) {
  return false;
}

bool ALParser::parseStatement() { return false; }

bool ALParser::Run(bool NoInitialTextSection, bool NoFinalize) {
  // init section
  Out.initSections(getTargetParser().getSTI());
  // Prime the lexer.
  auto T = Lex();
  getTargetParser().onBeginOfFile();

  while (getTok().isNot(ALToken::Eof)) {
    ErrorCount += parseStatement();
  }

  if (auto Rem = DataCounter % 4) {
    Out.emitZeros(4 - Rem);
  }

  // if (auto MainSymbol =
  //         dyn_cast<MCSymbolMMO>(getContext().getOrCreateSymbol(":Main"))) {
  //   *SharedInfo.GregList.rbegin() = MainSymbol->getEquivalent();
  // }

  getTargetParser().onEndOfFile();

  printPendingErrors();

  // emitPostamble();

  Out.finish(getLexer().getLoc());
  if (ErrorCount == 1) {
    errs() << "(One error were found.)\n";
  } else if (ErrorCount > 1) {
    errs() << '(' << ErrorCount << " error were found.)\n";
  }
  return ErrorCount;
}

/// Create an ALParser instance.
ALParser *llvm::MMIX::createMCMMIXALParser(SourceMgr &SrcMgr, MCContext &Ctx,
                                           MCStreamer &Out,
                                           const MCAsmInfo &MAI) {
  return new ALParser(Ctx, Out, SrcMgr, MAI);
}
