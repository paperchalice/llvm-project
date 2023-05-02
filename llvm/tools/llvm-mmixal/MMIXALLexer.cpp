#include "MMIXALLexer.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/Unicode.h"
#include <cctype>

namespace llvm {

static const DenseMap<StringRef, std::optional<uint32_t>> NCMap{
    {"NULL", 0x0000},
    {"START OF HEADING", 0x0001},
    {"START OF TEXT", 0x0002},
    {"END OF TEXT", 0x0003},
    {"END OF TRANSMISSION", 0x0004},
    {"ENQUIRY", 0x0005},
    {"ACKNOWLEDGE", 0x0006},
    {"ALERT", 0x0007},
    {"BACKSPACE", 0x0008},
    {"CHARACTER TABULATION", 0x0009},
    {"HORIZONTAL TABULATION", 0x0009},
    {"LINE FEED", 0x000A},
    {"NEW LINE", 0x000A},
    {"END OF LINE", 0x000A},
    {"LINE TABULATION", 0x000B},
    {"VERTICAL TABULATION", 0x000B},
    {"FORM FEED", 0x000C},
    {"CARRIAGE RETURN", 0x000D},
    {"SHIFT OUT", 0x000E},
    {"LOCKING-SHIFT ONE", 0x000E},
    {"SHIFT IN", 0x000F},
    {"LOCKING-SHIFT ZERO", 0x000F},
    {"DATA LINK ESCAPE", 0x0010},
    {"DEVICE CONTROL ONE", 0x0011},
    {"DEVICE CONTROL TWO", 0x0012},
    {"DEVICE CONTROL THREE", 0x0013},
    {"DEVICE CONTROL FOUR", 0x0014},
    {"NEGATIVE ACKNOWLEDGE", 0x0015},
    {"SYNCHRONOUS IDLE", 0x0016},
    {"END OF TRANSMISSION BLOCK", 0x0017},
    {"CANCEL", 0x0018},
    {"END OF MEDIUM", 0x0019},
    {"SUBSTITUTE", 0x001A},
    {"ESCAPE", 0x001B},
    {"INFORMATION SEPARATOR FOUR", 0x001C},
    {"FILE SEPARATOR", 0x001C},
    {"INFORMATION SEPARATOR THREE", 0x001D},
    {"GROUP SEPARATOR", 0x001D},
    {"INFORMATION SEPARATOR TWO", 0x001E},
    {"RECORD SEPARATOR", 0x001E},
    {"INFORMATION SEPARATOR ONE", 0x001F},
    {"UNIT SEPARATOR", 0x001F},
    {"DELETE", 0x007F},
    {"BREAK PERMITTED HERE", 0x0082},
    {"NO BREAK HERE", 0x0083},
    {"INDEX", 0x0084},
    {"NEXT LINE", 0x0085},
    {"START OF SELECTED AREA", 0x0086},
    {"END OF SELECTED AREA", 0x0087},
    {"CHARACTER TABULATION SET", 0x0088},
    {"HORIZONTAL TABULATION SET", 0x0088},
    {"CHARACTER TABULATION WITH JUSTIFICATION", 0x0089},
    {"HORIZONTAL TABULATION WITH JUSTIFICATION", 0x0089},
    {"LINE TABULATION SET", 0x008A},
    {"VERTICAL TABULATION SET", 0x008A},
    {"PARTIAL LINE FORWARD", 0x008B},
    {"PARTIAL LINE DOWN", 0x008B},
    {"PARTIAL LINE BACKWARD", 0x008C},
    {"PARTIAL LINE UP", 0x008C},
    {"REVERSE LINE FEED", 0x008D},
    {"REVERSE INDEX", 0x008D},
    {"SINGLE SHIFT TWO", 0x008E},
    {"SINGLE-SHIFT-2", 0x008E},
    {"SINGLE SHIFT THREE", 0x008F},
    {"SINGLE-SHIFT-3", 0x008F},
    {"DEVICE CONTROL STRING", 0x0090},
    {"PRIVATE USE ONE", 0x0091},
    {"PRIVATE USE-1", 0x0091},
    {"PRIVATE USE TWO", 0x0092},
    {"PRIVATE USE-2", 0x0092},
    {"SET TRANSMIT STATE", 0x0093},
    {"CANCEL CHARACTER", 0x0094},
    {"MESSAGE WAITING", 0x0095},
    {"START OF GUARDED AREA", 0x0096},
    {"START OF PROTECTED AREA", 0x0096},
    {"END OF GUARDED AREA", 0x0097},
    {"END OF PROTECTED AREA", 0x0097},
    {"START OF STRING", 0x0098},
    {"SINGLE CHARACTER INTRODUCER", 0x009A},
    {"CONTROL SEQUENCE INTRODUCER", 0x009B},
    {"STRING TERMINATOR", 0x009C},
    {"OPERATING SYSTEM COMMAND", 0x009D},
    {"PRIVACY MESSAGE", 0x009E},
    {"APPLICATION PROGRAM COMMAND", 0x009F},
};

int MMIXALLexer::getNextChar() {
  return (CurPtr == CurBuf.end()) ? EOF : static_cast<unsigned char>(*CurPtr++);
}

int MMIXALLexer::peekNextChar() {
  return (CurPtr == CurBuf.end()) ? EOF : static_cast<unsigned char>(*CurPtr);
}

bool MMIXALLexer::isIdentifierChar(int C) {
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

bool MMIXALLexer::isLineEndChar(int C) {
  switch (C) {
  case EOF:
  case '\n':
    return true;
  default:
    return false;
  }
}

AsmToken MMIXALLexer::ReturnError(const char *Loc, const std::string &Msg) {
  SetError(SMLoc::getFromPointer(Loc), Msg);

  return AsmToken(AsmToken::Error, StringRef(Loc, CurPtr - Loc));
}

StringRef MMIXALLexer::LexUntilEndOfLine() {
  while (!isLineEndChar(peekNextChar())) {
    ++CurPtr;
  }
  return StringRef(TokStart, CurPtr - TokStart);
}

AsmToken MMIXALLexer::LexWhiteSpace() {
  while (peekNextChar() == ' ' || peekNextChar() == '\t') {
    CurPtr++;
  }
  return AsmToken(AsmToken::Space, StringRef(TokStart, CurPtr - TokStart));
}

AsmToken MMIXALLexer::LexLineComment() {
  int CurChar = getNextChar();
  ptrdiff_t LenOfEnd = 1;
  while (!isLineEndChar(CurChar)) {
    CurChar = getNextChar();
  }

  IsAtStartOfLine = true;
  IsAtStartOfStatement = true;
  if (CurChar == EOF)
    LenOfEnd = 0;
  else if (*(CurPtr - 2) == '\r')
    LenOfEnd = 2;

  return AsmToken(AsmToken::EndOfStatement,
                  StringRef(TokStart, CurPtr - TokStart - LenOfEnd));
}

AsmToken MMIXALLexer::LexHexDigit() {
  while (std::isxdigit(peekNextChar())) {
    CurPtr++;
  }
  StringRef TokStr(TokStart, CurPtr - TokStart);
  APInt Value(64, 0);
  if (TokStr.drop_front().getAsInteger(16, Value)) {
    return ReturnError(TokStart, "invalid hexadecial number");
  }
  if (Value.isIntN(64))
    return AsmToken(AsmToken::Integer, TokStr, Value);
  return AsmToken(AsmToken::BigNum, TokStr, Value);
}

AsmToken MMIXALLexer::LexDecDigitOrIdentifier() {
  while (isIdentifierChar(peekNextChar())) {
    getNextChar();
  }
  StringRef TokStr(TokStart, CurPtr - TokStart);
  APInt Value;
  if (TokStr.getAsInteger(10, Value)) {
    return AsmToken(AsmToken::Identifier, TokStr);
  } else {
    if (Value.isIntN(64))
      return AsmToken(AsmToken::Integer, TokStr, Value);
    else
      return AsmToken(AsmToken::BigNum, TokStr, Value);
  }
}

bool MMIXALLexer::LexEscapeSequence(StringRef Literal, APInt &Value) {
  auto Content = Literal.drop_front();
  if (Content.size() == 1 && isalpha(Content[0])) {
    switch (Content[0]) {
    case 'a':
      Value = '\a';
      return false;
    case 'b':
      Value = '\b';
      return false;
    case 'f':
      Value = '\f';
      return false;
    case 'n':
      Value = '\n';
      return false;
    case 'r':
      Value = '\r';
      return false;
    case 't':
      Value = '\t';
      return false;
    case 'v':
      Value = '\v';
      return false;
    default:
      return true;
    }
  }

  // \nnn
  if (isdigit(Content[0])) {
    return Content.size() > 3 || Content.getAsInteger(8, Value);
  }

  // \o{n...}
  if (Content.starts_with("o{") && Content.ends_with("}")) {
    StringRef OctLiteral = Content.drop_front(2).drop_back();
    return OctLiteral.getAsInteger(8, Value);
  }

  // \xn... \x{n...}
  if (Content[0] == 'x') {
    StringRef HexLiteral = Content.drop_front();
    if (HexLiteral.starts_with("{") && HexLiteral.ends_with("}")) {
      HexLiteral = HexLiteral.drop_front().drop_back();
    }
    return HexLiteral.getAsInteger(16, Value);
  }

  // \unnnn \u{n...}
  if (Content[0] == 'u') {
    StringRef UCLiteral = Content.drop_front();
    if (UCLiteral.starts_with("{") && UCLiteral.ends_with("}")) {
      UCLiteral = UCLiteral.drop_front().drop_back();
      return UCLiteral.getAsInteger(16, Value);
    } else {
      return UCLiteral.size() > 4 || UCLiteral.getAsInteger(16, Value);
    }
  }

  // \Unnnnnnnn
  if (Content[0] == 'U') {
    StringRef UCLiteral = Content.drop_front();
    return UCLiteral.size() > 8 || UCLiteral.getAsInteger(0x10, Value);
  }

  // \N{name}
  if (Content.starts_with("N{") && Content.ends_with("}")) {
    StringRef NCLiteral = Content.drop_front(2).drop_back();
    auto V = NCMap.lookup(NCLiteral);
    if (V.has_value()) {
      Value = *V;
      return false;
    } else
      return true;
  }

  return true;
}

AsmToken MMIXALLexer::LexSingleQuote() {
  const auto CharVal = getNextChar(); // always consume char '
  if (isLineEndChar(CharVal)) {
    return ReturnError(TokStart, "incomplete character constant!");
  }
  if (getNextChar() != '\'') {
    return ReturnError(TokStart, "illegal character constant!");
  }
  StringRef TokStr(TokStart, CurPtr - TokStart);
  return AsmToken(AsmToken::Integer, TokStr);
}

AsmToken MMIXALLexer::LexSingleQuoteWithEscapeSequence() {
  auto CharLiteralStart = CurPtr;
  auto CurChar = getNextChar(); // always consume one char
  while (peekNextChar() != '\'' && !isLineEndChar(peekNextChar())) {
    CurChar = getNextChar();
  }
  if (isLineEndChar(CurChar)) {
    return ReturnError(TokStart, "incomplete character constant!");
  }
  assert(peekNextChar() == '\'' && "post condition is not satisfied!");
  StringRef CharLiteral(CharLiteralStart, CurPtr - CharLiteralStart);
  getNextChar(); // consume '
  StringRef Res = StringRef(TokStart, CurPtr - TokStart);
  if (CharLiteral.size() == 1) {
    return AsmToken(AsmToken::Integer, Res, CharLiteral[0]);
  }

  if (!StrictMode) {
    APInt Value(64, 0);
    if (CharLiteral[0] == '\\' && LexEscapeSequence(CharLiteral, Value))
      return ReturnError(TokStart, "illegal escape sequence!");
    else {
      // it is a unicode character
      if (isLegalUTF8Sequence(CharLiteral.bytes_begin(),
                              CharLiteral.bytes_end())) {
        const UTF8 *Target8 = CharLiteral.bytes_begin();
        UTF32 buf32[1];
        UTF32 *Target32 = &buf32[0];
        auto Result =
            ConvertUTF8toUTF32(&Target8, CharLiteral.bytes_end(), &Target32,
                               Target32 + 1, strictConversion);
        if (Result == conversionOK) {
          Value = buf32[0];
        } else {
          return ReturnError(TokStart, "illegal unicode character constant!");
        }
      } else {
        return ReturnError(TokStart, "illegal UTF8 constant!");
      }
    }
    return AsmToken(AsmToken::Integer, Res, Value);
  } else {
    return ReturnError(TokStart, "illegal character constant!");
  }
}

AsmToken MMIXALLexer::LexQuote() {
  int CurChar = getNextChar();
  while (CurChar != '"') {
    if (!StrictMode) {
      // TODO: handle escape sequence, TBD...
    }
    CurChar = getNextChar();
    if (isLineEndChar(CurChar))
      return ReturnError(TokStart, "incomplete string constant!");
  }
  return AsmToken(AsmToken::String, StringRef(TokStart, CurPtr - TokStart));
}

size_t MMIXALLexer::peekTokens(MutableArrayRef<AsmToken> Buf,
                               bool ShouldSkipSpace) {
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

StringRef MMIXALLexer::LexUntilEndOfStatement() {
  TokStart = CurPtr;
  while (peekNextChar() != ';' && !isLineEndChar(peekNextChar())) {
    ++CurPtr;
  }
  return StringRef(TokStart, CurPtr - TokStart);
}

AsmToken MMIXALLexer::LexToken() {
  if (RegardAsComment) {
    RegardAsComment = false;
    return LexLineComment();
  }
  TokStart = CurPtr;
  // This always consumes at least one character.
  auto CurChar = getNextChar();

  // lex line direcitive
  if (!IsPeeking && IsAtStartOfLine) {
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
        StringRef S = LexUntilEndOfLine();
        UnLex(TokenBuf[1]);
        UnLex(TokenBuf[0]);
        return AsmToken(AsmToken::HashDirective, S);
      } else {
        // else it is line comment
        return LexLineComment();
      }
    } else if (CurChar != EOF && !isspace(CurChar) &&
               !isIdentifierChar(CurChar)) {
      // otherwise if line start with non-identifier char it is a
      // line comment
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
      return LexDecDigitOrIdentifier();
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
  case '@':
    return AsmToken(AsmToken::At, StringRef(TokStart, 1));
  case ';':
    IsAtStartOfStatement = true;
    return AsmToken(AsmToken::EndOfStatement, StringRef(TokStart, 1));
  }

  return AsmToken();
}

void MMIXALLexer::regardAsComment() { RegardAsComment = true; }

void MMIXALLexer::setBuffer(StringRef Buf, const char *ptr,
                            bool EndStatementAtEOF) {
  CurBuf = Buf;

  if (ptr)
    CurPtr = ptr;
  else
    CurPtr = CurBuf.begin();

  TokStart = nullptr;
  this->EndStatementAtEOF = EndStatementAtEOF;
}

MMIXALLexer::MMIXALLexer(const MCAsmInfo &MAI, const bool &StrictMode)
    : StrictMode(StrictMode) {}

} // namespace llvm
