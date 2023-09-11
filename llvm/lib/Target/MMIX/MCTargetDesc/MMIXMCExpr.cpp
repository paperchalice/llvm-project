#include "MMIXMCExpr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Casting.h"

namespace llvm {

const MCExpr *MMIXMCExpr::create(const MCExpr *Expr,
                                 MMIXMCExpr::VariantKind Kind, MCContext &Ctx) {
  return new (Ctx) MMIXMCExpr(Expr, Kind);
}

const MCExpr *MMIXMCExpr::create(const MCExpr *Expr, std::uint64_t PC,
                                 VariantKind Kind, MCContext &Ctx) {
  return new (Ctx) MMIXMCExpr(Expr, PC, Kind);
}

MMIXMCExpr::MMIXMCExpr(const MCExpr *Expr, MMIXMCExpr::VariantKind Kind)
    : Expr(Expr), Kind(Kind) {}
MMIXMCExpr::MMIXMCExpr(const MCExpr *Expr, uint64_t PC,
                       MMIXMCExpr::VariantKind Kind)
    : Expr(Expr), PC(PC), Kind(Kind) {}
bool MMIXMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {
  switch (Kind) {
  case MMIXMCExpr::VK_MMIX_REG_EXPR:
  case MMIXMCExpr::VK_ROUND_MODE:
    return Expr->evaluateAsRelocatable(Res, Layout, Fixup);
  case MMIXMCExpr::VK_MMIX_PC_REL_BR:
  case MMIXMCExpr::VK_MMIX_PC_REL_JMP: {
    int64_t Val;
    if (Expr->evaluateAsAbsolute(Val)) {
      Res = MCValue::get(static_cast<int64_t>(Val - PC) / 4);
      return true;
    } else
      return false;
  }
  }
  return Expr->evaluateAsRelocatable(Res, Layout, Fixup);
}

void MMIXMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &) const {}

void MMIXMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*Expr);
}

MCFragment *MMIXMCExpr::findAssociatedFragment() const {
  return Expr->findAssociatedFragment();
}

void MMIXMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  switch (Kind) {
  case MMIXMCExpr::VK_MMIX_REG_EXPR:
    OS << "$(";
    Expr->print(OS, MAI);
    OS << ")";
    return;
  case MMIXMCExpr::VK_MMIX_PC_REL_JMP:
  case MMIXMCExpr::VK_MMIX_PC_REL_BR:
    OS << "@ + ";
    Expr->print(OS, MAI);
    return;
  case MMIXMCExpr::VK_ROUND_MODE: {
    std::int64_t Res = 0;
    Expr->evaluateAsAbsolute(Res);
    switch (Res) {
    case 0:
      OS << "ROUND_CURRENT";
    case 1:
    case 2:
    case 3:
    case 4:
      break;
    default:
      break;
    }
  }
  }
}

bool MMIXMCExpr::classof(const MCExpr *E) {
  return E->getKind() == MCExpr::Target;
}

bool MMIXMCExpr::classof(const MMIXMCExpr *) { return true; }

bool MMIXMCExpr::isGPRExpr(const MCExpr *Expr) {
  if (auto E = dyn_cast<MMIXMCExpr>(Expr)) {
    return E->Kind == MMIXMCExpr::VK_MMIX_REG_EXPR;
  }

  if (auto E = dyn_cast<MCSymbolRefExpr>(Expr)) {
    auto MMOSymbol = cast<MCSymbolMMO>(&E->getSymbol());
    return MMOSymbol->isReg();
  }

  if (const auto *BinExpr = dyn_cast<MCBinaryExpr>(Expr)) {
    switch (BinExpr->getOpcode()) {
    default:
      return false;
    case MCBinaryExpr::Add:
      return MMIXMCExpr::isGPRExpr(BinExpr->getLHS()) !=
             MMIXMCExpr::isGPRExpr(BinExpr->getRHS());
    case MCBinaryExpr::Sub: {
      bool IsRegLHS = MMIXMCExpr::isGPRExpr(BinExpr->getLHS());
      bool IsRegRHS = MMIXMCExpr::isGPRExpr(BinExpr->getRHS());
      if (IsRegLHS && IsRegRHS)
        return false;
      else if (IsRegLHS && !IsRegRHS)
        return true;
      else
        return false;
    }
    }
  } else {
    return false;
  }
}

MMIXMCExpr::VariantKind MMIXMCExpr::getKind() const { return Kind; }

} // namespace llvm
