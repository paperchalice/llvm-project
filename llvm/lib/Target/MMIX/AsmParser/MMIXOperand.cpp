//===-- MMIXOperand.cpp - Parse MMIX assembly operands --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXOperand.h"
#include "MCTargetDesc/MMIXInstPrinter.h"
#include "llvm/Support/raw_ostream.h"
namespace llvm {

MMIXOperand::ContentTy::ContentTy(StringRef Token) : Tok(Token) {}
MMIXOperand::ContentTy::ContentTy(const unsigned &RegNo) : Reg(RegNo) {}
MMIXOperand::ContentTy::ContentTy(const MCExpr *Expr) : Imm(Expr) {}

StringRef MMIXOperand::getToken() const { return Content.Tok; }
void MMIXOperand::addRegOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXOperand::addImmOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createImm(getImm()));
}
bool MMIXOperand::isToken() const { return Kind == KindTy::Token; }
bool MMIXOperand::isImm() const { return Kind == KindTy::Immediate; }
bool MMIXOperand::isReg() const { return Kind == KindTy::Register; }
unsigned MMIXOperand::getReg() const {
  assert(isReg() && "Operand is not Register");
  return Content.Reg.id();
}
int64_t MMIXOperand::getImm() const {
  int64_t Res = 0;
  Content.Imm->evaluateAsAbsolute(Res);
  return Res;
}
bool MMIXOperand::isMem() const { return Kind == KindTy::Memory; }
SMLoc MMIXOperand::getStartLoc() const { return StartLoc; }
SMLoc MMIXOperand::getEndLoc() const { return EndLoc; }
void MMIXOperand::print(raw_ostream &OS) const {
  switch (Kind) {
  default:
    break;
  case KindTy::Token:
    OS << getToken();
    break;
  case KindTy::Register:
    OS << MMIXInstPrinter::getRegisterName(getReg());
    break;
  case KindTy::Immediate:
    OS << getImm();
    break;
  }
}

MMIXOperand::MMIXOperand(StringRef Token, SMLoc NameLoc, SMLoc EndLoc)
    : Kind(KindTy::Token), StartLoc(NameLoc), EndLoc(EndLoc), Content(Token) {}
MMIXOperand::MMIXOperand(const unsigned &RegNo, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Register), StartLoc(StartLoc), EndLoc(EndLoc),
      Content(RegNo) {}
MMIXOperand::MMIXOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Immediate), StartLoc(StartLoc), EndLoc(EndLoc),
      Content(Expr) {}

std::unique_ptr<MMIXOperand> MMIXOperand::createMnemonic(StringRef Mnemonic,
                                                         SMLoc StartLoc) {
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Mnemonic.size());
  return std::make_unique<MMIXOperand>(Mnemonic, StartLoc, EndLoc);
}

std::unique_ptr<MMIXOperand> MMIXOperand::createRegister(const unsigned &RegNo,
                                                         SMLoc StartLoc,
                                                         SMLoc EndLoc) {
  return std::make_unique<MMIXOperand>(RegNo, StartLoc, EndLoc);
}
} // namespace llvm
