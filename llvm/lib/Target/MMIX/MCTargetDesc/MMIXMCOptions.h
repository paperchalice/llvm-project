//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCOPTIONS_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCOPTIONS_H

#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Option/LibraryOptions.h"
#include "llvm/Support/LibraryOptionsContext.h"

#define OPTIONS_STRUCT_DECL
#include "MMIXMCOptions.inc"

namespace llvm::MMIX_MC {
inline const MMIXMCOptions &getCLOpts(const MCTargetOptions &Options) {
  return cl::LibraryOptionsContext::get<MMIXMCOptions>(Options.LibraryOptions);
}
} // namespace llvm::MMIX_MC

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCOPTIONS_H
