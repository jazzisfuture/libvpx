/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits.h> /* INT16_MIN/MAX */

#include "test/acm_random.h"
#include "test/register_state_check.h"
#include "test/util.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

extern "C" {
#include "./vpx_config.h"
#include "./vp9_rtcd.h"
#include "vp9/common/vp9_common.h" /* clip_pixel */
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
}

namespace {
typedef void (*add_constant_residual_fn_t)(const int16_t diff, uint8_t *dest,
                                           int stride);

void add_constant_residual(const int16_t diff, uint8_t *dest, int stride,
                                  int width, int height) {
  int r, c;

  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++)
      dest[c] = clip_pixel(diff + dest[c]);

    dest += stride;
  }
}

class AddConstantResidualTest : public PARAMS(int, int, const add_constant_residual_fn_t) {
 public:
  static void SetUpTestCase() {
    // Force output to be 16 byte aligned.
    output_ = reinterpret_cast<uint8_t*>(
        vpx_memalign(kDataAlignment, kOutputBufferSize));
    reference_ = reinterpret_cast<uint8_t*>(
        vpx_memalign(kDataAlignment, kOutputBufferSize));
  }

  static void TearDownTestCase() {
    vpx_free(output_);
    output_ = NULL;
    vpx_free(reference_);
    reference_ = NULL;
  }

 protected:
  static const int kDataAlignment = 16;
  static const int kOuterBlockSize = 128;
  static const int kInputStride = kOuterBlockSize;
  static const int kOutputStride = kOuterBlockSize;
  static const int kMaxDimension = 32;
  static const int kInputBufferSize = kOuterBlockSize * kOuterBlockSize;
  static const int kOutputBufferSize = kOuterBlockSize * kOuterBlockSize;

  int Width() const { return GET_PARAM(0); }
  int Height() const { return GET_PARAM(1); }
  int BorderLeft() const {
    const int center = (kOuterBlockSize - Width()) / 2;
    return (center + (kDataAlignment - 1)) & ~(kDataAlignment - 1);
  }
  int BorderTop() const { return (kOuterBlockSize - Height()) / 2; }

  bool IsIndexInBorder(int i) {
    return (i < BorderTop() * kOuterBlockSize ||
            i >= (BorderTop() + Height()) * kOuterBlockSize ||
            i % kOuterBlockSize < BorderLeft() ||
            i % kOuterBlockSize >= (BorderLeft() + Width()));
  }

  virtual void SetUp() {
    /* Set up guard blocks for an inner block centered in the outer block */
    ::libvpx_test::ACMRandom prng;
    for (int i = 0; i < kOutputBufferSize; ++i) {
      if (IsIndexInBorder(i))
        reference_[i] = output_[i] = 255;
      else
        reference_[i] = output_[i] = prng.Rand8Extremes();
    }
  }

  void CheckGuardBlocks() {
    for (int i = 0; i < kOutputBufferSize; ++i) {
      if (IsIndexInBorder(i)) {
        EXPECT_EQ(255, output_[i]);
        EXPECT_EQ(255, reference_[i]);
      }
    }
  }

  uint8_t* output() const {
    return output_ + BorderTop() * kOuterBlockSize + BorderLeft();
  }

  uint8_t* reference() const {
    return reference_ + BorderTop() * kOuterBlockSize + BorderLeft();
  }

  static uint8_t* output_;
  static uint8_t* reference_;
};
uint8_t* AddConstantResidualTest::output_ = NULL;
uint8_t* AddConstantResidualTest::reference_ = NULL;

TEST_P(AddConstantResidualTest, AllDiff) {
  uint8_t* const out = output();
  uint8_t* const ref = reference(); 
  int16_t diff;

  for (diff = SHRT_MIN; diff < SHRT_MAX; diff++) {

    SetUp();

    add_constant_residual(diff, ref, kOutputStride, Width(), Height());

    REGISTER_STATE_CHECK(GET_PARAM(2)(diff, out, kOutputStride));

    CheckGuardBlocks();

    for (int y = 0; y < Height(); ++y)
      for (int x = 0; x < Width(); ++x)
        ASSERT_EQ(ref[y * kOutputStride + x], out[y * kOutputStride + x])
            << "mismatch with diff " << diff << " at (" << x << "," << y << "), ";
  }
}

using std::tr1::make_tuple;

const add_constant_residual_fn_t add_constant_residual_8x8_c =
    vp9_add_constant_residual_8x8_c;
const add_constant_residual_fn_t add_constant_residual_16x16_c =
    vp9_add_constant_residual_16x16_c;
const add_constant_residual_fn_t add_constant_residual_32x32_c =
    vp9_add_constant_residual_32x32_c;
INSTANTIATE_TEST_CASE_P(C, AddConstantResidualTest, ::testing::Values(
    make_tuple(8, 8, add_constant_residual_8x8_c),
    make_tuple(16, 16, add_constant_residual_16x16_c),
    make_tuple(32, 32, add_constant_residual_32x32_c)));

#if HAVE_SSE2
const add_constant_residual_fn_t add_constant_residual_8x8_sse2 =
    vp9_add_constant_residual_8x8_sse2;
const add_constant_residual_fn_t add_constant_residual_16x16_sse2 =
    vp9_add_constant_residual_16x16_sse2;
const add_constant_residual_fn_t add_constant_residual_32x32_sse2 =
    vp9_add_constant_residual_32x32_sse2;
INSTANTIATE_TEST_CASE_P(SSE2, AddConstantResidualTest, ::testing::Values(
    make_tuple(8, 8, add_constant_residual_8x8_sse2),
    make_tuple(16, 16, add_constant_residual_16x16_sse2),
    make_tuple(32, 32, add_constant_residual_32x32_sse2)));
#endif

#if HAVE_NEON
const add_constant_residual_fn_t add_constant_residual_8x8_neon =
    vp9_add_constant_residual_8x8_neon;
const add_constant_residual_fn_t add_constant_residual_16x16_neon =
    vp9_add_constant_residual_16x16_neon;
const add_constant_residual_fn_t add_constant_residual_32x32_neon =
    vp9_add_constant_residual_32x32_neon;
INSTANTIATE_TEST_CASE_P(NEON, AddConstantResidualTest, ::testing::Values(
    make_tuple(8, 8, add_constant_residual_8x8_neon),
    make_tuple(16, 16, add_constant_residual_16x16_neon),
    make_tuple(32, 32, add_constant_residual_32x32_neon)));
#endif
}  // namespace
