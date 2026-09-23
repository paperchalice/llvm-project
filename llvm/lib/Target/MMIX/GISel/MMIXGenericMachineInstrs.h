//===- MMIXGenericMachineInstrs.h -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// Declares convenience wrapper classes for interpreting MachineInstr instances
/// as MMIX specific generic operations.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MMIX_GISEL_MMIXGENERICMACHINEiNSTRS_H
#define LLVM_LIB_TARGET_MMIX_GISEL_MMIXGENERICMACHINEiNSTRS_H

#include "MCTargetDesc/MMIXMCTargetDesc.h"

#include "llvm/CodeGen/GlobalISel/GenericMachineInstrs.h"

namespace llvm::MMIX {

enum class Cond {
  N,  //< negative
  Z,  //< zero
  P,  //< positive
  OD, //< odd
  NN, // nonnegative
  NZ, // nonzero
  NP, // nonpositive
  EV, // even
};

constexpr inline Cond getNegCond(Cond C) {
  switch (C) {
  case Cond::N:
    return Cond::NN;
  case Cond::Z:
    return Cond::NZ;
  case Cond::P:
    return Cond::NP;
  case Cond::OD:
    return Cond::EV;
  case Cond::NN:
    return Cond::N;
  case Cond::NZ:
    return Cond::Z;
  case Cond::NP:
    return Cond::P;
  case Cond::EV:
    return Cond::OD;
  }
}

/// @brief G_ZS/G_CS instructions
class GSelectIf : public GenericMachineInstr {
public:
  Cond getCond() const { return static_cast<Cond>(getOperand(1).getImm()); }
  Register getCondReg() const { return getReg(2); }
  Register getTReg() const { return getReg(3); }
  Register getFReg() const { return getReg(4); }

  static bool classof(const MachineInstr *MI) {
    return MI->getOpcode() == MMIX::G_SELECT_IF;
  }
};

} // namespace llvm::MMIX

#endif // LLVM_LIB_TARGET_MMIX_GISEL_MMIXGENERICMACHINEiNSTRS_H
