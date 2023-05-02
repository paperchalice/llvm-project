#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm::MMIX {

enum Fixups {
  fixup_MMIX = FirstTargetFixupKind,
  fixup_MMIX_backward,
  fixup_MMIX_fixo,
  fixup_MMIX_fixr,
  fixup_MMIX_fixrx,
  fixup_MMIX_expand,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
