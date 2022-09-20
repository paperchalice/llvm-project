//=- MMIXRegisterBank.td - Describe the MMIX Banks ---------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKS_TD
#define LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKS_TD

/// General Purpose Registers, example.
def GPRBank : RegisterBank<"GPRBank", [GPR]>;

#endif // LLVM_LIB_TARGET_MMIX_MMIXREGISTERBANKS_TD
