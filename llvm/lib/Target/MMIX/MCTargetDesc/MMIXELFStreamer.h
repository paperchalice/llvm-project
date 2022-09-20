//===-- MMIXELFStreamer.h - ELF Streamer for MMIX ---------*- C++ -*-===//
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

class MMIXELFStreamer : public MMIXTargetStreamer {

};

MCELFStreamer *createMMIXELFStreamer(MCContext &Context,
                                        std::unique_ptr<MCAsmBackend> TAB,
                                        std::unique_ptr<MCObjectWriter> OW,
                                        std::unique_ptr<MCCodeEmitter> Emitter,
                                        bool RelaxAll);
}

#endif
