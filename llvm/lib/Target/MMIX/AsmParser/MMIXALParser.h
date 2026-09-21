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
#include "llvm/Support/SourceMgr.h"

namespace llvm::MMIX {

class LLVM_ABI ALParser : public MCAsmParser {
public:
  ALParser(MCContext &Ctx, MCStreamer &Out, SourceMgr &SrcMgr,
           const MCAsmInfo &MAI);

public:
  ALLexer &getLexer() { return Lexer; }

public:
  void addDirectiveHandler(StringRef Directive,
                           ExtensionDirectiveHandler Handler) override {}
  void addAliasForDirective(StringRef Directive, StringRef Alias) override {}

  bool Run(bool NoInitialTextSection, bool NoFinalize = false) override;

  const AsmToken &Lex() override {
    static AsmToken Dummy;
    return Dummy;
  }
  const ALToken &lex();
  const ALToken &getTok() const { return Lexer.getTok(); }

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

  void printMessage(SMLoc Loc, SourceMgr::DiagKind Kind, const Twine &Msg,
                    SMRange Range = {}) const {
    ArrayRef<SMRange> Ranges(Range);
    SrcMgr.PrintMessage(Loc, Kind, Msg, Ranges);
  }
  void Note(SMLoc L, const Twine &Msg, SMRange Range = {}) override;
  bool Warning(SMLoc L, const Twine &Msg, SMRange Range = {}) override;
  bool printError(SMLoc L, const Twine &Msg, SMRange Range = {}) override;

  bool parseIdentifier(StringRef &Res) override;
  StringRef parseStringToEndOfStatement() override { return ""; }
  bool parseEscapedString(std::string &Data) override { return false; }
  bool parseAngleBracketString(std::string &Data) override { return false; }
  void eatToEndOfStatement() override {}
  bool parseExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                        AsmTypeInfo *TypeInfo = nullptr) override;
  bool parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parseAbsoluteExpression(int64_t &Res) override { return false; }
  bool checkForValidSection() override { return false; }

private:
  bool parseStatement();

private:
  std::size_t ErrorCount = 0;
  bool SpecialMode = false;
  bool LastIsESPEC = false;
  std::string CurPrefix = ":";
  std::uint16_t SerialCnt = 1;
  StringRef CurrentFileName;
  // indicate how many bytes have been wrote
  // we are only interested in the \mathbb{Z}/4\mathbb{Z}
  std::size_t DataCounter = 0;

private:
  MMIX::ALLexer Lexer;
};

LLVM_ABI ALParser *createMCMMIXALParser(SourceMgr &SrcMgr, MCContext &Ctx,
                                        MCStreamer &Out, const MCAsmInfo &MAI);

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALPARSER_H
