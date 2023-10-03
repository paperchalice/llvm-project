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

def CC_MMIX_Knuth_Reg : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[
             r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15
           ]>>,
]>;

def CC_MMIX_Knuth_Caller_Reg : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[
             r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31
           ]>>,
]>;

let Entry = 1 in def CC_MMIX_Knuth : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32, i64], CCDelegateTo<CC_MMIX_Knuth_Reg>>,

  // stack
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  CCIfType<[i16], CCAssignToStack<2, 2>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;

let Entry = 1 in def CC_MMIX_Knuth_Caller : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32, i64], CCDelegateTo<CC_MMIX_Knuth_Caller_Reg>>,

  // stack
  CCAssignToStack<0, 0>,
]>;

let Entry = 1 in def RetCC_MMIX_Knuth : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[r0]>>,
]>;

let Entry = 1 in def RetCC_MMIX_Knuth_Caller : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[r15]>>,
]>;

//===----------------------------------------------------------------------===//
// Callee-saved Registers.
//===----------------------------------------------------------------------===//
def CSR_MMIX_Knuth : CalleeSavedRegs<(add rJ)> {
  let OtherPreserved = (sequence "r%u", 0, 14);
} // set call preserve

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_TD
