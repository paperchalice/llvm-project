#include "MMIXMCExpr.h"
#include "llvm/MC/MCContext.h"

namespace llvm {

const MCExpr *MMIXMCExpr::create(const MCExpr *Expr, MCContext &Ctx) {
  return new (Ctx) MMIXMCExpr(Expr);
}

MMIXMCExpr::MMIXMCExpr(const MCExpr *Expr) : Expr(Expr) {}

bool MMIXMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {
  return Expr->evaluateAsRelocatable(Res, Layout, Fixup);
}

void MMIXMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &) const {}

void MMIXMCExpr::visitUsedExpr(MCStreamer &Streamer) const {}

MCFragment *MMIXMCExpr::findAssociatedFragment() const {
  return Expr->findAssociatedFragment();
}

void MMIXMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  OS << "$(";
  Expr->print(OS, MAI);
  OS << ")";
}

bool MMIXMCExpr::classof(const MCExpr *E) {
  return E->getKind() == MCExpr::Target;
}

bool MMIXMCExpr::classof(const MMIXMCExpr *) { return true; }

} // namespace llvm
