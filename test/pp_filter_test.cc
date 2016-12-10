/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <limits.h>
#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "test/acm_random.h"
#include "test/buffer.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

using libvpx_test::Buffer;
using libvpx_test::ACMRandom;

typedef void (*VpxPostProcDownAndAcrossMbRowFunc)(
    unsigned char *src_ptr, unsigned char *dst_ptr, int src_pixels_per_line,
    int dst_pixels_per_line, int cols, unsigned char *flimit, int size);

typedef void (*VpxMbPostProcAcrossIpFunc)(unsigned char *src, int pitch,
                                          int rows, int cols, int flimit);

typedef void (*VpxMbPostProcDownFunc)(unsigned char *dst, int pitch, int rows,
                                      int cols, int flimit);

namespace {

// Compute the filter level used in post proc from the loop filter strength
int q2mbl(int x) {
  if (x < 20) x = 20;

  x = 50 + (x - 50) * 10 / 8;
  return x * x / 3;
}

class VpxPostProcDownAndAcrossMbRowTest
    : public ::testing::TestWithParam<VpxPostProcDownAndAcrossMbRowFunc> {
 public:
  virtual void TearDown() { libvpx_test::ClearSystemState(); }
};

// Test routine for the VPx post-processing function
// vpx_post_proc_down_and_across_mb_row_c.

TEST_P(VpxPostProcDownAndAcrossMbRowTest, CheckFilterOutput) {
  // Size of the underlying data block that will be filtered.
  const int block_width = 16;
  const int block_height = 16;

  // 5-tap filter needs 2 padding rows above and below the block in the input.
  Buffer<uint8_t> src_image = Buffer<uint8_t>(block_height, block_width, 2);

  // Filter extends output block by 8 samples at left and right edges.
  // Though the left padding is only 8 bytes, the assembly code tries to
  // read 16 bytes before the pointer.
  Buffer<uint8_t> dst_image = Buffer<uint8_t>(block_height, block_width, 16);

  uint8_t *const flimits =
      reinterpret_cast<uint8_t *>(vpx_memalign(16, block_width));
  (void)memset(flimits, 255, block_width);

  // Initialize pixels in the input:
  //   block pixels to value 1,
  //   border pixels to value 10.
  src_image.Set(1);
  src_image.SetPadding(10);

  // Initialize pixels in the output to 99.
  dst_image.Set(99);

  ASM_REGISTER_STATE_CHECK(GetParam()(src_image.Src(), dst_image.Src(),
                                      src_image.Stride(), dst_image.Stride(),
                                      block_width, flimits, 16));

  static const uint8_t kExpectedOutput[block_height] = {
    4, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4
  };

  uint8_t *pixel_ptr = dst_image.Src();
  for (int i = 0; i < block_height; ++i) {
    for (int j = 0; j < block_width; ++j) {
      EXPECT_EQ(kExpectedOutput[i], pixel_ptr[j]) << "at (" << i << ", " << j
                                                  << ")";
    }
    pixel_ptr += dst_image.Stride();
  }

  if (HasFailure()) {
    dst_image.DumpBuffer();
  }

  vpx_free(flimits);
};

class VpxMbPostProcAcrossIpTest
    : public ::testing::TestWithParam<VpxMbPostProcAcrossIpFunc> {
 public:
  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  void SetCols(unsigned char *s, int rows, int cols, int src_width) {
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        s[c] = c;
      }
      s += src_width;
    }
  }

  void RunComparison(const unsigned char *expected_output, unsigned char *src_c,
                     int rows, int cols, int src_pitch) {
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        ASSERT_EQ(expected_output[c], src_c[c]) << "at (" << r << ", " << c
                                                << ")";
      }
      src_c += src_pitch;
    }
  }

  void RunFilterLevel(unsigned char *s, int rows, int cols, int src_width,
                      int filter_level, const unsigned char *expected_output) {
    ASM_REGISTER_STATE_CHECK(
        GetParam()(s, src_width, rows, cols, filter_level));
    RunComparison(expected_output, s, rows, cols, src_width);
  }
};

TEST_P(VpxMbPostProcAcrossIpTest, CheckLowFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src = Buffer<uint8_t>(rows, cols, 17);
  src.SetPadding(10);
  SetCols(src.Src(), rows, cols, src.Stride());

  Buffer<uint8_t> expected_output = Buffer<uint8_t>(rows, cols, 0);
  SetCols(expected_output.Src(), rows, cols, expected_output.Stride());

  RunFilterLevel(src.Src(), rows, cols, src.Stride(), q2mbl(0),
                 expected_output.Src());
}

TEST_P(VpxMbPostProcAcrossIpTest, CheckMediumFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src = Buffer<uint8_t>(rows, cols, 17);
  src.SetPadding(10);
  SetCols(src.Src(), rows, cols, src.Stride());

  static const unsigned char kExpectedOutput[cols] = {
    2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13
  };

  RunFilterLevel(src.Src(), rows, cols, src.Stride(), q2mbl(70),
                 kExpectedOutput);
}

TEST_P(VpxMbPostProcAcrossIpTest, CheckHighFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src = Buffer<uint8_t>(rows, cols, 17);
  src.SetPadding(10);
  SetCols(src.Src(), rows, cols, src.Stride());

  static const unsigned char kExpectedOutput[cols] = {
    2, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 13
  };

  RunFilterLevel(src.Src(), rows, cols, src.Stride(), INT_MAX, kExpectedOutput);

  SetCols(src.Src(), rows, cols, src.Stride());

  RunFilterLevel(src.Src(), rows, cols, src.Stride(), q2mbl(100),
                 kExpectedOutput);
}

TEST_P(VpxMbPostProcAcrossIpTest, CheckCvsAssembly) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> c_mem = Buffer<uint8_t>(rows, cols, 17);
  Buffer<uint8_t> asm_mem = Buffer<uint8_t>(rows, cols, 17);

  // When level >= 100, the filter behaves the same as the level = INT_MAX
  // When level < 20, it behaves the same as the level = 0
  for (int level = 0; level < 100; level++) {
    c_mem.SetPadding(10);
    asm_mem.SetPadding(10);
    SetCols(c_mem.Src(), rows, cols, c_mem.Stride());
    SetCols(asm_mem.Src(), rows, cols, asm_mem.Stride());

    vpx_mbpost_proc_across_ip_c(c_mem.Src(), c_mem.Stride(), rows, cols,
                                q2mbl(level));
    ASM_REGISTER_STATE_CHECK(
        GetParam()(asm_mem.Src(), asm_mem.Stride(), rows, cols, q2mbl(level)));

    ASSERT_EQ(0, asm_mem.CompareBuffer(c_mem));
  }
}

class VpxMbPostProcDownTest
    : public ::testing::TestWithParam<VpxMbPostProcDownFunc> {
 public:
  virtual void TearDown() { libvpx_test::ClearSystemState(); }

 protected:
  void SetRows(unsigned char *src_c, int rows, int cols, int src_width) {
    for (int r = 0; r < rows; r++) {
      memset(src_c, r, cols);
      src_c += src_width;
    }
  }

  void RunComparison(const unsigned char *expected_output, unsigned char *src_c,
                     int rows, int cols, int src_pitch) {
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        ASSERT_EQ(expected_output[r * rows + c], src_c[c]) << "at (" << r
                                                           << ", " << c << ")";
      }
      src_c += src_pitch;
    }
  }

  void RunFilterLevel(unsigned char *s, int rows, int cols, int src_width,
                      int filter_level, const unsigned char *expected_output) {
    ASM_REGISTER_STATE_CHECK(
        GetParam()(s, src_width, rows, cols, filter_level));
    RunComparison(expected_output, s, rows, cols, src_width);
  }
};

TEST_P(VpxMbPostProcDownTest, CheckHighFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src_c = Buffer<uint8_t>(rows, cols, 17);
  src_c.SetPadding(10);

  SetRows(src_c.Src(), rows, cols, src_c.Stride());

  static const unsigned char kExpectedOutput[rows * cols] = {
    2,  2,  1,  1,  2,  2,  2,  2,  2,  2,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  3,  4,  4,  3,  3,  3,
    4,  4,  3,  4,  4,  3,  3,  4,  5,  4,  4,  4,  4,  4,  4,  4,  5,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  8,  9,  9,  8,  8,  8,  9,
    9,  8,  9,  9,  8,  8,  8,  9,  9,  10, 10, 9,  9,  9,  10, 10, 9,  10, 10,
    9,  9,  9,  10, 10, 10, 11, 10, 10, 10, 11, 10, 11, 10, 11, 10, 10, 10, 11,
    10, 11, 11, 11, 11, 11, 11, 11, 12, 11, 11, 11, 11, 11, 11, 11, 12, 11, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 12,
    13, 12, 13, 12, 12, 12, 13, 12, 13, 12, 13, 12, 13, 13, 13, 14, 13, 13, 13,
    13, 13, 13, 13, 14, 13, 13, 13, 13
  };

  RunFilterLevel(src_c.Src(), rows, cols, src_c.Stride(), INT_MAX,
                 kExpectedOutput);

  src_c.SetPadding(10);
  SetRows(src_c.Src(), rows, cols, src_c.Stride());
  RunFilterLevel(src_c.Src(), rows, cols, src_c.Stride(), q2mbl(100),
                 kExpectedOutput);
}

TEST_P(VpxMbPostProcDownTest, CheckMediumFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src_c = Buffer<uint8_t>(rows, cols, 17);
  src_c.SetPadding(10);

  SetRows(src_c.Src(), rows, cols, src_c.Stride());

  static const unsigned char kExpectedOutput[rows * cols] = {
    2,  2,  1,  1,  2,  2,  2,  2,  2,  2,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
    5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 13, 12,
    13, 12, 13, 12, 12, 12, 13, 12, 13, 12, 13, 12, 13, 13, 13, 14, 13, 13, 13,
    13, 13, 13, 13, 14, 13, 13, 13, 13
  };

  RunFilterLevel(src_c.Src(), rows, cols, src_c.Stride(), q2mbl(70),
                 kExpectedOutput);
}

TEST_P(VpxMbPostProcDownTest, CheckLowFilterOutput) {
  const int rows = 16;
  const int cols = 16;

  Buffer<uint8_t> src_c = Buffer<uint8_t>(rows, cols, 17);
  src_c.SetPadding(10);

  SetRows(src_c.Src(), rows, cols, src_c.Stride());

  unsigned char *expected_output = new unsigned char[rows * cols];
  ASSERT_TRUE(expected_output != NULL);
  SetRows(expected_output, rows, cols, cols);

  RunFilterLevel(src_c.Src(), rows, cols, src_c.Stride(), q2mbl(0),
                 expected_output);

  delete[] expected_output;
}

TEST_P(VpxMbPostProcDownTest, CheckCvsAssembly) {
  const int rows = 16;
  const int cols = 16;

  ACMRandom rnd;
  rnd.Reset(ACMRandom::DeterministicSeed());

  Buffer<uint8_t> src_c = Buffer<uint8_t>(rows, cols, 17);
  Buffer<uint8_t> src_asm = Buffer<uint8_t>(rows, cols, 17);

  for (int level = 0; level < 100; level++) {
    src_c.SetPadding(10);
    src_asm.SetPadding(10);
    src_c.Set(rnd, &ACMRandom::Rand8);
    src_asm.Set(src_c);

    vpx_mbpost_proc_down_c(src_c.Src(), src_c.Stride(), rows, cols,
                           q2mbl(level));
    ASM_REGISTER_STATE_CHECK(
        GetParam()(src_asm.Src(), src_asm.Stride(), rows, cols, q2mbl(level)));
    ASSERT_EQ(0, src_asm.CompareBuffer(src_c));

    src_c.SetPadding(10);
    src_asm.SetPadding(10);
    src_c.Set(rnd, &ACMRandom::Rand8Extremes);
    src_asm.Set(src_c);

    vpx_mbpost_proc_down_c(src_c.Src(), src_c.Stride(), rows, cols,
                           q2mbl(level));
    ASM_REGISTER_STATE_CHECK(
        GetParam()(src_asm.Src(), src_asm.Stride(), rows, cols, q2mbl(level)));
    ASSERT_EQ(0, src_asm.CompareBuffer(src_c));
  }
}

INSTANTIATE_TEST_CASE_P(
    C, VpxPostProcDownAndAcrossMbRowTest,
    ::testing::Values(vpx_post_proc_down_and_across_mb_row_c));

INSTANTIATE_TEST_CASE_P(C, VpxMbPostProcAcrossIpTest,
                        ::testing::Values(vpx_mbpost_proc_across_ip_c));

INSTANTIATE_TEST_CASE_P(C, VpxMbPostProcDownTest,
                        ::testing::Values(vpx_mbpost_proc_down_c));

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, VpxPostProcDownAndAcrossMbRowTest,
    ::testing::Values(vpx_post_proc_down_and_across_mb_row_sse2));

INSTANTIATE_TEST_CASE_P(SSE2, VpxMbPostProcAcrossIpTest,
                        ::testing::Values(vpx_mbpost_proc_across_ip_sse2));

INSTANTIATE_TEST_CASE_P(SSE2, VpxMbPostProcDownTest,
                        ::testing::Values(vpx_mbpost_proc_down_sse2));
#endif

#if HAVE_MSA
INSTANTIATE_TEST_CASE_P(
    MSA, VpxPostProcDownAndAcrossMbRowTest,
    ::testing::Values(vpx_post_proc_down_and_across_mb_row_msa));

INSTANTIATE_TEST_CASE_P(MSA, VpxMbPostProcAcrossIpTest,
                        ::testing::Values(vpx_mbpost_proc_across_ip_msa));

INSTANTIATE_TEST_CASE_P(MSA, VpxMbPostProcDownTest,
                        ::testing::Values(vpx_mbpost_proc_down_msa));
#endif

}  // namespace
