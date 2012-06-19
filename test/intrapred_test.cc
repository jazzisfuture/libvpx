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
#include "third_party/googletest/src/include/gtest/gtest.h"
extern "C" {
#include "vpx_config.h"
#include "vpx_rtcd.h"
#include "vp8/common/blockd.h"
}

namespace {

typedef void (*intra_pred_y_fn_t)(MACROBLOCKD *x,
                                  uint8_t *yabove_row,
                                  uint8_t *yleft,
                                  int left_stride,
                                  uint8_t *ypred_ptr,
                                  int y_stride);

class IntraPredYTest : public ::testing::TestWithParam<intra_pred_y_fn_t> {
 protected:
  virtual void SetUp() {
    pred_fn_ = GetParam();
    memset(&mb_, 0, sizeof(mb_));
    memset(&mi_, 0, sizeof(mi_));
    mb_.up_available = 1;
    mb_.left_available = 1;
    mb_.mode_info_context = &mi_;
    data_ptr_ = data_array_ + kBlockSize + kStride;
    for (int i = 0; i < kBlockSize; ++i) {
      data_ptr_[i - kStride] = 0x60 + i + (i >= kBlockSize / 2);  // top
      data_ptr_[kStride * i - 1] = 0x80 - i - (i >= kBlockSize / 2);  // left
    }
    data_ptr_[-1 - kStride] = 0x40;  // top-left
    data_ptr_[kBlockSize - kStride] = 0xc0;  // top-right
  }

  void SetLeftUnavailable() {
    mb_.left_available = 0;
    for (int i = -1; i < kBlockSize; ++i) {
      data_ptr_[kStride * i - 1] = 129;
    }
  }

  void SetTopUnavailable() {
    mb_.up_available = 0;
    memset(&data_ptr_[-1 - kStride], 127, kBlockSize + 2);
  }

  void SetTopLeftUnavailable() {
    SetLeftUnavailable();
    SetTopUnavailable();
  }

  void Predict(MB_PREDICTION_MODE mode) {
    mb_.mode_info_context->mbmi.mode = mode;
    pred_fn_(&mb_, data_ptr_ - kStride, data_ptr_ - 1, kStride,
             data_ptr_, kStride);
  }

  intra_pred_y_fn_t pred_fn_;
  MACROBLOCKD mb_;
  MODE_INFO mi_;
  // We use 48 so that the data pointer of the first pixel in each row of
  // each macroblock is 16-byte aligned, and this gives us access to the
  // top-left and top-right corner pixels belonging to the top-left/right
  // macroblocks.
  // We use 17 lines so we have one line above us for top-prediction.
  static const int kBlockSize = 16;
  static const int kStride = kBlockSize * 3;
  DECLARE_ALIGNED(16, uint8_t, data_array_[kStride * (kBlockSize + 1)]);
  uint8_t *data_ptr_;
};

TEST_P(IntraPredYTest, DC_PRED) {
  Predict(DC_PRED);
  for (int y = 1; y < kBlockSize; ++y)
    EXPECT_EQ(0, memcmp(data_ptr_, &data_ptr_[y * kStride], kBlockSize));
  for (int x = 1; x < kBlockSize; ++x)
    EXPECT_EQ(data_ptr_[0], data_ptr_[x]);
  EXPECT_EQ(0x70, data_ptr_[0]);
}

TEST_P(IntraPredYTest, DC_PRED_TOP) {
  SetLeftUnavailable();
  Predict(DC_PRED);
  for (int y = 1; y < kBlockSize; ++y)
    EXPECT_EQ(0, memcmp(data_ptr_, &data_ptr_[y * kStride], kBlockSize));
  for (int x = 1; x < kBlockSize; ++x)
    EXPECT_EQ(data_ptr_[0], data_ptr_[x]);
  EXPECT_EQ(0x68, data_ptr_[0]);
}

TEST_P(IntraPredYTest, DC_PRED_LEFT) {
  SetTopUnavailable();
  Predict(DC_PRED);
  for (int y = 1; y < kBlockSize; ++y)
    EXPECT_EQ(0, memcmp(data_ptr_, &data_ptr_[y * kStride], kBlockSize));
  for (int x = 1; x < kBlockSize; ++x)
    EXPECT_EQ(data_ptr_[0], data_ptr_[x]);
  EXPECT_EQ(0x78, data_ptr_[0]);
}

TEST_P(IntraPredYTest, DC_PRED_TOPLEFT) {
  SetTopLeftUnavailable();
  Predict(DC_PRED);
  for (int y = 0; y < kBlockSize; ++y)
    for (int x = 0; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[0], data_ptr_[y * kStride + x]);
  EXPECT_EQ(0x80, data_ptr_[0]);
}

TEST_P(IntraPredYTest, H_PRED) {
  Predict(H_PRED);
  for (int y = 0; y < kBlockSize; ++y)
    for (int x = 0; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[y * kStride - 1], data_ptr_[y * kStride + x]);
}

TEST_P(IntraPredYTest, V_PRED) {
  Predict(V_PRED);
  for (int y = 0; y < kBlockSize; ++y)
    EXPECT_EQ(0, memcmp(&data_ptr_[kStride * y], &data_ptr_[-kStride],
                        kBlockSize));
}

TEST_P(IntraPredYTest, TM_PRED) {
  Predict(TM_PRED);
  for (int y = 0; y < kBlockSize; ++y) {
    for (int x = 0; x < kBlockSize; ++x) {
      int expected = data_ptr_[x - kStride]
                   + data_ptr_[kStride * y - 1]
                   - data_ptr_[-1 - kStride];
      if (expected < 0)
        expected = 0;
      else if (expected > 255)
        expected = 255;
      EXPECT_EQ(expected, data_ptr_[y * kStride + x]);
    }
  }
}

INSTANTIATE_TEST_CASE_P(C, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_ssse3));
#endif

typedef void (*intra_pred_uv_fn_t)(MACROBLOCKD *x,
                                   uint8_t *uabove_row,
                                   uint8_t *vabove_row,
                                   uint8_t *uleft,
                                   uint8_t *vleft,
                                   int left_stride,
                                   uint8_t *upred_ptr,
                                   uint8_t *vpred_ptr,
                                   int pred_stride);

class IntraPredUVTest : public ::testing::TestWithParam<intra_pred_uv_fn_t> {
 protected:
  virtual void SetUp() {
    pred_fn_ = GetParam();
    memset(&mb_, 0, sizeof(mb_));
    memset(&mi_, 0, sizeof(mi_));
    mb_.up_available = 1;
    mb_.left_available = 1;
    mb_.mode_info_context = &mi_;
    for (int j = 0; j < 2; ++j) {
      data_ptr_[j] = data_array_[j] + kBlockSize + kStride;
      for (int i = 0; i < kBlockSize; ++i) {
        data_ptr_[j][i - kStride] = 0x60 + i + (i >= 4);  // top
        data_ptr_[j][kStride * i - 1] = 0x80 - i - (i >= 4);  // left
      }
      data_ptr_[j][-1 - kStride] = 0x40;  // top-left
      data_ptr_[j][kBlockSize - kStride] = 0xc0;  // top-right
    }
  }

  void SetLeftUnavailable() {
    mb_.left_available = 0;
    for (int i = -1; i < kBlockSize; ++i) {
      data_ptr_[0][kStride * i - 1] = 129;
      data_ptr_[1][kStride * i - 1] = 129;
    }
  }

  void SetTopUnavailable() {
    mb_.up_available = 0;
    memset(&data_ptr_[0][-1 - kStride], 127, kBlockSize + 2);
    memset(&data_ptr_[1][-1 - kStride], 127, kBlockSize + 2);
  }

  void SetTopLeftUnavailable() {
    SetLeftUnavailable();
    SetTopUnavailable();
  }

  void Predict(MB_PREDICTION_MODE mode) {
    mb_.mode_info_context->mbmi.uv_mode = mode;
    pred_fn_(&mb_, data_ptr_[0] - kStride, data_ptr_[1] - kStride,
             data_ptr_[0] - 1, data_ptr_[1] - 1, kStride,
             data_ptr_[0], data_ptr_[1], kStride);
  }

  intra_pred_uv_fn_t pred_fn_;
  MACROBLOCKD mb_;
  MODE_INFO mi_;
  // We use 24 so that the data pointer of the first pixel in each row of
  // each macroblock is 8-byte aligned, and this gives us access to the
  // top-left and top-right corner pixels belonging to the top-left/right
  // macroblocks.
  // We use 9 lines so we have one line above us for top-prediction.
  // [0] = U, [1] = V
  static const int kBlockSize = 8;
  static const int kStride = kBlockSize * 3;
  DECLARE_ALIGNED(8, uint8_t, data_array_[2][kStride * (kBlockSize + 1)]);
  uint8_t *data_ptr_[2];
};

TEST_P(IntraPredUVTest, DC_PRED) {
  Predict(DC_PRED);
  for (int p = 0; p < 2; ++p) {
    for (int y = 1; y < kBlockSize; ++y)
      EXPECT_EQ(0, memcmp(data_ptr_[p], &data_ptr_[p][y * kStride], kBlockSize));
    for (int x = 1; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[p][0], data_ptr_[p][x]);
    EXPECT_EQ(0x70, data_ptr_[p][0]);
  }
}

TEST_P(IntraPredUVTest, DC_PRED_TOP) {
  SetLeftUnavailable();
  Predict(DC_PRED);
  for (int p = 0; p < 2; ++p) {
    for (int y = 1; y < kBlockSize; ++y)
      EXPECT_EQ(0, memcmp(data_ptr_[p], &data_ptr_[p][y * kStride], kBlockSize));
    for (int x = 1; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[p][0], data_ptr_[p][x]);
    EXPECT_EQ(0x64, data_ptr_[p][0]);
  }
}

TEST_P(IntraPredUVTest, DC_PRED_LEFT) {
  SetTopUnavailable();
  Predict(DC_PRED);
  for (int p = 0; p < 2; ++p) {
    for (int y = 1; y < kBlockSize; ++y)
      EXPECT_EQ(0, memcmp(data_ptr_[p], &data_ptr_[p][y * kStride], kBlockSize));
    for (int x = 1; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[p][0], data_ptr_[p][x]);
    EXPECT_EQ(0x7C, data_ptr_[p][0]);
  }
}

TEST_P(IntraPredUVTest, DC_PRED_TOPLEFT) {
  SetTopLeftUnavailable();
  Predict(DC_PRED);
  for (int p = 0; p < 2; ++p) {
    for (int y = 1; y < kBlockSize; ++y)
      EXPECT_EQ(0, memcmp(data_ptr_[p], &data_ptr_[p][y * kStride], kBlockSize));
    for (int x = 1; x < kBlockSize; ++x)
      EXPECT_EQ(data_ptr_[p][0], data_ptr_[p][x]);
    EXPECT_EQ(0x80, data_ptr_[p][0]);
  }
}

TEST_P(IntraPredUVTest, H_PRED) {
  Predict(H_PRED);
  for (int p = 0; p < 2; ++p)
    for (int y = 0; y < kBlockSize; ++y)
      for (int x = 0; x < kBlockSize; ++x)
        EXPECT_EQ(data_ptr_[p][y * kStride - 1], data_ptr_[p][y * kStride + x]);
}

TEST_P(IntraPredUVTest, V_PRED) {
  Predict(V_PRED);
  for (int p = 0; p < 2; ++p)
    for (int y = 0; y < kBlockSize; ++y)
      EXPECT_EQ(0, memcmp(&data_ptr_[p][kStride * y], &data_ptr_[p][-kStride],
                          kBlockSize));
}

TEST_P(IntraPredUVTest, TM_PRED) {
  Predict(TM_PRED);
  for (int p = 0; p < 2; ++p) {
    for (int y = 0; y < kBlockSize; ++y) {
      for (int x = 0; x < kBlockSize; ++x) {
        int expected = data_ptr_[p][x - kStride]
                     + data_ptr_[p][kStride * y - 1]
                     - data_ptr_[p][-1 - kStride];
        if (expected < 0)
          expected = 0;
        else if (expected > 255)
          expected = 255;
        EXPECT_EQ(expected, data_ptr_[p][y * kStride + x]);
      }
    }
  }
}

INSTANTIATE_TEST_CASE_P(C, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_ssse3));
#endif

}  // namespace
