//===-- MMIXTargetObjectFile.h - MMIX Object Info -*- C++ ---------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_MMIX_MMIXTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class MMIXELFTargetObjectFile : public TargetLoweringObjectFileELF {

};

class MMIXMMOTargetObjectFile : public TargetLoweringObjectFileMMO {

};

class MMIXCOFFTargetObjectFile : public TargetLoweringObjectFileCOFF {

};

class MMIXMachOTargetObjectFile : public TargetLoweringObjectFileMachO {

};

}// namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXTARGETOBJECTFILE_H
