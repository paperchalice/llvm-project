//===-- MMIXTargetStreamer.h - MMIX Target Streamer ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class MMIXTargetStreamer : public MCTargetStreamer {
public:
explicit MMIXTargetStreamer(MCStreamer &S);
};

class MMIXTargetAsmStreamer : public MMIXTargetStreamer {
public:
  explicit MMIXTargetAsmStreamer(MCStreamer &S);
};

MCTargetStreamer *
createMMIXObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

MCTargetStreamer *createMMIXNullTargetStreamer(MCStreamer &S);

MCTargetStreamer *createMMIXAsmTargetStreamer(MCStreamer &S,
                                              formatted_raw_ostream &OS,
                                              MCInstPrinter *InstPrint,
                                              bool isVerboseAsm);

}

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXTARGETSTREAMER_H
