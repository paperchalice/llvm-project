#ifndef LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD

def MMIXCombiner : GICombiner<"MMIXCombinerImpl", [all_combines]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
