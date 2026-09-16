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

size_t ALLexer::peekTokens(MutableArrayRef<ALToken> Buf, bool ShouldSkipSpace) {
  SaveAndRestore SavedTokenStart(TokStart);
  SaveAndRestore SavedCurPtr(CurPtr);
  SaveAndRestore SavedAtStartOfLine(IsAtStartOfLine);
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

static bool isIdChar(char C) {
  switch (C) {
  case ':':
  case '_':
    return true;
  default:
    return std::isalnum(C) || C > 126;
  }
}

static bool isLocalLabel(StringRef L) {
  if (!L.ends_with('H'))
    return false;
  L = L.drop_back();
  return llvm::all_of(L, [](char C) { return std::isdigit(C); });
}

static bool isValidLabel(StringRef L) {
  if (isLocalLabel(L))
    return true;
  if (L.empty())
    return false;
  return !std::isdigit(L.front()) && llvm::all_of(L, isIdChar);
}

const ALToken &ALLexer::lex() {
  assert(!CurTok.empty());
  // Mark if we parsing out a EndOfStatement.
  JustConsumedEOL = CurTok.front().getKind() == ALToken::EndOfStatement;
  CurTok.erase(CurTok.begin());
  // LexToken may generate multiple tokens via UnLex but will always return
  // the first one. Place returned value at head of CurTok vector.
  if (CurTok.empty()) {
    ALToken T = LexToken();
    CurTok.insert(CurTok.begin(), T);
  }
  return CurTok.front();
}

ALToken ALLexer::lexIdentifier() {
  while (isIdChar(peekNextChar()))
    ++CurPtr;
  return ALToken(ALToken::Identifier, StringRef(TokStart, CurPtr - TokStart));
}

ALToken ALLexer::LexToken() {
  TokStart = CurPtr;
  // This always consumes at least one character.
  int CurChar = getNextChar();

  IsAtStartOfLine = false;
  IsAtStartOfStatement = false;
  switch (CurChar) {
  default:
    return lexIdentifier();
    break;
  case EOF:
    IsAtStartOfLine = true;
    return ALToken(ALToken::Eof, StringRef(TokStart, 0));
  case '\0':
  case ' ':
  case '\t':
  case '\v':
    while (peekNextChar() == ' ' || peekNextChar() == '\t')
      CurPtr++;
    if (SkipSpace)
      return LexToken(); // Ignore whitespace.
    else
      return ALToken(ALToken::Space, StringRef(TokStart, CurPtr - TokStart));
  case '\r':
    IsAtStartOfLine = true;
    IsAtStartOfStatement = true;
    if (peekNextChar() == '\n')
      ++CurPtr;
    return ALToken(ALToken::EndOfStatement,
                   StringRef(TokStart, CurPtr - TokStart));
  case '\n':
    IsAtStartOfLine = true;
    IsAtStartOfStatement = true;
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
    return ALToken(ALToken::EndOfStatement, StringRef(TokStart, 1));
  }

  return returnError(TokStart, "unexpected char");
}
