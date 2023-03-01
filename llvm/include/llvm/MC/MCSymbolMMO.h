#ifndef LLVM_MC_MCSYMBOLMMO_H
#define LLVM_MC_MCSYMBOLMMO_H

#include "llvm/MC/MCSymbol.h"

namespace llvm {

class MCSymbolMMO : public MCSymbol {
public:
  MCSymbolMMO(const StringMapEntry<bool> *Name, bool isTemporary)
      : MCSymbol(SymbolKindMMO, Name, isTemporary) {}
  static bool classof(const MCSymbol *S) { return S->isMMO(); }
};

} // namespace llvm

#endif // LLVM_MC_MCSYMBOLMMO_H
