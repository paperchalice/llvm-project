//===-- MMIXLegalizerInfo.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for MMIX.
//===----------------------------------------------------------------------===//

#include "MMIXLegalizerInfo.h"

using namespace llvm;

MMIXLegalizerInfo::MMIXLegalizerInfo() {
  using namespace TargetOpcode;
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);
  const LLT p0 = LLT::pointer(0, 64);

  // refer to llvm/Target/GenericOpcodes.td
  getActionDefinitionsBuilder({G_ANYEXT, G_SEXT, G_ZEXT})
      .legalForCartesianProduct({s64}, {s8, s16, s32});

  getActionDefinitionsBuilder(G_TRUNC)
    .legalForCartesianProduct({s8, s16, s32, s64}, {s8, s16, s32, s64})
    .clampScalar(0, s8, s64)
    .clampScalar(1, s8, s64)
    .widenScalarToNextPow2(0)
    .widenScalarToNextPow2(1);

  getActionDefinitionsBuilder({G_TRUNC_SSAT_S, G_TRUNC_SSAT_U, G_TRUNC_USAT_U})
      .unsupported();

  getActionDefinitionsBuilder(G_IMPLICIT_DEF).legalFor({s64});

  //------------------------------------------------------------------------------
  // Binary ops.
  //------------------------------------------------------------------------------

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_SDIV, G_UDIV, G_SREM,
                               G_UREM, G_SDIVREM, G_UDIVREM, G_AND, G_OR, G_XOR,
                               G_SHL, G_LSHR, G_ASHR, G_USUBSAT})
      .legalFor({s64})
      .clampScalar(0, s64, s64);

  // G_ABDS, G_ABDU, G_UAVGFLOOR

  // TODO: lower G_ROTR like to Knuth style MOR

  // FIXME: G_{S,U}CMP legalization is broken in LLVM
  getActionDefinitionsBuilder({G_SELECT, G_ICMP, G_SCMP, G_UCMP})
      .legalForCartesianProduct({s64, p0}, {s64, p0})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64);

  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({s64, p0})
      .clampScalar(0, s64, s64);
  getActionDefinitionsBuilder(G_FCONSTANT).legalFor({s64, s32});

  getActionDefinitionsBuilder({G_LOAD, G_SEXTLOAD, G_ZEXTLOAD, G_STORE})
      .legalForCartesianProduct({s64, p0}, {p0})
      .clampScalar(0, s64, s64);
}
