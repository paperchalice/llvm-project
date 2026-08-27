//=== MMIXCallingConvention.h - MMIX CC entry points ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the entry points for MMIX calling convention analysis.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_H
#define LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_H

#include "llvm/CodeGen/CallingConvLower.h"

namespace llvm {

LLVM_LIBRARY_VISIBILITY CCAssignFn CC_MMIX_Caller;

LLVM_LIBRARY_VISIBILITY CCAssignFn CC_MMIX_Callee;

LLVM_LIBRARY_VISIBILITY CCAssignFn RetCC_MMIX;

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_H
