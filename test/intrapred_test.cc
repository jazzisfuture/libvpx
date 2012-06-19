/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


extern "C" {
#include "vpx_config.h"
#include "vpx_rtcd.h"
#include "vp8/common/blockd.h"
}
#include "third_party/googletest/src/include/gtest/gtest.h"

typedef void (*intra_pred_y_fn_t)(MACROBLOCKD *x,
                                  unsigned char * yabove_row,
                                  unsigned char * yleft,
                                  int left_stride,
                                  unsigned char * ypred_ptr,
                                  int y_stride);

namespace {
class IntraPredYTest : public ::testing::TestWithParam<intra_pred_y_fn_t>
{
protected:
    virtual void SetUp()
    {
        int i;

        pred_fn = GetParam();
        memset(&mb, 0, sizeof(mb));
        memset(&mi, 0, sizeof(mi));
        mb.up_available = 1;
        mb.left_available = 1;
        mb.mode_info_context = &mi;
        stride = 48;
        data_ptr = data_array + 16 + stride;
        for (i = 0; i < 16; i++) {
            data_ptr[i - stride] = 0x60 + i + (i >= 8); // top
            data_ptr[stride * i - 1] = 0x80 - i - (i >= 8); // left
        }
        data_ptr[-1 - stride] = 0x40; // top-left
        data_ptr[16 - stride] = 0xc0; // top-right
    }

    void SetLeftUnavailable() {
        int i;

        mb.left_available = 0;
        for (i = -1; i < 16; i++) {
            data_ptr[stride * i - 1] = 129;
        }
    }

    void SetTopUnavailable() {
        mb.up_available = 0;
        memset(&data_ptr[-1 - stride], 127, 18);
    }

    void SetTopLeftUnavailable() {
        SetLeftUnavailable();
        SetTopUnavailable();
    }

    void Predict(MB_PREDICTION_MODE mode) {
        mb.mode_info_context->mbmi.mode = mode;
        pred_fn(&mb, data_ptr - stride, data_ptr - 1, stride, data_ptr, stride);
    }

    intra_pred_y_fn_t pred_fn;
    MACROBLOCKD mb;
    MODE_INFO mi;
    // we use 48 so that the data pointer of the first pixel in each row of
    // each macroblock is 16-byte aligned, and this gives us access to the
    // top-left and top-right corner pixels belonging to the top-left/right
    // macroblocks.
    // We use 17 lines so we have one line above us for top-prediction.
    DECLARE_ALIGNED(16, unsigned char, data_array[48 * 17]);
    unsigned char *data_ptr;
    int stride;
};

TEST_P(IntraPredYTest, DC_PRED)
{
    int x, y;

    Predict(DC_PRED);
    for (y = 1; y < 16; y++)
        EXPECT_EQ(0, memcmp(data_ptr, &data_ptr[y * stride], 16));
    for (x = 1; x < 16; x++)
        EXPECT_EQ(true, data_ptr[x] == data_ptr[0]);
    EXPECT_EQ(0x70, data_ptr[0]);
}

TEST_P(IntraPredYTest, DC_PRED_TOP)
{
    int x, y;

    SetLeftUnavailable();
    Predict(DC_PRED);
    for (y = 1; y < 16; y++)
        EXPECT_EQ(0, memcmp(data_ptr, &data_ptr[y * stride], 16));
    for (x = 1; x < 16; x++)
        EXPECT_EQ(true, data_ptr[x] == data_ptr[0]);
    EXPECT_EQ(0x68, data_ptr[0]);
}

TEST_P(IntraPredYTest, DC_PRED_LEFT)
{
    int x, y;

    SetTopUnavailable();
    Predict(DC_PRED);
    for (y = 1; y < 16; y++)
        EXPECT_EQ(0, memcmp(data_ptr, &data_ptr[y * stride], 16));
    for (x = 1; x < 16; x++)
        EXPECT_EQ(true, data_ptr[x] == data_ptr[0]);
    EXPECT_EQ(0x78, data_ptr[0]);
}

TEST_P(IntraPredYTest, DC_PRED_TOPLEFT)
{
    int x, y;

    SetTopLeftUnavailable();
    Predict(DC_PRED);
    for (y = 0; y < 16; y++)
        for (x = 0; x < 16; x++)
            EXPECT_EQ(true, data_ptr[y * stride + x] == data_ptr[0]);

    EXPECT_EQ(0x80, data_ptr[0]);
}

TEST_P(IntraPredYTest, H_PRED)
{
    int x, y;

    Predict(H_PRED);
    for (y = 0; y < 16; y++)
        for (x = 0; x < 16; x++)
            EXPECT_EQ(true, data_ptr[y * stride + x] == data_ptr[y * stride - 1]);
}

TEST_P(IntraPredYTest, V_PRED)
{
    int y;

    Predict(V_PRED);
    for (y = 0; y < 16; y++)
        EXPECT_EQ(0, memcmp(&data_ptr[stride * y], &data_ptr[-stride], 16));
}

TEST_P(IntraPredYTest, TM_PRED)
{
    int x, y;

    Predict(TM_PRED);
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            int expected = data_ptr[x - stride]
                         + data_ptr[stride * y - 1]
                         - data_ptr[-1 - stride];
            if (expected < 0)
                expected = 0;
            else if (expected > 255)
                expected = 255;
            EXPECT_EQ(true, data_ptr[y * stride + x] == expected);
        }
    }
}

INSTANTIATE_TEST_CASE_P(C, IntraPredYTest,
                        ::testing::Values(vp8_build_intra_predictors_mby_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredYTest,
                        ::testing::Values(vp8_build_intra_predictors_mby_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredYTest,
                        ::testing::Values(vp8_build_intra_predictors_mby_s_ssse3));
#endif

typedef void (*intra_pred_uv_fn_t)(MACROBLOCKD *x,
                                   unsigned char * uabove_row,
                                   unsigned char * vabove_row,
                                   unsigned char * uleft,
                                   unsigned char * vleft,
                                   int left_stride,
                                   unsigned char * upred_ptr,
                                   unsigned char * vpred_ptr,
                                   int pred_stride);

class IntraPredUVTest : public ::testing::TestWithParam<intra_pred_uv_fn_t>
{
protected:
    virtual void SetUp()
    {
        int i, j;

        pred_fn = GetParam();
        memset(&mb, 0, sizeof(mb));
        memset(&mi, 0, sizeof(mi));
        mb.up_available = 1;
        mb.left_available = 1;
        mb.mode_info_context = &mi;
        stride = 24;
        for (j = 0; j < 2; j++) {
            data_ptr[j] = data_array[j] + 8 + stride;
            for (i = 0; i < 8; i++) {
                data_ptr[j][i - stride] = 0x60 + i + (i >= 4); // top
                data_ptr[j][stride * i - 1] = 0x80 - i - (i >= 4); // left
            }
            data_ptr[j][-1 - stride] = 0x40; // top-left
            data_ptr[j][8 - stride] = 0xc0; // top-right
        }
    }

    void SetLeftUnavailable() {
        int i;

        mb.left_available = 0;
        for (i = -1; i < 8; i++) {
            data_ptr[0][stride * i - 1] = 129;
            data_ptr[1][stride * i - 1] = 129;
        }
    }

    void SetTopUnavailable() {
        mb.up_available = 0;
        memset(&data_ptr[0][-1 - stride], 127, 10);
        memset(&data_ptr[1][-1 - stride], 127, 10);
    }

    void SetTopLeftUnavailable() {
        SetLeftUnavailable();
        SetTopUnavailable();
    }

    void Predict(MB_PREDICTION_MODE mode) {
        mb.mode_info_context->mbmi.uv_mode = mode;
        pred_fn(&mb, data_ptr[0] - stride, data_ptr[1] - stride,
                data_ptr[0] - 1, data_ptr[1] - 1, stride,
                data_ptr[0], data_ptr[1], stride);
    }

    intra_pred_uv_fn_t pred_fn;
    MACROBLOCKD mb;
    MODE_INFO mi;
    // we use 24 so that the data pointer of the first pixel in each row of
    // each macroblock is 8-byte aligned, and this gives us access to the
    // top-left and top-right corner pixels belonging to the top-left/right
    // macroblocks.
    // We use 9 lines so we have one line above us for top-prediction.
    // [0] = U, [1] = V
    DECLARE_ALIGNED(16, unsigned char, data_array[2][24 * 9]);
    unsigned char *data_ptr[2];
    int stride;
};

TEST_P(IntraPredUVTest, DC_PRED)
{
    int x, y, p;

    Predict(DC_PRED);
    for (p = 0; p < 2; p++) {
        for (y = 1; y < 8; y++)
            EXPECT_EQ(0, memcmp(data_ptr[p], &data_ptr[p][y * stride], 8));
        for (x = 1; x < 8; x++)
            EXPECT_EQ(true, data_ptr[p][x] == data_ptr[p][0]);
        EXPECT_EQ(0x70, data_ptr[p][0]);
    }
}

TEST_P(IntraPredUVTest, DC_PRED_TOP)
{
    int x, y, p;

    SetLeftUnavailable();
    Predict(DC_PRED);
    for (p = 0; p < 2; p++) {
        for (y = 1; y < 8; y++)
            EXPECT_EQ(0, memcmp(data_ptr[p], &data_ptr[p][y * stride], 8));
        for (x = 1; x < 8; x++)
            EXPECT_EQ(true, data_ptr[p][x] == data_ptr[p][0]);
        EXPECT_EQ(0x64, data_ptr[p][0]);
    }
}

TEST_P(IntraPredUVTest, DC_PRED_LEFT)
{
    int x, y, p;

    SetTopUnavailable();
    Predict(DC_PRED);
    for (p = 0; p < 2; p++) {
        for (y = 1; y < 8; y++)
            EXPECT_EQ(0, memcmp(data_ptr[p], &data_ptr[p][y * stride], 8));
        for (x = 1; x < 8; x++)
            EXPECT_EQ(true, data_ptr[p][x] == data_ptr[p][0]);
        EXPECT_EQ(0x7C, data_ptr[p][0]);
    }
}

TEST_P(IntraPredUVTest, DC_PRED_TOPLEFT)
{
    int x, y, p;

    SetTopLeftUnavailable();
    Predict(DC_PRED);
    for (p = 0; p < 2; p++) {
        for (y = 1; y < 8; y++)
            EXPECT_EQ(0, memcmp(data_ptr[p], &data_ptr[p][y * stride], 8));
        for (x = 1; x < 8; x++)
            EXPECT_EQ(true, data_ptr[p][x] == data_ptr[p][0]);
        EXPECT_EQ(0x80, data_ptr[p][0]);
    }
}

TEST_P(IntraPredUVTest, H_PRED)
{
    int x, y, p;

    Predict(H_PRED);
    for (p = 0; p < 2; p++)
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
                EXPECT_EQ(true, data_ptr[p][y * stride + x] == data_ptr[p][y * stride - 1]);
}

TEST_P(IntraPredUVTest, V_PRED)
{
    int y, p;

    Predict(V_PRED);
    for (p = 0; p < 2; p++)
        for (y = 0; y < 8; y++)
            EXPECT_EQ(0, memcmp(&data_ptr[p][stride * y], &data_ptr[p][-stride], 8));
}

TEST_P(IntraPredUVTest, TM_PRED)
{
    int x, y, p;

    Predict(TM_PRED);
    for (p = 0; p < 2; p++) {
        for (y = 0; y < 8; y++) {
            for (x = 0; x < 8; x++) {
                int expected = data_ptr[p][x - stride]
                             + data_ptr[p][stride * y - 1]
                             - data_ptr[p][-1 - stride];
                if (expected < 0)
                    expected = 0;
                else if (expected > 255)
                    expected = 255;
                EXPECT_EQ(true, data_ptr[p][y * stride + x] == expected);
            }
        }
    }
}

INSTANTIATE_TEST_CASE_P(C, IntraPredUVTest,
                        ::testing::Values(vp8_build_intra_predictors_mbuv_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredUVTest,
                        ::testing::Values(vp8_build_intra_predictors_mbuv_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredUVTest,
                        ::testing::Values(vp8_build_intra_predictors_mbuv_s_ssse3));
#endif

}
