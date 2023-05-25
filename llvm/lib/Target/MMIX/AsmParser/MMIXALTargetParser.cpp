#include "MMIXALTargetParser.h"
#include "MCTargetDesc/MMIXMCExpr.h"
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
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace MMIXAL;

#define GET_REGISTER_MATCHER
// #define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
// #define GET_MNEMONIC_SPELL_CHECKER
// #define GET_MNEMONIC_CHECKER
#include "MMIXALGenAsmMatcher.inc"

MMIXALAsmParser::MMIXALAsmParser(const MCSubtargetInfo &STI,
                                 MCAsmParser &Parser, const MCInstrInfo &MII,
                                 const MCTargetOptions &Options,
                                 MMO::AsmSharedInfo &SharedInfo)
    : MCTargetAsmParser(Options, STI, MII), SharedInfo(SharedInfo) {}

bool MMIXALAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                    SMLoc &EndLoc) {
  OperandMatchResultTy MatchResult = tryParseRegister(Reg, StartLoc, EndLoc);
  return MatchResult != OperandMatchResultTy::MatchOperand_Success;
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
    return true;
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  }
  return false;
}

OperandMatchResultTy MMIXALAsmParser::tryParseRegister(MCRegister &RegNo,
                                                       SMLoc &StartLoc,
                                                       SMLoc &EndLoc) {
  auto RegTok = getTok();
  Lex();                                         // eat synthesis register token
  RegNo = MatchRegisterName(RegTok.getString()); // impossible to fail
  return MatchOperand_Success;
}

bool MMIXALAsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                       StringRef Name, SMLoc NameLoc,
                                       OperandVector &Operands) {

  // eat the mnemonic
  Operands.push_back(MMIXALOperand::createMnemonic(Name, NameLoc));
  auto CurTok = getTok();

  while (getTok().isNot(AsmToken::EndOfStatement)) {
    bool HasErr = parseOperand(Operands, Name);
    if (HasErr) {
      // regard as comment
      while (getTok().isNot(AsmToken::EndOfStatement)) {
        Lex();
      }
    }

    if (getTok().is(AsmToken::Comma)) {
      // eat comma
      Lex();
    }

    // reach end of instruction
    if (getTok().is(AsmToken::Space)) {
      while (getTok().isNot(AsmToken::EndOfStatement)) {
        Lex();
      }
    }
  }
  Lex(); // eat end of statement
  return false;
}

bool MMIXALAsmParser::ParseDirective(AsmToken DirectiveID) { return false; }

const MCExpr *MMIXALAsmParser::createTargetUnaryExpr(
    const MCExpr *E, AsmToken::TokenKind OperatorToken, MCContext &Ctx) {
  return MMIXMCExpr::create(E, MMIXMCExpr::VK_MMIX_REG_EXPR, Ctx);
}

bool MMIXALAsmParser::parseOperand(OperandVector &Operands,
                                   StringRef Mnemonic) {
  OperandMatchResultTy ResTy =
      MatchOperandParserImpl(Operands, Mnemonic, /*ParseForAllFeatures=*/true);
  if (ResTy == MatchOperand_Success) {
    return false;
  }
  if (ResTy == MatchOperand_ParseFail) {
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
    return true;
  } else {
    if (MMIXMCExpr::isGPRExpr(Expr)) {
      auto RegNo = toMCRegNum(Res);
      Operands.emplace_back(
          MMIXALOperand::createGPR(RegNo, StartLoc, getLexer().getLoc()));
    } else {
      Operands.emplace_back(
          MMIXALOperand::createImm(Res, StartLoc, getLexer().getLoc()));
    }
  }

  return false;
}

OperandMatchResultTy
MMIXALAsmParser::tryParseJumpDestOperand(OperandVector &Operands) {
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  auto EndLoc = getTok().getLoc();
  if (HasError) {
    return MatchOperand_ParseFail;
  }
  auto E = MMIXMCExpr::create(Expr, SharedInfo.PC,
                              MMIXMCExpr::VK_MMIX_PC_REL_JMP, getContext());
  if (MMIX::IsValidExpr(Expr, true)) {
    Operands.emplace_back(MMIXALOperand::createExpression(E, StartLoc, EndLoc));
    if (auto SE = dyn_cast<MCSymbolRefExpr>(Expr)) {
      SharedInfo.FixupList.emplace_front(
          MMO::FixupInfo{SharedInfo.PC, MMO::FixupInfo::FixupKind::FIXUP_JUMP,
                         &SE->getSymbol()});
    }
    return MatchOperand_Success;
  } else {
    return MatchOperand_ParseFail;
  }
}

OperandMatchResultTy
MMIXALAsmParser::tryParseBranchDestOperand(OperandVector &Operands) {
  const MCExpr *Expr = nullptr;
  auto StartLoc = getTok().getLoc();
  bool HasError = getParser().parseExpression(Expr);
  auto EndLoc = getTok().getLoc();
  if (HasError) {
    return MatchOperand_ParseFail;
  }

  if (MMIX::IsValidExpr(Expr, true)) {
    auto E = MMIXMCExpr::create(Expr, SharedInfo.PC,
                                MMIXMCExpr::VK_MMIX_PC_REL_BR, getContext());
    Operands.emplace_back(MMIXALOperand::createExpression(E, StartLoc, EndLoc));
    if (auto SE = dyn_cast<const MCSymbolRefExpr>(Expr)) {
      SharedInfo.FixupList.emplace_front(
          MMO::FixupInfo{SharedInfo.PC, MMO::FixupInfo::FixupKind::FIXUP_WYDE,
                         &SE->getSymbol()});
    }
    return MatchOperand_Success;
  } else {
    return MatchOperand_ParseFail;
  }
}

OperandMatchResultTy llvm::MMIXAL::MMIXALAsmParser::tryParseBaseAddressOperand(
    OperandVector &Operands) {
  return MatchOperand_NoMatch;
}

void llvm::MMIXAL::MMIXALAsmParser::resolveBaseAddress(
    MCInst &Inst, const OperandVector &Operands) {
  Inst.dump();
  assert(Operands.size() == 3 && "must 2 operands!");
  auto &DestReg = static_cast<MMIXALOperand &>(*Operands[1]);
  auto &DestOperand = static_cast<MMIXALOperand &>(*Operands[2]);
  auto DestAddress = static_cast<std::uint64_t>(DestOperand.getImm());

  Inst.addOperand(MCOperand::createReg(DestReg.getReg()));

  auto Predicate = [&](uint64_t Val) {
    if (DestAddress >= Val) {
      return DestAddress - Val < 0x100;
    } else {
      return false;
    }
  };
  auto SearchBegin = SharedInfo.GregList.cbegin() + 1;
  auto SearchEnd = SharedInfo.GregList.cend();
  auto Result = std::find_if(SearchBegin, SearchEnd, Predicate);
  if (Result != SearchEnd) {
    auto BaseAddress = *Result;
    auto RegNum = 254 - (Result - SearchBegin);
    // dirty workaround
    Inst.addOperand(
        MCOperand::createImm((RegNum << 8) | (DestAddress - BaseAddress)));
  } else if (SharedInfo.Expand) {
    auto Closest = SearchBegin;
    for (auto I = SearchBegin; I != SearchEnd; ++I) {
      if (*I <= DestAddress && *I >= *Closest) {
        Closest = I;
      }
    }
    std::uint64_t Diff = DestAddress - *Closest;
    for (int i = 0; i != 4; --i) {
      std::uint16_t Part = Diff >> (48 - i * 16) & 0xFFFF;
      if (Part) {
        char InstSet[4] = {static_cast<char>(0xE0 + i), '\xFF',
                           static_cast<char>(Part >> 8),
                           static_cast<char>(Part & 0xFF)};
        getStreamer().emitBinaryData(StringRef(InstSet, 4));
        SharedInfo.PC += 4;
        SharedInfo.MMOLoc += 4;
        ++SharedInfo.MMOLine;
      }
    }

    Inst.addOperand(
        MCOperand::createReg(toMCRegNum(254 - (Closest - SearchBegin))));
    Inst.addOperand(MCOperand::createReg(toMCRegNum(255)));
  }
}

unsigned llvm::MMIXAL::MMIXALAsmParser::toMCRegNum(std::uint8_t RegNum) {
  auto MCRegNum = getContext()
                      .getRegisterInfo()
                      ->getRegClass(MMIX::GPRRegClassID)
                      .getRegister(RegNum);
  return MCRegNum;
}

namespace llvm::MMIXAL {
/// Force static initialization.
LLVM_EXTERNAL_VISIBILITY MCTargetAsmParser *LLVMInitializeMMIXALTargetParser(
    const MCSubtargetInfo &STI, MCAsmParser &Parser, const MCInstrInfo &MII,
    const MCTargetOptions &Options, MMO::AsmSharedInfo &SharedInfo) {
  return new MMIXALAsmParser(STI, Parser, MII, Options, SharedInfo);
}
} // namespace llvm::MMIXAL