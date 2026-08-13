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
#include "MMIXAsmStreamer.h"
#include "MMIXELFStreamer.h"
#include "MMIXInstPrinter.h"
#include "MMIXMCAsmInfo.h"
#include "MMIXMCCodeEmitter.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "MMIXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MMIXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MMIXGenRegisterInfo.inc"

using namespace llvm;

static MCAsmInfo *createMMIXMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TheTriple,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new MMIXMCAsmInfoELF(Options);
  return MAI;
}

static MCInstrInfo *createMMIXMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMMIXMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMMIXMCRegisterInfo(const Triple &Triple) {
  MCRegisterInfo *X = new MCRegisterInfo();
  // TODO: add Return Address register as 2nd parameter
  InitMMIXMCRegisterInfo(X, MMIX::r0);
  // TODO: add Code view reg to mc reg conversion
  return X;
}

static MCSubtargetInfo *createMMIXMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  return createMMIXMCSubtargetInfoImpl(TT, CPU, /*TuneCPU=*/CPU, FS);
}

static MCInstPrinter *createMMIXMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new MMIXInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *createMMIXNullTargetStreamer(MCStreamer &S) {
  return new MMIXTargetStreamer(S);
}

static MCTargetStreamer *createMMIXAsmTargetStreamer(MCStreamer &S,
                                                     formatted_raw_ostream &OS,
                                                     MCInstPrinter *InstPrint) {
  return new MMIXTargetAsmStreamer(S, OS);
}

static MCStreamer *
createMMIXELFStreamer(const Triple &T, MCContext &Ctx,
                      std::unique_ptr<MCAsmBackend> &&TAB,
                      std::unique_ptr<MCObjectWriter> &&OW,
                      std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return new MMIXELFStreamer(Ctx, std::move(TAB), std::move(OW),
                             std::move(Emitter));
}

static MCAsmBackend *createMMIXAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &MRI,
                                          const MCTargetOptions &Options) {
  return new MMIXAsmBackend(STI, MRI, Options);
}

MCStreamer *createMMIXAsmStreamer(MCContext &Ctx,
                                  std::unique_ptr<formatted_raw_ostream> OS,
                                  std::unique_ptr<MCInstPrinter> IP,
                                  std::unique_ptr<MCCodeEmitter> CE,
                                  std::unique_ptr<MCAsmBackend> MAB) {
  return createAsmStreamer(Ctx, std::move(OS), std::move(IP), std::move(CE),
                           std::move(MAB));
}

static MCCodeEmitter *createMMIXMCCodeEmitter(const MCInstrInfo &MCII,
                                              MCContext &Ctx) {
  return new MMIXMCCodeEmitter(MCII, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXTargetMC() {
  Target &T = getTheMMIXTarget();

  // Register the MC asm info.
  RegisterMCAsmInfoFn RMCAInfo(T, createMMIXMCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(T, createMMIXMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(T, createMMIXMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(T, createMMIXMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(T, createMMIXMCInstPrinter);

  // Register the null TargetStreamer.
  TargetRegistry::RegisterNullTargetStreamer(T, createMMIXNullTargetStreamer);

  // Register MMIX style AsmStreamer.
  TargetRegistry::RegisterAsmStreamer(T, createMMIXAsmStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(T, createMMIXAsmTargetStreamer);

  // Register the obj target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(T,
                                               createMMIXObjectTargetStreamer);

  // Register the ELF streamer.
  TargetRegistry::RegisterELFStreamer(T, createMMIXELFStreamer);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(T, createMMIXAsmBackend);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(T, createMMIXMCCodeEmitter);
}
