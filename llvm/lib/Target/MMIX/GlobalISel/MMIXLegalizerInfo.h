#ifndef LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXLEGALIZERINFO_H
#define LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXLEGALIZERINFO_H

#include "llvm/CodeGen/GlobalISel/LegalizerHelper.h"
#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"

namespace llvm {

class MMIXSubtarget;

class MMIXLegalizerInfo: public LegalizerInfo {
public:
  MMIXLegalizerInfo(const MMIXSubtarget &ST);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXLEGALIZERINFO_H
