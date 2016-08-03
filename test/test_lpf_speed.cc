/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
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
#include "vp9/common/vp9_loopfilter.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

namespace {

const int kNumSourceTypes = 3;
const int kNumCoeffs = (24 * 24);

typedef void (*VpxLoopfilterFunc)(uint8_t *s, int p, const uint8_t *blimit,
                                  const uint8_t *limit, const uint8_t *thresh);

typedef struct {
  VpxLoopfilterFunc func;
  const char *name;
} VpxLoopfilterFuncInfo;

void TestLoopfilter(VpxLoopfilterFuncInfo const *funcInfos) {
  static const int kNumFuncs = 8;
  static const char *const kSourceTypes[kNumSourceTypes] = {
      "filter4", "filter8", "filter16"};
  static const char *const kSignatures[kNumSourceTypes][kNumFuncs] = {
      {"aa9082af0e7460cc6a79835b218a0d9c", "aa9082af0e7460cc6a79835b218a0d9c",
       "aa9082af0e7460cc6a79835b218a0d9c", "aa9082af0e7460cc6a79835b218a0d9c"},
      {"6101bad1e9b90c13b27545718a67b508", "9dea2992ad66ecb9cb3e4d51fc55dc81",
       "c766e37e2634b583036aed59faa93332", "8b584df8050f693ee477b7e30fbe9b77"},
      {"36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
       "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93"},
  };
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int count_test_block = 20000000;
#if CONFIG_VP9_HIGHBITDEPTH
  int32_t bd = bit_depth_;
  DECLARE_ALIGNED(16, uint16_t, s[kNumCoeffs]);
  DECLARE_ALIGNED(16, uint16_t, ss[kNumSourceTypes][kNumCoeffs]);
#else
  DECLARE_ALIGNED(8, uint8_t, s[kNumCoeffs]);
  DECLARE_ALIGNED(8, uint8_t, ss[kNumSourceTypes][kNumCoeffs]);
#endif  // CONFIG_VP9_HIGHBITDEPTH
  uint8_t tmp = static_cast<uint8_t>(rnd(3 * MAX_LOOP_FILTER + 4));
  DECLARE_ALIGNED(16, const uint8_t, blimit[16]) = {
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp,
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp};
  tmp = static_cast<uint8_t>(rnd(MAX_LOOP_FILTER));
  DECLARE_ALIGNED(16, const uint8_t, limit[16]) = {tmp, tmp, tmp, tmp, tmp, tmp,
                                                   tmp, tmp, tmp, tmp, tmp, tmp,
                                                   tmp, tmp, tmp, tmp};
  tmp = rnd.Rand8();
  DECLARE_ALIGNED(16, const uint8_t, thresh[16]) = {
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp,
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp};
  int32_t p = kNumCoeffs / 24;
  int elapsed_time_org = 0;

  for (int i = 0; i < kNumCoeffs / p; i++) {
    for (int j = 0; j < p; j++) {
      ss[0][i * p + j] = (rnd.Rand8() & 0x40) + (i >> 1) + (j >> 1);
    }
  }
  for (int i = 0; i < kNumCoeffs / p; i++) {
    for (int j = 0; j < p; j++) {
      ss[1][i * p + j] = (i >> 1) + (j >> 1);
    }
  }
  for (int i = 0; i < kNumCoeffs; ++i) {
    ss[2][i] = 0;
  }

  for (int sourceType = 0; sourceType < 3; ++sourceType) {
    printf("sourceType: %s\n", kSourceTypes[sourceType]);
    for (int k = 0; k < kNumFuncs; ++k) {
      vpx_usec_timer timer;
      vpx_usec_timer_start(&timer);
      for (int num_tests = 0; num_tests < count_test_block; ++num_tests) {
        memcpy(s, ss[sourceType], sizeof(s));
        funcInfos[k].func(s + 8 + p * 8, p, blimit, limit, thresh);
      }
      libvpx_test::ClearSystemState();
      vpx_usec_timer_mark(&timer);
      const int elapsed_time =
          static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
      libvpx_test::MD5 md5;
      md5.Add((const uint8_t *)s, sizeof(s));
      printf("             %-37s | time: %4d ms | MD5: %s", funcInfos[k].name,
             elapsed_time, md5.Get());
      EXPECT_STREQ(kSignatures[sourceType][k >> 1], md5.Get());
      if (!(k & 1)) {
        elapsed_time_org = elapsed_time;
        printf("\n");
      } else {
        printf(" | gain: ");
        printf((elapsed_time_org > elapsed_time) ? ANSI_COLOR_GREEN
                                                 : ANSI_COLOR_RED);
        printf("%5.1f%%\n",
               100.0 * (elapsed_time_org - elapsed_time) / elapsed_time_org);
        printf(ANSI_COLOR_RESET);
      }
    }
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...).
// The test name is 'arch.TestLoopfilter', e.g., C.TestLoopfilter.
#define LOOPFILTER_TEST(arch, asmFunc1, asmName1, neonFunc1, neonName1,     \
                        asmFunc2, asmName2, neonFunc2, neonName2, asmFunc3, \
                        asmName3, neonFunc3, neonName3, asmFunc4, asmName4, \
                        neonFunc4, neonName4)                               \
  TEST(arch, TestLoopfilter) {                                              \
    static const VpxLoopfilterFuncInfo funcInfos[] =                        \
        {                                                                   \
            {asmFunc1, asmName1}, {neonFunc1, neonName1},                   \
            {asmFunc2, asmName2}, {neonFunc2, neonName2},                   \
            {asmFunc3, asmName3}, {neonFunc3, neonName3},                   \
            {asmFunc4, asmName4}, {neonFunc4, neonName4},                   \
        };                                                                  \
    TestLoopfilter(funcInfos);                                              \
  }

// -----------------------------------------------------------------------------

#if (HAVE_NEON_ASM && HAVE_NEON)
LOOPFILTER_TEST(NEON, vpx_lpf_horizontal_edge_8_neon_asm,
                "vpx_lpf_horizontal_edge_8_neon_asm",
                vpx_lpf_horizontal_edge_8_neon,
                "vpx_lpf_horizontal_edge_8_neon",
                vpx_lpf_horizontal_edge_16_neon_asm,
                "vpx_lpf_horizontal_edge_16_neon_asm",
                vpx_lpf_horizontal_edge_16_neon,
                "vpx_lpf_horizontal_edge_16_neon", vpx_lpf_vertical_16_neon_asm,
                "vpx_lpf_vertical_16_neon_asm", vpx_lpf_vertical_16_neon,
                "vpx_lpf_vertical_16_neon", vpx_lpf_vertical_16_dual_neon_asm,
                "vpx_lpf_vertical_16_dual_neon_asm",
                vpx_lpf_vertical_16_dual_neon, "vpx_lpf_vertical_16_dual_neon")
#else
LOOPFILTER_TEST(C, vpx_lpf_horizontal_edge_8_c, "vpx_lpf_horizontal_edge_8_c",
                vpx_lpf_horizontal_edge_8_c, "vpx_lpf_horizontal_edge_8_c",
                vpx_lpf_horizontal_edge_16_c, "vpx_lpf_horizontal_edge_16_c",
                vpx_lpf_horizontal_edge_16_c, "vpx_lpf_horizontal_edge_16_c",
                vpx_lpf_vertical_16_c, "vpx_lpf_vertical_16_c",
                vpx_lpf_vertical_16_c, "vpx_lpf_vertical_16_c",
                vpx_lpf_vertical_16_dual_c, "vpx_lpf_vertical_16_dual_c",
                vpx_lpf_vertical_16_dual_c, "vpx_lpf_vertical_16_dual_c")
#endif  // (HAVE_NEON_ASM && HAVE_NEON)

// -----------------------------------------------------------------------------

#include "test/test_libvpx.cc"
