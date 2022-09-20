#ifndef LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H
#define LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"
#include "MMIXTargetMachine.h"

namespace llvm {

class MMIXSubtarget;

class MMIXTargetLowering : public TargetLowering {
public:
explicit MMIXTargetLowering(const MMIXTargetMachine &TM,
                                    const MMIXSubtarget &STI);

private:
const MMIXSubtarget &Subtarget;

};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_MMIXTARGETLOWERING_H
