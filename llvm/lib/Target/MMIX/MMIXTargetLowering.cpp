#include "MMIXTargetLowering.h"
#include "MMIXSubtarget.h"
#include "MMIXTargetMachine.h"

using namespace llvm;

MMIXTargetLowering::MMIXTargetLowering(const MMIXTargetMachine &TM,
                                       const MMIXSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Compute derived properties from the register classes
  addRegisterClass(MVT::i64, &MMIX::GPRRegClass);
  computeRegisterProperties(Subtarget.getRegisterInfo());
}

bool MMIXTargetLowering::functionArgumentNeedsConsecutiveRegisters(
    Type *Ty, CallingConv::ID CallConv, bool isVarArg,
    const DataLayout &DL) const {
  return Ty->isAggregateType();
}
