//=== MMIXCallingConvention.h - MMIX CC entry points ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the entry points for MMIX calling convention analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_H
#define LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_H

#include "llvm/CodeGen/CallingConvLower.h"

namespace llvm {
bool CC_MMIX_Knuth(unsigned ValNo, MVT ValVT, MVT LocVT,
                    CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                    CCState &State);

bool RetCC_MMIX_Knuth(unsigned ValNo, MVT ValVT, MVT LocVT,
                CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                CCState &State);
} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_H
