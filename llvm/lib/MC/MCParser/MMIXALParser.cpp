//===- MMOAsmParser.cpp - MMO Assembly Parser ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/DebugInfo/CodeView/SymbolRecord.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeView.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/AsmCond.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Unicode.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <deque>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

using CodePoint = UTF32;

// MMIXAL Lexer, assume input is UTF8
class MMIXALLexer : public MCAsmLexer {

private:
  const MCAsmInfo &MAI;

  const char *CurPtr = nullptr;
  StringRef CurBuf;
  bool IsAtStartOfLine = true;
  bool IsAtStartOfStatement = true;
  bool IsPeeking = false;
  bool EndStatementAtEOF = true;

  bool isAtStartOfComment(const char *Ptr) { return isalnum(*Ptr); }

  int getUCS(const char *Ptr) {
    int UCS = 0;
    return UCS;
  }

  bool isAtStatementSeparator(const char *Ptr) {
    return strncmp(Ptr, ";", 1) == 0;
  }

  UTF32 getNextChar() {
    if (CurPtr == CurBuf.end())
      return EOF;
    
    auto Num = getNumBytesForUTF8(*CurPtr);

    return (unsigned char)*CurPtr++;
  }

  int peekNextChar() {
    if (CurPtr == CurBuf.end())
      return EOF;
    return (unsigned char)*CurPtr;
  }

  AsmToken ReturnError(const char *Loc, const std::string &Msg) {
    SetError(SMLoc::getFromPointer(Loc), Msg);

    return AsmToken(AsmToken::Error, StringRef(Loc, CurPtr - Loc));
  }

  AsmToken LexIdentifier();
  AsmToken LexSlash();
  AsmToken LexLineComment() {
    // Mark This as an end of statement with a body of the
    // comment. While it would be nicer to leave this two tokens,
    // backwards compatability with TargetParsers makes keeping this in this
    // form better.
    const UTF8* Start = reinterpret_cast<const UTF8*>(TokStart);
    const auto CommentTextStart = CurPtr;
    int CurChar = getNextChar();
    while (CurChar != '\n' && CurChar != '\r' && CurChar != EOF)
      CurChar = getNextChar();
    const auto NewlinePtr = CurPtr;
    if (CurChar == '\r' && CurPtr != CurBuf.end() && *CurPtr == '\n')
      ++CurPtr;

    IsAtStartOfLine = true;
    // This is a whole line comment. leave newline
    if (IsAtStartOfStatement)
      return AsmToken(AsmToken::EndOfStatement,
                      StringRef(TokStart, CurPtr - TokStart));
    IsAtStartOfStatement = true;

    return AsmToken(AsmToken::EndOfStatement,
                    StringRef(TokStart, CurPtr - 1 - TokStart));
  }

  AsmToken LexDigit();
  AsmToken LexSingleQuote();
  AsmToken LexQuote();
  AsmToken LexFloatLiteral();
  AsmToken LexHexFloatLiteral(bool NoIntDigits);

  StringRef LexUntilEndOfLine() {
    TokStart = CurPtr;

    while (*CurPtr != '\n' && *CurPtr != '\r' && CurPtr != CurBuf.end()) {
      ++CurPtr;
    }
    return StringRef(TokStart, CurPtr - TokStart);
  }

protected:
  /// LexToken - Read the next token and return its code.
  AsmToken LexToken() override {
    TokStart = CurPtr;
    // This always consumes at least one character.
    int CurChar = getNextChar();

    // handle label and '#'
    if (!IsPeeking && IsAtStartOfLine && IsAtStartOfStatement) {
      if (CurChar == '#') {
        // If this starts with a '#', this may be a cpp
        // hash directive and otherwise a line comment.
        AsmToken TokenBuf[2];
        MutableArrayRef<AsmToken> Buf(TokenBuf, 2);
        size_t num = peekTokens(Buf, true);
        // There cannot be a space preceding this
        if (IsAtStartOfLine && num == 2 && TokenBuf[0].is(AsmToken::Integer) &&
            TokenBuf[1].is(AsmToken::String)) {
          CurPtr = TokStart; // reset curPtr;
          StringRef Directive = LexUntilEndOfLine();
          UnLex(TokenBuf[1]);
          UnLex(TokenBuf[0]);
          return AsmToken(AsmToken::HashDirective, Directive);
        }
      }
    }

    if (isAtStartOfComment(TokStart))
      return LexLineComment();

    if (isAtStatementSeparator(TokStart)) {
      IsAtStartOfLine = true;
      IsAtStartOfStatement = true;
      return AsmToken(AsmToken::EndOfStatement, StringRef(TokStart, 1));
    }

    // If we're missing a newline at EOF, make sure we still get an
    // EndOfStatement token before the Eof token.
    if (CurChar == EOF && !IsAtStartOfStatement && EndStatementAtEOF) {
      IsAtStartOfLine = true;
      IsAtStartOfStatement = true;
      return AsmToken(AsmToken::EndOfStatement, StringRef(TokStart, 0));
    }
    IsAtStartOfLine = false;
    bool OldIsAtStartOfStatement = IsAtStartOfStatement;
    IsAtStartOfStatement = false;

    switch (CurChar) {
    default:
    case '\n':
      IsAtStartOfLine = true;
      IsAtStartOfStatement = true;
      return AsmToken(AsmToken::EndOfStatement, StringRef(TokStart, 1));
    }

    return AsmToken();
  }

public:
  MMIXALLexer(const MCAsmInfo &MAI) : MAI(MAI) { setSkipSpace(false); }
  MMIXALLexer(const AsmLexer &) = delete;
  MMIXALLexer &operator=(const AsmLexer &) = delete;
  ~MMIXALLexer() override = default;

  void setBuffer(StringRef Buf, const char *ptr = nullptr,
                 bool EndStatementAtEOF = true) {
    CurBuf = Buf;

    if (ptr)
      CurPtr = ptr;
    else
      CurPtr = CurBuf.begin();

    TokStart = nullptr;
    this->EndStatementAtEOF = EndStatementAtEOF;
  }

  StringRef LexUntilEndOfStatement() override {
    TokStart = CurPtr;

    while (!isAtStartOfComment(CurPtr) &&     // Start of line comment.
           !isAtStatementSeparator(CurPtr) && // End of statement marker.
           *CurPtr != '\n' && *CurPtr != '\r' && CurPtr != CurBuf.end()) {
      ++CurPtr;
    }
    return StringRef(TokStart, CurPtr - TokStart);
  }
  size_t peekTokens(MutableArrayRef<AsmToken> Buf,
                    bool ShouldSkipSpace = true) override {
    SaveAndRestore SavedTokenStart(TokStart);
    SaveAndRestore SavedCurPtr(CurPtr);
    SaveAndRestore SavedAtStartOfLine(IsAtStartOfLine);
    SaveAndRestore SavedAtStartOfStatement(IsAtStartOfStatement);
    SaveAndRestore SavedSkipSpace(SkipSpace, ShouldSkipSpace);
    SaveAndRestore SavedIsPeeking(IsPeeking, true);
    std::string SavedErr = getErr();
    SMLoc SavedErrLoc = getErrLoc();

    size_t ReadCount;
    for (ReadCount = 0; ReadCount < Buf.size(); ++ReadCount) {
      AsmToken Token = LexToken();

      Buf[ReadCount] = Token;

      if (Token.is(AsmToken::Eof))
        break;
    }

    SetError(SavedErrLoc, SavedErr);
    return ReadCount;
  }

  const MCAsmInfo &getMAI() const { return MAI; }
};

// parser part

class MMIXALParser : public MCAsmParser {
private:
  AsmLexer Lexer;
  MCContext &Ctx;
  MCStreamer &Out;
  const MCAsmInfo &MAI;
  SourceMgr &SrcMgr;
  SourceMgr::DiagHandlerTy SavedDiagHandler;
  void *SavedDiagContext;
  std::unique_ptr<MCAsmParserExtension> PlatformParser;

  /// This is the current buffer index we're lexing from as managed by the
  /// SourceMgr object.
  unsigned CurBuffer;

public:
  MMIXALParser(SourceMgr &SM, MCContext &Ctx, MCStreamer &Out,
               const MCAsmInfo &MAI, unsigned CB)
      : Lexer(MAI), Ctx(Ctx), Out(Out), MAI(MAI), SrcMgr(SM),
        CurBuffer(CB ? CB : SM.getMainFileID()) {
    Lexer.setBuffer(SrcMgr.getMemoryBuffer(CurBuffer)->getBuffer());
    // follow mmixal behavior
    Lexer.setSkipSpace(false);
  }

public:
  void addDirectiveHandler(StringRef Directive,
                           ExtensionDirectiveHandler Handler) override {}
  void addAliasForDirective(StringRef Directive, StringRef Alias) override {}
  bool Run(bool NoInitialTextSection, bool NoFinalize = false) override;

  void setParsingMSInlineAsm(bool V) override {}
  bool isParsingMSInlineAsm() override { return false; }

  /// @name MCAsmParser Interface
  /// {

  SourceMgr &getSourceManager() override { return SrcMgr; }
  MCAsmLexer &getLexer() override { return Lexer; }
  MCContext &getContext() override { return Ctx; }
  MCStreamer &getStreamer() override { return Out; }
  const AsmToken &Lex() override;
  bool parseIdentifier(StringRef &Res) override { return false; }
  StringRef parseStringToEndOfStatement() override { return ""; }
  bool parseEscapedString(std::string &Data) override { return false; }
  bool parseAngleBracketString(std::string &Data) override { return false; }
  void eatToEndOfStatement() override {}
  bool parseExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                        AsmTypeInfo *TypeInfo) override;
  bool parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parseAbsoluteExpression(int64_t &Res) override { return false; }
  bool checkForValidSection() override { return false; }
  bool parseParenExprOfDepth(unsigned ParenDepth, const MCExpr *&Res,
                             SMLoc &EndLoc) override {
    return false;
  }
  /// }

  /// Parse MS-style inline assembly.
  bool parseMSInlineAsm(std::string &AsmString, unsigned &NumOutputs,
                        unsigned &NumInputs,
                        SmallVectorImpl<std::pair<void *, bool>> &OpDecls,
                        SmallVectorImpl<std::string> &Constraints,
                        SmallVectorImpl<std::string> &Clobbers,
                        const MCInstrInfo *MII, const MCInstPrinter *IP,
                        MCAsmParserSemaCallback &SI) override {
    return true;
  }

  void Note(SMLoc L, const Twine &Msg, SMRange Range = std::nullopt) override {}
  bool Warning(SMLoc L, const Twine &Msg,
               SMRange Range = std::nullopt) override {
    return false;
  }
  bool printError(SMLoc L, const Twine &Msg,
                  SMRange Range = std::nullopt) override {
    return true;
  }

private:
  AsmToken lexIdentifier();
  void lexLispComment();
  AsmToken lexUntilLineEnd();
  bool parseTerm(const MCExpr *&Res, SMLoc &EndLoc);
  bool parseLine();
};

AsmToken MMIXALParser::lexIdentifier() {
  auto CurTok = getLexer().getTok();
  if (CurTok.is(AsmToken::Identifier) || CurTok.is(AsmToken::Colon)) {
    auto TokBegin = CurTok.getString().begin();
    auto Sz = CurTok.getString().size();
    while (getLexer().peekTok(false).is(AsmToken::Colon) ||
           getLexer().peekTok(false).is(AsmToken::Identifier)) {
      Sz += getLexer().Lex().getString().size();
    }
    Lex(); // to next token
    return AsmToken(AsmToken::Identifier, StringRef(TokBegin, Sz));
  } else {
    return AsmToken(AsmToken::Error, "");
  }
}

const AsmToken &MMIXALParser::Lex() { return getLexer().Lex(); }

bool MMIXALParser::parseLine() {
  // line: ^[label]\s+<instruction>[;\s*<instruction>]\s+[arbitray contents]$
  // if we get error, discard all rest content until line end
  auto CurTok = getTok();
  getLexer().setSkipSpace(true);
  AsmToken Label;
  if (getTok().is(AsmToken::Space)) {
    // no label
    Label = getTok();
    Lex();
  } else {
    // we have label
    if (getTok().is(AsmToken::Colon) || getTok().is(AsmToken::Identifier)) {
      CurTok = lexIdentifier();
    } else {
      // invalid label
      lexUntilLineEnd();
    }
  }

  // now parse instruction
  // instruction: <opcode> <operands> [;]
  CurTok = getTok();
  ParseInstructionInfo IInfo;
  SmallVector<std::unique_ptr<MCParsedAsmOperand>, 4> ParsedOperands;
  while (getTok().isNot(AsmToken::EndOfStatement) &&
         getTok().isNot(AsmToken::Eof)) {
    auto Mnemonic = getTok().getString().lower(); // follow mmix doc
    bool HasErr = getTargetParser().ParseInstruction(IInfo, Mnemonic, CurTok,
                                                     ParsedOperands);
    if (HasErr) {
      lexUntilLineEnd();
      return false;
    }
  }
  getLexer().setSkipSpace(false);
  return false;
}

// <term> -> <primary expression> | <term><strong operator><primary expression>
bool MMIXALParser::parseTerm(const MCExpr *&Res, SMLoc &EndLoc) {
  auto CurTok = getTok();

  // <primary expression>
  if (parsePrimaryExpr(Res, EndLoc, nullptr)) {
    // <term><strong operator><primary expression>
    if (parseTerm(Res, EndLoc)) {
      return true;
    } else {
      auto LHS = Res;
      bool IsSlashSlash = false;
      CurTok = getTok();
      if (CurTok.is(AsmToken::Slash)) {
        if (getLexer().peekTok(false).is(AsmToken::Slash)) {
          Lex();
          Lex();
          IsSlashSlash = true;
        }
      } else {
        Lex();
      }
      const MCExpr *RHS = nullptr;
      if (parsePrimaryExpr(RHS, EndLoc, nullptr)) {
        return true;
      }

      switch (CurTok.getKind()) {
      case AsmToken::Star:
        Res = MCBinaryExpr::createMul(LHS, RHS, getContext());
        return false;
      case AsmToken::Slash:
        if (IsSlashSlash) {
          Res = MCBinaryExpr::createDiv(LHS, RHS, getContext());
        } else {
          Res = MCBinaryExpr::createDiv(LHS, RHS, getContext());
        }
        return false;
      case AsmToken::Percent:
        Res = MCBinaryExpr::createMod(LHS, RHS, getContext());
        return false;
      case AsmToken::LessLess:
        Res = MCBinaryExpr::createShl(LHS, RHS, getContext());
        return false;
      case AsmToken::GreaterGreater:
        Res = MCBinaryExpr::createAShr(LHS, RHS, getContext());
        return false;
      case AsmToken::Amp:
        Res = MCBinaryExpr::createAnd(LHS, RHS, getContext());
        return false;
      default:
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

// <expression> -> <term> | <expression> <weak operator> <term>
bool MMIXALParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  if (parseTerm(Res, EndLoc)) {
    if (parseExpression(Res, EndLoc)) {
      return true;
    } else {
      auto Op = getTok();
      auto LHS = Res;
      const MCExpr *RHS = nullptr;
      Lex(); // eat op
      switch (Op.getKind()) {
      case AsmToken::Plus:
        Res = MCBinaryExpr::createAdd(LHS, RHS, getContext());
        return false;
      case AsmToken::Minus:
        Res = MCBinaryExpr::createSub(LHS, RHS, getContext());
        return false;
      case AsmToken::Pipe:
        Res = MCBinaryExpr::createOr(LHS, RHS, getContext());
        return false;
      default:
        return true;
      }
    }
  } else {
    return false;
  }
}

bool MMIXALParser::Run(bool NoInitialTextSection, bool NoFinalize) {
  // Prime the lexer.
  auto T = Lex();
  getTargetParser().onBeginOfFile();
  T.dump(outs());
  outs() << "\n";
  while (getLexer().isNot(AsmToken::Eof)) {
    Lex().dump(outs());
    outs() << "\n";
  }
  return true;

  // While we have input, parse each statement.
  auto CurTok = getTok();
  while (Lexer.isNot(AsmToken::Eof)) {
    parseLine();
  }
  return true;
}

AsmToken MMIXALParser::lexUntilLineEnd() {
  auto CurTok = getTok();
  while (CurTok.getString() != "\n" && CurTok.isNot(AsmToken::Eof)) {
    CurTok = Lex();
  }
  return Lex();
}

// primary expression:
// <primary expression> -> <constant> | <symbol> | <local operand> | @ |
//                         <(expression)> | <unary operator><primary expression>
// <unary operator> -> + | - | ~ | $ | &
bool MMIXALParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                                    AsmTypeInfo *TypeInfo) {
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
  case AsmToken::Identifier:
  case AsmToken::At: {
    MCSymbol *Sym = Ctx.createTempSymbol();
    Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
  }
    Lex(); // eat '@'
    return false;
  case AsmToken::LParen:
    Lex(); // Eat the '('.
    return parseParenExpression(Res, EndLoc);
  case AsmToken::Plus: // do nothing
    Lex();
    return parsePrimaryExpr(Res, EndLoc, TypeInfo);
  case AsmToken::Minus: // subtract from zero
    Lex();
    {
      if (parsePrimaryExpr(Res, EndLoc, TypeInfo)) {
        return true;
      } else {
        Res = MCUnaryExpr::createMinus(Res, getContext(), FirstTokenLoc);
        return false;
      }
    }
  case AsmToken::Tilde: // complement the bits
    Lex();              // eat ~
    {
      if (parsePrimaryExpr(Res, EndLoc, TypeInfo)) {
        return true;
      } else {
        Res = MCUnaryExpr::createNot(Res, getContext(), FirstTokenLoc);
        return false;
      }
    }
    break;
  case AsmToken::Dollar: // change from pure value to register number
    // here leverage the target MCExpr to handle expressions which involve
    // registers
    return getTargetParser().parsePrimaryExpr(Res, EndLoc);
  case AsmToken::Amp: // take the serial number
    Lex();            // eat &
    {
      std::int64_t SerialNumber = 1;
      Res = MCConstantExpr::create(SerialNumber, getContext());
      return false;
    }
    break;
  default:
    break;
  }
  return true;
}

bool MMIXALParser::parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  if (parseExpression(Res, EndLoc)) {
    return false;
  }
  return parseRParen();
}

class MMOAsmParser : public MCAsmParserExtension {

public:
  MMOAsmParser() = default;
};

} // namespace

namespace llvm {

MCAsmParserExtension *createMMOAsmParser() { return new MMOAsmParser; }

MCAsmParser *createMCMMIXALParser(SourceMgr &SM, MCContext &C, MCStreamer &Out,
                                  const MCAsmInfo &MAI, unsigned CB) {
  return new MMIXALParser(SM, C, Out, MAI, CB);
}

} // namespace llvm
