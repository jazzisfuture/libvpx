/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"
#include <emmintrin.h>  // SSE2
#include "test/acm_random.h"
#include "test/util.h"
#include "vpx_dsp/vpx_dsp_common.h"

#define FDCT4x4_2D vpx_fdct4x4_sse2
#define FDCT8x8_2D vpx_fdct8x8_sse2
#define FDCT16x16_2D vpx_fdct16x16_sse2
#include "vpx_dsp/x86/fwd_txfm_impl_sse2.h"
#include "vp10/encoder/x86/dct_sse2.c"

using libvpx_test::ACMRandom;

//  DST 4x4 C reference function whose definition is in vp10/encoder/dct.c
void fdst4(const tran_low_t *input, tran_low_t *output);

//  The target DST 4x4 SSE2 function whose definition is
//  in vp10/encoder/x86/dct_sse2.c
void fdst4_sse2(__m128i *in);

namespace {

const int kNumCoeffs = 16;

typedef void (*FdstFunc)(const int16_t *in, tran_low_t *out);

void fdst4x4_ref(const int16_t *in, tran_low_t *out) {
  int i;
  int j;
  tran_low_t input[4];
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      input[j] = in[j * 4 + i];
    }
    fdst4(input, out);
    out += 4;
  }
}

void fdst4x4_sse2(const int16_t *in, tran_low_t *out) {
  __m128i input[4];
  DECLARE_ALIGNED(16, int16_t, output[kNumCoeffs]);

  input[0] = _mm_loadl_epi64((const __m128i *) (in + 0));
  input[1] = _mm_loadl_epi64((const __m128i *) (in + 4));
  input[2] = _mm_loadl_epi64((const __m128i *) (in + 8));
  input[3] = _mm_loadl_epi64((const __m128i *) (in + 12));

  fdst4_sse2(input);

  __m128i in01 = _mm_unpacklo_epi64(input[0], input[1]);
  __m128i in23 = _mm_unpacklo_epi64(input[2], input[3]);

  _mm_store_si128((__m128i *) (output + 0), in01);
  _mm_store_si128((__m128i *) (output + 8), in23);

  int i;
  for (i = 0; i < kNumCoeffs; ++i) {
    out[i] = output[i];
  }
}


class Dst4x4TestBase {
 public:
  virtual ~Dst4x4TestBase() {}

 private:
  void range_check(int16_t *in, unsigned int bits) {
    int16_t max = (1 << (bits - 1)) - 1;
    int16_t min = -(1 << (bits - 1));

    int16_t number = *in;
    if (number >= 0) {
      *in = number >= max ? max : number;
    } else {
      *in = number <= min ? min : number;
    }
  }

 protected:
  void RunBitsCheck() {
    ACMRandom rnd(ACMRandom::DeterministicSeed());
    const int count_test_block = 1000000;
    const int bits_limit = 12;

    DECLARE_ALIGNED(16, int16_t, input[kNumCoeffs]);
    DECLARE_ALIGNED(16, tran_low_t, output[kNumCoeffs]);
    DECLARE_ALIGNED(16, tran_low_t, output_sse2[kNumCoeffs]);

    int test_count = 0;
    while (test_count < count_test_block) {
      int i;
      for (i = 0; i < kNumCoeffs; ++i) {
        input[i] = rnd.Rand16();
        range_check(&input[i], bits_limit);
      }

      dst_ref_(input, output);
      dst_sse2_(input, output_sse2);

      for (i = 0; i < kNumCoeffs; ++i) {
        EXPECT_EQ(output[i], output_sse2[i]);
      }
      test_count++;
    }
  }

  FdstFunc dst_ref_;
  FdstFunc dst_sse2_;
};

typedef std::tr1::tuple<FdstFunc, FdstFunc> FdstParam;

class Dst4x4Test
    : public Dst4x4TestBase,
      public ::testing::TestWithParam<FdstParam> {
 public:
  virtual void SetUp() {
    dst_ref_ = GET_PARAM(0);
    dst_sse2_ = GET_PARAM(1);
  }
  virtual void TearDown() {}
};

TEST_P(Dst4x4Test, RunBitsCheck) {
  RunBitsCheck();
}

INSTANTIATE_TEST_CASE_P(
    SSE2, Dst4x4Test,
    ::testing::Values(
         FdstParam(&fdst4x4_ref, &fdst4x4_sse2)));


}  // namespace
