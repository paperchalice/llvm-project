//=- MMIXCallingConv.td - Calling Conventions for MMIX -*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for MMIX architecture.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_TD

let Entry = 1 in
def CC_MMIX : CallingConv<[
  // put calling convention here
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[r0]>>,

  // stack
  CCIfType<[i8], CCAssignToStack<1, 1>>,
]>;

let Entry = 1 in
def RetCC_MMIX : CallingConv<[
  // put calling convention here
]>;

//===----------------------------------------------------------------------===//
// Callee-saved Registers.
//===----------------------------------------------------------------------===//
def CSR_MMIX : CalleeSavedRegs<(add r0)>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_TD
