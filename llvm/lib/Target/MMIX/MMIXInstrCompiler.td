//===-- MMIXInstrCompiler.td - Pseudos and Patterns --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file describes the various pseudo instructions used by the compiler,
/// as well as Pat patterns used during instruction selection.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRCOMPILER_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRCOMPILER_TD

class Pseudo<dag outs, dag ins, string opstr=NAME>
    : MMIXInst<0, outs, ins, opstr, "; error: this should not be emitted"> {
  let isPseudo = 1;
}

def ADJCALLSTACKDOWN : Pseudo<(outs), (ins)>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins)>;

/**
 * \brief pseudo call instruction, will lower to PUSHJ/PUSHGO
 * the out operand is the hole register defined by POP, i.e. the register
 * number of X fied of PUSHJ/PUSHGO. first operand is dest function,
 * rest is arguments.
*/
def MP_CALL : Pseudo<(outs unknown:$hole), (ins unknown:$func, variable_ops)>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRCOMPILER_TD
