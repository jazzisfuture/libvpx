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

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/idct_neon.h"
#include "vpx_dsp/txfm_common.h"

void vpx_idct4x4_16_add_neon(const tran_low_t *input, uint8_t *dest,
                             int dest_stride) {
  const int16x4_t cospis = vld1_s16(cospi);
  uint32x2_t dest01_u32 = vmov_n_u32(0);
  uint32x2_t dest32_u32 = vmov_n_u32(0);
  uint8x8_t dest01, dest32;
  uint16x8_t dest01_u16, dest32_u16;
  int16x8_t s0, s1;
  uint8_t *d;

  assert(!((intptr_t)dest % sizeof(uint32_t)));
  assert(!(dest_stride % sizeof(uint32_t)));

  // Rows
  s0 = load_tran_low_to_s16(input);
  s1 = load_tran_low_to_s16(input + 8);
  idct4x4_16_kernel(cospis, &s0, &s1);

  // Columns
  s1 = vcombine_s16(vget_high_s16(s1), vget_low_s16(s1));
  idct4x4_16_kernel(cospis, &s0, &s1);
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
