/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
extern "C" {
#include "./vpx_config.h"
#include "./vp9_rtcd.h"
#include "vp9/common/vp9_blockd.h"
}

typedef void (*subtract_fn_t)(int rows, int cols,
                              int16_t *diff_ptr, ptrdiff_t diff_stride,
                              const uint8_t *src_ptr, ptrdiff_t src_stride,
                              const uint8_t *pred_ptr, ptrdiff_t pred_stride);

namespace vp9 {

class VP9SubtractBlockTest : public ::testing::TestWithParam<subtract_fn_t> {
 public:
  virtual void TearDown() {
    libvpx_test::ClearSystemState();
  }
};

using libvpx_test::ACMRandom;

TEST_P(VP9SubtractBlockTest, SimpleSubtract) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());

  // FIXME(rbultje) split in its own file
  for (BLOCK_SIZE_TYPE bs = BLOCK_SIZE_AB4X4; bs < BLOCK_SIZE_TYPES;
       bs = static_cast<BLOCK_SIZE_TYPE>(static_cast<int>(bs) + 1)) {
    const int bw = 4 << b_width_log2(bs);
    const int bh = 4 << b_height_log2(bs);
    int16_t *diff = new int16_t[bw * bh * 2];
    uint8_t *pred = new uint8_t[bw * bh * 2];
    uint8_t *src  = new uint8_t[bw * bh * 2];

    for (int n = 0; n < 10; n++) {
      for (int r = 0; r < bh; ++r) {
        for (int c = 0; c < bw * 2; ++c) {
          src[r * bw * 2 + c] = rnd.Rand8();
          pred[r * bw * 2 + c] = rnd.Rand8();
        }
      }

      GetParam()(bh, bw, diff, bw, src, bw, pred, bw);

      for (int r = 0; r < bh; ++r) {
        for (int c = 0; c < bw; ++c) {
          EXPECT_EQ(diff[r * bw + c],
                    (src[r * bw + c] - pred[r * bw + c])) << "r = " << r
                                                          << ", c = " << c
                                                          << ", bs = " << bs;
        }
      }

      GetParam()(bh, bw, diff, bw * 2, src, bw * 2, pred, bw * 2);

      for (int r = 0; r < bh; ++r) {
        for (int c = 0; c < bw; ++c) {
          EXPECT_EQ(diff[r * bw * 2 + c],
                    (src[r * bw * 2 + c] - pred[r * bw * 2 + c]))
                                                        << "r = " << r
                                                        << ", c = " << c
                                                        << ", bs = " << bs;
        }
      }
    }
    delete[] diff;
    delete[] pred;
    delete[] src;
  }
}

INSTANTIATE_TEST_CASE_P(C, VP9SubtractBlockTest,
                        ::testing::Values(vp9_subtract_block_c));

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, VP9SubtractBlockTest,
                        ::testing::Values(vp9_subtract_block_sse2));
#endif

}  // namespace vp9
