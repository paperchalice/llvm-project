//===-- MMIXALParser.cpp - Parse MMIX assembly language -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXALParser.h"
#include "TargetInfo/MMIXTargetInfo.h"

#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

/// Create an MMIXALParser instance.
MMIXALParser *llvm::createMCMMIXALParser(MCContext &Ctx, MCStreamer &Out,
                                         SourceMgr &SrcMgr,
                                         const MCAsmInfo &MAI) {
  return new MMIXALParser(Ctx, Out, SrcMgr, MAI);
}
