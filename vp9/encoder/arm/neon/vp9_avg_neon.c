/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>
#include "./vp9_rtcd.h"
#include "./vpx_config.h"

#include "vpx/vpx_integer.h"

static INLINE unsigned int horizontal_add_u16x8(const uint16x8_t v_16x8) {
  const uint32x4_t a = vpaddlq_u16(v_16x8);
  const uint64x2_t b = vpaddlq_u32(a);
  const uint32x2_t c = vadd_u32(vreinterpret_u32_u64(vget_low_u64(b)),
                                vreinterpret_u32_u64(vget_high_u64(b)));
  return vget_lane_u32(c, 0);
}

unsigned int vp9_avg_8x8_neon(const uint8_t *s, int p) {
  uint8x8_t v_s0 = vld1_u8(s);
  const uint8x8_t v_s1 = vld1_u8(s + p);
  uint16x8_t v_sum = vaddl_u8(v_s0, v_s1);

  v_s0 = vld1_u8(s + 2 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  v_s0 = vld1_u8(s + 3 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  v_s0 = vld1_u8(s + 4 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  v_s0 = vld1_u8(s + 5 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  v_s0 = vld1_u8(s + 6 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  v_s0 = vld1_u8(s + 7 * p);
  v_sum = vaddw_u8(v_sum, v_s0);

  return (horizontal_add_u16x8(v_sum) + 32) >> 6;
}

void vp9_int_pro_row_neon(int16_t hbuf[16], uint8_t const *ref,
                          const int ref_stride, const int height) {
  int i;
  uint16x8_t vec_sum_lo = vdupq_n_u16(0);
  uint16x8_t vec_sum_hi = vdupq_n_u16(0);

  for (i = 0; i < height; ++i) {
    const uint8x16_t vec_row = vld1q_u8(ref);
    vec_sum_lo = vaddw_u8(vec_sum_lo, vget_low_u8(vec_row));
    vec_sum_hi = vaddw_u8(vec_sum_hi, vget_high_u8(vec_row));
    ref += ref_stride;
  }

  if (height == 64) {
    vec_sum_lo = vshrq_n_u16(vec_sum_lo, 5);
    vec_sum_hi = vshrq_n_u16(vec_sum_hi, 5);
  } else if (height == 32) {
    vec_sum_lo = vshrq_n_u16(vec_sum_lo, 4);
    vec_sum_hi = vshrq_n_u16(vec_sum_hi, 4);
  } else {
    vec_sum_lo = vshrq_n_u16(vec_sum_lo, 3);
    vec_sum_hi = vshrq_n_u16(vec_sum_hi, 3);
  }

  vst1q_s16(hbuf, vreinterpretq_s16_u16(vec_sum_lo));
  hbuf += 8;
  vst1q_s16(hbuf, vreinterpretq_s16_u16(vec_sum_hi));
}
