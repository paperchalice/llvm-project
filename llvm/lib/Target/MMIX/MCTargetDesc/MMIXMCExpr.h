#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class MMIXMCExpr : public MCTargetExpr {
  // may used by disassembler
public:
  enum VariantKind {
    VK_MMIX_REG_EXPR,
    VK_MMIX_PC_REL_JMP,
    VK_MMIX_PC_REL_BR,
    VK_ROUND_MODE,
  };
  static const MCExpr *create(const MCExpr *Expr, VariantKind Kind,
                              MCContext &Ctx);
  static const MCExpr *create(const MCExpr *Expr, std::uint64_t PC,
                              bool EmitFixup, VariantKind Kind, MCContext &Ctx);

public:
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override;
  void fixELFSymbolsInTLSFixups(MCAssembler &) const override;

  static bool classof(const MCExpr *E);

public:
  static bool isGPRExpr(const MCExpr *Expr);
  bool shouldEmitFixup() const;
  VariantKind getKind() const;
  const MCExpr *getExpr() const { return Expr; }

private:
  MMIXMCExpr(const MCExpr *Expr, VariantKind Kind);
  MMIXMCExpr(const MCExpr *Expr, std::uint64_t PC, bool EmitFixup, VariantKind Kind);

private:
  const MCExpr *Expr;
  std::uint64_t PC = 0;
  bool EmitFixup = false;
  const VariantKind Kind;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
