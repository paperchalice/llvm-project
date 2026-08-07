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
#include "MMIXInstPrinter.h"
#include "MMIXMCAsmInfo.h"
#include "TargetInfo/MMIXTargetInfo.h"

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
  // return new MMIXInstPrinter(MAI, MII, MRI);
  return nullptr;
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
}
