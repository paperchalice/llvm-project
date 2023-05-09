#ifndef LLVM_MMIXAL_PARSER_H
#define LLVM_MMIXAL_PARSER_H

#include "MMIXALLexer.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/SourceMgr.h"
#include <forward_list>

namespace llvm {

struct MMIXMCAsmInfoMMIXAL;

class MMIXALParser : public MCAsmParser {
private:
  MMIXALLexer Lexer;
  MCContext &Ctx;
  MCStreamer &Out;
  const MCAsmInfo &MAI;
  SourceMgr &SrcMgr;
  SourceMgr::DiagHandlerTy SavedDiagHandler;
  void *SavedDiagContext;
  std::unique_ptr<MCAsmParserExtension> PlatformParser;

  /// This is the current buffer index we're lexing from as managed by
  /// the SourceMgr object.
  unsigned CurBuffer;
  bool StrictMode;
  raw_ostream &Lst;

  bool SpecialMode = false;
  std::size_t SpecialDataLoc = 0;
  std::string CurPrefix = ":";
  std::uint16_t SerialCnt = 1;
  StringRef CurrentFileName = "";
  SmallVector<StringRef, 257> FileNames;

  enum IdentifierKind {
    Invalid,
    Symbol,
    LocalLabelHere,
    LocalLabelBackward,
    LocalLabelForward,
  };

private:
  static IdentifierKind getIdentifierKind(StringRef Name);
  void alignPC(unsigned Alignment = 4);
  void syncMMO();
  bool parseBinOpRHS(unsigned Precedence, const MCExpr *&Res, SMLoc &EndLoc);
  unsigned getBinOpPrecedence(AsmToken::TokenKind K,
                              MCBinaryExpr::Opcode &Kind);
  bool parseStatement();
  void initInternalSymbols();
  void BypassLine();
  std::string getQualifiedName(StringRef Name);
  void resolveLabel(MCSymbol *Symbol);

  bool parsePseudoOperationIS(StringRef Label);
  bool parsePseudoOperationLOC(StringRef Label);
  bool parsePseudoOperationPREFIX();
  bool parsePseudoOperationGREG(StringRef Label);
  bool parsePseudoOperationLOCAL();
  bool parsePseudoOperationBSPEC();
  bool parsePseudoOperationOCTA();
  void emitPostamble();
  std::uint8_t getCurGreg();
  template <typename T> bool parseData() {
    const MCExpr *Res;
    SMLoc EndLoc;
    if (!SpecialMode) {
      alignPC(sizeof(T));
    }

    while (getTok().isNot(AsmToken::EndOfStatement)) {
      bool HasErr = parseExpression(Res, EndLoc);
      if (HasErr)
        return true;
      int64_t Val;
      if (!Res->evaluateAsAbsolute(Val)) {
        return true;
      }

      // before write sync mmo to produce line number info
      if(!SpecialMode && SharedInfo.PC %4==0) {
        syncMMO();
      }
      char Buf[sizeof(T)];
      support::endian::write<T>(Buf, Val, support::endianness::big);
      Out.emitBinaryData({Buf, sizeof(T)});
      if (!SpecialMode) {
        SharedInfo.PC += sizeof(T);
      } else {
        SpecialDataLoc += sizeof(T);
      }

      if (getTok().is(AsmToken::Comma))
        Lex();
    }
    return false;
  }

public:
  void addDirectiveHandler(StringRef Directive,
                           ExtensionDirectiveHandler Handler) override {}
  void addAliasForDirective(StringRef Directive, StringRef Alias) override {}
  void setParsingMSInlineAsm(bool V) override {}
  bool isParsingMSInlineAsm() override { return false; }

  bool Run(bool NoInitialTextSection, bool NoFinalize = false) override;

  // Notification
  void Note(SMLoc L, const Twine &Msg, SMRange Range = std::nullopt) override;
  void printMessage(SMLoc Loc, SourceMgr::DiagKind Kind, const Twine &Msg,
                    SMRange Range = std::nullopt) const;
  bool Warning(SMLoc L, const Twine &Msg,
               SMRange Range = std::nullopt) override;
  bool printError(SMLoc L, const Twine &Msg,
                  SMRange Range = std::nullopt) override;

  /// @name MCAsmParser Interface
  /// {

  SourceMgr &getSourceManager() override { return SrcMgr; }
  MCAsmLexer &getLexer() override { return Lexer; }
  MCContext &getContext() override { return Ctx; }
  MCStreamer &getStreamer() override { return Out; }
  const AsmToken &Lex() override;
  bool parseIdentifier(StringRef &Res) override;
  StringRef parseStringToEndOfStatement() override { return ""; }
  bool parseEscapedString(std::string &Data) override { return false; }
  bool parseAngleBracketString(std::string &Data) override { return false; }
  void eatToEndOfStatement() override {}
  bool parseExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parsePrimaryExpr(const MCExpr *&Res, SMLoc &EndLoc,
                        AsmTypeInfo *TypeInfo) override;
  bool parseParenExpression(const MCExpr *&Res, SMLoc &EndLoc) override;
  bool parseAbsoluteExpression(int64_t &Res) override { return false; }
  bool checkForValidSection() override { return false; }
  bool parseParenExprOfDepth(unsigned ParenDepth, const MCExpr *&Res,
                             SMLoc &EndLoc) override {
    return false;
  }
  /// }

  /// Parse MS-style inline assembly.
  bool parseMSInlineAsm(std::string &AsmString, unsigned &NumOutputs,
                        unsigned &NumInputs,
                        SmallVectorImpl<std::pair<void *, bool>> &OpDecls,
                        SmallVectorImpl<std::string> &Constraints,
                        SmallVectorImpl<std::string> &Clobbers,
                        const MCInstrInfo *MII, const MCInstPrinter *IP,
                        MCAsmParserSemaCallback &SI) override {
    return true;
  }

public:
  MMO::AsmSharedInfo SharedInfo;
  MMIXALParser(SourceMgr &SM, MCContext &Ctx, MCStreamer &Out,
               const MMIXMCAsmInfoMMIXAL &MAI, raw_ostream &Lst);
};

/// Create an MCMMIXALParser instance for parsing MMIXAL assembly
MMIXALParser *createMCMMIXALParser(SourceMgr &, MCContext &, MCStreamer &,
                                   const MMIXMCAsmInfoMMIXAL &MAI,
                                   raw_ostream &Lst);
} // namespace llvm
#endif // LLVM_MMIXAL_PARSER_H
