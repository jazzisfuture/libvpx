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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "third_party/googletest/src/include/gtest/gtest.h"
extern "C" {
#include "vpx_config.h"
#include "vpx_rtcd.h"
#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"
}

namespace {

class ACMRandom {
public:
  explicit ACMRandom(int seed) { Reset(seed); }

  void Reset(int seed) { srand(seed); }

  uint8_t Rand8(void) { return (rand() >> 8) & 0xff; }

  int PseudoUniform(int range) { return (rand() >> 8) % range; }

  int operator()(int n) { return PseudoUniform(n); }

  static int DeterministicSeed(void) { return 0xbaba; }
};

typedef void (*sixtap_predict_fn_t)
(
    unsigned char  *src_ptr,
    int  src_pixels_per_line,
    int  xoffset,
    int  yoffset,
    unsigned char *dst_ptr,
    int  dst_pitch
);

class SixtapPredictTest : public ::testing::TestWithParam<sixtap_predict_fn_t>
{
protected:
  // Six-tap filters need extra pixels outside of the macroblock. Therefore,
  // the source stride is 16 + 5.
  static const int src_stride = 21;
  static const int dst_stride = 16;

  virtual void SetUp() {
    sixtap_predict = GetParam();
    memset(src, 0, sizeof(src));
    memset(dst, 0, sizeof(dst));
    memset(dst_c, 0, sizeof(dst_c));
  }

  sixtap_predict_fn_t sixtap_predict;

  // The src stores the macroblock we will filter on, and the result is
  // stored in dst and dst_c(c reference code result).
  DECLARE_ALIGNED(16, unsigned char, src[src_stride * src_stride]);
  DECLARE_ALIGNED(16, unsigned char, dst[dst_stride * dst_stride]);
  DECLARE_ALIGNED(16, unsigned char, dst_c[dst_stride * dst_stride]);
};

TEST_P(SixtapPredictTest, TestWithData) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());

  for (int i = 0; i < src_stride * src_stride; i++)
    src[i] = rnd.Rand8();

  // Run tests for all possible offsets.
  for (int xoffset = 0; xoffset < 8; xoffset++) {
    for (int yoffset = 0; yoffset < 8; yoffset++) {
      // The case that both xoffset and yoffset are 0 is taken care of outside
      // of filter functions.
      if (xoffset != 0 || yoffset != 0) {
        // Call c reference function.
        vp8_sixtap_predict16x16_c(&src[src_stride*2 + 2], src_stride, xoffset,
                                  yoffset, dst_c, dst_stride);

        // Run test.
        sixtap_predict(&src[src_stride*2 + 2], src_stride, xoffset,
                       yoffset, dst, dst_stride);

        for(int i=0; i<256; i++)
          EXPECT_EQ(dst_c[i], dst[i]) << "i==" << i;
      }
    }
  }
}

INSTANTIATE_TEST_CASE_P(C, SixtapPredictTest,
                        ::testing::Values(vp8_sixtap_predict16x16_c));
#if HAVE_MMX
INSTANTIATE_TEST_CASE_P(MMX, SixtapPredictTest,
                        ::testing::Values(vp8_sixtap_predict16x16_mmx));
#endif
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, SixtapPredictTest,
                        ::testing::Values(vp8_sixtap_predict16x16_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, SixtapPredictTest,
                        ::testing::Values(vp8_sixtap_predict16x16_ssse3));
#endif

}  // namespace
