//===-- MMIXOperand.cpp - Parse MMIX assembly operands --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MMIXOperand.h"
#include "MCTargetDesc/MMIXInstPrinter.h"
#include "MCTargetDesc/MMIXMCExpr.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
namespace llvm {

MMIXOperand::ContentTy::ContentTy(StringRef Token) : Tok(Token) {}
MMIXOperand::ContentTy::ContentTy(const MCExpr *Expr) : Expr(Expr) {}
MMIXOperand::ContentTy::ContentTy(int64_t Imm) : Imm(Imm) {}
MMIXOperand::ContentTy::ContentTy(const MCRegister &Reg) : Reg(Reg) {}

StringRef MMIXOperand::getToken() const { return Content.Tok; }
void MMIXOperand::addRegOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXOperand::addImmOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createExpr(getExpr()));
}
void MMIXOperand::addExprOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createExpr(getExpr()));
}
bool MMIXOperand::isToken() const { return Kind == KindTy::Token; }
bool MMIXOperand::isImm() const {
  return (Kind == KindTy::Immediate && !isGPRExpr()) ||
         (Kind == KindTy::Expr && !isGPRExpr());
}
bool MMIXOperand::isReg() const {
  return Kind == KindTy::Register || isGPRExpr();
}
bool MMIXOperand::isMem() const { return Kind == KindTy::Memory; }
bool MMIXOperand::isGPRExpr() const { return Kind == KindTy::Register; }
bool MMIXOperand::isJumpDest() const {
  if (Kind == KindTy::Expr) {
    if (auto E = dyn_cast<MMIXMCExpr>(getExpr())) {
      return E->getKind() == MMIXMCExpr::VariantKind::VK_MMIX_PC_REL_JMP;
    }
  }
  return false;
}
bool MMIXOperand::isBranchDest() const {
  if (Kind == KindTy::Expr) {
    if (auto E = dyn_cast<MMIXMCExpr>(getExpr())) {
      return E->getKind() == MMIXMCExpr::VariantKind::VK_MMIX_PC_REL_BR;
    }
  }
  return false;
}
unsigned MMIXOperand::getReg() const {
  assert(Kind == KindTy::Register && "not register");
  return Content.Reg;
}

bool MMIXOperand::isRoundMode() const {
  auto Expr = getExpr();
  int64_t Res;
  Expr->evaluateAsAbsolute(Res);
  return isUInt<2>(Res) && !isGPRExpr();
}
int64_t MMIXOperand::getImm() const { return 0; }
const MCExpr *MMIXOperand::getExpr() const {
  assert(Kind == KindTy::Expr && "bad operand kind!");
  return Content.Expr;
}
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
    OS << "$"
       << StringRef(MMIXInstPrinter::getRegisterName(getReg())).drop_front();
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

void MMIXOperand::dump() const { print(dbgs()); }

MMIXOperand::MMIXOperand(StringRef Token, SMLoc NameLoc, SMLoc EndLoc)
    : Kind(KindTy::Token), StartLoc(NameLoc), EndLoc(EndLoc), Content(Token) {}
MMIXOperand::MMIXOperand(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Expr), StartLoc(StartLoc), EndLoc(EndLoc), Content(Expr) {}
MMIXOperand::MMIXOperand(const int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Immediate), StartLoc(StartLoc), EndLoc(EndLoc),
      Content(Imm) {}
MMIXOperand::MMIXOperand(const MCRegister &Reg, SMLoc StartLoc, SMLoc EndLoc)
    : Kind(KindTy::Register), StartLoc(StartLoc), EndLoc(EndLoc), Content(Reg) {
}

std::unique_ptr<MMIXOperand> MMIXOperand::createMnemonic(StringRef Mnemonic,
                                                         SMLoc StartLoc) {
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Mnemonic.size());
  return std::make_unique<MMIXOperand>(Mnemonic, StartLoc, EndLoc);
}

std::unique_ptr<MMIXOperand> MMIXOperand::createExpression(const MCExpr *Expr,
                                                           SMLoc StartLoc,
                                                           SMLoc EndLoc) {
  return std::make_unique<MMIXOperand>(Expr, StartLoc, EndLoc);
}

std::unique_ptr<MMIXOperand>
MMIXOperand::createImm(const int64_t &Imm, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXOperand>(Imm, StartLoc, EndLoc);
}
std::unique_ptr<MMIXOperand>
MMIXOperand::createGPR(const unsigned &RegNo, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXOperand>(MCRegister(RegNo), StartLoc, EndLoc);
}

} // namespace llvm
