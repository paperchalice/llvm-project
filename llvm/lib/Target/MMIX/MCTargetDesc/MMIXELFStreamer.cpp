//===- lib/MC/MMIXELFStreamer.cpp - ELF Object Output for MMIX ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file assembles .s files and emits MMIX ELF .o object files. Different
// from generic ELF streamer in emitting mapping symbols ( and ) to delimit
// regions of data and code.
//
//===----------------------------------------------------------------------===//

#include "MMIXTargetStreamer.h"
