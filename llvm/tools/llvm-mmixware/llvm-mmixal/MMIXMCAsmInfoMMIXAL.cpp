#include "MMIXMCAsmInfoMMIXAL.h"
#include "llvm/MC/MCTargetOptions.h"

namespace llvm {

MMIXMCAsmInfoMMIXAL::MMIXMCAsmInfoMMIXAL(const MCTargetOptions &Options)
    : Options(Options) {}

MMIXMCAsmInfoMMIXAL *createMMIXMCAsmInfoMMIXAL(const MCTargetOptions &Options) {
  return new MMIXMCAsmInfoMMIXAL(Options);
}

} // namespace llvm
