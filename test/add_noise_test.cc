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
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

namespace {

// TODO(jimbankoski): make width and height integers not unsigned.
typedef void (*AddNoiseFunc)(unsigned char *start, char *noise,
                             char blackclamp[16], char whiteclamp[16],
                             char bothclamp[16], unsigned int width,
                             unsigned int height, int pitch);

class AddNoiseTest
    : public ::testing::TestWithParam<AddNoiseFunc> {
 public:
  virtual void TearDown() {
    libvpx_test::ClearSystemState();
  }
};

static double stddev6(char a, char b, char c, char d, char e, char f) {
  double n = (a + b + c + d + e + f) / 6.0;
  double v = ((a - n) * (a - n) + (b - n) * (b - n) + (c - n) * (c - n) +
              (d - n) * (d - n) + (e - n) * (e - n) + (f - n) * (f - n)) /
             6.0;
  return sqrt(v);
}

// TODO(jimbankoski): The following 2 functions are duplicated in each codec.
// For now the vp9 one has been copied into the test as is. We should normalize
// these in vpx_dsp and not have 3 copies of these unless there is different
// noise we add for each codec.

static double gaussian(double sigma, double mu, double x) {
  return 1 / (sigma * sqrt(2.0 * 3.14159265)) *
         (exp(-(x - mu) * (x - mu) / (2 * sigma * sigma)));
}

static int setup_noise(int size, char *noise) {
  char char_dist[300];
  int ai = 4, qi = 24, i;
  double sigma = ai + .5 + .6 * (63 - qi) / 63.0;

  /* set up a lookup table of 256 entries that matches
   * a gaussian distribution with sigma determined by q.
   */
  {
    int next, i, j;

    next = 0;

    for (i = -32; i < 32; i++) {
      int a_i = (int) (0.5 + 256 * gaussian(sigma, 0, i));

      if (a_i) {
        for (j = 0; j < a_i; j++) {
          char_dist[next + j] = (char)(i);
        }

        next = next + j;
      }
    }

    for (; next < 256; next++)
      char_dist[next] = 0;
  }

  for (i = 0; i < 3072; i++) {
    noise[i] = char_dist[rand() & 0xff];  // NOLINT
  }

  // Returns the most negative value in distribution.
  return char_dist[0];
}

TEST_P(AddNoiseTest, CheckNoiseAdded) {
  DECLARE_ALIGNED(16, char, blackclamp[16]);
  DECLARE_ALIGNED(16, char, whiteclamp[16]);
  DECLARE_ALIGNED(16, char, bothclamp[16]);
  const int width  = 64;
  const int height = 64;
  const int image_size = width * height;
  int i;
  char noise[3072];

  int clamp = setup_noise(3072, noise);

  for (i = 0; i < 16; i++) {
    blackclamp[i] = -clamp;
    whiteclamp[i] = -clamp;
    bothclamp[i] = -2 * clamp;
  }

  uint8_t *const s =
      reinterpret_cast<uint8_t*>(vpx_calloc(image_size, 1));

  // Initialize pixels in the image to 99.
  memset(s, 99, image_size);

  ASM_REGISTER_STATE_CHECK(
        GetParam()(s, noise, blackclamp, whiteclamp, bothclamp,
                   width, height, width));

  // Check to make sure we don't end up having either the same or no added
  // noise either vertically or horizontally.
  for (i = 0; i < image_size - 6 * width - 6; ++i) {
    double hd = stddev6(s[i] - 99, s[i + 1] - 99, s[i + 2] - 99, s[i + 3] - 99,
                        s[i + 4] - 99, s[i + 5] - 99);
    double vd = stddev6(s[i] - 99, s[i + width] - 99, s[i + 2 * width] - 99,
                        s[i + 3 * width] - 99, s[i + 4 * width] - 99,
                        s[i + 5 * width] - 99);

    EXPECT_NE(hd, 0);
    EXPECT_NE(vd, 0);
  }

  // Initialize pixels in the image to 255 and check for roll over.
  memset(s, 255, image_size);

  ASM_REGISTER_STATE_CHECK(
        GetParam()(s, noise, blackclamp, whiteclamp, bothclamp,
                   width, height, width));

  // Check to make sure don't roll over .
  for (i = 0; i < image_size; ++i) {
    EXPECT_GT((int)s[i], 10) << "i = " << i;
  }

  // Initialize pixels in the image to 0 and check for roll under.
  (void)memset(s, 0, image_size);

  ASM_REGISTER_STATE_CHECK(
        GetParam()(s, noise, blackclamp, whiteclamp, bothclamp,
                   width, height, width));

  // Check to make sure don't roll under.
  for (i = 0; i < image_size; ++i) {
    EXPECT_LT((int)s[i], 245) << "i = " << i;
  }

  vpx_free(s);
};

// TODO(jimbankoski): Make the c work like assembly so we can enable this.
TEST_P(AddNoiseTest, DISABLED_CheckCvsAssembly) {
  const int width  = 64;
  const int height = 64;
  const int image_size = width * height;
  int i;
  char noise[3072];
  DECLARE_ALIGNED(16, char, blackclamp[16]);
  DECLARE_ALIGNED(16, char, whiteclamp[16]);
  DECLARE_ALIGNED(16, char, bothclamp[16]);

  int clamp = setup_noise(3072, noise);
  for (i = 0; i < 16; i++) {
    blackclamp[i] = -clamp;
    whiteclamp[i] = -clamp;
    bothclamp[i] = -2 * clamp;
  }

  uint8_t *const s =
      reinterpret_cast<uint8_t*>(vpx_calloc(image_size, 1));

  uint8_t *const d =
      reinterpret_cast<uint8_t*>(vpx_calloc(image_size, 1));

  // Initialize pixels in the image to 99.
  (void)memset(s, 99, image_size);
  (void)memset(d, 99, image_size);

  ASM_REGISTER_STATE_CHECK(
        GetParam()(s, noise, blackclamp, whiteclamp, bothclamp,
                   width, height, width));

  ASM_REGISTER_STATE_CHECK(
      vpx_plane_add_noise_c(d, noise, blackclamp, whiteclamp, bothclamp,
                   width, height, width));

  for (i = 0; i < image_size; ++i) {
    EXPECT_EQ((int)s[i], (int)d[i]) << "i = " << i;
  }

  vpx_free(d);
  vpx_free(s);
};


INSTANTIATE_TEST_CASE_P(C, AddNoiseTest,
    ::testing::Values(vpx_plane_add_noise_c));

#if HAVE_MMX
INSTANTIATE_TEST_CASE_P(MMX, AddNoiseTest,
    ::testing::Values(vpx_plane_add_noise_mmx));
#endif

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, AddNoiseTest,
    ::testing::Values(vpx_plane_add_noise_sse2));
#endif

#if HAVE_MSA
INSTANTIATE_TEST_CASE_P(MSA, AddNoiseTest,
    ::testing::Values(vpx_plane_add_noise_msa));
#endif
}  // namespace
