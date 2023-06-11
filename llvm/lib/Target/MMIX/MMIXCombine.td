#ifndef LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD

include "llvm/Target/GlobalISel/Combine.td"

def MMIXCombinerHelper: GICombinerHelper<
  "MMIXGenCombinerHelper", [all_combines]> {}

#endif // LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
