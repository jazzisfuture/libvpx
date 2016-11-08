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
#include "vpx_dsp/arm/idct_neon.h"
#include "vpx_dsp/inv_txfm.h"

void vpx_highbd_idct4x4_1_add_neon(const tran_low_t *input, uint8_t *dest8,
                                   int dest_stride, int bd) {
  int i;
  const int16x8_t max = vmovq_n_s16((1 << bd) - 1);
  const int16_t out0 =
      ROUND_POWER_OF_TWO((int16_t)input[0] * cospi_16_64, DCT_CONST_BITS);
  const int16_t out1 = ROUND_POWER_OF_TWO(out0 * cospi_16_64, DCT_CONST_BITS);
  const int16_t a1 = ROUND_POWER_OF_TWO(out1, 4);
  const int16x8_t dc = vdupq_n_s16(a1);
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  uint16x4_t d0, d1;
  int16x8_t a;
  uint16x8_t b;

  for (i = 0; i < 2; i++) {
    d0 = vld1_u16(dest);
    d1 = vld1_u16(dest + dest_stride);
    a = vreinterpretq_s16_u16(vcombine_u16(d0, d1));
    a = vaddq_s16(dc, a);
    a = vminq_s16(a, max);
    b = vqshluq_n_s16(a, 0);
    vst1_u16(dest, vget_low_u16(b));
    dest += dest_stride;
    vst1_u16(dest, vget_high_u16(b));
    dest += dest_stride;
  }
}

void vpx_highbd_idct4x4_16_add_neon(const tran_low_t *input, uint8_t *dest8,
                                    int dest_stride, int bd) {
  const int16x8_t max = vmovq_n_s16((1 << bd) - 1);
  const int16x4_t d20s16 = vld1_s16(cospi);
  int16x4_t dest0, dest1, dest2, dest3;
  int16x8_t dest01, dest32;
  uint16x8_t dest01_u16, dest32_u16;
  int16x8_t s0, s1;
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  uint16_t *d;

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
  dest0 = vld1_s16((const int16_t *)d);
  d += dest_stride;
  dest1 = vld1_s16((const int16_t *)d);
  d += dest_stride;
  dest2 = vld1_s16((const int16_t *)d);
  d += dest_stride;
  dest3 = vld1_s16((const int16_t *)d);
  dest01 = vcombine_s16(dest0, dest1);
  dest32 = vcombine_s16(dest3, dest2);

  dest01 = vaddq_s16(s0, dest01);
  dest32 = vaddq_s16(s1, dest32);
  dest01 = vminq_s16(dest01, max);
  dest32 = vminq_s16(dest32, max);
  dest01_u16 = vqshluq_n_s16(dest01, 0);
  dest32_u16 = vqshluq_n_s16(dest32, 0);

  vst1_u16(dest, vget_low_u16(dest01_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_high_u16(dest01_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_high_u16(dest32_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_low_u16(dest32_u16));
}
