/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_dsp_rtcd.h"

#include "test/acm_random.h"
#include "test/register_state_check.h"

namespace {

using ::libvpx_test::ACMRandom;

typedef void (*Hadamard8x8Func)(const int16_t *a, int a_stride,
                                int16_t *b);

class HadamardTest : public ::testing::TestWithParam<Hadamard8x8Func> {
 public:
  virtual void SetUp() {
    h_func_ = GetParam();
    rnd_.Reset(ACMRandom::DeterministicSeed());
  }

 protected:
  Hadamard8x8Func h_func_;
  ACMRandom rnd_;
};

void hadamard_loop(const int16_t *a, int a_stride, int16_t *b) {
  int16_t b_[8];
  for (int i = 0; i < 8; i += 2) {
    b_[i] = a[i * a_stride] + a[(i + 1) * a_stride];
    b_[i + 1] = a[i * a_stride] - a[(i + 1) * a_stride];
  }
  int16_t c[8];
  for (int i = 0; i < 8; i += 4) {
    c[i] = b_[i] + b_[i + 2];
    c[i + 1] = b_[i + 1] + b_[i + 3];
    c[i + 2] = b_[i] - b_[i + 2];
    c[i + 3] = b_[i + 1] - b_[i + 3];
  }
  b[0] = c[0] + c[4];
  b[7] = c[1] + c[5];
  b[3] = c[2] + c[6];
  b[4] = c[3] + c[7];
  b[2] = c[0] - c[4];
  b[6] = c[1] - c[5];
  b[1] = c[2] - c[6];
  b[5] = c[3] - c[7];
}

void reference_hadamard(const int16_t *a, int a_stride, int16_t *b) {
  int16_t buf[64];
  for(int i = 0; i < 8; i++) {
    hadamard_loop(a + i, a_stride, buf + i * 8);
  }

  for(int i = 0; i < 8; i++) {
    hadamard_loop(buf + i, 8, b + i * 8);
  }
}

TEST_P(HadamardTest, CompareReferenceRandom) {
  int16_t a[64], b[64], b_ref[64];
  for (int i = 0; i < 64; i++) {
    a[i] = rnd_.RandI9();
  }
  memset (b, 0, sizeof(*b) * 64);
  memset (b_ref, 0, sizeof(*b_ref) * 64);

  reference_hadamard(a, 8, b);
  ASM_REGISTER_STATE_CHECK(h_func_(a, 8, b_ref));

  for (int i = 0; i < 64; i++) {
    EXPECT_EQ(b_ref[i], b[i]);
  }
}

INSTANTIATE_TEST_CASE_P(C, HadamardTest, ::testing::Values(vpx_hadamard_8x8_c));

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, HadamardTest,
                        ::testing::Values(vpx_hadamard_8x8_sse2));
#endif  // HAVE_SSE2

#if HAVE_SSSE3 && CONFIG_USE_X86INC && ARCH_X86_64
INSTANTIATE_TEST_CASE_P(SSSE3, HadamardTest,
                        ::testing::Values(vpx_hadamard_8x8_ssse3));
#endif  // HAVE_SSSE3 && CONFIG_USE_X86INC && ARCH_X86_64
}  // namespace
