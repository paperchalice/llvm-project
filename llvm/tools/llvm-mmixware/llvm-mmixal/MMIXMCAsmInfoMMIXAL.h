#ifndef LLVM_MMIXAL_MMIXALINFO_H
#define LLVM_MMIXAL_MMIXALINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class MCStreamer;
class Target;
class Triple;

struct MMIXMCAsmInfoMMIXAL : public MCAsmInfo {
  const MCTargetOptions &Options;
  MMIXMCAsmInfoMMIXAL(const MCTargetOptions &Options);
  bool StrictMode;
};

MMIXMCAsmInfoMMIXAL *createMMIXMCAsmInfoMMIXAL(const MCTargetOptions &Options);

} // namespace llvm

#endif // LLVM_MMIXAL_MMIXALINFO_H
