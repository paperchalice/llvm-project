//===- MMIXTargetStreamer.cpp - MMIXTargetStreamer class ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MMIXTargetStreamer class.
//
//===----------------------------------------------------------------------===//

#include "MMIXTargetStreamer.h"

namespace llvm {

MMIXTargetStreamer::MMIXTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

MMIXTargetAsmStreamer::MMIXTargetAsmStreamer(MCStreamer &S): MMIXTargetStreamer(S) {}

MCTargetStreamer *
createMMIXObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new MMIXTargetStreamer(S);
}

MCTargetStreamer *
createMMIXNullTargetStreamer(MCStreamer &S) {
  return new MMIXTargetStreamer(S);
}

MCTargetStreamer *createMMIXAsmTargetStreamer(MCStreamer &S,
                                              formatted_raw_ostream &OS,
                                              MCInstPrinter *InstPrint,
                                              bool isVerboseAsm) {
  return new MMIXTargetAsmStreamer(S);
}

} // namespace llvm
