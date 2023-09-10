//===-- MMIXOperand.h - Parse MMIX assembly operands --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXOPERAND_H
#define LLVM_LIB_TARGET_MMIX_ASM_ASMPARSER_MMIXOPERAND_H

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

class MMIXOperand final : public MCParsedAsmOperand {
private:
  enum class KindTy {
    Token,
    Register,
    Immediate,
    Memory,
    Expr,
  } Kind;
  SMLoc StartLoc, EndLoc;
  union ContentTy {
    StringRef Tok;
    std::int64_t Imm;
    MCRegister Reg;
    const MCExpr *Expr;
    ContentTy(StringRef Token);
    ContentTy(const MCExpr *Expr);
    ContentTy(int64_t Imm);
    ContentTy(const MCRegister &Reg);
  } Content;

public:
  StringRef getToken() const;
  std::int64_t getImm() const;
  const MCExpr *getExpr() const;
  void addRegOperands(MCInst &Inst, unsigned N) const;
  void addImmOperands(MCInst &Inst, unsigned N) const;
  void addExprOperands(MCInst &Inst, unsigned N) const;
  void addSpecialRegisterOperands(MCInst &Inst, unsigned N) const {};

public:
  bool isToken() const override;
  bool isImm() const override;
  bool isReg() const override;
  bool isMem() const override;
  bool isGPRExpr() const;
  bool isJumpDest() const;
  bool isBranchDest() const;
  template <std::uint8_t W> bool isUImm() const {
    return Kind == KindTy::Immediate && isUInt<W>(getImm());
  }
  bool isRoundMode() const;
  bool isBaseAddress() const { return true; }
  unsigned getReg() const override;
  SMLoc getStartLoc() const override;
  SMLoc getEndLoc() const override;
  void print(raw_ostream &OS) const override;
  void dump() const override;

public:
  MMIXOperand(StringRef Tok, SMLoc NameLoc, SMLoc EndLoc);
  MMIXOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  MMIXOperand(const std::int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc);
  MMIXOperand(const MCRegister &Reg, SMLoc StartLoc, SMLoc EndLoc);

public:
  static std::unique_ptr<MMIXOperand> createMnemonic(StringRef Mnemonic,
                                                     SMLoc StartLoc);
  static std::unique_ptr<MMIXOperand>
  createExpression(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXOperand> createImm(const std::int64_t &Imm,
                                                SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXOperand> createGPR(const unsigned &RegNo,
                                                SMLoc StartLoc, SMLoc EndLoc);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASMPARSER_MMIXOPERAND_H
