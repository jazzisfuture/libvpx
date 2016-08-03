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
#if HAVE_NEON
#include "vpx_dsp/arm/loopfilter_16_neon.c"
#include "vpx_dsp/arm/loopfilter_4_neon.c"
#include "vpx_dsp/arm/loopfilter_8_neon.c"
#include "vpx_dsp/arm/loopfilter_neon.c"
#endif
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#undef MAX_LOOP_FILTER
#define MAX_LOOP_FILTER 63

namespace {

static const int kNumSourceTypes = 3;
static const int kNumCoeffs = (24 * 24);
static const int kNum6PFuncs = 8 + 4;
static const int kNum9PFuncs = 4 + 4;
static const char *const kSourceTypes[kNumSourceTypes] = {"filter4", "filter8",
                                                          "filter16"};
static const char *const kSignatures6P[kNumSourceTypes][kNum6PFuncs] = {
    {"72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816"},
    {"aaa5969dba7877579d3437515587459b", "aaa5969dba7877579d3437515587459b",
     "6101bad1e9b90c13b27545718a67b508", "c766e37e2634b583036aed59faa93332",
     "6101bad1e9b90c13b27545718a67b508", "9dea2992ad66ecb9cb3e4d51fc55dc81",
     "c766e37e2634b583036aed59faa93332", "8b584df8050f693ee477b7e30fbe9b77",
     "aaa5969dba7877579d3437515587459b", "aaa5969dba7877579d3437515587459b",
     "6101bad1e9b90c13b27545718a67b508", "c766e37e2634b583036aed59faa93332"},
    {"36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93"},
};
static const char *const kSignatures9P[kNumSourceTypes][kNum9PFuncs] = {
    {"72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816",
     "72125586f263377ffe651ad25b4eb816", "72125586f263377ffe651ad25b4eb816"},
    {"aaa5969dba7877579d3437515587459b", "aaa5969dba7877579d3437515587459b",
     "9dea2992ad66ecb9cb3e4d51fc55dc81", "8b584df8050f693ee477b7e30fbe9b77",
     "aaa5969dba7877579d3437515587459b", "aaa5969dba7877579d3437515587459b",
     "9dea2992ad66ecb9cb3e4d51fc55dc81", "8b584df8050f693ee477b7e30fbe9b77"},
    {"36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93",
     "36ab5bbc84127288a4bfbab259005f93", "36ab5bbc84127288a4bfbab259005f93"},
};

typedef void (*loop_op_t)(uint8_t *s, int p, const uint8_t *blimit,
                          const uint8_t *limit, const uint8_t *thresh);
typedef void (*dual_loop_op_t)(uint8_t *s, int p, const uint8_t *blimit0,
                               const uint8_t *limit0, const uint8_t *thresh0,
                               const uint8_t *blimit1, const uint8_t *limit1,
                               const uint8_t *thresh1);

typedef struct {
  bool testFlag;
  loop_op_t func;
  const char *name;
} VpxLoopfilter6PFuncInfo;

typedef struct {
  bool testFlag;
  dual_loop_op_t func;
  const char *name;
} VpxLoopfilter9PFuncInfo;

static void PrintTimeInfo(const uint8_t *s, const size_t sSize,
                          const char *name, const char *signature, const int k,
                          const int elapsed_time, int *elapsed_time_org) {
  libvpx_test::MD5 md5;
  md5.Add((const uint8_t *)s, sSize);
  printf("             %-45s | time: %4d ms", name, elapsed_time);
  EXPECT_STREQ(signature, md5.Get());
  if (!(k & 1)) {
    *elapsed_time_org = elapsed_time;
    printf("\n");
  } else {
    printf(" | gain: ");
    printf("%s", (*elapsed_time_org > elapsed_time) ? ANSI_COLOR_GREEN
                                                    : ANSI_COLOR_RED);
    printf("%5.1f%%\n",
           100.0 * (*elapsed_time_org - elapsed_time) / *elapsed_time_org);
    printf("%s", ANSI_COLOR_RESET);
  }
}

void TestLoopfilter(VpxLoopfilter6PFuncInfo const *loopfilter6PFuncInfos,
                    VpxLoopfilter9PFuncInfo const *loopfilter9PFuncInfos) {
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int count_test_block = 20000000;
  DECLARE_ALIGNED(8, uint8_t, s[kNumCoeffs]);
  DECLARE_ALIGNED(8, uint8_t, ss[kNumSourceTypes][kNumCoeffs]);
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
  tmp = static_cast<uint8_t>(rnd(3 * MAX_LOOP_FILTER + 4));
  DECLARE_ALIGNED(16, const uint8_t, blimit1[16]) = {
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp,
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp};
  tmp = static_cast<uint8_t>(rnd(MAX_LOOP_FILTER));
  DECLARE_ALIGNED(16, const uint8_t, limit1[16]) = {
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp,
      tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp};
  tmp = rnd.Rand8();
  DECLARE_ALIGNED(16, const uint8_t, thresh1[16]) = {
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
    printf("============================================================\n");
    printf("sourceType: %s\n", kSourceTypes[sourceType]);
    printf("-------------------- 6 parameters --------------------\n");
    for (int k = 0; k < 2 * kNum6PFuncs; ++k) {
      if (loopfilter6PFuncInfos[k].testFlag) {
        vpx_usec_timer timer;
        vpx_usec_timer_start(&timer);
        for (int num_tests = 0; num_tests < count_test_block; ++num_tests) {
          memcpy(s, ss[sourceType], sizeof(s));
          loopfilter6PFuncInfos[k].func(s + 8 + p * 8, p, blimit, limit,
                                        thresh);
        }
        libvpx_test::ClearSystemState();
        vpx_usec_timer_mark(&timer);
        const int elapsed_time =
            static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
        PrintTimeInfo(s, sizeof(s), loopfilter6PFuncInfos[k].name,
                      kSignatures6P[sourceType][k >> 1], k, elapsed_time,
                      &elapsed_time_org);
      }
    }

    printf("-------------------- 9 parameters --------------------\n");
    for (int k = 0; k < 2 * kNum9PFuncs; ++k) {
      if (loopfilter9PFuncInfos[k].testFlag) {
        vpx_usec_timer timer;
        vpx_usec_timer_start(&timer);
        for (int num_tests = 0; num_tests < count_test_block; ++num_tests) {
          memcpy(s, ss[sourceType], sizeof(s));
          loopfilter9PFuncInfos[k].func(s + 8 + p * 8, p, blimit, limit, thresh,
                                        blimit1, limit1, thresh1);
        }
        libvpx_test::ClearSystemState();
        vpx_usec_timer_mark(&timer);
        const int elapsed_time =
            static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
        PrintTimeInfo(s, sizeof(s), loopfilter9PFuncInfos[k].name,
                      kSignatures9P[sourceType][k >> 1], k, elapsed_time,
                      &elapsed_time_org);
      }
    }
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...).
// The test name is 'arch.TestLoopfilter', e.g., C.TestLoopfilter.
#define LOOPFILTER_SPEED_TEST(                                              \
    arch, h_4_func1, h_4_name1, h_4_func2, h_4_name2, v_4_func1, v_4_name1, \
    v_4_func2, v_4_name2, h_8_func1, h_8_name1, h_8_func2, h_8_name2,       \
    v_8_func1, v_8_name1, v_8_func2, v_8_name2, h_16_func1, h_16_name1,     \
    h_16_func2, h_16_name2, v_16_func1, v_16_name1, v_16_func2, v_16_name2, \
    h_16_dual_func1, h_16_dual_name1, h_16_dual_func2, h_16_dual_name2,     \
    v_16_dual_func1, v_16_dual_name1, v_16_dual_func2, v_16_dual_name2,     \
    h_4_dual_func1, h_4_dual_name1, h_4_dual_func2, h_4_dual_name2,         \
    v_4_dual_func1, v_4_dual_name1, v_4_dual_func2, v_4_dual_name2,         \
    h_8_dual_func1, h_8_dual_name1, h_8_dual_func2, h_8_dual_name2,         \
    v_8_dual_func1, v_8_dual_name1, v_8_dual_func2, v_8_dual_name2,         \
    /**/ h_4_func3, h_4_name3, v_4_func3, v_4_name3, h_4_dual_func3,        \
    h_4_dual_name3, v_4_dual_func3, v_4_dual_name3, h_8_func3, h_8_name3,   \
    v_8_func3, v_8_name3, h_8_dual_func3, h_8_dual_name3, v_8_dual_func3,   \
    v_8_dual_name3)                                                         \
  TEST(arch, TestLoopfilter) {                                              \
    static const VpxLoopfilter6PFuncInfo loopfilter6PFuncInfos[] = {        \
        {1, h_4_func1, h_4_name1},                                          \
        {1, h_4_func2, h_4_name2},                                          \
        {1, v_4_func1, v_4_name1},                                          \
        {1, v_4_func2, v_4_name2},                                          \
        {1, h_8_func1, h_8_name1},                                          \
        {1, h_8_func2, h_8_name2},                                          \
        {1, v_8_func1, v_8_name1},                                          \
        {1, v_8_func2, v_8_name2},                                          \
        {1, h_16_func1, h_16_name1},                                        \
        {1, h_16_func2, h_16_name2},                                        \
        {1, v_16_func1, v_16_name1},                                        \
        {1, v_16_func2, v_16_name2},                                        \
        {1, h_16_dual_func1, h_16_dual_name1},                              \
        {1, h_16_dual_func2, h_16_dual_name2},                              \
        {1, v_16_dual_func1, v_16_dual_name1},                              \
        {1, v_16_dual_func2, v_16_dual_name2},                              \
                                                                            \
        {1, h_4_func3, h_4_name3},                                          \
        {1, h_4_func2, h_4_name2},                                          \
        {1, v_4_func3, v_4_name3},                                          \
        {1, v_4_func2, v_4_name2},                                          \
        {1, h_8_func3, h_8_name3},                                          \
        {1, h_8_func2, h_8_name2},                                          \
        {1, v_8_func3, v_8_name3},                                          \
        {1, v_8_func2, v_8_name2},                                          \
    };                                                                      \
    static const VpxLoopfilter9PFuncInfo loopfilter9PFuncInfos[] = {        \
        {1, h_4_dual_func1, h_4_dual_name1},                                \
        {1, h_4_dual_func2, h_4_dual_name2},                                \
        {1, v_4_dual_func1, v_4_dual_name1},                                \
        {1, v_4_dual_func2, v_4_dual_name2},                                \
        {1, h_8_dual_func1, h_8_dual_name1},                                \
        {1, h_8_dual_func2, h_8_dual_name2},                                \
        {1, v_8_dual_func1, v_8_dual_name1},                                \
        {1, v_8_dual_func2, v_8_dual_name2},                                \
                                                                            \
        {1, h_4_dual_func3, h_4_dual_name3},                                \
        {1, h_4_dual_func2, h_4_dual_name2},                                \
        {1, v_4_dual_func3, v_4_dual_name3},                                \
        {1, v_4_dual_func2, v_4_dual_name2},                                \
        {1, h_8_dual_func3, h_8_dual_name3},                                \
        {1, h_8_dual_func2, h_8_dual_name2},                                \
        {1, v_8_dual_func3, v_8_dual_name3},                                \
        {1, v_8_dual_func2, v_8_dual_name2},                                \
    };                                                                      \
    TestLoopfilter(loopfilter6PFuncInfos, loopfilter9PFuncInfos);           \
  }

// -----------------------------------------------------------------------------

#if (HAVE_NEON_ASM && HAVE_NEON)
LOOPFILTER_SPEED_TEST(
    NEON, vpx_lpf_horizontal_4_neon_asm, "horizontal_4_neon (asm)",
    vpx_lpf_horizontal_4_neon, "horizontal_4_neon (intrin)",
    vpx_lpf_vertical_4_neon_asm, "vertical_4_neon (asm)",
    vpx_lpf_vertical_4_neon, "vertical_4_neon (intrin)",
    vpx_lpf_horizontal_8_neon_asm, "horizontal_8_neon (asm)",
    vpx_lpf_horizontal_8_neon, "horizontal_8_neon (intrin)",
    vpx_lpf_vertical_8_neon_asm, "vertical_8_neon (asm)",
    vpx_lpf_vertical_8_neon, "vertical_8_neon (intrin)",
    vpx_lpf_horizontal_edge_8_neon_asm, "horizontal_edge_8_neon (asm)",
    vpx_lpf_horizontal_edge_8_neon, "horizontal_edge_8_neon (intrin)",
    vpx_lpf_horizontal_edge_16_neon_asm, "horizontal_edge_16_neon (asm)",
    vpx_lpf_horizontal_edge_16_neon, "horizontal_edge_16_neon (intrin)",
    vpx_lpf_vertical_16_neon_asm, "vertical_16_neon (asm)",
    vpx_lpf_vertical_16_neon, "vertical_16_neon (intrin)",
    vpx_lpf_vertical_16_dual_neon_asm, "vertical_16_dual_neon (asm)",
    vpx_lpf_vertical_16_dual_neon, "vertical_16_dual_neon (intrin)",
    vpx_lpf_horizontal_4_dual_neon_asm, "horizontal_4_dual_neon (asm)",
    vpx_lpf_horizontal_4_dual_neon, "horizontal_4_dual_neon (intrin)",
    // There is no vpx_lpf_vertical_4_dual_neon_asm()
    vpx_lpf_vertical_4_dual_neon_org, "vertical_4_dual_neon (intrin org)",
    vpx_lpf_vertical_4_dual_neon, "vertical_4_dual_neon (intrin)",
    vpx_lpf_horizontal_8_dual_neon_asm, "horizontal_8_dual_neon (asm)",
    vpx_lpf_horizontal_8_dual_neon, "horizontal_8_dual_neon (intrin)",
    vpx_lpf_vertical_8_dual_neon_asm, "vertical_8_dual_neon (asm)",
    vpx_lpf_vertical_8_dual_neon, "vertical_8_dual_neon (intrin)",

    vpx_lpf_horizontal_4_neon_org, "horizontal_4_neon (intrin org)",
    vpx_lpf_vertical_4_neon_org, "vertical_4_neon (intrin org)",
    vpx_lpf_horizontal_4_dual_neon_org, "horizontal_4_dual_neon (intrin org)",
    vpx_lpf_vertical_4_dual_neon_org, "vertical_4_dual_neon (intrin org)",
    vpx_lpf_horizontal_8_neon_org, "horizontal_8_neon (intrin org)",
    vpx_lpf_vertical_8_neon_org, "vertical_8_neon (intrin org)",
    vpx_lpf_horizontal_8_dual_neon_org, "horizontal_8_dual_neon (intrin org)",
    vpx_lpf_vertical_8_dual_neon_org, "vertical_8_dual_neon (intrin org)")
#else
LOOPFILTER_SPEED_TEST(
    C, vpx_lpf_horizontal_4_c, "horizontal_4_c", vpx_lpf_horizontal_4_c,
    "horizontal_4_c", vpx_lpf_vertical_4_c, "vertical_4_c",
    vpx_lpf_vertical_4_c, "vertical_4_c", vpx_lpf_horizontal_8_c,
    "horizontal_8_c", vpx_lpf_horizontal_8_c, "horizontal_8_c",
    vpx_lpf_vertical_8_c, "vertical_8_c", vpx_lpf_vertical_8_c, "vertical_8_c",
    vpx_lpf_horizontal_edge_8_c, "horizontal_edge_8_c",
    vpx_lpf_horizontal_edge_8_c, "horizontal_edge_8_c",
    vpx_lpf_horizontal_edge_16_c, "horizontal_edge_16_c",
    vpx_lpf_horizontal_edge_16_c, "horizontal_edge_16_c", vpx_lpf_vertical_16_c,
    "vertical_16_c", vpx_lpf_vertical_16_c, "vertical_16_c",
    vpx_lpf_vertical_16_dual_c, "vertical_16_dual_c",
    vpx_lpf_vertical_16_dual_c, "vertical_16_dual_c",
    vpx_lpf_horizontal_4_dual_c, "horizontal_4_dual_c",
    vpx_lpf_horizontal_4_dual_c, "horizontal_4_dual_c",
    vpx_lpf_vertical_4_dual_c, "vertical_4_dual_c", vpx_lpf_vertical_4_dual_c,
    "vertical_4_dual_c", vpx_lpf_horizontal_8_dual_c, "horizontal_8_dual_c",
    vpx_lpf_horizontal_8_dual_c, "horizontal_8_dual_c",
    vpx_lpf_vertical_8_dual_c, "vertical_8_dual_c", vpx_lpf_vertical_8_dual_c,
    "vertical_8_dual_c",

    vpx_lpf_horizontal_4_c, "horizontal_4_c", vpx_lpf_vertical_4_c,
    "vertical_4_c", vpx_lpf_horizontal_4_dual_c, "horizontal_4_dual_c",
    vpx_lpf_vertical_4_dual_c, "vertical_4_dual_c", vpx_lpf_horizontal_8_c,
    "horizontal_8_c", vpx_lpf_vertical_8_c, "vertical_8_c",
    vpx_lpf_horizontal_8_dual_c, "horizontal_8_dual_c",
    vpx_lpf_vertical_8_dual_c, "vertical_8_dual_c")
#endif  // (HAVE_NEON_ASM && HAVE_NEON)

// -----------------------------------------------------------------------------

#include "test/test_libvpx.cc"
