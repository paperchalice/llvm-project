//===-- MMIXTargetMachine.h - Define TargetMachine for MMIX ---------------===//
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

#include "MMIXSubtarget.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"

#include <memory>
#include <optional>

namespace llvm {

class MMIXTargetMachine : public CodeGenTargetMachineImpl {
public:
  MMIXTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);

public:
  const MMIXSubtarget *getSubtargetImpl(const Function &F) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<MMIXSubtarget>> SubtargetMap;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXTARGETMACHINE_H
