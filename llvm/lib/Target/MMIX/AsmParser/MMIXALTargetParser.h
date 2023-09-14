#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALTARGETPARSER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALTARGETPARSER_H

#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"

#include <forward_list>

namespace llvm {
// dirty workaround
namespace MMIXAL {

class MMIXALAsmParser : public MCTargetAsmParser {

private:
  bool StrictMode;
  bool SpecialMode;
  MMO::AsmSharedInfo &SharedInfo;

#define GET_ASSEMBLER_HEADER
#include "MMIXALGenAsmMatcher.inc"
public:
  enum MMIXMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "MMIXALGenAsmMatcher.inc"
  };
public:
  MMIXALAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options,
                MMO::AsmSharedInfo &SharedInfo);

public:
  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;

  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  ParseStatus tryParseRegister(MCRegister &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseDirective(AsmToken DirectiveID) override;

  const MCExpr *createTargetUnaryExpr(const MCExpr *E,
                                      AsmToken::TokenKind OperatorToken,
                                      MCContext &Ctx) override;

private:
  bool parseOperand(OperandVector &Operands, StringRef Mnemonic);
  ParseStatus tryParseJumpDestOperand(OperandVector &Operands);
  ParseStatus tryParseBaseAddressOperand(OperandVector &Operands);
  ParseStatus parseSFR(OperandVector &Operands);
  void resolveBaseAddress(MCInst &Inst, const OperandVector &Operands);
  void emitSET(std::uint64_t Val);
};
} // namespace MMIXAL

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALTARGETPARSER_H
