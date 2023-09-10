//===-- MMIXALOperand.cpp - Parse MMIX assembly operands --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXALOperand.h"
#include "MCTargetDesc/MMIXInstPrinter.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
namespace llvm {

MMIXALOperand::ContentTy::ContentTy(StringRef Token) : Tok(Token) {}
MMIXALOperand::ContentTy::ContentTy(const MCExpr *Expr) : Expr(Expr) {}
MMIXALOperand::ContentTy::ContentTy(int64_t Imm) : Imm(Imm) {}
MMIXALOperand::ContentTy::ContentTy(const MCRegister &Reg) : Reg(Reg) {}

StringRef MMIXALOperand::getToken() const { return Content.Tok; }
void MMIXALOperand::addRegOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXALOperand::addSpecialRegisterOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXALOperand::addImmOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(getImm()));
}
void MMIXALOperand::addExprOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createExpr(getExpr()));
}
bool MMIXALOperand::isToken() const { return Kind == KindTy::Token; }
bool MMIXALOperand::isImm() const { return Kind == KindTy::Immediate; }
bool MMIXALOperand::isReg() const { return Kind == KindTy::Register; }
bool MMIXALOperand::isMem() const { return Kind == KindTy::Memory; }
bool MMIXALOperand::isJumpDest() const {
  if (Kind == KindTy::Expr) {
    if (auto E = dyn_cast<MMIXMCExpr>(getExpr())) {
      return E->getKind() == MMIXMCExpr::VariantKind::VK_MMIX_PC_REL_JMP;
    }
  }
  return false;
}
bool MMIXALOperand::isBranchDest() const {
  if (Kind == KindTy::Expr) {
    if (auto E = dyn_cast<MMIXMCExpr>(getExpr())) {
      return E->getKind() == MMIXMCExpr::VariantKind::VK_MMIX_PC_REL_BR;
    }
  }
  return false;
}
unsigned MMIXALOperand::getReg() const {
  assert(Kind == KindTy::Register && "not register");
  return Content.Reg;
}

bool MMIXALOperand::isRoundMode() const {
  return Kind == KindTy::Immediate && Content.Imm < 5 && Content.Imm >= 0;
}
int64_t MMIXALOperand::getImm() const {
  assert(Kind == KindTy::Immediate && "not immediate");
  return Content.Imm;
}
const MCExpr *MMIXALOperand::getExpr() const {
  assert(Kind == KindTy::Expr && "bad operand kind!");
  return Content.Expr;
}
SMLoc MMIXALOperand::getStartLoc() const { return StartLoc; }
SMLoc MMIXALOperand::getEndLoc() const { return EndLoc; }
void MMIXALOperand::print(raw_ostream &OS) const {
  switch (Kind) {
  default:
    break;
  case KindTy::Token:
    OS << getToken();
    break;
  case KindTy::Register:
    OS << "$" << StringRef(MMIXInstPrinter::getRegisterName(getReg()));
    break;
  case KindTy::Immediate:
    OS << getImm();
    break;
  case KindTy::Expr: {
    auto Expr = getExpr();
    Expr->print(OS, nullptr);
  } break;
  }
}

void MMIXALOperand::dump() const { print(dbgs()); }

MMIXALOperand::MMIXALOperand(StringRef Token, SMLoc NameLoc, SMLoc EndLoc)
    : Kind(KindTy::Token), StartLoc(NameLoc), EndLoc(EndLoc), Content(Token) {}
MMIXALOperand::MMIXALOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Expr), StartLoc(StartLoc), EndLoc(EndLoc), Content(Expr) {}
MMIXALOperand::MMIXALOperand(const int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Immediate), StartLoc(StartLoc), EndLoc(EndLoc),
      Content(Imm) {}
MMIXALOperand::MMIXALOperand(const MCRegister &Reg, SMLoc StartLoc,
                             SMLoc EndLoc)
    : Kind(KindTy::Register), StartLoc(StartLoc), EndLoc(EndLoc), Content(Reg) {
}

std::unique_ptr<MMIXALOperand> MMIXALOperand::createMnemonic(StringRef Mnemonic,
                                                             SMLoc StartLoc) {
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Mnemonic.size());
  return std::make_unique<MMIXALOperand>(Mnemonic, StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
MMIXALOperand::createExpression(const MCExpr *Expr, SMLoc StartLoc,
                                SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(Expr, StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
MMIXALOperand::createImm(const int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(Imm, StartLoc, EndLoc);
}
std::unique_ptr<MMIXALOperand>
MMIXALOperand::createReg(const unsigned &RegNo, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(MCRegister(RegNo), StartLoc, EndLoc);
}

} // namespace llvm
