/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/idct_neon.h"
#include "vpx_dsp/inv_txfm.h"

static INLINE void highbd_idct16x16_add8x1(int16x8_t res, const int16x8_t max,
                                           uint16_t **dest, const int stride) {
  uint16x8_t d = vld1q_u16(*dest);

  res = vqaddq_s16(res, vreinterpretq_s16_u16(d));
  res = vminq_s16(res, max);
  d = vqshluq_n_s16(res, 0);
  vst1q_u16(*dest, d);
  *dest += stride;
}

static void idct16x16_256_add_half1d_8bit(const void *input, int16_t *output,
                                          uint16_t *dest, int stride) {
  const int16x8_t cospis0 = vld1q_s16(kCospi);
  const int16x8_t cospis1 = vld1q_s16(kCospi + 8);
  const int16x4_t cospi_0_8_16_24 = vget_low_s16(cospis0);
  const int16x4_t cospi_4_12_20N_28 = vget_high_s16(cospis0);
  const int16x4_t cospi_2_30_10_22 = vget_low_s16(cospis1);
  const int16x4_t cospi_6_26_14_18N = vget_high_s16(cospis1);
  int16x8_t in[16], step1[16], step2[16], out[16];

  // Load input (16x8)
  if (output) {
    const tran_low_t *inputT = (const tran_low_t *)input;
    in[0] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[8] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[1] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[9] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[2] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[10] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[3] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[11] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[4] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[12] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[5] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[13] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[6] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[14] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[7] = load_tran_low_to_s16q(inputT);
    inputT += 8;
    in[15] = load_tran_low_to_s16q(inputT);
  } else {
    const int16_t *inputT = (const int16_t *)input;
    in[0] = vld1q_s16(inputT);
    inputT += 8;
    in[8] = vld1q_s16(inputT);
    inputT += 8;
    in[1] = vld1q_s16(inputT);
    inputT += 8;
    in[9] = vld1q_s16(inputT);
    inputT += 8;
    in[2] = vld1q_s16(inputT);
    inputT += 8;
    in[10] = vld1q_s16(inputT);
    inputT += 8;
    in[3] = vld1q_s16(inputT);
    inputT += 8;
    in[11] = vld1q_s16(inputT);
    inputT += 8;
    in[4] = vld1q_s16(inputT);
    inputT += 8;
    in[12] = vld1q_s16(inputT);
    inputT += 8;
    in[5] = vld1q_s16(inputT);
    inputT += 8;
    in[13] = vld1q_s16(inputT);
    inputT += 8;
    in[6] = vld1q_s16(inputT);
    inputT += 8;
    in[14] = vld1q_s16(inputT);
    inputT += 8;
    in[7] = vld1q_s16(inputT);
    inputT += 8;
    in[15] = vld1q_s16(inputT);
  }

  // Transpose
  transpose_s16_8x8(&in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6],
                    &in[7]);
  transpose_s16_8x8(&in[8], &in[9], &in[10], &in[11], &in[12], &in[13], &in[14],
                    &in[15]);

  // stage 1
  step1[0] = in[0 / 2];
  step1[1] = in[16 / 2];
  step1[2] = in[8 / 2];
  step1[3] = in[24 / 2];
  step1[4] = in[4 / 2];
  step1[5] = in[20 / 2];
  step1[6] = in[12 / 2];
  step1[7] = in[28 / 2];
  step1[8] = in[2 / 2];
  step1[9] = in[18 / 2];
  step1[10] = in[10 / 2];
  step1[11] = in[26 / 2];
  step1[12] = in[6 / 2];
  step1[13] = in[22 / 2];
  step1[14] = in[14 / 2];
  step1[15] = in[30 / 2];

  // stage 2
  step2[0] = step1[0];
  step2[1] = step1[1];
  step2[2] = step1[2];
  step2[3] = step1[3];
  step2[4] = step1[4];
  step2[5] = step1[5];
  step2[6] = step1[6];
  step2[7] = step1[7];
  idct_cospi_2_30(step1[8], step1[15], cospi_2_30_10_22, &step2[8], &step2[15]);
  idct_cospi_14_18(step1[9], step1[14], cospi_6_26_14_18N, &step2[9],
                   &step2[14]);
  idct_cospi_10_22(step1[10], step1[13], cospi_2_30_10_22, &step2[10],
                   &step2[13]);
  idct_cospi_6_26(step1[11], step1[12], cospi_6_26_14_18N, &step2[11],
                  &step2[12]);

  // stage 3
  step1[0] = step2[0];
  step1[1] = step2[1];
  step1[2] = step2[2];
  step1[3] = step2[3];
  idct_cospi_4_28(step2[4], step2[7], cospi_4_12_20N_28, &step1[4], &step1[7]);
  idct_cospi_12_20(step2[5], step2[6], cospi_4_12_20N_28, &step1[5], &step1[6]);
  step1[8] = vaddq_s16(step2[8], step2[9]);
  step1[9] = vsubq_s16(step2[8], step2[9]);
  step1[10] = vsubq_s16(step2[11], step2[10]);
  step1[11] = vaddq_s16(step2[11], step2[10]);
  step1[12] = vaddq_s16(step2[12], step2[13]);
  step1[13] = vsubq_s16(step2[12], step2[13]);
  step1[14] = vsubq_s16(step2[15], step2[14]);
  step1[15] = vaddq_s16(step2[15], step2[14]);

  // stage 4
  idct_cospi_16_16_q(step1[1], step1[0], cospi_0_8_16_24, &step2[1], &step2[0]);
  idct_cospi_8_24_q(step1[2], step1[3], cospi_0_8_16_24, &step2[2], &step2[3]);
  step2[4] = vaddq_s16(step1[4], step1[5]);
  step2[5] = vsubq_s16(step1[4], step1[5]);
  step2[6] = vsubq_s16(step1[7], step1[6]);
  step2[7] = vaddq_s16(step1[7], step1[6]);
  step2[8] = step1[8];
  idct_cospi_8_24_q(step1[14], step1[9], cospi_0_8_16_24, &step2[9],
                    &step2[14]);
  idct_cospi_8_24_neg_q(step1[13], step1[10], cospi_0_8_16_24, &step2[13],
                        &step2[10]);
  step2[11] = step1[11];
  step2[12] = step1[12];
  step2[15] = step1[15];

  // stage 5
  step1[0] = vaddq_s16(step2[0], step2[3]);
  step1[1] = vaddq_s16(step2[1], step2[2]);
  step1[2] = vsubq_s16(step2[1], step2[2]);
  step1[3] = vsubq_s16(step2[0], step2[3]);
  step1[4] = step2[4];
  idct_cospi_16_16_q(step2[5], step2[6], cospi_0_8_16_24, &step1[5], &step1[6]);
  step1[7] = step2[7];
  step1[8] = vaddq_s16(step2[8], step2[11]);
  step1[9] = vaddq_s16(step2[9], step2[10]);
  step1[10] = vsubq_s16(step2[9], step2[10]);
  step1[11] = vsubq_s16(step2[8], step2[11]);
  step1[12] = vsubq_s16(step2[15], step2[12]);
  step1[13] = vsubq_s16(step2[14], step2[13]);
  step1[14] = vaddq_s16(step2[14], step2[13]);
  step1[15] = vaddq_s16(step2[15], step2[12]);

  // stage 6
  step2[0] = vaddq_s16(step1[0], step1[7]);
  step2[1] = vaddq_s16(step1[1], step1[6]);
  step2[2] = vaddq_s16(step1[2], step1[5]);
  step2[3] = vaddq_s16(step1[3], step1[4]);
  step2[4] = vsubq_s16(step1[3], step1[4]);
  step2[5] = vsubq_s16(step1[2], step1[5]);
  step2[6] = vsubq_s16(step1[1], step1[6]);
  step2[7] = vsubq_s16(step1[0], step1[7]);
  idct_cospi_16_16_q(step1[10], step1[13], cospi_0_8_16_24, &step2[10],
                     &step2[13]);
  idct_cospi_16_16_q(step1[11], step1[12], cospi_0_8_16_24, &step2[11],
                     &step2[12]);
  step2[8] = step1[8];
  step2[9] = step1[9];
  step2[14] = step1[14];
  step2[15] = step1[15];

  // stage 7
  idct16x16_add_stage7(step2, out);

  if (output) {
    // pass 1: save the result into output
    vst1q_s16(output, out[0]);
    output += 16;
    vst1q_s16(output, out[1]);
    output += 16;
    vst1q_s16(output, out[2]);
    output += 16;
    vst1q_s16(output, out[3]);
    output += 16;
    vst1q_s16(output, out[4]);
    output += 16;
    vst1q_s16(output, out[5]);
    output += 16;
    vst1q_s16(output, out[6]);
    output += 16;
    vst1q_s16(output, out[7]);
    output += 16;
    vst1q_s16(output, out[8]);
    output += 16;
    vst1q_s16(output, out[9]);
    output += 16;
    vst1q_s16(output, out[10]);
    output += 16;
    vst1q_s16(output, out[11]);
    output += 16;
    vst1q_s16(output, out[12]);
    output += 16;
    vst1q_s16(output, out[13]);
    output += 16;
    vst1q_s16(output, out[14]);
    output += 16;
    vst1q_s16(output, out[15]);
  } else {
    // pass 2: add the result to dest.
    const int16x8_t max = vdupq_n_s16((1 << 8) - 1);
    out[0] = vrshrq_n_s16(out[0], 6);
    out[1] = vrshrq_n_s16(out[1], 6);
    out[2] = vrshrq_n_s16(out[2], 6);
    out[3] = vrshrq_n_s16(out[3], 6);
    out[4] = vrshrq_n_s16(out[4], 6);
    out[5] = vrshrq_n_s16(out[5], 6);
    out[6] = vrshrq_n_s16(out[6], 6);
    out[7] = vrshrq_n_s16(out[7], 6);
    out[8] = vrshrq_n_s16(out[8], 6);
    out[9] = vrshrq_n_s16(out[9], 6);
    out[10] = vrshrq_n_s16(out[10], 6);
    out[11] = vrshrq_n_s16(out[11], 6);
    out[12] = vrshrq_n_s16(out[12], 6);
    out[13] = vrshrq_n_s16(out[13], 6);
    out[14] = vrshrq_n_s16(out[14], 6);
    out[15] = vrshrq_n_s16(out[15], 6);
    highbd_idct16x16_add8x1(out[0], max, &dest, stride);
    highbd_idct16x16_add8x1(out[1], max, &dest, stride);
    highbd_idct16x16_add8x1(out[2], max, &dest, stride);
    highbd_idct16x16_add8x1(out[3], max, &dest, stride);
    highbd_idct16x16_add8x1(out[4], max, &dest, stride);
    highbd_idct16x16_add8x1(out[5], max, &dest, stride);
    highbd_idct16x16_add8x1(out[6], max, &dest, stride);
    highbd_idct16x16_add8x1(out[7], max, &dest, stride);
    highbd_idct16x16_add8x1(out[8], max, &dest, stride);
    highbd_idct16x16_add8x1(out[9], max, &dest, stride);
    highbd_idct16x16_add8x1(out[10], max, &dest, stride);
    highbd_idct16x16_add8x1(out[11], max, &dest, stride);
    highbd_idct16x16_add8x1(out[12], max, &dest, stride);
    highbd_idct16x16_add8x1(out[13], max, &dest, stride);
    highbd_idct16x16_add8x1(out[14], max, &dest, stride);
    highbd_idct16x16_add8x1(out[15], max, &dest, stride);
  }
}

static INLINE void highbd_idct16x16_add_wrap_low_8x2(const int64x2x2_t *const t,
                                                     int32x4x2_t *const d0,
                                                     int32x4x2_t *const d1) {
  int32x2x2_t t32[4];

  t32[0].val[0] = vrshrn_n_s64(t[0].val[0], 14);
  t32[0].val[1] = vrshrn_n_s64(t[0].val[1], 14);
  t32[1].val[0] = vrshrn_n_s64(t[1].val[0], 14);
  t32[1].val[1] = vrshrn_n_s64(t[1].val[1], 14);
  t32[2].val[0] = vrshrn_n_s64(t[2].val[0], 14);
  t32[2].val[1] = vrshrn_n_s64(t[2].val[1], 14);
  t32[3].val[0] = vrshrn_n_s64(t[3].val[0], 14);
  t32[3].val[1] = vrshrn_n_s64(t[3].val[1], 14);
  d0->val[0] = vcombine_s32(t32[0].val[0], t32[0].val[1]);
  d0->val[1] = vcombine_s32(t32[1].val[0], t32[1].val[1]);
  d1->val[0] = vcombine_s32(t32[2].val[0], t32[2].val[1]);
  d1->val[1] = vcombine_s32(t32[3].val[0], t32[3].val[1]);
}

static INLINE void highbd_idct_cospi_2_30(const int32x4x2_t s0,
                                          const int32x4x2_t s1,
                                          const int32x4_t cospi_2_30_10_22,
                                          int32x4x2_t *const d0,
                                          int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 1);
  t[0].val[0] = vmlsl_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[0].val[1] = vmlsl_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[1].val[0] = vmlsl_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[1].val[1] = vmlsl_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[2].val[0] = vmlal_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[2].val[1] = vmlal_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[3].val[0] = vmlal_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  t[3].val[1] = vmlal_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_2_30_10_22), 0);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_4_28(const int32x4x2_t s0,
                                          const int32x4x2_t s1,
                                          const int32x4_t cospi_4_12_20N_28,
                                          int32x4x2_t *const d0,
                                          int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 1);
  t[0].val[0] = vmlsl_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[0].val[1] = vmlsl_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[1].val[0] = vmlsl_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[1].val[1] = vmlsl_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[2].val[0] = vmlal_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[2].val[1] = vmlal_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[3].val[0] = vmlal_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  t[3].val[1] = vmlal_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 0);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_6_26(const int32x4x2_t s0,
                                          const int32x4x2_t s1,
                                          const int32x4_t cospi_6_26_14_18N,
                                          int32x4x2_t *const d0,
                                          int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 0);
  t[0].val[0] = vmlal_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[0].val[1] = vmlal_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[1].val[0] = vmlal_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[1].val[1] = vmlal_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[2].val[0] = vmlsl_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[2].val[1] = vmlsl_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[3].val[0] = vmlsl_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  t[3].val[1] = vmlsl_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_6_26_14_18N), 1);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_10_22(const int32x4x2_t s0,
                                           const int32x4x2_t s1,
                                           const int32x4_t cospi_2_30_10_22,
                                           int32x4x2_t *const d0,
                                           int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 1);
  t[0].val[0] = vmlsl_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[0].val[1] = vmlsl_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[1].val[0] = vmlsl_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[1].val[1] = vmlsl_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[2].val[0] = vmlal_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[2].val[1] = vmlal_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[3].val[0] = vmlal_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  t[3].val[1] = vmlal_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_2_30_10_22), 0);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_12_20(const int32x4x2_t s0,
                                           const int32x4x2_t s1,
                                           const int32x4_t cospi_4_12_20N_28,
                                           int32x4x2_t *const d0,
                                           int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_4_12_20N_28), 1);
  t[0].val[0] = vmlal_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[0].val[1] = vmlal_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[1].val[0] = vmlal_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[1].val[1] = vmlal_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[2].val[0] = vmlsl_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[2].val[1] = vmlsl_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[3].val[0] = vmlsl_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  t[3].val[1] = vmlsl_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_4_12_20N_28), 0);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_14_18(const int32x4x2_t s0,
                                           const int32x4x2_t s1,
                                           const int32x4_t cospi_6_26_14_18N,
                                           int32x4x2_t *const d0,
                                           int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 0);
  t[0].val[0] = vmlal_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[0].val[1] = vmlal_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[1].val[0] = vmlal_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[1].val[1] = vmlal_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[2].val[0] = vmlsl_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[2].val[1] = vmlsl_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[3].val[0] = vmlsl_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  t[3].val[1] = vmlsl_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_6_26_14_18N), 1);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_8_24_q_kernel(
    const int32x4x2_t s0, const int32x4x2_t s1, const int32x4_t cospi_0_8_16_24,
    int64x2x2_t *const t) {
  t[0].val[0] = vmull_lane_s32(vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[0].val[1] = vmull_lane_s32(vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[1].val[0] = vmull_lane_s32(vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[1].val[1] = vmull_lane_s32(vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[2].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[2].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[3].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[3].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 1);
  t[0].val[0] = vmlsl_lane_s32(t[0].val[0], vget_low_s32(s1.val[0]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[0].val[1] = vmlsl_lane_s32(t[0].val[1], vget_high_s32(s1.val[0]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[1].val[0] = vmlsl_lane_s32(t[1].val[0], vget_low_s32(s1.val[1]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[1].val[1] = vmlsl_lane_s32(t[1].val[1], vget_high_s32(s1.val[1]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[2].val[0] = vmlal_lane_s32(t[2].val[0], vget_low_s32(s0.val[0]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[2].val[1] = vmlal_lane_s32(t[2].val[1], vget_high_s32(s0.val[0]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[3].val[0] = vmlal_lane_s32(t[3].val[0], vget_low_s32(s0.val[1]),
                               vget_low_s32(cospi_0_8_16_24), 1);
  t[3].val[1] = vmlal_lane_s32(t[3].val[1], vget_high_s32(s0.val[1]),
                               vget_low_s32(cospi_0_8_16_24), 1);
}

static INLINE void highbd_idct_cospi_8_24_q(const int32x4x2_t s0,
                                            const int32x4x2_t s1,
                                            const int32x4_t cospi_0_8_16_24,
                                            int32x4x2_t *const d0,
                                            int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  highbd_idct_cospi_8_24_q_kernel(s0, s1, cospi_0_8_16_24, t);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_8_24_neg_q(const int32x4x2_t s0,
                                                const int32x4x2_t s1,
                                                const int32x4_t cospi_0_8_16_24,
                                                int32x4x2_t *const d0,
                                                int32x4x2_t *const d1) {
  int64x2x2_t t[4];

  highbd_idct_cospi_8_24_q_kernel(s0, s1, cospi_0_8_16_24, t);
  t[2].val[0] = vsubq_s64(vdupq_n_s64(0), t[2].val[0]);
  t[2].val[1] = vsubq_s64(vdupq_n_s64(0), t[2].val[1]);
  t[3].val[0] = vsubq_s64(vdupq_n_s64(0), t[3].val[0]);
  t[3].val[1] = vsubq_s64(vdupq_n_s64(0), t[3].val[1]);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct_cospi_16_16_q(const int32x4x2_t s0,
                                             const int32x4x2_t s1,
                                             const int32x4_t cospi_0_8_16_24,
                                             int32x4x2_t *const d0,
                                             int32x4x2_t *const d1) {
  int64x2x2_t t[6];

  t[4].val[0] = vmull_lane_s32(vget_low_s32(s1.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[4].val[1] = vmull_lane_s32(vget_high_s32(s1.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[5].val[0] = vmull_lane_s32(vget_low_s32(s1.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[5].val[1] = vmull_lane_s32(vget_high_s32(s1.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[0].val[0] = vmlsl_lane_s32(t[4].val[0], vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[0].val[1] = vmlsl_lane_s32(t[4].val[1], vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[1].val[0] = vmlsl_lane_s32(t[5].val[0], vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[1].val[1] = vmlsl_lane_s32(t[5].val[1], vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[2].val[0] = vmlal_lane_s32(t[4].val[0], vget_low_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[2].val[1] = vmlal_lane_s32(t[4].val[1], vget_high_s32(s0.val[0]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[3].val[0] = vmlal_lane_s32(t[5].val[0], vget_low_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  t[3].val[1] = vmlal_lane_s32(t[5].val[1], vget_high_s32(s0.val[1]),
                               vget_high_s32(cospi_0_8_16_24), 0);
  highbd_idct16x16_add_wrap_low_8x2(t, d0, d1);
}

static INLINE void highbd_idct16x16_add_stage7(const int32x4x2_t *const step2,
                                               int32x4x2_t *const out) {
  out[0].val[0] = vaddq_s32(step2[0].val[0], step2[15].val[0]);
  out[0].val[1] = vaddq_s32(step2[0].val[1], step2[15].val[1]);
  out[1].val[0] = vaddq_s32(step2[1].val[0], step2[14].val[0]);
  out[1].val[1] = vaddq_s32(step2[1].val[1], step2[14].val[1]);
  out[2].val[0] = vaddq_s32(step2[2].val[0], step2[13].val[0]);
  out[2].val[1] = vaddq_s32(step2[2].val[1], step2[13].val[1]);
  out[3].val[0] = vaddq_s32(step2[3].val[0], step2[12].val[0]);
  out[3].val[1] = vaddq_s32(step2[3].val[1], step2[12].val[1]);
  out[4].val[0] = vaddq_s32(step2[4].val[0], step2[11].val[0]);
  out[4].val[1] = vaddq_s32(step2[4].val[1], step2[11].val[1]);
  out[5].val[0] = vaddq_s32(step2[5].val[0], step2[10].val[0]);
  out[5].val[1] = vaddq_s32(step2[5].val[1], step2[10].val[1]);
  out[6].val[0] = vaddq_s32(step2[6].val[0], step2[9].val[0]);
  out[6].val[1] = vaddq_s32(step2[6].val[1], step2[9].val[1]);
  out[7].val[0] = vaddq_s32(step2[7].val[0], step2[8].val[0]);
  out[7].val[1] = vaddq_s32(step2[7].val[1], step2[8].val[1]);
  out[8].val[0] = vsubq_s32(step2[7].val[0], step2[8].val[0]);
  out[8].val[1] = vsubq_s32(step2[7].val[1], step2[8].val[1]);
  out[9].val[0] = vsubq_s32(step2[6].val[0], step2[9].val[0]);
  out[9].val[1] = vsubq_s32(step2[6].val[1], step2[9].val[1]);
  out[10].val[0] = vsubq_s32(step2[5].val[0], step2[10].val[0]);
  out[10].val[1] = vsubq_s32(step2[5].val[1], step2[10].val[1]);
  out[11].val[0] = vsubq_s32(step2[4].val[0], step2[11].val[0]);
  out[11].val[1] = vsubq_s32(step2[4].val[1], step2[11].val[1]);
  out[12].val[0] = vsubq_s32(step2[3].val[0], step2[12].val[0]);
  out[12].val[1] = vsubq_s32(step2[3].val[1], step2[12].val[1]);
  out[13].val[0] = vsubq_s32(step2[2].val[0], step2[13].val[0]);
  out[13].val[1] = vsubq_s32(step2[2].val[1], step2[13].val[1]);
  out[14].val[0] = vsubq_s32(step2[1].val[0], step2[14].val[0]);
  out[14].val[1] = vsubq_s32(step2[1].val[1], step2[14].val[1]);
  out[15].val[0] = vsubq_s32(step2[0].val[0], step2[15].val[0]);
  out[15].val[1] = vsubq_s32(step2[0].val[1], step2[15].val[1]);
}

static void highbd_idct16x16_256_add_half1d(const int32_t *input,
                                            int32_t *output, uint16_t *dest,
                                            const int stride, const int bd) {
  const int32x4_t cospi_0_8_16_24 = vld1q_s32(kCospi32 + 0);
  const int32x4_t cospi_4_12_20N_28 = vld1q_s32(kCospi32 + 4);
  const int32x4_t cospi_2_30_10_22 = vld1q_s32(kCospi32 + 8);
  const int32x4_t cospi_6_26_14_18N = vld1q_s32(kCospi32 + 12);
  int32x4x2_t in[16], step1[16], step2[16], out[16];

  // Load input (16x8)
  in[0].val[0] = vld1q_s32(input);
  in[0].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[8].val[0] = vld1q_s32(input);
  in[8].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[1].val[0] = vld1q_s32(input);
  in[1].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[9].val[0] = vld1q_s32(input);
  in[9].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[2].val[0] = vld1q_s32(input);
  in[2].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[10].val[0] = vld1q_s32(input);
  in[10].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[3].val[0] = vld1q_s32(input);
  in[3].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[11].val[0] = vld1q_s32(input);
  in[11].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[4].val[0] = vld1q_s32(input);
  in[4].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[12].val[0] = vld1q_s32(input);
  in[12].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[5].val[0] = vld1q_s32(input);
  in[5].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[13].val[0] = vld1q_s32(input);
  in[13].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[6].val[0] = vld1q_s32(input);
  in[6].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[14].val[0] = vld1q_s32(input);
  in[14].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[7].val[0] = vld1q_s32(input);
  in[7].val[1] = vld1q_s32(input + 4);
  input += 8;
  in[15].val[0] = vld1q_s32(input);
  in[15].val[1] = vld1q_s32(input + 4);

  // Transpose
  transpose_s32_8x8(&in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6],
                    &in[7]);
  transpose_s32_8x8(&in[8], &in[9], &in[10], &in[11], &in[12], &in[13], &in[14],
                    &in[15]);

  // stage 1
  step1[0] = in[0 / 2];
  step1[1] = in[16 / 2];
  step1[2] = in[8 / 2];
  step1[3] = in[24 / 2];
  step1[4] = in[4 / 2];
  step1[5] = in[20 / 2];
  step1[6] = in[12 / 2];
  step1[7] = in[28 / 2];
  step1[8] = in[2 / 2];
  step1[9] = in[18 / 2];
  step1[10] = in[10 / 2];
  step1[11] = in[26 / 2];
  step1[12] = in[6 / 2];
  step1[13] = in[22 / 2];
  step1[14] = in[14 / 2];
  step1[15] = in[30 / 2];

  // stage 2
  step2[0] = step1[0];
  step2[1] = step1[1];
  step2[2] = step1[2];
  step2[3] = step1[3];
  step2[4] = step1[4];
  step2[5] = step1[5];
  step2[6] = step1[6];
  step2[7] = step1[7];
  highbd_idct_cospi_2_30(step1[8], step1[15], cospi_2_30_10_22, &step2[8],
                         &step2[15]);
  highbd_idct_cospi_14_18(step1[9], step1[14], cospi_6_26_14_18N, &step2[9],
                          &step2[14]);
  highbd_idct_cospi_10_22(step1[10], step1[13], cospi_2_30_10_22, &step2[10],
                          &step2[13]);
  highbd_idct_cospi_6_26(step1[11], step1[12], cospi_6_26_14_18N, &step2[11],
                         &step2[12]);

  // stage 3
  step1[0] = step2[0];
  step1[1] = step2[1];
  step1[2] = step2[2];
  step1[3] = step2[3];
  highbd_idct_cospi_4_28(step2[4], step2[7], cospi_4_12_20N_28, &step1[4],
                         &step1[7]);
  highbd_idct_cospi_12_20(step2[5], step2[6], cospi_4_12_20N_28, &step1[5],
                          &step1[6]);
  step1[8].val[0] = vaddq_s32(step2[8].val[0], step2[9].val[0]);
  step1[8].val[1] = vaddq_s32(step2[8].val[1], step2[9].val[1]);
  step1[9].val[0] = vsubq_s32(step2[8].val[0], step2[9].val[0]);
  step1[9].val[1] = vsubq_s32(step2[8].val[1], step2[9].val[1]);
  step1[10].val[0] = vsubq_s32(step2[11].val[0], step2[10].val[0]);
  step1[10].val[1] = vsubq_s32(step2[11].val[1], step2[10].val[1]);
  step1[11].val[0] = vaddq_s32(step2[11].val[0], step2[10].val[0]);
  step1[11].val[1] = vaddq_s32(step2[11].val[1], step2[10].val[1]);
  step1[12].val[0] = vaddq_s32(step2[12].val[0], step2[13].val[0]);
  step1[12].val[1] = vaddq_s32(step2[12].val[1], step2[13].val[1]);
  step1[13].val[0] = vsubq_s32(step2[12].val[0], step2[13].val[0]);
  step1[13].val[1] = vsubq_s32(step2[12].val[1], step2[13].val[1]);
  step1[14].val[0] = vsubq_s32(step2[15].val[0], step2[14].val[0]);
  step1[14].val[1] = vsubq_s32(step2[15].val[1], step2[14].val[1]);
  step1[15].val[0] = vaddq_s32(step2[15].val[0], step2[14].val[0]);
  step1[15].val[1] = vaddq_s32(step2[15].val[1], step2[14].val[1]);

  // stage 4
  highbd_idct_cospi_16_16_q(step1[1], step1[0], cospi_0_8_16_24, &step2[1],
                            &step2[0]);
  highbd_idct_cospi_8_24_q(step1[2], step1[3], cospi_0_8_16_24, &step2[2],
                           &step2[3]);
  step2[4].val[0] = vaddq_s32(step1[4].val[0], step1[5].val[0]);
  step2[4].val[1] = vaddq_s32(step1[4].val[1], step1[5].val[1]);
  step2[5].val[0] = vsubq_s32(step1[4].val[0], step1[5].val[0]);
  step2[5].val[1] = vsubq_s32(step1[4].val[1], step1[5].val[1]);
  step2[6].val[0] = vsubq_s32(step1[7].val[0], step1[6].val[0]);
  step2[6].val[1] = vsubq_s32(step1[7].val[1], step1[6].val[1]);
  step2[7].val[0] = vaddq_s32(step1[7].val[0], step1[6].val[0]);
  step2[7].val[1] = vaddq_s32(step1[7].val[1], step1[6].val[1]);
  step2[8] = step1[8];
  highbd_idct_cospi_8_24_q(step1[14], step1[9], cospi_0_8_16_24, &step2[9],
                           &step2[14]);
  highbd_idct_cospi_8_24_neg_q(step1[13], step1[10], cospi_0_8_16_24,
                               &step2[13], &step2[10]);
  step2[11] = step1[11];
  step2[12] = step1[12];
  step2[15] = step1[15];

  // stage 5
  step1[0].val[0] = vaddq_s32(step2[0].val[0], step2[3].val[0]);
  step1[0].val[1] = vaddq_s32(step2[0].val[1], step2[3].val[1]);
  step1[1].val[0] = vaddq_s32(step2[1].val[0], step2[2].val[0]);
  step1[1].val[1] = vaddq_s32(step2[1].val[1], step2[2].val[1]);
  step1[2].val[0] = vsubq_s32(step2[1].val[0], step2[2].val[0]);
  step1[2].val[1] = vsubq_s32(step2[1].val[1], step2[2].val[1]);
  step1[3].val[0] = vsubq_s32(step2[0].val[0], step2[3].val[0]);
  step1[3].val[1] = vsubq_s32(step2[0].val[1], step2[3].val[1]);
  step1[4] = step2[4];
  highbd_idct_cospi_16_16_q(step2[5], step2[6], cospi_0_8_16_24, &step1[5],
                            &step1[6]);
  step1[7] = step2[7];
  step1[8].val[0] = vaddq_s32(step2[8].val[0], step2[11].val[0]);
  step1[8].val[1] = vaddq_s32(step2[8].val[1], step2[11].val[1]);
  step1[9].val[0] = vaddq_s32(step2[9].val[0], step2[10].val[0]);
  step1[9].val[1] = vaddq_s32(step2[9].val[1], step2[10].val[1]);
  step1[10].val[0] = vsubq_s32(step2[9].val[0], step2[10].val[0]);
  step1[10].val[1] = vsubq_s32(step2[9].val[1], step2[10].val[1]);
  step1[11].val[0] = vsubq_s32(step2[8].val[0], step2[11].val[0]);
  step1[11].val[1] = vsubq_s32(step2[8].val[1], step2[11].val[1]);
  step1[12].val[0] = vsubq_s32(step2[15].val[0], step2[12].val[0]);
  step1[12].val[1] = vsubq_s32(step2[15].val[1], step2[12].val[1]);
  step1[13].val[0] = vsubq_s32(step2[14].val[0], step2[13].val[0]);
  step1[13].val[1] = vsubq_s32(step2[14].val[1], step2[13].val[1]);
  step1[14].val[0] = vaddq_s32(step2[14].val[0], step2[13].val[0]);
  step1[14].val[1] = vaddq_s32(step2[14].val[1], step2[13].val[1]);
  step1[15].val[0] = vaddq_s32(step2[15].val[0], step2[12].val[0]);
  step1[15].val[1] = vaddq_s32(step2[15].val[1], step2[12].val[1]);

  // stage 6
  step2[0].val[0] = vaddq_s32(step1[0].val[0], step1[7].val[0]);
  step2[0].val[1] = vaddq_s32(step1[0].val[1], step1[7].val[1]);
  step2[1].val[0] = vaddq_s32(step1[1].val[0], step1[6].val[0]);
  step2[1].val[1] = vaddq_s32(step1[1].val[1], step1[6].val[1]);
  step2[2].val[0] = vaddq_s32(step1[2].val[0], step1[5].val[0]);
  step2[2].val[1] = vaddq_s32(step1[2].val[1], step1[5].val[1]);
  step2[3].val[0] = vaddq_s32(step1[3].val[0], step1[4].val[0]);
  step2[3].val[1] = vaddq_s32(step1[3].val[1], step1[4].val[1]);
  step2[4].val[0] = vsubq_s32(step1[3].val[0], step1[4].val[0]);
  step2[4].val[1] = vsubq_s32(step1[3].val[1], step1[4].val[1]);
  step2[5].val[0] = vsubq_s32(step1[2].val[0], step1[5].val[0]);
  step2[5].val[1] = vsubq_s32(step1[2].val[1], step1[5].val[1]);
  step2[6].val[0] = vsubq_s32(step1[1].val[0], step1[6].val[0]);
  step2[6].val[1] = vsubq_s32(step1[1].val[1], step1[6].val[1]);
  step2[7].val[0] = vsubq_s32(step1[0].val[0], step1[7].val[0]);
  step2[7].val[1] = vsubq_s32(step1[0].val[1], step1[7].val[1]);
  highbd_idct_cospi_16_16_q(step1[10], step1[13], cospi_0_8_16_24, &step2[10],
                            &step2[13]);
  highbd_idct_cospi_16_16_q(step1[11], step1[12], cospi_0_8_16_24, &step2[11],
                            &step2[12]);
  step2[8] = step1[8];
  step2[9] = step1[9];
  step2[14] = step1[14];
  step2[15] = step1[15];

  // stage 7
  highbd_idct16x16_add_stage7(step2, out);

  if (output) {
    // pass 1: save the result into output
    vst1q_s32(output + 0, out[0].val[0]);
    vst1q_s32(output + 4, out[0].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[1].val[0]);
    vst1q_s32(output + 4, out[1].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[2].val[0]);
    vst1q_s32(output + 4, out[2].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[3].val[0]);
    vst1q_s32(output + 4, out[3].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[4].val[0]);
    vst1q_s32(output + 4, out[4].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[5].val[0]);
    vst1q_s32(output + 4, out[5].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[6].val[0]);
    vst1q_s32(output + 4, out[6].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[7].val[0]);
    vst1q_s32(output + 4, out[7].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[8].val[0]);
    vst1q_s32(output + 4, out[8].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[9].val[0]);
    vst1q_s32(output + 4, out[9].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[10].val[0]);
    vst1q_s32(output + 4, out[10].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[11].val[0]);
    vst1q_s32(output + 4, out[11].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[12].val[0]);
    vst1q_s32(output + 4, out[12].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[13].val[0]);
    vst1q_s32(output + 4, out[13].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[14].val[0]);
    vst1q_s32(output + 4, out[14].val[1]);
    output += 16;
    vst1q_s32(output + 0, out[15].val[0]);
    vst1q_s32(output + 4, out[15].val[1]);
  } else {
    // pass 2: add the result to dest.
    const int16x8_t max = vdupq_n_s16((1 << bd) - 1);
    int16x8_t o[16];
    o[0] = vcombine_s16(vrshrn_n_s32(out[0].val[0], 6),
                        vrshrn_n_s32(out[0].val[1], 6));
    o[1] = vcombine_s16(vrshrn_n_s32(out[1].val[0], 6),
                        vrshrn_n_s32(out[1].val[1], 6));
    o[2] = vcombine_s16(vrshrn_n_s32(out[2].val[0], 6),
                        vrshrn_n_s32(out[2].val[1], 6));
    o[3] = vcombine_s16(vrshrn_n_s32(out[3].val[0], 6),
                        vrshrn_n_s32(out[3].val[1], 6));
    o[4] = vcombine_s16(vrshrn_n_s32(out[4].val[0], 6),
                        vrshrn_n_s32(out[4].val[1], 6));
    o[5] = vcombine_s16(vrshrn_n_s32(out[5].val[0], 6),
                        vrshrn_n_s32(out[5].val[1], 6));
    o[6] = vcombine_s16(vrshrn_n_s32(out[6].val[0], 6),
                        vrshrn_n_s32(out[6].val[1], 6));
    o[7] = vcombine_s16(vrshrn_n_s32(out[7].val[0], 6),
                        vrshrn_n_s32(out[7].val[1], 6));
    o[8] = vcombine_s16(vrshrn_n_s32(out[8].val[0], 6),
                        vrshrn_n_s32(out[8].val[1], 6));
    o[9] = vcombine_s16(vrshrn_n_s32(out[9].val[0], 6),
                        vrshrn_n_s32(out[9].val[1], 6));
    o[10] = vcombine_s16(vrshrn_n_s32(out[10].val[0], 6),
                         vrshrn_n_s32(out[10].val[1], 6));
    o[11] = vcombine_s16(vrshrn_n_s32(out[11].val[0], 6),
                         vrshrn_n_s32(out[11].val[1], 6));
    o[12] = vcombine_s16(vrshrn_n_s32(out[12].val[0], 6),
                         vrshrn_n_s32(out[12].val[1], 6));
    o[13] = vcombine_s16(vrshrn_n_s32(out[13].val[0], 6),
                         vrshrn_n_s32(out[13].val[1], 6));
    o[14] = vcombine_s16(vrshrn_n_s32(out[14].val[0], 6),
                         vrshrn_n_s32(out[14].val[1], 6));
    o[15] = vcombine_s16(vrshrn_n_s32(out[15].val[0], 6),
                         vrshrn_n_s32(out[15].val[1], 6));
    highbd_idct16x16_add8x1(o[0], max, &dest, stride);
    highbd_idct16x16_add8x1(o[1], max, &dest, stride);
    highbd_idct16x16_add8x1(o[2], max, &dest, stride);
    highbd_idct16x16_add8x1(o[3], max, &dest, stride);
    highbd_idct16x16_add8x1(o[4], max, &dest, stride);
    highbd_idct16x16_add8x1(o[5], max, &dest, stride);
    highbd_idct16x16_add8x1(o[6], max, &dest, stride);
    highbd_idct16x16_add8x1(o[7], max, &dest, stride);
    highbd_idct16x16_add8x1(o[8], max, &dest, stride);
    highbd_idct16x16_add8x1(o[9], max, &dest, stride);
    highbd_idct16x16_add8x1(o[10], max, &dest, stride);
    highbd_idct16x16_add8x1(o[11], max, &dest, stride);
    highbd_idct16x16_add8x1(o[12], max, &dest, stride);
    highbd_idct16x16_add8x1(o[13], max, &dest, stride);
    highbd_idct16x16_add8x1(o[14], max, &dest, stride);
    highbd_idct16x16_add8x1(o[15], max, &dest, stride);
  }
}

void vpx_highbd_idct16x16_256_add_neon(const tran_low_t *input, uint8_t *dest8,
                                       int stride, int bd) {
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);

  if (bd == 8) {
    int16_t row_idct_output[16 * 16];

    // pass 1
    // Parallel idct on the upper 8 rows
    idct16x16_256_add_half1d_8bit((const int16_t *)input, row_idct_output, dest,
                                  stride);

    // Parallel idct on the lower 8 rows
    idct16x16_256_add_half1d_8bit((const int16_t *)(input + 8 * 16),
                                  row_idct_output + 8, dest, stride);

    // pass 2
    // Parallel idct to get the left 8 columns
    idct16x16_256_add_half1d_8bit(row_idct_output, NULL, dest, stride);

    // Parallel idct to get the right 8 columns
    idct16x16_256_add_half1d_8bit(row_idct_output + 8 * 16, NULL, dest + 8,
                                  stride);
  } else {
    //    vpx_highbd_idct16x16_256_add_c(input, dest8, stride, bd);
    int32_t row_idct_output[16 * 16];

    // pass 1
    // Parallel idct on the upper 8 rows
    highbd_idct16x16_256_add_half1d(input, row_idct_output, dest, stride, bd);

    // Parallel idct on the lower 8 rows
    highbd_idct16x16_256_add_half1d(input + 8 * 16, row_idct_output + 8, dest,
                                    stride, bd);

    // pass 2
    // Parallel idct to get the left 8 columns
    highbd_idct16x16_256_add_half1d(row_idct_output, NULL, dest, stride, bd);

    // Parallel idct to get the right 8 columns
    highbd_idct16x16_256_add_half1d(row_idct_output + 8 * 16, NULL, dest + 8,
                                    stride, bd);
  }
}

static INLINE void highbd_idct16x16_1_add_pos_kernel(uint16_t **dest,
                                                     const int stride,
                                                     const int16x8_t res,
                                                     const int16x8_t max) {
  const uint16x8_t a0 = vld1q_u16(*dest + 0);
  const uint16x8_t a1 = vld1q_u16(*dest + 8);
  const int16x8_t b0 = vaddq_s16(res, vreinterpretq_s16_u16(a0));
  const int16x8_t b1 = vaddq_s16(res, vreinterpretq_s16_u16(a1));
  const int16x8_t c0 = vminq_s16(b0, max);
  const int16x8_t c1 = vminq_s16(b1, max);
  vst1q_u16(*dest + 0, vreinterpretq_u16_s16(c0));
  vst1q_u16(*dest + 8, vreinterpretq_u16_s16(c1));
  *dest += stride;
}

static INLINE void highbd_idct16x16_1_add_neg_kernel(uint16_t **dest,
                                                     const int stride,
                                                     const int16x8_t res) {
  const uint16x8_t a0 = vld1q_u16(*dest + 0);
  const uint16x8_t a1 = vld1q_u16(*dest + 8);
  const int16x8_t b0 = vaddq_s16(res, vreinterpretq_s16_u16(a0));
  const int16x8_t b1 = vaddq_s16(res, vreinterpretq_s16_u16(a1));
  const uint16x8_t c0 = vqshluq_n_s16(b0, 0);
  const uint16x8_t c1 = vqshluq_n_s16(b1, 0);
  vst1q_u16(*dest + 0, c0);
  vst1q_u16(*dest + 8, c1);
  *dest += stride;
}

void vpx_highbd_idct16x16_1_add_neon(const tran_low_t *input, uint8_t *dest8,
                                     int stride, int bd) {
  const tran_low_t out0 =
      HIGHBD_WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64), bd);
  const tran_low_t out1 =
      HIGHBD_WRAPLOW(dct_const_round_shift(out0 * cospi_16_64), bd);
  const int16_t a1 = ROUND_POWER_OF_TWO(out1, 6);
  const int16x8_t dc = vdupq_n_s16(a1);
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  int i;

  if (a1 >= 0) {
    const int16x8_t max = vdupq_n_s16((1 << bd) - 1);
    for (i = 0; i < 4; ++i) {
      highbd_idct16x16_1_add_pos_kernel(&dest, stride, dc, max);
      highbd_idct16x16_1_add_pos_kernel(&dest, stride, dc, max);
      highbd_idct16x16_1_add_pos_kernel(&dest, stride, dc, max);
      highbd_idct16x16_1_add_pos_kernel(&dest, stride, dc, max);
    }
  } else {
    for (i = 0; i < 4; ++i) {
      highbd_idct16x16_1_add_neg_kernel(&dest, stride, dc);
      highbd_idct16x16_1_add_neg_kernel(&dest, stride, dc);
      highbd_idct16x16_1_add_neg_kernel(&dest, stride, dc);
      highbd_idct16x16_1_add_neg_kernel(&dest, stride, dc);
    }
  }
}
