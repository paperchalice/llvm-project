#include "MMIXMCInstLower.h"
#include "MCTargetDesc/MMIXMCExpr.h"

#define DEBUG_TYPE "mmix-mcinstlower"

using namespace llvm;

MMIXMCInstLower::MMIXMCInstLower(MCContext &Ctx, const AsmPrinter &AP)
    : Ctx(Ctx), AP(AP) {}

void MMIXMCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) {
  assert(MI && "MI is nullptr");
  outs() << "TODO: implement " << __func__ << " in " << __FILE__ << '\n';
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
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GVal = MO.getGlobal();
    auto S = AP.getSymbol(GVal);
    auto SE = MMIXMCExpr::create(MCSymbolRefExpr::create(S, Ctx),
                                 MMIXMCExpr::VK_MMIX_PC_REL_JMP, Ctx);
    MCOp = MCOperand::createExpr(SE);
  }
  return true;
  default:
    return false;
  }
}
