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
#include "MMIX.h"
#include "MMIXCodeGenPassBuilder.h"
#include "MMIXPassConfig.h"
#include "MMIXSubtarget.h"
#include "MMIXTargetObjectFile.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/CodeGenPassBuilder.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

namespace {
constexpr char MMIXDLStr[] = ""; // TODO: define your data layout string

}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXTarget() {
  RegisterTargetMachine<MMIXTargetMachine> X(getTheMMIXTarget());
  auto PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
}

MMIXTargetMachine::MMIXTargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, MMIXDLStr, TT, CPU, FS, Options,
                        RM.value_or(Reloc::Model::Static),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF([&TT]() -> ::std::unique_ptr<TargetLoweringObjectFile> {
        switch (TT.getObjectFormat()) {

        case Triple::ObjectFormatType::ELF:
          return ::std::make_unique<MMIXELFTargetObjectFile>();

        default:
          break;
        }
        llvm_unreachable_internal("invalid bin format");
      }()) {
  initAsmInfo();
}

const TargetSubtargetInfo *
MMIXTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;
  std::string Key = CPU + TuneCPU + FS;

  // This needs to be done before we create a new subtarget since any
  // creation will depend on the TM and the code generation flags on the
  // function that reside in TargetOptions.
  resetTargetOptions(F);
  auto ABIName = Options.MCOptions.getABIName();
  if (const MDString *ModuleTargetABI = dyn_cast_or_null<MDString>(
          F.getParent()->getModuleFlag("target-abi"))) {
    ABIName = ModuleTargetABI->getString();
  }

  // FIXME: maybe unique_ptr?
  auto I = new MMIXSubtarget(TargetTriple, CPU, TuneCPU, FS, ABIName, *this);
  return I;
}

Error MMIXTargetMachine::buildCodeGenPipeline(
    ModulePassManager &MPM, MachineFunctionPassManager &MFPM,
    MachineFunctionAnalysisManager &MFAM, raw_pwrite_stream &S1,
    raw_pwrite_stream *S2, CodeGenFileType CGFT, CGPassBuilderOption CGPBOpt,
    PassInstrumentationCallbacks *PIC) {
  MMIXCodeGenPassBuilder MMIXCGPB{*this, CGPBOpt, PIC};
  MMIXCGPB.registerAnalyses(MFAM);
  return MMIXCGPB.buildPipeline(MPM, MFPM, S1, S2, CGFT);
}

TargetPassConfig *MMIXTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MMIXPassConfig(*this, PM);
}

TargetLoweringObjectFile *MMIXTargetMachine::getObjFileLowering() const {
  return TLOF.get();
}

TargetTransformInfo
MMIXTargetMachine::getTargetTransformInfo(const Function &F) const {
  // TODO: add your code here.
  return TargetTransformInfo{DataLayout{MMIXDLStr}};
}
