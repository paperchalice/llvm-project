//===-- MMIXMCTargetDesc.cpp - MMIX Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// This file provides MMIX-specific target descriptions.
///
//===----------------------------------------------------------------------===//

#include "MMIXMCTargetDesc.h"
#include "MMIXAsmBackend.h"
#include "MMIXInstPrinter.h"
#include "MMIXMCAsmInfo.h"
#include "MMIXMCCodeEmitter.h"
#include "MMIXMCInstrAnalysis.h"
#include "MMIXMCInstrInfo.h"
#include "MMIXRegisterInfo.h"
#include "MMIXSubtarget.h"
#include "MMIXTargetStreamer.h"
#include "TargetInfo/MMIXTargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_SUBTARGETINFO_MC_DESC
#include "MMIXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MMIXGenRegisterInfo.inc"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXTargetMC() {
  decltype(getTheMMIXTarget) *TargetGetterList[] = {
      getTheMMIXTarget,
  };

  for (const auto &Getter : TargetGetterList) {
    Target &T = Getter();
    // Register the MC asm info.
    TargetRegistry::RegisterMCAsmInfo(T, createMMIXMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(T, createMMIXMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(T, createMMIXMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(T, createMMIXMCSubtargetInfo);

    // Register the MC instruction analyzer.
    TargetRegistry::RegisterMCInstrAnalysis(T, createMMIXInstrAnalysis);

    // Register the MC Code Emitter
    TargetRegistry::RegisterMCCodeEmitter(T, createMMIXMCCodeEmitter);

    // Register the obj file streamers.

    TargetRegistry::RegisterELFStreamer(T, createMMIXELFStreamer);

    TargetRegistry::RegisterMMOStreamer(T, createMMIXMMOStreamer);

    // Register the null TargetStreamer.
    TargetRegistry::RegisterNullTargetStreamer(T, createMMIXNullTargetStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(T, createMMIXAsmTargetStreamer);

    // Register the obj target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(
        T, createMMIXObjectTargetStreamer);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(T, createMMIXMCInstPrinter);

    // Register the asm backend.
    // TargetRegistry::RegisterMCAsmBackend(T, createMMIXAsmBackend);
  }
}

namespace llvm {

MCStreamer *createMMIXELFStreamer(const Triple &T, MCContext &Context,
                                  std::unique_ptr<MCAsmBackend> &&MAB,
                                  std::unique_ptr<MCObjectWriter> &&OW,
                                  std::unique_ptr<MCCodeEmitter> &&Emitter,
                                  bool RelaxAll) {
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter), RelaxAll);
}

MCStreamer *createMMIXMMOStreamer(MCContext &Context,
                                  std::unique_ptr<MCAsmBackend> &&MAB,
                                  std::unique_ptr<MCObjectWriter> &&OW,
                                  std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return createMMOStreamer(Context, std::move(MAB), std::move(OW),
                               std::move(Emitter));
}

MCRegisterInfo *createMMIXMCRegisterInfo(const Triple &Triple) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // TODO: add Return Address register as 2nd parameter
  InitMMIXMCRegisterInfo(X, MMIX::r0);
  // TODO: add Code view reg to mc reg conversion
  return X;
}

MCSubtargetInfo *createMMIXMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                           StringRef FS) {
  if (CPU.empty()) {
    CPU = "generic";
  }
  return createMMIXMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

} // namespace llvm
