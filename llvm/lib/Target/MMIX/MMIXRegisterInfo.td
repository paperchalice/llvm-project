//===-- MMIXRegisterInfo.td - MMIX Register defs -----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
#define LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD

//===----------------------------------------------------------------------===//
//  Declarations that describe the MMIX register file
//===----------------------------------------------------------------------===//

class MMIXReg<bits<16> enc, string n, list<string> alt = []>
    : Register<n, alt> {
  let Namespace = "MMIX";
  let HWEncoding = enc;
}

class MMIXGPR<bits<16> enc> : MMIXReg<enc, "" #enc> {
  let DwarfNumbers = [enc];
}

// define general purpose registers, on MMIX, just a 8-bit number
// the result of this codes are: Reg1 ... Reg255
foreach i = {0...255} in {
  def r#i : MMIXGPR<i>;
}

def GPR : RegisterClass<"MMIX", [v8i8, v4i16, v2i32, i64, f64], 64,
                        (add(sequence "r%u", 0, 255))>;

// Map the pointer-like operands of the target-independent pseudo instructions
// (STACKMAP, PATCHPOINT, ...) onto a concrete MMIX register class.
defm : RemapAllTargetPseudoPointerOperands<GPR>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
