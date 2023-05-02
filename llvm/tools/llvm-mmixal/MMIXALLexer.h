#ifndef LLVM_MMIXAL_LEXER_H
#define LLVM_MMIXAL_LEXER_H

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"

namespace llvm {

class MMIXALLexer : public MCAsmLexer {
private:
  const char *CurPtr = nullptr;
  StringRef CurBuf;

  bool IsAtStartOfLine = true;
  bool IsAtStartOfStatement = true;
  bool IsPeeking = false;
  bool EndStatementAtEOF = true;
  bool RegardAsComment = false;
  const bool &StrictMode;

private:
  // utilities
  int getNextChar();
  int peekNextChar();
  bool isIdentifierChar(int C);
  static bool isLineEndChar(int C);
  AsmToken ReturnError(const char *Loc, const std::string &Msg);

private:
  StringRef LexUntilEndOfLine();
  AsmToken LexLineComment();
  AsmToken LexSingleQuote();
  AsmToken LexSingleQuoteWithEscapeSequence();
  AsmToken LexQuote();
  AsmToken LexWhiteSpace();
  AsmToken LexHexDigit();
  AsmToken LexDecDigitOrIdentifier();
  bool LexEscapeSequence(StringRef Literal, APInt &Value);

protected:
  AsmToken LexToken() override;
  StringRef LexUntilEndOfStatement() override;
  size_t peekTokens(MutableArrayRef<AsmToken> Buf,
                    bool ShouldSkipSpace = true) override;
public:
void setBuffer(StringRef Buf, const char *ptr = nullptr,
                 bool EndStatementAtEOF = true);
void regardAsComment();
public:
  MMIXALLexer(const MCAsmInfo &MAI, const bool &StrictMode);
  MMIXALLexer(const AsmLexer &) = delete;
  MMIXALLexer &operator=(const AsmLexer &) = delete;
  ~MMIXALLexer() override = default;
};

} // namespace llvm

#endif // LLVM_MMIXAL_LEXER_H
