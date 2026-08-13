//===-- MMIXOperand.h - Parse MMIX assembly operands ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXOPERAND_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXOPERAND_H

#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/Support/MathExtras.h"

#include <memory>
#include <variant>

namespace llvm {

class MMIXOperand final : public MCParsedAsmOperand {
  SMLoc StartLoc, EndLoc;
  std::variant</*Imm*/ std::int64_t, StringRef, MCRegister, const MCExpr *>
      Content;

public:
  StringRef getToken() const;
  std::int64_t getImm() const;
  const MCExpr *getBranchDest() const;
  void addRegOperands(MCInst &Inst, unsigned N) const;
  void addImmOperands(MCInst &Inst, unsigned N) const;
  void addBranchDestOperands(MCInst &Inst, unsigned N) const;
  bool isRoundingMode() const;
  bool isBranchDest() const;

  template <int N, bool isU> bool isImm() const {
    if (!std::holds_alternative<std::int64_t>(Content))
      return false;
    auto Imm = std::get<std::int64_t>(Content);
    if constexpr (isU)
      if (Imm < 0)
        return false;
    switch (N) {
    case -1:
      return true;
    case 0:
      return Imm == 0;
    default:
      if constexpr (isU)
        return isUInt<static_cast<unsigned>(N)>(static_cast<uint64_t>(Imm));
      else
        return isInt<static_cast<unsigned>(N)>(Imm);
    }
  }

  template <int N = -1> bool isUImm() const { return isImm<N, true>(); }
  template <int N = -1> bool isSImm() const { return isImm<N, false>(); }

public:
  bool isToken() const override;
  bool isImm() const override;
  bool isReg() const override;
  bool isMem() const override;

  MCRegister getReg() const override;
  SMLoc getStartLoc() const override { return StartLoc; }
  SMLoc getEndLoc() const override { return EndLoc; }
  void print(raw_ostream &OS, const MCAsmInfo &) const override;

public:
  MMIXOperand(SMLoc StartLoc, SMLoc EndLoc)
      : StartLoc(StartLoc), EndLoc(EndLoc) {};
  MMIXOperand(StringRef Tok, SMLoc NameLoc, SMLoc EndLoc);
  MMIXOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  MMIXOperand(const std::int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc);
  MMIXOperand(const MCRegister &Reg, SMLoc StartLoc, SMLoc EndLoc);

public:
  static std::unique_ptr<MMIXOperand> createMnemonic(StringRef Mnemonic,
                                                     SMLoc StartLoc);
  static std::unique_ptr<MMIXOperand> createReg(MCRegister Reg, SMLoc StartLoc,
                                                SMLoc EndLoc);
  static std::unique_ptr<MMIXOperand> createImm(std::int64_t Reg,
                                                SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXOperand>
  createBranchDest(const MCExpr *E, SMLoc StartLoc, SMLoc EndLoc);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXOPERAND_H
