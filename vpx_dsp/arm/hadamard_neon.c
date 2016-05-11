/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "./vpx_dsp_rtcd.h"

static void hadamard8x8_one_pass(int16x8_t *a0, int16x8_t *a1,
                                 int16x8_t *a2, int16x8_t *a3,
                                 int16x8_t *a4, int16x8_t *a5,
                                 int16x8_t *a6, int16x8_t *a7) {
  const int16x8_t b0 = vaddq_s16(*a0, *a1);
  const int16x8_t b1 = vsubq_s16(*a0, *a1);
  const int16x8_t b2 = vaddq_s16(*a2, *a3);
  const int16x8_t b3 = vsubq_s16(*a2, *a3);
  const int16x8_t b4 = vaddq_s16(*a4, *a5);
  const int16x8_t b5 = vsubq_s16(*a4, *a5);
  const int16x8_t b6 = vaddq_s16(*a6, *a7);
  const int16x8_t b7 = vsubq_s16(*a6, *a7);

  const int16x8_t c0 = vaddq_s16(b0, b2);
  const int16x8_t c1 = vaddq_s16(b1, b3);
  const int16x8_t c2 = vsubq_s16(b0, b2);
  const int16x8_t c3 = vsubq_s16(b1, b3);
  const int16x8_t c4 = vaddq_s16(b4, b6);
  const int16x8_t c5 = vaddq_s16(b5, b7);
  const int16x8_t c6 = vsubq_s16(b4, b6);
  const int16x8_t c7 = vsubq_s16(b5, b7);

  *a0 = vaddq_s16(c0, c4);
  *a1 = vsubq_s16(c2, c6);
  *a2 = vsubq_s16(c0, c4);
  *a3 = vaddq_s16(c2, c6);
  *a4 = vaddq_s16(c3, c7);
  *a5 = vsubq_s16(c3, c7);
  *a6 = vsubq_s16(c1, c5);
  *a7 = vaddq_s16(c1, c5);
}

static void transpose8x8(int16x8_t *a0, int16x8_t *a1,
                         int16x8_t *a2, int16x8_t *a3,
                         int16x8_t *a4, int16x8_t *a5,
                         int16x8_t *a6, int16x8_t *a7) {
  const int16x8_t a04_lo = vcombine_s16(vget_low_s16(*a0), vget_low_s16(*a4));
  const int16x8_t a15_lo = vcombine_s16(vget_low_s16(*a1), vget_low_s16(*a5));
  const int16x8_t a26_lo = vcombine_s16(vget_low_s16(*a2), vget_low_s16(*a6));
  const int16x8_t a37_lo = vcombine_s16(vget_low_s16(*a3), vget_low_s16(*a7));
  const int16x8_t a04_hi = vcombine_s16(vget_high_s16(*a0), vget_high_s16(*a4));
  const int16x8_t a15_hi = vcombine_s16(vget_high_s16(*a1), vget_high_s16(*a5));
  const int16x8_t a26_hi = vcombine_s16(vget_high_s16(*a2), vget_high_s16(*a6));
  const int16x8_t a37_hi = vcombine_s16(vget_high_s16(*a3), vget_high_s16(*a7));

  const int32x4x2_t a0246_lo = vtrnq_s32(vreinterpretq_s32_s16(a04_lo),
                                         vreinterpretq_s32_s16(a26_lo));
  const int32x4x2_t a1357_lo = vtrnq_s32(vreinterpretq_s32_s16(a15_lo),
                                         vreinterpretq_s32_s16(a37_lo));
  const int32x4x2_t a0246_hi = vtrnq_s32(vreinterpretq_s32_s16(a04_hi),
                                         vreinterpretq_s32_s16(a26_hi));
  const int32x4x2_t a1357_hi = vtrnq_s32(vreinterpretq_s32_s16(a15_hi),
                                         vreinterpretq_s32_s16(a37_hi));

  const int16x8x2_t b0 = vtrnq_s16(vreinterpretq_s16_s32(a0246_lo.val[0]),
                         vreinterpretq_s16_s32(a1357_lo.val[0]));
  const int16x8x2_t b1 = vtrnq_s16(vreinterpretq_s16_s32(a0246_lo.val[1]),
                         vreinterpretq_s16_s32(a1357_lo.val[1]));
  const int16x8x2_t b2 = vtrnq_s16(vreinterpretq_s16_s32(a0246_hi.val[0]),
                         vreinterpretq_s16_s32(a1357_hi.val[0]));
  const int16x8x2_t b3 = vtrnq_s16(vreinterpretq_s16_s32(a0246_hi.val[1]),
                         vreinterpretq_s16_s32(a1357_hi.val[1]));

  *a0 = b0.val[0];
  *a1 = b0.val[1];
  *a2 = b1.val[0];
  *a3 = b1.val[1];
  *a4 = b2.val[0];
  *a5 = b2.val[1];
  *a6 = b3.val[0];
  *a7 = b3.val[1];
}

void vpx_hadamard_8x8_neon(const int16_t *src_diff, int src_stride,
                           int16_t *coeff) {
  int16x8_t a0 = vld1q_s16(src_diff);
  int16x8_t a1 = vld1q_s16(src_diff + src_stride);
  int16x8_t a2 = vld1q_s16(src_diff + 2 * src_stride);
  int16x8_t a3 = vld1q_s16(src_diff + 3 * src_stride);
  int16x8_t a4 = vld1q_s16(src_diff + 4 * src_stride);
  int16x8_t a5 = vld1q_s16(src_diff + 5 * src_stride);
  int16x8_t a6 = vld1q_s16(src_diff + 6 * src_stride);
  int16x8_t a7 = vld1q_s16(src_diff + 7 * src_stride);

  hadamard8x8_one_pass(&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7);

  transpose8x8(&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7);

  hadamard8x8_one_pass(&a0, &a1, &a2, &a3, &a4, &a5, &a6, &a7);

  // Skip the second transpose because it is not required.

  vst1q_s16(coeff + 0, a0);
  vst1q_s16(coeff + 8, a1);
  vst1q_s16(coeff + 16, a2);
  vst1q_s16(coeff + 24, a3);
  vst1q_s16(coeff + 32, a4);
  vst1q_s16(coeff + 40, a5);
  vst1q_s16(coeff + 48, a6);
  vst1q_s16(coeff + 56, a7);
}
