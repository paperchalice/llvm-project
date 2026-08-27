//===-- MMIXInstrInfo.cpp - MMIX Instruction Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the MMIX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MMIXInstrInfo.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"
#include "MMIXSubtarget.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "MMIXGenInstrInfo.inc"

using namespace llvm;

MMIXInstrInfo::MMIXInstrInfo(const MMIXSubtarget &STI)
    : MMIXGenInstrInfo(STI, RI, /*CFSetupOpcode=*/MMIX::ADJCALLSTACKDOWN,
                       /*CFDestroyOpcode=*/MMIX::ADJCALLSTACKUP,
                       /*CatchRetOpcode=*/~0u,
                       /*ReturnOpcode=*/MMIX::POP),
      STI(STI), RI() {}
