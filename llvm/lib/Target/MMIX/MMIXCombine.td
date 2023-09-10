#ifndef LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD

include "llvm/Target/GlobalISel/Combine.td"

def MMIXCombiner : GICombiner<"MMIXCombinerImpl", [all_combines]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
