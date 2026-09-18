//=- MMIXSchedDefault.td - MMIX default Scheduling Defs ------*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the machine model for MMIX to support
// instruction scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_MMIXSCHEDDEFAULT_TD
#define LLVM_LIB_TARGET_MMIX_MMIXSCHEDDEFAULT_TD

def DefaultModel : SchedMachineModel {
  let IssueWidth = 5;
  let MicroOpBufferSize = 5;
  let PostRAScheduler = 1;
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXSCHEDDEFAULT_TD
