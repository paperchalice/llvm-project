#ifndef LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H
#define LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H

#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

#define MMIXCOMBINERHELPER_GENCOMBINERHELPER_DEPS
#include "MMIXGenGICombiner.inc"
#undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_DEPS

namespace llvm {

// #define MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
// #include "MMIXGenGICombiner.inc"
// #undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
FunctionPass *createMMIXCombiner();
}

#endif // LLVM_LIB_TARGET_MMIX_GLOBALISEL_MMIXCOMBINER_H
