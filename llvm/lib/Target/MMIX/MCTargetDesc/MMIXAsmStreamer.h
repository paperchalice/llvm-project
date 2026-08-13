//===-- MMIXAsmStreamer.h - Generic Streamer for MMIX ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares LLVM ASM style streamer, but syntax is more to MMIXAL.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMSTREAMER_H

#include "llvm/MC/MCAsmStreamer.h"

namespace llvm {

// TODO: Inherit MCAsmStreamer in future if possible...
class MMIXAsmStreamer {
public:
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXASMSTREAMER_H
