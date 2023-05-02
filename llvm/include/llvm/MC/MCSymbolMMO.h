#ifndef LLVM_MC_MCSYMBOLMMO_H
#define LLVM_MC_MCSYMBOLMMO_H

#include "llvm/MC/MCSymbol.h"

namespace llvm {

class MCSymbolMMO : public MCSymbol {
  std::uint64_t Equivalent;
public:
  MCSymbolMMO(const StringMapEntry<bool> *Name, bool isTemporary);
  void setGREG();
  void setReg();
  bool isReg() const;
  void setVariableValue(const std::uint64_t &Value, const MCExpr *Expr);
  std::uint64_t getEquivalent() const;
  static bool classof(const MCSymbol *S) { return S->isMMO(); }
};

namespace MMIX {

bool IsValidExpr(const MCExpr* Expr, bool AllowFutureReference);

}

} // namespace llvm

#endif // LLVM_MC_MCSYMBOLMMO_H
