#ifndef LLVM_MMIXAL_PARSER_H
#define LLVM_MMIXAL_PARSER_H

#include "MMIXALLexer.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCTargetOptions.h"
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

  std::size_t ErrorCount = 0;
  bool SpecialMode = false;
  bool LastIsESPEC = false;
  std::string CurPrefix = ":";
  std::uint16_t SerialCnt = 1;
  StringRef CurrentFileName;
  // indicate how many bytes have been wrote
  // we are only interested in the \mathbb{Z}/4\mathbb{Z}
  std::size_t DataCounter = 0;

  std::vector<StringRef> FileNames;
  std::vector<std::uint8_t> LocalRegList;

  enum IdentifierKind {
    Invalid,
    Symbol,
    LocalLabelHere,
    LocalLabelBackward,
    LocalLabelForward,
  };

private:
  static IdentifierKind getIdentifierKind(StringRef Name);
  // utilities to imitate DEK's MMIXAL
  void alignPC(unsigned Alignment = 4);
  ///
  // sync file and line number
  void syncMMO();
  /// sync file loc and PC
  void syncLOC();

  bool parseBinOpRHS(unsigned Precedence, const MCExpr *&Res, SMLoc &EndLoc);
  unsigned getBinOpPrecedence(AsmToken::TokenKind K,
                              MCBinaryExpr::Opcode &Kind);
  bool parseStatement();
  bool handleEndOfStatement();
  void initInternalSymbols();
  std::string getQualifiedName(StringRef Name);
  void resolveLabel(MCSymbol *Symbol);
  template <typename T> void emitData(T Data) {
    char Buf[sizeof(T)];
    support::endian::write<T, support::big>(Buf, Data);
    for (const auto &C : Buf) {
      if (DataCounter % 4 == 0 && !SpecialMode) {
        syncLOC();
        syncMMO();
        if (static_cast<std::uint8_t>(Buf[0]) == MMO::MM) {
          Out.emitBytes(StringRef("\x98\x00\x00\x01", 4));
        }
      }
      Out.emitBytes(StringRef(&C, 1));
      ++DataCounter;
      if (DataCounter % 4 == 0) {
        ++SharedInfo.MMOLine;
      }
      if (!SpecialMode) {
        ++SharedInfo.PC;
        ++SharedInfo.MMOLoc;
      }
    }
  }

  bool parsePseudoOperationIS(StringRef Label);
  bool parsePseudoOperationLOC(StringRef Label);
  bool parsePseudoOperationPREFIX();
  bool parsePseudoOperationGREG(StringRef Label);
  bool parsePseudoOperationLOCAL();
  bool parsePseudoOperationBSPEC();
  bool parsePseudoOperationOCTA();
  template <typename T> bool parsePseudoOperationBWTO() {
    // firstly align the output
    // if it is the first data related instruction
    if (!SpecialMode) {
      alignPC(sizeof(T));
      auto Rem = SharedInfo.PC % 4;
      if (Rem != 0 && DataCounter == 0) {
        syncLOC();
        syncMMO();
        Out.emitZeros(Rem);
        DataCounter += Rem;
      }
    } else if (!SpecialMode) {
      auto OldLoc = SharedInfo.PC;
      alignPC(sizeof(T));
      auto Diff = SharedInfo.PC - OldLoc;
      for (std::uint64_t I = 0; I != Diff; ++I) {
        emitData<std::uint8_t>(0);
      }
    }

    auto CurTok = getTok();
    while (CurTok.isNot(AsmToken::EndOfStatement)) {
      if (CurTok.is(AsmToken::String)) {
        auto Str = CurTok.getStringContents();
        for (const auto &B : Str.bytes()) {
          emitData<T>(B);
        }
        CurTok = Lex();
      } else {
        const MCExpr *Res;
        SMLoc EndLoc;
        if (parseExpression(Res, EndLoc)) {
          break;
        }
        std::int64_t Val;
        if (Res->evaluateAsAbsolute(Val)) {
          emitData(static_cast<T>(Val));
        } else {
          if (isa<MCSymbolRefExpr>(Res) && sizeof(T) == 8) {
            auto SRE = dyn_cast<MCSymbolRefExpr>(Res);
            if (getTargetParser().getTargetOptions().MCRelaxAll) {
              // emit a fake instruction to record fixup
              MCInst I;
              I.setOpcode(0);
              I.addOperand(MCOperand::createExpr(SRE));
              if (DataCounter % 4 == 0 && !SpecialMode) {
                syncLOC();
                syncMMO();
              }
              Out.emitInstruction(I, *Ctx.getSubtargetInfo());
              emitData(static_cast<T>(0x98'00'00'00'98'00'00'00));
            } else {
              SharedInfo.FixupList.push_front(
                  {SharedInfo.PC, MMO::FixupInfo::FixupKind::FIXUP_OCTA,
                   &SRE->getSymbol()});
              emitData(static_cast<T>(0));
            }
            continue;
          }
          break;
        }
      }

      CurTok = getTok();
      if (CurTok.is(AsmToken::Comma)) {
        CurTok = Lex();
      } else {
        break;
      }
    }
    return handleEndOfStatement(); // eat end of statement
  }

  void emitPostamble();
  std::uint8_t getCurGreg();
  void checkLocalRegs();

public:
  void addDirectiveHandler(StringRef Directive,
                           ExtensionDirectiveHandler Handler) override {}
  void addAliasForDirective(StringRef Directive, StringRef Alias) override {}
  void setParsingMSInlineAsm(bool V) override {}
  bool isParsingMSInlineAsm() override { return false; }

  bool Run(bool NoInitialTextSection, bool NoFinalize = false) override;
  std::size_t getErrorCount() const;

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
