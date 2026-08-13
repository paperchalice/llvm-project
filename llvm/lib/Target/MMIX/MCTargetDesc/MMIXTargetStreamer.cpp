//===- MMIXTargetStreamer.cpp - MMIXTargetStreamer class ------------------===//
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
#include "MMIXMCExpr.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"

#include <string>

using namespace llvm;

bool MMIXTargetStreamer::isOSBinFormatELF() {
  return getContext().getSubtargetInfo()->getTargetTriple().isOSBinFormatELF();
}

void MMIXTargetStreamer::emitGREG(StringRef Name) {
  MCContext &Ctx = getContext();
  int64_t LastGReg = getLastGlobalReg();
  const MCExpr &Expr = *GlobalRegs[254 - LastGReg];
  bool HasName = !Name.empty();
  MCStreamer &Streamer = getStreamer();
  Streamer.emitRawComment(" allocate GREG $" + std::to_string(LastGReg));
  // TODO: implement register alias
  [[maybe_unused]] MCSymbol *Symbol =
      HasName ? Ctx.getOrCreateSymbol(Name) : Ctx.createTempSymbol();
  Streamer.pushSection();
  if (isOSBinFormatELF()) {
    MCSection *RegContents = Ctx.getELFSection(
        ".MMIX.reg_contents", ELF::SHT_PROGBITS, ELF::SHF_WRITE);
    Streamer.switchSection(RegContents);
  }
  Streamer.emitValue(&Expr, sizeof(uint64_t));
  Streamer.popSection();
}

void MMIXTargetAsmStreamer::emitPREFIX() {
  MCStreamer &S = getStreamer();
  S.emitRawComment("\t% .PREFIX\t\":" + getPrefix() + "\"",
                   /*TabPrefix=*/false);
}

MCTargetStreamer *
llvm::createMMIXObjectTargetStreamer(MCStreamer &S,
                                     const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new MMIXTargetELFStreamer(S);
  return new MMIXTargetStreamer(S);
}
