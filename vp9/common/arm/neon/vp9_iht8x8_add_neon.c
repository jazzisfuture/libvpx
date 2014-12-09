/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>
#include <assert.h>

static int16_t cospi_2_64 = 16305;
static int16_t cospi_4_64 = 16069;
static int16_t cospi_6_64 = 15679;
static int16_t cospi_8_64 = 15137;
static int16_t cospi_10_64 = 14449;
static int16_t cospi_12_64 = 13623;
static int16_t cospi_14_64 = 12665;
static int16_t cospi_16_64 = 11585;
static int16_t cospi_18_64 = 10394;
static int16_t cospi_20_64 = 9102;
static int16_t cospi_22_64 = 7723;
static int16_t cospi_24_64 = 6270;
static int16_t cospi_26_64 = 4756;
static int16_t cospi_28_64 = 3196;
static int16_t cospi_30_64 = 1606;

static inline void transpose8x8(int16x8_t *output0,
                                int16x8_t *output1,
                                int16x8_t *output2,
                                int16x8_t *output3,
                                int16x8_t *output4,
                                int16x8_t *output5,
                                int16x8_t *output6,
                                int16x8_t *output7) {
  int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3,
    v_s16x4_4, v_s16x4_5, v_s16x4_6, v_s16x4_7,
    v_s16x4_8, v_s16x4_9, v_s16x4_10, v_s16x4_11,
    v_s16x4_12, v_s16x4_13, v_s16x4_14, v_s16x4_15;
  int32x4x2_t m_s32x4x2_0, m_s32x4x2_1, m_s32x4x2_2, m_s32x4x2_3;
  int16x8x2_t m_s16x8x2_0, m_s16x8x2_1, m_s16x8x2_2, m_s16x8x2_3;

  v_s16x4_0 = vget_low_s16(*output0);
  v_s16x4_1 = vget_high_s16(*output0);
  v_s16x4_2 = vget_low_s16(*output1);
  v_s16x4_3 = vget_high_s16(*output1);
  v_s16x4_4 = vget_low_s16(*output2);
  v_s16x4_5 = vget_high_s16(*output2);
  v_s16x4_6 = vget_low_s16(*output3);
  v_s16x4_7 = vget_high_s16(*output3);
  v_s16x4_8 = vget_low_s16(*output4);
  v_s16x4_9 = vget_high_s16(*output4);
  v_s16x4_10 = vget_low_s16(*output5);
  v_s16x4_11 = vget_high_s16(*output5);
  v_s16x4_12 = vget_low_s16(*output6);
  v_s16x4_13 = vget_high_s16(*output6);
  v_s16x4_14 = vget_low_s16(*output7);
  v_s16x4_15 = vget_high_s16(*output7);

  *output0  = vcombine_s16(v_s16x4_0, v_s16x4_8);
  *output1  = vcombine_s16(v_s16x4_2, v_s16x4_10);
  *output2 = vcombine_s16(v_s16x4_4, v_s16x4_12);
  *output3 = vcombine_s16(v_s16x4_6, v_s16x4_14);
  *output4 = vcombine_s16(v_s16x4_1, v_s16x4_9);
  *output5 = vcombine_s16(v_s16x4_3, v_s16x4_11);
  *output6 = vcombine_s16(v_s16x4_5, v_s16x4_13);
  *output7 = vcombine_s16(v_s16x4_7, v_s16x4_15);

  m_s32x4x2_0 = vtrnq_s32(vreinterpretq_s32_s16(*output0),
                          vreinterpretq_s32_s16(*output2));
  m_s32x4x2_1 = vtrnq_s32(vreinterpretq_s32_s16(*output1),
                          vreinterpretq_s32_s16(*output3));
  m_s32x4x2_2 = vtrnq_s32(vreinterpretq_s32_s16(*output4),
                          vreinterpretq_s32_s16(*output6));
  m_s32x4x2_3 = vtrnq_s32(vreinterpretq_s32_s16(*output5),
                          vreinterpretq_s32_s16(*output7));

  m_s16x8x2_0 = vtrnq_s16(vreinterpretq_s16_s32(m_s32x4x2_0.val[0]),
                          vreinterpretq_s16_s32(m_s32x4x2_1.val[0]));
  m_s16x8x2_1 = vtrnq_s16(vreinterpretq_s16_s32(m_s32x4x2_0.val[1]),
                          vreinterpretq_s16_s32(m_s32x4x2_1.val[1]));
  m_s16x8x2_2 = vtrnq_s16(vreinterpretq_s16_s32(m_s32x4x2_2.val[0]),
                          vreinterpretq_s16_s32(m_s32x4x2_3.val[0]));
  m_s16x8x2_3 = vtrnq_s16(vreinterpretq_s16_s32(m_s32x4x2_2.val[1]),
                          vreinterpretq_s16_s32(m_s32x4x2_3.val[1]));

  *output0 = m_s16x8x2_0.val[0];
  *output1 = m_s16x8x2_0.val[1];
  *output2 = m_s16x8x2_1.val[0];
  *output3 = m_s16x8x2_1.val[1];
  *output4 = m_s16x8x2_2.val[0];
  *output5 = m_s16x8x2_2.val[1];
  *output6 = m_s16x8x2_3.val[0];
  *output7 = m_s16x8x2_3.val[1];
}

static inline void idct8x8_1d(int16x8_t *output0,
                              int16x8_t *output1,
                              int16x8_t *output2,
                              int16x8_t *output3,
                              int16x8_t *output4,
                              int16x8_t *output5,
                              int16x8_t *output6,
                              int16x8_t *output7) {
  int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3,
    v_s16x4_4, v_s16x4_5, v_s16x4_6, v_s16x4_7,
    v_s16x4_8, v_s16x4_9, v_s16x4_10, v_s16x4_11,
    v_s16x4_12, v_s16x4_13, v_s16x4_14, v_s16x4_15,
    v_s16x4_16, v_s16x4_17, v_s16x4_18, v_s16x4_19,
    v_s16x4_20, v_s16x4_21, v_s16x4_22, v_s16x4_23,
    v_s16x4_24, v_s16x4_25, v_s16x4_26, v_s16x4_27;
  int16x8_t v_s16x8_0, v_s16x8_1, v_s16x8_2, v_s16x8_3,
    v_s16x8_4, v_s16x8_5, v_s16x8_6, v_s16x8_7;
  int32x4_t v_s32x4_0, v_s32x4_1, v_s32x4_2, v_s32x4_3, v_s32x4_4, v_s32x4_5;
  int32x4_t v_s32x4_6, v_s32x4_7, v_s32x4_8, v_s32x4_9, v_s32x4_10;

  v_s16x4_0 = vdup_n_s16(cospi_28_64);
  v_s16x4_1 = vdup_n_s16(cospi_4_64);
  v_s16x4_2 = vdup_n_s16(cospi_12_64);
  v_s16x4_3 = vdup_n_s16(cospi_20_64);

  v_s16x4_12 = vget_low_s16(*output0);
  v_s16x4_13 = vget_high_s16(*output0);
  v_s16x4_14 = vget_low_s16(*output1);
  v_s16x4_15 = vget_high_s16(*output1);
  v_s16x4_16 = vget_low_s16(*output2);
  v_s16x4_17 = vget_high_s16(*output2);
  v_s16x4_18 = vget_low_s16(*output3);
  v_s16x4_19 = vget_high_s16(*output3);
  v_s16x4_20 = vget_low_s16(*output4);
  v_s16x4_21 = vget_high_s16(*output4);
  v_s16x4_22 = vget_low_s16(*output5);
  v_s16x4_23 = vget_high_s16(*output5);
  v_s16x4_24 = vget_low_s16(*output6);
  v_s16x4_25 = vget_high_s16(*output6);
  v_s16x4_26 = vget_low_s16(*output7);
  v_s16x4_27 = vget_high_s16(*output7);

  v_s32x4_0 = vmull_s16(v_s16x4_14, v_s16x4_0);
  v_s32x4_1 = vmull_s16(v_s16x4_15, v_s16x4_0);
  v_s32x4_2 = vmull_s16(v_s16x4_22, v_s16x4_2);
  v_s32x4_3 = vmull_s16(v_s16x4_23, v_s16x4_2);

  v_s32x4_0 = vmlsl_s16(v_s32x4_0, v_s16x4_26, v_s16x4_1);
  v_s32x4_1 = vmlsl_s16(v_s32x4_1, v_s16x4_27, v_s16x4_1);
  v_s32x4_2 = vmlsl_s16(v_s32x4_2, v_s16x4_18, v_s16x4_3);
  v_s32x4_3 = vmlsl_s16(v_s32x4_3, v_s16x4_19, v_s16x4_3);

  v_s16x4_4  = vqrshrn_n_s32(v_s32x4_0, 14);
  v_s16x4_5  = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_6 = vqrshrn_n_s32(v_s32x4_2, 14);
  v_s16x4_7 = vqrshrn_n_s32(v_s32x4_3, 14);
  v_s16x8_4 = vcombine_s16(v_s16x4_4, v_s16x4_5);
  v_s16x8_5 = vcombine_s16(v_s16x4_6, v_s16x4_7);

  v_s32x4_0 = vmull_s16(v_s16x4_14, v_s16x4_1);
  v_s32x4_1 = vmull_s16(v_s16x4_15, v_s16x4_1);
  v_s32x4_5 = vmull_s16(v_s16x4_22, v_s16x4_3);
  v_s32x4_9 = vmull_s16(v_s16x4_23, v_s16x4_3);

  v_s32x4_0 = vmlal_s16(v_s32x4_0, v_s16x4_26, v_s16x4_0);
  v_s32x4_1 = vmlal_s16(v_s32x4_1, v_s16x4_27, v_s16x4_0);
  v_s32x4_5 = vmlal_s16(v_s32x4_5, v_s16x4_18, v_s16x4_2);
  v_s32x4_9 = vmlal_s16(v_s32x4_9, v_s16x4_19, v_s16x4_2);

  v_s16x4_10 = vqrshrn_n_s32(v_s32x4_0, 14);
  v_s16x4_11 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_8 = vqrshrn_n_s32(v_s32x4_5, 14);
  v_s16x4_9 = vqrshrn_n_s32(v_s32x4_9, 14);
  v_s16x8_6 = vcombine_s16(v_s16x4_8, v_s16x4_9);
  v_s16x8_7 = vcombine_s16(v_s16x4_10, v_s16x4_11);

  v_s16x4_0 = vdup_n_s16(cospi_16_64);

  v_s32x4_0 = vmull_s16(v_s16x4_12, v_s16x4_0);
  v_s32x4_1 = vmull_s16(v_s16x4_13, v_s16x4_0);
  v_s32x4_9 = vmull_s16(v_s16x4_12, v_s16x4_0);
  v_s32x4_10 = vmull_s16(v_s16x4_13, v_s16x4_0);

  v_s32x4_0 = vmlal_s16(v_s32x4_0, v_s16x4_20, v_s16x4_0);
  v_s32x4_1 = vmlal_s16(v_s32x4_1, v_s16x4_21, v_s16x4_0);
  v_s32x4_9 = vmlsl_s16(v_s32x4_9, v_s16x4_20, v_s16x4_0);
  v_s32x4_10 = vmlsl_s16(v_s32x4_10, v_s16x4_21, v_s16x4_0);

  v_s16x4_0 = vdup_n_s16(cospi_24_64);
  v_s16x4_1 = vdup_n_s16(cospi_8_64);

  v_s16x4_14 = vqrshrn_n_s32(v_s32x4_0, 14);
  v_s16x4_15 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_18 = vqrshrn_n_s32(v_s32x4_9, 14);
  v_s16x4_19 = vqrshrn_n_s32(v_s32x4_10, 14);
  *output1  = vcombine_s16(v_s16x4_14, v_s16x4_15);
  *output3 = vcombine_s16(v_s16x4_18, v_s16x4_19);

  v_s32x4_0 = vmull_s16(v_s16x4_16, v_s16x4_0);
  v_s32x4_1 = vmull_s16(v_s16x4_17, v_s16x4_0);
  v_s32x4_4 = vmull_s16(v_s16x4_16, v_s16x4_1);
  v_s32x4_8 = vmull_s16(v_s16x4_17, v_s16x4_1);

  v_s32x4_0 = vmlsl_s16(v_s32x4_0, v_s16x4_24, v_s16x4_1);
  v_s32x4_1 = vmlsl_s16(v_s32x4_1, v_s16x4_25, v_s16x4_1);
  v_s32x4_4 = vmlal_s16(v_s32x4_4, v_s16x4_24, v_s16x4_0);
  v_s32x4_8 = vmlal_s16(v_s32x4_8, v_s16x4_25, v_s16x4_0);

  v_s16x4_22 = vqrshrn_n_s32(v_s32x4_0, 14);
  v_s16x4_23 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_26 = vqrshrn_n_s32(v_s32x4_4, 14);
  v_s16x4_27 = vqrshrn_n_s32(v_s32x4_8, 14);
  *output5 = vcombine_s16(v_s16x4_22, v_s16x4_23);
  *output7 = vcombine_s16(v_s16x4_26, v_s16x4_27);

  v_s16x8_0 = vaddq_s16(*output1, *output7);
  v_s16x8_1 = vaddq_s16(*output3, *output5);
  v_s16x8_2 = vsubq_s16(*output3, *output5);
  v_s16x8_3 = vsubq_s16(*output1, *output7);

  *output5 = vsubq_s16(v_s16x8_4, v_s16x8_5);
  v_s16x8_4   = vaddq_s16(v_s16x8_4, v_s16x8_5);
  *output6 = vsubq_s16(v_s16x8_7, v_s16x8_6);
  v_s16x8_7   = vaddq_s16(v_s16x8_7, v_s16x8_6);
  v_s16x4_22 = vget_low_s16(*output5);
  v_s16x4_23 = vget_high_s16(*output5);
  v_s16x4_24 = vget_low_s16(*output6);
  v_s16x4_25 = vget_high_s16(*output6);

  v_s16x4_12 = vdup_n_s16(cospi_16_64);

  v_s32x4_5  = vmull_s16(v_s16x4_24, v_s16x4_12);
  v_s32x4_6 = vmull_s16(v_s16x4_25, v_s16x4_12);
  v_s32x4_7 = vmull_s16(v_s16x4_24, v_s16x4_12);
  v_s32x4_8 = vmull_s16(v_s16x4_25, v_s16x4_12);

  v_s32x4_5  = vmlsl_s16(v_s32x4_5,  v_s16x4_22, v_s16x4_12);
  v_s32x4_6 = vmlsl_s16(v_s32x4_6, v_s16x4_23, v_s16x4_12);
  v_s32x4_7 = vmlal_s16(v_s32x4_7, v_s16x4_22, v_s16x4_12);
  v_s32x4_8 = vmlal_s16(v_s32x4_8, v_s16x4_23, v_s16x4_12);

  v_s16x4_6 = vqrshrn_n_s32(v_s32x4_5, 14);
  v_s16x4_7 = vqrshrn_n_s32(v_s32x4_6, 14);
  v_s16x4_8 = vqrshrn_n_s32(v_s32x4_7, 14);
  v_s16x4_9 = vqrshrn_n_s32(v_s32x4_8, 14);
  v_s16x8_5 = vcombine_s16(v_s16x4_6, v_s16x4_7);
  v_s16x8_6 = vcombine_s16(v_s16x4_8, v_s16x4_9);

  *output0  = vaddq_s16(v_s16x8_0, v_s16x8_7);
  *output1  = vaddq_s16(v_s16x8_1, v_s16x8_6);
  *output2 = vaddq_s16(v_s16x8_2, v_s16x8_5);
  *output3 = vaddq_s16(v_s16x8_3, v_s16x8_4);
  *output4 = vsubq_s16(v_s16x8_3, v_s16x8_4);
  *output5 = vsubq_s16(v_s16x8_2, v_s16x8_5);
  *output6 = vsubq_s16(v_s16x8_1, v_s16x8_6);
  *output7 = vsubq_s16(v_s16x8_0, v_s16x8_7);
}

static inline void iadst8x8_1d(int16x8_t *output0,
                               int16x8_t *output1,
                               int16x8_t *output2,
                               int16x8_t *output3,
                               int16x8_t *output4,
                               int16x8_t *output5,
                               int16x8_t *output6,
                               int16x8_t *output7) {
  int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3,
    v_s16x4_4, v_s16x4_5, v_s16x4_6, v_s16x4_7,
    v_s16x4_8, v_s16x4_9, v_s16x4_10, v_s16x4_11,
    v_s16x4_12, v_s16x4_13, v_s16x4_14, v_s16x4_15,
    v_s16x4_16, v_s16x4_17, v_s16x4_18, v_s16x4_19,
    v_s16x4_20, v_s16x4_21, v_s16x4_22, v_s16x4_23,
    v_s16x4_24, v_s16x4_25, v_s16x4_26, v_s16x4_27,
    v_s16x4_28, v_s16x4_29, v_s16x4_30, v_s16x4_31;
  int16x8_t v_s16x8_0, v_s16x8_1, v_s16x8_2, v_s16x8_3;
  int32x4_t v_s32x4_0, v_s32x4_1, v_s32x4_2, v_s32x4_3,
    v_s32x4_4, v_s32x4_5, v_s32x4_6, v_s32x4_7,
    v_s32x4_8, v_s32x4_9, v_s32x4_10, v_s32x4_11,
    v_s32x4_12, v_s32x4_13, v_s32x4_14, v_s32x4_15;

  v_s16x4_16 = vget_low_s16(*output0);
  v_s16x4_17 = vget_high_s16(*output0);
  v_s16x4_18 = vget_low_s16(*output1);
  v_s16x4_19 = vget_high_s16(*output1);
  v_s16x4_20 = vget_low_s16(*output2);
  v_s16x4_21 = vget_high_s16(*output2);
  v_s16x4_22 = vget_low_s16(*output3);
  v_s16x4_23 = vget_high_s16(*output3);
  v_s16x4_24 = vget_low_s16(*output4);
  v_s16x4_25 = vget_high_s16(*output4);
  v_s16x4_26 = vget_low_s16(*output5);
  v_s16x4_27 = vget_high_s16(*output5);
  v_s16x4_28 = vget_low_s16(*output6);
  v_s16x4_29 = vget_high_s16(*output6);
  v_s16x4_30 = vget_low_s16(*output7);
  v_s16x4_31 = vget_high_s16(*output7);

  v_s16x4_14 = vdup_n_s16(cospi_2_64);
  v_s16x4_15 = vdup_n_s16(cospi_30_64);

  v_s32x4_1 = vmull_s16(v_s16x4_30, v_s16x4_14);
  v_s32x4_2 = vmull_s16(v_s16x4_31, v_s16x4_14);
  v_s32x4_3 = vmull_s16(v_s16x4_30, v_s16x4_15);
  v_s32x4_4 = vmull_s16(v_s16x4_31, v_s16x4_15);

  v_s16x4_30 = vdup_n_s16(cospi_18_64);
  v_s16x4_31 = vdup_n_s16(cospi_14_64);

  v_s32x4_1 = vmlal_s16(v_s32x4_1, v_s16x4_16, v_s16x4_15);
  v_s32x4_2 = vmlal_s16(v_s32x4_2, v_s16x4_17, v_s16x4_15);
  v_s32x4_3 = vmlsl_s16(v_s32x4_3, v_s16x4_16, v_s16x4_14);
  v_s32x4_4 = vmlsl_s16(v_s32x4_4, v_s16x4_17, v_s16x4_14);

  v_s32x4_5 = vmull_s16(v_s16x4_22, v_s16x4_30);
  v_s32x4_6 = vmull_s16(v_s16x4_23, v_s16x4_30);
  v_s32x4_7 = vmull_s16(v_s16x4_22, v_s16x4_31);
  v_s32x4_8 = vmull_s16(v_s16x4_23, v_s16x4_31);

  v_s32x4_5 = vmlal_s16(v_s32x4_5, v_s16x4_24, v_s16x4_31);
  v_s32x4_6 = vmlal_s16(v_s32x4_6, v_s16x4_25, v_s16x4_31);
  v_s32x4_7 = vmlsl_s16(v_s32x4_7, v_s16x4_24, v_s16x4_30);
  v_s32x4_8 = vmlsl_s16(v_s32x4_8, v_s16x4_25, v_s16x4_30);

  v_s32x4_11 = vaddq_s32(v_s32x4_1, v_s32x4_5);
  v_s32x4_12 = vaddq_s32(v_s32x4_2, v_s32x4_6);
  v_s32x4_1 = vsubq_s32(v_s32x4_1, v_s32x4_5);
  v_s32x4_2 = vsubq_s32(v_s32x4_2, v_s32x4_6);

  v_s16x4_22 = vqrshrn_n_s32(v_s32x4_11, 14);
  v_s16x4_23 = vqrshrn_n_s32(v_s32x4_12, 14);
  *output3 = vcombine_s16(v_s16x4_22, v_s16x4_23);

  v_s32x4_12 = vaddq_s32(v_s32x4_3, v_s32x4_7);
  v_s32x4_15 = vaddq_s32(v_s32x4_4, v_s32x4_8);
  v_s32x4_3 = vsubq_s32(v_s32x4_3, v_s32x4_7);
  v_s32x4_4 = vsubq_s32(v_s32x4_4, v_s32x4_8);

  v_s16x4_2  = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_3  = vqrshrn_n_s32(v_s32x4_2, 14);
  v_s16x4_24 = vqrshrn_n_s32(v_s32x4_12, 14);
  v_s16x4_25 = vqrshrn_n_s32(v_s32x4_15, 14);
  v_s16x4_6  = vqrshrn_n_s32(v_s32x4_3, 14);
  v_s16x4_7  = vqrshrn_n_s32(v_s32x4_4, 14);
  *output4 = vcombine_s16(v_s16x4_24, v_s16x4_25);

  v_s16x4_0 = vdup_n_s16(cospi_10_64);
  v_s16x4_1 = vdup_n_s16(cospi_22_64);
  v_s32x4_4 = vmull_s16(v_s16x4_26, v_s16x4_0);
  v_s32x4_5 = vmull_s16(v_s16x4_27, v_s16x4_0);
  v_s32x4_2 = vmull_s16(v_s16x4_26, v_s16x4_1);
  v_s32x4_6 = vmull_s16(v_s16x4_27, v_s16x4_1);

  v_s16x4_30 = vdup_n_s16(cospi_26_64);
  v_s16x4_31 = vdup_n_s16(cospi_6_64);

  v_s32x4_4 = vmlal_s16(v_s32x4_4, v_s16x4_20, v_s16x4_1);
  v_s32x4_5 = vmlal_s16(v_s32x4_5, v_s16x4_21, v_s16x4_1);
  v_s32x4_2 = vmlsl_s16(v_s32x4_2, v_s16x4_20, v_s16x4_0);
  v_s32x4_6 = vmlsl_s16(v_s32x4_6, v_s16x4_21, v_s16x4_0);

  v_s32x4_0 = vmull_s16(v_s16x4_18, v_s16x4_30);
  v_s32x4_13 = vmull_s16(v_s16x4_19, v_s16x4_30);

  v_s32x4_0 = vmlal_s16(v_s32x4_0, v_s16x4_28, v_s16x4_31);
  v_s32x4_13 = vmlal_s16(v_s32x4_13, v_s16x4_29, v_s16x4_31);

  v_s32x4_10 = vmull_s16(v_s16x4_18, v_s16x4_31);
  v_s32x4_9 = vmull_s16(v_s16x4_19, v_s16x4_31);

  v_s32x4_10 = vmlsl_s16(v_s32x4_10, v_s16x4_28, v_s16x4_30);
  v_s32x4_9 = vmlsl_s16(v_s32x4_9, v_s16x4_29, v_s16x4_30);

  v_s32x4_14 = vaddq_s32(v_s32x4_2, v_s32x4_10);
  v_s32x4_15 = vaddq_s32(v_s32x4_6, v_s32x4_9);
  v_s32x4_2 = vsubq_s32(v_s32x4_2, v_s32x4_10);
  v_s32x4_6 = vsubq_s32(v_s32x4_6, v_s32x4_9);

  v_s16x4_28 = vqrshrn_n_s32(v_s32x4_14, 14);
  v_s16x4_29 = vqrshrn_n_s32(v_s32x4_15, 14);
  v_s16x4_4 = vqrshrn_n_s32(v_s32x4_2, 14);
  v_s16x4_5 = vqrshrn_n_s32(v_s32x4_6, 14);
  *output6 = vcombine_s16(v_s16x4_28, v_s16x4_29);

  v_s32x4_9 = vaddq_s32(v_s32x4_4, v_s32x4_0);
  v_s32x4_10 = vaddq_s32(v_s32x4_5, v_s32x4_13);
  v_s32x4_4 = vsubq_s32(v_s32x4_4, v_s32x4_0);
  v_s32x4_5 = vsubq_s32(v_s32x4_5, v_s32x4_13);

  v_s16x4_30 = vdup_n_s16(cospi_8_64);
  v_s16x4_31 = vdup_n_s16(cospi_24_64);

  v_s16x4_18 = vqrshrn_n_s32(v_s32x4_9, 14);
  v_s16x4_19 = vqrshrn_n_s32(v_s32x4_10, 14);
  v_s16x4_8 = vqrshrn_n_s32(v_s32x4_4, 14);
  v_s16x4_9 = vqrshrn_n_s32(v_s32x4_5, 14);
  *output1 = vcombine_s16(v_s16x4_18, v_s16x4_19);

  v_s32x4_5 = vmull_s16(v_s16x4_2, v_s16x4_30);
  v_s32x4_6 = vmull_s16(v_s16x4_3, v_s16x4_30);
  v_s32x4_7 = vmull_s16(v_s16x4_2, v_s16x4_31);
  v_s32x4_0 = vmull_s16(v_s16x4_3, v_s16x4_31);

  v_s32x4_5 = vmlal_s16(v_s32x4_5, v_s16x4_6, v_s16x4_31);
  v_s32x4_6 = vmlal_s16(v_s32x4_6, v_s16x4_7, v_s16x4_31);
  v_s32x4_7 = vmlsl_s16(v_s32x4_7, v_s16x4_6, v_s16x4_30);
  v_s32x4_0 = vmlsl_s16(v_s32x4_0, v_s16x4_7, v_s16x4_30);

  v_s32x4_1 = vmull_s16(v_s16x4_4, v_s16x4_30);
  v_s32x4_3 = vmull_s16(v_s16x4_5, v_s16x4_30);
  v_s32x4_10 = vmull_s16(v_s16x4_4, v_s16x4_31);
  v_s32x4_2 = vmull_s16(v_s16x4_5, v_s16x4_31);

  v_s32x4_1 = vmlsl_s16(v_s32x4_1, v_s16x4_8, v_s16x4_31);
  v_s32x4_3 = vmlsl_s16(v_s32x4_3, v_s16x4_9, v_s16x4_31);
  v_s32x4_10 = vmlal_s16(v_s32x4_10, v_s16x4_8, v_s16x4_30);
  v_s32x4_2 = vmlal_s16(v_s32x4_2, v_s16x4_9, v_s16x4_30);

  *output0 = vaddq_s16(*output3, *output1);
  *output3 = vsubq_s16(*output3, *output1);
  v_s16x8_1 = vaddq_s16(*output4, *output6);
  *output4 = vsubq_s16(*output4, *output6);

  v_s32x4_14 = vaddq_s32(v_s32x4_5, v_s32x4_1);
  v_s32x4_15 = vaddq_s32(v_s32x4_6, v_s32x4_3);
  v_s32x4_5 = vsubq_s32(v_s32x4_5, v_s32x4_1);
  v_s32x4_6 = vsubq_s32(v_s32x4_6, v_s32x4_3);

  v_s16x4_18 = vqrshrn_n_s32(v_s32x4_14, 14);
  v_s16x4_19 = vqrshrn_n_s32(v_s32x4_15, 14);
  v_s16x4_10 = vqrshrn_n_s32(v_s32x4_5, 14);
  v_s16x4_11 = vqrshrn_n_s32(v_s32x4_6, 14);
  *output1 = vcombine_s16(v_s16x4_18, v_s16x4_19);

  v_s32x4_1 = vaddq_s32(v_s32x4_7, v_s32x4_10);
  v_s32x4_3 = vaddq_s32(v_s32x4_0, v_s32x4_2);
  v_s32x4_7 = vsubq_s32(v_s32x4_7, v_s32x4_10);
  v_s32x4_0 = vsubq_s32(v_s32x4_0, v_s32x4_2);

  v_s16x4_28 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_29 = vqrshrn_n_s32(v_s32x4_3, 14);
  v_s16x4_14 = vqrshrn_n_s32(v_s32x4_7, 14);
  v_s16x4_15 = vqrshrn_n_s32(v_s32x4_0, 14);
  *output6 = vcombine_s16(v_s16x4_28, v_s16x4_29);

  v_s16x4_30 = vdup_n_s16(cospi_16_64);

  v_s16x4_22 = vget_low_s16(*output3);
  v_s16x4_23 = vget_high_s16(*output3);
  v_s32x4_2 = vmull_s16(v_s16x4_22, v_s16x4_30);
  v_s32x4_3 = vmull_s16(v_s16x4_23, v_s16x4_30);
  v_s32x4_13 = vmull_s16(v_s16x4_22, v_s16x4_30);
  v_s32x4_1 = vmull_s16(v_s16x4_23, v_s16x4_30);

  v_s16x4_24 = vget_low_s16(*output4);
  v_s16x4_25 = vget_high_s16(*output4);
  v_s32x4_2 = vmlal_s16(v_s32x4_2, v_s16x4_24, v_s16x4_30);
  v_s32x4_3 = vmlal_s16(v_s32x4_3, v_s16x4_25, v_s16x4_30);
  v_s32x4_13 = vmlsl_s16(v_s32x4_13, v_s16x4_24, v_s16x4_30);
  v_s32x4_1 = vmlsl_s16(v_s32x4_1, v_s16x4_25, v_s16x4_30);

  v_s16x4_4 = vqrshrn_n_s32(v_s32x4_2, 14);
  v_s16x4_5 = vqrshrn_n_s32(v_s32x4_3, 14);
  v_s16x4_24 = vqrshrn_n_s32(v_s32x4_13, 14);
  v_s16x4_25 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x8_0 = vcombine_s16(v_s16x4_4, v_s16x4_5);
  *output4 = vcombine_s16(v_s16x4_24, v_s16x4_25);

  v_s32x4_13 = vmull_s16(v_s16x4_10, v_s16x4_30);
  v_s32x4_1 = vmull_s16(v_s16x4_11, v_s16x4_30);
  v_s32x4_11 = vmull_s16(v_s16x4_10, v_s16x4_30);
  v_s32x4_0 = vmull_s16(v_s16x4_11, v_s16x4_30);

  v_s32x4_13 = vmlal_s16(v_s32x4_13, v_s16x4_14, v_s16x4_30);
  v_s32x4_1 = vmlal_s16(v_s32x4_1, v_s16x4_15, v_s16x4_30);
  v_s32x4_11 = vmlsl_s16(v_s32x4_11, v_s16x4_14, v_s16x4_30);
  v_s32x4_0 = vmlsl_s16(v_s32x4_0, v_s16x4_15, v_s16x4_30);

  v_s16x4_20 = vqrshrn_n_s32(v_s32x4_13, 14);
  v_s16x4_21 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_12 = vqrshrn_n_s32(v_s32x4_11, 14);
  v_s16x4_13 = vqrshrn_n_s32(v_s32x4_0, 14);
  *output2 = vcombine_s16(v_s16x4_20, v_s16x4_21);
  v_s16x8_3 = vcombine_s16(v_s16x4_12, v_s16x4_13);

  v_s16x8_2 = vdupq_n_s16(0);

  *output1  = vsubq_s16(v_s16x8_2, *output1);
  *output3 = vsubq_s16(v_s16x8_2, v_s16x8_0);
  *output5 = vsubq_s16(v_s16x8_2, v_s16x8_3);
  *output7 = vsubq_s16(v_s16x8_2, v_s16x8_1);
}

void vp9_iht8x8_64_add_neon(int16_t *src,
                            uint8_t *dst,
                            int dst_stride,
                            int tx_type) {
  int i;
  uint8_t *d1, *d2;
  uint8x8_t v_u8x8_0, v_u8x8_1, v_u8x8_2, v_u8x8_3;
  uint64x1_t v_u64x1_0, v_u64x1_1, v_u64x1_2, v_u64x1_3;
  int16x8_t v_s16x8_0, v_s16x8_1, v_s16x8_2, v_s16x8_3,
    v_s16x8_4, v_s16x8_5, v_s16x8_6, v_s16x8_7;
  uint16x8_t v_u16x8_0, v_u16x8_1, v_u16x8_2, v_u16x8_3;

  v_s16x8_0 = vld1q_s16(src);
  v_s16x8_1 = vld1q_s16(src + 8);
  v_s16x8_2 = vld1q_s16(src + 8 * 2);
  v_s16x8_3 = vld1q_s16(src + 8 * 3);
  v_s16x8_4 = vld1q_s16(src + 8 * 4);
  v_s16x8_5 = vld1q_s16(src + 8 * 5);
  v_s16x8_6 = vld1q_s16(src + 8 * 6);
  v_s16x8_7 = vld1q_s16(src + 8 * 7);

  transpose8x8(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
               &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

  switch (tx_type) {
    case 0:  // idct_idct is not supported. Fall back to C
      vp9_iht8x8_64_add_c(src, dst, dst_stride, tx_type);
      return;
      break;
    case 1:  // iadst_idct
      // generate IDCT constants
      // GENERATE_IDCT_CONSTANTS

      // first transform rows
      idct8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                 &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // transpose the matrix
      transpose8x8(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                   &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // generate IADST constants
      // GENERATE_IADST_CONSTANTS

      // then transform columns
      iadst8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                  &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);
      break;
    case 2:  // idct_iadst
      // generate IADST constants
      // GENERATE_IADST_CONSTANTS

      // first transform rows
      iadst8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                  &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // transpose the matrix
      transpose8x8(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                   &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // generate IDCT constants
      // GENERATE_IDCT_CONSTANTS

      // then transform columns
      idct8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                 &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);
      break;
    case 3:  // iadst_iadst
      // generate IADST constants
      // GENERATE_IADST_CONSTANTS

      // first transform rows
      iadst8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                  &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // transpose the matrix
      transpose8x8(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                   &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);

      // then transform columns
      iadst8x8_1d(&v_s16x8_0, &v_s16x8_1, &v_s16x8_2, &v_s16x8_3,
                  &v_s16x8_4, &v_s16x8_5, &v_s16x8_6, &v_s16x8_7);
      break;
    default:  // iadst_idct
      assert(0);
      break;
  }

  v_s16x8_0 = vrshrq_n_s16(v_s16x8_0, 5);
  v_s16x8_1 = vrshrq_n_s16(v_s16x8_1, 5);
  v_s16x8_2 = vrshrq_n_s16(v_s16x8_2, 5);
  v_s16x8_3 = vrshrq_n_s16(v_s16x8_3, 5);
  v_s16x8_4 = vrshrq_n_s16(v_s16x8_4, 5);
  v_s16x8_5 = vrshrq_n_s16(v_s16x8_5, 5);
  v_s16x8_6 = vrshrq_n_s16(v_s16x8_6, 5);
  v_s16x8_7 = vrshrq_n_s16(v_s16x8_7, 5);

  for (d1 = d2 = dst, i = 0; i < 2; i++) {
    if (i != 0) {
      v_s16x8_0 = v_s16x8_4;
      v_s16x8_1 = v_s16x8_5;
      v_s16x8_2 = v_s16x8_6;
      v_s16x8_3 = v_s16x8_7;
    }

    v_u64x1_0 = vld1_u64((uint64_t *)d1);
    d1 += dst_stride;
    v_u64x1_1 = vld1_u64((uint64_t *)d1);
    d1 += dst_stride;
    v_u64x1_2 = vld1_u64((uint64_t *)d1);
    d1 += dst_stride;
    v_u64x1_3 = vld1_u64((uint64_t *)d1);
    d1 += dst_stride;

    v_u16x8_0 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_0),
                          vreinterpret_u8_u64(v_u64x1_0));
    v_u16x8_1 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_1),
                          vreinterpret_u8_u64(v_u64x1_1));
    v_u16x8_2 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_2),
                         vreinterpret_u8_u64(v_u64x1_2));
    v_u16x8_3 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_3),
                         vreinterpret_u8_u64(v_u64x1_3));

    v_u8x8_0 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_0));
    v_u8x8_1 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_1));
    v_u8x8_2 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_2));
    v_u8x8_3 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_3));

    vst1_u64((uint64_t *)d2, vreinterpret_u64_u8(v_u8x8_0));
    d2 += dst_stride;
    vst1_u64((uint64_t *)d2, vreinterpret_u64_u8(v_u8x8_1));
    d2 += dst_stride;
    vst1_u64((uint64_t *)d2, vreinterpret_u64_u8(v_u8x8_2));
    d2 += dst_stride;
    vst1_u64((uint64_t *)d2, vreinterpret_u64_u8(v_u8x8_3));
    d2 += dst_stride;
  }
}
