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

#include "./vp9_rtcd.h"
#include "vp9/common/vp9_entropy.h"
#include "vpx/vpx_integer.h"

extern "C" {
void vp9_idct8x8_64_add_c(const tran_low_t *input, uint8_t *output, int pitch);
}

const int kNumCoeffs = 64;
const double kPi = 3.141592653589793238462643383279502884;
void reference_8x8_dct_1d(const double in[8], double out[8], int stride) {
  const double kInvSqrt2 = 0.707106781186547524400844362104;
  for (int k = 0; k < 8; k++) {
    out[k] = 0.0;
    for (int n = 0; n < 8; n++)
      out[k] += in[n] * cos(kPi * (2 * n + 1) * k / 16.0);
    if (k == 0)
      out[k] = out[k] * kInvSqrt2;
  }
}

void reference_8x8_dct_2d(const int16_t input[kNumCoeffs],
                            double output[kNumCoeffs]) {
  // First transform columns
  for (int i = 0; i < 8; ++i) {
    double temp_in[8], temp_out[8];
    for (int j = 0; j < 8; ++j)
      temp_in[j] = input[j*8 + i];
    reference_8x8_dct_1d(temp_in, temp_out, 1);
    for (int j = 0; j < 8; ++j)
      output[j * 8 + i] = temp_out[j];
  }
  // Then transform rows
  for (int i = 0; i < 8; ++i) {
    double temp_in[8], temp_out[8];
    for (int j = 0; j < 8; ++j)
      temp_in[j] = output[j + i*8];
    reference_8x8_dct_1d(temp_in, temp_out, 1);
    // Scale by some magic number
    for (int j = 0; j < 8; ++j)
      output[j + i * 8] = temp_out[j] * 2;
  }
}

using libvpx_test::ACMRandom;

namespace {
<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
typedef void (*fdct_t)(const int16_t *in, tran_low_t *out, int stride);
typedef void (*idct_t)(const tran_low_t *in, uint8_t *out, int stride);
typedef void (*fht_t) (const int16_t *in, tran_low_t *out, int stride,
                       int tx_type);
typedef void (*iht_t) (const tran_low_t *in, uint8_t *out, int stride,
                       int tx_type);
=======
typedef void (*FdctFunc)(const int16_t *in, int16_t *out, int stride);
typedef void (*IdctFunc)(const int16_t *in, uint8_t *out, int stride);
typedef void (*FhtFunc)(const int16_t *in, int16_t *out, int stride,
                        int tx_type);
typedef void (*IhtFunc)(const int16_t *in, uint8_t *out, int stride,
                        int tx_type);
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
typedef std::tr1::tuple<fdct_t, idct_t, int, int> dct_8x8_param_t;
typedef std::tr1::tuple<fht_t, iht_t, int, int> ht_8x8_param_t;
=======
typedef std::tr1::tuple<FdctFunc, IdctFunc, int> Dct8x8Param;
typedef std::tr1::tuple<FhtFunc, IhtFunc, int> Ht8x8Param;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

void fdct8x8_ref(const int16_t *in, tran_low_t *out, int stride, int tx_type) {
  vp9_fdct8x8_c(in, out, stride);
}

void fht8x8_ref(const int16_t *in, tran_low_t *out, int stride, int tx_type) {
  vp9_fht8x8_c(in, out, stride, tx_type);
}

#if CONFIG_VP9_HIGH

void idct8x8_10(const tran_low_t *in, uint8_t *out, int stride) {
  vp9_high_idct8x8_64_add_c(in, out, stride, 10);
}

void idct8x8_12(const tran_low_t *in, uint8_t *out, int stride) {
  vp9_high_idct8x8_64_add_c(in, out, stride, 12);
}

void iht8x8_10(const tran_low_t *in, uint8_t *out, int stride, int tx_type) {
  vp9_high_iht8x8_64_add_c(in, out, stride, tx_type, 10);
}

void iht8x8_12(const tran_low_t *in, uint8_t *out, int stride, int tx_type) {
  vp9_high_iht8x8_64_add_c(in, out, stride, tx_type, 12);
}

#endif

class FwdTrans8x8TestBase {
 public:
  virtual ~FwdTrans8x8TestBase() {}

 protected:
  virtual void RunFwdTxfm(int16_t *in, tran_low_t *out, int stride) = 0;
  virtual void RunInvTxfm(tran_low_t *out, uint8_t *dst, int stride) = 0;

  void RunSignBiasCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    DECLARE_ALIGNED_ARRAY(16, int16_t, test_input_block, 64);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, test_output_block, 64);
    int count_sign_block[64][2];
    const int count_test_block = 100000;

    memset(count_sign_block, 0, sizeof(count_sign_block));

    for (int i = 0; i < count_test_block; ++i) {
      // Initialize a test block with input range [-255, 255].
      for (int j = 0; j < 64; ++j)
        test_input_block[j] = ((rnd.Rand16() >> (16 - bit_depth_)) & mask_) -
                              ((rnd.Rand16() >> (16 - bit_depth_)) & mask_);
      ASM_REGISTER_STATE_CHECK(
          RunFwdTxfm(test_input_block, test_output_block, pitch_));

      for (int j = 0; j < 64; ++j) {
        if (test_output_block[j] < 0)
          ++count_sign_block[j][0];
        else if (test_output_block[j] > 0)
          ++count_sign_block[j][1];
      }
    }

    for (int j = 0; j < 64; ++j) {
      const int diff = abs(count_sign_block[j][0] - count_sign_block[j][1]);
      const int max_diff = 1125;
      EXPECT_LT(diff, max_diff << (bit_depth_ - 8))
          << "Error: 8x8 FDCT/FHT has a sign bias > "
          << 1. * max_diff / count_test_block * 100 << "%"
          << " for input range [-255, 255] at index " << j
          << " count0: " << count_sign_block[j][0]
          << " count1: " << count_sign_block[j][1]
          << " diff: " << diff;
    }

    memset(count_sign_block, 0, sizeof(count_sign_block));

    for (int i = 0; i < count_test_block; ++i) {
      // Initialize a test block with input range [-15, 15].
      for (int j = 0; j < 64; ++j)
        test_input_block[j] = ((rnd.Rand16() & mask_) >> 4) -
                              ((rnd.Rand16() & mask_) >> 4);
      ASM_REGISTER_STATE_CHECK(
          RunFwdTxfm(test_input_block, test_output_block, pitch_));

      for (int j = 0; j < 64; ++j) {
        if (test_output_block[j] < 0)
          ++count_sign_block[j][0];
        else if (test_output_block[j] > 0)
          ++count_sign_block[j][1];
      }
    }

    for (int j = 0; j < 64; ++j) {
      const int diff = abs(count_sign_block[j][0] - count_sign_block[j][1]);
      const int max_diff = 10000;
      EXPECT_LT(diff, max_diff << (bit_depth_ - 8))
          << "Error: 4x4 FDCT/FHT has a sign bias > "
          << 1. * max_diff / count_test_block * 100 << "%"
          << " for input range [-15, 15] at index " << j
          << " count0: " << count_sign_block[j][0]
          << " count1: " << count_sign_block[j][1]
          << " diff: " << diff;
    }
  }

  void RunRoundTripErrorCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    int max_error = 0;
    int total_error = 0;
    const int count_test_block = 100000;
    DECLARE_ALIGNED_ARRAY(16, int16_t, test_input_block, 64);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, test_temp_block, 64);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, dst, 64);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, dst16, 64);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, src, 64);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, src16, 64);

    for (int i = 0; i < count_test_block; ++i) {
      // Initialize a test block with input range [-mask_, mask_].
      for (int j = 0; j < 64; ++j) {
        if (bit_depth_ == 8) {
          src[j] = rnd.Rand8();
          dst[j] = rnd.Rand8();
          test_input_block[j] = src[j] - dst[j];
        } else {
          src16[j] = rnd.Rand16() & mask_;
          dst16[j] = rnd.Rand16() & mask_;
          test_input_block[j] = src16[j] - dst16[j];
        }
      }

      ASM_REGISTER_STATE_CHECK(
          RunFwdTxfm(test_input_block, test_temp_block, pitch_));
      for (int j = 0; j < 64; ++j) {
          if (test_temp_block[j] > 0) {
            test_temp_block[j] += 2;
            test_temp_block[j] /= 4;
            test_temp_block[j] *= 4;
          } else {
            test_temp_block[j] -= 2;
            test_temp_block[j] /= 4;
            test_temp_block[j] *= 4;
          }
      }
      if (bit_depth_ == 8)
        ASM_REGISTER_STATE_CHECK(
            RunInvTxfm(test_temp_block, dst, pitch_));
#if CONFIG_VP9_HIGH
      else
        ASM_REGISTER_STATE_CHECK(
            RunInvTxfm(test_temp_block, CONVERT_TO_BYTEPTR(dst16), pitch_));
#endif

      for (int j = 0; j < 64; ++j) {
        const int diff = bit_depth_ == 8 ? dst[j] - src[j] :
                                           dst16[j] - src16[j];
        const int error = diff * diff;
        if (max_error < error)
          max_error = error;
        total_error += error;
      }
    }

    EXPECT_GE(1 << 2 * (bit_depth_ - 8), max_error)
      << "Error: 8x8 FDCT/IDCT or FHT/IHT has an individual"
      << " roundtrip error > 1";

    EXPECT_GE((count_test_block << 2 * (bit_depth_ - 8))/5, total_error)
      << "Error: 8x8 FDCT/IDCT or FHT/IHT has average roundtrip "
      << "error > 1/5 per block";
  }

  void RunExtremalCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    int max_error = 0;
    int total_error = 0;
    int total_coeff_error = 0;
    const int count_test_block = 100000;
    DECLARE_ALIGNED_ARRAY(16, int16_t, test_input_block, 64);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, test_temp_block, 64);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, ref_temp_block, 64);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, dst, 64);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, dst16, 64);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, src, 64);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, src16, 64);

    for (int i = 0; i < count_test_block; ++i) {
      // Initialize a test block with input range [-mask_, mask_].
      for (int j = 0; j < 64; ++j) {
        if (bit_depth_ == 8) {
          if (i == 0) {
            src[j] = 255;
            dst[j] = 0;
          } else if (i == 1) {
            src[j] = 0;
            dst[j] = 255;
          } else {
            src[j] = rnd.Rand8() % 2 ? 255 : 0;
            dst[j] = rnd.Rand8() % 2 ? 255 : 0;
          }

          test_input_block[j] = src[j] - dst[j];
        } else {
          if (i == 0) {
            src16[j] = mask_;
            dst16[j] = 0;
          } else if (i == 1) {
            src16[j] = 0;
            dst16[j] = mask_;
          } else {
            src16[j] = rnd.Rand8() % 2 ? mask_ : 0;
            dst16[j] = rnd.Rand8() % 2 ? mask_ : 0;
          }

          test_input_block[j] = src16[j] - dst16[j];
        }
      }

      ASM_REGISTER_STATE_CHECK(
          RunFwdTxfm(test_input_block, test_temp_block, pitch_));
      ASM_REGISTER_STATE_CHECK(
          fwd_txfm_ref(test_input_block, ref_temp_block, pitch_, tx_type_));
      if (bit_depth_ == 8)
        ASM_REGISTER_STATE_CHECK(
            RunInvTxfm(test_temp_block, dst, pitch_));
#if CONFIG_VP9_HIGH
      else
        ASM_REGISTER_STATE_CHECK(
            RunInvTxfm(test_temp_block, CONVERT_TO_BYTEPTR(dst16), pitch_));
#endif

      for (int j = 0; j < 64; ++j) {
        const int diff = bit_depth_ == 8 ? dst[j] - src[j] :
                                           dst16[j] - src16[j];
        const int error = diff * diff;
        if (max_error < error)
          max_error = error;
        total_error += error;

        const int coeff_diff = test_temp_block[j] - ref_temp_block[j];
        total_coeff_error += abs(coeff_diff);
      }

      EXPECT_GE(1 << 2 * (bit_depth_ - 8), max_error)
          << "Error: Extremal 8x8 FDCT/IDCT or FHT/IHT has"
          << "an individual roundtrip error > 1";

      EXPECT_GE((count_test_block << 2 * (bit_depth_ - 8))/5, total_error)
          << "Error: Extremal 8x8 FDCT/IDCT or FHT/IHT has average"
          << " roundtrip error > 1/5 per block";

      EXPECT_EQ(0, total_coeff_error)
          << "Error: Extremal 8x8 FDCT/FHT has"
          << "overflow issues in the intermediate steps > 1";
    }
  }

  void RunInvAccuracyCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    const int count_test_block = 1000;
    DECLARE_ALIGNED_ARRAY(16, int16_t, in, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, coeff, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, dst, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, dst16, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, uint8_t, src, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, uint16_t, src16, kNumCoeffs);

    for (int i = 0; i < count_test_block; ++i) {
      double out_r[kNumCoeffs];

      // Initialize a test block with input range [-255, 255].
      for (int j = 0; j < kNumCoeffs; ++j) {
        if (bit_depth_ == 8) {
          src[j] = rnd.Rand8() % 2 ? 255 : 0;
          dst[j] = src[j] > 0 ? 0 : 255;
          in[j] = src[j] - dst[j];
        } else {
          src16[j] = rnd.Rand8() % 2 ? mask_ : 0;
          dst16[j] = src16[j] > 0 ? 0 : mask_;
          in[j] = src16[j] - dst16[j];
        }
      }

      reference_8x8_dct_2d(in, out_r);
      for (int j = 0; j < kNumCoeffs; ++j)
        coeff[j] = round(out_r[j]);

      if (bit_depth_ == 8)
        ASM_REGISTER_STATE_CHECK(RunInvTxfm(coeff, dst, pitch_));
#if CONFIG_VP9_HIGH
      else
        ASM_REGISTER_STATE_CHECK(RunInvTxfm(coeff, CONVERT_TO_BYTEPTR(dst16),
                                            pitch_));
#endif

      for (int j = 0; j < kNumCoeffs; ++j) {
        const uint32_t diff = bit_depth_ == 8 ? dst[j] - src[j] :
                                                dst16[j] - src16[j];
        const uint32_t error = diff * diff;
        EXPECT_GE(1u << 2 * (bit_depth_ - 8), error)
            << "Error: 8x8 IDCT has error " << error
            << " at index " << j;
      }
    }
  }

  void RunFwdAccuracyCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    const int count_test_block = 1000;
    DECLARE_ALIGNED_ARRAY(16, int16_t, in, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, coeff_r, kNumCoeffs);
    DECLARE_ALIGNED_ARRAY(16, tran_low_t, coeff, kNumCoeffs);

    for (int i = 0; i < count_test_block; ++i) {
      double out_r[kNumCoeffs];

      // Initialize a test block with input range [-255, 255].
      for (int j = 0; j < kNumCoeffs; ++j) {
        in[j] = rnd.Rand8() % 2 == 0 ? mask_ : -mask_;
      }

      RunFwdTxfm(in, coeff, pitch_);
      reference_8x8_dct_2d(in, out_r);
      for (int j = 0; j < kNumCoeffs; ++j)
        coeff_r[j] = round(out_r[j]);

      for (int j = 0; j < kNumCoeffs; ++j) {
        const uint32_t diff = coeff[j] - coeff_r[j];
        const uint32_t error = diff * diff;
        EXPECT_GE(9u << 2 * (bit_depth_ - 8), error)
            << "Error: 8x8 DCT has error " << error
            << " at index " << j;
      }
    }
  }

  int pitch_;
  int tx_type_;
<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
  fht_t fwd_txfm_ref;
  int bit_depth_;
  int mask_;
=======
  FhtFunc fwd_txfm_ref;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")
};

class FwdTrans8x8DCT
    : public FwdTrans8x8TestBase,
      public ::testing::TestWithParam<Dct8x8Param> {
 public:
  virtual ~FwdTrans8x8DCT() {}

  virtual void SetUp() {
    fwd_txfm_  = GET_PARAM(0);
    inv_txfm_  = GET_PARAM(1);
    tx_type_   = GET_PARAM(2);
    bit_depth_ = GET_PARAM(3);
    pitch_     = 8;
    fwd_txfm_ref = fdct8x8_ref;
    mask_ = (1 << bit_depth_) - 1;
  }

  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  void RunFwdTxfm(int16_t *in, tran_low_t *out, int stride) {
    fwd_txfm_(in, out, stride);
  }
  void RunInvTxfm(tran_low_t *out, uint8_t *dst, int stride) {
    inv_txfm_(out, dst, stride);
  }

  FdctFunc fwd_txfm_;
  IdctFunc inv_txfm_;
};

TEST_P(FwdTrans8x8DCT, SignBiasCheck) {
  RunSignBiasCheck();
}

TEST_P(FwdTrans8x8DCT, RoundTripErrorCheck) {
  RunRoundTripErrorCheck();
}

TEST_P(FwdTrans8x8DCT, ExtremalCheck) {
  RunExtremalCheck();
}

TEST_P(FwdTrans8x8DCT, FwdAccuracyCheck) {
  RunFwdAccuracyCheck();
}

TEST_P(FwdTrans8x8DCT, InvAccuracyCheck) {
  RunInvAccuracyCheck();
}

class FwdTrans8x8HT
    : public FwdTrans8x8TestBase,
      public ::testing::TestWithParam<Ht8x8Param> {
 public:
  virtual ~FwdTrans8x8HT() {}

  virtual void SetUp() {
    fwd_txfm_  = GET_PARAM(0);
    inv_txfm_  = GET_PARAM(1);
    tx_type_   = GET_PARAM(2);
    bit_depth_ = GET_PARAM(3);
    pitch_     = 8;
    fwd_txfm_ref = fht8x8_ref;
    mask_ = (1 << bit_depth_) - 1;
  }

  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  void RunFwdTxfm(int16_t *in, tran_low_t *out, int stride) {
    fwd_txfm_(in, out, stride, tx_type_);
  }
  void RunInvTxfm(tran_low_t *out, uint8_t *dst, int stride) {
    inv_txfm_(out, dst, stride, tx_type_);
  }

  FhtFunc fwd_txfm_;
  IhtFunc inv_txfm_;
};

TEST_P(FwdTrans8x8HT, SignBiasCheck) {
  RunSignBiasCheck();
}

TEST_P(FwdTrans8x8HT, RoundTripErrorCheck) {
  RunRoundTripErrorCheck();
}

TEST_P(FwdTrans8x8HT, ExtremalCheck) {
  RunExtremalCheck();
}

using std::tr1::make_tuple;

#if CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    C, FwdTrans8x8DCT,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c, &vp9_idct8x8_64_add_c, 0, 8),
        make_tuple(&vp9_high_fdct8x8_c, &idct8x8_10, 0, 10),
        make_tuple(&vp9_high_fdct8x8_c, &idct8x8_12, 0, 12)));
#else
INSTANTIATE_TEST_CASE_P(
    C, FwdTrans8x8DCT,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_c, &vp9_idct8x8_64_add_c, 0, 8)));
#endif
#if CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    C, FwdTrans8x8HT,
    ::testing::Values(
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 0, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 1, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 2, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 3, 8),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_10, 0, 10),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_10, 1, 10),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_10, 2, 10),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_10, 3, 10),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_12, 0, 12),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_12, 1, 12),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_12, 2, 12),
        make_tuple(&vp9_high_fht8x8_c, &iht8x8_12, 3, 12)));
#else
INSTANTIATE_TEST_CASE_P(
    C, FwdTrans8x8HT,
    ::testing::Values(
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 0, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 1, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 2, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_c, 3, 8)));
#endif

#if HAVE_NEON_ASM && !CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    NEON, FwdTrans8x8DCT,
    ::testing::Values(
<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
        make_tuple(&vp9_fdct8x8_c, &vp9_idct8x8_64_add_neon, 0, 8)));
=======
        make_tuple(&vp9_fdct8x8_neon, &vp9_idct8x8_64_add_neon, 0)));
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")
INSTANTIATE_TEST_CASE_P(
    DISABLED_NEON, FwdTrans8x8HT,
    ::testing::Values(
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_neon, 0, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_neon, 1, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_neon, 2, 8),
        make_tuple(&vp9_fht8x8_c, &vp9_iht8x8_64_add_neon, 3, 8)));
#endif

#if HAVE_SSE2 && !CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    SSE2, FwdTrans8x8DCT,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_sse2, &vp9_idct8x8_64_add_sse2, 0, 8)));
INSTANTIATE_TEST_CASE_P(
    SSE2, FwdTrans8x8HT,
    ::testing::Values(
        make_tuple(&vp9_fht8x8_sse2, &vp9_iht8x8_64_add_sse2, 0, 8),
        make_tuple(&vp9_fht8x8_sse2, &vp9_iht8x8_64_add_sse2, 1, 8),
        make_tuple(&vp9_fht8x8_sse2, &vp9_iht8x8_64_add_sse2, 2, 8),
        make_tuple(&vp9_fht8x8_sse2, &vp9_iht8x8_64_add_sse2, 3, 8)));
#endif

#if HAVE_SSSE3 && ARCH_X86_64 && !CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    SSSE3, FwdTrans8x8DCT,
    ::testing::Values(
<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
        make_tuple(&vp9_fdct8x8_ssse3, &vp9_idct8x8_64_add_ssse3, 0, 8)));
#endif

#if HAVE_AVX2 && !CONFIG_VP9_HIGH
INSTANTIATE_TEST_CASE_P(
    AVX2, FwdTrans8x8DCT,
    ::testing::Values(
        make_tuple(&vp9_fdct8x8_avx2, &vp9_idct8x8_64_add_c, 0, 8)));
INSTANTIATE_TEST_CASE_P(
    AVX2, FwdTrans8x8HT,
    ::testing::Values(
        make_tuple(&vp9_fht8x8_avx2, &vp9_iht8x8_64_add_c, 0, 8),
        make_tuple(&vp9_fht8x8_avx2, &vp9_iht8x8_64_add_c, 1, 8),
        make_tuple(&vp9_fht8x8_avx2, &vp9_iht8x8_64_add_c, 2, 8),
        make_tuple(&vp9_fht8x8_avx2, &vp9_iht8x8_64_add_c, 3, 8)));
=======
        make_tuple(&vp9_fdct8x8_ssse3, &vp9_idct8x8_64_add_ssse3, 0)));
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")
#endif
}  // namespace
