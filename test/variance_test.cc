/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
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
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"

namespace {

typedef unsigned int (*VarianceMxNFunc)(const uint8_t *a, int a_stride,
                                        const uint8_t *b, int b_stride,
                                        unsigned int *sse);
typedef unsigned int (*SubpixVarMxNFunc)(const uint8_t *a, int a_stride,
                                         int xoffset, int yoffset,
                                         const uint8_t *b, int b_stride,
                                         unsigned int *sse);
typedef unsigned int (*SubpixAvgVarMxNFunc)(const uint8_t *a, int a_stride,
                                            int xoffset, int yoffset,
                                            const uint8_t *b, int b_stride,
                                            uint32_t *sse,
                                            const uint8_t *second_pred);
typedef unsigned int (*Get4x4SseFunc)(const uint8_t *a, int a_stride,
                                      const uint8_t *b, int b_stride);
typedef unsigned int (*SumOfSquaresFunction)(const int16_t *src);

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
    default:
      break;
  }
}

static unsigned int mb_ss_ref(const int16_t *src) {
  unsigned int res = 0;
  for (int i = 0; i < 256; ++i) {
    res += src[i] * src[i];
  }
  return res;
}

/* Note:
 *  Our codebase calculates the "diff" value in the variance algorithm by
 *  (src - ref).
 */
static uint32_t variance_ref(const uint8_t *src, const uint8_t *ref,
                             int l2w, int l2h, int src_stride_coeff,
                             int ref_stride_coeff, uint32_t *sse_ptr,
                             bool use_high_bit_depth_,
                             vpx_bit_depth_t bit_depth) {
  int64_t se = 0;
  uint64_t sse = 0;
  const int w = 1 << l2w;
  const int h = 1 << l2h;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int diff;
      if (!use_high_bit_depth_) {
        diff = src[w * y * src_stride_coeff + x] -
               ref[w * y * ref_stride_coeff + x];
        se += diff;
        sse += diff * diff;
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        diff = CONVERT_TO_SHORTPTR(src)[w * y * src_stride_coeff + x] -
               CONVERT_TO_SHORTPTR(ref)[w * y * ref_stride_coeff + x];
        se += diff;
        sse += diff * diff;
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
  }
  RoundHighBitDepth(bit_depth, &se, &sse);
  *sse_ptr = static_cast<uint32_t>(sse);
  return static_cast<uint32_t>(sse -
                               ((static_cast<int64_t>(se) * se) >>
                                (l2w + l2h)));
}

/* The subpel reference functions differ from the codec version in one aspect:
 * they calculate the bilinear factors directly instead of using a lookup table
 * and therefore upshift xoff and yoff by 1. Only every other calculated value
 * is used so the codec version shrinks the table to save space and maintain
 * compatibility with vp8.
 */
static uint32_t subpel_variance_ref(const uint8_t *ref, const uint8_t *src,
                                    int l2w, int l2h, int xoff, int yoff,
                                    uint32_t *sse_ptr,
                                    bool use_high_bit_depth_,
                                    vpx_bit_depth_t bit_depth) {
  int64_t se = 0;
  uint64_t sse = 0;
  const int w = 1 << l2w;
  const int h = 1 << l2h;

  xoff <<= 1;
  yoff <<= 1;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      // Bilinear interpolation at a 16th pel step.
      if (!use_high_bit_depth_) {
        const int a1 = ref[(w + 1) * (y + 0) + x + 0];
        const int a2 = ref[(w + 1) * (y + 0) + x + 1];
        const int b1 = ref[(w + 1) * (y + 1) + x + 0];
        const int b2 = ref[(w + 1) * (y + 1) + x + 1];
        const int a = a1 + (((a2 - a1) * xoff + 8) >> 4);
        const int b = b1 + (((b2 - b1) * xoff + 8) >> 4);
        const int r = a + (((b - a) * yoff + 8) >> 4);
        const int diff = r - src[w * y + x];
        se += diff;
        sse += diff * diff;
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        uint16_t *ref16 = CONVERT_TO_SHORTPTR(ref);
        uint16_t *src16 = CONVERT_TO_SHORTPTR(src);
        const int a1 = ref16[(w + 1) * (y + 0) + x + 0];
        const int a2 = ref16[(w + 1) * (y + 0) + x + 1];
        const int b1 = ref16[(w + 1) * (y + 1) + x + 0];
        const int b2 = ref16[(w + 1) * (y + 1) + x + 1];
        const int a = a1 + (((a2 - a1) * xoff + 8) >> 4);
        const int b = b1 + (((b2 - b1) * xoff + 8) >> 4);
        const int r = a + (((b - a) * yoff + 8) >> 4);
        const int diff = r - src16[w * y + x];
        se += diff;
        sse += diff * diff;
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
  }
  RoundHighBitDepth(bit_depth, &se, &sse);
  *sse_ptr = static_cast<uint32_t>(sse);
  return static_cast<uint32_t>(sse -
                               ((static_cast<int64_t>(se) * se) >>
                                (l2w + l2h)));
}

static uint32_t subpel_avg_variance_ref(const uint8_t *ref,
                                        const uint8_t *src,
                                        const uint8_t *second_pred,
                                        int l2w, int l2h,
                                        int xoff, int yoff,
                                        uint32_t *sse_ptr,
                                        bool use_high_bit_depth,
                                        vpx_bit_depth_t bit_depth) {
  int64_t se = 0;
  uint64_t sse = 0;
  const int w = 1 << l2w;
  const int h = 1 << l2h;

  xoff <<= 1;
  yoff <<= 1;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      // bilinear interpolation at a 16th pel step
      if (!use_high_bit_depth) {
        const int a1 = ref[(w + 1) * (y + 0) + x + 0];
        const int a2 = ref[(w + 1) * (y + 0) + x + 1];
        const int b1 = ref[(w + 1) * (y + 1) + x + 0];
        const int b2 = ref[(w + 1) * (y + 1) + x + 1];
        const int a = a1 + (((a2 - a1) * xoff + 8) >> 4);
        const int b = b1 + (((b2 - b1) * xoff + 8) >> 4);
        const int r = a + (((b - a) * yoff + 8) >> 4);
        const int diff =
            ((r + second_pred[w * y + x] + 1) >> 1) - src[w * y + x];
        se += diff;
        sse += diff * diff;
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        uint16_t *ref16 = CONVERT_TO_SHORTPTR(ref);
        uint16_t *src16 = CONVERT_TO_SHORTPTR(src);
        uint16_t *sec16   = CONVERT_TO_SHORTPTR(second_pred);
        const int a1 = ref16[(w + 1) * (y + 0) + x + 0];
        const int a2 = ref16[(w + 1) * (y + 0) + x + 1];
        const int b1 = ref16[(w + 1) * (y + 1) + x + 0];
        const int b2 = ref16[(w + 1) * (y + 1) + x + 1];
        const int a = a1 + (((a2 - a1) * xoff + 8) >> 4);
        const int b = b1 + (((b2 - b1) * xoff + 8) >> 4);
        const int r = a + (((b - a) * yoff + 8) >> 4);
        const int diff = ((r + sec16[w * y + x] + 1) >> 1) - src16[w * y + x];
        se += diff;
        sse += diff * diff;
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
  }
  RoundHighBitDepth(bit_depth, &se, &sse);
  *sse_ptr = static_cast<uint32_t>(sse);
  return static_cast<uint32_t>(sse -
                               ((static_cast<int64_t>(se) * se) >>
                                (l2w + l2h)));
}

////////////////////////////////////////////////////////////////////////////////

class SumOfSquaresTest : public ::testing::TestWithParam<SumOfSquaresFunction> {
 public:
  SumOfSquaresTest() : func_(GetParam()) {}

  virtual ~SumOfSquaresTest() {
    libvpx_test::ClearSystemState();
  }

 protected:
  void ConstTest();
  void RefTest();

  SumOfSquaresFunction func_;
  ACMRandom rnd_;
};

void SumOfSquaresTest::ConstTest() {
  int16_t mem[256];
  unsigned int res;
  for (int v = 0; v < 256; ++v) {
    for (int i = 0; i < 256; ++i) {
      mem[i] = v;
    }
    ASM_REGISTER_STATE_CHECK(res = func_(mem));
    EXPECT_EQ(256u * (v * v), res);
  }
}

void SumOfSquaresTest::RefTest() {
  int16_t mem[256];
  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 256; ++j) {
      mem[j] = rnd_.Rand8() - rnd_.Rand8();
    }

    const unsigned int expected = mb_ss_ref(mem);
    unsigned int res;
    ASM_REGISTER_STATE_CHECK(res = func_(mem));
    EXPECT_EQ(expected, res);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Encapsulating struct to store the function to test along with
// some testing context.
// Can be used for MSE, SSE, Variance, etc.
template<typename Func>
struct TestParams {
  TestParams(int log2w = 0, int log2h = 0,
             Func function = NULL, int bit_depth_value = 0)
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

  int log2width;
  int log2height;
  int width, height;
  int block_size;
  Func func;
  vpx_bit_depth_t bit_depth;
  int use_high_bit_depth;
  uint32_t mask;
};

// Main class for testing a function type
template<typename FunctionType>
class MainTestClass :
    public ::testing::TestWithParam<TestParams<FunctionType> > {
 public:
  virtual void SetUp() {
    params_ = this->GetParam();

    rnd_.Reset(ACMRandom::DeterministicSeed());
    const size_t unit =
       use_high_bit_depth() ? sizeof(uint16_t) : sizeof(uint8_t);
    src_ = reinterpret_cast<uint8_t *>(vpx_memalign(16, block_size() * unit));
    ref_ = new uint8_t[block_size() * unit];
    ASSERT_TRUE(src_ != NULL);
    ASSERT_TRUE(ref_ != NULL);
  }

  virtual void TearDown() {
    vpx_free(static_cast<void*>(src_));
    delete[] ref_;
    src_ = NULL;
    ref_ = NULL;
    libvpx_test::ClearSystemState();
  }

 protected:
  // We could sub-class MainTestClass into dedicated class for Variance
  // and MSE/SSE, but it involves a lot of 'this->xxx' dereferencing
  // to access top class fields xxx. That's cumbersome, so for now we'll just
  // implement the testing methods here:

  // Variance tests
  void ZeroTest();
  void RefTest();
  void RefStrideTest();
  void OneQuarterTest();

  // MSE/SSE tests
  void RefTest_mse();
  void RefTest_sse();
  void MaxTest_mse();
  void MaxTest_sse();

 protected:
  ACMRandom rnd_;
  uint8_t *src_;
  uint8_t *ref_;
  TestParams<FunctionType> params_;

  // some relay helpers
  bool use_high_bit_depth() const { return params_.use_high_bit_depth; }
  int byte_shift() const { return params_.bit_depth - 8; }
  int block_size() const { return params_.block_size; }
  int width() const { return params_.width; }
  uint32_t mask() const { return params_.mask; }
};

////////////////////////////////////////////////////////////////////////////////
// Tests related to variance.

template<typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::ZeroTest() {
  for (int i = 0; i <= 255; ++i) {
    if (!use_high_bit_depth()) {
      memset(src_, i, block_size());
#if CONFIG_VP9_HIGHBITDEPTH
    } else {
      vpx_memset16(CONVERT_TO_SHORTPTR(src_), i << byte_shift(), block_size());
#endif  // CONFIG_VP9_HIGHBITDEPTH
    }
    for (int j = 0; j <= 255; ++j) {
      if (!use_high_bit_depth()) {
        memset(ref_, j, block_size());
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        vpx_memset16(CONVERT_TO_SHORTPTR(ref_), j << byte_shift(), block_size());
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
      unsigned int sse, var;
      ASM_REGISTER_STATE_CHECK(
          var = params_.func(src_, width(), ref_, width(), &sse));
      EXPECT_EQ(0u, var) << "src values: " << i << " ref values: " << j;
    }
  }
}

template<typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::RefTest() {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < block_size(); j++) {
      if (!use_high_bit_depth()) {
        src_[j] = rnd_.Rand8();
        ref_[j] = rnd_.Rand8();
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        CONVERT_TO_SHORTPTR(src_)[j] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_)[j] = rnd_.Rand16() & mask();
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
    unsigned int sse1, sse2, var1, var2;
    const int stride_coeff = 1;
    ASM_REGISTER_STATE_CHECK(
        var1 = params_.func(src_, width(), ref_, width(), &sse1));
    var2 = variance_ref(src_, ref_, params_.log2width, params_.log2height,
                        stride_coeff, stride_coeff, &sse2,
                        use_high_bit_depth(), params_.bit_depth);
    EXPECT_EQ(sse1, sse2) << "Error at test index: " << i;
    EXPECT_EQ(var1, var2) << "Error at test index: " << i;
  }
}

template<typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::RefStrideTest() {
  for (int i = 0; i < 10; ++i) {
    int ref_stride_coeff = i % 2;
    int src_stride_coeff = (i >> 1) % 2;
    for (int j = 0; j < block_size(); j++) {
      int ref_ind = (j / width()) * ref_stride_coeff * width() + j % width();
      int src_ind = (j / width()) * src_stride_coeff * width() + j % width();
      if (!use_high_bit_depth()) {
        src_[src_ind] = rnd_.Rand8();
        ref_[ref_ind] = rnd_.Rand8();
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        CONVERT_TO_SHORTPTR(src_)[src_ind] = rnd_.Rand16() & mask();
        CONVERT_TO_SHORTPTR(ref_)[ref_ind] = rnd_.Rand16() & mask();
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
    }
    unsigned int sse1, sse2;
    unsigned int var1, var2;

    ASM_REGISTER_STATE_CHECK(
        var1 = params_.func(src_, width() * src_stride_coeff,
                         ref_, width() * ref_stride_coeff, &sse1));
    var2 = variance_ref(src_, ref_,
                        params_.log2width, params_.log2height,
                        src_stride_coeff, ref_stride_coeff, &sse2,
                        use_high_bit_depth(), params_.bit_depth);
    EXPECT_EQ(sse1, sse2) << "Error at test index: " << i;
    EXPECT_EQ(var1, var2) << "Error at test index: " << i;
  }
}

template<typename VarianceFunctionType>
void MainTestClass<VarianceFunctionType>::OneQuarterTest() {
  const int half = block_size() / 2;
  if (!use_high_bit_depth()) {
    memset(src_, 255, block_size());
    memset(ref_, 255, half);
    memset(ref_ + half, 0, half);
#if CONFIG_VP9_HIGHBITDEPTH
  } else {
    vpx_memset16(CONVERT_TO_SHORTPTR(src_), 255 << byte_shift(), block_size());
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_), 255 << byte_shift(), half);
    vpx_memset16(CONVERT_TO_SHORTPTR(ref_) + half, 0, half);
#endif  // CONFIG_VP9_HIGHBITDEPTH
  }
  unsigned int sse, var, expected;
  ASM_REGISTER_STATE_CHECK(
      var = params_.func(src_, width(), ref_, width(), &sse));
  expected = block_size() * 255 * 255 / 4;
  EXPECT_EQ(expected, var);
}

////////////////////////////////////////////////////////////////////////////////
// Tests related to MSE / SSE.

template<typename FunctionType>
void MainTestClass<FunctionType>::RefTest_mse() {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < block_size(); j++) {
      src_[j] = rnd_.Rand8();
      ref_[j] = rnd_.Rand8();
    }
    unsigned int sse1, sse2;
    const int stride_coeff = 1;
    ASM_REGISTER_STATE_CHECK(
      params_.func(src_, width(), ref_, width(), &sse1));
    variance_ref(src_, ref_,
                 params_.log2width, params_.log2height,
                 stride_coeff, stride_coeff, &sse2,
                 false, VPX_BITS_8);
    EXPECT_EQ(sse1, sse2);
  }
}

template<typename FunctionType>
void MainTestClass<FunctionType>::RefTest_sse() {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < block_size(); j++) {
      src_[j] = rnd_.Rand8();
      ref_[j] = rnd_.Rand8();
    }
    unsigned int sse2;
    unsigned int var1;
    const int stride_coeff = 1;
    ASM_REGISTER_STATE_CHECK(var1 = params_.func(src_, width(), ref_, width()));
    variance_ref(src_, ref_, params_.log2width, params_.log2height,
                 stride_coeff, stride_coeff, &sse2, false, VPX_BITS_8);
    EXPECT_EQ(var1, sse2);
  }
}

template<typename FunctionType>
void MainTestClass<FunctionType>::MaxTest_mse() {
  memset(src_, 255, block_size());
  memset(ref_, 0, block_size());
  unsigned int sse;
  ASM_REGISTER_STATE_CHECK(params_.func(src_, width(), ref_, width(), &sse));
  const unsigned int expected = block_size() * 255 * 255;
  EXPECT_EQ(expected, sse);
}

template<typename FunctionType>
void MainTestClass<FunctionType>::MaxTest_sse() {
  memset(src_, 255, block_size());
  memset(ref_, 0, block_size());
  unsigned int var;
  ASM_REGISTER_STATE_CHECK(var = params_.func(src_, width(), ref_, width()));
  const unsigned int expected = block_size() * 255 * 255;
  EXPECT_EQ(expected, var);
}

////////////////////////////////////////////////////////////////////////////////

using ::std::tr1::get;
using ::std::tr1::make_tuple;
using ::std::tr1::tuple;

template<typename SubpelVarianceFunctionType>
class SubpelVarianceTest
    : public ::testing::TestWithParam<tuple<int, int,
                                            SubpelVarianceFunctionType, int> > {
 public:
  virtual void SetUp() {
    const tuple<int, int, SubpelVarianceFunctionType, int>& params =
        this->GetParam();
    log2width_  = get<0>(params);
    width_ = 1 << log2width_;
    log2height_ = get<1>(params);
    height_ = 1 << log2height_;
    subpel_variance_ = get<2>(params);
    if (get<3>(params)) {
      bit_depth_ = (vpx_bit_depth_t) get<3>(params);
      use_high_bit_depth_ = true;
    } else {
      bit_depth_ = VPX_BITS_8;
      use_high_bit_depth_ = false;
    }
    mask_ = (1 << bit_depth_)-1;

    rnd_.Reset(ACMRandom::DeterministicSeed());
    block_size_ = width_ * height_;
    if (!use_high_bit_depth_) {
      src_ = reinterpret_cast<uint8_t *>(vpx_memalign(16, block_size_));
      sec_ = reinterpret_cast<uint8_t *>(vpx_memalign(16, block_size_));
      ref_ = new uint8_t[block_size_ + width_ + height_ + 1];
#if CONFIG_VP9_HIGHBITDEPTH
    } else {
      src_ = CONVERT_TO_BYTEPTR(
          reinterpret_cast<uint16_t *>(
              vpx_memalign(16, block_size_*sizeof(uint16_t))));
      sec_ = CONVERT_TO_BYTEPTR(
          reinterpret_cast<uint16_t *>(
              vpx_memalign(16, block_size_*sizeof(uint16_t))));
      ref_ = CONVERT_TO_BYTEPTR(
          new uint16_t[block_size_ + width_ + height_ + 1]);
#endif  // CONFIG_VP9_HIGHBITDEPTH
    }
    ASSERT_TRUE(src_ != NULL);
    ASSERT_TRUE(sec_ != NULL);
    ASSERT_TRUE(ref_ != NULL);
  }

  virtual void TearDown() {
    if (!use_high_bit_depth_) {
      vpx_free(src_);
      delete[] ref_;
      vpx_free(sec_);
#if CONFIG_VP9_HIGHBITDEPTH
    } else {
      vpx_free(CONVERT_TO_SHORTPTR(src_));
      delete[] CONVERT_TO_SHORTPTR(ref_);
      vpx_free(CONVERT_TO_SHORTPTR(sec_));
#endif  // CONFIG_VP9_HIGHBITDEPTH
    }
    libvpx_test::ClearSystemState();
  }

 protected:
  void RefTest();
  void ExtremeRefTest();

  ACMRandom rnd_;
  uint8_t *src_;
  uint8_t *ref_;
  uint8_t *sec_;
  bool use_high_bit_depth_;
  vpx_bit_depth_t bit_depth_;
  int width_, log2width_;
  int height_, log2height_;
  int block_size_,  mask_;
  SubpelVarianceFunctionType subpel_variance_;
};

template<typename SubpelVarianceFunctionType>
void SubpelVarianceTest<SubpelVarianceFunctionType>::RefTest() {
  for (int x = 0; x < 8; ++x) {
    for (int y = 0; y < 8; ++y) {
      if (!use_high_bit_depth_) {
        for (int j = 0; j < block_size_; j++) {
          src_[j] = rnd_.Rand8();
        }
        for (int j = 0; j < block_size_ + width_ + height_ + 1; j++) {
          ref_[j] = rnd_.Rand8();
        }
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        for (int j = 0; j < block_size_; j++) {
          CONVERT_TO_SHORTPTR(src_)[j] = rnd_.Rand16() & mask_;
        }
        for (int j = 0; j < block_size_ + width_ + height_ + 1; j++) {
          CONVERT_TO_SHORTPTR(ref_)[j] = rnd_.Rand16() & mask_;
        }
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
      unsigned int sse1, sse2;
      unsigned int var1;
      ASM_REGISTER_STATE_CHECK(var1 = subpel_variance_(ref_, width_ + 1, x, y,
                                                       src_, width_, &sse1));
      const unsigned int var2 = subpel_variance_ref(ref_, src_,
                                                    log2width_, log2height_,
                                                    x, y, &sse2,
                                                    use_high_bit_depth_,
                                                    bit_depth_);
      EXPECT_EQ(sse1, sse2) << "at position " << x << ", " << y;
      EXPECT_EQ(var1, var2) << "at position " << x << ", " << y;
    }
  }
}

template<typename SubpelVarianceFunctionType>
void SubpelVarianceTest<SubpelVarianceFunctionType>::ExtremeRefTest() {
  // Compare against reference.
  // Src: Set the first half of values to 0, the second half to the maximum.
  // Ref: Set the first half of values to the maximum, the second half to 0.
  for (int x = 0; x < 8; ++x) {
    for (int y = 0; y < 8; ++y) {
      const int half = block_size_ / 2;
      if (!use_high_bit_depth_) {
        memset(src_, 0, half);
        memset(src_ + half, 255, half);
        memset(ref_, 255, half);
        memset(ref_ + half, 0, half + width_ + height_ + 1);
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        vpx_memset16(CONVERT_TO_SHORTPTR(src_), mask_, half);
        vpx_memset16(CONVERT_TO_SHORTPTR(src_) + half, 0, half);
        vpx_memset16(CONVERT_TO_SHORTPTR(ref_), 0, half);
        vpx_memset16(CONVERT_TO_SHORTPTR(ref_) + half, mask_,
                     half + width_ + height_ + 1);
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
      unsigned int sse1, sse2;
      unsigned int var1;
      ASM_REGISTER_STATE_CHECK(
          var1 = subpel_variance_(ref_, width_ + 1, x, y, src_, width_, &sse1));
      const unsigned int var2 =
          subpel_variance_ref(ref_, src_, log2width_, log2height_,
                              x, y, &sse2, use_high_bit_depth_, bit_depth_);
      EXPECT_EQ(sse1, sse2) << "for xoffset " << x << " and yoffset " << y;
      EXPECT_EQ(var1, var2) << "for xoffset " << x << " and yoffset " << y;
    }
  }
}

template<>
void SubpelVarianceTest<SubpixAvgVarMxNFunc>::RefTest() {
  for (int x = 0; x < 8; ++x) {
    for (int y = 0; y < 8; ++y) {
      if (!use_high_bit_depth_) {
        for (int j = 0; j < block_size_; j++) {
          src_[j] = rnd_.Rand8();
          sec_[j] = rnd_.Rand8();
        }
        for (int j = 0; j < block_size_ + width_ + height_ + 1; j++) {
          ref_[j] = rnd_.Rand8();
        }
#if CONFIG_VP9_HIGHBITDEPTH
      } else {
        for (int j = 0; j < block_size_; j++) {
          CONVERT_TO_SHORTPTR(src_)[j] = rnd_.Rand16() & mask_;
          CONVERT_TO_SHORTPTR(sec_)[j] = rnd_.Rand16() & mask_;
        }
        for (int j = 0; j < block_size_ + width_ + height_ + 1; j++) {
          CONVERT_TO_SHORTPTR(ref_)[j] = rnd_.Rand16() & mask_;
        }
#endif  // CONFIG_VP9_HIGHBITDEPTH
      }
      uint32_t sse1, sse2;
      uint32_t var1, var2;
      ASM_REGISTER_STATE_CHECK(
          var1 = subpel_variance_(ref_, width_ + 1, x, y,
                                  src_, width_, &sse1, sec_));
      var2 = subpel_avg_variance_ref(ref_, src_, sec_,
                                     log2width_, log2height_,
                                     x, y, &sse2,
                                     use_high_bit_depth_,
                                     static_cast<vpx_bit_depth_t>(bit_depth_));
      EXPECT_EQ(sse1, sse2) << "at position " << x << ", " << y;
      EXPECT_EQ(var1, var2) << "at position " << x << ", " << y;
    }
  }
}

typedef MainTestClass<Get4x4SseFunc> VpxSseTest;
typedef MainTestClass<VarianceMxNFunc> VpxMseTest;
typedef MainTestClass<VarianceMxNFunc> VpxVarianceTest;
typedef SubpelVarianceTest<SubpixVarMxNFunc> VpxSubpelVarianceTest;
typedef SubpelVarianceTest<SubpixAvgVarMxNFunc> VpxSubpelAvgVarianceTest;

TEST_P(VpxSseTest, Ref_sse) { RefTest_sse(); }
TEST_P(VpxSseTest, Max_sse) { MaxTest_sse(); }
TEST_P(VpxMseTest, Ref_mse) { RefTest_mse(); }
TEST_P(VpxMseTest, Max_mse) { MaxTest_mse(); }
TEST_P(VpxVarianceTest, Zero) { ZeroTest(); }
TEST_P(VpxVarianceTest, Ref) { RefTest(); }
TEST_P(VpxVarianceTest, RefStride) { RefStrideTest(); }
TEST_P(VpxVarianceTest, OneQuarter) { OneQuarterTest(); }
TEST_P(SumOfSquaresTest, Const) { ConstTest(); }
TEST_P(SumOfSquaresTest, Ref) { RefTest(); }
TEST_P(VpxSubpelVarianceTest, Ref) { RefTest(); }
TEST_P(VpxSubpelVarianceTest, ExtremeRef) { ExtremeRefTest(); }
TEST_P(VpxSubpelAvgVarianceTest, Ref) { RefTest(); }

INSTANTIATE_TEST_CASE_P(C, SumOfSquaresTest,
                        ::testing::Values(vpx_get_mb_ss_c));

typedef TestParams<Get4x4SseFunc> VpxSseParams;
INSTANTIATE_TEST_CASE_P(C, VpxSseTest,
    ::testing::Values(VpxSseParams(2, 2, &vpx_get4x4sse_cs_c)));

typedef TestParams<VarianceMxNFunc> MseParams;
INSTANTIATE_TEST_CASE_P(C, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_c),
                      MseParams(4, 3, &vpx_mse16x8_c),
                      MseParams(3, 4, &vpx_mse8x16_c),
                      MseParams(3, 3, &vpx_mse8x8_c)));

typedef TestParams<VarianceMxNFunc> VarianceParams;
INSTANTIATE_TEST_CASE_P(
    C, VpxVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance64x64_c),
                      VarianceParams(6, 5, &vpx_variance64x32_c),
                      VarianceParams(5, 6, &vpx_variance32x64_c),
                      VarianceParams(5, 5, &vpx_variance32x32_c),
                      VarianceParams(5, 4, &vpx_variance32x16_c),
                      VarianceParams(4, 5, &vpx_variance16x32_c),
                      VarianceParams(4, 4, &vpx_variance16x16_c),
                      VarianceParams(4, 3, &vpx_variance16x8_c),
                      VarianceParams(3, 4, &vpx_variance8x16_c),
                      VarianceParams(3, 3, &vpx_variance8x8_c),
                      VarianceParams(3, 2, &vpx_variance8x4_c),
                      VarianceParams(2, 3, &vpx_variance4x8_c),
                      VarianceParams(2, 2, &vpx_variance4x4_c)));

INSTANTIATE_TEST_CASE_P(
    C, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_variance64x64_c, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_variance64x32_c, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_variance32x64_c, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_c, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_variance32x16_c, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_variance16x32_c, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_variance16x16_c, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_variance16x8_c, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_variance8x16_c, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_c, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_variance8x4_c, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_variance4x8_c, 0),
                      make_tuple(2, 2, &vpx_sub_pixel_variance4x4_c, 0)));

INSTANTIATE_TEST_CASE_P(
    C, VpxSubpelAvgVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_avg_variance64x64_c, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_avg_variance64x32_c, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_avg_variance32x64_c, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_avg_variance32x32_c, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_avg_variance32x16_c, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_avg_variance16x32_c, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_avg_variance16x16_c, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_avg_variance16x8_c, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_avg_variance8x16_c, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_avg_variance8x8_c, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_avg_variance8x4_c, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_avg_variance4x8_c, 0),
                      make_tuple(2, 2, &vpx_sub_pixel_avg_variance4x4_c, 0)));

#if CONFIG_VP9_HIGHBITDEPTH
typedef MainTestClass<VarianceMxNFunc> VpxHBDMseTest;
typedef MainTestClass<VarianceMxNFunc> VpxHBDVarianceTest;
typedef SubpelVarianceTest<SubpixVarMxNFunc> VpxHBDSubpelVarianceTest;
typedef SubpelVarianceTest<SubpixAvgVarMxNFunc>
    VpxHBDSubpelAvgVarianceTest;

TEST_P(VpxHBDMseTest, Ref_mse) { RefTest_mse(); }
TEST_P(VpxHBDMseTest, Max_mse) { MaxTest_mse(); }
TEST_P(VpxHBDVarianceTest, Zero) { ZeroTest(); }
TEST_P(VpxHBDVarianceTest, Ref) { RefTest(); }
TEST_P(VpxHBDVarianceTest, RefStride) { RefStrideTest(); }
TEST_P(VpxHBDVarianceTest, OneQuarter) { OneQuarterTest(); }
TEST_P(VpxHBDSubpelVarianceTest, Ref) { RefTest(); }
TEST_P(VpxHBDSubpelVarianceTest, ExtremeRef) { ExtremeRefTest(); }
TEST_P(VpxHBDSubpelAvgVarianceTest, Ref) { RefTest(); }

/* TODO(debargha): This test does not support the highbd version
INSTANTIATE_TEST_CASE_P(
    C, VpxHBDMseTest,
    ::testing::Values(make_tuple(4, 4, &vpx_highbd_12_mse16x16_c),
                      make_tuple(4, 4, &vpx_highbd_12_mse16x8_c),
                      make_tuple(4, 4, &vpx_highbd_12_mse8x16_c),
                      make_tuple(4, 4, &vpx_highbd_12_mse8x8_c),
                      make_tuple(4, 4, &vpx_highbd_10_mse16x16_c),
                      make_tuple(4, 4, &vpx_highbd_10_mse16x8_c),
                      make_tuple(4, 4, &vpx_highbd_10_mse8x16_c),
                      make_tuple(4, 4, &vpx_highbd_10_mse8x8_c),
                      make_tuple(4, 4, &vpx_highbd_8_mse16x16_c),
                      make_tuple(4, 4, &vpx_highbd_8_mse16x8_c),
                      make_tuple(4, 4, &vpx_highbd_8_mse8x16_c),
                      make_tuple(4, 4, &vpx_highbd_8_mse8x8_c)));
*/

INSTANTIATE_TEST_CASE_P(
    C, VpxHBDVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_highbd_12_variance64x64_c, 12),
                      VarianceParams(6, 5, &vpx_highbd_12_variance64x32_c, 12),
                      VarianceParams(5, 6, &vpx_highbd_12_variance32x64_c, 12),
                      VarianceParams(5, 5, &vpx_highbd_12_variance32x32_c, 12),
                      VarianceParams(5, 4, &vpx_highbd_12_variance32x16_c, 12),
                      VarianceParams(4, 5, &vpx_highbd_12_variance16x32_c, 12),
                      VarianceParams(4, 4, &vpx_highbd_12_variance16x16_c, 12),
                      VarianceParams(4, 3, &vpx_highbd_12_variance16x8_c, 12),
                      VarianceParams(3, 4, &vpx_highbd_12_variance8x16_c, 12),
                      VarianceParams(3, 3, &vpx_highbd_12_variance8x8_c, 12),
                      VarianceParams(3, 2, &vpx_highbd_12_variance8x4_c, 12),
                      VarianceParams(2, 3, &vpx_highbd_12_variance4x8_c, 12),
                      VarianceParams(2, 2, &vpx_highbd_12_variance4x4_c, 12),
                      VarianceParams(6, 6, &vpx_highbd_10_variance64x64_c, 10),
                      VarianceParams(6, 5, &vpx_highbd_10_variance64x32_c, 10),
                      VarianceParams(5, 6, &vpx_highbd_10_variance32x64_c, 10),
                      VarianceParams(5, 5, &vpx_highbd_10_variance32x32_c, 10),
                      VarianceParams(5, 4, &vpx_highbd_10_variance32x16_c, 10),
                      VarianceParams(4, 5, &vpx_highbd_10_variance16x32_c, 10),
                      VarianceParams(4, 4, &vpx_highbd_10_variance16x16_c, 10),
                      VarianceParams(4, 3, &vpx_highbd_10_variance16x8_c, 10),
                      VarianceParams(3, 4, &vpx_highbd_10_variance8x16_c, 10),
                      VarianceParams(3, 3, &vpx_highbd_10_variance8x8_c, 10),
                      VarianceParams(3, 2, &vpx_highbd_10_variance8x4_c, 10),
                      VarianceParams(2, 3, &vpx_highbd_10_variance4x8_c, 10),
                      VarianceParams(2, 2, &vpx_highbd_10_variance4x4_c, 10),
                      VarianceParams(6, 6, &vpx_highbd_8_variance64x64_c, 8),
                      VarianceParams(6, 5, &vpx_highbd_8_variance64x32_c, 8),
                      VarianceParams(5, 6, &vpx_highbd_8_variance32x64_c, 8),
                      VarianceParams(5, 5, &vpx_highbd_8_variance32x32_c, 8),
                      VarianceParams(5, 4, &vpx_highbd_8_variance32x16_c, 8),
                      VarianceParams(4, 5, &vpx_highbd_8_variance16x32_c, 8),
                      VarianceParams(4, 4, &vpx_highbd_8_variance16x16_c, 8),
                      VarianceParams(4, 3, &vpx_highbd_8_variance16x8_c, 8),
                      VarianceParams(3, 4, &vpx_highbd_8_variance8x16_c, 8),
                      VarianceParams(3, 3, &vpx_highbd_8_variance8x8_c, 8),
                      VarianceParams(3, 2, &vpx_highbd_8_variance8x4_c, 8),
                      VarianceParams(2, 3, &vpx_highbd_8_variance4x8_c, 8),
                      VarianceParams(2, 2, &vpx_highbd_8_variance4x4_c, 8)));

INSTANTIATE_TEST_CASE_P(
    C, VpxHBDSubpelVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_highbd_8_sub_pixel_variance64x64_c, 8),
        make_tuple(6, 5, &vpx_highbd_8_sub_pixel_variance64x32_c, 8),
        make_tuple(5, 6, &vpx_highbd_8_sub_pixel_variance32x64_c, 8),
        make_tuple(5, 5, &vpx_highbd_8_sub_pixel_variance32x32_c, 8),
        make_tuple(5, 4, &vpx_highbd_8_sub_pixel_variance32x16_c, 8),
        make_tuple(4, 5, &vpx_highbd_8_sub_pixel_variance16x32_c, 8),
        make_tuple(4, 4, &vpx_highbd_8_sub_pixel_variance16x16_c, 8),
        make_tuple(4, 3, &vpx_highbd_8_sub_pixel_variance16x8_c, 8),
        make_tuple(3, 4, &vpx_highbd_8_sub_pixel_variance8x16_c, 8),
        make_tuple(3, 3, &vpx_highbd_8_sub_pixel_variance8x8_c, 8),
        make_tuple(3, 2, &vpx_highbd_8_sub_pixel_variance8x4_c, 8),
        make_tuple(2, 3, &vpx_highbd_8_sub_pixel_variance4x8_c, 8),
        make_tuple(2, 2, &vpx_highbd_8_sub_pixel_variance4x4_c, 8),
        make_tuple(6, 6, &vpx_highbd_10_sub_pixel_variance64x64_c, 10),
        make_tuple(6, 5, &vpx_highbd_10_sub_pixel_variance64x32_c, 10),
        make_tuple(5, 6, &vpx_highbd_10_sub_pixel_variance32x64_c, 10),
        make_tuple(5, 5, &vpx_highbd_10_sub_pixel_variance32x32_c, 10),
        make_tuple(5, 4, &vpx_highbd_10_sub_pixel_variance32x16_c, 10),
        make_tuple(4, 5, &vpx_highbd_10_sub_pixel_variance16x32_c, 10),
        make_tuple(4, 4, &vpx_highbd_10_sub_pixel_variance16x16_c, 10),
        make_tuple(4, 3, &vpx_highbd_10_sub_pixel_variance16x8_c, 10),
        make_tuple(3, 4, &vpx_highbd_10_sub_pixel_variance8x16_c, 10),
        make_tuple(3, 3, &vpx_highbd_10_sub_pixel_variance8x8_c, 10),
        make_tuple(3, 2, &vpx_highbd_10_sub_pixel_variance8x4_c, 10),
        make_tuple(2, 3, &vpx_highbd_10_sub_pixel_variance4x8_c, 10),
        make_tuple(2, 2, &vpx_highbd_10_sub_pixel_variance4x4_c, 10),
        make_tuple(6, 6, &vpx_highbd_12_sub_pixel_variance64x64_c, 12),
        make_tuple(6, 5, &vpx_highbd_12_sub_pixel_variance64x32_c, 12),
        make_tuple(5, 6, &vpx_highbd_12_sub_pixel_variance32x64_c, 12),
        make_tuple(5, 5, &vpx_highbd_12_sub_pixel_variance32x32_c, 12),
        make_tuple(5, 4, &vpx_highbd_12_sub_pixel_variance32x16_c, 12),
        make_tuple(4, 5, &vpx_highbd_12_sub_pixel_variance16x32_c, 12),
        make_tuple(4, 4, &vpx_highbd_12_sub_pixel_variance16x16_c, 12),
        make_tuple(4, 3, &vpx_highbd_12_sub_pixel_variance16x8_c, 12),
        make_tuple(3, 4, &vpx_highbd_12_sub_pixel_variance8x16_c, 12),
        make_tuple(3, 3, &vpx_highbd_12_sub_pixel_variance8x8_c, 12),
        make_tuple(3, 2, &vpx_highbd_12_sub_pixel_variance8x4_c, 12),
        make_tuple(2, 3, &vpx_highbd_12_sub_pixel_variance4x8_c, 12),
        make_tuple(2, 2, &vpx_highbd_12_sub_pixel_variance4x4_c, 12)));

INSTANTIATE_TEST_CASE_P(
    C, VpxHBDSubpelAvgVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_highbd_8_sub_pixel_avg_variance64x64_c, 8),
        make_tuple(6, 5, &vpx_highbd_8_sub_pixel_avg_variance64x32_c, 8),
        make_tuple(5, 6, &vpx_highbd_8_sub_pixel_avg_variance32x64_c, 8),
        make_tuple(5, 5, &vpx_highbd_8_sub_pixel_avg_variance32x32_c, 8),
        make_tuple(5, 4, &vpx_highbd_8_sub_pixel_avg_variance32x16_c, 8),
        make_tuple(4, 5, &vpx_highbd_8_sub_pixel_avg_variance16x32_c, 8),
        make_tuple(4, 4, &vpx_highbd_8_sub_pixel_avg_variance16x16_c, 8),
        make_tuple(4, 3, &vpx_highbd_8_sub_pixel_avg_variance16x8_c, 8),
        make_tuple(3, 4, &vpx_highbd_8_sub_pixel_avg_variance8x16_c, 8),
        make_tuple(3, 3, &vpx_highbd_8_sub_pixel_avg_variance8x8_c, 8),
        make_tuple(3, 2, &vpx_highbd_8_sub_pixel_avg_variance8x4_c, 8),
        make_tuple(2, 3, &vpx_highbd_8_sub_pixel_avg_variance4x8_c, 8),
        make_tuple(2, 2, &vpx_highbd_8_sub_pixel_avg_variance4x4_c, 8),
        make_tuple(6, 6, &vpx_highbd_10_sub_pixel_avg_variance64x64_c, 10),
        make_tuple(6, 5, &vpx_highbd_10_sub_pixel_avg_variance64x32_c, 10),
        make_tuple(5, 6, &vpx_highbd_10_sub_pixel_avg_variance32x64_c, 10),
        make_tuple(5, 5, &vpx_highbd_10_sub_pixel_avg_variance32x32_c, 10),
        make_tuple(5, 4, &vpx_highbd_10_sub_pixel_avg_variance32x16_c, 10),
        make_tuple(4, 5, &vpx_highbd_10_sub_pixel_avg_variance16x32_c, 10),
        make_tuple(4, 4, &vpx_highbd_10_sub_pixel_avg_variance16x16_c, 10),
        make_tuple(4, 3, &vpx_highbd_10_sub_pixel_avg_variance16x8_c, 10),
        make_tuple(3, 4, &vpx_highbd_10_sub_pixel_avg_variance8x16_c, 10),
        make_tuple(3, 3, &vpx_highbd_10_sub_pixel_avg_variance8x8_c, 10),
        make_tuple(3, 2, &vpx_highbd_10_sub_pixel_avg_variance8x4_c, 10),
        make_tuple(2, 3, &vpx_highbd_10_sub_pixel_avg_variance4x8_c, 10),
        make_tuple(2, 2, &vpx_highbd_10_sub_pixel_avg_variance4x4_c, 10),
        make_tuple(6, 6, &vpx_highbd_12_sub_pixel_avg_variance64x64_c, 12),
        make_tuple(6, 5, &vpx_highbd_12_sub_pixel_avg_variance64x32_c, 12),
        make_tuple(5, 6, &vpx_highbd_12_sub_pixel_avg_variance32x64_c, 12),
        make_tuple(5, 5, &vpx_highbd_12_sub_pixel_avg_variance32x32_c, 12),
        make_tuple(5, 4, &vpx_highbd_12_sub_pixel_avg_variance32x16_c, 12),
        make_tuple(4, 5, &vpx_highbd_12_sub_pixel_avg_variance16x32_c, 12),
        make_tuple(4, 4, &vpx_highbd_12_sub_pixel_avg_variance16x16_c, 12),
        make_tuple(4, 3, &vpx_highbd_12_sub_pixel_avg_variance16x8_c, 12),
        make_tuple(3, 4, &vpx_highbd_12_sub_pixel_avg_variance8x16_c, 12),
        make_tuple(3, 3, &vpx_highbd_12_sub_pixel_avg_variance8x8_c, 12),
        make_tuple(3, 2, &vpx_highbd_12_sub_pixel_avg_variance8x4_c, 12),
        make_tuple(2, 3, &vpx_highbd_12_sub_pixel_avg_variance4x8_c, 12),
        make_tuple(2, 2, &vpx_highbd_12_sub_pixel_avg_variance4x4_c, 12)));
#endif  // CONFIG_VP9_HIGHBITDEPTH

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, SumOfSquaresTest,
                        ::testing::Values(vpx_get_mb_ss_sse2));

INSTANTIATE_TEST_CASE_P(SSE2, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_sse2),
                      MseParams(4, 3, &vpx_mse16x8_sse2),
                      MseParams(3, 4, &vpx_mse8x16_sse2),
                      MseParams(3, 3, &vpx_mse8x8_sse2)));

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance64x64_sse2),
                      VarianceParams(6, 5, &vpx_variance64x32_sse2),
                      VarianceParams(5, 6, &vpx_variance32x64_sse2),
                      VarianceParams(5, 5, &vpx_variance32x32_sse2),
                      VarianceParams(5, 4, &vpx_variance32x16_sse2),
                      VarianceParams(4, 5, &vpx_variance16x32_sse2),
                      VarianceParams(4, 4, &vpx_variance16x16_sse2),
                      VarianceParams(4, 3, &vpx_variance16x8_sse2),
                      VarianceParams(3, 4, &vpx_variance8x16_sse2),
                      VarianceParams(3, 3, &vpx_variance8x8_sse2),
                      VarianceParams(3, 2, &vpx_variance8x4_sse2),
                      VarianceParams(2, 3, &vpx_variance4x8_sse2),
                      VarianceParams(2, 2, &vpx_variance4x4_sse2)));

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_variance64x64_sse2, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_variance64x32_sse2, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_variance32x64_sse2, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_sse2, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_variance32x16_sse2, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_variance16x32_sse2, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_variance16x16_sse2, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_variance16x8_sse2, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_variance8x16_sse2, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_sse2, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_variance8x4_sse2, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_variance4x8_sse2, 0),
                      make_tuple(2, 2, &vpx_sub_pixel_variance4x4_sse2, 0)));

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxSubpelAvgVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_sub_pixel_avg_variance64x64_sse2, 0),
        make_tuple(6, 5, &vpx_sub_pixel_avg_variance64x32_sse2, 0),
        make_tuple(5, 6, &vpx_sub_pixel_avg_variance32x64_sse2, 0),
        make_tuple(5, 5, &vpx_sub_pixel_avg_variance32x32_sse2, 0),
        make_tuple(5, 4, &vpx_sub_pixel_avg_variance32x16_sse2, 0),
        make_tuple(4, 5, &vpx_sub_pixel_avg_variance16x32_sse2, 0),
        make_tuple(4, 4, &vpx_sub_pixel_avg_variance16x16_sse2, 0),
        make_tuple(4, 3, &vpx_sub_pixel_avg_variance16x8_sse2, 0),
        make_tuple(3, 4, &vpx_sub_pixel_avg_variance8x16_sse2, 0),
        make_tuple(3, 3, &vpx_sub_pixel_avg_variance8x8_sse2, 0),
        make_tuple(3, 2, &vpx_sub_pixel_avg_variance8x4_sse2, 0),
        make_tuple(2, 3, &vpx_sub_pixel_avg_variance4x8_sse2, 0),
        make_tuple(2, 2, &vpx_sub_pixel_avg_variance4x4_sse2, 0)));

#if CONFIG_VP9_HIGHBITDEPTH
/* TODO(debargha): This test does not support the highbd version
INSTANTIATE_TEST_CASE_P(
    SSE2, VpxHBDMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_highbd_12_mse16x16_sse2),
                      MseParams(4, 3, &vpx_highbd_12_mse16x8_sse2),
                      MseParams(3, 4, &vpx_highbd_12_mse8x16_sse2),
                      MseParams(3, 3, &vpx_highbd_12_mse8x8_sse2),
                      MseParams(4, 4, &vpx_highbd_10_mse16x16_sse2),
                      MseParams(4, 3, &vpx_highbd_10_mse16x8_sse2),
                      MseParams(3, 4, &vpx_highbd_10_mse8x16_sse2),
                      MseParams(3, 3, &vpx_highbd_10_mse8x8_sse2),
                      MseParams(4, 4, &vpx_highbd_8_mse16x16_sse2),
                      MseParams(4, 3, &vpx_highbd_8_mse16x8_sse2),
                      MseParams(3, 4, &vpx_highbd_8_mse8x16_sse2),
                      MseParams(3, 3, &vpx_highbd_8_mse8x8_sse2)));
*/

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxHBDVarianceTest, ::testing::Values(
      VarianceParams(6, 6, &vpx_highbd_12_variance64x64_sse2, 12),
      VarianceParams(6, 5, &vpx_highbd_12_variance64x32_sse2, 12),
      VarianceParams(5, 6, &vpx_highbd_12_variance32x64_sse2, 12),
      VarianceParams(5, 5, &vpx_highbd_12_variance32x32_sse2, 12),
      VarianceParams(5, 4, &vpx_highbd_12_variance32x16_sse2, 12),
      VarianceParams(4, 5, &vpx_highbd_12_variance16x32_sse2, 12),
      VarianceParams(4, 4, &vpx_highbd_12_variance16x16_sse2, 12),
      VarianceParams(4, 3, &vpx_highbd_12_variance16x8_sse2, 12),
      VarianceParams(3, 4, &vpx_highbd_12_variance8x16_sse2, 12),
      VarianceParams(3, 3, &vpx_highbd_12_variance8x8_sse2, 12),
      VarianceParams(6, 6, &vpx_highbd_10_variance64x64_sse2, 10),
      VarianceParams(6, 5, &vpx_highbd_10_variance64x32_sse2, 10),
      VarianceParams(5, 6, &vpx_highbd_10_variance32x64_sse2, 10),
      VarianceParams(5, 5, &vpx_highbd_10_variance32x32_sse2, 10),
      VarianceParams(5, 4, &vpx_highbd_10_variance32x16_sse2, 10),
      VarianceParams(4, 5, &vpx_highbd_10_variance16x32_sse2, 10),
      VarianceParams(4, 4, &vpx_highbd_10_variance16x16_sse2, 10),
      VarianceParams(4, 3, &vpx_highbd_10_variance16x8_sse2, 10),
      VarianceParams(3, 4, &vpx_highbd_10_variance8x16_sse2, 10),
      VarianceParams(3, 3, &vpx_highbd_10_variance8x8_sse2, 10),
      VarianceParams(6, 6, &vpx_highbd_8_variance64x64_sse2, 8),
      VarianceParams(6, 5, &vpx_highbd_8_variance64x32_sse2, 8),
      VarianceParams(5, 6, &vpx_highbd_8_variance32x64_sse2, 8),
      VarianceParams(5, 5, &vpx_highbd_8_variance32x32_sse2, 8),
      VarianceParams(5, 4, &vpx_highbd_8_variance32x16_sse2, 8),
      VarianceParams(4, 5, &vpx_highbd_8_variance16x32_sse2, 8),
      VarianceParams(4, 4, &vpx_highbd_8_variance16x16_sse2, 8),
      VarianceParams(4, 3, &vpx_highbd_8_variance16x8_sse2, 8),
      VarianceParams(3, 4, &vpx_highbd_8_variance8x16_sse2, 8),
      VarianceParams(3, 3, &vpx_highbd_8_variance8x8_sse2, 8)));

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxHBDSubpelVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_highbd_12_sub_pixel_variance64x64_sse2, 12),
        make_tuple(6, 5, &vpx_highbd_12_sub_pixel_variance64x32_sse2, 12),
        make_tuple(5, 6, &vpx_highbd_12_sub_pixel_variance32x64_sse2, 12),
        make_tuple(5, 5, &vpx_highbd_12_sub_pixel_variance32x32_sse2, 12),
        make_tuple(5, 4, &vpx_highbd_12_sub_pixel_variance32x16_sse2, 12),
        make_tuple(4, 5, &vpx_highbd_12_sub_pixel_variance16x32_sse2, 12),
        make_tuple(4, 4, &vpx_highbd_12_sub_pixel_variance16x16_sse2, 12),
        make_tuple(4, 3, &vpx_highbd_12_sub_pixel_variance16x8_sse2, 12),
        make_tuple(3, 4, &vpx_highbd_12_sub_pixel_variance8x16_sse2, 12),
        make_tuple(3, 3, &vpx_highbd_12_sub_pixel_variance8x8_sse2, 12),
        make_tuple(3, 2, &vpx_highbd_12_sub_pixel_variance8x4_sse2, 12),
        make_tuple(6, 6, &vpx_highbd_10_sub_pixel_variance64x64_sse2, 10),
        make_tuple(6, 5, &vpx_highbd_10_sub_pixel_variance64x32_sse2, 10),
        make_tuple(5, 6, &vpx_highbd_10_sub_pixel_variance32x64_sse2, 10),
        make_tuple(5, 5, &vpx_highbd_10_sub_pixel_variance32x32_sse2, 10),
        make_tuple(5, 4, &vpx_highbd_10_sub_pixel_variance32x16_sse2, 10),
        make_tuple(4, 5, &vpx_highbd_10_sub_pixel_variance16x32_sse2, 10),
        make_tuple(4, 4, &vpx_highbd_10_sub_pixel_variance16x16_sse2, 10),
        make_tuple(4, 3, &vpx_highbd_10_sub_pixel_variance16x8_sse2, 10),
        make_tuple(3, 4, &vpx_highbd_10_sub_pixel_variance8x16_sse2, 10),
        make_tuple(3, 3, &vpx_highbd_10_sub_pixel_variance8x8_sse2, 10),
        make_tuple(3, 2, &vpx_highbd_10_sub_pixel_variance8x4_sse2, 10),
        make_tuple(6, 6, &vpx_highbd_8_sub_pixel_variance64x64_sse2, 8),
        make_tuple(6, 5, &vpx_highbd_8_sub_pixel_variance64x32_sse2, 8),
        make_tuple(5, 6, &vpx_highbd_8_sub_pixel_variance32x64_sse2, 8),
        make_tuple(5, 5, &vpx_highbd_8_sub_pixel_variance32x32_sse2, 8),
        make_tuple(5, 4, &vpx_highbd_8_sub_pixel_variance32x16_sse2, 8),
        make_tuple(4, 5, &vpx_highbd_8_sub_pixel_variance16x32_sse2, 8),
        make_tuple(4, 4, &vpx_highbd_8_sub_pixel_variance16x16_sse2, 8),
        make_tuple(4, 3, &vpx_highbd_8_sub_pixel_variance16x8_sse2, 8),
        make_tuple(3, 4, &vpx_highbd_8_sub_pixel_variance8x16_sse2, 8),
        make_tuple(3, 3, &vpx_highbd_8_sub_pixel_variance8x8_sse2, 8),
        make_tuple(3, 2, &vpx_highbd_8_sub_pixel_variance8x4_sse2, 8)));

INSTANTIATE_TEST_CASE_P(
    SSE2, VpxHBDSubpelAvgVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_highbd_12_sub_pixel_avg_variance64x64_sse2, 12),
        make_tuple(6, 5, &vpx_highbd_12_sub_pixel_avg_variance64x32_sse2, 12),
        make_tuple(5, 6, &vpx_highbd_12_sub_pixel_avg_variance32x64_sse2, 12),
        make_tuple(5, 5, &vpx_highbd_12_sub_pixel_avg_variance32x32_sse2, 12),
        make_tuple(5, 4, &vpx_highbd_12_sub_pixel_avg_variance32x16_sse2, 12),
        make_tuple(4, 5, &vpx_highbd_12_sub_pixel_avg_variance16x32_sse2, 12),
        make_tuple(4, 4, &vpx_highbd_12_sub_pixel_avg_variance16x16_sse2, 12),
        make_tuple(4, 3, &vpx_highbd_12_sub_pixel_avg_variance16x8_sse2, 12),
        make_tuple(3, 4, &vpx_highbd_12_sub_pixel_avg_variance8x16_sse2, 12),
        make_tuple(3, 3, &vpx_highbd_12_sub_pixel_avg_variance8x8_sse2, 12),
        make_tuple(3, 2, &vpx_highbd_12_sub_pixel_avg_variance8x4_sse2, 12),
        make_tuple(6, 6, &vpx_highbd_10_sub_pixel_avg_variance64x64_sse2, 10),
        make_tuple(6, 5, &vpx_highbd_10_sub_pixel_avg_variance64x32_sse2, 10),
        make_tuple(5, 6, &vpx_highbd_10_sub_pixel_avg_variance32x64_sse2, 10),
        make_tuple(5, 5, &vpx_highbd_10_sub_pixel_avg_variance32x32_sse2, 10),
        make_tuple(5, 4, &vpx_highbd_10_sub_pixel_avg_variance32x16_sse2, 10),
        make_tuple(4, 5, &vpx_highbd_10_sub_pixel_avg_variance16x32_sse2, 10),
        make_tuple(4, 4, &vpx_highbd_10_sub_pixel_avg_variance16x16_sse2, 10),
        make_tuple(4, 3, &vpx_highbd_10_sub_pixel_avg_variance16x8_sse2, 10),
        make_tuple(3, 4, &vpx_highbd_10_sub_pixel_avg_variance8x16_sse2, 10),
        make_tuple(3, 3, &vpx_highbd_10_sub_pixel_avg_variance8x8_sse2, 10),
        make_tuple(3, 2, &vpx_highbd_10_sub_pixel_avg_variance8x4_sse2, 10),
        make_tuple(6, 6, &vpx_highbd_8_sub_pixel_avg_variance64x64_sse2, 8),
        make_tuple(6, 5, &vpx_highbd_8_sub_pixel_avg_variance64x32_sse2, 8),
        make_tuple(5, 6, &vpx_highbd_8_sub_pixel_avg_variance32x64_sse2, 8),
        make_tuple(5, 5, &vpx_highbd_8_sub_pixel_avg_variance32x32_sse2, 8),
        make_tuple(5, 4, &vpx_highbd_8_sub_pixel_avg_variance32x16_sse2, 8),
        make_tuple(4, 5, &vpx_highbd_8_sub_pixel_avg_variance16x32_sse2, 8),
        make_tuple(4, 4, &vpx_highbd_8_sub_pixel_avg_variance16x16_sse2, 8),
        make_tuple(4, 3, &vpx_highbd_8_sub_pixel_avg_variance16x8_sse2, 8),
        make_tuple(3, 4, &vpx_highbd_8_sub_pixel_avg_variance8x16_sse2, 8),
        make_tuple(3, 3, &vpx_highbd_8_sub_pixel_avg_variance8x8_sse2, 8),
        make_tuple(3, 2, &vpx_highbd_8_sub_pixel_avg_variance8x4_sse2, 8)));
#endif  // CONFIG_VP9_HIGHBITDEPTH
#endif  // HAVE_SSE2

#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(
    SSSE3, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_variance64x64_ssse3, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_variance64x32_ssse3, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_variance32x64_ssse3, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_ssse3, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_variance32x16_ssse3, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_variance16x32_ssse3, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_variance16x16_ssse3, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_variance16x8_ssse3, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_variance8x16_ssse3, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_ssse3, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_variance8x4_ssse3, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_variance4x8_ssse3, 0),
                      make_tuple(2, 2, &vpx_sub_pixel_variance4x4_ssse3, 0)));

INSTANTIATE_TEST_CASE_P(
    SSSE3, VpxSubpelAvgVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_sub_pixel_avg_variance64x64_ssse3, 0),
        make_tuple(6, 5, &vpx_sub_pixel_avg_variance64x32_ssse3, 0),
        make_tuple(5, 6, &vpx_sub_pixel_avg_variance32x64_ssse3, 0),
        make_tuple(5, 5, &vpx_sub_pixel_avg_variance32x32_ssse3, 0),
        make_tuple(5, 4, &vpx_sub_pixel_avg_variance32x16_ssse3, 0),
        make_tuple(4, 5, &vpx_sub_pixel_avg_variance16x32_ssse3, 0),
        make_tuple(4, 4, &vpx_sub_pixel_avg_variance16x16_ssse3, 0),
        make_tuple(4, 3, &vpx_sub_pixel_avg_variance16x8_ssse3, 0),
        make_tuple(3, 4, &vpx_sub_pixel_avg_variance8x16_ssse3, 0),
        make_tuple(3, 3, &vpx_sub_pixel_avg_variance8x8_ssse3, 0),
        make_tuple(3, 2, &vpx_sub_pixel_avg_variance8x4_ssse3, 0),
        make_tuple(2, 3, &vpx_sub_pixel_avg_variance4x8_ssse3, 0),
        make_tuple(2, 2, &vpx_sub_pixel_avg_variance4x4_ssse3, 0)));
#endif  // HAVE_SSSE3

#if HAVE_AVX2
INSTANTIATE_TEST_CASE_P(AVX2, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_avx2)));

INSTANTIATE_TEST_CASE_P(
    AVX2, VpxVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance64x64_avx2),
                      VarianceParams(6, 5, &vpx_variance64x32_avx2),
                      VarianceParams(5, 5, &vpx_variance32x32_avx2),
                      VarianceParams(5, 4, &vpx_variance32x16_avx2),
                      VarianceParams(4, 4, &vpx_variance16x16_avx2)));

INSTANTIATE_TEST_CASE_P(
    AVX2, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_variance64x64_avx2, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_avx2, 0)));

INSTANTIATE_TEST_CASE_P(
    AVX2, VpxSubpelAvgVarianceTest,
    ::testing::Values(
        make_tuple(6, 6, &vpx_sub_pixel_avg_variance64x64_avx2, 0),
        make_tuple(5, 5, &vpx_sub_pixel_avg_variance32x32_avx2, 0)));
#endif  // HAVE_AVX2

#if HAVE_MEDIA
INSTANTIATE_TEST_CASE_P(MEDIA, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_media)));

INSTANTIATE_TEST_CASE_P(
    MEDIA, VpxVarianceTest,
    ::testing::Values(VarianceParams(4, 4, &vpx_variance16x16_media),
                      VarianceParams(3, 3, &vpx_variance8x8_media)));

INSTANTIATE_TEST_CASE_P(
    MEDIA, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(4, 4, &vpx_sub_pixel_variance16x16_media, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_media, 0)));
#endif  // HAVE_MEDIA

#if HAVE_NEON
INSTANTIATE_TEST_CASE_P(NEON, VpxSseTest,
    ::testing::Values(MseParams(2, 2, &vpx_get4x4sse_cs_neon)));

INSTANTIATE_TEST_CASE_P(NEON, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_neon)));

INSTANTIATE_TEST_CASE_P(
    NEON, VpxVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance64x64_neon),
                      VarianceParams(6, 5, &vpx_variance64x32_neon),
                      VarianceParams(5, 6, &vpx_variance32x64_neon),
                      VarianceParams(5, 5, &vpx_variance32x32_neon),
                      VarianceParams(4, 4, &vpx_variance16x16_neon),
                      VarianceParams(4, 3, &vpx_variance16x8_neon),
                      VarianceParams(3, 4, &vpx_variance8x16_neon),
                      VarianceParams(3, 3, &vpx_variance8x8_neon)));

INSTANTIATE_TEST_CASE_P(
    NEON, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_variance64x64_neon, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_neon, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_variance16x16_neon, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_neon, 0)));
#endif  // HAVE_NEON

#if HAVE_MSA
INSTANTIATE_TEST_CASE_P(MSA, SumOfSquaresTest,
                        ::testing::Values(vpx_get_mb_ss_msa));

INSTANTIATE_TEST_CASE_P(MSA, VpxSseTest,
    ::testing::Values(MseParams(2, 2, &vpx_get4x4sse_cs_msa)));

INSTANTIATE_TEST_CASE_P(MSA, VpxMseTest,
    ::testing::Values(MseParams(4, 4, &vpx_mse16x16_msa),
                      MseParams(4, 3, &vpx_mse16x8_msa),
                      MseParams(3, 4, &vpx_mse8x16_msa),
                      MseParams(3, 3, &vpx_mse8x8_msa)));

INSTANTIATE_TEST_CASE_P(
    MSA, VpxVarianceTest,
    ::testing::Values(VarianceParams(6, 6, &vpx_variance64x64_msa),
                      VarianceParams(6, 5, &vpx_variance64x32_msa),
                      VarianceParams(5, 6, &vpx_variance32x64_msa),
                      VarianceParams(5, 5, &vpx_variance32x32_msa),
                      VarianceParams(5, 4, &vpx_variance32x16_msa),
                      VarianceParams(4, 5, &vpx_variance16x32_msa),
                      VarianceParams(4, 4, &vpx_variance16x16_msa),
                      VarianceParams(4, 3, &vpx_variance16x8_msa),
                      VarianceParams(3, 4, &vpx_variance8x16_msa),
                      VarianceParams(3, 3, &vpx_variance8x8_msa),
                      VarianceParams(3, 2, &vpx_variance8x4_msa),
                      VarianceParams(2, 3, &vpx_variance4x8_msa),
                      VarianceParams(2, 2, &vpx_variance4x4_msa)));

INSTANTIATE_TEST_CASE_P(
    MSA, VpxSubpelVarianceTest,
    ::testing::Values(make_tuple(2, 2, &vpx_sub_pixel_variance4x4_msa, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_variance4x8_msa, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_variance8x4_msa, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_variance8x8_msa, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_variance8x16_msa, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_variance16x8_msa, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_variance16x16_msa, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_variance16x32_msa, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_variance32x16_msa, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_variance32x32_msa, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_variance32x64_msa, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_variance64x32_msa, 0),
                      make_tuple(6, 6, &vpx_sub_pixel_variance64x64_msa, 0)));

INSTANTIATE_TEST_CASE_P(
    MSA, VpxSubpelAvgVarianceTest,
    ::testing::Values(make_tuple(6, 6, &vpx_sub_pixel_avg_variance64x64_msa, 0),
                      make_tuple(6, 5, &vpx_sub_pixel_avg_variance64x32_msa, 0),
                      make_tuple(5, 6, &vpx_sub_pixel_avg_variance32x64_msa, 0),
                      make_tuple(5, 5, &vpx_sub_pixel_avg_variance32x32_msa, 0),
                      make_tuple(5, 4, &vpx_sub_pixel_avg_variance32x16_msa, 0),
                      make_tuple(4, 5, &vpx_sub_pixel_avg_variance16x32_msa, 0),
                      make_tuple(4, 4, &vpx_sub_pixel_avg_variance16x16_msa, 0),
                      make_tuple(4, 3, &vpx_sub_pixel_avg_variance16x8_msa, 0),
                      make_tuple(3, 4, &vpx_sub_pixel_avg_variance8x16_msa, 0),
                      make_tuple(3, 3, &vpx_sub_pixel_avg_variance8x8_msa, 0),
                      make_tuple(3, 2, &vpx_sub_pixel_avg_variance8x4_msa, 0),
                      make_tuple(2, 3, &vpx_sub_pixel_avg_variance4x8_msa, 0),
                      make_tuple(2, 2, &vpx_sub_pixel_avg_variance4x4_msa, 0)));
#endif  // HAVE_MSA
}  // namespace
