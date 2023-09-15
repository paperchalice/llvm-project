//===-- MMIXALOperand.h - Parse MMIX assembly operands --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALOPERAND_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXALOPERAND_H

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/SMLoc.h"

#include <variant>
namespace llvm {

class MMIXALOperand final : public MCParsedAsmOperand {
private:
  SMLoc StartLoc, EndLoc;

  struct Memory {
    std::variant<std::uint64_t, const MCSymbolRefExpr *> DestinationAddress;
    std::uint64_t CurrentAddress;
  };
  std::variant<StringRef, std::int64_t, MCRegister, Memory> Content;

public:
  StringRef getToken() const;
  std::int64_t getImm() const;
  std::uint64_t getConcreteMem() const;
  void addRegOperands(MCInst &Inst, unsigned N) const;
  void addImmOperands(MCInst &Inst, unsigned N) const;
  void addMemOperands(MCInst &Inst, unsigned N) const;

public:
  bool isToken() const override;
  bool isImm() const override;
  bool isReg() const override;
  bool isMem() const override;
  bool isJumpDest() const;
  template <std::uint8_t W> bool isUImm() const {
    return std::holds_alternative<std::int64_t>(Content) && isUInt<W>(getImm());
  }
  bool isRoundMode() const;
  unsigned getReg() const override;
  SMLoc getStartLoc() const override;
  SMLoc getEndLoc() const override;
  void print(raw_ostream &OS) const override;
  void dump() const override;

public:
  MMIXALOperand(StringRef Tok, SMLoc NameLoc, SMLoc EndLoc);
  MMIXALOperand(std::int64_t Imm, SMLoc StartLoc, SMLoc EndLoc);
  MMIXALOperand(MCRegister Reg, SMLoc StartLoc, SMLoc EndLoc);
  MMIXALOperand(std::uint64_t Dest, std::uint64_t PC, SMLoc StartLoc,
                SMLoc EndLoc);
  MMIXALOperand(const MCSymbolRefExpr *SymbolRef, std::uint64_t PC,
                SMLoc StartLoc, SMLoc EndLoc);

public:
  static std::unique_ptr<MMIXALOperand> createMnemonic(StringRef Mnemonic,
                                                       SMLoc StartLoc);
  static std::unique_ptr<MMIXALOperand> createImm(std::int64_t Imm,
                                                  SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXALOperand> createReg(unsigned RegNo,
                                                  SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXALOperand>
  createMem(std::uint64_t Dest, std::uint64_t PC, SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXALOperand>
  createMem(const MCSymbolRefExpr *SymbolRef, std::uint64_t PC, SMLoc StartLoc,
            SMLoc EndLoc);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASMPARSER_MMIXOPERAND_H
