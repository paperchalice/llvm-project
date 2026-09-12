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

defvar ArgRegs = [r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15];

let Entry = 1 in {
def RetCC_MMIX : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i8], CCBitConvertToType<i64>>,
  CCIfType<[i16], CCBitConvertToType<i64>>,
  CCIfType<[i32], CCBitConvertToType<i64>>,
  CCIfType<[i64, v8i8, v4i16, v2i32], CCCustom<"assignToRegCustom<&MMIXRegisterInfo::getReturnThreshold>">>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;

def CC_MMIX_Callee : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i8], CCBitConvertToType<i64>>,
  CCIfType<[i16], CCBitConvertToType<i64>>,
  CCIfType<[i32], CCBitConvertToType<i64>>,
  CCIfType<[v8i8], CCBitConvertToType<i64>>,
  CCIfType<[i64, v8i8, v4i16, v2i32], CCCustom<"assignToRegCustom<&MMIXRegisterInfo::getParamThreshold>">>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;

def CC_MMIX_Caller : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i8], CCBitConvertToType<i64>>,
  CCIfType<[i16], CCBitConvertToType<i64>>,
  CCIfType<[i32], CCBitConvertToType<i64>>,
  CCIfType<[i64, v8i8, v4i16, v2i32], CCCustom<"assignToRegCustom<&MMIXRegisterInfo::getParamThreshold>">>, // in fact, use vregs
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONVENTION_TD
