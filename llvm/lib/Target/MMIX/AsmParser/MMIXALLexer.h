//===-- MMIXALLexer.h - Lex MMIX assembly language ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H

namespace llvm {

class MMIXALToken {
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
    Colon,
    Space,
    Plus,
    Minus,
    Tilde,
    Slash, // '/'
    SlashSlash, // "//"
    BackSlash, // '\'
    LParen,
    RParen,
    LBrac,
    RBrac,
    LCurly,
    RCurly,
    Question,
    Star,
    Dot,
    Comma,
    Dollar,
    Equal,
    EqualEqual,

    Pipe,
    PipePipe,
    Caret,
    Amp,
    AmpAmp,
    Exclaim,
    ExclaimEqual,
    Percent,
    Hash,
    Less,
    LessEqual,
    LessLess,
    LessGreater,
    Greater,
    GreaterEqual,
    GreaterGreater,
    At,
    MinusGreater,
  };

private:
  TokenKind Kind = TokenKind::Eof;
};

class MMIXALLexer {};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALLEXER_H
