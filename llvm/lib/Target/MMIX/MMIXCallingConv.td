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

// just use regs as placeholder
let Entry = 1 in def CC_MMIX_Knuth : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[
             r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14
           ]>>,

  // stack
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  CCIfType<[i16], CCAssignToStack<2, 2>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;

let Entry = 1 in def CC_MMIX_Knuth_Callee : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1], CCPromoteToType<i8>>,
  CCIfType<[i8, i16, i32, i64], CCAssignToReg<[
             r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15
           ]>>,
  // stack
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  CCIfType<[i16], CCAssignToStack<2, 2>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;


// just use regs as placeholder
let Entry = 1 in def RetCC_MMIX_Knuth : CallingConv<[
  CCIfType<[iPTR], CCBitConvertToType<i64>>,
  CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
  CCIfType<[i64], CCAssignToReg<[r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15]>>,
  
  // stack
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  CCIfType<[i16], CCAssignToStack<2, 2>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[i64], CCAssignToStack<8, 8>>,
]>;

//===----------------------------------------------------------------------===//
// Callee-saved Registers.
//===----------------------------------------------------------------------===//
def CSR_MMIX_Knuth : CalleeSavedRegs<(add rJ)> {
  let OtherPreserved = (sequence "r%u", 0, 14);
} // set call preserve

#endif // LLVM_LIB_TARGET_MMIX_MMIXCALLINGCONV_TD
