//===-- MMIXInstrInfo.td - Target Description for MMIX ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the MMIX instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD

//===----------------------------------------------------------------------===//
// Instruction Formats
//===----------------------------------------------------------------------===//

include "MMIXInstrFormats.td"

def Inst: MMIXInst<0x0000, (outs GPR:$dst), (ins GPR:$src1, GPR:$src2), "inst", "$dst, $src1, $src2", []>;
def :Pat<(add GPR:$src1, GPR:$src2), (Inst GPR:$src1, GPR:$src2)>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRINFO_TD