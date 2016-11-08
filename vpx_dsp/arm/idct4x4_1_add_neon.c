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
#include "vpx_dsp/inv_txfm.h"

void vpx_idct4x4_1_add_neon(const tran_low_t *input, uint8_t *dest,
                            int dest_stride) {
  int i;
  const int16_t out0 = dct_const_round_shift((int16_t)input[0] * cospi_16_64);
  const int16_t out1 = dct_const_round_shift(out0 * cospi_16_64);
  const int16_t a1 = ROUND_POWER_OF_TWO(out1, 4);
  const int16x8_t dc = vdupq_n_s16(a1);
  uint32x2_t d0 = vmov_n_u32(0);
  const uint8_t *dest0 = dest;
  uint8_t *dest1 = dest;
  uint16x8_t d;
  uint8x8_t row;

  for (i = 0; i < 2; i++) {
    d0 = vld1_lane_u32((const uint32_t *)dest0, d0, 0);
    dest0 += dest_stride;
    d0 = vld1_lane_u32((const uint32_t *)dest0, d0, 1);
    dest0 += dest_stride;
    d = vaddw_u8(vreinterpretq_u16_s16(dc), vreinterpret_u8_u32(d0));
    row = vqmovun_s16(vreinterpretq_s16_u16(d));
    vst1_lane_u32((uint32_t *)dest1, vreinterpret_u32_u8(row), 0);
    dest1 += dest_stride;
    vst1_lane_u32((uint32_t *)dest1, vreinterpret_u32_u8(row), 1);
    dest1 += dest_stride;
  }
}
