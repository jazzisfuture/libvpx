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
#include <math.h>
#include <string.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/transpose_neon.h"
#include "vpx_dsp/inv_txfm.h"
#include "vpx_dsp/txfm_common.h"

/*******************************************************************************
 * Interesting bits
 ******************************************************************************/
// Multiply a by a_const and expand to int32_t. Saturate, shift and narrow by
// 14.
static void multiply_shift_and_narrow(int16x8_t a, int16_t a_const,
                                      int16x8_t *b) {
  const int32x4_t temp_low = vmull_n_s16(vget_low_s16(a), a_const);
  const int32x4_t temp_high = vmull_n_s16(vget_high_s16(a), a_const);
  // Shift by 14 + rounding will be within 16 bits for well formed streams.
  // See WRAPLOW and dct_const_round_shift for details.
  *b = vcombine_s16(vqrshrn_n_s32(temp_low, 14), vqrshrn_n_s32(temp_high, 14));
}

// Add a and b, then multiply by ab_const. Saturate, shift and narrow by 14.
static void add_multiply_shift_and_narrow(int16x8_t a, int16x8_t b,
                                          int16_t ab_const, int16x8_t *c) {
  const int16x8_t temp = vqaddq_s16(a, b);
  const int32x4_t temp_low = vmull_n_s16(vget_low_s16(temp), ab_const);
  const int32x4_t temp_high = vmull_n_s16(vget_high_s16(temp), ab_const);
  *c = vcombine_s16(vqrshrn_n_s32(temp_low, 14), vqrshrn_n_s32(temp_high, 14));
}

// Subtract b from a, then multiply by ab_const. Saturate, shift and narrow by
// 14.
static void sub_multiply_shift_and_narrow(int16x8_t a, int16x8_t b,
                                          int16_t ab_const, int16x8_t *c) {
  const int16x8_t temp = vqsubq_s16(a, b);
  const int32x4_t temp_low = vmull_n_s16(vget_low_s16(temp), ab_const);
  const int32x4_t temp_high = vmull_n_s16(vget_high_s16(temp), ab_const);
  *c = vcombine_s16(vqrshrn_n_s32(temp_low, 14), vqrshrn_n_s32(temp_high, 14));
}

// Multiply a by a_const and b by b_const, then accumulate. Saturate, shift and
// narrow by 14.
static void multiply_accumulate_shift_and_narrow(int16x8_t a, int16_t a_const,
                                                 int16x8_t b, int16_t b_const,
                                                 int16x8_t *c) {
  int32x4_t temp_low = vmull_n_s16(vget_low_s16(a), a_const);
  int32x4_t temp_high = vmull_n_s16(vget_high_s16(a), a_const);
  temp_low = vmlal_n_s16(temp_low, vget_low_s16(b), b_const);
  temp_high = vmlal_n_s16(temp_high, vget_high_s16(b), b_const);
  *c = vcombine_s16(vqrshrn_n_s32(temp_low, 14), vqrshrn_n_s32(temp_high, 14));
}

// Only for the _34_ variant. Since it only uses values from the top left 8x8 it
// can safely assume all the remaining values are 0 and skip an awful lot of
// calculations. In fact, only the first 7 columns make the cut. None of the
// elements in the 8th column are used so it skips any calls to input[7] too.
// In C this does a single row of 32 for each call. Here it transposes the top
// left 8x8 to allow using SIMD.

// The first 34 non zero coefficients are arranged as follows:
//   0  1  2  3  4  5  6  7
// 0 0  2  5 10 17 25
// 1    1  4  8 15 22 30
// 2    3  7 12 18 28
// 3    6 11 16 23 31
// 4    9 14 19 29
// 5   13 20 26
// 6   21 27 33
// 7   24 32
static void idct32_neon(const tran_low_t *input, tran_low_t *output) {
  int16x8_t in0, in1, in2, in3, in4, in5, in6, in7;
  int16x8_t s1_0, s1_1, s1_2, s1_3, s1_4, s1_5, s1_6, s1_7, s1_8, s1_9, s1_10,
      s1_11, s1_12, s1_13, s1_14, s1_15, s1_16, s1_17, s1_18, s1_19, s1_20,
      s1_21, s1_22, s1_23, s1_24, s1_25, s1_26, s1_27, s1_28, s1_29, s1_30,
      s1_31;
  int16x8_t s2_0, s2_1, s2_2, s2_3, s2_4, s2_5, s2_6, s2_7, s2_8, s2_9, s2_10,
      s2_11, s2_12, s2_13, s2_14, s2_15, s2_16, s2_17, s2_18, s2_19, s2_20,
      s2_21, s2_22, s2_23, s2_24, s2_25, s2_26, s2_27, s2_28, s2_29, s2_30,
      s2_31;
  int16x8_t s3_24, s3_25, s3_26, s3_27;

  in0 = vld1q_s16(input);
  input += 32;
  in1 = vld1q_s16(input);
  input += 32;
  in2 = vld1q_s16(input);
  input += 32;
  in3 = vld1q_s16(input);
  input += 32;
  in4 = vld1q_s16(input);
  input += 32;
  in5 = vld1q_s16(input);
  input += 32;
  in6 = vld1q_s16(input);
  input += 32;
  in7 = vld1q_s16(input);

  transpose_s16_8x8(&in0, &in1, &in2, &in3, &in4, &in5, &in6, &in7);

  // stage 1
  // input[1] * cospi_31_64 - input[31] * cospi_1_64 (but input[31] == 0)
  multiply_shift_and_narrow(in1, cospi_31_64, &s1_16);
  // input[1] * cospi_1_64 + input[31] * cospi_31_64 (but input[31] == 0)
  multiply_shift_and_narrow(in1, cospi_1_64, &s1_31);

  multiply_shift_and_narrow(in5, cospi_27_64, &s1_20);
  multiply_shift_and_narrow(in5, cospi_5_64, &s1_27);

  multiply_shift_and_narrow(in3, -cospi_29_64, &s1_23);
  multiply_shift_and_narrow(in3, cospi_3_64, &s1_24);

  // stage 2
  multiply_shift_and_narrow(in2, cospi_30_64, &s2_8);
  multiply_shift_and_narrow(in2, cospi_2_64, &s2_15);

  multiply_shift_and_narrow(in6, -cospi_26_64, &s2_11);
  multiply_shift_and_narrow(in6, cospi_6_64, &s2_12);

  // stage 3
  multiply_shift_and_narrow(in4, cospi_28_64, &s1_4);
  multiply_shift_and_narrow(in4, cospi_4_64, &s1_7);

  multiply_accumulate_shift_and_narrow(s1_16, -cospi_4_64, s1_31, cospi_28_64,
                                       &s1_17);
  multiply_accumulate_shift_and_narrow(s1_16, cospi_28_64, s1_31, cospi_4_64,
                                       &s1_30);

  multiply_accumulate_shift_and_narrow(s1_20, -cospi_20_64, s1_27, cospi_12_64,
                                       &s1_21);
  multiply_accumulate_shift_and_narrow(s1_20, cospi_12_64, s1_27, cospi_20_64,
                                       &s1_26);

  multiply_accumulate_shift_and_narrow(s1_23, -cospi_12_64, s1_24, -cospi_20_64,
                                       &s1_22);
  multiply_accumulate_shift_and_narrow(s1_23, -cospi_20_64, s1_24, cospi_12_64,
                                       &s1_25);

  // stage 4
  // this was originally stored to s2_0 but way down below it finally gets used
  // and wants to use s2_ as the destination. so switch this one to s1_0
  multiply_shift_and_narrow(in0, cospi_16_64, &s1_0);

  multiply_accumulate_shift_and_narrow(s2_8, -cospi_8_64, s2_15, cospi_24_64,
                                       &s2_9);
  multiply_accumulate_shift_and_narrow(s2_8, cospi_24_64, s2_15, cospi_8_64,
                                       &s2_14);

  multiply_accumulate_shift_and_narrow(s2_11, -cospi_24_64, s2_12, -cospi_8_64,
                                       &s2_10);
  multiply_accumulate_shift_and_narrow(s2_11, -cospi_8_64, s2_12, cospi_24_64,
                                       &s2_13);

  s2_20 = vqsubq_s16(s1_23, s1_20);
  s2_21 = vqsubq_s16(s1_22, s1_21);
  s2_22 = vqaddq_s16(s1_21, s1_22);
  s2_23 = vqaddq_s16(s1_20, s1_23);
  s2_24 = vqaddq_s16(s1_24, s1_27);
  s2_25 = vqaddq_s16(s1_25, s1_26);
  s2_26 = vqsubq_s16(s1_25, s1_26);
  s2_27 = vqsubq_s16(s1_24, s1_27);

  // stage 5
  sub_multiply_shift_and_narrow(s1_7, s1_4, cospi_16_64, &s1_5);
  add_multiply_shift_and_narrow(s1_4, s1_7, cospi_16_64, &s1_6);

  s1_8 = vqaddq_s16(s2_8, s2_11);
  s1_9 = vqaddq_s16(s2_9, s2_10);
  s1_10 = vqsubq_s16(s2_9, s2_10);
  s1_11 = vqsubq_s16(s2_8, s2_11);
  s1_12 = vqsubq_s16(s2_15, s2_12);
  s1_13 = vqsubq_s16(s2_14, s2_13);
  s1_14 = vqaddq_s16(s2_13, s2_14);
  s1_15 = vqaddq_s16(s2_12, s2_15);

  multiply_accumulate_shift_and_narrow(s1_17, -cospi_8_64, s1_30, cospi_24_64,
                                       &s1_18);
  multiply_accumulate_shift_and_narrow(s1_17, cospi_24_64, s1_30, cospi_8_64,
                                       &s1_29);

  multiply_accumulate_shift_and_narrow(s1_16, -cospi_8_64, s1_31, cospi_24_64,
                                       &s1_19);
  multiply_accumulate_shift_and_narrow(s1_16, cospi_24_64, s1_31, cospi_8_64,
                                       &s1_28);

  multiply_accumulate_shift_and_narrow(s2_20, -cospi_24_64, s2_27, -cospi_8_64,
                                       &s1_20);
  multiply_accumulate_shift_and_narrow(s2_20, -cospi_8_64, s2_27, cospi_24_64,
                                       &s1_27);

  multiply_accumulate_shift_and_narrow(s2_21, -cospi_24_64, s2_26, -cospi_8_64,
                                       &s1_21);
  multiply_accumulate_shift_and_narrow(s2_21, -cospi_8_64, s2_26, cospi_24_64,
                                       &s1_26);

  // stage 6
  s2_0 = vqaddq_s16(s1_0, s1_7);
  s2_1 = vqaddq_s16(s1_0, s1_6);
  s2_2 = vqaddq_s16(s1_0, s1_5);
  s2_3 = vqaddq_s16(s1_0, s1_4);
  s2_4 = vqsubq_s16(s1_0, s1_4);
  s2_5 = vqsubq_s16(s1_0, s1_5);
  s2_6 = vqsubq_s16(s1_0, s1_6);
  s2_7 = vqsubq_s16(s1_0, s1_7);

  sub_multiply_shift_and_narrow(s1_13, s1_10, cospi_16_64, &s2_10);
  add_multiply_shift_and_narrow(s1_10, s1_13, cospi_16_64, &s2_13);

  sub_multiply_shift_and_narrow(s1_12, s1_11, cospi_16_64, &s2_11);
  add_multiply_shift_and_narrow(s1_11, s1_12, cospi_16_64, &s2_12);

  // This skips the step2[23] -> step1[23] redirect. it should work out OK
  // because step1[23] doesn't get overwritten until the last part.
  s2_16 = vqaddq_s16(s1_16, s2_23);
  s2_17 = vqaddq_s16(s1_17, s2_22);
  s2_18 = vqaddq_s16(s1_18, s1_21);
  s2_19 = vqaddq_s16(s1_19, s1_20);
  s2_20 = vqsubq_s16(s1_19, s1_20);
  s2_21 = vqsubq_s16(s1_18, s1_21);
  s2_22 = vqsubq_s16(s1_17, s2_22);
  s2_23 = vqsubq_s16(s1_16, s2_23);

  // Different block
  // The C copies step2[24] - we could *directly* copy it, which is a waste, or
  // maybe store it to step1? It gets confusing though. Not sure s3_ will help.
  s3_24 = vqsubq_s16(s1_31, s2_24);
  s3_25 = vqsubq_s16(s1_30, s2_25);
  s3_26 = vqsubq_s16(s1_29, s1_26);
  s3_27 = vqsubq_s16(s1_28, s1_27);
  s2_28 = vqaddq_s16(s1_27, s1_28);
  s2_29 = vqaddq_s16(s1_26, s1_29);
  s2_30 = vqaddq_s16(s2_25, s1_30);
  s2_31 = vqaddq_s16(s2_24, s1_31);

  // stage 7
  s1_0 = vqaddq_s16(s2_0, s2_15);
  s1_1 = vqaddq_s16(s2_1, s2_14);
  s1_2 = vqaddq_s16(s2_2, s2_13);
  s1_3 = vqaddq_s16(s2_3, s2_12);
  s1_4 = vqaddq_s16(s2_4, s2_11);
  s1_5 = vqaddq_s16(s2_5, s2_10);
  s1_6 = vqaddq_s16(s2_6, s2_9);
  s1_7 = vqaddq_s16(s2_7, s2_8);
  s1_8 = vqsubq_s16(s2_7, s2_8);
  s1_9 = vqsubq_s16(s2_6, s2_9);
  s1_10 = vqsubq_s16(s2_5, s2_10);
  s1_11 = vqsubq_s16(s2_4, s2_11);
  s1_12 = vqsubq_s16(s2_3, s2_12);
  s1_13 = vqsubq_s16(s2_2, s2_13);
  s1_14 = vqsubq_s16(s2_1, s2_14);
  s1_15 = vqsubq_s16(s2_0, s2_15);

  sub_multiply_shift_and_narrow(s3_27, s2_20, cospi_16_64, &s1_20);
  add_multiply_shift_and_narrow(s2_20, s3_27, cospi_16_64, &s1_27);

  sub_multiply_shift_and_narrow(s3_26, s2_21, cospi_16_64, &s1_21);
  add_multiply_shift_and_narrow(s2_21, s3_26, cospi_16_64, &s1_26);

  sub_multiply_shift_and_narrow(s3_25, s2_22, cospi_16_64, &s1_22);
  add_multiply_shift_and_narrow(s2_22, s3_25, cospi_16_64, &s1_25);

  sub_multiply_shift_and_narrow(s3_24, s2_23, cospi_16_64, &s1_23);
  add_multiply_shift_and_narrow(s2_23, s3_24, cospi_16_64, &s1_24);

  // final stage
  vst1q_s16(output, vqaddq_s16(s1_0, s2_31));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_1, s2_30));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_2, s2_29));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_3, s2_28));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_4, s1_27));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_5, s1_26));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_6, s1_25));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_7, s1_24));
  output += 32;

  vst1q_s16(output, vqaddq_s16(s1_8, s1_23));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_9, s1_22));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_10, s1_21));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_11, s1_20));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_12, s2_19));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_13, s2_18));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_14, s2_17));
  output += 32;
  vst1q_s16(output, vqaddq_s16(s1_15, s2_16));
  output += 32;

  vst1q_s16(output, vqsubq_s16(s1_15, s2_16));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_14, s2_17));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_13, s2_18));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_12, s2_19));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_11, s1_20));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_10, s1_21));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_9, s1_22));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_8, s1_23));
  output += 32;

  vst1q_s16(output, vqsubq_s16(s1_7, s1_24));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_6, s1_25));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_5, s1_26));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_4, s1_27));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_3, s2_28));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_2, s2_29));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_1, s2_30));
  output += 32;
  vst1q_s16(output, vqsubq_s16(s1_0, s2_31));
}

/*******************************************************************************
 * Not interesting bits
 ******************************************************************************/
#define LOAD_FROM_TRANSPOSED(first, second)  \
  q14s16 = vld1q_s16(trans_buf + first * 8); \
  q13s16 = vld1q_s16(trans_buf + second * 8);

#define LOAD_FROM_OUTPUT(first, second, qA, qB) \
  qA = vld1q_s16(out + first * 32);             \
  qB = vld1q_s16(out + second * 32);

#define STORE_IN_OUTPUT(first, second, qA, qB) \
  vst1q_s16(out + first * 32, qA);             \
  vst1q_s16(out + second * 32, qB);

#define STORE_COMBINE_CENTER_RESULTS(r10, r9) \
  __STORE_COMBINE_CENTER_RESULTS(r10, r9, stride, q6s16, q7s16, q8s16, q9s16);
static INLINE void __STORE_COMBINE_CENTER_RESULTS(uint8_t *p1, uint8_t *p2,
                                                  int stride, int16x8_t q6s16,
                                                  int16x8_t q7s16,
                                                  int16x8_t q8s16,
                                                  int16x8_t q9s16) {
  int16x4_t d8s16, d9s16, d10s16, d11s16;

  d8s16 = vld1_s16((int16_t *)p1);
  p1 += stride;
  d11s16 = vld1_s16((int16_t *)p2);
  p2 -= stride;
  d9s16 = vld1_s16((int16_t *)p1);
  d10s16 = vld1_s16((int16_t *)p2);

  q7s16 = vrshrq_n_s16(q7s16, 6);
  q8s16 = vrshrq_n_s16(q8s16, 6);
  q9s16 = vrshrq_n_s16(q9s16, 6);
  q6s16 = vrshrq_n_s16(q6s16, 6);

  q7s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q7s16), vreinterpret_u8_s16(d9s16)));
  q8s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q8s16), vreinterpret_u8_s16(d10s16)));
  q9s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q9s16), vreinterpret_u8_s16(d11s16)));
  q6s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q6s16), vreinterpret_u8_s16(d8s16)));

  d9s16 = vreinterpret_s16_u8(vqmovun_s16(q7s16));
  d10s16 = vreinterpret_s16_u8(vqmovun_s16(q8s16));
  d11s16 = vreinterpret_s16_u8(vqmovun_s16(q9s16));
  d8s16 = vreinterpret_s16_u8(vqmovun_s16(q6s16));

  vst1_s16((int16_t *)p1, d9s16);
  p1 -= stride;
  vst1_s16((int16_t *)p2, d10s16);
  p2 += stride;
  vst1_s16((int16_t *)p1, d8s16);
  vst1_s16((int16_t *)p2, d11s16);
  return;
}

#define STORE_COMBINE_EXTREME_RESULTS(r7, r6) \
  __STORE_COMBINE_EXTREME_RESULTS(r7, r6, stride, q4s16, q5s16, q6s16, q7s16);
static INLINE void __STORE_COMBINE_EXTREME_RESULTS(uint8_t *p1, uint8_t *p2,
                                                   int stride, int16x8_t q4s16,
                                                   int16x8_t q5s16,
                                                   int16x8_t q6s16,
                                                   int16x8_t q7s16) {
  int16x4_t d4s16, d5s16, d6s16, d7s16;

  d4s16 = vld1_s16((int16_t *)p1);
  p1 += stride;
  d7s16 = vld1_s16((int16_t *)p2);
  p2 -= stride;
  d5s16 = vld1_s16((int16_t *)p1);
  d6s16 = vld1_s16((int16_t *)p2);

  q5s16 = vrshrq_n_s16(q5s16, 6);
  q6s16 = vrshrq_n_s16(q6s16, 6);
  q7s16 = vrshrq_n_s16(q7s16, 6);
  q4s16 = vrshrq_n_s16(q4s16, 6);

  q5s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q5s16), vreinterpret_u8_s16(d5s16)));
  q6s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q6s16), vreinterpret_u8_s16(d6s16)));
  q7s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q7s16), vreinterpret_u8_s16(d7s16)));
  q4s16 = vreinterpretq_s16_u16(
      vaddw_u8(vreinterpretq_u16_s16(q4s16), vreinterpret_u8_s16(d4s16)));

  d5s16 = vreinterpret_s16_u8(vqmovun_s16(q5s16));
  d6s16 = vreinterpret_s16_u8(vqmovun_s16(q6s16));
  d7s16 = vreinterpret_s16_u8(vqmovun_s16(q7s16));
  d4s16 = vreinterpret_s16_u8(vqmovun_s16(q4s16));

  vst1_s16((int16_t *)p1, d5s16);
  p1 -= stride;
  vst1_s16((int16_t *)p2, d6s16);
  p2 += stride;
  vst1_s16((int16_t *)p2, d7s16);
  vst1_s16((int16_t *)p1, d4s16);
  return;
}

#define DO_BUTTERFLY_STD(const_1, const_2, qA, qB) \
  DO_BUTTERFLY(q14s16, q13s16, const_1, const_2, qA, qB);
static INLINE void DO_BUTTERFLY(int16x8_t q14s16, int16x8_t q13s16,
                                int16_t first_const, int16_t second_const,
                                int16x8_t *qAs16, int16x8_t *qBs16) {
  int16x4_t d30s16, d31s16;
  int32x4_t q8s32, q9s32, q10s32, q11s32, q12s32, q15s32;
  int16x4_t dCs16, dDs16, dAs16, dBs16;

  dCs16 = vget_low_s16(q14s16);
  dDs16 = vget_high_s16(q14s16);
  dAs16 = vget_low_s16(q13s16);
  dBs16 = vget_high_s16(q13s16);

  d30s16 = vdup_n_s16(first_const);
  d31s16 = vdup_n_s16(second_const);

  q8s32 = vmull_s16(dCs16, d30s16);
  q10s32 = vmull_s16(dAs16, d31s16);
  q9s32 = vmull_s16(dDs16, d30s16);
  q11s32 = vmull_s16(dBs16, d31s16);
  q12s32 = vmull_s16(dCs16, d31s16);

  q8s32 = vsubq_s32(q8s32, q10s32);
  q9s32 = vsubq_s32(q9s32, q11s32);

  q10s32 = vmull_s16(dDs16, d31s16);
  q11s32 = vmull_s16(dAs16, d30s16);
  q15s32 = vmull_s16(dBs16, d30s16);

  q11s32 = vaddq_s32(q12s32, q11s32);
  q10s32 = vaddq_s32(q10s32, q15s32);

  *qAs16 = vcombine_s16(vqrshrn_n_s32(q8s32, 14), vqrshrn_n_s32(q9s32, 14));
  *qBs16 = vcombine_s16(vqrshrn_n_s32(q11s32, 14), vqrshrn_n_s32(q10s32, 14));
  return;
}

static INLINE void idct32_transpose_pair(const tran_low_t *input,
                                         int16_t *t_buf) {
  const int16_t *in;
  int i;
  const int stride = 32;
  int16x8_t q8s16, q9s16, q10s16, q11s16, q12s16, q13s16, q14s16, q15s16;

  for (i = 0; i < 4; i++, input += 8) {
    in = input;
    q8s16 = vld1q_s16(in);
    in += stride;
    q9s16 = vld1q_s16(in);
    in += stride;
    q10s16 = vld1q_s16(in);
    in += stride;
    q11s16 = vld1q_s16(in);
    in += stride;
    q12s16 = vld1q_s16(in);
    in += stride;
    q13s16 = vld1q_s16(in);
    in += stride;
    q14s16 = vld1q_s16(in);
    in += stride;
    q15s16 = vld1q_s16(in);

    transpose_s16_8x8(&q8s16, &q9s16, &q10s16, &q11s16, &q12s16, &q13s16,
                      &q14s16, &q15s16);

    vst1q_s16(t_buf, q8s16);
    t_buf += 8;
    vst1q_s16(t_buf, q9s16);
    t_buf += 8;
    vst1q_s16(t_buf, q10s16);
    t_buf += 8;
    vst1q_s16(t_buf, q11s16);
    t_buf += 8;
    vst1q_s16(t_buf, q12s16);
    t_buf += 8;
    vst1q_s16(t_buf, q13s16);
    t_buf += 8;
    vst1q_s16(t_buf, q14s16);
    t_buf += 8;
    vst1q_s16(t_buf, q15s16);
    t_buf += 8;
  }
  return;
}

static INLINE void idct32_bands_end_1st_pass(int16_t *out, int16x8_t q2s16,
                                             int16x8_t q3s16, int16x8_t q6s16,
                                             int16x8_t q7s16, int16x8_t q8s16,
                                             int16x8_t q9s16, int16x8_t q10s16,
                                             int16x8_t q11s16, int16x8_t q12s16,
                                             int16x8_t q13s16, int16x8_t q14s16,
                                             int16x8_t q15s16) {
  int16x8_t q0s16, q1s16, q4s16, q5s16;

  STORE_IN_OUTPUT(16, 17, q6s16, q7s16);
  STORE_IN_OUTPUT(14, 15, q8s16, q9s16);

  LOAD_FROM_OUTPUT(30, 31, q0s16, q1s16);
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_IN_OUTPUT(30, 31, q6s16, q7s16);
  STORE_IN_OUTPUT(0, 1, q4s16, q5s16);

  LOAD_FROM_OUTPUT(12, 13, q0s16, q1s16);
  q2s16 = vaddq_s16(q10s16, q1s16);
  q3s16 = vaddq_s16(q11s16, q0s16);
  q4s16 = vsubq_s16(q11s16, q0s16);
  q5s16 = vsubq_s16(q10s16, q1s16);

  LOAD_FROM_OUTPUT(18, 19, q0s16, q1s16);
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_IN_OUTPUT(18, 19, q6s16, q7s16);
  STORE_IN_OUTPUT(12, 13, q8s16, q9s16);

  LOAD_FROM_OUTPUT(28, 29, q0s16, q1s16);
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_IN_OUTPUT(28, 29, q6s16, q7s16);
  STORE_IN_OUTPUT(2, 3, q4s16, q5s16);

  LOAD_FROM_OUTPUT(10, 11, q0s16, q1s16);
  q2s16 = vaddq_s16(q12s16, q1s16);
  q3s16 = vaddq_s16(q13s16, q0s16);
  q4s16 = vsubq_s16(q13s16, q0s16);
  q5s16 = vsubq_s16(q12s16, q1s16);

  LOAD_FROM_OUTPUT(20, 21, q0s16, q1s16);
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_IN_OUTPUT(20, 21, q6s16, q7s16);
  STORE_IN_OUTPUT(10, 11, q8s16, q9s16);

  LOAD_FROM_OUTPUT(26, 27, q0s16, q1s16);
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_IN_OUTPUT(26, 27, q6s16, q7s16);
  STORE_IN_OUTPUT(4, 5, q4s16, q5s16);

  LOAD_FROM_OUTPUT(8, 9, q0s16, q1s16);
  q2s16 = vaddq_s16(q14s16, q1s16);
  q3s16 = vaddq_s16(q15s16, q0s16);
  q4s16 = vsubq_s16(q15s16, q0s16);
  q5s16 = vsubq_s16(q14s16, q1s16);

  LOAD_FROM_OUTPUT(22, 23, q0s16, q1s16);
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_IN_OUTPUT(22, 23, q6s16, q7s16);
  STORE_IN_OUTPUT(8, 9, q8s16, q9s16);

  LOAD_FROM_OUTPUT(24, 25, q0s16, q1s16);
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_IN_OUTPUT(24, 25, q6s16, q7s16);
  STORE_IN_OUTPUT(6, 7, q4s16, q5s16);
  return;
}

static INLINE void idct32_bands_end_2nd_pass(
    int16_t *out, uint8_t *dest, int stride, int16x8_t q2s16, int16x8_t q3s16,
    int16x8_t q6s16, int16x8_t q7s16, int16x8_t q8s16, int16x8_t q9s16,
    int16x8_t q10s16, int16x8_t q11s16, int16x8_t q12s16, int16x8_t q13s16,
    int16x8_t q14s16, int16x8_t q15s16) {
  uint8_t *r6 = dest + 31 * stride;
  uint8_t *r7 = dest /* +  0 * stride*/;
  uint8_t *r9 = dest + 15 * stride;
  uint8_t *r10 = dest + 16 * stride;
  int str2 = stride << 1;
  int16x8_t q0s16, q1s16, q4s16, q5s16;

  STORE_COMBINE_CENTER_RESULTS(r10, r9);
  r10 += str2;
  r9 -= str2;

  LOAD_FROM_OUTPUT(30, 31, q0s16, q1s16)
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_COMBINE_EXTREME_RESULTS(r7, r6);
  r7 += str2;
  r6 -= str2;

  LOAD_FROM_OUTPUT(12, 13, q0s16, q1s16)
  q2s16 = vaddq_s16(q10s16, q1s16);
  q3s16 = vaddq_s16(q11s16, q0s16);
  q4s16 = vsubq_s16(q11s16, q0s16);
  q5s16 = vsubq_s16(q10s16, q1s16);

  LOAD_FROM_OUTPUT(18, 19, q0s16, q1s16)
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_COMBINE_CENTER_RESULTS(r10, r9);
  r10 += str2;
  r9 -= str2;

  LOAD_FROM_OUTPUT(28, 29, q0s16, q1s16)
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_COMBINE_EXTREME_RESULTS(r7, r6);
  r7 += str2;
  r6 -= str2;

  LOAD_FROM_OUTPUT(10, 11, q0s16, q1s16)
  q2s16 = vaddq_s16(q12s16, q1s16);
  q3s16 = vaddq_s16(q13s16, q0s16);
  q4s16 = vsubq_s16(q13s16, q0s16);
  q5s16 = vsubq_s16(q12s16, q1s16);

  LOAD_FROM_OUTPUT(20, 21, q0s16, q1s16)
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_COMBINE_CENTER_RESULTS(r10, r9);
  r10 += str2;
  r9 -= str2;

  LOAD_FROM_OUTPUT(26, 27, q0s16, q1s16)
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_COMBINE_EXTREME_RESULTS(r7, r6);
  r7 += str2;
  r6 -= str2;

  LOAD_FROM_OUTPUT(8, 9, q0s16, q1s16)
  q2s16 = vaddq_s16(q14s16, q1s16);
  q3s16 = vaddq_s16(q15s16, q0s16);
  q4s16 = vsubq_s16(q15s16, q0s16);
  q5s16 = vsubq_s16(q14s16, q1s16);

  LOAD_FROM_OUTPUT(22, 23, q0s16, q1s16)
  q8s16 = vaddq_s16(q4s16, q1s16);
  q9s16 = vaddq_s16(q5s16, q0s16);
  q6s16 = vsubq_s16(q5s16, q0s16);
  q7s16 = vsubq_s16(q4s16, q1s16);
  STORE_COMBINE_CENTER_RESULTS(r10, r9);

  LOAD_FROM_OUTPUT(24, 25, q0s16, q1s16)
  q4s16 = vaddq_s16(q2s16, q1s16);
  q5s16 = vaddq_s16(q3s16, q0s16);
  q6s16 = vsubq_s16(q3s16, q0s16);
  q7s16 = vsubq_s16(q2s16, q1s16);
  STORE_COMBINE_EXTREME_RESULTS(r7, r6);
  return;
}

void vpx_idct32x32_34_add_neon(const tran_low_t *input, uint8_t *dest,
                               int stride) {
  int i;
  int16_t trans_buf[32 * 8];
  int16_t pass1[32 * 32];
  int16_t pass2[32 * 32];
  int16_t *out;
  int16x8_t q0s16, q1s16, q2s16, q3s16, q4s16, q5s16, q6s16, q7s16;
  int16x8_t q8s16, q9s16, q10s16, q11s16, q12s16, q13s16, q14s16, q15s16;

  /*******************************************************************************
   * Slightly interesting bit
   ******************************************************************************/
  out = pass1;
  // It's over reading something. When it used a separate buffer (and that was
  // zero'd) it also passed the tests.
  memset(pass1, 0, sizeof(pass1));
  idct32_neon(input, out);

  /*******************************************************************************
   * OK that's over
   ******************************************************************************/
  input = pass1;
  out = pass2;
  for (i = 0; i < 4; i++, input += 32 * 8, out += 8) {  // idct32_bands_loop
    idct32_transpose_pair(input, trans_buf);

    // -----------------------------------------
    // BLOCK A: 16-19,28-31
    // -----------------------------------------
    // generate 16,17,30,31
    // part of stage 1
    LOAD_FROM_TRANSPOSED(1, 31)
    DO_BUTTERFLY_STD(cospi_31_64, cospi_1_64, &q0s16, &q2s16)
    LOAD_FROM_TRANSPOSED(17, 15)
    DO_BUTTERFLY_STD(cospi_15_64, cospi_17_64, &q1s16, &q3s16)
    // part of stage 2
    q4s16 = vaddq_s16(q0s16, q1s16);
    q13s16 = vsubq_s16(q0s16, q1s16);
    q6s16 = vaddq_s16(q2s16, q3s16);
    q14s16 = vsubq_s16(q2s16, q3s16);
    // part of stage 3
    DO_BUTTERFLY_STD(cospi_28_64, cospi_4_64, &q5s16, &q7s16)

    // generate 18,19,28,29
    // part of stage 1
    LOAD_FROM_TRANSPOSED(9, 23)
    DO_BUTTERFLY_STD(cospi_23_64, cospi_9_64, &q0s16, &q2s16)
    LOAD_FROM_TRANSPOSED(25, 7)
    DO_BUTTERFLY_STD(cospi_7_64, cospi_25_64, &q1s16, &q3s16)
    // part of stage 2
    q13s16 = vsubq_s16(q3s16, q2s16);
    q3s16 = vaddq_s16(q3s16, q2s16);
    q14s16 = vsubq_s16(q1s16, q0s16);
    q2s16 = vaddq_s16(q1s16, q0s16);
    // part of stage 3
    DO_BUTTERFLY_STD(-cospi_4_64, -cospi_28_64, &q1s16, &q0s16)
    // part of stage 4
    q8s16 = vaddq_s16(q4s16, q2s16);
    q9s16 = vaddq_s16(q5s16, q0s16);
    q10s16 = vaddq_s16(q7s16, q1s16);
    q15s16 = vaddq_s16(q6s16, q3s16);
    q13s16 = vsubq_s16(q5s16, q0s16);
    q14s16 = vsubq_s16(q7s16, q1s16);
    STORE_IN_OUTPUT(16, 31, q8s16, q15s16)
    STORE_IN_OUTPUT(17, 30, q9s16, q10s16)
    // part of stage 5
    DO_BUTTERFLY_STD(cospi_24_64, cospi_8_64, &q0s16, &q1s16)
    STORE_IN_OUTPUT(29, 18, q1s16, q0s16)
    // part of stage 4
    q13s16 = vsubq_s16(q4s16, q2s16);
    q14s16 = vsubq_s16(q6s16, q3s16);
    // part of stage 5
    DO_BUTTERFLY_STD(cospi_24_64, cospi_8_64, &q4s16, &q6s16)
    STORE_IN_OUTPUT(19, 28, q4s16, q6s16)

    // -----------------------------------------
    // BLOCK B: 20-23,24-27
    // -----------------------------------------
    // generate 20,21,26,27
    // part of stage 1
    LOAD_FROM_TRANSPOSED(5, 27)
    DO_BUTTERFLY_STD(cospi_27_64, cospi_5_64, &q0s16, &q2s16)
    LOAD_FROM_TRANSPOSED(21, 11)
    DO_BUTTERFLY_STD(cospi_11_64, cospi_21_64, &q1s16, &q3s16)
    // part of stage 2
    q13s16 = vsubq_s16(q0s16, q1s16);
    q0s16 = vaddq_s16(q0s16, q1s16);
    q14s16 = vsubq_s16(q2s16, q3s16);
    q2s16 = vaddq_s16(q2s16, q3s16);
    // part of stage 3
    DO_BUTTERFLY_STD(cospi_12_64, cospi_20_64, &q1s16, &q3s16)

    // generate 22,23,24,25
    // part of stage 1
    LOAD_FROM_TRANSPOSED(13, 19)
    DO_BUTTERFLY_STD(cospi_19_64, cospi_13_64, &q5s16, &q7s16)
    LOAD_FROM_TRANSPOSED(29, 3)
    DO_BUTTERFLY_STD(cospi_3_64, cospi_29_64, &q4s16, &q6s16)
    // part of stage 2
    q14s16 = vsubq_s16(q4s16, q5s16);
    q5s16 = vaddq_s16(q4s16, q5s16);
    q13s16 = vsubq_s16(q6s16, q7s16);
    q6s16 = vaddq_s16(q6s16, q7s16);
    // part of stage 3
    DO_BUTTERFLY_STD(-cospi_20_64, -cospi_12_64, &q4s16, &q7s16)
    // part of stage 4
    q10s16 = vaddq_s16(q7s16, q1s16);
    q11s16 = vaddq_s16(q5s16, q0s16);
    q12s16 = vaddq_s16(q6s16, q2s16);
    q15s16 = vaddq_s16(q4s16, q3s16);
    // part of stage 6
    LOAD_FROM_OUTPUT(16, 17, q14s16, q13s16)
    q8s16 = vaddq_s16(q14s16, q11s16);
    q9s16 = vaddq_s16(q13s16, q10s16);
    q13s16 = vsubq_s16(q13s16, q10s16);
    q11s16 = vsubq_s16(q14s16, q11s16);
    STORE_IN_OUTPUT(17, 16, q9s16, q8s16)
    LOAD_FROM_OUTPUT(30, 31, q14s16, q9s16)
    q8s16 = vsubq_s16(q9s16, q12s16);
    q10s16 = vaddq_s16(q14s16, q15s16);
    q14s16 = vsubq_s16(q14s16, q15s16);
    q12s16 = vaddq_s16(q9s16, q12s16);
    STORE_IN_OUTPUT(30, 31, q10s16, q12s16)
    // part of stage 7
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q13s16, &q14s16)
    STORE_IN_OUTPUT(25, 22, q14s16, q13s16)
    q13s16 = q11s16;
    q14s16 = q8s16;
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q13s16, &q14s16)
    STORE_IN_OUTPUT(24, 23, q14s16, q13s16)
    // part of stage 4
    q14s16 = vsubq_s16(q5s16, q0s16);
    q13s16 = vsubq_s16(q6s16, q2s16);
    DO_BUTTERFLY_STD(-cospi_8_64, -cospi_24_64, &q5s16, &q6s16);
    q14s16 = vsubq_s16(q7s16, q1s16);
    q13s16 = vsubq_s16(q4s16, q3s16);
    DO_BUTTERFLY_STD(-cospi_8_64, -cospi_24_64, &q0s16, &q1s16);
    // part of stage 6
    LOAD_FROM_OUTPUT(18, 19, q14s16, q13s16)
    q8s16 = vaddq_s16(q14s16, q1s16);
    q9s16 = vaddq_s16(q13s16, q6s16);
    q13s16 = vsubq_s16(q13s16, q6s16);
    q1s16 = vsubq_s16(q14s16, q1s16);
    STORE_IN_OUTPUT(18, 19, q8s16, q9s16)
    LOAD_FROM_OUTPUT(28, 29, q8s16, q9s16)
    q14s16 = vsubq_s16(q8s16, q5s16);
    q10s16 = vaddq_s16(q8s16, q5s16);
    q11s16 = vaddq_s16(q9s16, q0s16);
    q0s16 = vsubq_s16(q9s16, q0s16);
    STORE_IN_OUTPUT(28, 29, q10s16, q11s16)
    // part of stage 7
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q13s16, &q14s16)
    STORE_IN_OUTPUT(20, 27, q13s16, q14s16)
    DO_BUTTERFLY(q0s16, q1s16, cospi_16_64, cospi_16_64, &q1s16, &q0s16);
    STORE_IN_OUTPUT(21, 26, q1s16, q0s16)

    // -----------------------------------------
    // BLOCK C: 8-10,11-15
    // -----------------------------------------
    // generate 8,9,14,15
    // part of stage 2
    LOAD_FROM_TRANSPOSED(2, 30)
    DO_BUTTERFLY_STD(cospi_30_64, cospi_2_64, &q0s16, &q2s16)
    LOAD_FROM_TRANSPOSED(18, 14)
    DO_BUTTERFLY_STD(cospi_14_64, cospi_18_64, &q1s16, &q3s16)
    // part of stage 3
    q13s16 = vsubq_s16(q0s16, q1s16);
    q0s16 = vaddq_s16(q0s16, q1s16);
    q14s16 = vsubq_s16(q2s16, q3s16);
    q2s16 = vaddq_s16(q2s16, q3s16);
    // part of stage 4
    DO_BUTTERFLY_STD(cospi_24_64, cospi_8_64, &q1s16, &q3s16)

    // generate 10,11,12,13
    // part of stage 2
    LOAD_FROM_TRANSPOSED(10, 22)
    DO_BUTTERFLY_STD(cospi_22_64, cospi_10_64, &q5s16, &q7s16)
    LOAD_FROM_TRANSPOSED(26, 6)
    DO_BUTTERFLY_STD(cospi_6_64, cospi_26_64, &q4s16, &q6s16)
    // part of stage 3
    q14s16 = vsubq_s16(q4s16, q5s16);
    q5s16 = vaddq_s16(q4s16, q5s16);
    q13s16 = vsubq_s16(q6s16, q7s16);
    q6s16 = vaddq_s16(q6s16, q7s16);
    // part of stage 4
    DO_BUTTERFLY_STD(-cospi_8_64, -cospi_24_64, &q4s16, &q7s16)
    // part of stage 5
    q8s16 = vaddq_s16(q0s16, q5s16);
    q9s16 = vaddq_s16(q1s16, q7s16);
    q13s16 = vsubq_s16(q1s16, q7s16);
    q14s16 = vsubq_s16(q3s16, q4s16);
    q10s16 = vaddq_s16(q3s16, q4s16);
    q15s16 = vaddq_s16(q2s16, q6s16);
    STORE_IN_OUTPUT(8, 15, q8s16, q15s16)
    STORE_IN_OUTPUT(9, 14, q9s16, q10s16)
    // part of stage 6
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q1s16, &q3s16)
    STORE_IN_OUTPUT(13, 10, q3s16, q1s16)
    q13s16 = vsubq_s16(q0s16, q5s16);
    q14s16 = vsubq_s16(q2s16, q6s16);
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q1s16, &q3s16)
    STORE_IN_OUTPUT(11, 12, q1s16, q3s16)

    // -----------------------------------------
    // BLOCK D: 0-3,4-7
    // -----------------------------------------
    // generate 4,5,6,7
    // part of stage 3
    LOAD_FROM_TRANSPOSED(4, 28)
    DO_BUTTERFLY_STD(cospi_28_64, cospi_4_64, &q0s16, &q2s16)
    LOAD_FROM_TRANSPOSED(20, 12)
    DO_BUTTERFLY_STD(cospi_12_64, cospi_20_64, &q1s16, &q3s16)
    // part of stage 4
    q13s16 = vsubq_s16(q0s16, q1s16);
    q0s16 = vaddq_s16(q0s16, q1s16);
    q14s16 = vsubq_s16(q2s16, q3s16);
    q2s16 = vaddq_s16(q2s16, q3s16);
    // part of stage 5
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q1s16, &q3s16)

    // generate 0,1,2,3
    // part of stage 4
    LOAD_FROM_TRANSPOSED(0, 16)
    DO_BUTTERFLY_STD(cospi_16_64, cospi_16_64, &q5s16, &q7s16)
    LOAD_FROM_TRANSPOSED(8, 24)
    DO_BUTTERFLY_STD(cospi_24_64, cospi_8_64, &q14s16, &q6s16)
    // part of stage 5
    q4s16 = vaddq_s16(q7s16, q6s16);
    q7s16 = vsubq_s16(q7s16, q6s16);
    q6s16 = vsubq_s16(q5s16, q14s16);
    q5s16 = vaddq_s16(q5s16, q14s16);
    // part of stage 6
    q8s16 = vaddq_s16(q4s16, q2s16);
    q9s16 = vaddq_s16(q5s16, q3s16);
    q10s16 = vaddq_s16(q6s16, q1s16);
    q11s16 = vaddq_s16(q7s16, q0s16);
    q12s16 = vsubq_s16(q7s16, q0s16);
    q13s16 = vsubq_s16(q6s16, q1s16);
    q14s16 = vsubq_s16(q5s16, q3s16);
    q15s16 = vsubq_s16(q4s16, q2s16);
    // part of stage 7
    LOAD_FROM_OUTPUT(14, 15, q0s16, q1s16)
    q2s16 = vaddq_s16(q8s16, q1s16);
    q3s16 = vaddq_s16(q9s16, q0s16);
    q4s16 = vsubq_s16(q9s16, q0s16);
    q5s16 = vsubq_s16(q8s16, q1s16);
    LOAD_FROM_OUTPUT(16, 17, q0s16, q1s16)
    q8s16 = vaddq_s16(q4s16, q1s16);
    q9s16 = vaddq_s16(q5s16, q0s16);
    q6s16 = vsubq_s16(q5s16, q0s16);
    q7s16 = vsubq_s16(q4s16, q1s16);

    idct32_bands_end_2nd_pass(out, dest, stride, q2s16, q3s16, q6s16, q7s16,
                              q8s16, q9s16, q10s16, q11s16, q12s16, q13s16,
                              q14s16, q15s16);
    dest += 8;
  }
}
