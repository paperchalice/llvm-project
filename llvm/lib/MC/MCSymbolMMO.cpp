#include "llvm/MC/MCSymbolMMO.h"
#include "llvm/BinaryFormat/MMO.h"
#include "llvm/Support/Casting.h"

namespace llvm {

static constexpr auto REG_FLAG = 1;

MCSymbolMMO::MCSymbolMMO(const StringMapEntry<bool> *Name, bool isTemporary)
    : MCSymbol(SymbolKindMMO, Name, isTemporary) {
  setIndex(0);
}
void MCSymbolMMO::setGREG() { setFlags(REG_FLAG); }
void MCSymbolMMO::setReg() { setFlags(REG_FLAG); }
bool MCSymbolMMO::isReg() const { return getFlags() == REG_FLAG; }
void MCSymbolMMO::setVariableValue(const uint64_t &Value, const MCExpr *Expr) {
  MCSymbol::setVariableValue(Expr);
  Equivalent = Value;
}

std::uint64_t MCSymbolMMO::getEquivalent() const { return Equivalent; }

namespace MMIX {
bool IsValidExpr(const MCExpr *Expr, bool AllowFutureReference) {
  int64_t Res;
  bool HasErr = Expr->evaluateAsAbsolute(Res);
  if (HasErr) {
    if (AllowFutureReference && isa<MCSymbolRefExpr>(Expr)) {
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}
} // namespace MMIX

} // namespace llvm
