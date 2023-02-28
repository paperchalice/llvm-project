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
#include "llvm/Support/ConvertUTF.h"
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
  bool IsStrictMode = true;

  bool isAtStatementSeparator(const char *Ptr) {
    return strncmp(Ptr, ";", 1) == 0;
  }

  int getNextChar() {
    return (CurPtr == CurBuf.end()) ? EOF
                                    : static_cast<unsigned char>(*CurPtr++);
  }

  int peekNextChar() {
    return (CurPtr == CurBuf.end()) ? EOF : static_cast<unsigned char>(*CurPtr);
  }

  bool isIdentifierChar(int C) {
    if (C > 126)
      return true;
    if (isalnum(C))
      return true;
    switch (C) {
    case ':':
    case '_':
      return true;
    default:
      return false;
    }
  }

  AsmToken ReturnError(const char *Loc, const std::string &Msg) {
    SetError(SMLoc::getFromPointer(Loc), Msg);

    return AsmToken(AsmToken::Error, StringRef(Loc, CurPtr - Loc));
  }

  AsmToken LexIdentifier() {
    while (isIdentifierChar(peekNextChar())) {
      CurPtr++;
    }
    StringRef TokStr(TokStart, CurPtr - TokStart);
    return AsmToken(AsmToken::Identifier, TokStr);
  }

  AsmToken LexWhiteSpace() {
    while (peekNextChar() == ' ' || peekNextChar() == '\t') {
      CurPtr++;
    }
    return AsmToken(AsmToken::Space, StringRef(TokStart, CurPtr - TokStart));
  }

  AsmToken LexLineComment() {
    // Mark This as an end of statement with a body of the
    // comment. While it would be nicer to leave this two tokens,
    // backwards compatability with TargetParsers makes keeping this in this
    // form better.
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

  AsmToken LexHexDigit() {

    while (isHexDigit(peekNextChar())) {
      CurPtr++;
    }
    StringRef TokStr(TokStart, CurPtr - TokStart);
    APInt Value;
    if (TokStr.drop_front().getAsInteger(16, Value)) {
      return ReturnError(TokStart, "invalid hexadecial number");
    }
    if (Value.isIntN(64))
      return AsmToken(AsmToken::Integer, TokStr, Value);
    return AsmToken(AsmToken::BigNum, TokStr, Value);
  }

  AsmToken LexDecDigit() {
    while (std::isdigit(peekNextChar())) {
      CurPtr++;
    }
    StringRef TokStr(TokStart, CurPtr - TokStart);
    APInt Value;
    if (TokStr.getAsInteger(10, Value)) {
      return ReturnError(TokStart, "invalid decimal number");
    }

    if (Value.isIntN(64))
      return AsmToken(AsmToken::Integer, TokStr, Value);
    return AsmToken(AsmToken::BigNum, TokStr, Value);
  }

  AsmToken LexSingleQuote() {
    auto CharVal = getNextChar(); // get the character...
    auto CurChar = CharVal;

    if (!IsStrictMode) {
      // TODO: we can handle more complex content...
    }

    CurChar = getNextChar(); // expect '\''

    if (CurChar != '\'')
      return ReturnError(TokStart, "illegal character constant!");

    if (CharVal == '\n')
      return ReturnError(TokStart, "incomplete character constant!");

    // The idea here being that 'c' is basically just an integral
    // constant.
    StringRef Res = StringRef(TokStart, CurPtr - TokStart);
    return AsmToken(AsmToken::Integer, Res, CharVal);
  }

  AsmToken LexQuote() {
    int CurChar = getNextChar();
    while (CurChar != '"') {
      if (!IsStrictMode) {
        // TODO: handle escape sequence, TBD...
      }
      CurChar = getNextChar();
      if (CurChar == EOF || CurChar == '\n')
        return ReturnError(TokStart, "incomplete string constant!");
    }
    return AsmToken(AsmToken::String, StringRef(TokStart, CurPtr - TokStart));
  }

protected:
  /// LexToken - Read the next token and return its code.
  AsmToken LexToken() override {
    TokStart = CurPtr;
    // This always consumes at least one character.
    char CurChar = getNextChar();

    // lex line direcitive
    if (!IsPeeking && CurChar == '#' && IsAtStartOfStatement) {
      // If this starts with a '#', this may be a cpp
      // hash directive and otherwise a line comment.
      AsmToken TokenBuf[2];
      MutableArrayRef<AsmToken> Buf(TokenBuf, 2);
      size_t num = peekTokens(Buf, true);
      // There cannot be a space preceding this
      if (IsAtStartOfLine && num == 2 && TokenBuf[0].is(AsmToken::Integer) &&
          TokenBuf[1].is(AsmToken::String)) {
        CurPtr = TokStart; // reset curPtr;
        StringRef s = LexUntilEndOfLine();
        UnLex(TokenBuf[1]);
        UnLex(TokenBuf[0]);
        return AsmToken(AsmToken::HashDirective, s);
      } else {
        // else it is line comment
        return LexLineComment();
      }
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
      if (isIdentifierChar(CurChar))
        return LexIdentifier();
      else
        return ReturnError(TokStart, "unknown token");
    case EOF:
      if (EndStatementAtEOF) {
        IsAtStartOfLine = true;
        IsAtStartOfStatement = true;
      }
      return AsmToken(AsmToken::Eof, StringRef(TokStart, 0));
    case 0:
    case ' ':
    case '\t': {
      IsAtStartOfStatement = OldIsAtStartOfStatement;
      auto TokSpace = LexWhiteSpace();
      return SkipSpace ? LexToken() : TokSpace;
    }
    case '\r': {
      IsAtStartOfLine = true;
      IsAtStartOfStatement = true;
      // If this is a CR followed by LF, treat that as one token.
      if (CurPtr != CurBuf.end() && *CurPtr == '\n')
        ++CurPtr;
      return AsmToken(AsmToken::EndOfStatement,
                      StringRef(TokStart, CurPtr - TokStart));
    }
    case '\n':
      IsAtStartOfLine = true;
      IsAtStartOfStatement = true;
      return AsmToken(AsmToken::EndOfStatement, StringRef(TokStart, 1));
    case '\'':
      return LexSingleQuote();
    case '"':
      return LexQuote();
    case '#':
      return LexHexDigit();
    case '+':
      return AsmToken(AsmToken::Plus, StringRef(TokStart, 1));
    case '-':
      return AsmToken(AsmToken::Minus, StringRef(TokStart, 1));
    case '~':
      return AsmToken(AsmToken::Tilde, StringRef(TokStart, 1));
    case '(':
      return AsmToken(AsmToken::LParen, StringRef(TokStart, 1));
    case ')':
      return AsmToken(AsmToken::RParen, StringRef(TokStart, 1));
    case '*':
      return AsmToken(AsmToken::Star, StringRef(TokStart, 1));
    case ',':
      return AsmToken(AsmToken::Comma, StringRef(TokStart, 1));
    case '|':
      return AsmToken(AsmToken::Pipe, StringRef(TokStart, 1));
    case '&':
      return AsmToken(AsmToken::Amp, StringRef(TokStart, 1));
    case '$':
      return AsmToken(AsmToken::Dollar, StringRef(TokStart, 1));
    case '%':
      return AsmToken(AsmToken::Percent, StringRef(TokStart, 1));
    case '/':
      if (peekNextChar() == '/') {
        getNextChar();
        return AsmToken(AsmToken::Slash, StringRef(TokStart, 2));
      } else {
        return AsmToken(AsmToken::Slash, StringRef(TokStart, 1));
      }
    case '<': {
      if (peekNextChar() == '<') {
        CurPtr++;
        return AsmToken(AsmToken::LessLess, StringRef(TokStart, 2));
      } else {
        return AsmToken(AsmToken::Error, StringRef());
      }
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return LexDecDigit();
    case ';':
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
  bool isStrictMode() { return IsStrictMode; }

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

    while (!isAtStatementSeparator(CurPtr) && // End of statement marker.
           *CurPtr != '\n' && *CurPtr != '\r' && CurPtr != CurBuf.end()) {
      ++CurPtr;
    }
    return StringRef(TokStart, CurPtr - TokStart);
  }

  StringRef LexUntilEndOfLine() {
    while (peekNextChar() != '\n' && peekNextChar() != '\r' &&
           peekNextChar() != EOF) {
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
  MMIXALLexer Lexer;
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
                        AsmTypeInfo *TypeInfo) override {
    return getTargetParser().parsePrimaryExpr(Res, EndLoc);
  }
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
  bool parseBinOpRHS(unsigned Precedence, const MCExpr *&Res, SMLoc &EndLoc);
  unsigned getBinOpPrecedence(AsmToken::TokenKind K,
                              MCBinaryExpr::Opcode &Kind);
  void lexLispComment();
  bool parseStatement();
};

const AsmToken &MMIXALParser::Lex() { return getLexer().Lex(); }

bool MMIXALParser::parseStatement() {
  // line: ^[label]\s+<instruction>[;\s*<instruction>]\s+[arbitray contents]$
  // if we get error, discard all rest content until line end
  bool Failed = false;

  auto CurTok = getTok();
  // parse label
  if (Lexer.isStrictMode()) {
    if (CurTok.isNot(AsmToken::Space)) {
      // the first token is always label
      if (CurTok.is(AsmToken::Identifier)) {
        // create symbol
        getContext().getOrCreateSymbol(CurTok.getString());
        Lexer.setSkipSpace(true);
        Lex(); // eat identifier
      } else if (CurTok.is(AsmToken::Integer)) {
        // it is a local label, peek next token
        auto Suffix = Lexer.peekTok(false);
        if (Suffix.getString() == "H") {
          Lexer.setSkipSpace(true);
          Lex(); // eat number
          Lex(); // eat 'H'
          getContext().createDirectionalLocalSymbol(CurTok.getIntVal());
        } else {
          return Error(CurTok.getLoc(), "improper local label");
        }
      } else {
        // it is a line comment
        Lexer.LexUntilEndOfLine();
        Lex();
        return parseToken(AsmToken::EndOfStatement);
      }
    }
  } else {
    // TODO:
  }

  // parse instruction field
  auto InstTok = getTok();
  // handle 2ADD etc
  if (InstTok.is(AsmToken::Integer) || InstTok.is(AsmToken::BigNum)) {
    if (std::isdigit(InstTok.getString()[0])) {
      auto &AddPart = Lexer.peekTok(false);
      const char *TokStart = InstTok.getString().begin();
      Lex(); // eat digits
      if (AddPart.is(AsmToken::Identifier)) {
        Lex(); // eat this identifier
        StringRef TokStr(TokStart, InstTok.getString().size() +
                                       AddPart.getString().size());
        Lexer.UnLex(AsmToken(AsmToken::Identifier, TokStr));
        InstTok = getTok();
      }
    }
  }

  if (InstTok.isNot(AsmToken::Identifier)) {
    return Error(InstTok.getLoc(), "unknown opcode");
  }

  Lexer.setSkipSpace(!Lexer.isStrictMode()); // we use space to identify whether in strict mmixal mode
  Lex(); // eat opcode
  // parse instruciton
  ParseInstructionInfo IInfo;
  SmallVector<std::unique_ptr<MCParsedAsmOperand>, 4> Operands;
  Failed = getTargetParser().ParseInstruction(IInfo, InstTok.getString().lower(), InstTok,
                                     Operands);

  return Failed;
}

unsigned MMIXALParser::getBinOpPrecedence(AsmToken::TokenKind K,
                                          MCBinaryExpr::Opcode &Kind) {
  constexpr unsigned weak_prec = 2, strong_prec = 3;

  switch (K) {
  default:
    return 0;
  // weak
  case AsmToken::Plus:
    Kind = MCBinaryExpr::Add;
    return weak_prec;
  case AsmToken::Minus:
    Kind = MCBinaryExpr::Sub;
    return weak_prec;
  case AsmToken::Pipe:
    Kind = MCBinaryExpr::Or;
    return weak_prec;
  case AsmToken::Caret:
    Kind = MCBinaryExpr::Xor;
    return weak_prec;

  // strong
  case AsmToken::Star:
    Kind = MCBinaryExpr::Mul;
    return strong_prec;
  case AsmToken::Slash:
    Kind = MCBinaryExpr::Div;
    return strong_prec;
  // case AsmToken::DoubleSlash
  case AsmToken::Percent:
    Kind = MCBinaryExpr::Mod;
    return strong_prec;
  case AsmToken::LessLess:
    Kind = MCBinaryExpr::Shl;
    return strong_prec;
  case AsmToken::GreaterGreater:
    Kind = MCBinaryExpr::AShr;
    return strong_prec;
  }
}

bool MMIXALParser::parseBinOpRHS(unsigned Precedence, const MCExpr *&Res,
                                 SMLoc &EndLoc) {
  SMLoc StartLoc = Lexer.getLoc();
  while (true) {
    MCBinaryExpr::Opcode Kind = MCBinaryExpr::Add;
    unsigned TokPrec = getBinOpPrecedence(Lexer.getKind(), Kind);

    // If the next token is lower precedence than we are allowed to eat, return
    // successfully with what we ate already.
    if (TokPrec < Precedence)
      return false;

    Lex();

    // Eat the next primary expression.
    const MCExpr *RHS;
    if (parsePrimaryExpr(RHS, EndLoc, nullptr))
      return true;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    MCBinaryExpr::Opcode Dummy;
    unsigned NextTokPrec = getBinOpPrecedence(Lexer.getKind(), Dummy);
    if (TokPrec < NextTokPrec && parseBinOpRHS(TokPrec + 1, RHS, EndLoc))
      return true;

    // Merge LHS and RHS according to operator.
    Res = MCBinaryExpr::create(Kind, Res, RHS, getContext(), StartLoc);
  }
}

// <expression> -> <term> | <expression> <weak operator> <term>
bool MMIXALParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  Res = nullptr;

  if (parsePrimaryExpr(Res, EndLoc, nullptr) || parseBinOpRHS(1, Res, EndLoc))
    return true;
  // Try to constant fold it up front, if possible. Do not exploit
  // assembler here.
  // int64_t Value;
  // if (Res->evaluateAsAbsolute(Value))
  //   Res = MCConstantExpr::create(Value, getContext());

  return false;
}

bool MMIXALParser::Run(bool NoInitialTextSection, bool NoFinalize) {
  // Prime the lexer.
  auto T = Lex();
  getTargetParser().onBeginOfFile();

  while (getTok().isNot(AsmToken::Eof)) {
    parseStatement();
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
