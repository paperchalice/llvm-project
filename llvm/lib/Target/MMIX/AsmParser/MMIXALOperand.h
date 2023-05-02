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
  enum class KindTy {
    Token,
    Register,
    Immediate,
    Memory,
    Expr,
  } Kind;
  const MCExpr *Expr;
  SMLoc StartLoc, EndLoc;
  union ContentTy {
    StringRef Tok;
    std::int64_t Imm;
    MCRegister Reg;
    const MCExpr *Expr;
    ContentTy(StringRef Token);
    ContentTy(const MCExpr *Expr);
    ContentTy(int64_t Imm);
    explicit ContentTy(const MCRegister &Reg);
  } Content;

public:
  StringRef getToken() const;
  std::int64_t getImm() const;
  const MCExpr *getExpr() const;
  void addRegOperands(MCInst &Inst, unsigned N) const;
  void addImmOperands(MCInst &Inst, unsigned N) const;
  void addExprOperands(MCInst &Inst, unsigned N) const;

public:
  bool isToken() const override;
  bool isImm() const override;
  bool isReg() const override;
  bool isMem() const override;
  bool isGPRExpr() const;
  bool isJumpDest() const;
  bool isBranchDest() const;
  bool isUImm8() const;
  bool isUImm16() const;
  bool isUImm24() const;
  bool isRoundMode() const;
  unsigned getReg() const override;
  SMLoc getStartLoc() const override;
  SMLoc getEndLoc() const override;
  void print(raw_ostream &OS) const override;
  void dump() const override;

public:
  MMIXALOperand(StringRef Tok, SMLoc NameLoc, SMLoc EndLoc);
  MMIXALOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  MMIXALOperand(const std::int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc);
  MMIXALOperand(const MCRegister &Reg, SMLoc StartLoc, SMLoc EndLoc);

public:
  static std::unique_ptr<MMIXALOperand> createMnemonic(StringRef Mnemonic,
                                                       SMLoc StartLoc);
  static std::unique_ptr<MMIXALOperand>
  createExpression(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXALOperand> createImm(const std::int64_t &Imm,
                                                  SMLoc StartLoc, SMLoc EndLoc);
  static std::unique_ptr<MMIXALOperand> createGPR(const unsigned &RegNo,
                                                  SMLoc StartLoc, SMLoc EndLoc);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASMPARSER_MMIXOPERAND_H
