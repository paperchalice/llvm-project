#include "MMIXALParser.h"
#include "MMIXMCAsmInfoMMIXAL.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/Support/FormatVariadic.h"

#define DEBUG_TYPE "mmixal"

namespace llvm {

static StringRef QUOTE("\x98\x00\00\01", 4);
static struct {
  StringRef Name;
  uint64_t Value;
} Predefs[] = {
    // ROUND
    {"ROUND_CURRENT", 0},
    {"ROUND_OFF", 1},
    {"ROUND_UP", 2},
    {"ROUND_DOWN", 3},
    {"ROUND_NEAR", 4},
    // constant
    {"Inf", 0x7FFUL << 52},
    // segment
    {"Data_Segment", 2UL << 60},
    {"Pool_Segment", 4UL << 60},
    {"Stack_Segment", 6UL << 60},
    // BIT
    {"D_BIT", 1 << 7},
    {"V_BIT", 1 << 6},
    {"W_BIT", 1 << 5},
    {"I_BIT", 1 << 4},
    {"O_BIT", 1 << 3},
    {"U_BIT", 1 << 2},
    {"Z_BIT", 1 << 1},
    {"X_BIT", 1 << 0},
    // Handler
    {"D_Handler", 0x10},
    {"V_Handler", 0x20},
    {"W_Handler", 0x30},
    {"I_Handler", 0x40},
    {"O_Handler", 0x50},
    {"U_Handler", 0x60},
    {"Z_Handler", 0x70},
    {"X_Handler", 0x80},
    // IO
    {"StdIn", 0},
    {"StdOut", 1},
    {"StdErr", 2},
    // IO mode
    {"TextRead", 0},
    {"TextWrite", 1},
    {"BinaryRead", 2},
    {"BinaryWrite", 3},
    {"BinaryReadWrite", 4},
    // special function
    {"Halt", 0},
    {"Fopen", 1},
    {"Fclose", 2},
    {"Fread", 3},
    {"Fgets", 4},
    {"Fgetws", 5},
    {"Fwrite", 6},
    {"Fputs", 7},
    {"Fputws", 8},
    {"Fseek", 9},
    {"Ftell", 10},
    // special register
    {"rB", 0},
    {"rD", 1},
    {"rE", 2},
    {"rH", 3},
    {"rJ", 4},
    {"rM", 5},
    {"rR", 6},
    {"rBB", 7},
    {"rC", 8},
    {"rN", 9},
    {"rO", 10},
    {"rS", 11},
    {"rI", 12},
    {"rT", 13},
    {"rTT", 14},
    {"rK", 15},
    {"rQ", 16},
    {"rU", 17},
    {"rV", 18},
    {"rG", 19},
    {"rL", 20},
    {"rA", 21},
    {"rF", 22},
    {"rP", 23},
    {"rW", 24},
    {"rX", 25},
    {"rY", 26},
    {"rZ", 27},
    {"rWW", 28},
    {"rXX", 29},
    {"rYY", 30},
    {"rZZ", 31}};

static bool isGPRExpr(const MCExpr *Expr) {
  if (dyn_cast<MCTargetExpr>(Expr)) {
    return true;
  }

  if (auto E = dyn_cast<MCSymbolRefExpr>(Expr)) {
    auto MMOSymbol = cast<MCSymbolMMO>(&E->getSymbol());
    return MMOSymbol->isReg();
  }

  if (const auto *BinExpr = dyn_cast<MCBinaryExpr>(Expr)) {
    switch (BinExpr->getOpcode()) {
    default:
      return false;
    case MCBinaryExpr::Add:
      return isGPRExpr(BinExpr->getLHS()) != isGPRExpr(BinExpr->getRHS());
    case MCBinaryExpr::Sub: {
      bool IsRegLHS = isGPRExpr(BinExpr->getLHS());
      bool IsRegRHS = isGPRExpr(BinExpr->getRHS());
      if (IsRegLHS && IsRegRHS)
        return false;
      else if (IsRegLHS && !IsRegRHS)
        return true;
      else
        return false;
    }
    }
  } else {
    return false;
  }
}

const AsmToken &MMIXALParser::Lex() { return getLexer().Lex(); }

bool MMIXALParser::parseIdentifier(StringRef &Res) {
  auto CurTok = getTok();
  if (CurTok.isNot(AsmToken::Identifier))
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

std::string MMIXALParser::getQualifiedName(StringRef Name) {
  if (Name.starts_with(":")) {
    // it is a qualified name
    return Name.str();
  } else {
    return CurPrefix + Name.str();
  }
}

void MMIXALParser::resolveLabel(MCSymbol *Symbol) {
  if (!getTargetParser().getTargetOptions().MCRelaxAll)
    return;

  for (auto Fixup : SharedInfo.FixupList) {
    if (Symbol == Fixup.Symbol) {
      switch (Fixup.Kind) {
      case MMO::FixupInfo::FixupKind::FIXUP_OCTA: {
        char HighByte = Fixup.Addr >> 56;
        std::uint64_t Addr = Fixup.Addr & ((1UL << 56) - 1);
        char Z = 1;
        Out.emitBinaryData(StringRef("\x98\x03", 2));
        Out.emitBinaryData(StringRef(&HighByte, 1));
        char Data[4];
        uint32_t AddrHigh = Hi_32(Addr);
        if (AddrHigh) {
          Z = 2;
          Out.emitBinaryData(StringRef(&Z, 1));
          support::endian::write32be(Data, AddrHigh);
          Out.emitBinaryData(StringRef(Data, 4));
        } else {
          Out.emitBinaryData(StringRef(&Z, 1));
        }
        uint32_t AddrLow = Lo_32(Addr);
        support::endian::write32be(Data, AddrLow);
        Out.emitBinaryData(StringRef(Data, 4));
        break;
      }

      case MMO::FixupInfo::FixupKind::FIXUP_WYDE:
        if (SharedInfo.PC >= Fixup.Addr) {
          auto Delta = (SharedInfo.PC - Fixup.Addr) / 4;
          if (Delta > UINT16_MAX) {
            return;
          }
          Out.emitBinaryData(StringRef("\x98\x04", 2));
          char Data[2];
          support::endian::write16be(Data, Delta);
          Out.emitBinaryData(StringRef(Data, 2));
        } else {
          Out.emitBinaryData(StringRef("\x98\x05\x00\x10", 4));
          uint32_t Delta = (SharedInfo.PC - Fixup.Addr) / 4;
          char Data[4];
          support::endian::write32be(Data, Delta);
          Data[0] = 1;
          Data[1] = 0;
          Out.emitBinaryData(StringRef(Data, 4));
        }
        break;
      case MMO::FixupInfo::FixupKind::FIXUP_JUMP: {
        std::int64_t Delta =
            static_cast<std::int64_t>(SharedInfo.PC - Fixup.Addr) / 4;
        if (Delta >= 0 && Delta <= UINT16_MAX) {
          Out.emitBinaryData(StringRef("\x98\x04", 2));
          char Data[2];
          support::endian::write16be(Data, static_cast<std::uint16_t>(Delta));
          Out.emitBinaryData(StringRef(Data, 2));
        } else {
          Out.emitBinaryData(StringRef("\x98\x05\x00\x18", 4));
          char Data[4];
          support::endian::write32be(Data, Lo_32(Delta));
          if (Delta < 0)
            Data[0] = 1;
          Out.emitBinaryData(StringRef(Data, 4));
        }
      }
      }
    }
  }
}

bool MMIXALParser::parsePseudoOperationIS(StringRef Label) {
  const MCExpr *Res;
  SMLoc EndLoc;
  bool HasErr = parseExpression(Res, EndLoc);
  if (HasErr)
    return true;
  int64_t Val = 0;
  Res->evaluateAsAbsolute(Val);

  if (!Label.empty()) {
    auto Symbol = getContext().getOrCreateSymbol(getQualifiedName(Label));
    auto MMOSymbol = cast<MCSymbolMMO>(Symbol);
    // other target specific kinds are exlusive by jump and branch instruction
    // so we can use it
    if (isGPRExpr(Res)) {
      MMOSymbol->setReg();
      Val %= 256;
    }
    MMOSymbol->setVariableValue(static_cast<std::uint64_t>(Val), Res);
    resolveLabel(Symbol);
  }
  return handleEndOfStatement();
}

bool MMIXALParser::parsePseudoOperationPREFIX() {
  const auto &P = getTok();
  if (P.is(AsmToken::Identifier)) {
    Lex();
    CurPrefix = getQualifiedName(P.getString());
    return handleEndOfStatement();
  } else {
    return !handleEndOfStatement();
  }
}

bool MMIXALParser::parsePseudoOperationGREG(StringRef Label) {
  if (SpecialMode) {
    Error(getLexer().getLoc(), "cannot use `GREG' in special mode!");
    return true;
  }

  if (getTok().isNot(AsmToken::EndOfStatement)) {
    const MCExpr *Res;
    SMLoc EndLoc;
    if (parseExpression(Res, EndLoc))
      return true;
    int64_t RegVal;
    if (!Res->evaluateAsAbsolute(RegVal))
      return true;
    SharedInfo.GregList.emplace_front(static_cast<uint64_t>(RegVal));
  } else {
    SharedInfo.GregList.emplace_front(0);
  }

  if (!Label.empty()) {
    auto Symbol = getContext().getOrCreateSymbol(getQualifiedName(Label));
    auto MMOSymbol = cast<MCSymbolMMO>(Symbol);
    MMOSymbol->setVariableValue(
        getCurGreg(), MCConstantExpr::create(getCurGreg(), getContext()));
    getStreamer().emitSymbolAttribute(Symbol, MCSA_Global);
    MMOSymbol->setReg();
  }
  return handleEndOfStatement();
}

bool MMIXALParser::parsePseudoOperationLOCAL() {
  const MCExpr *Res;
  SMLoc EndLoc;
  bool HasErr = parseExpression(Res, EndLoc);
  if (HasErr)
    return true;
  ++SharedInfo.MMOLine;
  int64_t Reg;
  if (Res->evaluateAsAbsolute(Reg) && Reg < 256) {
    LocalRegList.emplace_back(static_cast<std::uint8_t>(Reg));
  }
  return handleEndOfStatement();
}

bool MMIXALParser::parsePseudoOperationLOC(StringRef Label) {
  const MCExpr *Res;
  SMLoc EndLoc;
  if (parseExpression(Res, EndLoc))
    return true;
  std::int64_t NewLoc;
  if (!Res->evaluateAsAbsolute(NewLoc))
    return true;

  SharedInfo.PC = static_cast<std::uint64_t>(NewLoc);
  return handleEndOfStatement();
}

bool MMIXALParser::parsePseudoOperationBSPEC() {
  if (SpecialMode) {
    Error(getLexer().getLoc(), "cannot use `LOCAL' in special mode!");
    return true;
  }
  const MCExpr *Res;
  SMLoc EndLoc;
  bool HasErr = parseExpression(Res, EndLoc);
  if (HasErr)
    return true;
  int64_t Val;
  bool Success = Res->evaluateAsAbsolute(Val);
  if (!Success)
    return true;
  syncMMO();
  StringRef Bspec("\x98\x08", 2);
  uint16_t ValueToWrite = static_cast<uint16_t>(Val) % 0x1'0000;
  char Data[2];
  support::endian::write16be(Data, ValueToWrite);
  Out.emitBinaryData(Bspec);
  Out.emitBinaryData(StringRef(Data, 2));
  SpecialMode = true;
  return handleEndOfStatement();
}

bool MMIXALParser::parsePseudoOperationOCTA() { return false; }

void MMIXALParser::emitPostamble() {
  Out.emitBinaryData(StringRef("\x98\x0a\x00", 3));
  char Count = 256 - SharedInfo.GregList.size();
  Out.emitBinaryData(StringRef(&Count, 1));
  for (const auto RegVal : SharedInfo.GregList) {
    char Buf[8];
    StringRef SR(Buf, 8);
    support::endian::write64be(Buf, RegVal);
    Out.emitBinaryData(SR);
  }
}

std::uint8_t MMIXALParser::getCurGreg() {
  assert(SharedInfo.GregList.size() >= 1 && "at least 1 greg");
  return 0x100 - SharedInfo.GregList.size();
}

void MMIXALParser::initInternalSymbols() {
  for (auto &Predef : Predefs) {
    auto Symbol = getContext().getOrCreateSymbol(":" + Predef.Name);
    auto MMOSymbol = cast<MCSymbolMMO>(Symbol);
    MMOSymbol->setVariableValue(
        Predef.Value, MCConstantExpr::create(static_cast<int64_t>(Predef.Value),
                                             getContext()));
    Symbol->setIndex(0);
    Out.emitSymbolAttribute(Symbol, MCSA_Internal);
  }
  // special register names are also symbols...
}

MMIXALParser::IdentifierKind MMIXALParser::getIdentifierKind(StringRef Name) {
  if (Name.empty())
    return MMIXALParser::Invalid;
  if (!isdigit(Name[0])) {
    return MMIXALParser::Symbol;
  }
  auto Num = Name.drop_back();
  if (std::all_of(Num.begin(), Num.end(), isdigit)) {
    if (Name.ends_with("H"))
      return MMIXALParser::LocalLabelHere;
    if (Name.ends_with("B"))
      return MMIXALParser::LocalLabelBackward;
    if (Name.ends_with("F"))
      return MMIXALParser::LocalLabelForward;
  }
  return MMIXALParser::Invalid;
}

void MMIXALParser::syncMMO() {
  if (SharedInfo.PC & (0xEUL << 60) || SpecialMode)
    return;

  static StringRef LastFileName;

  if (CurrentFileName != LastFileName) {
    auto FindResult =
        std::find(FileNames.begin(), FileNames.end(), CurrentFileName);
    Out.emitBinaryData({"\x98\x06", 2});
    if (FindResult == FileNames.end()) {
      FileNames.push_back(CurrentFileName);
      char Y = FileNames.size() - 1;
      Out.emitBinaryData({&Y, 1});
      auto StrSize = CurrentFileName.size();
      auto AlignedSize = alignTo<4>(StrSize);
      char TetraCount = AlignedSize / 4;
      Out.emitBinaryData({&TetraCount, 1});
      Out.emitBinaryData(CurrentFileName);
      Out.emitZeros(AlignedSize - StrSize);
    } else {
      char Index = FindResult - FileNames.begin();
      Out.emitBinaryData({&Index, 1});
      Out.emitZeros(1);
    }
    LastFileName = CurrentFileName;
  }

  if (SharedInfo.MMOLine != SharedInfo.CurrentLine) {
    Out.emitBinaryData({"\x98\x07", 2});
    char Buf[2];
    support::endian::write16be(Buf, SharedInfo.CurrentLine);
    Out.emitBinaryData({Buf, 2});
    SharedInfo.MMOLine = SharedInfo.CurrentLine;
  }
}

bool MMIXALParser::parseBinOpRHS(unsigned Precedence, const MCExpr *&Res,
                                 SMLoc &EndLoc) {
  while (true) {
    if (getTok().is(AsmToken::Space)) {
      return false;
    }
    MCBinaryExpr::Opcode Kind = MCBinaryExpr::Add;
    auto TokenKind = Lexer.getKind();
    auto IsSlashSlash =
        getTok().is(AsmToken::Slash) && getTok().getString().size() == 2;
    unsigned TokPrec = getBinOpPrecedence(TokenKind, Kind);
    SMLoc BinOpLoc = getTok().getLoc();

    // If the next token is lower precedence than we are allowed to eat,
    // return successfully with what we ate already.
    if (TokPrec < Precedence)
      return false;

    Lex();

    // Eat the next primary expression.
    const MCExpr *RHS;
    if (parsePrimaryExpr(RHS, EndLoc, nullptr)) {
      const char C = *BinOpLoc.getPointer();
      Error(BinOpLoc, formatv("syntax error after character `{0}'!", C));
      return true;
    }

    // If BinOp binds less tightly with RHS than the operator after RHS,
    // let the pending operator take RHS as its LHS.
    MCBinaryExpr::Opcode Dummy;
    unsigned NextTokPrec = getBinOpPrecedence(Lexer.getKind(), Dummy);
    if (TokPrec < NextTokPrec && parseBinOpRHS(TokPrec + 1, RHS, EndLoc))
      return true;

    // Merge LHS and RHS according to operator.
    if (!IsSlashSlash)
      Res = MCBinaryExpr::create(Kind, Res, RHS, getContext(), BinOpLoc);
    else {
      auto LHS = MCBinaryExpr::createShl(
          MCConstantExpr::create(64, getContext()), Res, getContext());
      Res = MCBinaryExpr::createDiv(LHS, RHS, getContext());
    }
  }
}

void MMIXALParser::syncLOC() {
  auto Delta = SharedInfo.PC - SharedInfo.MMOLoc;
  if (Delta < 4 && !LastIsESPEC) {
    SharedInfo.MMOLoc = SharedInfo.PC;
    return;
  } else if (Delta < 0x1'0000 && !LastIsESPEC) {
    Out.emitBytes(StringRef("\x98\x02", 2));
    char Buf[2];
    support::endian::write16be(Buf, static_cast<std::uint16_t>(Delta));
    Out.emitBytes(StringRef(Buf, 2));
  } else {
    Out.emitBytes(StringRef("\x98\x01", 2));
    char HighByte = SharedInfo.PC >> 56;
    std::uint64_t Loc = SharedInfo.PC & ((1UL << 56) - 1);
    Out.emitBytes(StringRef(&HighByte, 1));
    if (Hi_32(Loc)) {
      Out.emitBytes(StringRef("\x2", 1));
      char Buf[8];
      support::endian::write64be(Buf, Loc);
      Out.emitBytes(StringRef(Buf, sizeof(Buf)));
    } else {
      char Buf[4];
      Out.emitBytes(StringRef("\x1", 1));
      support::endian::write32be(Buf, Lo_32(Loc));
      Out.emitBytes(StringRef(Buf, sizeof(Buf)));
    }
    ++SharedInfo.MMOLine;
  }
  SharedInfo.MMOLoc = SharedInfo.PC;
}

unsigned MMIXALParser::getBinOpPrecedence(AsmToken::TokenKind K,
                                          MCBinaryExpr::Opcode &Kind) {
  constexpr unsigned weak_prec = 10, strong_prec = 20;

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

void MMIXALParser::alignPC(unsigned Alignment) {
  auto NewPC = alignTo(SharedInfo.PC, Alignment);
  SharedInfo.PC = NewPC;
}

// primary expression:
// <primary expression> -> <constant> | <symbol> | <local operand> | @ |
//                         <(expression)> | <unary operator><primary
//                         expression>
// <unary operator> -> + | - | ~ | $ | &
bool MMIXALParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                                    AsmTypeInfo *TypeInfo) {
  SMLoc FirstTokenLoc = getLexer().getLoc();
  auto CurTok = getTok();
  EndLoc = CurTok.getLoc();
  // handle 'At' in target parser
  switch (CurTok.getKind()) {
  case AsmToken::Error:
    return Error(Lexer.getErrLoc(), Lexer.getErr());
  case AsmToken::Integer:
    Res = MCConstantExpr::create(getTok().getIntVal(), getContext());
    Lex();
    return false;
  case AsmToken::Identifier: {
    auto TokString = getTok().getString();
    MCSymbol *Symbol = nullptr;
    auto Kind = getIdentifierKind(TokString);
    if (Kind == IdentifierKind::LocalLabelBackward ||
        Kind == IdentifierKind::LocalLabelForward) {
      unsigned LocalLabelVal;
      bool Before = TokString.ends_with("B");
      if (TokString.drop_back().getAsInteger<unsigned>(10, LocalLabelVal)) {
        return true;
      }
      Symbol = getContext().getDirectionalLocalSymbol(LocalLabelVal, Before);
    } else {
      Symbol = getContext().lookupSymbol(getQualifiedName(TokString));
      if (!Symbol) {
        Symbol = getContext().getOrCreateSymbol(getQualifiedName(TokString));
        Symbol->setIndex(++SerialCnt);
      }
    }

    assert(Symbol && "Invalid symbol!");
    Res = MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None,
                                  getContext(), EndLoc);
    Lex();
    return false;
  }
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
    if (parsePrimaryExpr(Res, EndLoc, TypeInfo)) {
      return true;
    } else {
      Res = MCUnaryExpr::createNot(Res, getContext(), FirstTokenLoc);
      return false;
    }
  case AsmToken::At:
    Lex(); // eat At
    Res = MCConstantExpr::create(static_cast<int64_t>(SharedInfo.PC),
                                 getContext(), true, 8);
    return false;
  case AsmToken::Dollar: // change from pure value to register number
    // here leverage the target MCExpr to handle expressions which
    // involve registers
    Lex(); // eat '$'
    if (getTargetParser().parsePrimaryExpr(Res, EndLoc))
      return true;
    else {
      Res = getTargetParser().createTargetUnaryExpr(Res, AsmToken::Dollar,
                                                    getContext());
      return false;
    }
  case AsmToken::Amp: // take the serial number
    Lex();            // eat &
    {
      auto TokStr = getTok().getString();
      auto Symbol = getContext().lookupSymbol(getQualifiedName(TokStr));
      if (!Symbol) {
        Symbol = getContext().getOrCreateSymbol(getQualifiedName(TokStr));
        Symbol->setIndex(++SerialCnt);
      }
      Res = MCConstantExpr::create(Symbol->getIndex(), getContext());
      Lex();
      return false;
    }
  default:
    return true;
  }
}

bool MMIXALParser::parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  if (parseExpression(Res, EndLoc)) {
    return false;
  }
  return parseRParen();
}

void MMIXALParser::checkLocalRegs() {
  std::uint16_t Bound = getCurGreg();
  for (const auto RegNum : LocalRegList) {
    if (RegNum >= Bound) {
      Error(SMLoc(), "out of local reg bound");
    }
  }
}

// <expression> -> <term> | <expression> <weak operator> <term>
bool MMIXALParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  Res = nullptr;

  if (parsePrimaryExpr(Res, EndLoc, nullptr) || parseBinOpRHS(1, Res, EndLoc))
    return true;
  return false;
}

bool MMIXALParser::parseStatement() {
  // line: ^[label]\s+<instruction>[;\s*<instruction>]\s+[arbitray
  // contents]$ if we get error, discard all rest content until line end
  //
  // label must after newline immediately, because in MMIXAL if we ignore
  // whitespace, the following code snippet
  // ```
  // JMP Dest
  // Main TRAP
  // ```
  // is ambiguous
  bool Failed = false;

  auto CurTok = getTok();
  if (CurTok.is(AsmToken::HashDirective)) {
    Lex(); // skip hash directive
    SharedInfo.CurrentLine = getTok().getIntVal() - 1;
    Lex(); // skip line number
    CurrentFileName = getTok().getStringContents();
    Lex(); // eat line number
    return handleEndOfStatement();
  }

  // parse label
  StringRef Label = "";
  Lexer.setSkipSpace(true);
  if (CurTok.isNot(AsmToken::Space)) {
    // we get a label
    // check if it start with number
    if (parseIdentifier(Label)) {
      Lexer.setSkipSpace(false);
      return handleEndOfStatement();
    }
  } else {
    Lex(); // eat space
  }

  // handle label field
  MCSymbol *LabelSymbol = nullptr;
  if (!Label.empty()) {
    auto Kind = getIdentifierKind(Label);
    if (Kind == IdentifierKind::LocalLabelHere) {
      unsigned Val;
      if (Label.drop_back().getAsInteger<unsigned>(10, Val)) {
        Error(CurTok.getLoc(), "improper local label `12C'!");
        return true;
      }
      LabelSymbol = getContext().createDirectionalLocalSymbol(Val);
      LabelSymbol->setIndex(0);
    } else {
      LabelSymbol = getContext().lookupSymbol(getQualifiedName(Label));
      if (!LabelSymbol) {
        LabelSymbol = getContext().getOrCreateSymbol(getQualifiedName(Label));
        LabelSymbol->setIndex(++SerialCnt);
        getStreamer().emitSymbolAttribute(LabelSymbol, MCSA_Global);
      }
    }
    auto MMOSymbol = cast<MCSymbolMMO>(LabelSymbol);
    getContext().getSymbols();

    // may modify in future
    MMOSymbol->setVariableValue(
        SharedInfo.PC, MCConstantExpr::create(
                           static_cast<int64_t>(SharedInfo.PC), getContext()));
  }

  // parse instruction field
  auto InstTok = getTok();

  if (InstTok.isNot(AsmToken::Identifier)) {
    Warning(InstTok.getLoc(), "ilegal opcode");
    return handleEndOfStatement();
  }

  Lex();                     // eat opcode and white space
  Lexer.setSkipSpace(false); // expressions can't contain space

  // handle pseudo operation
  if (InstTok.getString() == "IS") {
    return parsePseudoOperationIS(Label);
  }
  if (InstTok.getString() == "LOC") {
    return parsePseudoOperationLOC(Label);
  }
  if (InstTok.getString() == "PREFIX") {
    return parsePseudoOperationPREFIX();
  }
  if (InstTok.getString() == "GREG") {
    return parsePseudoOperationGREG(Label);
  }
  if (InstTok.getString() == "LOCAL") {
    return parsePseudoOperationLOCAL();
  }
  if (InstTok.getString() == "BSPEC") {
    syncLOC();
    syncMMO();
    return parsePseudoOperationBSPEC();
  }
  if (InstTok.getString() == "ESPEC") {
    if (auto Rem = DataCounter % 4) {
      Out.emitZeros(4 - Rem);
      SharedInfo.MMOLoc += 4 - Rem;
      SharedInfo.PC += 4 - Rem;
    }
    SpecialMode = false;
    DataCounter = 0;
    LastIsESPEC = true;
    return handleEndOfStatement();
  }

  if (InstTok.getString() == "BYTE") {
    return parsePseudoOperationBWTO<std::uint8_t>();
  }
  if (InstTok.getString() == "WYDE") {
    return parsePseudoOperationBWTO<std::uint16_t>();
  }
  if (InstTok.getString() == "TETRA") {
    return parsePseudoOperationBWTO<std::uint32_t>();
  }
  if (InstTok.getString() == "OCTA") {
    return parsePseudoOperationBWTO<std::uint64_t>();
  }

  // check if in special mode
  if (SpecialMode) {
    Error(getLexer().getLoc(),
          "cannot use `" + InstTok.getString() + "' in special mode!");
    return true;
  }

  // prepare work
  // clear
  if (DataCounter != 0) {
    if (auto Rem = DataCounter % 4) {
      SharedInfo.PC += 4 - Rem;
      Out.emitZeros(4 - Rem);
      ++SharedInfo.MMOLine;
    }
    DataCounter = 0;
  }

  alignPC();
  syncLOC();
  if (LabelSymbol) {
    auto MMOSymbol = cast<MCSymbolMMO>(LabelSymbol);
    MMOSymbol->setVariableValue(
        SharedInfo.PC, MCConstantExpr::create(
                           static_cast<int64_t>(SharedInfo.PC), getContext()));
    resolveLabel(LabelSymbol);
  }
  syncMMO();
  // parse instruciton
  ParseInstructionInfo IInfo;
  SmallVector<std::unique_ptr<MCParsedAsmOperand>, 4> Operands;
  if (getTargetParser().ParseInstruction(IInfo, InstTok.getString().lower(),
                                         InstTok, Operands)) {
    handleEndOfStatement();
    return true;
  }

  uint64_t ErrorInfo;
  unsigned int Opcode;
  Failed = getTargetParser().MatchAndEmitInstruction(
      Operands[0]->getStartLoc(), Opcode, Operands, Out, ErrorInfo,
      getTargetParser().isParsingMSInlineAsm());
  SharedInfo.PC += 4;
  SharedInfo.MMOLoc += 4;
  ++SharedInfo.MMOLine;
  return handleEndOfStatement() || Failed;
}

bool MMIXALParser::handleEndOfStatement() {
  if (getTok().isNot(AsmToken::EndOfStatement)) {
    Lexer.regardAsComment();
    Lex();
    ++SharedInfo.CurrentLine;
    return parseToken(AsmToken::EndOfStatement);
  } else {
    if (getTok().getString().ends_with("\n")) {
      ++SharedInfo.CurrentLine;
    }
    return parseToken(AsmToken::EndOfStatement);
  }
}

void MMIXALParser::Note(SMLoc L, const Twine &Msg, SMRange Range) {}

void MMIXALParser::printMessage(SMLoc Loc, SourceMgr::DiagKind Kind,
                                const Twine &Msg, SMRange Range) const {
  ArrayRef<SMRange> Ranges(Range);
  SrcMgr.PrintMessage(Loc, Kind, Msg, Ranges);
}

bool MMIXALParser::Warning(SMLoc L, const Twine &Msg, SMRange Range) {
  if (getTargetParser().getTargetOptions().MCNoWarn)
    return false;
  if (getTargetParser().getTargetOptions().MCFatalWarnings)
    return Error(L, Msg, Range);
  printMessage(L, SourceMgr::DK_Warning, Msg, Range);
  return false;
}

bool MMIXALParser::printError(SMLoc L, const Twine &Msg, SMRange Range) {
  printMessage(L, SourceMgr::DK_Error, Msg, Range);
  return false;
}

bool MMIXALParser::Run(bool NoInitialTextSection, bool NoFinalize) {
  // init section
  Out.initSections(false, getTargetParser().getSTI());
  // Prime the lexer.
  auto T = Lex();
  getTargetParser().onBeginOfFile();

  while (getTok().isNot(AsmToken::Eof)) {
    ErrorCount += parseStatement();
  }

  if (auto Rem = DataCounter % 4) {
    Out.emitZeros(4 - Rem);
  }

  if (auto MainSymbol =
          dyn_cast<MCSymbolMMO>(getContext().getOrCreateSymbol(":Main"))) {
    *SharedInfo.GregList.rbegin() = MainSymbol->getEquivalent();
  }

  getTargetParser().onEndOfFile();

  printPendingErrors();

  emitPostamble();

  Out.finish(Lexer.getLoc());
  if (ErrorCount == 1) {
    errs() << "(One error were found.)\n";
  } else if (ErrorCount > 1) {
    errs() << '(' << ErrorCount << " error were found.)\n";
  }
  return ErrorCount;
}

std::size_t MMIXALParser::getErrorCount() const { return ErrorCount; }

MMIXALParser::MMIXALParser(SourceMgr &SM, MCContext &Ctx, MCStreamer &Out,
                           const MMIXMCAsmInfoMMIXAL &MAI, raw_ostream &Lst)
    : Lexer(MAI, this->StrictMode), Ctx(Ctx), Out(Out), MAI(MAI), SrcMgr(SM),
      CurBuffer(SM.getMainFileID()), StrictMode(MAI.StrictMode), Lst(Lst) {
  auto MB = SrcMgr.getMemoryBuffer(CurBuffer);
  CurrentFileName = MB->getBufferIdentifier();
  Lexer.setBuffer(MB->getBuffer());
  Lexer.setSkipSpace(false);
  initInternalSymbols();
  auto MainSymb =
      getContext().getOrCreateSymbol(":Main"); // ensure it is the first entry
  MainSymb->setIndex(1);                       // Main always the first symbol
  getStreamer().emitSymbolAttribute(MainSymb, MCSA_Global);
}

MMIXALParser *createMCMMIXALParser(SourceMgr &SM, MCContext &C, MCStreamer &Out,
                                   const MMIXMCAsmInfoMMIXAL &MAI,
                                   raw_ostream &Lst) {
  return new MMIXALParser(SM, C, Out, MAI, Lst);
}

} // namespace llvm
