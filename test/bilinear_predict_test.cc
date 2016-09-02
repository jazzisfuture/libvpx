/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
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

#include "./vp8_rtcd.h"
#include "./vpx_config.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/util.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

namespace {

typedef void (*BilinearPredictFunc)(uint8_t *src_ptr, int src_pixels_per_line,
                                    int xoffset, int yoffset, uint8_t *dst_ptr,
                                    int dst_pitch);

typedef std::tr1::tuple<int, int, BilinearPredictFunc> BilinearPredictParam;

class BilinearPredictTest
    : public ::testing::TestWithParam<BilinearPredictParam> {
 public:
  static void SetUpTestCase() {
    src_ = reinterpret_cast<uint8_t *>(vpx_memalign(kDataAlignment, kSrcSize));
    dst_ = reinterpret_cast<uint8_t *>(vpx_memalign(kDataAlignment, kDstSize));
    dst_c_ =
        reinterpret_cast<uint8_t *>(vpx_memalign(kDataAlignment, kDstSize));
  }

  static void TearDownTestCase() {
    vpx_free(src_);
    src_ = NULL;
    vpx_free(dst_);
    dst_ = NULL;
    vpx_free(dst_c_);
    dst_c_ = NULL;
  }

  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  // Make test arrays big enough for 16x16 functions. Bilinear filters
  // need a single extra pixels on the right side.
  static const int kSrcStride = 17;
  static const int kDstStride = 16;
  static const int kDataAlignment = 16;
  static const int kSrcSize = kSrcStride * kSrcStride + 1;
  static const int kDstSize = kDstStride * kDstStride + 1;

  virtual void SetUp() {
    width_ = GET_PARAM(0);
    height_ = GET_PARAM(1);
    bilinear_predict_ = GET_PARAM(2);
    memset(src_, 0, kSrcSize);
    memset(dst_, 0, kDstSize);
    memset(dst_c_, 0, kDstSize);
  }

  int width_;
  int height_;
  BilinearPredictFunc bilinear_predict_;
  // The src stores the macroblock we will filter on, and makes it 1 byte larger
  // in order to test unaligned access. The result is stored in dst and dst_c(c
  // reference code result).
  static uint8_t *src_;
  static uint8_t *dst_;
  static uint8_t *dst_c_;
};

uint8_t *BilinearPredictTest::src_ = NULL;
uint8_t *BilinearPredictTest::dst_ = NULL;
uint8_t *BilinearPredictTest::dst_c_ = NULL;

using libvpx_test::ACMRandom;

TEST_P(BilinearPredictTest, TestWithRandomData) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  for (int i = 0; i < kSrcSize; ++i) src_[i] = rnd.Rand8();

  // Run tests for all possible offsets.
  for (int xoffset = 0; xoffset < 8; ++xoffset) {
    for (int yoffset = 0; yoffset < 8; ++yoffset) {
      // Call 16x16 c reference function. Each pixel is subjected to the same
      // transformation no matter the size of the transform. Only compare the
      // affected area.
      vp8_bilinear_predict16x16_c(&src_[kSrcStride * 2 + 2], kSrcStride,
                                  xoffset, yoffset, dst_c_, kDstStride);

      ASM_REGISTER_STATE_CHECK(bilinear_predict_(&src_[kSrcStride * 2 + 2],
                                                 kSrcStride, xoffset, yoffset,
                                                 dst_, kDstStride));

      for (int i = 0; i < height_; ++i) {
        for (int j = 0; j < width_; ++j)
          ASSERT_EQ(dst_c_[i * kDstStride + j], dst_[i * kDstStride + j])
              << "i==" << (i * width_ + j);
      }

      // For 4x4 move start and end points to next pixel to test if the function
      // reads unaligned data correctly.
      if (height_ == 4 && width_ == 4) {
        vp8_bilinear_predict16x16_c(&src_[kSrcStride * 2 + 2 + 1], kSrcStride,
                                    xoffset, yoffset, dst_c_ + 1, kDstStride);

        ASM_REGISTER_STATE_CHECK(
            bilinear_predict_(&src_[kSrcStride * 2 + 2 + 1], kSrcStride,
                              xoffset, yoffset, dst_ + 1, kDstStride));

        for (int i = 0; i < height_; ++i) {
          for (int j = 0; j < width_; ++j)
            ASSERT_EQ(dst_c_[i * kDstStride + j + 1],
                      dst_[i * kDstStride + j + 1])
                << "i==" << (i * width_ + j);
        }
      }
    }
  }
}

using std::tr1::make_tuple;

INSTANTIATE_TEST_CASE_P(
    C, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_c),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_c),
                      make_tuple(8, 4, &vp8_bilinear_predict8x4_c),
                      make_tuple(4, 4, &vp8_bilinear_predict4x4_c)));
#if HAVE_NEON
INSTANTIATE_TEST_CASE_P(
    NEON, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_neon),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_neon),
                      make_tuple(8, 4, &vp8_bilinear_predict8x4_neon),
                      make_tuple(4, 4, &vp8_bilinear_predict4x4_neon)));
#endif
#if HAVE_MMX
INSTANTIATE_TEST_CASE_P(
    MMX, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_mmx),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_mmx),
                      make_tuple(8, 4, &vp8_bilinear_predict8x4_mmx),
                      make_tuple(4, 4, &vp8_bilinear_predict4x4_mmx)));
#endif
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_sse2),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_sse2)));
#endif
#if HAVE_SSSE3
// TODO(johannkoneig): Fix ssse3 code or test.
// https://bugs.chromium.org/p/webm/issues/detail?id=1287
INSTANTIATE_TEST_CASE_P(
    DISABLED_SSSE3, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_ssse3),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_ssse3)));
#endif
#if HAVE_MSA
INSTANTIATE_TEST_CASE_P(
    MSA, BilinearPredictTest,
    ::testing::Values(make_tuple(16, 16, &vp8_bilinear_predict16x16_msa),
                      make_tuple(8, 8, &vp8_bilinear_predict8x8_msa),
                      make_tuple(8, 4, &vp8_bilinear_predict8x4_msa),
                      make_tuple(4, 4, &vp8_bilinear_predict4x4_msa)));
#endif
}  // namespace
