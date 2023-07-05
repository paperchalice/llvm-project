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

class MMIXReg<bits<16> enc, string n, list<string> alt = []> : Register<n, alt> {
  let Namespace = "MMIX";
  let HWEncoding = enc;
}

class MMIXGPR<bits<16> enc> : MMIXReg<enc, "" # enc>{
  let DwarfNumbers = [enc];
}

class MMIXSR<bits<16> enc, string n> : MMIXReg<enc, n> {
  let DwarfNumbers = [!add(enc, 0x100)];
}

// define general purpose registers, on MMIX, just a 8-bit number
// the result of this codes are: Reg1 ... Reg255
foreach i = {0...255} in {
  def r # i: MMIXGPR<i>;
}

def rA: MMIXSR<21, "rA">; // arithmetic status register
def rB: MMIXSR<0, "rB">; // bootstrap register (trip)
def rC: MMIXSR<8, "rC">; // continuation register
def rD: MMIXSR<1, "rD">; // dividend register
def rE: MMIXSR<2, "rE">; // epsilon register
def rF: MMIXSR<22, "rF">; // failure location register
def rG: MMIXSR<19, "rG">; // global threshold register
def rH: MMIXSR<3, "rH">; // himult register
def rI: MMIXSR<12, "rI">; // interval counter
def rJ: MMIXSR<4, "rJ">; // return-jump register
def rK: MMIXSR<15, "rK">; // interrupt mask register
def rL: MMIXSR<20, "rL">; // local threshold register
def rM: MMIXSR<5, "rM">; // multiplex mask register
def rN: MMIXSR<9, "rN">; // serial number
def rO: MMIXSR<10, "rO">; // register stack offset
def rP: MMIXSR<23, "rP">; // prediction register
def rQ: MMIXSR<16, "rQ">; // interrupt request register
def rR: MMIXSR<6, "rR">; // remainder register
def rS: MMIXSR<11, "rS">; // register stack pointer
def rT: MMIXSR<13, "rT">; // trap address register
def rU: MMIXSR<17, "rU">; // usage counter
def rV: MMIXSR<18, "rV">; // virtual translation register
def rW: MMIXSR<24, "rW">; // where-interrupted register (trip)
def rX: MMIXSR<25, "rX">; // execution register (trip)
def rY: MMIXSR<26, "rY">; // Y operand (trip)
def rZ: MMIXSR<27, "rZ">; // Z operand (trip)
def rBB: MMIXSR<7, "rBB">; // bootstrap register (trap)
def rTT: MMIXSR<14, "rTT">; // dynamic trap address register
def rWW: MMIXSR<28, "rWW">; // where-interrupted register (trap)
def rXX: MMIXSR<29, "rXX">; // execution register (trap)
def rYY: MMIXSR<30, "rYY">; // Y operand (trap)
def rZZ: MMIXSR<31, "rZZ">; // Z operand (trap)

// example register class, here is gpr
def GPR: RegisterClass<"MMIX", [v8i8, v4i16, v2i32, i64], 64, (add (sequence "r%u", 0, 255))>;
def SPR: RegisterClass<"MMIX", [i64], 64, (add (sequence "r%c", 0x41, 0x5a), rBB, rTT, rWW, rXX, rYY, rZZ)>;
#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERINFO_TD
