#ifndef LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H
#define LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H

#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define GET_GICOMBINER_DEPS
#include "MMIXGenPreLegalizerCombiner.inc"
#undef GET_GICOMBINER_DEPS

namespace llvm {

// #define MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
// #include "MMIXGenGICombiner.inc"
// #undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
FunctionPass *createMMIXPreLegalizerCombiner();
FunctionPass *createMMIXO0PreLegalizerCombiner();
void initializeMMIXPreLegalizerCombinerPass(PassRegistry &);
} // namespace llvm

#endif // LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H
