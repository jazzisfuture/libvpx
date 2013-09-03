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

extern "C" {
#include "vp9/common/vp9_entropy.h"
#include "./vp9_rtcd.h"
}

#include "test/acm_random.h"
#include "vpx/vpx_integer.h"

using libvpx_test::ACMRandom;

namespace {
#ifdef _MSC_VER
static int round(double x) {
  if (x < 0)
    return (int)ceil(x - 0.5);
  else
    return (int)floor(x + 0.5);
}
#endif

const double kPi = 3.141592653589793238462643383279502884;
void reference_32x32_dct_1d(double in[32], double out[32], int stride) {
  const double kInvSqrt2 = 0.707106781186547524400844362104;
  for (int k = 0; k < 32; k++) {
    out[k] = 0.0;
    for (int n = 0; n < 32; n++)
      out[k] += in[n] * cos(kPi * (2 * n + 1) * k / 64.0);
    if (k == 0)
      out[k] = out[k] * kInvSqrt2;
  }
}

void reference_32x32_dct_2d(int16_t input[32*32], double output[32*32]) {
  // First transform columns
  for (int i = 0; i < 32; ++i) {
    double temp_in[32], temp_out[32];
    for (int j = 0; j < 32; ++j)
      temp_in[j] = input[j*32 + i];
    reference_32x32_dct_1d(temp_in, temp_out, 1);
    for (int j = 0; j < 32; ++j)
      output[j * 32 + i] = temp_out[j];
  }
  // Then transform rows
  for (int i = 0; i < 32; ++i) {
    double temp_in[32], temp_out[32];
    for (int j = 0; j < 32; ++j)
      temp_in[j] = output[j + i*32];
    reference_32x32_dct_1d(temp_in, temp_out, 1);
    // Scale by some magic number
    for (int j = 0; j < 32; ++j)
      output[j + i * 32] = temp_out[j] / 4;
  }
}

TEST(VP9Idct32x32Test, AccuracyCheck) {
}


class Trans32x32Test : public ::testing::TestWithParam<int> {
 public:
  virtual ~Trans32x32Test() {}
  virtual void SetUp() {
    version_ = GetParam();
    if (version_ == 0) {
      fwd_txfm_ = vp9_short_fdct32x32_c;
      inv_txfm_ = vp9_short_idct32x32_add_c;
    } else if (version_ == 1) {
      fwd_txfm_ = vp9_short_fdct32x32;
      inv_txfm_ = vp9_short_idct32x32_add;
    } else if (version_ == 2) {
      fwd_txfm_ = vp9_short_fdct32x32_rd_c;
      inv_txfm_ = vp9_short_idct32x32_add;
    } else {
      fwd_txfm_ = vp9_short_fdct32x32_rd;
      inv_txfm_ = vp9_short_idct32x32_add;
    }
  }

 protected:
  int version_;
  void (*fwd_txfm_)(int16_t* in, int16_t* out, int stride);
  void (*inv_txfm_)(int16_t* in, uint8_t* dst, int stride);
};


TEST_P(Trans32x32Test, AccuracyCheck) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  unsigned int max_error = 0;
  int64_t total_error = 0;
  const int count_test_block = 1000;
  DECLARE_ALIGNED_ARRAY(16, int16_t, test_input_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, test_temp_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, dst, 1024);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, src, 1024);

  for (int i = 0; i < count_test_block; ++i) {
    for (int j = 0; j < 1024; ++j) {
      src[j] = rnd.Rand8();
      dst[j] = rnd.Rand8();
    }
    // Initialize a test block with input range [-255, 255].
    for (int j = 0; j < 1024; ++j)
      test_input_block[j] = src[j] - dst[j];

    const int pitch = 64;
    fwd_txfm_(test_input_block, test_temp_block, pitch);
    inv_txfm_(test_temp_block, dst, 32);

    for (int j = 0; j < 1024; ++j) {
      const unsigned diff = dst[j] - src[j];
      const unsigned error = diff * diff;
      if (max_error < error)
        max_error = error;
      total_error += error;
    }
  }

  if (version_ >= 2) {
    max_error /= 1;
    total_error /= 45;
  }

  EXPECT_GE(1u, max_error)
      << "Error: 32x32 FDCT/IDCT has an individual round-trip error > 1";

  EXPECT_GE(count_test_block, total_error)
      << "Error: 32x32 FDCT/IDCT has average round-trip error > 1 per block";
}

TEST_P(Trans32x32Test, CoeffCheck) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  const int count_test_block = 1000;

  DECLARE_ALIGNED_ARRAY(16, int16_t, input_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, output_ref_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, output_block, 1024);

  for (int i = 0; i < count_test_block; ++i) {
    for (int j = 0; j < 1024; ++j)
      input_block[j] = rnd.Rand8() - rnd.Rand8();

    const int pitch = 64;
    vp9_short_fdct32x32_c(input_block, output_ref_block, pitch);
    vp9_short_fdct32x32(input_block, output_block, pitch);

    for (int j = 0; j < 1024; ++j)
      EXPECT_EQ(output_block[j], output_ref_block[j])
        << "Error: 32x32 FDCT versions have mismatched coefficients";
  }
}

TEST_P(Trans32x32Test, MemCheck) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  const int count_test_block = 2000;

  DECLARE_ALIGNED_ARRAY(16, int16_t, input_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, input_extreme_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, output_ref_block, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, output_block, 1024);

  for (int i = 0; i < count_test_block; ++i) {
    // Initialize a test block with input range [-255, 255].
    for (int j = 0; j < 1024; ++j) {
      input_block[j] = rnd.Rand8() - rnd.Rand8();
      input_extreme_block[j] = rnd.Rand8() % 2 ? 255 : -255;
    }
    if (i == 0)
      for (int j = 0; j < 1024; ++j)
        input_extreme_block[j] = 255;
    if (i == 1)
      for (int j = 0; j < 1024; ++j)
        input_extreme_block[j] = -255;

    const int pitch = 64;
    vp9_short_fdct32x32_c(input_extreme_block, output_ref_block, pitch);
    vp9_short_fdct32x32(input_extreme_block, output_block, pitch);

    // The minimum quant value is 4.
    for (int j = 0; j < 1024; ++j) {
      EXPECT_EQ(output_block[j], output_ref_block[j])
          << "Error: 32x32 FDCT versions have mismatched coefficients";
      EXPECT_GE(4*DCT_MAX_VALUE, abs(output_ref_block[j]))
          << "Error: 32x32 FDCT C has coefficient larger than 4*DCT_MAX_VALUE";
      EXPECT_GE(4*DCT_MAX_VALUE, abs(output_block[j]))
          << "Error: 32x32 FDCT has coefficient larger than "
             "4*DCT_MAX_VALUE";
    }
  }
}

TEST_P(Trans32x32Test, InverseAccuracy) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  const int count_test_block = 1000;
  DECLARE_ALIGNED_ARRAY(16, int16_t, in, 1024);
  DECLARE_ALIGNED_ARRAY(16, int16_t, coeff, 1024);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, dst, 1024);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, src, 1024);

  for (int i = 0; i < count_test_block; ++i) {
    double out_r[1024];

    for (int j = 0; j < 1024; ++j) {
      src[j] = rnd.Rand8();
      dst[j] = rnd.Rand8();
    }
    // Initialize a test block with input range [-255, 255].
    for (int j = 0; j < 1024; ++j)
      in[j] = src[j] - dst[j];

    reference_32x32_dct_2d(in, out_r);
    for (int j = 0; j < 1024; j++)
      coeff[j] = round(out_r[j]);
    inv_txfm_(coeff, dst, 32);
    for (int j = 0; j < 1024; ++j) {
      int diff = dst[j] - src[j];
      int error = diff * diff;
      if (version_ >= 2)
        error /= 1;
      EXPECT_GE(1, error)
          << "Error: 32x32 IDCT has error " << error
          << " at index " << j;
    }
  }
}

INSTANTIATE_TEST_CASE_P(VP9, Trans32x32Test, ::testing::Range(0, 4));
}  // namespace
