/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "./vp8_rtcd.h"

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "test/buffer.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "vpx/vpx_integer.h"

typedef void (*IdctFunc)(int16_t *input, unsigned char *pred_ptr,
                         int pred_stride, unsigned char *dst_ptr,
                         int dst_stride);
namespace {

using libvpx_test::Buffer;

class IDCTTest : public ::testing::TestWithParam<IdctFunc> {
 protected:
  virtual void SetUp() {
    UUT = GetParam();
  }

  virtual void TearDown() { libvpx_test::ClearSystemState(); }

  IdctFunc UUT;
};

TEST_P(IDCTTest, TestAllZeros) {
  // When the input is '0' the output will be '0'.
  Buffer<int16_t> input = Buffer<int16_t>(4, 4, 0);
  Buffer<uint8_t> predict = Buffer<uint8_t>(4, 4, 3);
  Buffer<uint8_t> output = Buffer<uint8_t>(4, 4, 3);
  input.Set(0);
  predict.Set(0);

  ASM_REGISTER_STATE_CHECK(UUT(input.Src(), predict.Src(), predict.Stride(),
                               output.Src(), output.Stride()));

  ASSERT_EQ(0, output.CheckValues(0));
  ASSERT_EQ(0, output.CheckPadding());
}

TEST_P(IDCTTest, TestAllOnes) {
  Buffer<int16_t> input = Buffer<int16_t>(4, 4, 0);
  Buffer<uint8_t> predict = Buffer<uint8_t>(4, 4, 3);
  Buffer<uint8_t> output = Buffer<uint8_t>(4, 4, 3);
  input.Set(0);
  // When the first element is '4' it will fill the output buffer with '1'.
  input.Src()[0] = 4;
  predict.Set(0);

  ASM_REGISTER_STATE_CHECK(UUT(input.Src(), predict.Src(), predict.Stride(),
                               output.Src(), output.Stride()));

  ASSERT_EQ(0, output.CheckValues(1));
  ASSERT_EQ(0, output.CheckPadding());
}

TEST_P(IDCTTest, TestAddOne) {
  Buffer<int16_t> input = Buffer<int16_t>(4, 4, 0);
  Buffer<uint8_t> predict = Buffer<uint8_t>(4, 4, 3);
  Buffer<uint8_t> output = Buffer<uint8_t>(4, 4, 3);
  // Set the transform output to '1' and make sure it gets added to the
  // prediction buffer.
  input.Set(0);
  input.Src()[0] = 4;

  uint8_t *pred = predict.Src();
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      pred[y * predict.Stride() + x] = y * 4 + x;
    }
  }

  ASM_REGISTER_STATE_CHECK(UUT(input.Src(), predict.Src(), predict.Stride(),
                               output.Src(), output.Stride()));

  uint8_t *out = output.Src();
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      EXPECT_EQ(1 + y * 4 + x, out[y * output.Stride() + x]);
    }
  }

  if (HasFailure()) {
    output.DumpBuffer();
  }

  ASSERT_EQ(0, output.CheckPadding());
}

TEST_P(IDCTTest, TestWithData) {
  Buffer<int16_t> input = Buffer<int16_t>(4, 4, 0);
  Buffer<uint8_t> predict = Buffer<uint8_t>(4, 4, 3);
  Buffer<uint8_t> output = Buffer<uint8_t>(4, 4, 3);
  // Test a single known input.
  predict.Set(0);

  int16_t *in = input.Src();
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      in[y * input.Stride() + x] = y * 4 + x;
    }
  }

  ASM_REGISTER_STATE_CHECK(UUT(input.Src(), predict.Src(), predict.Stride(),
                               output.Src(), output.Stride()));

  uint8_t *out = output.Src();
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      switch (y * 4 + x) {
        case 0: EXPECT_EQ(11, out[y * output.Stride() + x]); break;
        case 2:
        case 5:
        case 8: EXPECT_EQ(3, out[y * output.Stride() + x]); break;
        case 10: EXPECT_EQ(1, out[y * output.Stride() + x]); break;
        default: EXPECT_EQ(0, out[y * output.Stride() + x]);
      }
    }
  }

  if (HasFailure()) {
    output.DumpBuffer();
  }

  ASSERT_EQ(0, output.CheckPadding());
}

INSTANTIATE_TEST_CASE_P(C, IDCTTest, ::testing::Values(vp8_short_idct4x4llm_c));

#if HAVE_NEON
INSTANTIATE_TEST_CASE_P(NEON, IDCTTest,
                        ::testing::Values(vp8_short_idct4x4llm_neon));
#endif  // HAVE_NEON

#if HAVE_MMX
INSTANTIATE_TEST_CASE_P(MMX, IDCTTest,
                        ::testing::Values(vp8_short_idct4x4llm_mmx));
#endif  // HAVE_MMX

#if HAVE_MSA
INSTANTIATE_TEST_CASE_P(MSA, IDCTTest,
                        ::testing::Values(vp8_short_idct4x4llm_msa));
#endif  // HAVE_MSA
}
