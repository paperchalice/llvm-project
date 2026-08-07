//===-- MMIXTargetMachine.cpp - Define TargetMachine for MMIX -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the MMIX specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#include "MMIXTargetMachine.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXTarget() {
  // Register the target.
  RegisterTargetMachine<MMIXTargetMachine> RTM0(getTheMMIXTarget());
}

MMIXTargetMachine::MMIXTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               RM.value_or(Reloc::Model::Static),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
  setGlobalISel(true);
  setFastISel(false);
  setO0WantsFastISel(false);
  setRequiresStructuredCFG(false);
}

const MMIXSubtarget *
MMIXTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  StringRef CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString() : getTargetCPU();
  StringRef TuneCPU = TuneAttr.isValid() ? TuneAttr.getValueAsString() : CPU;
  StringRef FS =
      FSAttr.isValid() ? FSAttr.getValueAsString() : getTargetFeatureString();
  std::string Key = (CPU + TuneCPU + FS).str();

  auto &I = SubtargetMap[Key];
  if (!I)
    I = std::make_unique<MMIXSubtarget>(TargetTriple, CPU, TuneCPU, FS, *this);
  return I.get();
}
