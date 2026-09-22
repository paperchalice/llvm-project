//===-- MMIXLegalizerInfo.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for MMIX.
//===----------------------------------------------------------------------===//

#include "MMIXLegalizerInfo.h"

using namespace llvm;

MMIXLegalizerInfo::MMIXLegalizerInfo() {
  using namespace TargetOpcode;
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);
  const LLT v8s8 = LLT::fixed_vector(8, s8);
  const LLT v4s16 = LLT::fixed_vector(4, s16);
  const LLT v2s32 = LLT::fixed_vector(2, s32);
  const LLT f32 = LLT::float32();
  const LLT f64 = LLT::float64();
  const LLT p0 = LLT::pointer(0, 64);

  // try to keep order same as TargetOpcodes.def
  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_SDIV, G_UDIV})
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder({G_SREM, G_UREM})
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder(G_SDIVREM).lower();

  getActionDefinitionsBuilder({G_UDIVREM, G_AND, G_OR, G_XOR})
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder(
      {G_ABDS, G_ABDU, G_UAVGFLOOR, G_UAVGCEIL, G_SAVGFLOOR, G_SAVGCEIL})
      .lower();

  getActionDefinitionsBuilder({G_IMPLICIT_DEF, G_PHI})
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder({G_FRAME_INDEX, G_GLOBAL_VALUE}).legalFor({p0});

  getActionDefinitionsBuilder(G_PTRAUTH_GLOBAL_VALUE).unsupported();

  getActionDefinitionsBuilder(G_CONSTANT_POOL).legalFor({p0});

  getActionDefinitionsBuilder(
      {G_EXTRACT, G_UNMERGE_VALUES, G_INSERT, G_MERGE_VALUES})
      .legalForCartesianProduct({s64}, {s64});

  getActionDefinitionsBuilder(
      {G_BUILD_VECTOR, G_BUILD_VECTOR_TRUNC, G_CONCAT_VECTORS})
      .lower();

  getActionDefinitionsBuilder(G_INTTOPTR).legalForCartesianProduct({p0}, {s64});

  getActionDefinitionsBuilder(G_PTRTOINT).legalForCartesianProduct({s64}, {p0});

  getActionDefinitionsBuilder(G_BITCAST)
      .legalFor({{s64, p0}, {p0, s64}})
      .legalForCartesianProduct({s64, v8s8, v4s16, v2s32},
                                {s64, v8s8, v4s16, v2s32});

  getActionDefinitionsBuilder({G_FREEZE, G_CONSTANT_FOLD_BARRIER})
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder({G_INTRINSIC_FPTRUNC_ROUND, G_INTRINSIC_TRUNC,
                               G_INTRINSIC_ROUND, G_INTRINSIC_LRINT,
                               G_INTRINSIC_LLRINT, G_INTRINSIC_ROUNDEVEN,
                               G_READCYCLECOUNTER, G_READSTEADYCOUNTER})
      .unsupported();

  getActionDefinitionsBuilder({G_LOAD, G_SEXTLOAD, G_ZEXTLOAD})
      .legalForCartesianProduct({s64}, {p0});

  getActionDefinitionsBuilder(G_FPEXTLOAD)
      .legalForCartesianProduct({f64}, {p0});

  getActionDefinitionsBuilder(
      {G_INDEXED_LOAD, G_INDEXED_SEXTLOAD, G_INDEXED_ZEXTLOAD})
      .lower();

  getActionDefinitionsBuilder({G_STORE, G_FPTRUNCSTORE})
      .legalForCartesianProduct({s64}, {p0});

  getActionDefinitionsBuilder(G_INDEXED_STORE).lower();

  // TODO: some of thems are legal
  getActionDefinitionsBuilder({G_ATOMIC_CMPXCHG_WITH_SUCCESS,
                               G_ATOMIC_CMPXCHG,
                               G_ATOMICRMW_XCHG,
                               G_ATOMICRMW_ADD,
                               G_ATOMICRMW_SUB,
                               G_ATOMICRMW_AND,
                               G_ATOMICRMW_NAND,
                               G_ATOMICRMW_OR,
                               G_ATOMICRMW_XOR,
                               G_ATOMICRMW_MAX,
                               G_ATOMICRMW_MIN,
                               G_ATOMICRMW_UMAX,
                               G_ATOMICRMW_UMIN,
                               G_ATOMICRMW_FADD,
                               G_ATOMICRMW_FSUB,
                               G_ATOMICRMW_FMAX,
                               G_ATOMICRMW_FMIN,
                               G_ATOMICRMW_FMAXIMUM,
                               G_ATOMICRMW_FMINIMUM,
                               G_ATOMICRMW_FMAXIMUMNUM,
                               G_ATOMICRMW_FMINIMUMNUM,
                               G_ATOMICRMW_UINC_WRAP,
                               G_ATOMICRMW_UDEC_WRAP,
                               G_ATOMICRMW_USUB_COND,
                               G_ATOMICRMW_USUB_SAT,
                               G_FENCE})
      .unsupported();

  getActionDefinitionsBuilder(G_PREFETCH).legalFor({p0});

  getActionDefinitionsBuilder(G_BRCOND)
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder(G_BRINDIRECT).legalFor({s64, p0});

  // G_INVOKE_REGION_START
  // G_INTRINSIC
  // G_INTRINSIC_W_SIDE_EFFECTS
  // G_INTRINSIC_CONVERGENT
  // G_INTRINSIC_CONVERGENT_W_SIDE_EFFECTS

  getActionDefinitionsBuilder(G_ANYEXT)
      .legalForCartesianProduct({s64}, {s8, s16, s32})
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64)
      .clampScalar(0, s64, s64)
      .clampScalar(1, s8, s32);

  getActionDefinitionsBuilder(G_TRUNC)
      .legalForCartesianProduct({s1, s8, s16, s32, s64}, {s64})
      .clampScalar(0, s8, s64)
      .clampScalar(1, s64, s64)
      .alwaysLegal()
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64);

  getActionDefinitionsBuilder({G_TRUNC_SSAT_S, G_TRUNC_SSAT_U, G_TRUNC_USAT_U})
      .lower();

  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({s64})
      .clampScalar(0, s64, s64)
      .widenScalarToNextMultipleOf(0, 64);

  getActionDefinitionsBuilder(G_FCONSTANT).legalFor({f64});

  getActionDefinitionsBuilder({G_VASTART, G_VAARG})
      .legalForCartesianProduct({s64}, {p0});

  getActionDefinitionsBuilder(G_SEXT)
      .legalForCartesianProduct({s64}, {s8, s16, s32})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s8, s32)
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64);

  getActionDefinitionsBuilder(G_SEXT_INREG).lower();

  getActionDefinitionsBuilder(G_ZEXT)
      .legalForCartesianProduct({s64}, {s8, s16, s32})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s8, s32)
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64);

  getActionDefinitionsBuilder({G_SHL, G_LSHR, G_ASHR})
      .legalForCartesianProduct({s64}, {s64})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64)
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64);

  getActionDefinitionsBuilder({G_FSHL, G_FSHR, G_ROTR, G_ROTL}).lower();

  getActionDefinitionsBuilder({G_ICMP, G_SCMP, G_UCMP, G_SELECT})
      .legalForCartesianProduct({s64, p0}, {s64, p0})
      .widenScalarToNextPow2(0, 64)
      .widenScalarToNextPow2(1, 64)
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64);
  getActionDefinitionsBuilder(G_FCMP)
      .legalForCartesianProduct({s64}, {f64})
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64)
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64);

  getActionDefinitionsBuilder({G_UADDO, G_UADDE, G_USUBO, G_USUBE, G_SADDO,
                               G_SADDE, G_SSUBO, G_SSUBE, G_UMULO, G_SMULO})
      .clampScalar(0, s64, s64)
      .lower();

  getActionDefinitionsBuilder(G_UMULH)
      .legalFor({s64})
      .widenScalarToNextMultipleOf(0, 64)
      .clampScalar(0, s64, s64);

  getActionDefinitionsBuilder({G_SMULH, G_UADDSAT, G_SADDSAT})
      .clampScalar(0, s64, s64)
      .lower();

  getActionDefinitionsBuilder(G_USUBSAT).legalFor({s64, v8s8, v4s16, v2s32});

  getActionDefinitionsBuilder({G_SSUBSAT, G_USHLSAT, G_SSHLSAT, G_SMULFIX,
                               G_UMULFIX, G_SMULFIXSAT, G_UMULFIXSAT, G_SDIVFIX,
                               G_UDIVFIX, G_SDIVFIXSAT, G_UDIVFIXSAT})
      .lower();

  getActionDefinitionsBuilder({G_FADD, G_FSUB, G_FMUL}).legalFor({f64});

  getActionDefinitionsBuilder({G_FMA, G_FMAD}).lower();

  getActionDefinitionsBuilder({G_FDIV, G_FREM}).legalFor({f64});

  getActionDefinitionsBuilder({G_FMODF, G_FPOW, G_FPOWI, G_FEXP, G_FEXP2,
                               G_FEXP10, G_FLOG, G_FLOG2, G_FLOG10, G_FLDEXP,
                               G_FFREXP, G_FNEG})
      .lower();

  getActionDefinitionsBuilder(G_FPEXT).legalForCartesianProduct({f64}, {f32});
  getActionDefinitionsBuilder(G_FPTRUNC).legalForCartesianProduct({f32}, {f64});

  getActionDefinitionsBuilder(
      {G_FPTOSI, G_FPTOUI, G_SITOFP, G_UITOFP, G_FPTOSI_SAT, G_FPTOUI_SAT,
       G_FABS, G_FCOPYSIGN, G_IS_FPCLASS, G_FCANONICALIZE, G_FMINNUM, G_FMAXNUM,
       G_FMINIMUM, G_FMAXIMUM, G_FMINIMUMNUM, G_FMAXIMUMNUM})
      .lower();

  getActionDefinitionsBuilder({G_GET_FPENV, G_SET_FPENV, G_RESET_FPENV,
                               G_GET_FPMODE, G_SET_FPMODE, G_RESET_FPMODE,
                               G_GET_ROUNDING, G_SET_ROUNDING})
      .libcall();

  getActionDefinitionsBuilder({G_PTR_ADD, G_PTRMASK})
      .legalForCartesianProduct({p0}, {s64});

  getActionDefinitionsBuilder({G_SMIN, G_SMAX, G_UMIN, G_UMAX, G_ABS}).lower();

  getActionDefinitionsBuilder({G_LROUND, G_LLROUND}).lower();

  getActionDefinitionsBuilder(G_BR).alwaysLegal();

  getActionDefinitionsBuilder(G_BRJT).lower();

  getActionDefinitionsBuilder(
      {G_VSCALE, G_INSERT_SUBVECTOR, G_EXTRACT_SUBVECTOR, G_INSERT_VECTOR_ELT,
       G_EXTRACT_VECTOR_ELT, G_SHUFFLE_VECTOR, G_SPLAT_VECTOR, G_STEP_VECTOR,
       G_VECTOR_COMPRESS})
      .lower();

  getActionDefinitionsBuilder(
      {G_CTTZ, G_CTTZ_ZERO_POISON, G_CTLZ, G_CTLZ_ZERO_POISON, G_CTLS})
      .lower();

  getActionDefinitionsBuilder(G_CTPOP)
      .legalForCartesianProduct({s64}, {s64})
      .widenScalarToNextMultipleOf(0, 64)
      .widenScalarToNextMultipleOf(1, 64)
      .clampScalar(0, s64, s64)
      .clampScalar(1, s64, s64);

  getActionDefinitionsBuilder({G_BSWAP, G_BITREVERSE, G_CLMUL}).lower();

  getActionDefinitionsBuilder({G_FCEIL, G_FCOS, G_FSIN, G_FSINCOS, G_FTAN,
                               G_FACOS, G_FASIN, G_FATAN, G_FATAN2, G_FCOSH,
                               G_FSINH, G_FTANH})
      .libcall();

  getActionDefinitionsBuilder(G_FSQRT).legalFor({f64});

  getActionDefinitionsBuilder({G_FFLOOR, G_FRINT, G_FNEARBYINT}).libcall();

  getActionDefinitionsBuilder({G_ADDRSPACE_CAST, G_BLOCK_ADDR, G_JUMP_TABLE,
                               G_DYN_STACKALLOC, G_STACKSAVE, G_STACKRESTORE})
      .lower();

  getActionDefinitionsBuilder({G_STRICT_FADD, G_STRICT_FSUB, G_STRICT_FMUL,
                               G_STRICT_FDIV, G_STRICT_FREM, G_STRICT_FMA,
                               G_STRICT_FSQRT, G_STRICT_FLDEXP, G_STRICT_FCMP,
                               G_STRICT_FCMPS})
      .lower();

  getActionDefinitionsBuilder(
      {G_READ_REGISTER, G_WRITE_REGISTER, G_MEMCPY, G_MEMCPY_INLINE, G_MEMMOVE,
       G_MEMSET, G_BZERO, G_MEMSET_INLINE, G_TRAP, G_DEBUGTRAP, G_UBSANTRAP})
      .lower();

  getActionDefinitionsBuilder(
      {G_VECREDUCE_SEQ_FADD, G_VECREDUCE_SEQ_FMUL, G_VECREDUCE_FADD,
       G_VECREDUCE_FMUL, G_VECREDUCE_FMAX, G_VECREDUCE_FMIN,
       G_VECREDUCE_FMAXIMUM, G_VECREDUCE_FMINIMUM, G_VECREDUCE_ADD,
       G_VECREDUCE_MUL, G_VECREDUCE_AND, G_VECREDUCE_OR, G_VECREDUCE_XOR,
       G_VECREDUCE_SMAX, G_VECREDUCE_SMIN, G_VECREDUCE_UMAX, G_VECREDUCE_UMIN})
      .lower();

  getActionDefinitionsBuilder({G_SBFX, G_UBFX}).lower();
}
