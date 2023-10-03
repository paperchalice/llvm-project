//=== MMIXCallingConv.cpp - MMIX Custom Calling Convention Impl   -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of custom routines for the MMIX
// Calling Convention that aren't done by tablegen.
//
//===----------------------------------------------------------------------===//

#include "MMIXCallingConv.h"
#include "MMIXRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
using namespace llvm;

#include "MMIXGenCallingConv.inc"
