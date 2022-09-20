//===-- MMIXRegisterInfo.td - MMIX Register defs -------------*- tablegen -*-===//
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

let Namespace = "MMIX" in {

class MMIXReg<bits<16> Enc, string n, list<string> alt = []> : Register<n, alt> {
  let HWEncoding = Enc;
}

// example register
//  this register store return address
def RA  : MMIXReg<0, "RA">, DwarfRegNum<[0]>;
def SP  : MMIXReg<1, "SP">, DwarfRegNum<[1]>;

} // Namespace = "MMIX"

// example register class, here is gpr
def GPR : RegisterClass<"MMIX", [i64], 64, (add
    RA
  )> {

}

#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
