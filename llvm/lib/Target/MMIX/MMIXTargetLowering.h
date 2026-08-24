//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// MMIX specific lowering information.
///
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H
#define LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class MMIXTargetLowering : public TargetLowering {
public:
  using TargetLowering::TargetLowering;

public:
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H
