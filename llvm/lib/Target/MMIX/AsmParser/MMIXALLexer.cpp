//===-- MMIXALLexer.cpp - Lex MMIX assembly language ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXALLexer.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/SaveAndRestore.h"

using namespace llvm;
using namespace llvm::MMIX;

SMLoc ALToken::getLoc() const { return SMLoc::getFromPointer(Str.data()); }

SMLoc ALToken::getEndLoc() const {
  return SMLoc::getFromPointer(Str.data() + Str.size());
}

SMRange ALToken::getLocRange() const { return SMRange(getLoc(), getEndLoc()); }

void ALToken::dump(raw_ostream &OS) const {
  switch (Kind) {
  case ALToken::Error:
    OS << "error";
    break;
  case ALToken::Identifier:
    OS << "identifier: " << getString();
    break;
  case ALToken::Integer:
    OS << "int: " << getString();
    break;
  case ALToken::String:
    OS << "string: " << getString();
    break;

    // clang-format off
  case ALToken::Amp:                OS << "Amp"; break;
  case ALToken::At:                 OS << "At"; break;
  case ALToken::Caret:              OS << "Caret"; break;
  case ALToken::Comma:              OS << "Comma"; break;
  case ALToken::Comment:            OS << "Comment"; break;
  case ALToken::Dollar:             OS << "Dollar"; break;
  case ALToken::EndOfStatement:     OS << "EndOfStatement"; break;
  case ALToken::Eof:                OS << "Eof"; break;
  case ALToken::GreaterGreater:     OS << "GreaterGreater"; break;
  case ALToken::HashDirective:      OS << "HashDirective"; break;
  case ALToken::LParen:             OS << "LParen"; break;
  case ALToken::LessLess:           OS << "LessLess"; break;
  case ALToken::Minus:              OS << "Minus"; break;
  case ALToken::Percent:            OS << "Percent"; break;
  case ALToken::Pipe:               OS << "Pipe"; break;
  case ALToken::Plus:               OS << "Plus"; break;
  case ALToken::RParen:             OS << "RParen"; break;
  case ALToken::Slash:              OS << "Slash"; break;
  case ALToken::SlashSlash:         OS<< "SlashSlash"; break;
  case ALToken::Space:              OS << "Space"; break;
  case ALToken::Star:               OS << "Star"; break;
  case ALToken::Tilde:              OS << "Tilde"; break;
    // clang-format on
  }

  // Print the token string.
  OS << " (\"";
  OS.write_escaped(getString());
  OS << "\")";
}

void ALLexer::setBuffer(StringRef Buf, const char *ptr) {
  // Buffer must be NULL-terminated. NULL terminator must reside at `Buf.end()`.
  // It must be safe to dereference `Buf.end()`.
  assert(*Buf.end() == '\0' &&
         "Buffer provided to AsmLexer lacks null terminator.");

  CurBuf = Buf;

  if (ptr)
    CurPtr = ptr;
  else
    CurPtr = CurBuf.begin();

  TokStart = nullptr;
}

StringRef ALLexer::lexUntilEndOfLine() {
  TokStart = CurPtr;

  while (*CurPtr != '\n' && *CurPtr != '\r' && CurPtr != CurBuf.end())
    ++CurPtr;

  return StringRef(TokStart, CurPtr - TokStart);
}

ALToken ALLexer::lexLineComment() {
  int CurChar = getNextChar();
  while (CurChar != '\n' && CurChar != '\r' && CurChar != EOF)
    CurChar = getNextChar();
  if (CurChar == '\r' && peekNextChar() != EOF && peekNextChar() == '\n')
    ++CurPtr;

  IsAtStartOfLine = true;
  // This is a whole line comment. leave newline
  if (IsAtStartOfInstruction)
    return ALToken(ALToken::EndOfStatement,
                   StringRef(TokStart, CurPtr - TokStart));
  IsAtStartOfInstruction = true;

  auto Comment = StringRef(TokStart, CurPtr - 1 - TokStart);
  return ALToken(ALToken::EndOfStatement, Comment);
}

/// ReturnError - Set the error to the specified string at the specified
/// location.  This is defined to always return ALToken::Error.
ALToken ALLexer::returnError(const char *Loc, const std::string &Msg) {
  setError(SMLoc::getFromPointer(Loc), Msg);

  return ALToken(ALToken::Error, StringRef(Loc, CurPtr - Loc));
}

int ALLexer::getNextChar() {
  if (CurPtr == CurBuf.end())
    return EOF;
  return (unsigned char)*CurPtr++;
}

int ALLexer::peekNextChar() {
  if (CurPtr == CurBuf.end())
    return EOF;
  return (unsigned char)*CurPtr;
}

static bool isIdStartChar(int C) {
  switch (C) {
  case ':':
  case '_':
    return true;
  default:
    return std::isalpha(C) || C > 126;
  }
}

static bool isIdChar(int C) { return isIdStartChar(C) || std::isdigit(C); }

ALToken ALLexer::lexDigit(bool IsHex) {
  StringRef IntStr;
  APInt APIntVal;
  unsigned Radix = IsHex ? 16 : 10;
  if (IsHex) {
    while (llvm::isHexDigit(peekNextChar()))
      ++CurPtr;
    IntStr = StringRef(TokStart, CurPtr - TokStart);
  } else {
    while (std::isdigit(peekNextChar()))
      ++CurPtr;
    IntStr = StringRef(TokStart, CurPtr - TokStart);
  }
  IntStr.getAsInteger(Radix, APIntVal);
  if (APIntVal.getBitWidth() > 64)
    APIntVal = APIntVal.trunc(64);
  uint64_t IntVal = APIntVal.getZExtValue();
  return ALToken(ALToken::Integer, IntStr, IntVal);
}

ALToken ALLexer::lexIdentifier() {
  if (LexMode == Mode::LexMnemonic) {
    StringRef Content = StringRef(TokStart, CurBuf.end() - TokStart);
    if (Content.starts_with("2ADDU") || Content.starts_with("4ADDU") ||
        Content.starts_with("8ADDU"))
      CurPtr += 4;
    if (Content.starts_with("16ADDU"))
      CurPtr += 5;
    LexMode = Mode::LexOp;
  }
  while (isIdChar(peekNextChar()))
    ++CurPtr;
  return ALToken(ALToken::Identifier, StringRef(TokStart, CurPtr - TokStart));
}

ALToken ALLexer::lexLabel() {
  // normal label
  if (isIdStartChar(*TokStart)) {
    while (isIdChar(peekNextChar()))
      ++CurPtr;
    return ALToken(ALToken::Identifier, StringRef(TokStart, CurPtr - TokStart));
  }

  // directional label
  if (std::isdigit(*TokStart)) {
    while (std::isdigit(peekNextChar()))
      ++CurPtr;
    if (getNextChar() == 'H')
      return ALToken(ALToken::Identifier,
                     StringRef(TokStart, CurPtr - TokStart));
  }
  return ALToken(ALToken::Error, "improper local label");
}

size_t ALLexer::peekTokens(MutableArrayRef<ALToken> Buf, bool ShouldSkipSpace) {
  SaveAndRestore SavedTokenStart(TokStart);
  SaveAndRestore SavedCurPtr(CurPtr);
  SaveAndRestore SavedAtStartOfLine(IsAtStartOfLine);
  SaveAndRestore SavedAtStartOfInstruction(IsAtStartOfInstruction);
  SaveAndRestore SavedSkipSpace(SkipSpace, ShouldSkipSpace);
  SaveAndRestore SavedIsPeeking(IsPeeking, true);
  std::string SavedErr = getErr();
  SMLoc SavedErrLoc = getErrLoc();

  size_t ReadCount;
  for (ReadCount = 0; ReadCount < Buf.size(); ++ReadCount) {
    ALToken Token = LexToken();

    Buf[ReadCount] = Token;

    if (Token.is(ALToken::Eof)) {
      ReadCount++;
      break;
    }
  }

  setError(SavedErrLoc, SavedErr);
  return ReadCount;
}

ALToken ALLexer::LexToken() {
  TokStart = CurPtr;
  // This always consumes at least one character.
  int CurChar = getNextChar();

  if (!IsPeeking && CurChar == '#' && IsAtStartOfLine) {
    // If this starts with a '#', this may be a cpp
    // hash directive and otherwise a line comment.
    ALToken TokenBuf[2];
    MutableArrayRef<ALToken> Buf(TokenBuf, 2);
    size_t num = peekTokens(Buf, true);
    // There cannot be a space preceding this
    if (IsAtStartOfInstruction && num == 2 &&
        TokenBuf[0].is(ALToken::Integer) && TokenBuf[1].is(ALToken::String)) {
      CurPtr = TokStart; // reset curPtr;
      StringRef s = lexUntilEndOfLine();
      UnLex(TokenBuf[1]);
      UnLex(TokenBuf[0]);
      return ALToken(ALToken::HashDirective, s);
    }
  }

  // if label field begin with non number or character, then all content
  // after this char is comment
  if (!IsPeeking && IsAtStartOfInstruction) {
    if (!isspace(CurChar) && !isIdChar(CurChar) && CurChar != EOF)
      return lexLineComment();
  }

  // If we're missing a newline at EOF, make sure we still get an
  // EndOfStatement token before the Eof token.
  if (CurChar == EOF && !IsAtStartOfInstruction) {
    IsAtStartOfLine = true;
    IsAtStartOfInstruction = true;
    return ALToken(ALToken::EndOfStatement, StringRef(TokStart, 0));
  }

  if (IsAtStartOfInstruction && !std::isspace(CurChar) && CurChar != EOF) {
    ALToken LabelTok = lexLabel();
    LexMode = Mode::LexMnemonic;
    return LabelTok;
  }

  IsAtStartOfLine = false;
  IsAtStartOfInstruction = false;
  switch (CurChar) {
  default:
    if (isIdStartChar(CurChar))
      return lexIdentifier();
    break;
  case EOF:
    IsAtStartOfLine = true;
    IsAtStartOfInstruction = true;
    return ALToken(ALToken::Eof, StringRef(TokStart, 0));
  case '\0':
  case ' ':
  case '\t':
  case '\v':
    while (peekNextChar() == ' ' || peekNextChar() == '\t')
      CurPtr++;
    if (LexMode == Mode::LexLabel)
      LexMode = Mode::LexMnemonic;
    return LexToken(); // Ignore whitespace.
  case '\r':
    IsAtStartOfLine = true;
    IsAtStartOfInstruction = true;
    LexMode = Mode::LexLabel;
    if (peekNextChar() == '\n')
      ++CurPtr;
    return ALToken(ALToken::EndOfStatement,
                   StringRef(TokStart, CurPtr - TokStart));
  case '\n':
    IsAtStartOfLine = true;
    IsAtStartOfInstruction = true;
    LexMode = Mode::LexLabel;
    return ALToken(ALToken::EndOfStatement, StringRef(TokStart, 1));
  case '+':
    return ALToken(ALToken::Plus, StringRef(TokStart, 1));
  case '-':
    return ALToken(ALToken::Minus, StringRef(TokStart, 1));
  case '~':
    return ALToken(ALToken::Tilde, StringRef(TokStart, 1));
  case '(':
    return ALToken(ALToken::LParen, StringRef(TokStart, 1));
  case ')':
    return ALToken(ALToken::RParen, StringRef(TokStart, 1));
  case '*':
    return ALToken(ALToken::Star, StringRef(TokStart, 1));
  case ',':
    return ALToken(ALToken::Comma, StringRef(TokStart, 1));
  case '$':
    return ALToken(ALToken::Dollar, StringRef(TokStart, 1));
  case '@':
    return ALToken(ALToken::At, StringRef(TokStart, 1));
  case '&':
    return ALToken(ALToken::Amp, StringRef(TokStart, 1));
  case '|':
    return ALToken(ALToken::Pipe, StringRef(TokStart, 1));
  case '/':
    if (peekNextChar() == '/') {
      ++CurPtr;
      return ALToken(ALToken::SlashSlash, StringRef(TokStart, 2));
    }
    return ALToken(ALToken::Slash, StringRef(TokStart, 1));
  case '%':
    return ALToken(ALToken::Percent, StringRef(TokStart, 1));
  case '^':
    return ALToken(ALToken::Caret, StringRef(TokStart, 1));
  case ';':
    IsAtStartOfInstruction = true;
    return ALToken(ALToken::EndOfStatement, StringRef(TokStart, 1));

  case '#':
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
    if (LexMode == Mode::LexMnemonic)
      return lexIdentifier();
    return lexDigit(CurChar == '#');
  }

  return returnError(TokStart, "unexpected char");
}
