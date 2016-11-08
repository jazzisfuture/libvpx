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

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/idct_neon.h"
#include "vpx_dsp/arm/transpose_neon.h"
#include "vpx_dsp/txfm_common.h"

static INLINE void idct4x4_16_kernel(const int16x4_t d20s16, int16x8_t *s0,
                                     int16x8_t *s1) {
  int16x4_t d0, d1, d2, d3, d4, d5;
  int16x8_t q4, q5;
  int32x4_t q3, q0, q1, q2;

  transpose_s16_4x4q(s0, s1);
  d0 = vget_low_s16(*s0);
  d1 = vget_high_s16(*s0);
  d2 = vget_low_s16(*s1);
  d3 = vget_high_s16(*s1);
  d4 = vadd_s16(d0, d1);
  d5 = vsub_s16(d0, d1);
  q0 = vmull_lane_s16(d4, d20s16, 2);
  q1 = vmull_lane_s16(d5, d20s16, 2);
  q2 = vmull_lane_s16(d2, d20s16, 3);
  q3 = vmull_lane_s16(d2, d20s16, 1);
  q2 = vmlsl_lane_s16(q2, d3, d20s16, 1);
  q3 = vmlal_lane_s16(q3, d3, d20s16, 3);
  d0 = vqrshrn_n_s32(q0, 14);
  d1 = vqrshrn_n_s32(q1, 14);
  d2 = vqrshrn_n_s32(q2, 14);
  d3 = vqrshrn_n_s32(q3, 14);
  q4 = vcombine_s16(d0, d1);
  q5 = vcombine_s16(d3, d2);
  *s0 = vaddq_s16(q4, q5);
  *s1 = vsubq_s16(q4, q5);
}

void vpx_idct4x4_16_add_neon(const tran_low_t *input, uint8_t *dest,
                             int dest_stride) {
  DECLARE_ALIGNED(16, static const int16_t, cospi[4]) = {
    0, (int16_t)cospi_8_64, (int16_t)cospi_16_64, (int16_t)cospi_24_64
  };
  const int16x4_t d20s16 = vld1_s16(cospi);
  uint32x2_t dest01_u32 = vmov_n_u32(0);
  uint32x2_t dest32_u32 = vmov_n_u32(0);
  uint8x8_t dest01, dest32;
  uint16x8_t dest01_u16, dest32_u16;
  int16x8_t s0, s1;
  uint8_t *d;

  // Rows
  s0 = load_tran_low_to_s16(input);
  s1 = load_tran_low_to_s16(input + 8);
  idct4x4_16_kernel(d20s16, &s0, &s1);

  // Columns
  s1 = vcombine_s16(vget_high_s16(s1), vget_low_s16(s1));
  idct4x4_16_kernel(d20s16, &s0, &s1);
  s0 = vrshrq_n_s16(s0, 4);
  s1 = vrshrq_n_s16(s1, 4);

  d = dest;
  dest01_u32 = vld1_lane_u32((const uint32_t *)d, dest01_u32, 0);
  d += dest_stride;
  dest01_u32 = vld1_lane_u32((const uint32_t *)d, dest01_u32, 1);
  d += dest_stride;
  dest32_u32 = vld1_lane_u32((const uint32_t *)d, dest32_u32, 1);
  d += dest_stride;
  dest32_u32 = vld1_lane_u32((const uint32_t *)d, dest32_u32, 0);

  dest01_u16 =
      vaddw_u8(vreinterpretq_u16_s16(s0), vreinterpret_u8_u32(dest01_u32));
  dest32_u16 =
      vaddw_u8(vreinterpretq_u16_s16(s1), vreinterpret_u8_u32(dest32_u32));
  dest01 = vqmovun_s16(vreinterpretq_s16_u16(dest01_u16));
  dest32 = vqmovun_s16(vreinterpretq_s16_u16(dest32_u16));

  vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(dest01), 0);
  dest += dest_stride;
  vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(dest01), 1);
  dest += dest_stride;
  vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(dest32), 1);
  dest += dest_stride;
  vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(dest32), 0);
}
