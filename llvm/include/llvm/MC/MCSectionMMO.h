#ifndef LLVM_MC_MCSECTIONMMO_H
#define LLVM_MC_MCSECTIONMMO_H

#include "llvm/MC/MCSection.h"
#include "llvm/MC/SectionKind.h"

namespace llvm {

class MCSymbol;

class MCSectionMMO final : public MCSection {
  friend class MCContext;
  MCSectionMMO(StringRef Name, SectionKind K, MCSymbol *Begin)
      : MCSection(SV_COFF, Name, K, Begin) {}

public:
  void printSwitchToSection(const MCAsmInfo &, const Triple &, raw_ostream &,
                            const MCExpr *) const override;
  bool useCodeAlign() const override;
  bool isVirtualSection() const override;
};

} // namespace llvm

#endif // LLVM_MC_MCSECTIONMMO_H
