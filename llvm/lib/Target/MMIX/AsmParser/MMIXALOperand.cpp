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
#include "MCTargetDesc/MMIXMCInstrInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;
} // namespace

StringRef MMIXALOperand::getToken() const {
  return std::get<StringRef>(Content);
}

void MMIXALOperand::addRegOperands(MCInst &Inst, unsigned N) const {
  Inst.addOperand(MCOperand::createReg(getReg()));
}
void MMIXALOperand::addImmOperands(MCInst &Inst, unsigned N) const {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(getImm()));
}
void MMIXALOperand::addMemOperands(MCInst &Inst, unsigned N) const {
  auto Mem = std::get<Memory>(Content);
  std::visit(
      overload{[&Inst, &Mem](std::uint64_t Dest) {
                 std::int64_t Imm =
                     static_cast<std::int64_t>(Dest - Mem.CurrentAddress) / 4;
                 // thanks to tablegen, the B version is always +1
                 if (Imm < 0)
                   Inst.setOpcode(Inst.getOpcode() + 1);
                 Inst.addOperand(MCOperand::createImm(Imm));
               },
               [&Inst](const MCSymbolRefExpr *S) {
                 Inst.addOperand(MCOperand::createExpr(S));
               }},
      Mem.DestinationAddress);
}

bool MMIXALOperand::isToken() const {
  return std::holds_alternative<StringRef>(Content);
}
bool MMIXALOperand::isImm() const {
  return std::holds_alternative<std::int64_t>(Content);
}
bool MMIXALOperand::isReg() const {
  return std::holds_alternative<MCRegister>(Content);
}
bool MMIXALOperand::isMem() const {
  return std::holds_alternative<Memory>(Content);
}
bool MMIXALOperand::isJumpDest() const {
  return std::holds_alternative<Memory>(Content);
}

unsigned MMIXALOperand::getReg() const { return std::get<MCRegister>(Content); }

bool MMIXALOperand::isRoundMode() const {
  if (auto Imm = std::get_if<std::int64_t>(&Content))
    return *Imm >= 0 && *Imm < 5;
  else
    return false;
}
std::int64_t MMIXALOperand::getImm() const {
  return std::get<std::int64_t>(Content);
}
std::uint64_t MMIXALOperand::getConcreteMem() const {
  auto Mem = std::get<Memory>(Content);
  return std::get<std::uint64_t>(Mem.DestinationAddress);
}

SMLoc MMIXALOperand::getStartLoc() const { return StartLoc; }
SMLoc MMIXALOperand::getEndLoc() const { return EndLoc; }
void MMIXALOperand::print(raw_ostream &OS) const {
  std::visit(overload{[&OS](StringRef Tok) { OS << Tok; },
                      [&OS](std::int64_t Imm) { OS << Imm; },
                      [&OS](MCRegister Reg) {
                        auto RegName = MMIXInstPrinter::getRegisterName(Reg);
                        if (isdigit(RegName[0]))
                          OS << '$';
                        OS << RegName;
                      },
                      [&OS](Memory Mem) {
                        OS << '[';
                        std::visit(
                            overload{[&OS](std::uint64_t Addr) { OS << Addr; },
                                     [&OS](const MCSymbolRefExpr *Symb) {
                                       OS << Symb->getSymbol().getName();
                                     }},
                            Mem.DestinationAddress);
                        OS << ']';
                      }},
             Content);
}

void MMIXALOperand::dump() const { print(dbgs()); }

MMIXALOperand::MMIXALOperand(StringRef Token, SMLoc NameLoc, SMLoc EndLoc)
    : StartLoc(NameLoc), EndLoc(EndLoc), Content(Token) {}
MMIXALOperand::MMIXALOperand(int64_t Imm, SMLoc StartLoc, SMLoc EndLoc)
    : StartLoc(StartLoc), EndLoc(EndLoc), Content(Imm) {}
MMIXALOperand::MMIXALOperand(MCRegister Reg, SMLoc StartLoc, SMLoc EndLoc)
    : StartLoc(StartLoc), EndLoc(EndLoc), Content(Reg) {}
llvm::MMIXALOperand::MMIXALOperand(std::uint64_t Dest, std::uint64_t PC,
                                   SMLoc StartLoc, SMLoc EndLoc)
    : StartLoc(StartLoc), EndLoc(EndLoc), Content(Memory{Dest, PC}) {}
llvm::MMIXALOperand::MMIXALOperand(const MCSymbolRefExpr *SymbolRef,
                                   std::uint64_t PC, SMLoc StartLoc,
                                   SMLoc EndLoc)
    : StartLoc(StartLoc), EndLoc(EndLoc), Content(Memory{SymbolRef, PC}) {}

std::unique_ptr<MMIXALOperand> MMIXALOperand::createMnemonic(StringRef Mnemonic,
                                                             SMLoc StartLoc) {
  SMLoc EndLoc = SMLoc::getFromPointer(StartLoc.getPointer() + Mnemonic.size());
  return std::make_unique<MMIXALOperand>(Mnemonic, StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
MMIXALOperand::createImm(std::int64_t Imm, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(Imm, StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
MMIXALOperand::createReg(unsigned RegNo, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(MCRegister(RegNo), StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
llvm::MMIXALOperand::createMem(std::uint64_t Dest, std::uint64_t PC,
                               SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(Dest, PC, StartLoc, EndLoc);
}

std::unique_ptr<MMIXALOperand>
llvm::MMIXALOperand::createMem(const MCSymbolRefExpr *SymbolRef,
                               std::uint64_t PC, SMLoc StartLoc, SMLoc EndLoc) {
  return std::make_unique<MMIXALOperand>(SymbolRef, PC, StartLoc, EndLoc);
}
