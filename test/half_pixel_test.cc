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
#include "test/util.h"
#include "vpx/vpx_codec.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/variance.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_timer.h"

namespace {

using ::testing::make_tuple;
using libvpx_test::ACMRandom;

typedef ::testing::tuple<vpx_half_pixel_fn_t, vpx_half_pixel_fn_t, int>
    HalfPixelParam;

class HalfPixelTest : public ::testing::TestWithParam<HalfPixelParam> {
 public:
  virtual ~HalfPixelTest() {}
  virtual void SetUp() {
    func_c_ = GET_PARAM(0);
    func_ = GET_PARAM(1);
    pixel_size_ = GET_PARAM(2);

    assert(pixel_size_ == 1 || pixel_size_ == 2);
    rnd_.Reset(ACMRandom::DeterministicSeed());
    const size_t unit = (pixel_size_ == 2) ? sizeof(uint16_t) : sizeof(uint8_t);
    size_of_src_ = 512 * 66 * unit;
    size_of_dst_ = 128 * 128 * unit;

    s_ = new uint8_t[size_of_src_];
    r_ = reinterpret_cast<uint8_t *>(vpx_memalign(32, size_of_dst_));
    d_ = reinterpret_cast<uint8_t *>(vpx_memalign(32, size_of_dst_));
    ASSERT_TRUE(s_ != NULL);
    ASSERT_TRUE(r_ != NULL);
    ASSERT_TRUE(d_ != NULL);
    src_ = s_;
    ref_ = r_;
    dst_ = d_;
#if CONFIG_VP9_HIGHBITDEPTH
    if (pixel_size_ == 2) {
      src_ = CONVERT_TO_BYTEPTR(s_);
      ref_ = CONVERT_TO_BYTEPTR(r_);
      dst_ = CONVERT_TO_BYTEPTR(d_);
    }
#endif
  }

  virtual void TearDown() {
    delete[] s_;
    vpx_free(r_);
    vpx_free(d_);
    libvpx_test::ClearSystemState();
  }

 protected:
  void RefTest() {
    for (int i = 0; i < 100; ++i) {
      for (size_t j = 0; j < size_of_src_; ++j) {
        s_[j] = rnd_.Rand8();
      }
      for (size_t j = 0; j < size_of_dst_; ++j) {
        r_[j] = d_[j] = rnd_.Rand8();
      }

      const int src_stride = 64 + rnd_.Rand8();
      const int expected = func_c_(src_, src_stride, ref_);
      int res;
      ASM_REGISTER_STATE_CHECK(res = func_(src_, src_stride, dst_));
      EXPECT_EQ(expected, res);
      EXPECT_EQ(0, memcmp(r_, d_, size_of_dst_));
    }
  }

  ACMRandom rnd_;
  vpx_half_pixel_fn_t func_c_;
  vpx_half_pixel_fn_t func_;
  int pixel_size_;
  size_t size_of_src_;
  size_t size_of_dst_;
  uint8_t *src_;
  uint8_t *ref_;
  uint8_t *dst_;
  uint8_t *s_;
  uint8_t *r_;
  uint8_t *d_;
};

TEST_P(HalfPixelTest, Ref) { RefTest(); }

#if HAVE_SSE2

const HalfPixelParam sse2_half_pixel_tests[] = {
#if CONFIG_VP9_HIGHBITDEPTH
  make_tuple(vpx_highbd_hor_half_pixel4x4_c, vpx_highbd_hor_half_pixel4x4_sse2,
             2),
  make_tuple(vpx_highbd_hor_half_pixel4x8_c, vpx_highbd_hor_half_pixel4x8_sse2,
             2),
  make_tuple(vpx_highbd_hor_half_pixel8x4_c, vpx_highbd_hor_half_pixel8x4_sse2,
             2),
  make_tuple(vpx_highbd_hor_half_pixel8x8_c, vpx_highbd_hor_half_pixel8x8_sse2,
             2),
  make_tuple(vpx_highbd_hor_half_pixel8x16_c,
             vpx_highbd_hor_half_pixel8x16_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel16x8_c,
             vpx_highbd_hor_half_pixel16x8_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel16x16_c,
             vpx_highbd_hor_half_pixel16x16_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel16x32_c,
             vpx_highbd_hor_half_pixel16x32_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel32x16_c,
             vpx_highbd_hor_half_pixel32x16_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel32x32_c,
             vpx_highbd_hor_half_pixel32x32_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel32x64_c,
             vpx_highbd_hor_half_pixel32x64_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel64x32_c,
             vpx_highbd_hor_half_pixel64x32_sse2, 2),
  make_tuple(vpx_highbd_hor_half_pixel64x64_c,
             vpx_highbd_hor_half_pixel64x64_sse2, 2),

  make_tuple(vpx_highbd_ver_half_pixel4x4_c, vpx_highbd_ver_half_pixel4x4_sse2,
             2),
  make_tuple(vpx_highbd_ver_half_pixel4x8_c, vpx_highbd_ver_half_pixel4x8_sse2,
             2),
  make_tuple(vpx_highbd_ver_half_pixel8x4_c, vpx_highbd_ver_half_pixel8x4_sse2,
             2),
  make_tuple(vpx_highbd_ver_half_pixel8x8_c, vpx_highbd_ver_half_pixel8x8_sse2,
             2),
  make_tuple(vpx_highbd_ver_half_pixel8x16_c,
             vpx_highbd_ver_half_pixel8x16_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel16x8_c,
             vpx_highbd_ver_half_pixel16x8_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel16x16_c,
             vpx_highbd_ver_half_pixel16x16_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel16x32_c,
             vpx_highbd_ver_half_pixel16x32_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel32x16_c,
             vpx_highbd_ver_half_pixel32x16_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel32x32_c,
             vpx_highbd_ver_half_pixel32x32_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel32x64_c,
             vpx_highbd_ver_half_pixel32x64_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel64x32_c,
             vpx_highbd_ver_half_pixel64x32_sse2, 2),
  make_tuple(vpx_highbd_ver_half_pixel64x64_c,
             vpx_highbd_ver_half_pixel64x64_sse2, 2),
#endif  // CONFIG_VP9_HIGHBITDEPTH

  make_tuple(vpx_hor_half_pixel4x4_c, vpx_hor_half_pixel4x4_sse2, 1),
  make_tuple(vpx_hor_half_pixel4x8_c, vpx_hor_half_pixel4x8_sse2, 1),
  make_tuple(vpx_hor_half_pixel8x4_c, vpx_hor_half_pixel8x4_sse2, 1),
  make_tuple(vpx_hor_half_pixel8x8_c, vpx_hor_half_pixel8x8_sse2, 1),
  make_tuple(vpx_hor_half_pixel8x16_c, vpx_hor_half_pixel8x16_sse2, 1),
  make_tuple(vpx_hor_half_pixel16x8_c, vpx_hor_half_pixel16x8_sse2, 1),
  make_tuple(vpx_hor_half_pixel16x16_c, vpx_hor_half_pixel16x16_sse2, 1),
  make_tuple(vpx_hor_half_pixel16x32_c, vpx_hor_half_pixel16x32_sse2, 1),
  make_tuple(vpx_hor_half_pixel32x16_c, vpx_hor_half_pixel32x16_sse2, 1),
  make_tuple(vpx_hor_half_pixel32x32_c, vpx_hor_half_pixel32x32_sse2, 1),
  make_tuple(vpx_hor_half_pixel32x64_c, vpx_hor_half_pixel32x64_sse2, 1),
  make_tuple(vpx_hor_half_pixel64x32_c, vpx_hor_half_pixel64x32_sse2, 1),
  make_tuple(vpx_hor_half_pixel64x64_c, vpx_hor_half_pixel64x64_sse2, 1),

  make_tuple(vpx_ver_half_pixel4x4_c, vpx_ver_half_pixel4x4_sse2, 1),
  make_tuple(vpx_ver_half_pixel4x8_c, vpx_ver_half_pixel4x8_sse2, 1),
  make_tuple(vpx_ver_half_pixel8x4_c, vpx_ver_half_pixel8x4_sse2, 1),
  make_tuple(vpx_ver_half_pixel8x8_c, vpx_ver_half_pixel8x8_sse2, 1),
  make_tuple(vpx_ver_half_pixel8x16_c, vpx_ver_half_pixel8x16_sse2, 1),
  make_tuple(vpx_ver_half_pixel16x8_c, vpx_ver_half_pixel16x8_sse2, 1),
  make_tuple(vpx_ver_half_pixel16x16_c, vpx_ver_half_pixel16x16_sse2, 1),
  make_tuple(vpx_ver_half_pixel16x32_c, vpx_ver_half_pixel16x32_sse2, 1),
  make_tuple(vpx_ver_half_pixel32x16_c, vpx_ver_half_pixel32x16_sse2, 1),
  make_tuple(vpx_ver_half_pixel32x32_c, vpx_ver_half_pixel32x32_sse2, 1),
  make_tuple(vpx_ver_half_pixel32x64_c, vpx_ver_half_pixel32x64_sse2, 1),
  make_tuple(vpx_ver_half_pixel64x32_c, vpx_ver_half_pixel64x32_sse2, 1),
  make_tuple(vpx_ver_half_pixel64x64_c, vpx_ver_half_pixel64x64_sse2, 1),
};

INSTANTIATE_TEST_CASE_P(SSE2, HalfPixelTest,
                        ::testing::ValuesIn(sse2_half_pixel_tests));

#endif  // HAVE_SSE2

}  // namespace
