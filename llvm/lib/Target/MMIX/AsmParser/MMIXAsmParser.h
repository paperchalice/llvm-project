//===-- MMIXAsmParser.h - Parse MMIX assembly to MCInst instructions ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXASMPARSER_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXASMPARSER_H

#include "MCTargetDesc/MMIXTargetStreamer.h"

#include "llvm/MC/MCParser/MCTargetAsmParser.h"

namespace llvm {

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
class LLVM_LIBRARY_VISIBILITY MMIXAsmParser : public MCTargetAsmParser {
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
  ParseStatus parseRelAddr(OperandVector &Operands);
  // Directive parsers
private:
  ParseStatus parseGREG(SMLoc Loc);
  ParseStatus parsePREFIX(SMLoc Loc);
  ParseStatus parseBSPEC(SMLoc Loc);
  ParseStatus parseESPEC(SMLoc Loc);
  ParseStatus parseIS(SMLoc Loc);
  ParseStatus parseLOCAL(SMLoc Loc);

  void assignIndex(MCSymbol &Symbol);

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

}

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXASMPARSER_H
