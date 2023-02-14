#include "llvm/MC/MCSectionMMO.h"

namespace llvm {

void MCSectionMMO::printSwitchToSection(const MCAsmInfo &, const Triple &,
                                        raw_ostream &, const MCExpr *) const {}
bool MCSectionMMO::useCodeAlign() const { return false; }

bool MCSectionMMO::isVirtualSection() const { return false; }
} // namespace llvm

// namespace llvmm
