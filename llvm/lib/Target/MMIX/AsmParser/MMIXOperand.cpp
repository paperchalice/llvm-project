//===-- MMIXOperand.cpp - Parse MMIX assembly operands --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXOperand.h"
#include "MCTargetDesc/MMIXInstPrinter.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

bool MMIXOperand::isToken() const {
  return std::holds_alternative<StringRef>(Content);
}
bool MMIXOperand::isImm() const {
  return std::holds_alternative<std::int64_t>(Content);
}
bool MMIXOperand::isReg() const {
  return std::holds_alternative<MCRegister>(Content);
}
bool MMIXOperand::isMem() const { return false; }

bool MMIXOperand::isRoundingMode() const {
  if (!isImm())
    return false;
  std::int64_t Val = getImm();
  return 0 <= Val && Val <= 4;
}

bool MMIXOperand::isBranchDest() const {
  return std::holds_alternative<const MCExpr *>(Content);
}

StringRef MMIXOperand::getToken() const { return std::get<StringRef>(Content); }
void MMIXOperand::addRegOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXOperand::addImmOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(getImm()));
}
void MMIXOperand::addBranchDestOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createExpr(getBranchDest()));
}

MCRegister MMIXOperand::getReg() const {
  assert(isReg() && "not register");
  return std::get<MCRegister>(Content);
}
std::int64_t MMIXOperand::getImm() const {
  assert(isImm() && "not immediate");
  return std::get<std::int64_t>(Content);
}
const MCExpr *MMIXOperand::getBranchDest() const {
  return std::get<const MCExpr *>(Content);
}

void MMIXOperand::print(raw_ostream &OS, const MCAsmInfo &MAI) const {
  if (isToken()) {
    OS << getToken();
  } else if (isReg()) {
    OS << MMIXInstPrinter::getRegisterName(getReg());
  } else if (isImm()) {
    OS << getImm();
  } else if (isBranchDest()) {
    MAI.printExpr(OS, *getBranchDest());
  }
}

MMIXOperand::MMIXOperand(StringRef Token, SMLoc NameLoc, SMLoc EndLoc)
    : StartLoc(NameLoc), EndLoc(EndLoc), Content(Token) {}

std::unique_ptr<MMIXOperand> MMIXOperand::createMnemonic(StringRef Mnemonic,
                                                         SMLoc StartLoc) {
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Mnemonic.size());
  return std::make_unique<MMIXOperand>(Mnemonic, StartLoc, EndLoc);
}

std::unique_ptr<MMIXOperand>
MMIXOperand::createReg(MCRegister Reg, SMLoc StartLoc, SMLoc EndLoc) {
  auto Op = std::make_unique<MMIXOperand>(StartLoc, EndLoc);
  Op->Content = Reg;
  return Op;
}

std::unique_ptr<MMIXOperand>
MMIXOperand::createImm(std::int64_t Imm, SMLoc StartLoc, SMLoc EndLoc) {
  auto Op = std::make_unique<MMIXOperand>(StartLoc, EndLoc);
  Op->Content = Imm;
  return Op;
}

std::unique_ptr<MMIXOperand>
MMIXOperand::createBranchDest(const MCExpr *S, SMLoc StartLoc, SMLoc EndLoc) {
  auto Op = std::make_unique<MMIXOperand>(StartLoc, EndLoc);
  Op->Content = S;
  return Op;
}
