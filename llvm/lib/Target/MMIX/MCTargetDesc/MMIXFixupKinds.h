#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm::MMIX {

enum Fixups {
  fixup_MMIX = FirstTargetFixupKind - 1,
#define MMIX_FIXUP_X(A0, A1, A2, A3) A0,
#include "MMIXFixupX.def"

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
