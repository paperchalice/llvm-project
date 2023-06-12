#include "MMIXMCInstLower.h"

#define DEBUG_TYPE "mmix-mcinstlower"

using namespace llvm;

MMIXMCInstLower::MMIXMCInstLower(MCContext &Ctx, const AsmPrinter &AP)
    : Ctx(Ctx), AP(AP) {}

void MMIXMCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) {
  assert(MI && "MI is nullptr");
  MI->dump();
  switch (MI->getOpcode()) {
  default:
    OutMI.setOpcode(MI->getOpcode());
    break;
  }
  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (lowerOperand(MO, MCOp))
      OutMI.addOperand(MCOp);
  }
}

bool MMIXMCInstLower::lowerOperand(const MachineOperand &MO, MCOperand &MCOp) {
  switch (MO.getType()) {
  case MachineOperand::MO_Register: {
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
  }
    return true;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    return true;
  default:
    return true;
  }
  return true;
}
