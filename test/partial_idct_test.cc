/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/util.h"

extern "C" {
#include "./vp9_rtcd.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_scan.h"
}

#include "vpx/vpx_integer.h"

using libvpx_test::ACMRandom;

namespace {
typedef void (*fwd_txfm_t)(const int16_t *in, int16_t *out, int stride);
typedef void (*inv_txfm_t)(const int16_t *in, uint8_t *out, int stride);
typedef std::tr1::tuple<fwd_txfm_t,
                        inv_txfm_t,
                        inv_txfm_t,
                        TX_SIZE, int> partial_itxfm_param_t;
const int kMaxNumCoeffs = 1024;
class PartialIDctTest : public ::testing::TestWithParam<partial_itxfm_param_t> {
 public:
  virtual ~PartialIDctTest() {}
  virtual void SetUp() {
    fwd_txfm_ = GET_PARAM(0);
    full_itxfm_ = GET_PARAM(1);
    partial_itxfm_ = GET_PARAM(2);
    tx_size_  = GET_PARAM(3);
    last_nonzero_ = GET_PARAM(4);
  }

  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  int last_nonzero_;
  TX_SIZE tx_size_;
  fwd_txfm_t fwd_txfm_;
  inv_txfm_t full_itxfm_;
  inv_txfm_t partial_itxfm_;
};

TEST_P(PartialIDctTest, ResultsMatch) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  int32_t max_error = 0;
  const int count_test_block = 1000;
  DECLARE_ALIGNED_ARRAY(16, int16_t, test_diff_block, kMaxNumCoeffs);
  DECLARE_ALIGNED_ARRAY(16, int16_t, test_coef_block, kMaxNumCoeffs);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, dst1, kMaxNumCoeffs);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, dst2, kMaxNumCoeffs);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, src, kMaxNumCoeffs);
  int size;

  switch (tx_size_) {
  case TX_4X4:
    size = 4;
    break;
  case TX_8X8:
    size = 8;
    break;
  case TX_16X16:
    size = 16;
    break;
  case TX_32X32:
    size = 32;
    break;
  default:
    break;
  }


  for (int i = 0; i < count_test_block; ++i) {
    // Initialize a test block with input range [-255,test_input_block,  255].
    for (int j = 0; j < size * size; ++j) {
      src[j] = rnd.Rand8();
      dst1[j] = dst2[j] = rnd.Rand8();
      test_diff_block[j] = src[j] - dst1[j];
    }

    REGISTER_STATE_CHECK(fwd_txfm_(test_diff_block, test_coef_block, size));

    // clear out high order coefficients
    for (int j = last_nonzero_; j < size * size; ++j) {
      test_coef_block[vp9_default_scan_orders[tx_size_].scan[j]] =0;
    }
    REGISTER_STATE_CHECK(partial_itxfm_(test_coef_block, dst1, size));
    REGISTER_STATE_CHECK(partial_itxfm_(test_coef_block, dst2, size));

    for (int j = 0; j < size * size; ++j) {
      const int32_t diff = dst1[j] - dst2[j];
      const int32_t error = diff * diff;
      if (max_error < error)
        max_error = error;
    }
  }

  EXPECT_EQ(0, max_error)
      << "Error: partial inverse transform produces different results";
}
using std::tr1::make_tuple;

INSTANTIATE_TEST_CASE_P(
    C_TX32X32_34, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct32x32_c,
                   &vp9_idct32x32_1024_add_c,
                   &vp9_idct32x32_34_add_c,
                   TX_32X32, 34)));
INSTANTIATE_TEST_CASE_P(
    C_TX32X32_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct32x32_c,
                   &vp9_idct32x32_1024_add_c,
                   &vp9_idct32x32_1_add_c,
                   TX_32X32, 1)));

INSTANTIATE_TEST_CASE_P(
    C_TX16X16_10, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct16x16_c,
                   &vp9_idct16x16_256_add_c,
                   &vp9_idct16x16_10_add_c,
                   TX_16X16, 34)));
INSTANTIATE_TEST_CASE_P(
    C_TX16X16_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct16x16_c,
                   &vp9_idct16x16_256_add_c,
                   &vp9_idct16x16_1_add_c,
                   TX_16X16, 1)));
INSTANTIATE_TEST_CASE_P(
    C_TX8X8_10, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c,
                   &vp9_idct8x8_64_add_c,
                   &vp9_idct8x8_10_add_c,
                   TX_8X8, 34)));
INSTANTIATE_TEST_CASE_P(
    C_TX8X8_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c,
                   &vp9_idct8x8_64_add,
                   &vp9_idct8x8_1_add_c,
                   TX_8X8, 1)));
INSTANTIATE_TEST_CASE_P(
    C_TX4X4_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct4x4_c,
                   &vp9_idct4x4_16_add_c,
                   &vp9_idct4x4_1_add_c,
                   TX_4X4, 1)));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2_TX32X32_34, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct32x32_c,
                   &vp9_idct32x32_1024_add_c,
                   &vp9_idct32x32_34_add_sse2,
                   TX_32X32, 34)));
INSTANTIATE_TEST_CASE_P(
    SSE2_TX32X32_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct32x32_c,
                   &vp9_idct32x32_1024_add_c,
                   &vp9_idct32x32_1_add_sse2,
                   TX_32X32, 1)));

INSTANTIATE_TEST_CASE_P(
    SSE2_TX16X16_10, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct16x16_c,
                   &vp9_idct16x16_256_add_c,
                   &vp9_idct16x16_10_add_sse2,
                   TX_16X16, 34)));
INSTANTIATE_TEST_CASE_P(
    SSE2_TX16X16_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct16x16_c,
                   &vp9_idct16x16_256_add_c,
                   &vp9_idct16x16_1_add_sse2,
                   TX_16X16, 1)));
INSTANTIATE_TEST_CASE_P(
    SSE2_TX8X8_10, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c,
                   &vp9_idct8x8_64_add_c,
                   &vp9_idct8x8_10_add_sse2,
                   TX_8X8, 34)));
INSTANTIATE_TEST_CASE_P(
    SSE2_TX8X8_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c,
                   &vp9_idct8x8_64_add_c,
                   &vp9_idct8x8_1_add_sse2,
                   TX_8X8, 1)));
INSTANTIATE_TEST_CASE_P(
    SSE2_TX4X4_1, PartialIDctTest,
    ::testing::Values(
        make_tuple(&vp9_fdct4x4_c,
                   &vp9_idct4x4_16_add_c,
                   &vp9_idct4x4_1_add_sse2,
                   TX_4X4, 1)));

#endif

}  // namespace

