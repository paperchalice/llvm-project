//===-- MMIXTargetMachine.h - Define TargetMachine for MMIX ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the MMIX specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXTARGETMACHINE_H
#define LLVM_LIB_TARGET_MMIX_MMIXTARGETMACHINE_H

#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>

namespace llvm {

class MMIXTargetMachine final : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
public:
  MMIXTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                     CodeGenOpt::Level OL, bool JIT);
  const TargetSubtargetInfo *getSubtargetImpl(const Function &F) const override;

  Error buildCodeGenPipeline(ModulePassManager &MPM,
                                   MachineFunctionPassManager &MFPM,
                                   MachineFunctionAnalysisManager &MFAM,
                                   raw_pwrite_stream &S1, raw_pwrite_stream *S2,
                                   CodeGenFileType CGFT, CGPassBuilderOption CGPBOpt,
                                   PassInstrumentationCallbacks *PIC) override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXTARGETMACHINE_H
