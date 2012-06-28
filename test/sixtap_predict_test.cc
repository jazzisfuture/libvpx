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
#include <time.h>
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

typedef void (*sixtap_predict_fn_t)(uint8_t *src_ptr,
                                    int  src_pixels_per_line,
                                    int  xoffset,
                                    int  yoffset,
                                    uint8_t *dst_ptr,
                                    int  dst_pitch);

class SixtapPredictTest : public ::testing::TestWithParam<sixtap_predict_fn_t> {
 protected:
  // Six-tap filters need extra pixels outside of the macroblock. Therefore,
  // the source stride is 16 + 5.
  static const int kSrcStride = 21;
  static const int kDstStride = 16;

  virtual void SetUp() {
    sixtap_predict_ = GetParam();
    memset(src_, 0, sizeof(src_));
    memset(dst_, 0, sizeof(dst_));
    memset(dst_c_, 0, sizeof(dst_c_));
  }

  sixtap_predict_fn_t sixtap_predict_;

  // The src stores the macroblock we will filter on, and the result is
  // stored in dst and dst_c(c reference code result).
  DECLARE_ALIGNED(16, uint8_t, src_[kSrcStride * kSrcStride]);
  DECLARE_ALIGNED(16, uint8_t, dst_[kDstStride * kDstStride]);
  DECLARE_ALIGNED(16, uint8_t, dst_c_[kDstStride * kDstStride]);
};

TEST_P(SixtapPredictTest, TestWithPresetData) {
  const size_t src_size = sizeof(src_)/sizeof(uint8_t);
  const size_t dst_size = sizeof(dst_)/sizeof(uint8_t);

  static const uint8_t test_data[src_size] = {
    205, 238,  46, 247, 196, 153,  16, 158, 236,  59, 250,  75, 232,  34,  27,
    3, 210,  87, 212, 206, 183,  22, 224, 120, 155, 180,  58,  47, 132,  60,
    19,  82,  43,  66,  74, 239, 219,  90, 141, 200, 150, 135,  20, 127, 170,
    47, 131, 125, 134,  87,  76,  61, 110,  44, 181,  10, 225, 239,  57, 102,
    43,  77, 185,  87, 143,   3,  70, 107,  94, 211,  51, 244,  91,  71, 116,
    5, 118, 247, 131, 252,  79, 207,  58, 189, 252, 240, 200, 222, 224,   2,
    68,  12,  79, 253,  99, 222,   1, 169,  74,  95, 124, 125,  84, 216, 197,
    200, 221,  60, 192,  97,  56,  15,  48, 115, 205,  44,  99, 149,  10,  68,
    152,  79,  80, 231,  77, 179, 198,  78,  93,  16, 174, 218, 142,   2, 178,
    83, 203, 144, 143, 139, 241, 200, 155,  33,  60, 104,  78, 160, 254,  89,
    228, 150, 169,  53, 126, 246, 232,  69,  68,  69,  86, 243,  31, 228, 246,
    209,  56, 193,  98, 200,  77,  83, 145, 232, 117, 205,  81, 196, 110,  79,
    29,  82, 230, 198, 136, 100, 189, 112, 170,   2, 182,   0, 245, 214, 228,
    235, 168,  29, 173,  10, 230, 251,  93, 119, 228, 211,  69,  54, 151, 179,
    133, 181,   6, 108, 124, 142, 208,  57, 255, 122,  59, 182, 123,  49, 140,
    96,  29,  53, 125, 203,  63,  99, 198, 157, 219, 171, 113,  32, 225,   8,
    212, 103, 189, 219, 211,  57, 106, 164, 115, 105,  31, 175,  32, 154, 224,
    172, 250, 254, 226, 120, 201,  34, 220, 144, 192, 183,  59,  49, 216,  28,
    57, 173, 131, 247, 136,  87,  49, 243, 251, 164,  92,  26,  83, 125, 180,
    52,  42, 175,  50,  12,  39, 251,  46,   4, 139, 238, 187, 199,  32, 148,
    227,  89,  65, 103,  81, 202, 191, 130, 190, 186,  39,  27, 213, 123, 152,
    138, 175, 194,  58, 226, 206,  97, 222, 253, 101, 105, 236,  33,  49,  12,
    182,  21, 102, 248, 125, 183, 194,  60,  58, 128, 247,  97, 155, 204, 220,
    52,  87, 140, 246, 145, 110, 196, 243,  76, 194,  88, 182, 174, 122, 231,
    186,  48, 252,  32,  40, 121, 216, 235, 181,  18, 108, 172, 115,   8, 121,
    80,  60, 208, 220,  50,  98,  75, 247,  85, 151, 185, 174,  77, 104,  40,
    53,  35,  88,  49,  67, 129, 170,  28, 109,  96,  46, 217,  13, 162, 225,
    134, 243,  30,  87, 207,  81, 185,  26,  72,  15, 178,   2, 189, 255, 107,
    229,  53, 142,  62, 102, 210, 192,  17, 238,  45, 113,  29,   6, 126, 191,
    232,   5, 178,   6,  93, 130
  };

  static const uint8_t expected_dst[dst_size] = {
    98,  48, 123, 143, 144,  87, 114,  70,  90,  81, 120,  62, 252, 186,  97,
    90,  46, 127, 117,  99, 151,  70, 216, 105, 103, 111,   2, 183, 183, 142,
    211, 104, 191, 244, 148,   0,  38,  19, 127, 225, 100, 167,  49, 116,  56,
    105, 103, 119,  66, 135, 103,  91,  36,  53, 149, 145,  78, 127, 132,  33,
    137, 165,  66,  92,  83,  82, 198, 202,  87,  63, 194, 120, 178, 127, 123,
    172, 237, 158, 165,  77, 106, 197, 151, 147,  63, 176, 230, 201,  58,  54,
    39, 130, 203,  71, 224, 238,  59, 127, 191, 220, 165, 167,  69, 178,  89,
    113,  78, 113, 219, 215, 130,  94,  27, 233, 191, 226, 225, 108, 117, 115,
    49, 251, 192,  69, 144, 201, 143,  48,  45, 132, 119, 133, 135, 116, 206,
    118,  90, 183, 114,  91, 124,  76,  42,  86, 199, 223, 183, 114, 107, 181,
    60, 178, 138, 193, 243, 181,  38, 132, 184,  89, 171, 155, 255, 255, 188,
    123, 142,  63, 210, 141, 150, 131,  62,  74, 143,  26,  86, 104, 194, 217,
    159,  56,  25,  98, 128, 171,  62,  95, 185,  55,   0,  98, 203, 142,  72,
    183, 182, 112, 115,  94, 118, 214, 137, 116, 167, 159,   7,  93, 120, 100,
    255, 146, 130, 255, 223,  62, 133, 191,  70,  45,  58, 176,  67, 142, 110,
    169, 179, 126, 131, 171, 167,  93, 132, 168, 201, 139, 147, 197, 172,  96,
    212, 135, 117, 192,  34,  74, 154, 204, 205, 108,   5, 116, 154,  61,  22,
    141
  };

  uint8_t *src = const_cast<uint8_t *>(test_data);

  sixtap_predict_(&src[kSrcStride * 2 + 2], kSrcStride,
                            2, 2, dst_c_, kDstStride);

  for (size_t i = 0; i < dst_size; ++i)
    EXPECT_EQ(dst_c_[i], expected_dst[i]) << "i==" << i;
}

TEST_P(SixtapPredictTest, TestWithRandomData) {
  const size_t src_size = sizeof(src_)/sizeof(uint8_t);
  const size_t dst_size = sizeof(dst_)/sizeof(uint8_t);

  ACMRandom rnd(time(NULL));
  for (size_t i = 0; i < src_size; ++i)
    src_[i] = rnd.Rand8();

  // Run tests for all possible offsets.
  for (int xoffset = 0; xoffset < 8; ++xoffset) {
    for (int yoffset = 0; yoffset < 8; ++yoffset) {
      // The case that both xoffset and yoffset are 0 is taken care of outside
      // of filter functions.
      if (xoffset != 0 || yoffset != 0) {
        // Call c reference function.
        vp8_sixtap_predict16x16_c(&src_[kSrcStride * 2 + 2], kSrcStride,
                                  xoffset, yoffset, dst_c_, kDstStride);

        // Run test.
        sixtap_predict_(&src_[kSrcStride * 2 + 2], kSrcStride,
                        xoffset, yoffset, dst_, kDstStride);

        for (size_t i = 0; i < dst_size; ++i)
          EXPECT_EQ(dst_c_[i], dst_[i]) << "i==" << i;
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
