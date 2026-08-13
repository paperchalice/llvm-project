//===-- MMIXFixupKinds.h - Fixup kinds for MMIX ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements ELF streamer information for the MMIX backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm::MMIX {

enum Fixups {
  fixup_MMIX_jmp = FirstTargetFixupKind,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXFIXUPKINDS_H
