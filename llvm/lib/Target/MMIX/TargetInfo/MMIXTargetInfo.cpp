//===-- MMIXTargetInfo.cpp - MMIX Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/MMIXTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheMMIXTarget() {
  static Target TheMMIXTarget;
  return TheMMIXTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeMMIXTargetInfo() {
  RegisterTarget<Triple::mmix, /*HasJIT=*/false> Target1(
      getTheMMIXTarget(), "mmix", "A RISC Computer for the Third Millennium", "MMIX");
}
