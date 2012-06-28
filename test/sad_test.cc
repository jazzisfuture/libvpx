/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "third_party/googletest/src/include/gtest/gtest.h"
extern "C" {
#include "vpx_config.h"
#include "vpx_rtcd.h"
#include "vp8/common/blockd.h"
}

namespace {

class SADBase {
 protected:
  virtual unsigned int SAD(unsigned int max_sad) = 0;

  enum SADBlockType {SOURCE, REFERENCE};

  // Sum of Absolute Differences. Given two blocks, calculate the absolute
  // difference between two pixels in the same relative location; accumulate.
  unsigned int ReferenceSAD(unsigned int max_sad) {
    unsigned int sad = 0;

    for (int h = 0; h < height_; ++h) {
      for (int w = 0; w < width_; ++w) {
        sad += abs(source_data_[h * source_stride_ + w]
               - reference_data_[h * reference_stride_ + w]);
      }
      if (sad > max_sad) {
        break;
      }
    }
    return sad;
  }

  void FillConstant(SADBlockType block, uint8_t fill_constant) {
    uint8_t *data = NULL;
    int stride;
    if (block == SOURCE) {
      stride = source_stride_;
      data = source_data_;
    } else { // if (block == REFERENCE)
      stride = reference_stride_;
      data = reference_data_;
    }

    for (int h = 0; h < height_; ++h) {
      for (int w = 0; w < width_; ++w) {
        data[h * stride + w] = fill_constant;
      }
    }
  }

  void FillRandom(SADBlockType block) {
    uint8_t *data = NULL;
    int stride;
    if (block == SOURCE) {
      stride = source_stride_;
      data = source_data_;
    } else { // if (block === REFERENCE)
      stride = reference_stride_;
      data = reference_data_;
    }

    for (int h = 0; h < height_; ++h) {
      for (int w = 0; w < width_; ++w) {
        data[h * stride + w] = rand();
      }
    }
  }

  void CheckSad(unsigned int max_sad) {
    unsigned int reference_sad, exp_sad;

    reference_sad = ReferenceSAD(max_sad);
    exp_sad = SAD(max_sad);

    if (reference_sad <= max_sad) {
      ASSERT_EQ(exp_sad, reference_sad);
    } else {
      // Alternative implementations are not required to check max_sad
      ASSERT_GE(exp_sad, reference_sad);
    }
  }

  // Actual test
  void RunTest() {
    {
      SCOPED_TRACE("Min source, max reference");
      FillConstant(SOURCE, 0);
      FillConstant(REFERENCE, 255);
      CheckSad(UINT_MAX);
    }
    {
      SCOPED_TRACE("Max source, min reference");
      FillConstant(SOURCE, 255);
      FillConstant(REFERENCE, 0);
      CheckSad(UINT_MAX);
    }
    {
      SCOPED_TRACE("Short reference stride");
      int tmp_stride = reference_stride_;
      reference_stride_ >>= 1;
      FillRandom(SOURCE);
      FillRandom(REFERENCE);
      CheckSad(UINT_MAX);
      reference_stride_ = tmp_stride;
    }
    {
      SCOPED_TRACE("Short source stride");
      int tmp_stride = source_stride_;
      source_stride_ >>= 1;
      FillRandom(SOURCE);
      FillRandom(REFERENCE);
      CheckSad(UINT_MAX);
      source_stride_ = tmp_stride;
    }
    {
      // Verify that, when max_sad is set, the implementation does not return a
      // value lower than the reference.
      SCOPED_TRACE("Small max_sad");
      FillConstant(SOURCE, 255);
      FillConstant(REFERENCE, 0);
      CheckSad(128);
    }
  }

  // Handle blocks up to  16x16 with stride up to 32
  int height_, width_;
  DECLARE_ALIGNED(16, uint8_t, source_data_[16*32]);
  int source_stride_;
  DECLARE_ALIGNED(16, uint8_t, reference_data_[16*32]);
  int reference_stride_;
};

typedef unsigned int (*sad_m_by_n_fn_t)(const unsigned char *source_ptr, int source_stride,
                                        const unsigned char *reference_ptr, int reference_stride,
                                        int max_sad);

class Sad16x16Test : public ::testing::TestWithParam<sad_m_by_n_fn_t>,
    protected SADBase {
 protected:
  static const int kHeight = 16;
  static const int kWidth = 16;
  static const int kStride = kWidth * 2;

  virtual void SetUp() {
    sad_fn_ = GetParam();
    height_ = kHeight;
    width_ = kWidth;
    source_stride_ = kStride;
    reference_stride_ = kStride;
  }

  virtual unsigned int SAD(unsigned int max_sad) {
    return sad_fn_(source_data_, source_stride_,
                   reference_data_, reference_stride_,
                   max_sad);
  }

  sad_m_by_n_fn_t sad_fn_;
};

TEST_P(Sad16x16Test, SadTests) {
  RunTest();
}

INSTANTIATE_TEST_CASE_P(C, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_c));
// ARM tests
#if HAVE_MEDIA
INSTANTIATE_TEST_CASE_P(MEDIA, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_armv6));
#endif
#if HAVE_NEON
INSTANTIATE_TEST_CASE_P(NEON, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_neon));
#endif

// X86 tests
#if HAVE_MMX
INSTANTIATE_TEST_CASE_P(MMX, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_mmx));
#endif
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_wmt));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSE3, Sad16x16Test,
                        ::testing::Values(vp8_sad16x16_sse3));
#endif

}  // namespace
