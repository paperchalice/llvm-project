//===-- MMIXAsmParser.h - Parse MMIX assembly to MCInst instructions --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXASMPARSER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXASMPARSER_H

#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"

namespace llvm {

class MMIXAsmParser : public MCTargetAsmParser {
#define GET_ASSEMBLER_HEADER
#include "MMIXGenAsmMatcher.inc"
public:
  enum MMIXMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "MMIXGenAsmMatcher.inc"
  };
public:
  MMIXAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options);

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
  ParseStatus parseMemOperand(OperandVector &Operands) {
    return ParseStatus::Failure;
  };
  ParseStatus tryParseJumpDestOperand(OperandVector &Operands);
  ParseStatus tryParseBranchDestOperand(OperandVector &Operands);
  ParseStatus parseSFR(OperandVector &Operands) { return ParseStatus::Failure; }
  void resolveBaseAddress(MCInst &Inst, const OperandVector &Operands);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASMPARSER_MMIXASMPARSER_H