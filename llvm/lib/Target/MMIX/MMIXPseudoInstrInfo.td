#ifndef LLVM_LIB_TARGET_MMIX_MMIXPSEUDOINSTRINFO_TD
#define LLVM_LIB_TARGET_MMIX_MMIXPSEUDOINSTRINFO_TD

class MMIXPseudo : Instruction {
  let Namespace = "MMIX";
  dag OutOperandList = (outs);
  dag InOperandList = (ins);

  let hasSideEffects = false;
  let mayLoad = false;
  let mayStore = false;
  let isPseudo = true;
  let isCodeGenOnly = true;
}

class MMIXCallFramePseudo : MMIXPseudo {
  dag InOperandList = (ins i64imm:$bytes, i64imm:$prior);

  let Defs = [r254];
  let Uses = [r254];
}

// ADJCALLSTACKDOWN/UP implicitly use/def SP because they may be expanded into
// a stack adjustment and the codegen must know that they may modify the stack
// pointer before prolog-epilog rewriting occurs.
// Pessimistically assume ADJCALLSTACKDOWN / ADJCALLSTACKUP will become
// sub / add which can clobber SR.

// Call frame setup pseudo.
def ADJCALLSTACKDOWN : MMIXCallFramePseudo;

// Call frame destroy pseudo.
def ADJCALLSTACKUP : MMIXCallFramePseudo;

def CALL : MMIXPseudo {
  let OutOperandList = (outs unknown:$ret);
  let InOperandList = (ins unknown:$reg_tuple, ui16imm:$YZ);
  let isCall = true;
  let Constraints = "$ret = $reg_tuple";
}

#endif // LLVM_LIB_TARGET_MMIX_MMIXPSEUDOINSTRINFO_TD
