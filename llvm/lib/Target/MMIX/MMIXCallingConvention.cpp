//=== MMIXCallingConvention.cpp - MMIX CC entry points --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file define the entry points for MMIX calling convention analysis.
///
//===----------------------------------------------------------------------===//

#include "MMIXCallingConvention.h"

using namespace llvm;

// TableGen provides definitions of the calling convention analysis entry
// points.
#define GET_CALLING_CONV_IMPL
#include "MMIXGenCallingConv.inc"
