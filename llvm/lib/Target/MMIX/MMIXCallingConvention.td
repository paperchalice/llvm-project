//=- MMIXCallingConv.td - Calling Conventions for AArch64 -------------------=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This describes the calling conventions for MMIX architecture.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_TD

let Entry = 1 in
def RetCC_MMIX : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[r0]>>,
]>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_TD
