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
#include "./vp9_rtcd.h"
#include "./vpx_config.h"

#include "vpx/vpx_integer.h"

unsigned int vp9_sad32x32_neon(const uint8_t *src, int src_stride,
                               const uint8_t *ref, int ref_stride) {
  int i;
  uint16x8_t vec_accum_lo = vdupq_n_u16(0);
  uint16x8_t vec_accum_hi = vdupq_n_u16(0);

  for (i = 0; i < 32; i++) {
    const uint8x16_t vec_src = vld1q_u8(src);
    const uint8x16_t vec_src_16 = vld1q_u8(src + 16);
    const uint8x16_t vec_ref = vld1q_u8(ref);
    const uint8x16_t vec_ref_16 = vld1q_u8(ref + 16);
    src += src_stride;
    ref += ref_stride;
    vec_accum_lo = vabal_u8(vec_accum_lo, vget_low_u8(vec_src),
                            vget_low_u8(vec_ref));
    vec_accum_hi = vabal_u8(vec_accum_hi, vget_high_u8(vec_src),
                            vget_high_u8(vec_ref));
    vec_accum_lo = vabal_u8(vec_accum_lo, vget_low_u8(vec_src_16),
                            vget_low_u8(vec_ref_16));
    vec_accum_hi = vabal_u8(vec_accum_hi, vget_high_u8(vec_src_16),
                            vget_high_u8(vec_ref_16));
  }
  {
    const uint32x4_t a = vpaddlq_u16(vaddq_u16(vec_accum_lo, vec_accum_hi));
    const uint64x2_t b = vpaddlq_u32(a);
    const uint32x2_t c = vadd_u32(vreinterpret_u32_u64(vget_low_u64(b)),
                                  vreinterpret_u32_u64(vget_high_u64(b)));
    return vget_lane_u32(c, 0);
  }
}

unsigned int vp9_sad16x16_neon(const uint8_t *src, int src_stride,
                               const uint8_t *ref, int ref_stride) {
  int i;
  uint16x8_t vec_accum_lo = vdupq_n_u16(0);
  uint16x8_t vec_accum_hi = vdupq_n_u16(0);

  for (i = 0; i < 16; i++) {
    const uint8x16_t vec_src = vld1q_u8(src);
    const uint8x16_t vec_ref = vld1q_u8(ref);
    src += src_stride;
    ref += ref_stride;
    vec_accum_lo = vabal_u8(vec_accum_lo, vget_low_u8(vec_src),
                            vget_low_u8(vec_ref));
    vec_accum_hi = vabal_u8(vec_accum_hi, vget_high_u8(vec_src),
                            vget_high_u8(vec_ref));
  }
  {
    const uint32x4_t a = vpaddlq_u16(vaddq_u16(vec_accum_lo, vec_accum_hi));
    const uint64x2_t b = vpaddlq_u32(a);
    const uint32x2_t c = vadd_u32(vreinterpret_u32_u64(vget_low_u64(b)),
                                  vreinterpret_u32_u64(vget_high_u64(b)));
    return vget_lane_u32(c, 0);
  }
}
