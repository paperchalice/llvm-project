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

  // scalar type
  constexpr auto p0 = LLT::pointer(0, 64);
  constexpr auto s8 = LLT::scalar(8);
  constexpr auto s16 = LLT::scalar(16);
  constexpr auto s32 = LLT::scalar(32);
  constexpr auto s64 = LLT::scalar(64);

  constexpr auto v8s8 = LLT::fixed_vector(8, 8);
  constexpr auto v4s16 = LLT::fixed_vector(4, 16);
  constexpr auto v2s32 = LLT::fixed_vector(2, 32);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_SDIV, G_UDIV, G_SREM,
                               G_UREM, G_SDIVREM, G_UDIVREM, G_AND, G_OR,
                               G_XOR})
      .legalFor({s64})
      .clampScalar(0, s64, s64);

  {
    auto &G_IMPLICIT_DEF_Builder =
        getActionDefinitionsBuilder(G_IMPLICIT_DEF).legalFor({s64});
    for (int i = 2; i != 64; ++i)
      G_IMPLICIT_DEF_Builder.legalFor({LLT::fixed_vector(i, s64)});
  }

  getActionDefinitionsBuilder(G_PHI).legalFor({s64});

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_GLOBAL_VALUE}).legalFor({p0});

  getActionDefinitionsBuilder(
      {G_CONSTANT_POOL, G_EXTRACT, G_UNMERGE_VALUES, G_INSERT});

  getActionDefinitionsBuilder(G_BUILD_VECTOR)
      .legalFor({{v8s8, s8}, {v4s16, s16}, {v2s32, s32}});

  getActionDefinitionsBuilder({G_BUILD_VECTOR_TRUNC, G_CONCAT_VECTORS});

  getActionDefinitionsBuilder(G_PTRTOINT)
      .legalForCartesianProduct({v8s8, v4s16, v2s32, s64}, {p0});

  getActionDefinitionsBuilder(G_INTTOPTR)
      .legalForCartesianProduct({p0}, {v8s8, v4s16, v2s32, s64});

  getActionDefinitionsBuilder(G_BITCAST).legalForCartesianProduct(
      {p0, s64, v8s8, v4s16, v2s32});

  getActionDefinitionsBuilder(G_FREEZE).legalFor({p0, s64});

  getActionDefinitionsBuilder({G_INTRINSIC_FPTRUNC_ROUND, G_INTRINSIC_TRUNC,
                               G_INTRINSIC_ROUND, G_INTRINSIC_LRINT,
                               G_INTRINSIC_ROUNDEVEN});

  getActionDefinitionsBuilder(G_READCYCLECOUNTER).legalFor({s64});

  getActionDefinitionsBuilder({G_LOAD, G_SEXTLOAD, G_ZEXTLOAD})
      .legalForCartesianProduct({s8, s16, s32, s64}, {p0});

  getActionDefinitionsBuilder(
      {G_INDEXED_LOAD, G_INDEXED_SEXTLOAD, G_INDEXED_ZEXTLOAD})
      .legalForTypesWithMemDesc({{s64, p0, s8, 8},
                                 {s64, p0, s16, 16},
                                 {s64, p0, s32, 32},
                                 {s64, p0, s64, 64}});

  getActionDefinitionsBuilder({G_STORE, G_INDEXED_STORE})
      .legalForTypesWithMemDesc({{s64, p0, s8, 1},
                                 {s64, p0, s16, 1},
                                 {s64, p0, s32, 1},
                                 {s64, p0, s64, 1}});

  getActionDefinitionsBuilder(
      {G_ATOMIC_CMPXCHG_WITH_SUCCESS, G_ATOMIC_CMPXCHG, G_ATOMICRMW_XCHG,
       G_ATOMICRMW_ADD, G_ATOMICRMW_SUB, G_ATOMICRMW_AND, G_ATOMICRMW_NAND,
       G_ATOMICRMW_OR, G_ATOMICRMW_XOR, G_ATOMICRMW_MAX, G_ATOMICRMW_MIN,
       G_ATOMICRMW_UMAX, G_ATOMICRMW_UMIN, G_ATOMICRMW_FADD, G_ATOMICRMW_FSUB,
       G_ATOMICRMW_FMAX, G_ATOMICRMW_FMIN, G_ATOMICRMW_UINC_WRAP,
       G_ATOMICRMW_UDEC_WRAP});

  getActionDefinitionsBuilder(G_FENCE);

  getActionDefinitionsBuilder(G_BRCOND).legalFor({s64}).clampScalar(0, s64,
                                                                    s64);

  getActionDefinitionsBuilder(G_BRINDIRECT).legalFor({p0});

  getActionDefinitionsBuilder(
      {G_INVOKE_REGION_START, G_INTRINSIC, G_INTRINSIC_W_SIDE_EFFECTS});

  getActionDefinitionsBuilder(G_ANYEXT).legalFor({s8, s16, s32});

  getActionDefinitionsBuilder(G_TRUNC).legalFor({s64});

  getActionDefinitionsBuilder(G_CONSTANT).legalFor({p0, s8, s16, s32, s64});

  getActionDefinitionsBuilder(G_FCONSTANT).legalFor({s64});

  getActionDefinitionsBuilder(G_VASTART).legalFor({p0});

  getActionDefinitionsBuilder(G_VAARG).customForCartesianProduct(
      {s8, s16, s32, s64, p0}, {p0});

  getActionDefinitionsBuilder({G_SEXT, G_SEXT_INREG, G_ZEXT});

  getActionDefinitionsBuilder({G_SHL, G_LSHR, G_ASHR})
      .legalForCartesianProduct({s64});

  getActionDefinitionsBuilder({G_FSHL, G_FSHR, G_ROTR, G_ROTL});

  getActionDefinitionsBuilder({G_ICMP, G_FCMP, G_SELECT})
      .legalForCartesianProduct({s64, s64, s64, s64})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64);

  getActionDefinitionsBuilder({G_UADDO, G_UADDE, G_USUBO, G_USUBE});

  getActionDefinitionsBuilder(
      {G_SADDO, G_SADDE, G_SSUBO, G_SSUBE, G_UMULH, G_SMULH});

  getActionDefinitionsBuilder(
      {G_UADDSAT, G_SADDSAT, G_USUBSAT, G_SSUBSAT, G_USHLSAT, G_SSHLSAT});

  getActionDefinitionsBuilder({G_SMULFIX, G_UMULFIX, G_SMULFIXSAT, G_UMULFIXSAT,
                               G_SDIVFIX, G_UDIVFIX, G_SDIVFIXSAT,
                               G_UDIVFIXSAT});

  getActionDefinitionsBuilder({G_FADD, G_FSUB, G_FMUL}).legalFor({s64});

  getActionDefinitionsBuilder({G_FMA, G_FMAD});

  getActionDefinitionsBuilder({G_FDIV, G_FREM}).legalFor({s64});

  getActionDefinitionsBuilder(
      {G_FPOW,    G_FPOWI,        G_FEXP,        G_FEXP2,         G_FLOG,
       G_FLOG2,   G_FLOG10,       G_FLDEXP,      G_FNEG,          G_FPEXT,
       G_FPTRUNC, G_FPTOSI,       G_FPTOUI,      G_SITOFP,        G_UITOFP,
       G_FABS,    G_FCOPYSIGN,    G_IS_FPCLASS,  G_FCANONICALIZE, G_FMINNUM,
       G_FMAXNUM, G_FMINNUM_IEEE, G_FMAXNUM_IEEE});

  getActionDefinitionsBuilder(G_PTR_ADD).legalForCartesianProduct({p0}, {s64});

  getActionDefinitionsBuilder(
      {G_PTRMASK, G_SMIN, G_SMAX, G_UMIN, G_UMAX, G_ABS, G_LROUND, G_LLROUND});

  getActionDefinitionsBuilder(G_BR);

  getActionDefinitionsBuilder(G_BRJT).legalIf([=](const LegalityQuery &Query) {
    return Query.Types[0] == p0 && Query.Types[1] == s64;
  });

  getActionDefinitionsBuilder(
      {G_INSERT_VECTOR_ELT, G_EXTRACT_VECTOR_ELT, G_SHUFFLE_VECTOR});

  getActionDefinitionsBuilder(
      {G_CTTZ, G_CTTZ_ZERO_UNDEF, G_CTLZ, G_CTLZ_ZERO_UNDEF});

  getActionDefinitionsBuilder(G_CTPOP).legalFor({s64});

  getActionDefinitionsBuilder({G_BSWAP, G_BITREVERSE, G_FCEIL, G_FCOS, G_FSIN});

  getActionDefinitionsBuilder(G_FSQRT).legalFor({s64});

  getActionDefinitionsBuilder(
      {G_FFLOOR, G_FRINT, G_FNEARBYINT, G_ADDRSPACE_CAST});

  getActionDefinitionsBuilder(G_BLOCK_ADDR).legalFor({p0});

  getActionDefinitionsBuilder(G_JUMP_TABLE).legalFor({{p0}, {s64}});

  getActionDefinitionsBuilder(G_DYN_STACKALLOC);

  getActionDefinitionsBuilder({
      G_STRICT_FADD,
      G_STRICT_FSUB,
      G_STRICT_FMUL,
      G_STRICT_FDIV,
      G_STRICT_FREM,
      G_STRICT_FMA,
      G_STRICT_FSQRT,
      G_STRICT_FLDEXP,
  });

  getActionDefinitionsBuilder({G_READ_REGISTER, G_WRITE_REGISTER});

  getActionDefinitionsBuilder({
      G_MEMCPY,
      G_MEMCPY_INLINE,
      G_MEMMOVE,
      G_MEMSET,
      G_BZERO,
  });

  getActionDefinitionsBuilder({G_SBFX, G_UBFX});

  getActionDefinitionsBuilder({
      G_VECREDUCE_SEQ_FADD,
      G_VECREDUCE_SEQ_FMUL,
      G_VECREDUCE_FADD,
      G_VECREDUCE_FMUL,
      G_VECREDUCE_FMAX,
      G_VECREDUCE_FMIN,
      G_VECREDUCE_ADD,
      G_VECREDUCE_MUL,
      G_VECREDUCE_AND,
      G_VECREDUCE_OR,
      G_VECREDUCE_XOR,
      G_VECREDUCE_SMAX,
      G_VECREDUCE_SMIN,
      G_VECREDUCE_UMAX,
      G_VECREDUCE_UMIN,
  });
}
