//=----- MMIXInstrGISel.td - MMIX GISel target pseudos -*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// MMIX GlobalISel target pseudo instruction definitions. This is kept
// separately from the other tablegen files for organizational purposes, but
// share the same infrastructure.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD

class MMIXGenericInstruction : GenericInstruction {
  let Namespace = "MMIX";
}

//===----------------------------------------------------------------------===//
// MMIX specific generic instructions
//===----------------------------------------------------------------------===//

// Generic floating-point comparison.
def G_3WAY_FCMP : MMIXGenericInstruction {
  let OutOperandList = (outs type0:$dst);
  let InOperandList = (ins type1:$src1, type1:$src2);
  let hasSideEffects = true;
}
def SDTFPCmpOp : SDTypeProfile<1, 2, [
  SDTCisSameAs<1, 2>, SDTCisInt<0>, SDTCisFP<1>
]>;
def mmix_3way_fcmp : SDNode<"ISD::MMIX_3WAY_FCMP", SDTFPCmpOp, []>;

def G_SELECT_IF : MMIXGenericInstruction {
  let OutOperandList = (outs type0:$dst);
  let InOperandList = (ins unknown:$cond, type1:$tst, type1:$src1, type1:$src2);
  let hasSideEffects = true;
}
def SDTSelectIfOp : SDTypeProfile<1, 4, [
  SDTCisSameAs<3, 4>, SDTCisSameAs<0, 3>,
  SDTCisInt<1>, SDTCisInt<2>
]>;
def selectif : SDNode<"ISD::MMIX_SET_IF", SDTSelectIfOp, []>;
foreach i = !range(!size(CondList)) in {
  defvar Cond = CondList[i];
  def selectif#Cond : PatFrag<(ops node:$tst, node:$tt, node:$ff),
                              (selectif i, node:$tst, node:$tt, node:$ff)>;
}

//===----------------------------------------------------------------------===//
// Missing GINodeEquiv
//===----------------------------------------------------------------------===//

def : GINodeEquiv<G_UCMP, ucmp>;
def : GINodeEquiv<G_SCMP, scmp>;

def : GINodeEquiv<G_3WAY_FCMP, mmix_3way_fcmp>;
def : GINodeEquiv<G_SELECT_IF, selectif>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD
