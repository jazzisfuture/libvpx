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
#include "vpx_dsp/txfm_common.h"

#if CONFIG_VP9_HIGHBITDEPTH
static INLINE void idct16x16_38_add_load_tran_low_kernel(
    const tran_low_t **input, int16_t **out) {
  int16x8_t s;

  s = load_tran_low_to_s16q(*input);
  vst1q_s16(*out, s);
  *input += 16;
  *out += 16;
}

static INLINE void idct16x16_38_add_load_tran_low(const tran_low_t *input,
                                                  int16_t *out) {
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
  idct16x16_38_add_load_tran_low_kernel(&input, &out);
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

static void idct16x16_38_add_half1d(const int16_t *input, int16_t *output,
                                    uint8_t *dest, int stride) {
  const int16x8_t cospis0 = vld1q_s16(kCospi);
  const int16x8_t cospis1 = vld1q_s16(kCospi + 8);
  const int16x8_t cospisd0 = vaddq_s16(cospis0, cospis0);
  const int16x8_t cospisd1 = vaddq_s16(cospis1, cospis1);
  const int16x4_t cospi_0_8_16_24 = vget_low_s16(cospis0);
  const int16x4_t cospid_0_8_16_24 = vget_low_s16(cospisd0);
  const int16x4_t cospid_4_12_20N_28 = vget_high_s16(cospisd0);
  const int16x4_t cospid_2_30_10_22 = vget_low_s16(cospisd1);
  const int16x4_t cospid_6_26_14_18N = vget_high_s16(cospisd1);
  int16x8_t in[8], step1[16], step2[16], out[16];

  // Load input (8x8)
  if (output) {
    const tran_low_t *inputT = (const tran_low_t *)input;
    in[0] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[1] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[2] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[3] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[4] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[5] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[6] = load_tran_low_to_s16q(inputT);
    inputT += 16;
    in[7] = load_tran_low_to_s16q(inputT);
  } else {
    in[0] = vld1q_s16(input);
    input += 16;
    in[1] = vld1q_s16(input);
    input += 16;
    in[2] = vld1q_s16(input);
    input += 16;
    in[3] = vld1q_s16(input);
    input += 16;
    in[4] = vld1q_s16(input);
    input += 16;
    in[5] = vld1q_s16(input);
    input += 16;
    in[6] = vld1q_s16(input);
    input += 16;
    in[7] = vld1q_s16(input);
  }

  // Transpose
  transpose_s16_8x8(&in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6],
                    &in[7]);

  // stage 1
  step1[0] = in[0 / 2];
  step1[2] = in[8 / 2];
  step1[4] = in[4 / 2];
  step1[6] = in[12 / 2];
  step1[8] = in[2 / 2];
  step1[10] = in[10 / 2];
  step1[12] = in[6 / 2];
  step1[14] = in[14 / 2];  // 0 in pass 1

  // stage 2
  step2[0] = step1[0];
  step2[2] = step1[2];
  step2[4] = step1[4];
  step2[6] = step1[6];
  step2[8] = vqrdmulhq_lane_s16(step1[8], cospid_2_30_10_22, 1);
  step2[9] = vqrdmulhq_lane_s16(step1[14], cospid_6_26_14_18N, 3);
  step2[10] = vqrdmulhq_lane_s16(step1[10], cospid_2_30_10_22, 3);
  step2[11] = vqrdmulhq_lane_s16(step1[12], cospid_6_26_14_18N, 1);
  step2[12] = vqrdmulhq_lane_s16(step1[12], cospid_6_26_14_18N, 0);
  step2[13] = vqrdmulhq_lane_s16(step1[10], cospid_2_30_10_22, 2);
  step2[14] = vqrdmulhq_lane_s16(step1[14], cospid_6_26_14_18N, 2);
  step2[15] = vqrdmulhq_lane_s16(step1[8], cospid_2_30_10_22, 0);

  // stage 3
  step1[0] = step2[0];
  step1[2] = step2[2];
  step1[4] = vqrdmulhq_lane_s16(step2[4], cospid_4_12_20N_28, 3);
  step1[5] = vqrdmulhq_lane_s16(step2[6], cospid_4_12_20N_28, 2);
  step1[6] = vqrdmulhq_lane_s16(step2[6], cospid_4_12_20N_28, 1);
  step1[7] = vqrdmulhq_lane_s16(step2[4], cospid_4_12_20N_28, 0);
  step1[8] = vaddq_s16(step2[8], step2[9]);
  step1[9] = vsubq_s16(step2[8], step2[9]);
  step1[10] = vsubq_s16(step2[11], step2[10]);
  step1[11] = vaddq_s16(step2[11], step2[10]);
  step1[12] = vaddq_s16(step2[12], step2[13]);
  step1[13] = vsubq_s16(step2[12], step2[13]);
  step1[14] = vsubq_s16(step2[15], step2[14]);
  step1[15] = vaddq_s16(step2[15], step2[14]);

  // stage 4
  step2[0] = step2[1] = vqrdmulhq_lane_s16(step1[0], cospid_0_8_16_24, 2);
  step2[2] = vqrdmulhq_lane_s16(step1[2], cospid_0_8_16_24, 3);
  step2[3] = vqrdmulhq_lane_s16(step1[2], cospid_0_8_16_24, 1);
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
    idct16x16_add8x1(out[0], &dest, stride);
    idct16x16_add8x1(out[1], &dest, stride);
    idct16x16_add8x1(out[2], &dest, stride);
    idct16x16_add8x1(out[3], &dest, stride);
    idct16x16_add8x1(out[4], &dest, stride);
    idct16x16_add8x1(out[5], &dest, stride);
    idct16x16_add8x1(out[6], &dest, stride);
    idct16x16_add8x1(out[7], &dest, stride);
    idct16x16_add8x1(out[8], &dest, stride);
    idct16x16_add8x1(out[9], &dest, stride);
    idct16x16_add8x1(out[10], &dest, stride);
    idct16x16_add8x1(out[11], &dest, stride);
    idct16x16_add8x1(out[12], &dest, stride);
    idct16x16_add8x1(out[13], &dest, stride);
    idct16x16_add8x1(out[14], &dest, stride);
    idct16x16_add8x1(out[15], &dest, stride);
  }
}

void vpx_idct16x16_38_add_neon(const tran_low_t *input, uint8_t *dest,
                               int stride) {
  int16_t row_idct_output[16 * 16];

  // pass 1
  // Parallel idct on the upper 8 rows
  idct16x16_38_add_half1d((const int16_t *)input, row_idct_output, dest,
                          stride);

  // pass 2
  // Parallel idct to get the left 8 columns
  idct16x16_38_add_half1d(row_idct_output, NULL, dest, stride);

  // Parallel idct to get the right 8 columns
  idct16x16_38_add_half1d(row_idct_output + 16 * 8, NULL, dest + 8, stride);
}
