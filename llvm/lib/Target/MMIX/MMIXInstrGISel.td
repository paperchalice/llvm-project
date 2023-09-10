#ifndef LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD
#define LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD

class MMIXGenericInstruction : GenericInstruction {
  let Namespace = "MMIX";
}

def G_FOO : MMIXGenericInstruction {
  let OutOperandList = (outs type0 : $a);
  let InOperandList = (ins type0 : $b);
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXINSTRGISEL_TD
