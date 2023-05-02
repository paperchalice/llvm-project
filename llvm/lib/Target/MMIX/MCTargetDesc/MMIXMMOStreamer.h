//===-- MMIXMMOStreamer.h - MMO Streamer for MMIX ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements MMO streamer information for the MMIX backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMMOSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMMOSTREAMER_H

#include "MMIXTargetStreamer.h"
#include "llvm/MC/MCMMOStreamer.h"

namespace llvm {

class MMIXMMOStreamer : public MMIXTargetStreamer {

};

MCMMOStreamer *createMMIXMMOStreamer(MCContext &Context,
                                        std::unique_ptr<MCAsmBackend> TAB,
                                        std::unique_ptr<MCObjectWriter> OW,
                                        std::unique_ptr<MCCodeEmitter> Emitter,
                                        bool RelaxAll);
}

#endif
