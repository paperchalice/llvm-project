//===-- MMIXAsmParser.cpp - Parse MMIX assembly to MCInst instructions --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MMIXInstPrinter.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MCTargetDesc/MMIXTargetStreamer.h"
#include "MMIXInstrInfo.h"
#include "MMIXOperand.h"
#include "TargetInfo/MMIXTargetInfo.h"
#include "Utils/MMIXBaseInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "mmix-asm-parser"

namespace {

/// \brief LLVM MC style ASM parser for MMIX
///
/// The syntax is strive to keep compatible with
/// mmixal from mmixware, but due to the LLVM MC constrains,
/// lots of cases are incompatible.
/// Pseudo instructions like IS are handled by MC style directive,
/// so they must have a dot prefix.
/// PREFIX pseudo is not supported, because change MCSymbol name currently is
/// impossible. LLVM MCAsmParser always requires label has the form `<identifier:>`
/// so mmixal style label is not possible, as well as namespace prefix.
/// Also, mmixal is whitespace sensitive, that is not the case of MCAsmParser.
/// Directional label handling is also incompatible.
class MMIXAsmParser : public MCTargetAsmParser {
public:
  enum MMIXMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "MMIXGenAsmMatcher.inc"
  };

  /// @name Auto-generated Match Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "MMIXGenAsmMatcher.inc"

  /// }

  // MCTargetAsmParser interface methods
public:
  void Initialize(MCAsmParser &Parser) override;
  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;

  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc) override;

  void onLabelParsed(MCSymbol *Symbol) override;

  ParseStatus parseDirective(AsmToken DirectiveID) override;

public:
  ParseStatus parseBranchDest(OperandVector &Operands);

  // Directive parsers
private:
  ParseStatus parseGREG(SMLoc Loc);
  ParseStatus parsePREFIX(SMLoc Loc);
  ParseStatus parseBSPEC(SMLoc Loc);
  ParseStatus parseESPEC(SMLoc Loc);
  ParseStatus parseIS(SMLoc Loc);
  ParseStatus parseLOCAL(SMLoc Loc);

  void assignIndex(MCSymbol &Symbol);

  // MMIX specific expression parser
  bool parseExpression(const MCExpr *&Res, SMLoc &EndLoc);
  bool parseExpression(const MCExpr *&Res) {
    SMLoc Loc;
    return parseExpression(Res, Loc);
  }
  bool parseBinOpRHS(unsigned Precedence, const MCExpr *&Res, SMLoc &EndLoc);
  bool isRegExpr(const MCExpr *Expr);

  int CurGReg = 254;
  std::vector<const MCExpr *> GRegVals;
  DenseSet<MCRegister> LocalRegsNeedCheck;

  uint32_t SerialCount = 2;
  uint32_t genSerialCount() { return SerialCount++; }

  MMIXTargetStreamer &getTargetStreamer() {
    return *static_cast<MMIXTargetStreamer *>(
        getParser().getStreamer().getTargetStreamer());
  }

public:
  MMIXAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII);
};

} // namespace

#define DEBUG_TYPE "mmix-asm-parser"

#define GET_REGISTER_MATCHER
// #define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
// #define GET_MNEMONIC_CHECKER
#include "MMIXGenAsmMatcher.inc"

// return 0(false) means success;

// ctor
MMIXAsmParser::MMIXAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII)
    : MCTargetAsmParser(STI, MII) {}

void MMIXAsmParser::Initialize(MCAsmParser &Parser) {
  MCAsmParserExtension::Initialize(Parser);

  static const StringMap<std::int64_t> PreDefs = {
      {"ROUND_CURRENT", 0},
      {"ROUND_OFF", 1},
      {"ROUND_UP", 2},
      {"ROUND_DOWN", 3},
      {"ROUND_NEAR", 4},

      {"Inf", 0x7FF0'0000'0000'0000},
      {"Data_Segment", 0x2000'0000'0000'0000},
      {"Pool_Segment", 0x4000'0000'0000'0000},
      {"Stack_Segment", 0x6000'0000'0000'0000},

      {"D_BIT", 0x80},
      {"V_BIT", 0x40},
      {"W_BIT", 0x20},
      {"I_BIT", 0x10},
      {"O_BIT", 0x08},
      {"U_BIT", 0x04},
      {"Z_BIT", 0x02},
      {"X_BIT", 0x01},

      {"D_Handler", 0x10},
      {"V_Handler", 0x20},
      {"W_Handler", 0x30},
      {"I_Handler", 0x40},
      {"O_Handler", 0x50},
      {"U_Handler", 0x60},
      {"Z_Handler", 0x70},
      {"X_Handler", 0x80},

      {"StdIn", 0},
      {"StdOut", 1},
      {"StdErr", 2},
      {"TextRead", 0},
      {"TextWrite", 1},
      {"BinaryRead", 2},
      {"BinaryWrite", 3},
      {"BinaryReadWrite", 4},

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
  };
  MCContext &Context = getContext();
  for (const auto &PreDef : PreDefs) {
    MCSymbol *Symbol = Context.getOrCreateSymbol(PreDef.getKey());
    Symbol->setVariableValue(
        MCConstantExpr::create(PreDef.getValue(), Context));
    Symbol->setRedefinable(true);
  }

  // MMIXAL requires special registers are predefined constants
  const MCRegisterClass &SPRClass = getMMIXMCRegisterClass(MMIX::SPRRegClassID);
  for (unsigned I = 0, E = SPRClass.getNumRegs(); I != E; ++I) {
    MCRegister Reg = MMIX::getSPRFromEnc(I);
    MCSymbol *Symbol =
        Context.getOrCreateSymbol(MMIXInstPrinter::getRegisterName(Reg));
    Symbol->setVariableValue(MCConstantExpr::create(I, Context));
    Symbol->setRedefinable(true);
  }

  static const StringMap<StringRef> AliasMap = {
      {".short", ".WYDE"},
      {".int", ".TETRA"},
  };
  for (const auto &AliasKV : AliasMap) {
    StringRef Alias = AliasKV.getKey();
    StringRef Directive = AliasKV.getValue();
    Parser.addAliasForDirective(Directive, Alias);
    Parser.addAliasForDirective(Directive.lower(), Alias);
  }
  Parser.addAliasForDirective(".LOC", ".org");
}

bool MMIXAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (MatchResult) {
  default:
    return Error(IDLoc, "MMIXAsmParser::matchAndEmitInstruction match failed!");
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_InvalidOperand: {
    // Any time we get here, there's nothing fancy to
    // do. Just get the
    // operand SMLoc and display the diagnostic.
    SMLoc ErrorLoc = ((MMIXOperand &)*Operands[ErrorInfo]).getStartLoc();
    if (ErrorLoc == SMLoc())
      ErrorLoc = IDLoc;
    return Error(ErrorLoc, "invalid operand");
  }
  case Match_MnemonicFail: {
    std::string Suggestion = MMIXMnemonicSpellCheck(
        ((MMIXOperand &)*Operands[0]).getToken(),
        ComputeAvailableFeatures(STI->getFeatureBits()), 0);
    return Error(IDLoc, "unrecognized instruction mnemonic" + Suggestion);
  }
  }
  return false;
}

bool MMIXAsmParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  ParseStatus MatchResult = tryParseRegister(RegNo, StartLoc, EndLoc);
  return !MatchResult.isSuccess();
}

ParseStatus MMIXAsmParser::tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                            SMLoc &EndLoc) {

  AsmToken RegTok = getTok();
  RegNo = MatchRegisterName(RegTok.getString());
  if (RegNo == MMIX::NoRegister)
    return ParseStatus::NoMatch;
  Lex(); // eat register token
  return ParseStatus::Success;
}

bool MMIXAsmParser::parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  // AsmLexer &Lexer = getLexer();
  // MCContext &Ctx = getContext();
  // MCStreamer &Streamer = getStreamer();
  // try pseudos
  // {
  // because `Name` is always lowercase, so it is impossible
  // to implement IS correctly with LLVM MC.
  // e.g.
  // FOO is $1 will be foo = $1, which is unexpected.
  // }

  // eat the mnemonic
  Operands.push_back(MMIXOperand::createMnemonic(Name, NameLoc));

  while (getTok().isNot(AsmToken::EndOfStatement)) {
    if (getTok().is(AsmToken::Comma)) {
      // eat comma
      Lex();
    }

    // operand is always expression
    const MCExpr *Expr = nullptr;
    auto StartLoc = getTok().getLoc();
    ParseStatus CustomMatchResult = MatchOperandParserImpl(Operands, Name);
    if (CustomMatchResult.isSuccess())
      continue;
    SMLoc EndLoc;
    bool HasError = parseExpression(Expr, EndLoc);

    if (HasError)
      return true;

    if (const auto *TE = dyn_cast<MMIXMCExpr>(Expr)) {
      if (TE->isRegExpr())
        Operands.push_back(
            MMIXOperand::createReg(TE->getMCReg(), StartLoc, EndLoc));
    } else if (const auto *E = dyn_cast<MCConstantExpr>(Expr)) {
      Operands.push_back(
          MMIXOperand::createImm(E->getValue(), StartLoc, EndLoc));
    } else {
      Operands.push_back(MMIXOperand::createBranchDest(Expr, StartLoc, EndLoc));
    }
  }
  return false;
}

ParseStatus MMIXAsmParser::parseBranchDest(OperandVector &Operands) {
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = parseExpression(Expr);
  auto EndLoc = getTok().getLoc();

  if (HasError)
    return ParseStatus::Failure;
  Operands.push_back(MMIXOperand::createBranchDest(Expr, StartLoc, EndLoc));
  return ParseStatus::Success;
}

/// See mmixal document
bool MMIXAsmParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  // Parse the expression.
  Res = nullptr;
  if (parsePrimaryExpr(Res, EndLoc) || parseBinOpRHS(1, Res, EndLoc))
    return true;

  // Try to constant fold it up front, if possible. Do not exploit
  // assembler here.
  bool IsRegExpr = isRegExpr(Res);
  int64_t Value;
  if (Res->evaluateAsAbsolute(Value)) {
    MCContext &Ctx = getContext();
    if (IsRegExpr)
      Res = MMIXMCExpr::createRegExpr(Value, Ctx);
    else
      Res = MCConstantExpr::create(Value, Ctx);
  } else if (IsRegExpr) {
    return Error(EndLoc, "register expression is not constant");
  }

  return false;
}

static unsigned getBinOpPrecedence(AsmToken::TokenKind K,
                                   MCBinaryExpr::Opcode &Kind) {
  switch (K) {
  default:
    return 0; // not a binop

  // weak operator: +,-,|,^
  case AsmToken::Plus:
    Kind = MCBinaryExpr::Add;
    return 5;
  case AsmToken::Minus:
    Kind = MCBinaryExpr::Sub;
    return 5;
  case AsmToken::Pipe:
    Kind = MCBinaryExpr::Or;
    return 5;

  case AsmToken::Caret:
    Kind = MCBinaryExpr::Xor;
    return 5;

  // strong operator: *,/,//,%,<<,>>,&
  case AsmToken::Star:
    Kind = MCBinaryExpr::Mul;
    return 6;
  case AsmToken::Slash:
    Kind = MCBinaryExpr::Div;
    return 6;
  case AsmToken::Percent:
    Kind = MCBinaryExpr::Mod;
    return 6;
  case AsmToken::LessLess:
    Kind = MCBinaryExpr::Shl;
    return 6;
  case AsmToken::GreaterGreater:
    Kind = MCBinaryExpr::LShr;
    return 6;
  case AsmToken::Amp:
    Kind = MCBinaryExpr::And;
    return 6;
  }
}

bool MMIXAsmParser::parseBinOpRHS(unsigned Precedence, const MCExpr *&Res,
                                  SMLoc &EndLoc) {
  AsmLexer &Lexer = getLexer();
  SMLoc StartLoc = Lexer.getLoc();
  bool IsFracDiv = false;
  while (true) {
    if (Lexer.getKind() == AsmToken::Slash &&
        Lexer.peekTok(/*ShouldSkipSpace=*/true).is(AsmToken::Slash)) {
      Lex(); // eat extra /
      IsFracDiv = true;
    }

    MCBinaryExpr::Opcode Kind = MCBinaryExpr::Add;
    unsigned TokPrec = getBinOpPrecedence(Lexer.getKind(), Kind);

    // If the next token is lower precedence than we are allowed to eat, return
    // successfully with what we ate already.
    if (TokPrec < Precedence)
      return false;

    Lex();

    // Eat the next primary expression.
    const MCExpr *RHS;
    if (parsePrimaryExpr(RHS, EndLoc))
      return true;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    MCBinaryExpr::Opcode Dummy;
    unsigned NextTokPrec = getBinOpPrecedence(Lexer.getKind(), Dummy);
    if (TokPrec < NextTokPrec && parseBinOpRHS(TokPrec + 1, RHS, EndLoc))
      return true;

    // Merge LHS and RHS according to operator.
    if (IsFracDiv)
      Res = MMIXMCExpr::createFracDivExpr(Res, RHS, getContext());
    else
      Res = MCBinaryExpr::create(Kind, Res, RHS, getContext(), StartLoc);
  }
}

bool MMIXAsmParser::isRegExpr(const MCExpr *Expr) {
  MCExpr::ExprKind Kind = Expr->getKind();
  switch (Kind) {
  default:
    return false;
  case MCExpr::Target: {
    const auto *TExpr = dyn_cast<MMIXMCExpr>(Expr);
    return TExpr->isRegExpr();
  }
  case MCExpr::SymbolRef: {
    const auto *SRExpr = dyn_cast<MCSymbolRefExpr>(Expr);
    const MCSymbol &Symbol = SRExpr->getSymbol();
    if (!Symbol.isVariable())
      return false;
    return isRegExpr(Symbol.getVariableValue());
  }
  case MCExpr::Binary:
    const auto *BinExpr = dyn_cast<MCBinaryExpr>(Expr);
    MCBinaryExpr::Opcode Opc = BinExpr->getOpcode();
    const MCExpr *LHS = BinExpr->getLHS(), *RHS = BinExpr->getRHS();
    switch (Opc) {
    default:
      Warning(BinExpr->getLoc(), "unexpected binop, treat as immediate");
      return false;
    case MCBinaryExpr::Add:
      return isRegExpr(LHS) ^ isRegExpr(RHS);
    case MCBinaryExpr::Sub: {
      bool IsLHSRegExpr = isRegExpr(LHS), IsRHSRegExpr = isRegExpr(LHS);
      if (!IsLHSRegExpr && IsRHSRegExpr) {
        Warning(BinExpr->getLoc(),
                "reg - pure is not well defined, treat as immediate");
        return false;
      }
      return IsLHSRegExpr && !IsRHSRegExpr;
    }
    }
  }
}

// MMIX has following extra primaryexpr:
// primaryexpr ::= $ primaryexpr | & symbolrefexpr
bool MMIXAsmParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc) {
  AsmToken::TokenKind FirstTokenKind = getLexer().getKind();
  AsmLexer &Lexer = getLexer();
  MCStreamer &Out = getStreamer();
  MCContext &Ctx = getContext();
  MCAsmParser &Parser = getParser();
  switch (FirstTokenKind) {
  case AsmToken::At: { // MMIXAL uses @ as PC
    MCSymbol *Sym = Ctx.createTempSymbol();
    Out.emitLabel(Sym);
    Res = MCSymbolRefExpr::create(Sym, getContext());
    EndLoc = Lexer.getTok().getEndLoc();
    Lex(); // Eat identifier.
    return false;
  }
  case AsmToken::Amp: {
    Lex(); // Eat &.
    const MCExpr *Expr;
    SMLoc SerialEndLoc;
    SMLoc CurLoc = Lexer.getLoc();
    Parser.parsePrimaryExpr(Expr, SerialEndLoc);
    const auto *Ref = dyn_cast<MCSymbolRefExpr>(Expr);
    if (!Ref)
      return Error(CurLoc, "expect a symbol ref");
    const MCSymbol &Symbol = Ref->getSymbol();
    Res = MCConstantExpr::create(Symbol.getIndex(), Ctx);
    return false;
  }
  case AsmToken::Dollar: {
    Lex(); // Eat $.
    const MCExpr *Expr;
    if (parsePrimaryExpr(Expr, EndLoc))
      return true;
    Res = MMIXMCExpr::createRegExpr(Expr, Ctx);
    return false;
  }
  case AsmToken::LParen: {
    Lex();
    parseExpression(Res, EndLoc);
    return getParser().parseToken(AsmToken::RParen, "expect (");
  }
  default:
    break;
  }

  bool Result = MCTargetAsmParser::parsePrimaryExpr(Res, EndLoc);
  if (!Result) {
    if (const auto *Ref = dyn_cast<MCSymbolRefExpr>(Res))
      assignIndex(const_cast<MCSymbol &>(Ref->getSymbol()));
  }
  return Result;
}

void MMIXAsmParser::onLabelParsed(MCSymbol *Symbol) { assignIndex(*Symbol); }

ParseStatus MMIXAsmParser::parseDirective(AsmToken DirectiveID) {
  StringRef IDVal = DirectiveID.getIdentifier();
  SMLoc Loc = DirectiveID.getLoc();

  auto DirectiveParser =
      StringSwitch<ParseStatus (MMIXAsmParser::*)(SMLoc)>(IDVal)
          .Case(".PREFIX", &MMIXAsmParser::parsePREFIX)
          .Case(".GREG", &MMIXAsmParser::parseGREG)
          .Case(".BSPEC", &MMIXAsmParser::parseBSPEC)
          .Case(".ESPEC", &MMIXAsmParser::parseESPEC)
          .Case(".IS", &MMIXAsmParser::parseIS)
          .Case(".LOCAL", &MMIXAsmParser::parseLOCAL)
          .Default(nullptr);
  if (!DirectiveParser)
    return ParseStatus::NoMatch;
  return (this->*DirectiveParser)(Loc);
}

// ::= .GREG expr[, identifier]
ParseStatus MMIXAsmParser::parseGREG(SMLoc Loc) {
  MCAsmParser &Parser = getParser();
  AsmLexer &Lexer = getLexer();
  const MCExpr *Expr;
  if (parseExpression(Expr))
    return Error(Lexer.getLoc(), "expect expression");

  StringRef Name;
  if (Lexer.getTok().is(AsmToken::Comma)) {
    Lex(); // eat comma
    bool Result = Parser.parseIdentifier(Name);
    if (Result)
      return Error(Lexer.getLoc(), "expect identifier or quoted identifier");
    MCContext &Ctx = getContext();
    MCSymbol *Symbol = Ctx.getOrCreateSymbol(Name);
    Symbol->setVariableValue(MMIXMCExpr::createRegExpr(
        MCConstantExpr::create(254 - GRegVals.size(), Ctx), Ctx));
  }
  GRegVals.push_back(Expr);
  getTargetStreamer().emitGREG(*Expr, Name);
  return parseEOL();
}

// ::= .PREFIX identifier
ParseStatus MMIXAsmParser::parsePREFIX(SMLoc Loc) {
  MCAsmParser &Parser = getParser();
  AsmLexer &Lexer = getLexer();

  StringRef NewPrefix;
  std::string Prefix = getTargetStreamer().getPrefix().str();
  bool Result = Parser.parseIdentifier(NewPrefix);
  if (Result)
    return Error(Lexer.getLoc(), "expect identifier or quoted identifier");
  if (NewPrefix.starts_with(':'))
    Prefix = NewPrefix.drop_front();
  else
    Prefix += NewPrefix;
  getTargetStreamer().emitPREFIX(Prefix);
  return parseEOL() || Error(Loc, ".PREFIX is not supported with LLVM MC");
}

ParseStatus MMIXAsmParser::parseBSPEC(SMLoc Loc) {
  AsmLexer &Lexer = getLexer();
  MCStreamer &Streamer = getStreamer();
  MCContext &Ctx = getContext();

  Streamer.pushSection();
  const MCExpr *Expr;
  parseExpression(Expr);
  const auto *CExpr = dyn_cast<MCConstantExpr>(Expr);
  if (!CExpr)
    return Error(Lexer.getLoc(), "expect constant expression");
  uint16_t Val = CExpr->getValue();
  bool IsELF =
      getContext().getSubtargetInfo()->getTargetTriple().isOSBinFormatELF();
  if (IsELF) {
    MCSection *RegContents = Ctx.getELFSection(
        ".MMIX.spec_data." + std::to_string(Val), ELF::SHT_PROGBITS, 0);
    Streamer.switchSection(RegContents);
  }
  return ParseStatus::Success;
}

ParseStatus MMIXAsmParser::parseESPEC(SMLoc Loc) {
  getStreamer().popSection();
  getParser().eatToEndOfStatement();
  return ParseStatus::Success;
}

ParseStatus MMIXAsmParser::parseIS(SMLoc Loc) {
  MCAsmParser &Parser = getParser();
  MCContext &Ctx = getContext();
  MCStreamer &Streamer = getStreamer();
  StringRef Name;
  if (Parser.check(Parser.parseIdentifier(Name), "expected identifier") ||
      Parser.parseComma())
    return ParseStatus::Failure;
  const MCExpr *Expr;
  if (parseExpression(Expr))
    return ParseStatus::Failure;

  Streamer.emitAssignment(Ctx.getOrCreateSymbol(Name), Expr);
  return ParseStatus::Success;
}

ParseStatus MMIXAsmParser::parseLOCAL(SMLoc Loc) {
  const MCExpr *Expr;
  if (parseExpression(Expr))
    return ParseStatus::Failure;
  AsmLexer &Lexer = getLexer();
  if (const auto *RegExpr = dyn_cast<MMIXMCExpr>(Expr)) {
    if (RegExpr->isRegExpr()) {
      LocalRegsNeedCheck.insert(RegExpr->getMCReg());
      return ParseStatus::Success;
    }
  }
  Error(Lexer.getLoc(), "expect register expression");
  return ParseStatus::Failure;
}

void MMIXAsmParser::assignIndex(MCSymbol &Symbol) {
  StringRef Name = Symbol.getName();
  if (Symbol.getIndex() != 0 || Symbol.isTemporary() || Name.empty())
    return;
  uint32_t Index = Name == "Main" ? 1 : genSerialCount();
  Symbol.setIndex(Index);
}

/// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXAsmParser() {
  RegisterMCAsmParser<MMIXAsmParser> TheAsmParser(getTheMMIXTarget());
}
