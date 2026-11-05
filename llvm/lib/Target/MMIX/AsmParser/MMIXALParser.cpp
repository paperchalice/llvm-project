//===-- ALParser.cpp - Parse MMIX assembly language -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXALParser.h"
#include "MCTargetDesc/MMIXInstPrinter.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MCTargetDesc/MMIXTargetStreamer.h"
#include "MMIXOperand.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;
using namespace llvm::MMIX;

ALParser::ALParser(MCContext &Ctx, MCStreamer &Out, SourceMgr &SrcMgr,
                   const MCAsmInfo &MAI)
    : MCAsmParser(Ctx, Out, SrcMgr, MAI) {
  Lexer.setBuffer(SrcMgr.getMemoryBuffer(SrcMgr.getMainFileID())->getBuffer());
}

const ALToken &ALParser::lex() {
  if (Lexer.getTok().is(ALToken::Error))
    Error(Lexer.getErrLoc(), Lexer.getErr());

  // if it's a end of statement with a comment in it
  if (getTok().is(ALToken::EndOfStatement)) {
    // if this is a line comment output it.
    if (!getTok().getString().empty() && getTok().getString().front() != '\n' &&
        getTok().getString().front() != '\r' && MAI.preserveAsmComments())
      Out.addExplicitComment(Twine(getTok().getString()));
  }

  const ALToken &Tok = Lexer.lex();

  return Tok;
}

void ALParser::Note(SMLoc L, const Twine &Msg, SMRange Range) {
  printPendingErrors();
  printMessage(L, SourceMgr::DK_Note, Msg, Range);
}
bool ALParser::Warning(SMLoc L, const Twine &Msg, SMRange Range) {
  if (getTargetParser().getTargetOptions().MCNoWarn)
    return false;
  if (getTargetParser().getTargetOptions().MCFatalWarnings)
    return Error(L, Msg, Range);
  printMessage(L, SourceMgr::DK_Warning, Msg, Range);
  return false;
}
bool ALParser::printError(SMLoc L, const Twine &Msg, SMRange Range) {
  HadError = true;
  printMessage(L, SourceMgr::DK_Error, Msg, Range);
  return true;
}

bool ALParser::parseIdentifier(StringRef &Res) {
  auto CurTok = getTok();
  if (CurTok.isNot(ALToken::Identifier))
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

bool ALParser::parseExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  Res = nullptr;

  if (parsePrimaryExpr(Res, EndLoc, nullptr) || parseBinOpRHS(1, Res, EndLoc))
    return true;
  return false;
}

bool ALParser::parseExpression(const MCExpr *&Res) {
  SMLoc EndLoc;
  return parseExpression(Res, EndLoc);
}

static unsigned getBinOpPrecedence(ALToken::TokenKind Kind, int &OpKind) {
  switch (Kind) {
  default:
    return 0;
  // weak operators
  case ALToken::Plus:
    OpKind = MCBinaryExpr::Add;
    return 1;
  case ALToken::Minus:
    OpKind = MCBinaryExpr::Sub;
    return 1;
  case ALToken::Pipe:
    OpKind = MCBinaryExpr::Or;
    return 1;
  case ALToken::Caret:
    OpKind = MCBinaryExpr::Xor;
    return 1;
  // strong operators
  case ALToken::Star:
    OpKind = MCBinaryExpr::Mul;
    return 2;
  case ALToken::Slash:
    OpKind = MCBinaryExpr::Div;
    return 2;
  case ALToken::SlashSlash:
    OpKind = -1;
    return 2;
  case ALToken::Percent:
    OpKind = MCBinaryExpr::Mod;
    return 2;
  case ALToken::LessLess:
    OpKind = MCBinaryExpr::Shl;
    return 2;
  case ALToken::GreaterGreater:
    OpKind = MCBinaryExpr::LShr;
    return 2;
  case ALToken::Amp:
    OpKind = MCBinaryExpr::And;
    return 2;
  }
}

bool ALParser::parseBinOpRHS(unsigned Precedence, const MCExpr *&Res,
                             SMLoc &EndLoc) {
  SMLoc StartLoc = Lexer.getLoc();
  while (true) {
    int Kind = MCBinaryExpr::Add;
    unsigned TokPrec = getBinOpPrecedence(Lexer.getKind(), Kind);

    // If the next token is lower precedence than we are allowed to eat, return
    // successfully with what we ate already.
    if (TokPrec < Precedence)
      return false;

    lex();

    // Eat the next primary expression.
    const MCExpr *RHS;
    if (getTargetParser().parsePrimaryExpr(RHS, EndLoc))
      return true;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int Dummy;
    unsigned NextTokPrec = getBinOpPrecedence(Lexer.getKind(), Dummy);
    if (TokPrec < NextTokPrec && parseBinOpRHS(TokPrec + 1, RHS, EndLoc))
      return true;

    // Merge LHS and RHS according to operator.
    if (Kind != -1)
      Res = MCBinaryExpr::create((MCBinaryExpr::Opcode)Kind, Res, RHS,
                                 getContext(), StartLoc);
    else
      Res = MMIXMCExpr::createFracDivExpr(Res, RHS, getContext());
  }
  return false;
}

bool ALParser::parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) {
  if (parseExpression(Res, EndLoc)) {
    return false;
  }
  return parseRParen();
}

// primary expression:
// <primary expression> -> <constant> | <symbol> | <local operand> | @ |
//                         <(expression)> | <unary operator><primary
//                         expression>
// <unary operator> -> + | - | ~ | $ | &
bool ALParser::parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                                AsmTypeInfo *TypeInfo) {
  ALToken FirstTok = getLexer().getTok();
  SMLoc FirstTokenLoc = FirstTok.getLoc();
  MCContext &Ctx = getContext();
  switch (FirstTok.getKind()) {
  default:
    return TokError("unknown token in expression");
  // If we have an error assume that we've already handled it.
  case ALToken::Error:
    return true;
  case ALToken::Integer: {
    uint64_t IntVal = FirstTok.getUIntVal();
    Res = MCConstantExpr::create(IntVal, Ctx);
    EndLoc = FirstTok.getEndLoc();
    lex(); // Eat token.
    return false;
  }
  case ALToken::At: {
    MCSymbol *Sym = Ctx.createTempSymbol();
    Out.emitLabel(Sym);
    Res = MCSymbolRefExpr::create(Sym, getContext());
    EndLoc = Lexer.getTok().getEndLoc();
    lex(); // Eat @.
    return false;
  }
  case ALToken::Identifier: {
    std::string SymbolName = CurPrefix + FirstTok.getString().str();
    MCSymbol *Sym = Ctx.getInlineAsmLabel(SymbolName);
    if (!Sym)
      Sym = Ctx.parseSymbol(SymbolName);

    // Otherwise create a symbol ref.
    Res = MCSymbolRefExpr::create(Sym, getContext(), FirstTokenLoc);
    return false;
  }
  case ALToken::Dollar: {
    lex(); // Eat the operator.
    if (parsePrimaryExpr(Res, EndLoc, TypeInfo))
      return true;
    int64_t RegNo;
    if (!Res->evaluateAsAbsolute(RegNo))
      return true;
    Res = MMIXMCExpr::createRegExpr(RegNo, Ctx);
    return false;
  }

  case ALToken::Plus:
    lex(); // Eat the operator.
    if (parsePrimaryExpr(Res, EndLoc, TypeInfo))
      return true;
    Res = MCUnaryExpr::createPlus(Res, getContext(), FirstTokenLoc);
    return false;
  case ALToken::Minus:
    lex(); // Eat the operator.
    if (parsePrimaryExpr(Res, EndLoc, TypeInfo))
      return true;
    Res = MCUnaryExpr::createMinus(Res, getContext(), FirstTokenLoc);
    return false;
  }
}

int ALParser::parseStatement(ParseStatementInfo &Info) {
  assert(!hasPendingError() && "parseStatement started with pending error");
  while (getTok().is(ALToken::Space))
    lex();
  if (getTok().is(ALToken::EndOfStatement)) {
    // if this is a line comment we can drop it safely
    if (getTok().getString().empty() || getTok().getString().front() == '\r' ||
        getTok().getString().front() == '\n')
      Out.addBlankLine();
    lex();
    return 0;
  }

  // Statements always start with an identifier.
  std::optional<ALToken> Label;
  // if current state is mnemonic, this indicate previous state
  // is label
  if (Lexer.getState() == LexerState::Mnemonic) {
    ALToken LabelTok = getTok();
    if (LabelTok.is(ALToken::Error))
      return 1;
    Label = getTok();
  }

  MCSymbol *Sym = nullptr;
  if (Label) {
    if (Label->is(ALToken::Identifier)) {
      StringRef LabelName = Label->getString();
      Sym = getContext().parseSymbol(
          LabelName.starts_with(':') ? LabelName : CurPrefix + LabelName);
    } else {
      assert(Label->is(ALToken::DirectionalLabel) &&
             "expect directional label!");
      Sym = getContext().createDirectionalLocalSymbol(Label->getUIntVal());
    }

    SMLoc LabelLoc = Label->getLoc();
    getTargetParser().doBeforeLabelEmit(Sym, LabelLoc);
    Out.emitLabel(Sym, LabelLoc);
    getTargetParser().onLabelParsed(Sym);

    // eat the label
    lex();
  }

  // Consume any end of statement token, if present, to avoid spurious
  // addBlankLine calls().
  if (getTok().is(ALToken::EndOfStatement)) {
    lex();
    return 0;
  }

  return parseAndMatchAndEmitTargetInstruction(Info);
}

int ALParser::parseAndMatchAndEmitTargetInstruction(ParseStatementInfo &Info) {
  ALToken MnemonicTok = getTok();
  Lexer.lex(); // eat mnemonic
  // Canonicalize the opcode to lower case.
  std::string OpcodeStr = MnemonicTok.getString().lower();
  ParseInstructionInfo IInfo(Info.AsmRewrites);
  bool ParseHadError = getTargetParser().parseInstruction(
      IInfo, OpcodeStr, MnemonicTok.getLoc(), Info.ParsedOperands);
  Info.ParseError = ParseHadError;

  // Dump the parsed representation, if requested.
  if (getShowParsedOperands()) {
    SmallString<256> Str;
    raw_svector_ostream OS(Str);
    OS << "parsed instruction: [";
    for (unsigned i = 0; i != Info.ParsedOperands.size(); ++i) {
      if (i != 0)
        OS << ", ";
      Info.ParsedOperands[i]->print(OS, MAI);
    }
    OS << "]";

    printMessage(MnemonicTok.getLoc(), SourceMgr::DK_Note, OS.str());
  }

  // If parsing succeeded, match the instruction.
  if (!ParseHadError) {
    uint64_t ErrorInfo;
    if (getTargetParser().matchAndEmitInstruction(
            MnemonicTok.getLoc(), Info.Opcode, Info.ParsedOperands, Out,
            ErrorInfo, getTargetParser().isParsingMSInlineAsm()))
      return true;
  }
  return 0;
}

bool ALParser::Run(bool NoInitialTextSection, bool NoFinalize) {
  // init section
  Out.initSections(getTargetParser().getSTI());
  // Prime the lexer.
  std::ignore = lex();
  getTargetParser().onBeginOfFile();

  SmallVector<AsmRewrite, 4> AsmStrRewrites;

  while (getTok().isNot(ALToken::Eof)) {
    ParseStatementInfo Info(&AsmStrRewrites);
    int NewErrCount = parseStatement(Info);
    ErrorCount += NewErrCount;

    // If we have a Lexer Error we are on an Error Token. Load in Lexer Error
    // for printing ErrMsg via lex() only if no (presumably better) parser error
    // exists.
    if (NewErrCount && !hasPendingError() && Lexer.getTok().is(ALToken::Error))
      lex();

    // parseStatement returned true so may need to emit an error.
    printPendingErrors();

    // Skipping to the next line if needed.
    if (NewErrCount && !getLexer().justConsumedEOL())
      eatToEndOfStatement();
  }

  if (auto Rem = DataCounter % 4) {
    Out.emitZeros(4 - Rem);
  }

  // if (auto MainSymbol =
  //         dyn_cast<MCSymbolMMO>(getContext().getOrCreateSymbol(":Main"))) {
  //   *SharedInfo.GregList.rbegin() = MainSymbol->getEquivalent();
  // }

  getTargetParser().onEndOfFile();

  printPendingErrors();

  // emitPostamble();

  Out.finish(getLexer().getLoc());
  if (ErrorCount == 1) {
    errs() << "(One error were found.)\n";
  } else if (ErrorCount > 1) {
    errs() << '(' << ErrorCount << " error were found.)\n";
  }
  return ErrorCount;
}

/// Throw away the rest of the line for testing purposes.
void ALParser::eatToEndOfStatement() {
  while (Lexer.isNot(ALToken::EndOfStatement) && Lexer.isNot(ALToken::Eof))
    Lexer.lex();

  // Eat EOL and skip the comments that follow.
  if (Lexer.is(ALToken::EndOfStatement))
    Lexer.lex();
}

/// MMIX AL Target parser part

#define GET_REGISTER_MATCHER
// #define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
// #define GET_MNEMONIC_CHECKER
#include "MMIXGenALMatcher.inc"

void MMIXALParser::Initialize(MCAsmParser &Parser) {
  MCAsmParserExtension::Initialize(Parser);

  static const StringMap<std::int64_t> PreDefs = {
      {":ROUND_CURRENT", 0},
      {":ROUND_OFF", 1},
      {":ROUND_UP", 2},
      {":ROUND_DOWN", 3},
      {":ROUND_NEAR", 4},

      {":Inf", 0x7FF0'0000'0000'0000},
      {":Data_Segment", 0x2000'0000'0000'0000},
      {":Pool_Segment", 0x4000'0000'0000'0000},
      {":Stack_Segment", 0x6000'0000'0000'0000},

      {":D_BIT", 0x80},
      {":V_BIT", 0x40},
      {":W_BIT", 0x20},
      {":I_BIT", 0x10},
      {":O_BIT", 0x08},
      {":U_BIT", 0x04},
      {":Z_BIT", 0x02},
      {":X_BIT", 0x01},

      {":D_Handler", 0x10},
      {":V_Handler", 0x20},
      {":W_Handler", 0x30},
      {":I_Handler", 0x40},
      {":O_Handler", 0x50},
      {":U_Handler", 0x60},
      {":Z_Handler", 0x70},
      {":X_Handler", 0x80},

      {":StdIn", 0},
      {":StdOut", 1},
      {":StdErr", 2},
      {":TextRead", 0},
      {":TextWrite", 1},
      {":BinaryRead", 2},
      {":BinaryWrite", 3},
      {":BinaryReadWrite", 4},

      {":Halt", 0},
      {":Fopen", 1},
      {":Fclose", 2},
      {":Fread", 3},
      {":Fgets", 4},
      {":Fgetws", 5},
      {":Fwrite", 6},
      {":Fputs", 7},
      {":Fputws", 8},
      {":Fseek", 9},
      {":Ftell", 10},
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
  for (unsigned I = 0x100, E = SPRClass.getNumRegs() + 0x100; I != E; ++I) {
    const MCRegisterInfo &MRI = *getContext().getRegisterInfo();
    MCRegister Reg = *MRI.getLLVMRegNum(I, /*IsEH=*/false);
    MCSymbol *Symbol =
        Context.getOrCreateSymbol(MMIXInstPrinter::getRegisterName(Reg));
    Symbol->setVariableValue(MCConstantExpr::create(I - 0x100, Context));
    Symbol->setRedefinable(true);
  }
}

MMIXALParser::MMIXALParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                           const MCInstrInfo &MII)
    : MCTargetAsmParser(STI, MII) {}

bool MMIXALParser::parseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                 SMLoc &EndLoc) {
  ParseStatus MatchResult = tryParseRegister(RegNo, StartLoc, EndLoc);
  return !MatchResult.isSuccess();
}

ParseStatus MMIXALParser::tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                           SMLoc &EndLoc) {

  if (getTok().isNot(MMIX::ALToken::Dollar))
    return ParseStatus::NoMatch;
  StartLoc = getTok().getLoc();
  Lex(); // eat $
  SmallString<4> RegString("$");
  RegString.append(getTok().getString());
  RegNo = MatchRegisterName(RegString);
  if (RegNo == MMIX::NoRegister)
    return ParseStatus::NoMatch;
  Lex(); // eat register token
  return ParseStatus::Success;
}

bool MMIXALParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                           OperandVector &Operands,
                                           MCStreamer &Out, uint64_t &ErrorInfo,
                                           bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (MatchResult) {
  default:
    return Error(IDLoc, "MMIXALParser::matchAndEmitInstruction match failed!");
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

ParseStatus MMIXALParser::parseRelAddr(OperandVector &Operands) {
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  auto EndLoc = getTok().getLoc();

  if (HasError)
    return ParseStatus::Failure;
  Operands.push_back(MMIXOperand::createRelAddr(Expr, StartLoc, EndLoc));
  return ParseStatus::Success;
}

bool MMIXALParser::parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                    SMLoc NameLoc, OperandVector &Operands) {
  return false;
}

/// Create an ALParser instance.
ALParser *llvm::MMIX::createMCMMIXALParser(SourceMgr &SrcMgr, MCContext &Ctx,
                                           MCStreamer &Out,
                                           const MCAsmInfo &MAI) {
  return new ALParser(Ctx, Out, SrcMgr, MAI);
}
