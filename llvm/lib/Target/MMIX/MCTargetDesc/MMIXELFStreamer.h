//===-- MMIXELFStreamer.h - ELF Streamer for MMIX -------------------------===//
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

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXELFSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXELFSTREAMER_H

#include "MMIXTargetStreamer.h"

#include "llvm/MC/MCELFStreamer.h"

namespace llvm {

class MMIXELFStreamer : public MCELFStreamer {
public:
  using MCELFStreamer::MCELFStreamer;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXELFSTREAMER_H
