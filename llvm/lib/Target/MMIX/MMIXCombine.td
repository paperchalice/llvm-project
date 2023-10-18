#ifndef LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
#define LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD

def MMIXPreLegalizerCombiner : GICombiner<"MMIXPreLegalizerCombinerImpl",
  [all_combines]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}

def MMIXO0PreLegalizerCombiner : GICombiner<"MMIXO0PreLegalizerCombinerImpl",
  [optnone_combines]> {
  let CombineAllMethodName = "tryCombineAllImpl";
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXCOMBINE_TD
