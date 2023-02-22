#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class MMIXMCExpr : public MCTargetExpr {
public:
  static const MCExpr *create(const MCExpr *Expr,
                                   MCContext &Ctx);

public:
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override;
  void fixELFSymbolsInTLSFixups(MCAssembler &) const override;

  static bool classof(const MCExpr *E);
 
  static bool classof(const MMIXMCExpr *);

private:
  MMIXMCExpr(const MCExpr* Expr);
private:
  const MCExpr *Expr;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCEXPR_H
