//===-- MMIXALLexer.h - Lex MMIX assembly language ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm::MMIX {

/// @brief Similar to ALToken, but only for MMIXAL
class ALToken {
public:
  enum TokenKind {
    // Markers
    Eof,
    Error,

    // String values.
    Identifier,
    String,

    // Integer values.
    Integer,

    // Comments
    Comment,
    HashDirective,
    // No-value.
    EndOfStatement,
    Space,
    Comma,
    LParen,
    RParen,
    Plus,
    Minus,
    Tilde,
    Dollar,
    At,
    Amp,
    Star,
    Slash,
    SlashSlash,
    Percent,
    LessLess,
    GreaterGreater,
    Pipe,
    Caret,
  };

  ALToken() = default;
  ALToken(TokenKind Kind, StringRef Str, uint64_t IntVal = 0)
      : Kind(Kind), Str(Str), IntVal(IntVal) {}

  TokenKind getKind() const { return Kind; }
  bool is(TokenKind K) const { return Kind == K; }
  bool isNot(TokenKind K) const { return Kind != K; }

  LLVM_ABI SMLoc getLoc() const;
  LLVM_ABI SMLoc getEndLoc() const;
  LLVM_ABI SMRange getLocRange() const;

  /// Get the contents of a string token (without quotes).
  StringRef getStringContents() const {
    assert(Kind == String && "This token isn't a string!");
    return Str.slice(1, Str.size() - 1);
  }

  /// Get the identifier string for the current token, which should be an
  /// identifier or a string. This gets the portion of the string which should
  /// be used as the identifier, e.g., it does not include the quotes on
  /// strings.
  StringRef getIdentifier() const {
    if (Kind == Identifier)
      return getString();
    return getStringContents();
  }

  /// Get the string for the current token, this includes all characters (for
  /// example, the quotes on strings) in the token.
  ///
  /// The returned StringRef points into the source manager's memory buffer, and
  /// is safe to store across calls to Lex().
  StringRef getString() const { return Str; }

  // FIXME: Don't compute this in advance, it makes every token larger, and is
  // also not generally what we want (it is nicer for recovery etc. to lex 123br
  // as a single token, then diagnose as an invalid number).
  uint64_t getUIntVal() const {
    assert(Kind == Integer && "This token isn't an integer!");
    return IntVal;
  }

  LLVM_ABI void dump(raw_ostream &OS) const;

private:
  TokenKind Kind = TokenKind::Eof;

  /// A reference to the entire token contents; this is always a pointer into
  /// a memory buffer owned by the source manager.
  StringRef Str;

  uint64_t IntVal;
};

/// @brief Same as AsmLexer, but specific for MMIXAL
class ALLexer {
  /// The current token, stored in the base class for faster access.
  SmallVector<MMIX::ALToken, 1> CurTok = {ALToken(ALToken::Space, StringRef())};

  const char *CurPtr = nullptr;
  /// NULL-terminated buffer. NULL terminator must reside at `CurBuf.end()`.
  StringRef CurBuf;

  /// The location and description of the current error
  SMLoc ErrLoc;
  std::string Err;

  const char *TokStart = nullptr;
  bool IsAtStartOfInstruction = true;
  bool IsAtStartOfLine = true;
  bool JustConsumedEOL = true;
  bool IsPeeking = false;
  bool SkipSpace = true;

  enum class Mode {
    LexLabel,
    LexMnemonic,
    LexOp,
  } LexMode = Mode::LexLabel;

private:
  LLVM_ABI ALToken LexToken();

  [[nodiscard]] int getNextChar();
  [[nodiscard]] int peekNextChar();

public:
  ALLexer() = default;
  ALLexer(const ALLexer &) = delete;
  ALLexer &operator=(const ALLexer &) = delete;

public:
  /// Get the current error location
  SMLoc getErrLoc() { return ErrLoc; }

  /// Get the current error string
  const std::string &getErr() { return Err; }

  /// Consume the next token from the input stream and return it.
  ///
  /// The lexer will continuously return the end-of-file token once the end of
  /// the main input file has been reached.
  const ALToken &Lex() {
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

  void UnLex(ALToken const &Token) { CurTok.insert(CurTok.begin(), Token); }

  bool justConsumedEOL() { return JustConsumedEOL; }

  LLVM_ABI StringRef LexUntilEndOfStatement();

  /// Get the current source location.
  SMLoc getLoc() const { return SMLoc::getFromPointer(TokStart); }

  /// Get the current (last) lexed token.
  const ALToken &getTok() const { return CurTok[0]; }

  /// Look ahead at the next token to be lexed.
  const ALToken peekTok(bool ShouldSkipSpace = true) {
    ALToken Tok;

    MutableArrayRef<ALToken> Buf(Tok);
    size_t ReadCount = peekTokens(Buf, ShouldSkipSpace);

    assert(ReadCount == 1);
    (void)ReadCount;

    return Tok;
  }

  /// Look ahead an arbitrary number of tokens.
  LLVM_ABI size_t peekTokens(MutableArrayRef<ALToken> Buf,
                             bool ShouldSkipSpace = true);

public:
  void setBuffer(StringRef Buf, const char *ptr = nullptr);

private:
  void setError(SMLoc errLoc, const std::string &err) {
    ErrLoc = errLoc;
    Err = err;
  }

  ALToken returnError(const char *Loc, const std::string &Msg);
  ALToken lexLineComment();
  ALToken lexLabel();
  StringRef lexUntilEndOfLine();
  ALToken lexDigit(bool IsHex);
  ALToken lexIdentifier();
};

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H
