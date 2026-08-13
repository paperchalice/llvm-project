//===-- MMIXMCExpr.cpp - MMIX specific MC expression classes --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H

#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCRegister.h"
#include <tuple>
#include <variant>

namespace llvm {

class MCContext;

class MMIXMCExpr : public MCTargetExpr {
  using FracDivType = std::tuple<const MCExpr *, const MCExpr *>;

public:
  static const MMIXMCExpr *createRegExpr(const MCExpr *Expr, MCContext &Ctx);
  static const MMIXMCExpr *createRegExpr(uint8_t RegNo, MCContext &Ctx) {
    return createRegExpr(MCConstantExpr::create(RegNo, Ctx), Ctx);
  }
  static const MMIXMCExpr *createFracDivExpr(const MCExpr *LHS,
                                             const MCExpr *RHS, MCContext &Ctx);

  bool isRegExpr() const;
  bool isFracDiv() const;
  MCRegister getMCReg() const;

public:
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAssembler *Asm) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override;

  static bool classof(const MCExpr *E);

private:
  std::variant<const MCExpr *, FracDivType> Content;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
