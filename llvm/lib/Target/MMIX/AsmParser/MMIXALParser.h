//===-- MMIXALParser.h - Parse MMIX assembly language ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALPARSER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALPARSER_H

#include "MMIXALLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"

namespace llvm {

class LLVM_ABI MMIXALParser : public MCAsmParser {
public:
  MMIXALParser(MCContext &Ctx, MCStreamer &Out, SourceMgr &SrcMgr,
               const MCAsmInfo &MAI)
      : MCAsmParser(Ctx, Out, SrcMgr, MAI) {}

public:
  void addDirectiveHandler(StringRef Directive,
                           ExtensionDirectiveHandler Handler) override {}
  void addAliasForDirective(StringRef Directive, StringRef Alias) override {}

  bool Run(bool NoInitialTextSection, bool NoFinalize = false) override;

  const AsmToken &Lex() override;

  void setParsingMSInlineAsm(bool V) override {}

  bool isParsingMSInlineAsm() override { return false; }

  bool parseMSInlineAsm(std::string &AsmString, unsigned &NumOutputs,
                        unsigned &NumInputs,
                        SmallVectorImpl<std::pair<void *, bool>> &OpDecls,
                        SmallVectorImpl<std::string> &Constraints,
                        SmallVectorImpl<std::string> &Clobbers,
                        const MCInstrInfo *MII, MCInstPrinter *IP,
                        MCAsmParserSemaCallback &SI) override {
    return true;
  }

  void Note(SMLoc L, const Twine &Msg, SMRange Range = {}) override;
  bool Warning(SMLoc L, const Twine &Msg, SMRange Range = {}) override;
  bool printError(SMLoc L, const Twine &Msg, SMRange Range = {}) override;

  bool parseIdentifier(StringRef &Res) override;
  StringRef parseStringToEndOfStatement() override;
  bool parseEscapedString(std::string &Data) override;
  bool parseAngleBracketString(std::string &Data) override;
  void eatToEndOfStatement() override;
  bool parseExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                        AsmTypeInfo *TypeInfo = nullptr) override;
  bool parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parseAbsoluteExpression(int64_t &Res) override;
  bool checkForValidSection() override;

private:
  MMIXALLexer Lexer;
};

LLVM_ABI MMIXALParser *createMCMMIXALParser(MCContext &Ctx, MCStreamer &Out,
                                            SourceMgr &SrcMgr,
                                            const MCAsmInfo &MAI);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALPARSER_H
