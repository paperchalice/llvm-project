//===-- MMIXMCTargetDesc.h - MMIX Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides MMIX specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H
#define LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCSchedule.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {

class Target;
class MCAsmBackend;
class MCInstrAnalysis;
class MCAsmInfo;
class MCCodeEmitter;
class MCRegisterInfo;
class MCContext;
class MCStreamer;

MCAsmBackend *createMMIXAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

MCInstrAnalysis *createMMIXInstrAnalysis(const MCInstrInfo *Info);

MCCodeEmitter *createMMIXMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx);

MCAsmInfo *createMMIXMCAsmInfo(const MCRegisterInfo &MRI,
                               const Triple &TheTriple,
                               const MCTargetOptions &Options);

MCInstPrinter *createMMIXMCInstPrinter(const Triple &T, unsigned SyntaxVariant,
                                       const MCAsmInfo &MAI,
                                       const MCInstrInfo &MII,
                                       const MCRegisterInfo &MRI);

MCStreamer *createMMIXELFStreamer(const Triple &T, MCContext &Context,
                                  std::unique_ptr<MCAsmBackend> &&MAB,
                                  std::unique_ptr<MCObjectWriter> &&OW,
                                  std::unique_ptr<MCCodeEmitter> &&Emitter,
                                  bool RelaxAll);

MCStreamer *createMMIXMMOStreamer(MCContext &Context,
                                  std::unique_ptr<MCAsmBackend> &&MAB,
                                  std::unique_ptr<MCObjectWriter> &&OW,
                                  std::unique_ptr<MCCodeEmitter> &&Emitter);

MCRegisterInfo *createMMIXMCRegisterInfo(const Triple &Triple);

MCSubtargetInfo *createMMIXMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                           StringRef FS);

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MCTARGETDESC_MMIXMCTARGETDESC_H
