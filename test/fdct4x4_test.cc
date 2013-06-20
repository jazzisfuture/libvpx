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
#include "vp9_rtcd.h"
}

#include "acm_random.h"
#include "vpx/vpx_integer.h"

using libvpx_test::ACMRandom;

namespace {
class test_arg {
 public:
  test_arg(int16_t *in, int16_t *out, uint8_t *dst, int stride, int n):
    input(in),
    output(out),
    rec(dst),
    pitch(stride),
    tx_type(n) {}

  ~test_arg() {}
 public:
  int16_t *input;
  int16_t *output;
  uint8_t *rec;
  int pitch;
  int tx_type;
};

void fdct4x4(test_arg &arg) {
  vp9_short_fdct4x4_c(arg.input, arg.output, arg.pitch);
}
void idct4x4_add(test_arg &arg) {
  vp9_short_idct4x4_add_c(arg.output, arg.rec, arg.pitch >> 1);
}
void fht4x4(test_arg &arg) {
  vp9_short_fht4x4_c(arg.input, arg.output, arg.pitch >> 1, arg.tx_type);
}
void iht4x4_add(test_arg &arg) {
  vp9_short_iht4x4_add_c(arg.output, arg.rec, arg.pitch >> 1, arg.tx_type);
}

class FwdTrans4x4Test : public ::testing::TestWithParam<int> {
 public:
  FwdTrans4x4Test(void (*fwd_func)(test_arg &), void (*inv_func)(test_arg &)):
    fwd_txfm(fwd_func),
    inv_txfm(inv_func){}
  FwdTrans4x4Test() { SetUpTestTxfm(); }
  ~FwdTrans4x4Test() {}

  void SetUpTestTxfm() {
    tx_type = GetParam();
    if (tx_type == 0) {
      fwd_txfm = fdct4x4;
      inv_txfm = idct4x4_add;
    } else {
      fwd_txfm = fht4x4;
      inv_txfm = iht4x4_add;
    }
  }

 protected:
  void RunFwdTxfm (test_arg &args) {
    (*fwd_txfm)(args);
  }

  void RunInvTxfm (test_arg &args) {
    (*inv_txfm)(args);
  }

  int tx_type;
  void (*fwd_txfm)(test_arg &arg);
  void (*inv_txfm)(test_arg &arg);
};

TEST_P(FwdTrans4x4Test, SignBiasCheck) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  int16_t test_input_block[16];
  int16_t test_output_block[16];
  const int pitch = 8;
  int count_sign_block[16][2];
  const int count_test_block = 1000000;

  memset(count_sign_block, 0, sizeof(count_sign_block));
  for (int i = 0; i < count_test_block; ++i) {
    // Initialize a test block with input range [-255, 255].
    for (int j = 0; j < 16; ++j)
      test_input_block[j] = rnd.Rand8() - rnd.Rand8();

    test_arg args(test_input_block, test_output_block, NULL, pitch, tx_type);
    RunFwdTxfm(args);

    for (int j = 0; j < 16; ++j) {
      if (test_output_block[j] < 0)
        ++count_sign_block[j][0];
      else if (test_output_block[j] > 0)
        ++count_sign_block[j][1];
    }
  }

  for (int j = 0; j < 16; ++j) {
    const bool bias_acceptable = (abs(count_sign_block[j][0] -
                                      count_sign_block[j][1]) < 10000);
    EXPECT_TRUE(bias_acceptable)
        << "Error: 4x4 FDCT/FHT has a sign bias > 1%"
        << " for input range [-255, 255] at index " << j
        << " tx_type " << tx_type;
  }

  memset(count_sign_block, 0, sizeof(count_sign_block));
  for (int i = 0; i < count_test_block; ++i) {
    // Initialize a test block with input range [-15, 15].
    for (int j = 0; j < 16; ++j)
      test_input_block[j] = (rnd.Rand8() >> 4) - (rnd.Rand8() >> 4);

    test_arg args(test_input_block, test_output_block, NULL, pitch, tx_type);
    RunFwdTxfm(args);

    for (int j = 0; j < 16; ++j) {
      if (test_output_block[j] < 0)
        ++count_sign_block[j][0];
      else if (test_output_block[j] > 0)
        ++count_sign_block[j][1];
    }
  }

  for (int j = 0; j < 16; ++j) {
    const bool bias_acceptable = (abs(count_sign_block[j][0] -
                                      count_sign_block[j][1]) < 100000);
    EXPECT_TRUE(bias_acceptable)
        << "Error: 4x4 FDCT/FHT has a sign bias > 10%"
        << " for input range [-15, 15] at index " << j;
  }
}

TEST_P(FwdTrans4x4Test, RoundTripErrorCheck) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());

  int max_error = 0;
  double total_error = 0;
  const int count_test_block = 1000000;
  for (int i = 0; i < count_test_block; ++i) {
    int16_t test_input_block[16];
    int16_t test_temp_block[16];
    uint8_t dst[16], src[16];

    for (int j = 0; j < 16; ++j) {
      src[j] = rnd.Rand8();
      dst[j] = rnd.Rand8();
    }
    // Initialize a test block with input range [-255, 255].
    for (int j = 0; j < 16; ++j)
      test_input_block[j] = src[j] - dst[j];

    const int pitch = 8;
    test_arg args(test_input_block, test_temp_block, dst, pitch, tx_type);
    RunFwdTxfm(args);

    for (int j = 0; j < 16; ++j) {
        if(test_temp_block[j] > 0) {
          test_temp_block[j] += 2;
          test_temp_block[j] /= 4;
          test_temp_block[j] *= 4;
        } else {
          test_temp_block[j] -= 2;
          test_temp_block[j] /= 4;
          test_temp_block[j] *= 4;
        }
    }

    // inverse transform and reconstruct the pixel block
    RunInvTxfm(args);

    for (int j = 0; j < 16; ++j) {
      const int diff = dst[j] - src[j];
      const int error = diff * diff;
      if (max_error < error)
        max_error = error;
      total_error += error;
    }
  }
  EXPECT_GE(1, max_error)
      << "Error: FDCT/IDCT or FHT/IHT has an individual roundtrip error > 1";

  EXPECT_GE(count_test_block, total_error)
      << "Error: FDCT/IDCT or FHT/IHT has average "
          "roundtrip error > 1 per block";
}

INSTANTIATE_TEST_CASE_P(VP9, FwdTrans4x4Test, ::testing::Range(0, 4));
}  // namespace
