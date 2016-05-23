/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
//  Test and time VPX lpf functions

#include <stdio.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_dsp_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/md5_helper.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------

namespace {

typedef void (*VpxLpf4Func)(uint8_t *s, int pitch,
                            const uint8_t *blimit, const uint8_t *limit,
                            const uint8_t *thresh);

void TestLpf4(VpxLpf4Func const *funcs) {
  static const int kNumFuncs = 2;
  static const char *const kSignatures[kNumFuncs] = {
    "03d4de8b830f620512f3e60b3c34929d",
    "2f847bd7d121a7b1da48cc9fce7e1e16"
  };
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int kNumTests = 10000000;
  const int pitch = 1080;
  const int kTotalPixels = 8 * pitch;
  const uint8_t bl_value = rnd.Rand8() % 128;
  const uint8_t l_value = rnd.Rand8() % 128;
  const uint8_t t_value = rnd.Rand8() % 128;
  const uint8_t blimit[16] = {
    bl_value, bl_value, bl_value, bl_value, bl_value, bl_value,
    bl_value, bl_value, bl_value, bl_value, bl_value, bl_value,
    bl_value, bl_value, bl_value, bl_value
  };
  const uint8_t limit[16] = {
    l_value, l_value, l_value, l_value, l_value, l_value,
    l_value, l_value, l_value, l_value, l_value, l_value,
    l_value, l_value, l_value, l_value
  };
  const uint8_t thresh[16] = {
    t_value, t_value, t_value, t_value, t_value, t_value,
    t_value, t_value, t_value, t_value, t_value, t_value,
    t_value, t_value, t_value, t_value
  };
  DECLARE_ALIGNED(16, uint8_t, src[kTotalPixels]);
  uint8_t *s = src + 4 * pitch;

  for (int k = 0; k < kNumFuncs; ++k) {
    for (int i = 0; i < kTotalPixels; ++i) {
      src[i] = rnd.Rand8();
    }
    if (funcs[k] == NULL) continue;
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int num_tests = 0; num_tests < kNumTests; ++num_tests) {
      funcs[k](s, pitch, blimit, limit, thresh);
    }
    libvpx_test::ClearSystemState();
    vpx_usec_timer_mark(&timer);
    const int elapsed_time =
        static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
    libvpx_test::MD5 md5;
    md5.Add((const uint8_t*)src, sizeof(src));
    printf("             %4d ms     MD5: %s\n", elapsed_time, md5.Get());
    EXPECT_STREQ(kSignatures[k], md5.Get());
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...).
// The test name is 'arch.TestLpf4', e.g., C.TestLpf4.
#define LPF4_PRED_TEST(arch, func1, func2)             \
  TEST(arch, TestLpf4) {                               \
    static const VpxLpf4Func funcs[] = {func1, func2}; \
    TestLpf4(funcs);                                   \
  }

// -----------------------------------------------------------------------------

LPF4_PRED_TEST(C, vpx_lpf_horizontal_4_c, vpx_lpf_vertical_4_c)

#if HAVE_MMX && CONFIG_USE_X86INC
LPF4_PRED_TEST(MMX, vpx_lpf_horizontal_4_mmx, vpx_lpf_vertical_4_mmx)
#endif  // HAVE_MMX && CONFIG_USE_X86INC

#if HAVE_SSE2 && CONFIG_USE_X86INC
LPF4_PRED_TEST(SSE2, vpx_lpf_horizontal_4_sse2, vpx_lpf_vertical_4_sse2)
#endif  // HAVE_SSE2 && CONFIG_USE_X86INC

// -----------------------------------------------------------------------------

#include "test/test_libvpx.cc"
