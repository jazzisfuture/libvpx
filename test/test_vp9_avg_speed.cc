/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
//  Test and time VP9 IntPro functions

#include <stdio.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include <vector>

#include "./vp9_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/md5_helper.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

// -----------------------------------------------------------------------------

namespace {

typedef void (*IntProRowFunc)(int16_t hbuf[16], uint8_t const *ref,
                              const int ref_stride, const int height);
typedef int16_t (*IntProColFunc)(uint8_t const *ref, const int width);

void TestIntProRow(const char name[], IntProRowFunc row_func,
                   const std::vector<int> &heights) {
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int width = 16;
  // Handle blocks up 16x64
  const int kDataBlockSize = 16 * 64;
  DECLARE_ALIGNED(16, uint8_t, src[kDataBlockSize]);
  DECLARE_ALIGNED(16, int16_t, dest[16]);

  for (std::vector<int>::const_iterator iter = heights.begin();
       iter != heights.end(); ++iter) {
    const int height = *iter;
    ASSERT_LE(width * height, kDataBlockSize);

    for (int i = 0; i < width * height; ++i) src[i] = rnd.Rand8();
    const int kNumTests = 2000000;

    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int num_tests = 0; num_tests < kNumTests; ++num_tests) {
      row_func(dest, src, 0, height);
    }
    vpx_usec_timer_mark(&timer);
    libvpx_test::ClearSystemState();
    const int elapsed_time =
            static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
    libvpx_test::MD5 md5;
    md5.Add(reinterpret_cast<uint8_t*>(dest), sizeof(dest[0]) * 16);
    printf("%20s: height:%d %5d ms     MD5: %s\n", name, height, elapsed_time,
           md5.Get());
  }
}

void TestIntProCol(const char name[], IntProColFunc col_func,
                   const std::vector<int> &widths) {
  libvpx_test::ACMRandom rnd(libvpx_test::ACMRandom::DeterministicSeed());
  const int height = 1;
  // Handle one row up to 64 bytes.
  const int kDataBlockSize = 64;
  DECLARE_ALIGNED(16, uint8_t, src[kDataBlockSize]);

  for (std::vector<int>::const_iterator iter = widths.begin();
       iter != widths.end(); ++iter) {
    const int width = *iter;

    for (int i = 0; i < width * height; ++i) src[i] = rnd.Rand8();
    const int kNumTests = 20000000;

    int16_t sum = 0;
    vpx_usec_timer timer;
    vpx_usec_timer_start(&timer);
    for (int num_tests = 0; num_tests < kNumTests; ++num_tests) {
      sum = col_func(src, width);
    }
    vpx_usec_timer_mark(&timer);
    libvpx_test::ClearSystemState();
    const int elapsed_time =
            static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
    printf("%20s: width:%d %5d ms     sum: %d\n", name, width, elapsed_time,
           sum);
  }
}

}  // namespace

// Defines a test case for |arch| (e.g., C, SSE2, ...) The test name is
// 'arch.test_func', e.g., C.TestIntProRow.
#define INT_PRO_TEST(arch, test_func, name, avg_func)                       \
  TEST(arch, test_func) {                                                   \
    const int v[] = { 16, 32, 64 };                                         \
    std::vector<int> values(v, v + sizeof(v) / sizeof(v[0]));               \
    test_func(name, avg_func, values);                                      \
  }

INT_PRO_TEST(C, TestIntProRow, "IntProRow", &vp9_int_pro_row_c);
INT_PRO_TEST(C, TestIntProCol, "IntProCol", &vp9_int_pro_col_c);

#if HAVE_SSE2

INT_PRO_TEST(SSE2, TestIntProRow, "IntProRow", &vp9_int_pro_row_sse2);
INT_PRO_TEST(SSE2, TestIntProCol, "IntProCol", &vp9_int_pro_col_sse2);

#endif  // HAVE_SSE2

