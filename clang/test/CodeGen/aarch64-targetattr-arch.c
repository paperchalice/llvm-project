// RUN: %clang_cc1 -triple aarch64 -target-feature +v8a -verify -DHAS8 -S %s -o -
// RUN: %clang_cc1 -triple aarch64 -target-feature +v8.1a -verify -DHAS81 -S %s -o -
// RUN: %clang_cc1 -triple aarch64 -target-feature +v9a -verify -DHAS9 -S %s -o -
// REQUIRES: aarch64-registered-target

#ifdef HAS9
// expected-no-diagnostics
#endif

#include <arm_acle.h>
#include <arm_fp16.h>
#include <arm_sve.h>

__attribute__((target("arch=armv8.1-a")))
int test_crc_attr()
{
  return __crc32cd(1, 1);
}

__attribute__((target("arch=armv9-a")))
svint8_t test_svadd_attr(svbool_t pg, svint8_t op1, svint8_t op2)
{
  return svadd_s8_z(pg, op1, op2);
}

__attribute__((target("arch=armv9-a")))
float16_t test_fp16_on_v9(float16_t x, float16_t y)
{
  return vabdh_f16(x, y);
}

void test_error1()
{
#ifdef HAS8
// expected-error@+2{{always_inline function '__crc32cd' requires target feature 'crc'}}
#endif
  __crc32cd(1, 1);
}

void test_error2()
{
#if defined(HAS8) || defined(HAS81)
// expected-error@+2{{SVE vector type 'svint8_t' (aka '__SVInt8_t') cannot be used in a target without sve}}
#endif
  svundef_s8();
}
