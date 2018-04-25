/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <cstdlib>
#include <new>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "vpx/vpx_codec.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/variance.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

namespace {

using libvpx_test::ACMRandom;

// Truncate high bit depth results by downshifting (with rounding) by:
// 2 * (bit_depth - 8) for sse
// (bit_depth - 8) for se
static void RoundHighBitDepth(int bit_depth, int64_t *se, uint64_t *sse) {
  switch (bit_depth) {
    case VPX_BITS_12:
      *sse = (*sse + 128) >> 8;
      *se = (*se + 8) >> 4;
      break;
    case VPX_BITS_10:
      *sse = (*sse + 8) >> 4;
      *se = (*se + 2) >> 2;
      break;
    case VPX_BITS_8:
    default: break;
  }
}

/* Note:
 *  Our codebase calculates the "diff" value in the variance algorithm by
 *  (src - ref).
 */
static uint32_t variance_ref(const uint8_t *src, const uint8_t *ref, int l2w,
                             int l2h, int src_stride, int ref_stride,
                             uint32_t *sse_ptr, bool use_high_bit_depth_,
                             vpx_bit_depth_t bit_depth) {
  int64_t se = 0;
  uint64_t sse = 0;
  const int w = 1 << l2w;
  const int h = 1 << l2h;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int diff;
      if (!use_high_bit_depth_) {
        diff = src[y * src_stride + x] - ref[y * ref_stride + x];
        se += diff;
        sse += diff * diff;
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        diff = CONVERT_TO_SHORTPTR(src)[y * src_stride + x] -
               CONVERT_TO_SHORTPTR(ref)[y * ref_stride + x];
        se += diff;
        sse += diff * diff;
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
  }
  RoundHighBitDepth(bit_depth, &se, &sse);
  *sse_ptr = static_cast<uint32_t>(sse);
  return static_cast<uint32_t>(
      sse - ((static_cast<int64_t>(se) * se) >> (l2w + l2h)));
}

////////////////////////////////////////////////////////////////////////////////
// Encapsulating struct to store the function to test along with
// some testing context.
// Can be used for MSE, SSE, Variance, etc.

template <typename Func>
struct TestParams {
  TestParams(int log2w = 0, int log2h = 0, Func function = NULL,
             int bit_depth_value = 0)
      : log2width(log2w), log2height(log2h), func(function) {
    use_high_bit_depth = (bit_depth_value > 0);
    if (use_high_bit_depth) {
      bit_depth = static_cast<vpx_bit_depth_t>(bit_depth_value);
    } else {
      bit_depth = VPX_BITS_8;
    }
    width = 1 << log2width;
    height = 1 << log2height;
    block_size = width * height;
    mask = (1u << bit_depth) - 1;
  }

  int log2width, log2height;
  int width, height;
  int block_size;
  Func func;
  vpx_bit_depth_t bit_depth;
  bool use_high_bit_depth;
  uint32_t mask;
};

template <typename Func>
std::ostream &operator<<(std::ostream &os, const TestParams<Func> &p) {
  return os << "log2width/height:" << p.log2width << "/" << p.log2height
            << " function:" << reinterpret_cast<const void *>(p.func)
            << " bit-depth:" << p.bit_depth;
}

// Main class for testing a function type
template <typename FunctionType>
class MainTestClass
    : public ::testing::TestWithParam<TestParams<FunctionType> > {
 public:
  virtual void SetUp() {
    params_ = this->GetParam();

    rnd_.Reset(ACMRandom::DeterministicSeed());
    const size_t unit =
        use_high_bit_depth() ? sizeof(uint16_t) : sizeof(uint8_t);
    src_ = reinterpret_cast<uint8_t *>(vpx_memalign(16, block_size() * unit));
    ref_[0] = new uint8_t[block_size() * unit];
    ref_[1] = new uint8_t[block_size() * unit];
    ref_[2] = new uint8_t[block_size() * unit];
    ref_[3] = new uint8_t[block_size() * unit];
    ASSERT_TRUE(src_ != NULL);
    ASSERT_TRUE(ref_[0] != NULL);
    ASSERT_TRUE(ref_[1] != NULL);
    ASSERT_TRUE(ref_[2] != NULL);
    ASSERT_TRUE(ref_[3] != NULL);
#if CONFIG_VP9_HIGHBITDEPTH
    if (use_high_bit_depth()) {
      src_ = CONVERT_TO_BYTEPTR(src_);
      ref_[0] = CONVERT_TO_BYTEPTR(ref_[0]);
      ref_[1] = CONVERT_TO_BYTEPTR(ref_[1]);
      ref_[2] = CONVERT_TO_BYTEPTR(ref_[2]);
      ref_[3] = CONVERT_TO_BYTEPTR(ref_[3]);
    }
#endif
    ref[0] = ref_[0];
    ref[1] = ref_[1];
    ref[2] = ref_[2];
    ref[3] = ref_[3];
  }

  virtual void TearDown() {
#if CONFIG_VP9_HIGHBITDEPTH
    if (use_high_bit_depth()) {
      // TODO(skal): remove!
      src_ = reinterpret_cast<uint8_t *>(CONVERT_TO_SHORTPTR(src_));
      ref_[0] = reinterpret_cast<uint8_t *>(CONVERT_TO_SHORTPTR(ref_[0]));
      ref_[1] = reinterpret_cast<uint8_t *>(CONVERT_TO_SHORTPTR(ref_[1]));
      ref_[2] = reinterpret_cast<uint8_t *>(CONVERT_TO_SHORTPTR(ref_[2]));
      ref_[3] = reinterpret_cast<uint8_t *>(CONVERT_TO_SHORTPTR(ref_[3]));
    }
#endif

    vpx_free(src_);
    delete[] ref_[0];
    delete[] ref_[1];
    delete[] ref_[2];
    delete[] ref_[3];
    src_ = NULL;
    ref_[0] = NULL;
    ref_[1] = NULL;
    ref_[2] = NULL;
    ref_[3] = NULL;
    libvpx_test::ClearSystemState();
  }

 protected:
  void ZeroTest();
  void RefTest();
  void RefStrideTest();
  void OneQuarterTest();
  void SpeedTest();

 protected:
  ACMRandom rnd_;
  uint8_t *src_;
  const uint8_t *ref[4];
  uint8_t *ref_[4];
  TestParams<FunctionType> params_;

  // some relay helpers
  bool use_high_bit_depth() const { return params_.use_high_bit_depth; }
  int byte_shift() const { return params_.bit_depth - 8; }
  int block_size() const { return params_.block_size; }
  int width() const { return params_.width; }
  int height() const { return params_.height; }
  uint32_t mask() const { return params_.mask; }
};

template <typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::ZeroTest() {
  for (int i = 0; i <= 255; ++i) {
    if (!use_high_bit_depth()) {
      memset(src_, i, block_size());
    } else {
      uint16_t *const src16 = CONVERT_TO_SHORTPTR(src_);
      for (int k = 0; k < block_size(); ++k) src16[k] = i << byte_shift();
    }
    for (int j = 0; j <= 255; ++j) {
      if (!use_high_bit_depth()) {
        memset(ref_[0], j, block_size());
        memset(ref_[1], j, block_size());
        memset(ref_[2], j, block_size());
        memset(ref_[3], j, block_size());
      } else {
        uint16_t *ref16;
        ref16 = CONVERT_TO_SHORTPTR(ref_[0]);
        for (int k = 0; k < block_size(); ++k) ref16[k] = j << byte_shift();
        ref16 = CONVERT_TO_SHORTPTR(ref_[1]);
        for (int k = 0; k < block_size(); ++k) ref16[k] = j << byte_shift();
        ref16 = CONVERT_TO_SHORTPTR(ref_[2]);
        for (int k = 0; k < block_size(); ++k) ref16[k] = j << byte_shift();
        ref16 = CONVERT_TO_SHORTPTR(ref_[3]);
        for (int k = 0; k < block_size(); ++k) ref16[k] = j << byte_shift();
      }
      DECLARE_ALIGNED(16, uint32_t, sse[4]);
      DECLARE_ALIGNED(16, uint32_t, var[4]);
      ASM_REGISTER_STATE_CHECK(
          params_.func(src_, width(), ref, width(), sse, var));
      EXPECT_EQ(0u, var[0]) << "src values: " << i << " ref values: " << j;
      EXPECT_EQ(0u, var[1]) << "src values: " << i << " ref values: " << j;
      EXPECT_EQ(0u, var[2]) << "src values: " << i << " ref values: " << j;
      EXPECT_EQ(0u, var[3]) << "src values: " << i << " ref values: " << j;
    }
  }
}

template <typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::RefTest() {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < block_size(); j++) {
      if (!use_high_bit_depth()) {
        src_[j] = rnd_.Rand8();
        ref_[0][j] = rnd_.Rand8();
        ref_[1][j] = rnd_.Rand8();
        ref_[2][j] = rnd_.Rand8();
        ref_[3][j] = rnd_.Rand8();
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        CONVERT_TO_SHORTPTR(src_)[j] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[0])[j] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[1])[j] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[2])[j] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[3])[j] = rnd_.Rand16() & mask();
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
    DECLARE_ALIGNED(16, uint32_t, sse1[4]);
    DECLARE_ALIGNED(16, uint32_t, sse2[4]);
    DECLARE_ALIGNED(16, uint32_t, var1[4]);
    DECLARE_ALIGNED(16, uint32_t, var2[4]);
    const int stride = width();
    ASM_REGISTER_STATE_CHECK(
        params_.func(src_, stride, ref, stride, sse1, var1));
    var2[0] = variance_ref(ref_[0], src_, params_.log2width, params_.log2height,
                           stride, stride, &sse2[0], use_high_bit_depth(),
                           params_.bit_depth);
    var2[1] = variance_ref(ref_[1], src_, params_.log2width, params_.log2height,
                           stride, stride, &sse2[1], use_high_bit_depth(),
                           params_.bit_depth);
    var2[2] = variance_ref(ref_[2], src_, params_.log2width, params_.log2height,
                           stride, stride, &sse2[2], use_high_bit_depth(),
                           params_.bit_depth);
    var2[3] = variance_ref(ref_[3], src_, params_.log2width, params_.log2height,
                           stride, stride, &sse2[3], use_high_bit_depth(),
                           params_.bit_depth);
    EXPECT_EQ(sse1[0], sse2[0]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[1], sse2[1]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[2], sse2[2]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[3], sse2[3]) << "Error at test index: " << i;
    EXPECT_EQ(var1[0], var2[0]) << "Error at test index: " << i;
    EXPECT_EQ(var1[1], var2[1]) << "Error at test index: " << i;
    EXPECT_EQ(var1[2], var2[2]) << "Error at test index: " << i;
    EXPECT_EQ(var1[3], var2[3]) << "Error at test index: " << i;
  }
}

template <typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::RefStrideTest() {
  for (int i = 0; i < 10; ++i) {
    const int ref_stride = (i & 1) * width();
    const int src_stride = ((i >> 1) & 1) * width();
    for (int j = 0; j < block_size(); j++) {
      const int ref_ind = (j / width()) * ref_stride + j % width();
      const int src_ind = (j / width()) * src_stride + j % width();
      if (!use_high_bit_depth()) {
        src_[src_ind] = rnd_.Rand8();
        ref_[0][ref_ind] = rnd_.Rand8();
        ref_[1][ref_ind] = rnd_.Rand8();
        ref_[2][ref_ind] = rnd_.Rand8();
        ref_[3][ref_ind] = rnd_.Rand8();
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        CONVERT_TO_SHORTPTR(src_)[src_ind] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[0])[ref_ind] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[1])[ref_ind] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[2])[ref_ind] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_[3])[ref_ind] = rnd_.Rand16() & mask();
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
    DECLARE_ALIGNED(16, uint32_t, sse1[4]);
    DECLARE_ALIGNED(16, uint32_t, sse2[4]);
    DECLARE_ALIGNED(16, uint32_t, var1[4]);
    DECLARE_ALIGNED(16, uint32_t, var2[4]);

    ASM_REGISTER_STATE_CHECK(
        params_.func(src_, src_stride, ref, ref_stride, sse1, var1));
    var2[0] = variance_ref(ref_[0], src_, params_.log2width, params_.log2height,
                           ref_stride, src_stride, &sse2[0],
                           use_high_bit_depth(), params_.bit_depth);
    var2[1] = variance_ref(ref_[1], src_, params_.log2width, params_.log2height,
                           ref_stride, src_stride, &sse2[1],
                           use_high_bit_depth(), params_.bit_depth);
    var2[2] = variance_ref(ref_[2], src_, params_.log2width, params_.log2height,
                           ref_stride, src_stride, &sse2[2],
                           use_high_bit_depth(), params_.bit_depth);
    var2[3] = variance_ref(ref_[3], src_, params_.log2width, params_.log2height,
                           ref_stride, src_stride, &sse2[3],
                           use_high_bit_depth(), params_.bit_depth);
    EXPECT_EQ(sse1[0], sse2[0]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[1], sse2[1]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[2], sse2[2]) << "Error at test index: " << i;
    EXPECT_EQ(sse1[3], sse2[3]) << "Error at test index: " << i;
    EXPECT_EQ(var1[0], var2[0]) << "Error at test index: " << i;
    EXPECT_EQ(var1[1], var2[1]) << "Error at test index: " << i;
    EXPECT_EQ(var1[2], var2[2]) << "Error at test index: " << i;
    EXPECT_EQ(var1[3], var2[3]) << "Error at test index: " << i;
  }
}

template <typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::OneQuarterTest() {
  const int half = block_size() / 2;
  if (!use_high_bit_depth()) {
    memset(src_, 255, block_size());
    memset(ref_[0], 255, half);
    memset(ref_[1], 255, half);
    memset(ref_[2], 255, half);
    memset(ref_[3], 255, half);
    memset(ref_[0] + half, 0, half);
    memset(ref_[1] + half, 0, half);
    memset(ref_[2] + half, 0, half);
    memset(ref_[3] + half, 0, half);
#if CONFIG_VP9_HIGHBITDEPTH
  } else {
    vpx_memset16(CONVERT_TO_SHORTPTR(src_), 255 << byte_shift(), block_size());
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[0]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[1]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[2]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[3]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[0]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[1]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[2]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[3]) + half, 0, half);
#endif  // CONFIG_VP9_HIGHBITDEPTH
  }
  unsigned int expected;
  DECLARE_ALIGNED(16, uint32_t, sse[4]);
  DECLARE_ALIGNED(16, uint32_t, var[4]);
  ASM_REGISTER_STATE_CHECK(params_.func(src_, width(), ref, width(), sse, var));
  expected = block_size() * 255 * 255 / 4;
  EXPECT_EQ(expected, var[0]);
  EXPECT_EQ(expected, var[1]);
  EXPECT_EQ(expected, var[2]);
  EXPECT_EQ(expected, var[3]);
}

template <typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::SpeedTest() {
  const int half = block_size() / 2;
  if (!use_high_bit_depth()) {
    memset(src_, 255, block_size());
    memset(ref_, 255, half);
    memset(ref_, 255, half);
    memset(ref_, 255, half);
    memset(ref_, 255, half);
    memset(ref_ + half, 0, half);
    memset(ref_ + half, 0, half);
    memset(ref_ + half, 0, half);
    memset(ref_ + half, 0, half);
#if CONFIG_VP9_HIGHBITDEPTH
  } else {
    vpx_memset16(CONVERT_TO_SHORTPTR(src_), 255 << byte_shift(), block_size());
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[0]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[1]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[2]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[3]), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[0]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[1]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[2]) + half, 0, half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_[3]) + half, 0, half);
#endif  // CONFIG_VP9_HIGHBITDEPTH
  }
  DECLARE_ALIGNED(16, uint32_t, sse[4]);
  DECLARE_ALIGNED(16, uint32_t, var[4]);

  vpx_usec_timer timer;
  vpx_usec_timer_start(&timer);
  for (int i = 0; i < (1 << 30) / block_size(); ++i) {
    params_.func(src_, width(), ref, width(), sse, var);
  }
  vpx_usec_timer_mark(&timer);
  const int elapsed_time = static_cast<int>(vpx_usec_timer_elapsed(&timer));
  printf("Variance %dx%d time: %5d ms\n", width(), height(),
         elapsed_time / 1000);
}

typedef MainTestClass<vpx_variance_four_fn_t> VpxVarianceFourTest;

TEST_P(VpxVarianceFourTest, Zero) { ZeroTest(); }
TEST_P(VpxVarianceFourTest, Ref) { RefTest(); }
TEST_P(VpxVarianceFourTest, RefStride) { RefStrideTest(); }
TEST_P(VpxVarianceFourTest, OneQuarter) { OneQuarterTest(); }
TEST_P(VpxVarianceFourTest, DISABLED_Speed) { SpeedTest(); }

typedef TestParams<vpx_variance_four_fn_t> MseParams;

typedef TestParams<vpx_variance_four_fn_t> VarianceParams;
INSTANTIATE_TEST_CASE_P(
    C, VpxVarianceFourTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance_four_64x64_c),
                      VarianceParams(6, 5, &vpx_variance_four_64x32_c),
                      VarianceParams(5, 6, &vpx_variance_four_32x64_c),
                      VarianceParams(5, 5, &vpx_variance_four_32x32_c),
                      VarianceParams(5, 4, &vpx_variance_four_32x16_c),
                      VarianceParams(4, 5, &vpx_variance_four_16x32_c),
                      VarianceParams(4, 4, &vpx_variance_four_16x16_c),
                      VarianceParams(4, 3, &vpx_variance_four_16x8_c),
                      VarianceParams(3, 4, &vpx_variance_four_8x16_c),
                      VarianceParams(3, 3, &vpx_variance_four_8x8_c),
                      VarianceParams(3, 2, &vpx_variance_four_8x4_c),
                      VarianceParams(2, 3, &vpx_variance_four_4x8_c),
                      VarianceParams(2, 2, &vpx_variance_four_4x4_c)));

#if CONFIG_VP9_HIGHBITDEPTH
typedef MainTestClass<vpx_variance_four_fn_t> VpxHBDVarianceFourTest;

TEST_P(VpxHBDVarianceFourTest, Zero) { ZeroTest(); }
TEST_P(VpxHBDVarianceFourTest, Ref) { RefTest(); }
TEST_P(VpxHBDVarianceFourTest, RefStride) { RefStrideTest(); }
TEST_P(VpxHBDVarianceFourTest, OneQuarter) { OneQuarterTest(); }
TEST_P(VpxHBDVarianceFourTest, DISABLED_Speed) { SpeedTest(); }

INSTANTIATE_TEST_CASE_P(
    C, VpxHBDVarianceFourTest,
    ::testing::Values(
        VarianceParams(6, 6, &vpx_highbd_12_variance_four_64x64_c, 12),
        VarianceParams(6, 5, &vpx_highbd_12_variance_four_64x32_c, 12),
        VarianceParams(5, 6, &vpx_highbd_12_variance_four_32x64_c, 12),
        VarianceParams(5, 5, &vpx_highbd_12_variance_four_32x32_c, 12),
        VarianceParams(5, 4, &vpx_highbd_12_variance_four_32x16_c, 12),
        VarianceParams(4, 5, &vpx_highbd_12_variance_four_16x32_c, 12),
        VarianceParams(4, 4, &vpx_highbd_12_variance_four_16x16_c, 12),
        VarianceParams(4, 3, &vpx_highbd_12_variance_four_16x8_c, 12),
        VarianceParams(3, 4, &vpx_highbd_12_variance_four_8x16_c, 12),
        VarianceParams(3, 3, &vpx_highbd_12_variance_four_8x8_c, 12),
        VarianceParams(3, 2, &vpx_highbd_12_variance_four_8x4_c, 12),
        VarianceParams(2, 3, &vpx_highbd_12_variance_four_4x8_c, 12),
        VarianceParams(2, 2, &vpx_highbd_12_variance_four_4x4_c, 12),
        VarianceParams(6, 6, &vpx_highbd_10_variance_four_64x64_c, 10),
        VarianceParams(6, 5, &vpx_highbd_10_variance_four_64x32_c, 10),
        VarianceParams(5, 6, &vpx_highbd_10_variance_four_32x64_c, 10),
        VarianceParams(5, 5, &vpx_highbd_10_variance_four_32x32_c, 10),
        VarianceParams(5, 4, &vpx_highbd_10_variance_four_32x16_c, 10),
        VarianceParams(4, 5, &vpx_highbd_10_variance_four_16x32_c, 10),
        VarianceParams(4, 4, &vpx_highbd_10_variance_four_16x16_c, 10),
        VarianceParams(4, 3, &vpx_highbd_10_variance_four_16x8_c, 10),
        VarianceParams(3, 4, &vpx_highbd_10_variance_four_8x16_c, 10),
        VarianceParams(3, 3, &vpx_highbd_10_variance_four_8x8_c, 10),
        VarianceParams(3, 2, &vpx_highbd_10_variance_four_8x4_c, 10),
        VarianceParams(2, 3, &vpx_highbd_10_variance_four_4x8_c, 10),
        VarianceParams(2, 2, &vpx_highbd_10_variance_four_4x4_c, 10),
        VarianceParams(6, 6, &vpx_highbd_8_variance_four_64x64_c, 8),
        VarianceParams(6, 5, &vpx_highbd_8_variance_four_64x32_c, 8),
        VarianceParams(5, 6, &vpx_highbd_8_variance_four_32x64_c, 8),
        VarianceParams(5, 5, &vpx_highbd_8_variance_four_32x32_c, 8),
        VarianceParams(5, 4, &vpx_highbd_8_variance_four_32x16_c, 8),
        VarianceParams(4, 5, &vpx_highbd_8_variance_four_16x32_c, 8),
        VarianceParams(4, 4, &vpx_highbd_8_variance_four_16x16_c, 8),
        VarianceParams(4, 3, &vpx_highbd_8_variance_four_16x8_c, 8),
        VarianceParams(3, 4, &vpx_highbd_8_variance_four_8x16_c, 8),
        VarianceParams(3, 3, &vpx_highbd_8_variance_four_8x8_c, 8),
        VarianceParams(3, 2, &vpx_highbd_8_variance_four_8x4_c, 8),
        VarianceParams(2, 3, &vpx_highbd_8_variance_four_4x8_c, 8),
        VarianceParams(2, 2, &vpx_highbd_8_variance_four_4x4_c, 8)));
#endif  // CONFIG_VP9_HIGHBITDEPTH

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, VpxVarianceFourTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance_four_64x64_sse2),
                      VarianceParams(6, 5, &vpx_variance_four_64x32_sse2),
                      VarianceParams(5, 6, &vpx_variance_four_32x64_sse2),
                      VarianceParams(5, 5, &vpx_variance_four_32x32_sse2),
                      VarianceParams(5, 4, &vpx_variance_four_32x16_sse2),
                      VarianceParams(4, 5, &vpx_variance_four_16x32_sse2),
                      VarianceParams(4, 4, &vpx_variance_four_16x16_sse2),
                      VarianceParams(4, 3, &vpx_variance_four_16x8_sse2),
                      VarianceParams(3, 4, &vpx_variance_four_8x16_sse2),
                      VarianceParams(3, 3, &vpx_variance_four_8x8_sse2),
                      VarianceParams(3, 2, &vpx_variance_four_8x4_sse2),
                      VarianceParams(2, 3, &vpx_variance_four_4x8_sse2),
                      VarianceParams(2, 2, &vpx_variance_four_4x4_sse2)));

#if 0   // CONFIG_VP9_HIGHBITDEPTH
INSTANTIATE_TEST_CASE_P(
    SSE2, VpxHBDVarianceFourTest,
    ::testing::Values(
        VarianceParams(6, 6, &vpx_highbd_12_variance_four_64x64_sse2, 12),
        VarianceParams(6, 5, &vpx_highbd_12_variance_four_64x32_sse2, 12),
        VarianceParams(5, 6, &vpx_highbd_12_variance_four_32x64_sse2, 12),
        VarianceParams(5, 5, &vpx_highbd_12_variance_four_32x32_sse2, 12),
        VarianceParams(5, 4, &vpx_highbd_12_variance_four_32x16_sse2, 12),
        VarianceParams(4, 5, &vpx_highbd_12_variance_four_16x32_sse2, 12),
        VarianceParams(4, 4, &vpx_highbd_12_variance_four_16x16_sse2, 12),
        VarianceParams(4, 3, &vpx_highbd_12_variance_four_16x8_sse2, 12),
        VarianceParams(3, 4, &vpx_highbd_12_variance_four_8x16_sse2, 12),
        VarianceParams(3, 3, &vpx_highbd_12_variance_four_8x8_sse2, 12),
        VarianceParams(6, 6, &vpx_highbd_10_variance_four_64x64_sse2, 10),
        VarianceParams(6, 5, &vpx_highbd_10_variance_four_64x32_sse2, 10),
        VarianceParams(5, 6, &vpx_highbd_10_variance_four_32x64_sse2, 10),
        VarianceParams(5, 5, &vpx_highbd_10_variance_four_32x32_sse2, 10),
        VarianceParams(5, 4, &vpx_highbd_10_variance_four_32x16_sse2, 10),
        VarianceParams(4, 5, &vpx_highbd_10_variance_four_16x32_sse2, 10),
        VarianceParams(4, 4, &vpx_highbd_10_variance_four_16x16_sse2, 10),
        VarianceParams(4, 3, &vpx_highbd_10_variance_four_16x8_sse2, 10),
        VarianceParams(3, 4, &vpx_highbd_10_variance_four_8x16_sse2, 10),
        VarianceParams(3, 3, &vpx_highbd_10_variance_four_8x8_sse2, 10),
        VarianceParams(6, 6, &vpx_highbd_8_variance_four_64x64_sse2, 8),
        VarianceParams(6, 5, &vpx_highbd_8_variance_four_64x32_sse2, 8),
        VarianceParams(5, 6, &vpx_highbd_8_variance_four_32x64_sse2, 8),
        VarianceParams(5, 5, &vpx_highbd_8_variance_four_32x32_sse2, 8),
        VarianceParams(5, 4, &vpx_highbd_8_variance_four_32x16_sse2, 8),
        VarianceParams(4, 5, &vpx_highbd_8_variance_four_16x32_sse2, 8),
        VarianceParams(4, 4, &vpx_highbd_8_variance_four_16x16_sse2, 8),
        VarianceParams(4, 3, &vpx_highbd_8_variance_four_16x8_sse2, 8),
        VarianceParams(3, 4, &vpx_highbd_8_variance_four_8x16_sse2, 8),
        VarianceParams(3, 3, &vpx_highbd_8_variance_four_8x8_sse2, 8)));
#endif  // CONFIG_VP9_HIGHBITDEPTH
#endif  // HAVE_SSE2
}  // namespace
