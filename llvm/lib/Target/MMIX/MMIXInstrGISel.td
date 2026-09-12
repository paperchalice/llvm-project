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

//===----------------------------------------------------------------------===//
// Missing GINodeEquiv
//===----------------------------------------------------------------------===//

def : GINodeEquiv<G_UCMP, ucmp>;
def : GINodeEquiv<G_SCMP, scmp>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD
