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
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/SMLoc.h"

namespace llvm {

class MMIXOperand final: public MCParsedAsmOperand {
private:
  enum class KindTy { Token, Register, Immediate, Memory } Kind;
  SMLoc StartLoc, EndLoc;
  union ContentTy {
    StringRef Tok;
    MCRegister Reg;
    const MCExpr* Imm;
    const MCExpr* Mem;

    ContentTy(StringRef Token);
    ContentTy(unsigned RegNo);
    ContentTy(const MCExpr *Expr);
  } Content;
public:
  StringRef getToken() const;
  void addRegOperands(MCInst &Inst, unsigned N) const;
  void addImmOperands(MCInst &Inst, unsigned N) const;

public:
  bool isToken() const override;
  bool isImm() const override;
  bool isReg() const override;
  unsigned getReg() const override;
  int64_t getImm() const;
  bool isMem() const override;
  SMLoc getStartLoc() const override;
  SMLoc getEndLoc() const override;
  void print(raw_ostream &OS) const override;

public:
  MMIXOperand(StringRef Tok, SMLoc NameLoc, SMLoc EndLoc);
  MMIXOperand(unsigned, SMLoc StartLoc, SMLoc EndLoc);
  MMIXOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_ASMPARSER_MMIXOPERAND_H
