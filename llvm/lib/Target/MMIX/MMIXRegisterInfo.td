//===-- MMIXRegisterInfo.td - MMIX Register defs -------------*- tablegen
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
#define LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD

include "llvm/TableGen/SearchableTable.td"

//===----------------------------------------------------------------------===//
//  Declarations that describe the MMIX register file
//===----------------------------------------------------------------------===//


def SFREnc : GenericEnum {
  let FilterClass = "SFREncEnum";
  let NameField = "Name";
  let ValueField = "Encoding";
}

class SFREncEnum<bits<16> enc> {
  string Name = NAME;
  bits<16> Encoding = enc;
}

class MMIXReg<bits<16> enc, string n, list<string> alt = []>
    : Register<n, alt> {
  let Namespace = "MMIX";
  let HWEncoding = enc;
}

class MMIXGPR<bits<16> enc> : MMIXReg<enc, "" #enc> {
  let DwarfNumbers = [enc];
}

class MMIXSR<bits<16> enc, string n = NAME> : MMIXReg<enc, n>, SFREncEnum<enc> {
  let DwarfNumbers = [!add(enc, 0x100)];
}

// define general purpose registers, on MMIX, just a 8-bit number
// the result of this codes are: Reg1 ... Reg255
foreach i = {0...255} in {
  def r #i : MMIXGPR<i>;
}

def rB : MMIXSR<0>;    // bootstrap register (trip)
def rD : MMIXSR<1>;    // dividend register
def rE : MMIXSR<2>;    // epsilon register
def rH : MMIXSR<3>;    // himult register
def rJ : MMIXSR<4>;    // return-jump register
def rM : MMIXSR<5>;    // multiplex mask register
def rR : MMIXSR<6>;    // remainder register
def rBB : MMIXSR<7>;  // bootstrap register (trap)
def rC : MMIXSR<8>;    // continuation register
def rN : MMIXSR<9>;    // serial number
def rO : MMIXSR<10>;   // register stack offset
def rS : MMIXSR<11>;   // register stack pointer
def rI : MMIXSR<12>;   // interval counter
def rT : MMIXSR<13>;   // trap address register
def rTT : MMIXSR<14>; // dynamic trap address register
def rK : MMIXSR<15>;   // interrupt mask register
def rQ : MMIXSR<16>;   // interrupt request register
def rU : MMIXSR<17>;   // usage counter
def rV : MMIXSR<18>;   // virtual translation register
def rG : MMIXSR<19>;   // global threshold register
def rL : MMIXSR<20>;   // local threshold register
def rA : MMIXSR<21>;   // arithmetic status register
def rF : MMIXSR<22>;   // failure location register
def rP : MMIXSR<23>;   // prediction register
def rW : MMIXSR<24>;   // where-interrupted register (trip)
def rX : MMIXSR<25>;   // execution register (trip)
def rY : MMIXSR<26>;   // Y operand (trip)
def rZ : MMIXSR<27>;   // Z operand (trip)
def rWW : MMIXSR<28>; // where-interrupted register (trap)
def rXX : MMIXSR<29>; // execution register (trap)
def rYY : MMIXSR<30>; // Y operand (trap)
def rZZ : MMIXSR<31>; // Z operand (trap)

// example register class, here is gpr
def GPR : RegisterClass<"MMIX", [v8i8, v4i16, v2i32, i64], 64,
                        (add(sequence "r%u", 0, 255))>;
def SPR : RegisterClass<"MMIX", [i64], 64,
                        (add(sequence "r%c", 0x41, 0x5a), rBB, rTT, rWW, rXX,
                         rYY, rZZ)> {
  let isAllocatable = false;
}
#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
