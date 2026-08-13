//===- MMIXaseInfo.cpp - Helper methods for MMIX --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXBaseInfo.h"
#include "MCTargetDesc/MMIXMCTargetDesc.h"

#include <array>

using namespace llvm;

MCRegister MMIX::getSPRFromEnc(unsigned Enc) {
  if (Enc >= 32)
    return MMIX::NoRegister;

  static std::array Map = {
      MMIX::rB,  MMIX::rD,  MMIX::rE,  MMIX::rH, MMIX::rJ, MMIX::rM, MMIX::rR,
      MMIX::rBB, MMIX::rC,  MMIX::rN,  MMIX::rO, MMIX::rS, MMIX::rI, MMIX::rT,
      MMIX::rTT, MMIX::rK,  MMIX::rQ,  MMIX::rU, MMIX::rV, MMIX::rG, MMIX::rL,
      MMIX::rA,  MMIX::rF,  MMIX::rP,  MMIX::rW, MMIX::rX, MMIX::rY, MMIX::rZ,
      MMIX::rWW, MMIX::rXX, MMIX::rYY, MMIX::rZZ};
  return Map[Enc];
}
