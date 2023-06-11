#include "MMIXCombiner.h"
#include "MMIX.h"
#include "llvm/CodeGen/GlobalISel/CombinerHelper.h"
#include "llvm/CodeGen/GlobalISel/CombinerInfo.h"
#include "llvm/CodeGen/GlobalISel/GISelChangeObserver.h"
#include "llvm/CodeGen/GlobalISel/GISelKnownBits.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegionInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

#define DEBUG_TYPE "mmix-combiner"

using namespace llvm;

#define MMIXCOMBINERHELPER_GENCOMBINERHELPER_H
#include "MMIXGenGICombiner.inc"
#undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_H

#define MMIXCOMBINERHELPER_GENCOMBINERHELPER_CPP
#include "MMIXGenGICombiner.inc"
#undef MMIXCOMBINERHELPER_GENCOMBINERHELPER_CPP
