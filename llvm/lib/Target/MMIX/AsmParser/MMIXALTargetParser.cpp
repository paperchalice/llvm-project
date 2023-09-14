#include "MMIXALTargetParser.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "MCTargetDesc/MMIXMCInstrInfo.h"
#include "MMIXALOperand.h"
#include "MMIXInstrInfo.h"
#include "MMIXRegisterInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/MathExtras.h"

#define DEBUG_TYPE "mmixal-parser"

using namespace llvm;
using namespace MMIXAL;

// #define GET_REGISTER_MATCHER
// #define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
#define GET_MNEMONIC_CHECKER
#include "MMIXALGenAsmMatcher.inc"

static unsigned SFRDecodeTable[] = {
    MMIX::rB,  MMIX::rD,  MMIX::rE,  MMIX::rH,  MMIX::rJ, MMIX::rM, MMIX::rR,
    MMIX::rBB, MMIX::rC,  MMIX::rN,  MMIX::rO,  MMIX::rS, MMIX::rI, MMIX::rT,
    MMIX::rTT, MMIX::rK,  MMIX::rQ,  MMIX::rU,  MMIX::rV, MMIX::rG, MMIX::rL,
    MMIX::rA,  MMIX::rF,  MMIX::rP,  MMIX::rW,  MMIX::rX, MMIX::rY, MMIX::rZ,
    MMIX::rWW, MMIX::rXX, MMIX::rYY, MMIX::rZZ,
};

MMIXALAsmParser::MMIXALAsmParser(const MCSubtargetInfo &STI,
                                 MCAsmParser &Parser, const MCInstrInfo &MII,
                                 const MCTargetOptions &Options,
                                 MMO::AsmSharedInfo &SharedInfo)
    : MCTargetAsmParser(Options, STI, MII), SharedInfo(SharedInfo) {}

bool MMIXALAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                    SMLoc &EndLoc) {
  return true;
}

bool MMIXALAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                              OperandVector &Operands,
                                              MCStreamer &Out,
                                              uint64_t &ErrorInfo,
                                              bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned MatchResult =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (MatchResult) {
  default:
    Error(IDLoc, "Unable to match Instruction");
    return true;
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  }
  return false;
}

ParseStatus MMIXALAsmParser::tryParseRegister(MCRegister &RegNo,
                                              SMLoc &StartLoc, SMLoc &EndLoc) {
  return ParseStatus::Failure;
}

bool MMIXALAsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                       StringRef Name, SMLoc NameLoc,
                                       OperandVector &Operands) {

  // check mnemonic
  auto FBS = getAvailableFeatures();
  auto VariantID = getParser().getAssemblerDialect();
  if (!MMIXALCheckMnemonic(Name, FBS, VariantID)) {
    const auto S = MMIXALMnemonicSpellCheck(Name, FBS, VariantID);
    const auto Message =
        formatv("{0} `{1}'{2}", "unknown operation code", Name, S);
    Error(NameLoc, Message);
    return true;
  }

  // eat the mnemonic
  Operands.push_back(MMIXALOperand::createMnemonic(Name, NameLoc));
  auto CurTok = getTok();

  if (getTok().isNot(AsmToken::EndOfStatement)) {
    if (parseOperand(Operands, Name)) {
      // if parser doesn't consume any token, it is start of a comment
      if (CurTok.getLoc() == getTok().getLoc()) {
        return false;
      }
      return true;
    }
    while (getTok().isNot(AsmToken::EndOfStatement) &&
           getTok().isNot(AsmToken::Space)) {
      auto StartLoc = getTok().getLoc();
      if (getTok().is(AsmToken::Comma)) {
        getLexer().setSkipSpace(true);
        Lex();
        getLexer().setSkipSpace(false);
      }

      if (getTok().is(AsmToken::EndOfStatement)) {
        Error(StartLoc, "Syntax error after `,'!");
        return true;
      }
      if (parseOperand(Operands, Name)) {
        return true;
      }
    }
  }

  // don't eat end of statement for line counter
  return false;
}

bool MMIXALAsmParser::ParseDirective(AsmToken DirectiveID) { return false; }

const MCExpr *MMIXALAsmParser::createTargetUnaryExpr(
    const MCExpr *E, AsmToken::TokenKind OperatorToken, MCContext &Ctx) {
  return MMIXMCExpr::create(E, MMIXMCExpr::VK_MMIX_REG_EXPR, Ctx);
}

bool MMIXALAsmParser::parseOperand(OperandVector &Operands,
                                   StringRef Mnemonic) {
  ParseStatus ResTy =
      MatchOperandParserImpl(Operands, Mnemonic, /*ParseForAllFeatures=*/true);
  if (ResTy.isSuccess()) {
    return false;
  }

  if (ResTy.isFailure()) {
    Error(getTok().getLoc(), "Parse failed in TargetParser");
    return true;
  }

  // operand is always expression
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  if (HasError) {
    return true;
  }
  // evaluate all expressionj eagerly
  int64_t Res;
  bool Success = Expr->evaluateAsAbsolute(Res);
  if (!Success) {
    if (MMIXMCExpr::isGPRExpr(Expr)) {
      SMRange R = {StartLoc, getTok().getLoc()};
      Error(StartLoc, "can registerize pure values only!", R);
    }
    return true;
  } else {
    if (MMIXMCExpr::isGPRExpr(Expr)) {
      if (Res >= 0x100) {
        Warning(StartLoc, "register number too large, will be reduced mod 256");
        Res %= 0x100;
      }
      auto RegNo = MMIX::r0 + Res;
      Operands.emplace_back(
          MMIXALOperand::createReg(RegNo, StartLoc, getLexer().getLoc()));
    } else {
      Operands.emplace_back(
          MMIXALOperand::createImm(Res, StartLoc, getLexer().getLoc()));
    }
  }

  return false;
}

ParseStatus MMIXALAsmParser::parseSFR(OperandVector &Operands) {
  // operand is always expression
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  if (HasError) {
    return ParseStatus::Failure;
  }
  // evaluate all expressionj eagerly
  int64_t Res;
  bool Success = Expr->evaluateAsAbsolute(Res);
  if (!Success) {
    if (MMIXMCExpr::isGPRExpr(Expr)) {
      SMRange R = {StartLoc, getTok().getLoc()};
      Error(StartLoc, "can registerize pure values only!", R);
    }
    return ParseStatus::Failure;
  } else {
    if (Res >= 0x20) {
      Warning(StartLoc, "register number too large, will be reduced mod 32");
      Res %= 32;
    }

    Operands.emplace_back(MMIXALOperand::createReg(
        SFRDecodeTable[Res], StartLoc, getLexer().getLoc()));
    return ParseStatus::Success;
  }
}

ParseStatus MMIXALAsmParser::tryParseJumpDestOperand(OperandVector &Operands) {
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  auto EndLoc = getTok().getLoc();
  if (HasError)
    return ParseStatus::Failure;

  // if we get a GPR operand, it is not a base address form
  if (MMIXMCExpr::isGPRExpr(Expr)) {
    int64_t Res;
    bool Success = Expr->evaluateAsAbsolute(Res);
    if (Success) {
      auto RegNo = MMIX::r0 + Res;
      Operands.emplace_back(
          MMIXALOperand::createReg(RegNo, StartLoc, getLexer().getLoc()));
      return parseToken(AsmToken::Comma) ? ParseStatus::Failure
                                         : ParseStatus::NoMatch;
    }
  }

  int64_t Res;
  bool Success = Expr->evaluateAsAbsolute(Res);
  if (Success) {
    Operands.emplace_back(
        MMIXALOperand::createMem(Res, SharedInfo.PC, StartLoc, EndLoc));
    return ParseStatus::Success;
  } else if (auto SE = dyn_cast<MCSymbolRefExpr>(Expr)) {
    MMIXALOperand &Operand = (MMIXALOperand &)Operands[0];
    auto FK = Operand.getToken() == "JMP"
                  ? MMO::FixupInfo::FixupKind::FIXUP_JUMP
                  : MMO::FixupInfo::FixupKind::FIXUP_WYDE;
    SharedInfo.FixupList.emplace_front(
        MMO::FixupInfo{SharedInfo.PC, FK, &SE->getSymbol()});
    Operands.emplace_back(
        MMIXALOperand::createMem(SE, SharedInfo.PC, StartLoc, EndLoc));
    return ParseStatus::Success;
  } else {
    Error(StartLoc, "Invalid jump dest!");
    return ParseStatus::Failure;
  }
}

ParseStatus
MMIXALAsmParser::tryParseBaseAddressOperand(OperandVector &Operands) {
  return ParseStatus::NoMatch;
}

static auto FindClosestGREG(const std::deque<std::uint64_t> &Q,
                            std::uint64_t Val) {
  auto Begin = Q.begin(), End = Q.end() - 1;
  auto Closest = End;
  for (auto I = Begin; I != End; ++I) {
    if (*I <= Val && *I > *Closest) {
      Closest = I;
    }
  }
  return Closest;
}

#define GET_InstEnc_DECL
#include "MMIXGenSearchableTables.inc"

void MMIXALAsmParser::emitSET(std::uint64_t Val) {
  for (int i = InstEnc::SETH; i <= InstEnc::ORL; ++i) {
    std::uint16_t Part = 0;
    switch (i & 0b0111) {
    case 0:
      Part = Val >> 48;
      break;
    case 1:
      Part = Val >> 32;
      break;
    case 2:
      Part = Val >> 16;
      break;
    case 3:
      Part = Val;
      break;
    }
    if (Part || i == InstEnc::SETL) {
      char InstSet[4] = {static_cast<char>(i), '\xFF',
                         static_cast<char>(Part >> 8),
                         static_cast<char>(Part & 0xFF)};
      getStreamer().emitBinaryData(StringRef(InstSet, 4));

      // emit line number
      getStreamer().emitBinaryData(StringRef("\x98\x07", 2));
      char Buf[2];
      support::endian::write16be(
          Buf, static_cast<std::uint16_t>(SharedInfo.CurrentLine));
      getStreamer().emitBinaryData(StringRef(Buf, 2));

      SharedInfo.PC += 4;
      SharedInfo.MMOLoc += 4;
      ++SharedInfo.MMOLine;
      i |= InstEnc::ORH;
    }
  }
}

void MMIXALAsmParser::resolveBaseAddress(MCInst &Inst,
                                         const OperandVector &Operands) {
  assert(Operands.size() == 3 && "must 2 operands!");

  static_cast<MMIXALOperand &>(*Operands[1]).addRegOperands(Inst, 1);
  auto &DestOperand = static_cast<MMIXALOperand &>(*Operands[2]);
  auto DestAddress = DestOperand.getConcreteMem();

  auto End = SharedInfo.GregList.end() - 1;
  auto Closest = FindClosestGREG(SharedInfo.GregList, DestAddress);
  if (Closest != End) {
    std::uint64_t Diff = DestAddress - *Closest;
    unsigned Reg = MMIX::r0 + 0xFF - (End - Closest);
    if (Diff < 0x100) {
      Inst.addOperand(MCOperand::createReg(Reg));
      Inst.addOperand(MCOperand::createImm(Diff));
      return;
    } else if (SharedInfo.Expand) {
      if (Diff < DestAddress) {
        emitSET(Diff);
        Inst.setOpcode(Inst.getOpcode() - 2); // HACK: use tablegen feature
        Inst.addOperand(MCOperand::createReg(Reg));
        Inst.addOperand(MCOperand::createReg(MMIX::r255));
        return;
      }
    }
  }

  // if the diff is overflow or we can't find close enough
  // global register
  if (SharedInfo.Expand) {
    emitSET(DestAddress);
    Inst.addOperand(MCOperand::createReg(MMIX::r255));
    Inst.addOperand(MCOperand::createImm(0));
  }

  if (Inst.getNumOperands() == 1) {
    Error(Operands[2]->getStartLoc(),
          "no base address is close enough to the address A!");
    Inst.addOperand(MCOperand::createImm(0));
    Inst.setFlags(MMIX::DONT_EMIT);
  }
}

namespace llvm::MMIXAL {
/// Force static initialization.
LLVM_EXTERNAL_VISIBILITY MCTargetAsmParser *LLVMInitializeMMIXALTargetParser(
    const MCSubtargetInfo &STI, MCAsmParser &Parser, const MCInstrInfo &MII,
    const MCTargetOptions &Options, MMO::AsmSharedInfo &SharedInfo) {
  return new MMIXALAsmParser(STI, Parser, MII, Options, SharedInfo);
}
} // namespace llvm::MMIXAL