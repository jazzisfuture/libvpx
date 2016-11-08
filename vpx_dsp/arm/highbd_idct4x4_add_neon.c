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
  const int16x8_t max = vdupq_n_s16((1 << bd) - 1);
  const int16_t out0 =
      ROUND_POWER_OF_TWO((int16_t)input[0] * cospi_16_64, DCT_CONST_BITS);
  const int16_t out1 = ROUND_POWER_OF_TWO(out0 * cospi_16_64, DCT_CONST_BITS);
  const int16_t a1 = ROUND_POWER_OF_TWO(out1, 4);
  const int16x8_t dc = vdupq_n_s16(a1);
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  int16x8_t a;
  uint16x8_t b;
  uint16x4_t d0, d1;

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
  const int16x8_t max = vdupq_n_s16((1 << bd) - 1);
  const int16x4_t cospis = vld1_s16(kCospi);
  uint16_t *dest = CONVERT_TO_SHORTPTR(dest8);
  const uint16_t *dst = dest;
  int16x8_t a0, a1;
  int16x4_t d0, d1, d2, d3;
  int16x8_t d01, d32;
  uint16x8_t d01_u16, d32_u16;

  // Rows
  a0 = load_tran_low_to_s16(input);
  a1 = load_tran_low_to_s16(input + 8);
  idct4x4_16_kernel(cospis, &a0, &a1);

  // Columns
  a1 = vcombine_s16(vget_high_s16(a1), vget_low_s16(a1));
  idct4x4_16_kernel(cospis, &a0, &a1);
  a0 = vrshrq_n_s16(a0, 4);
  a1 = vrshrq_n_s16(a1, 4);

  d0 = vld1_s16((const int16_t *)dst);
  dst += dest_stride;
  d1 = vld1_s16((const int16_t *)dst);
  dst += dest_stride;
  d2 = vld1_s16((const int16_t *)dst);
  dst += dest_stride;
  d3 = vld1_s16((const int16_t *)dst);
  d01 = vcombine_s16(d0, d1);
  d32 = vcombine_s16(d3, d2);

  d01 = vaddq_s16(a0, d01);
  d32 = vaddq_s16(a1, d32);
  d01 = vminq_s16(d01, max);
  d32 = vminq_s16(d32, max);
  d01_u16 = vqshluq_n_s16(d01, 0);
  d32_u16 = vqshluq_n_s16(d32, 0);

  vst1_u16(dest, vget_low_u16(d01_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_high_u16(d01_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_high_u16(d32_u16));
  dest += dest_stride;
  vst1_u16(dest, vget_low_u16(d32_u16));
}
