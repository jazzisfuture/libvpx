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
#include "vpx/vpx_integer.h"
#include "vpx_dsp/vpx_filter.h"
#include "vpx_dsp/x86/convolve.h"
#include "vpx_dsp/vpx_convolve.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/md5_helper.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"
#include "vp9/common/vp9_filter.h"

// -----------------------------------------------------------------------------

namespace {

typedef void (*VpxSubpixelFunc)(
    const uint8_t *src_ptr,
    ptrdiff_t src_pitch,
    uint8_t *output_ptr,
    ptrdiff_t out_pitch,
    uint32_t output_height,
    const int16_t *filter);

typedef struct {
  VpxSubpixelFunc func;
  const char *name;
} VpxSubpixelFuncInfo;

#define kMaxNumFuncs 24

void TestSubpixelFilterBlock1d(VpxSubpixelFuncInfo const *funcInfos,
                                const uint32_t *heights,
                                const int kNumFuncs,
                                const int kNumHeights,
                                bool speedTest) {
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int kNumTests = 100000000;
  const int src_stride = 1080;
  const int dst_stride = 512;
  const int kTotalPixels = 24 * src_stride;
  char signature[100];
  DECLARE_ALIGNED(16, int16_t, filters[8]);
  DECLARE_ALIGNED(16, uint8_t, src[kTotalPixels]);
  DECLARE_ALIGNED(16, uint8_t, dst[kTotalPixels]);
  DECLARE_ALIGNED(16, uint8_t, tmp[kTotalPixels]);
  uint8_t *s = src + 4 * src_stride;
  filters[0] = -1;
  filters[1] = 5;
  filters[2] = -18;
  filters[3] = 105;
  filters[4] = 48;
  filters[5] = -14;
  filters[6] = 4;
  filters[7] = -1;

  if (speedTest) {
    for (int i = 0; i < kTotalPixels; ++i) {
      src[i] = rnd.Rand8();
      tmp[i] = rnd.Rand8();
    }

    for (int i = 0; i < kNumHeights; ++i) {
      const uint32_t height = heights[i];
      int elapsed_time_org = 0;
      for (int k = 0; k < kNumFuncs; ++k) {
        memcpy(dst, tmp, kTotalPixels);
        if (funcInfos[k].func == NULL) continue;
        vpx_usec_timer timer;
        vpx_usec_timer_start(&timer);
        for (int num_tests = 0; num_tests < kNumTests; ++num_tests) {
          funcInfos[k].func(s, src_stride, dst, dst_stride, height, filters);
        }
        libvpx_test::ClearSystemState();
        vpx_usec_timer_mark(&timer);
        const int elapsed_time =
            static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
        libvpx_test::MD5 md5;
        md5.Add((const uint8_t*)dst, sizeof(dst));
        printf("             %-27s | height: %d | time: %4d ms | MD5: %s",
            funcInfos[k].name, height, elapsed_time, md5.Get());
        if (!(k & 1)) {
          elapsed_time_org = elapsed_time;
          snprintf(signature, 100, "%s", md5.Get());
          printf("\n");
        } else {
          EXPECT_STREQ(signature, md5.Get());
          printf(" | gain: %5.1f%%\n",
                 100.0 * (elapsed_time_org - elapsed_time) / elapsed_time_org);
        }
      }
    }
  } else {  // unit test
    for (int j = 0; j < 1000; ++j) {
      for (int i = 0; i < kTotalPixels; ++i) {
        src[i] = rnd.Rand8();
        tmp[i] = rnd.Rand8();
      }

      for (int i = 0; i < kNumHeights; ++i) {
        const uint32_t height = heights[i];
        for (int k = 0; k < kNumFuncs; ++k) {
          memcpy(dst, tmp, kTotalPixels);
          if (funcInfos[k].func == NULL) continue;
          funcInfos[k].func(s, src_stride, dst, dst_stride, height, filters);
          libvpx_test::MD5 md5;
          md5.Add((const uint8_t*)dst, sizeof(dst));
          if (!(k & 1)) {
            snprintf(signature, 100, "%s", md5.Get());
          } else {
            EXPECT_STREQ(signature, md5.Get());
            if (strcmp(signature, md5.Get())) {
              printf("\n%s\n", funcInfos[k].name);
              exit(0);
            }
          }
        }
      }
    }
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...).
// The test name is 'arch.TestSubpixelFilterBlock1d',
// e.g., C.TestSubpixelFilterBlock1d.
#define SUBPIXEL_FILTER_TEST(arch, kNumFuncs,                                  \
    func_d4h8_org,  func_d4h8_opt,  func_d4h8_avg_org,  func_d4h8_avg_opt,     \
    func_d8h8_org,  func_d8h8_opt,  func_d8h8_avg_org,  func_d8h8_avg_opt,     \
    func_d16h8_org, func_d16h8_opt, func_d16h8_avg_org, func_d16h8_avg_opt,    \
    func_d4v8_org,  func_d4v8_opt,  func_d4v8_avg_org,  func_d4v8_avg_opt,     \
    func_d8v8_org,  func_d8v8_opt,  func_d8v8_avg_org,  func_d8v8_avg_opt,     \
    func_d16v8_org, func_d16v8_opt, func_d16v8_avg_org, func_d16v8_avg_opt)    \
  TEST(arch, TestSubpixelFilterBlock1d) {                                      \
    static const VpxSubpixelFuncInfo funcInfos[kMaxNumFuncs] = {               \
        {func_d4h8_org,      "block1d4_h8      (org)"},                        \
        {func_d4h8_opt,      "block1d4_h8"},                                   \
        {func_d4h8_avg_org,  "block1d4_h8_avg  (org)"},                        \
        {func_d4h8_avg_opt,  "block1d4_h8_avg"},                               \
        {func_d8h8_org,      "block1d8_h8      (org)"},                        \
        {func_d8h8_opt,      "block1d8_h8"},                                   \
        {func_d8h8_avg_org,  "block1d8_h8_avg  (org)"},                        \
        {func_d8h8_avg_opt,  "block1d8_h8_avg"},                               \
        {func_d16h8_org,     "block1d16_h8     (org)"},                        \
        {func_d16h8_opt,     "block1d16_h8"},                                  \
        {func_d16h8_avg_org, "block1d16_h8_avg (org)"},                        \
        {func_d16h8_avg_opt, "block1d16_h8_avg"},                              \
        {func_d4v8_org,      "block1d4_h8      (org)"},                        \
        {func_d4v8_opt,      "block1d4_h8"},                                   \
        {func_d4v8_avg_org,  "block1d4_h8_avg  (org)"},                        \
        {func_d4v8_avg_opt,  "block1d4_h8_avg"},                               \
        {func_d8v8_org,      "block1d8_h8      (org)"},                        \
        {func_d8v8_opt,      "block1d8_h8"},                                   \
        {func_d8v8_avg_org,  "block1d8_h8_avg  (org)"},                        \
        {func_d8v8_avg_opt,  "block1d8_h8_avg"},                               \
        {func_d16v8_org,     "block1d16_h8     (org)"},                        \
        {func_d16v8_opt,     "block1d16_h8"},                                  \
        {func_d16v8_avg_org, "block1d16_h8_avg (org)"},                        \
        {func_d16v8_avg_opt, "block1d16_h8_avg"},                              \
    };                                                                         \
    static const uint32_t heights[] = {8, 9};                                  \
    /* unit test */                                                            \
    TestSubpixelFilterBlock1d(funcInfos, heights, kNumFuncs, 2, false);        \
    /* speed test */                                                           \
    TestSubpixelFilterBlock1d(funcInfos, heights, kNumFuncs, 2, true);         \
  }

// -----------------------------------------------------------------------------

// The C test just runs one set of C functions to avoid the build error:
//   "TestSubpixelFilterBlock1d defined but not used".
SUBPIXEL_FILTER_TEST(C, 1,
  vpx_filter_block1d4_h8_c,      vpx_filter_block1d4_h8_c,
  vpx_filter_block1d4_h8_avg_c,  vpx_filter_block1d4_h8_avg_c,
  vpx_filter_block1d8_h8_c,      vpx_filter_block1d8_h8_c,
  vpx_filter_block1d8_h8_avg_c,  vpx_filter_block1d8_h8_avg_c,
  vpx_filter_block1d16_h8_c,     vpx_filter_block1d16_h8_c,
  vpx_filter_block1d16_h8_avg_c, vpx_filter_block1d16_h8_avg_c,
  vpx_filter_block1d4_v8_c,      vpx_filter_block1d4_v8_c,
  vpx_filter_block1d4_v8_avg_c,  vpx_filter_block1d4_v8_avg_c,
  vpx_filter_block1d8_v8_c,      vpx_filter_block1d8_v8_c,
  vpx_filter_block1d8_v8_avg_c,  vpx_filter_block1d8_v8_avg_c,
  vpx_filter_block1d16_v8_c,     vpx_filter_block1d16_v8_c,
  vpx_filter_block1d16_v8_avg_c, vpx_filter_block1d16_v8_avg_c)

#if HAVE_SSSE3 && CONFIG_USE_X86INC
SUBPIXEL_FILTER_TEST(SSSE3, kMaxNumFuncs,
  vpx_filter_block1d4_h8_org_ssse3,      vpx_filter_block1d4_h8_ssse3,
  vpx_filter_block1d4_h8_avg_org_ssse3,  vpx_filter_block1d4_h8_avg_ssse3,
  vpx_filter_block1d8_h8_org_ssse3,      vpx_filter_block1d8_h8_ssse3,
  vpx_filter_block1d8_h8_avg_org_ssse3,  vpx_filter_block1d8_h8_avg_ssse3,
  vpx_filter_block1d16_h8_org_ssse3,     vpx_filter_block1d16_h8_ssse3,
  vpx_filter_block1d16_h8_avg_org_ssse3, vpx_filter_block1d16_h8_avg_ssse3,
  vpx_filter_block1d4_v8_org_ssse3,      vpx_filter_block1d4_v8_ssse3,
  vpx_filter_block1d4_v8_avg_org_ssse3,  vpx_filter_block1d4_v8_avg_ssse3,
  vpx_filter_block1d8_v8_org_ssse3,      vpx_filter_block1d8_v8_ssse3,
  vpx_filter_block1d8_v8_avg_org_ssse3,  vpx_filter_block1d8_v8_avg_ssse3,
  vpx_filter_block1d16_v8_org_ssse3,     vpx_filter_block1d16_v8_ssse3,
  vpx_filter_block1d16_v8_avg_org_ssse3, vpx_filter_block1d16_v8_avg_ssse3)
#endif  // HAVE_SSSE3 && CONFIG_USE_X86INC

// -----------------------------------------------------------------------------

#include "test/test_libvpx.cc"
