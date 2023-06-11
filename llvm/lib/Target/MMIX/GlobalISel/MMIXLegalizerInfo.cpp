#include "MMIXLegalizerInfo.h"
#include "llvm/CodeGen/GlobalISel/MIPatternMatch.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/GlobalISel/Utils.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/MathExtras.h"
#include <initializer_list>

#define DEBUG_TYPE "mmix-legalinfo"

using namespace llvm;

MMIXLegalizerInfo::MMIXLegalizerInfo(const MMIXSubtarget &ST) {
  using namespace TargetOpcode;
  constexpr auto p0 = LLT::pointer(0, 64);
  constexpr auto s8 = LLT::scalar(8);
  constexpr auto s16 = LLT::scalar(16);
  constexpr auto s32 = LLT::scalar(32);
  constexpr auto s64 = LLT::scalar(64);
  // constant
  getActionDefinitionsBuilder(G_CONSTANT).legalFor({p0, s8, s16, s32, s64});

  // adding and subtracting
  getActionDefinitionsBuilder(G_ADD)
      .legalFor({p0, s64})
      .clampScalar(0, s64, s64);
}
