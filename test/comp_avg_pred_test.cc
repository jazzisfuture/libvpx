/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include <string.h>

#include "./vpx_dsp_rtcd.h"
#include "test/acm_random.h"
#include "test/buffer.h"
#include "vpx_ports/vpx_timer.h"

namespace {

using ::libvpx_test::ACMRandom;
using ::libvpx_test::Buffer;

typedef void (*AvgPredFunc)(uint8_t *a, const uint8_t *b, int w, int h,
                            const uint8_t *c, int c_stride);

const int kMaxBlockSize = 64;
uint8_t avg_with_rounding(uint8_t a, uint8_t b) {
  uint16_t c = a + b;
  c += 1;
  c >>= 1;
  return c;
}

// Compares 2 buffers but only up to w*h - ignores trailing values. The existing
// calls use a 64x64 comp buffer for all size calls. The SSE2 over-reads and
// over-writes but this is acceptable under those circumstances.
bool compare_buffers(const uint8_t *a, const uint8_t *b, int w, int h) {
  int size = w * h;
  return !memcmp(a, b, sizeof(*a) * size);
}

void reference_pred(const Buffer<uint8_t> &a, const Buffer<uint8_t> &b, int w,
                    int h, uint8_t *c) {
  for (int height = 0; height < h; ++height) {
    for (int width = 0; width < w; ++width) {
      c[height * w + width] =
          avg_with_rounding(a.TopLeftPixel()[height * a.stride() + width],
                            b.TopLeftPixel()[height * b.stride() + width]);
    }
  }
}

class AvgPredTest : public ::testing::TestWithParam<AvgPredFunc> {
 public:
  virtual void SetUp() {
    avg_pred_func_ = GetParam();
    rnd_.Reset(ACMRandom::DeterministicSeed());
  }

 protected:
  AvgPredFunc avg_pred_func_;
  ACMRandom rnd_;
};

TEST_P(AvgPredTest, SizeCombinations) {
  // This is called as part of the sub pixel variance. As such it must be one of
  // the variance block sizes.

  for (int width_pow = 2; width_pow <= 6; ++width_pow) {
    for (int height_pow = width_pow - 1; height_pow <= width_pow + 1;
         ++height_pow) {
      // Don't test 4x2 or 64x128
      if (height_pow == 1 || height_pow == 7) continue;

      const int width = 1 << width_pow;
      const int height = 1 << height_pow;
      Buffer<uint8_t> a = Buffer<uint8_t>(width, height, 0);
      // Only the prediction buffer may have a stride not equal to width.
      Buffer<uint8_t> b = Buffer<uint8_t>(width, height, 8);
      Buffer<uint8_t> c_ref = Buffer<uint8_t>(kMaxBlockSize, kMaxBlockSize, 0);
      Buffer<uint8_t> c_chk = Buffer<uint8_t>(kMaxBlockSize, kMaxBlockSize, 0);

      a.Set(&rnd_, &ACMRandom::Rand8);
      b.Set(&rnd_, &ACMRandom::Rand8);
      c_ref.Set(0);
      c_chk.Set(0);

      reference_pred(a, b, width, height, c_ref.TopLeftPixel());
      avg_pred_func_(c_chk.TopLeftPixel(), a.TopLeftPixel(), width, height,
                     b.TopLeftPixel(), b.stride());
      EXPECT_TRUE(compare_buffers(c_chk.TopLeftPixel(), c_ref.TopLeftPixel(),
                                  width, height));
      if (HasFailure()) {
        c_chk.PrintDifference(c_ref);
        ASSERT_TRUE(false);
      }
    }
  }
}

TEST_P(AvgPredTest, CompareReferenceRandom) {
  const int width = 64;
  const int height = 32;
  Buffer<uint8_t> a = Buffer<uint8_t>(width, height, 0);
  Buffer<uint8_t> b = Buffer<uint8_t>(width, height, 8);
  Buffer<uint8_t> c_ref = Buffer<uint8_t>(kMaxBlockSize, kMaxBlockSize, 0);
  Buffer<uint8_t> c_chk = Buffer<uint8_t>(kMaxBlockSize, kMaxBlockSize, 0);

  for (int i = 0; i < 500; ++i) {
    a.Set(&rnd_, &ACMRandom::Rand8);
    b.Set(&rnd_, &ACMRandom::Rand8);
    c_ref.Set(0);
    c_chk.Set(0);

    reference_pred(a, b, width, height, c_ref.TopLeftPixel());
    avg_pred_func_(c_chk.TopLeftPixel(), a.TopLeftPixel(), width, height,
                   b.TopLeftPixel(), b.stride());
    EXPECT_TRUE(compare_buffers(c_chk.TopLeftPixel(), c_ref.TopLeftPixel(),
                                width, height));
    if (HasFailure()) {
      c_chk.PrintDifference(c_ref);
      ASSERT_TRUE(false);
    }
  }
}

TEST_P(AvgPredTest, DISABLED_Speed) {
  for (int width_pow = 2; width_pow <= 6; ++width_pow) {
    for (int height_pow = width_pow - 1; height_pow <= width_pow + 1;
         ++height_pow) {
      // Don't test 4x2 or 64x128
      if (height_pow == 1 || height_pow == 7) continue;

      const int width = 1 << width_pow;
      const int height = 1 << height_pow;
      Buffer<uint8_t> a = Buffer<uint8_t>(width, height, 0);
      Buffer<uint8_t> b = Buffer<uint8_t>(width, height, 8);
      Buffer<uint8_t> c = Buffer<uint8_t>(kMaxBlockSize, kMaxBlockSize, 0);

      a.Set(&rnd_, &ACMRandom::Rand8);
      b.Set(&rnd_, &ACMRandom::Rand8);

      vpx_usec_timer timer;
      vpx_usec_timer_start(&timer);
      for (int i = 0; i < 1000000; ++i) {
        avg_pred_func_(c.TopLeftPixel(), a.TopLeftPixel(), width, height,
                       b.TopLeftPixel(), b.stride());
      }
      vpx_usec_timer_mark(&timer);

      const int elapsed_time =
          static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
      printf("Average Test %dx%d time: %5d ms\n", width, height, elapsed_time);
    }
  }
}

INSTANTIATE_TEST_CASE_P(C, AvgPredTest,
                        ::testing::Values(&vpx_comp_avg_pred_c));

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, AvgPredTest,
                        ::testing::Values(&vpx_comp_avg_pred_sse2));
#endif  // HAVE_SSE2
}  // namespace
