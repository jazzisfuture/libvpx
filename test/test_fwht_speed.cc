/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
//  Test and time VPX fwht4x4 functions

#include <stdio.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vp9_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/md5_helper.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------

namespace {

typedef void (*VpxFwhtFunc)(const int16_t *input, tran_low_t *output,
                            int stride);

void TestFwht4x4(VpxFwhtFunc const *funcs) {
  static const int kNumFuncs = 1;
  static const char *const kSignatures[kNumFuncs] = {
    "d1b8952b956e5c994bac5c4d59bf6700",
  };
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int stride = 1080;
  const int kTotalPixels = 4 * stride;
  DECLARE_ALIGNED(16, int16_t, input[kTotalPixels]);
  DECLARE_ALIGNED(16, tran_low_t, output[16]);
  for (int i = 0; i < kTotalPixels; ++i) {
    input[i] = rnd.Rand8();
  }
  const int kNumTests = 100000000;

  for (int k = 0; k < kNumFuncs; ++k) {
    if (funcs[k] == NULL) continue;
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int num_tests = 0; num_tests < kNumTests; ++num_tests) {
      funcs[k](input, output, stride);
    }
    libvpx_test::ClearSystemState();
    vpx_usec_timer_mark(&timer);
    const int elapsed_time =
        static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
    libvpx_test::MD5 md5;
    md5.Add((const uint8_t*)output, sizeof(output));
    printf("             %4d ms     MD5: %s\n", elapsed_time, md5.Get());
    EXPECT_STREQ(kSignatures[k], md5.Get());
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...).
// The test name is 'arch.TestFwht4x4', e.g., C.TestFwht4x4.
#define FWHT4X4_PRED_TEST(arch, func)          \
  TEST(arch, TestFwht4x4) {                    \
    static const VpxFwhtFunc funcs[] = {func}; \
    TestFwht4x4(funcs);                        \
  }

// -----------------------------------------------------------------------------

FWHT4X4_PRED_TEST(C, vp9_fwht4x4_c)

#if HAVE_MMX && CONFIG_USE_X86INC
FWHT4X4_PRED_TEST(MMX, vp9_fwht4x4_mmx)
#endif  // HAVE_MMX && CONFIG_USE_X86INC

#if HAVE_SSE2 && CONFIG_USE_X86INC
FWHT4X4_PRED_TEST(SSE2, vp9_fwht4x4_sse2)
#endif  // HAVE_SSE2 && CONFIG_USE_X86INC

// -----------------------------------------------------------------------------

#include "test/test_libvpx.cc"
