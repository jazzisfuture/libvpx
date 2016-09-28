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

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/arm/transpose_neon.h"
#include "vpx_ports/mem.h"

// Note:
// 1. src is not always 32-bit aligned, so don't call vld1_lane_u32(src).
// 2. After refactoring the shared code in kernel loops with inline functions,
// the decoder speed dropped a lot when using gcc compiler. Therefore there is
// no refactoring for those parts by now.
// 3. For horizontal convolve, there is an alternative optimization that
// convolves a single row in each loop. For each row, 8 sample banks with 4 or 8
// samples in each are read from memory: src, (src+1), (src+2), (src+3),
// (src+4), (src+5), (src+6), (src+7), or prepared by vector extract
// instructions. This optimization is much faster in speed unit test, but slowed
// down the whole decoder by 5%.

#ifndef FUNC_MULTIPLY_BY_Q0
#define FUNC_MULTIPLY_BY_Q0
static INLINE int32x4_t MULTIPLY_BY_Q0(int16x4_t dsrc0, int16x4_t dsrc1,
                                       int16x4_t dsrc2, int16x4_t dsrc3,
                                       int16x4_t dsrc4, int16x4_t dsrc5,
                                       int16x4_t dsrc6, int16x4_t dsrc7,
                                       int16x8_t q0s16) {
  int32x4_t qdst;
  int16x4_t d0s16, d1s16;

  d0s16 = vget_low_s16(q0s16);
  d1s16 = vget_high_s16(q0s16);

  qdst = vmull_lane_s16(dsrc0, d0s16, 0);
  qdst = vmlal_lane_s16(qdst, dsrc1, d0s16, 1);
  qdst = vmlal_lane_s16(qdst, dsrc2, d0s16, 2);
  qdst = vmlal_lane_s16(qdst, dsrc3, d0s16, 3);
  qdst = vmlal_lane_s16(qdst, dsrc4, d1s16, 0);
  qdst = vmlal_lane_s16(qdst, dsrc5, d1s16, 1);
  qdst = vmlal_lane_s16(qdst, dsrc6, d1s16, 2);
  qdst = vmlal_lane_s16(qdst, dsrc7, d1s16, 3);
  return qdst;
}
#endif

void vpx_convolve8_horiz_neon_org(const uint8_t *src, ptrdiff_t src_stride,
                                  uint8_t *dst, ptrdiff_t dst_stride,
                                  const int16_t *filter_x, int x_step_q4,
                                  const int16_t *filter_y,  // unused
                                  int y_step_q4,            // unused
                                  int w, int h) {
  int width;
  const uint8_t *s, *psrc;
  uint8_t *d, *pdst;
  uint8x8_t d2u8, d3u8, d24u8, d25u8, d26u8, d27u8, d28u8, d29u8;
  uint32x2_t d2u32, d3u32, d28u32, d29u32, d30u32, d31u32;
  uint8x16_t q12u8, q13u8, q14u8, q15u8;
  int16x4_t d16s16, d17s16, d18s16, d19s16, d20s16, d22s16, d23s16;
  int16x4_t d24s16, d25s16, d26s16, d27s16;
  uint16x4_t d2u16, d3u16, d4u16, d5u16, d16u16, d17u16, d18u16, d19u16;
  int16x8_t q0s16;
  uint16x8_t q1u16, q2u16, q8u16, q9u16, q10u16, q11u16, q12u16, q13u16;
  int32x4_t q1s32, q2s32, q14s32, q15s32;
  uint16x8x2_t q0x2u16;
  uint8x8x2_t d0x2u8, d1x2u8;
  uint32x2x2_t d0x2u32;
  uint16x4x2_t d0x2u16, d1x2u16;
  uint32x4x2_t q0x2u32;

  assert(x_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_y;

  q0s16 = vld1q_s16(filter_x);

  src -= 3;  // adjust for taps
  for (; h > 0; h -= 4, src += src_stride * 4,
                dst += dst_stride * 4) {  // loop_horiz_v
    s = src;
    d24u8 = vld1_u8(s);
    s += src_stride;
    d25u8 = vld1_u8(s);
    s += src_stride;
    d26u8 = vld1_u8(s);
    s += src_stride;
    d27u8 = vld1_u8(s);

    q12u8 = vcombine_u8(d24u8, d25u8);
    q13u8 = vcombine_u8(d26u8, d27u8);

    q0x2u16 =
        vtrnq_u16(vreinterpretq_u16_u8(q12u8), vreinterpretq_u16_u8(q13u8));
    d24u8 = vreinterpret_u8_u16(vget_low_u16(q0x2u16.val[0]));
    d25u8 = vreinterpret_u8_u16(vget_high_u16(q0x2u16.val[0]));
    d26u8 = vreinterpret_u8_u16(vget_low_u16(q0x2u16.val[1]));
    d27u8 = vreinterpret_u8_u16(vget_high_u16(q0x2u16.val[1]));
    d0x2u8 = vtrn_u8(d24u8, d25u8);
    d1x2u8 = vtrn_u8(d26u8, d27u8);

    __builtin_prefetch(src + src_stride * 4);
    __builtin_prefetch(src + src_stride * 5);
    __builtin_prefetch(src + src_stride * 6);

    q8u16 = vmovl_u8(d0x2u8.val[0]);
    q9u16 = vmovl_u8(d0x2u8.val[1]);
    q10u16 = vmovl_u8(d1x2u8.val[0]);
    q11u16 = vmovl_u8(d1x2u8.val[1]);

    d16u16 = vget_low_u16(q8u16);
    d17u16 = vget_high_u16(q8u16);
    d18u16 = vget_low_u16(q9u16);
    d19u16 = vget_high_u16(q9u16);
    q8u16 = vcombine_u16(d16u16, d18u16);  // vswp 17 18
    q9u16 = vcombine_u16(d17u16, d19u16);

    d20s16 = vreinterpret_s16_u16(vget_low_u16(q10u16));
    d23s16 = vreinterpret_s16_u16(vget_high_u16(q10u16));  // vmov 23 21
    for (width = w, psrc = src + 7, pdst = dst; width > 0;
         width -= 4, psrc += 4, pdst += 4) {  // loop_horiz
      s = psrc;
      d28u32 = vld1_dup_u32((const uint32_t *)s);
      s += src_stride;
      d29u32 = vld1_dup_u32((const uint32_t *)s);
      s += src_stride;
      d31u32 = vld1_dup_u32((const uint32_t *)s);
      s += src_stride;
      d30u32 = vld1_dup_u32((const uint32_t *)s);

      __builtin_prefetch(psrc + 64);

      d0x2u16 =
          vtrn_u16(vreinterpret_u16_u32(d28u32), vreinterpret_u16_u32(d31u32));
      d1x2u16 =
          vtrn_u16(vreinterpret_u16_u32(d29u32), vreinterpret_u16_u32(d30u32));
      d0x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[0]),   // d28
                       vreinterpret_u8_u16(d1x2u16.val[0]));  // d29
      d1x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[1]),   // d31
                       vreinterpret_u8_u16(d1x2u16.val[1]));  // d30

      __builtin_prefetch(psrc + 64 + src_stride);

      q14u8 = vcombine_u8(d0x2u8.val[0], d0x2u8.val[1]);
      q15u8 = vcombine_u8(d1x2u8.val[1], d1x2u8.val[0]);
      q0x2u32 =
          vtrnq_u32(vreinterpretq_u32_u8(q14u8), vreinterpretq_u32_u8(q15u8));

      d28u8 = vreinterpret_u8_u32(vget_low_u32(q0x2u32.val[0]));
      d29u8 = vreinterpret_u8_u32(vget_high_u32(q0x2u32.val[0]));
      q12u16 = vmovl_u8(d28u8);
      q13u16 = vmovl_u8(d29u8);

      __builtin_prefetch(psrc + 64 + src_stride * 2);

      d16s16 = vreinterpret_s16_u16(vget_low_u16(q8u16));
      d17s16 = vreinterpret_s16_u16(vget_high_u16(q8u16));
      d18s16 = vreinterpret_s16_u16(vget_low_u16(q9u16));
      d19s16 = vreinterpret_s16_u16(vget_high_u16(q9u16));
      d22s16 = vreinterpret_s16_u16(vget_low_u16(q11u16));
      d24s16 = vreinterpret_s16_u16(vget_low_u16(q12u16));
      d25s16 = vreinterpret_s16_u16(vget_high_u16(q12u16));
      d26s16 = vreinterpret_s16_u16(vget_low_u16(q13u16));
      d27s16 = vreinterpret_s16_u16(vget_high_u16(q13u16));

      q1s32 = MULTIPLY_BY_Q0(d16s16, d17s16, d20s16, d22s16, d18s16, d19s16,
                             d23s16, d24s16, q0s16);
      q2s32 = MULTIPLY_BY_Q0(d17s16, d20s16, d22s16, d18s16, d19s16, d23s16,
                             d24s16, d26s16, q0s16);
      q14s32 = MULTIPLY_BY_Q0(d20s16, d22s16, d18s16, d19s16, d23s16, d24s16,
                              d26s16, d27s16, q0s16);
      q15s32 = MULTIPLY_BY_Q0(d22s16, d18s16, d19s16, d23s16, d24s16, d26s16,
                              d27s16, d25s16, q0s16);

      __builtin_prefetch(psrc + 60 + src_stride * 3);

      d2u16 = vqrshrun_n_s32(q1s32, 7);
      d3u16 = vqrshrun_n_s32(q2s32, 7);
      d4u16 = vqrshrun_n_s32(q14s32, 7);
      d5u16 = vqrshrun_n_s32(q15s32, 7);

      q1u16 = vcombine_u16(d2u16, d3u16);
      q2u16 = vcombine_u16(d4u16, d5u16);

      d2u8 = vqmovn_u16(q1u16);
      d3u8 = vqmovn_u16(q2u16);

      d0x2u16 = vtrn_u16(vreinterpret_u16_u8(d2u8), vreinterpret_u16_u8(d3u8));
      d0x2u32 = vtrn_u32(vreinterpret_u32_u16(d0x2u16.val[0]),
                         vreinterpret_u32_u16(d0x2u16.val[1]));
      d0x2u8 = vtrn_u8(vreinterpret_u8_u32(d0x2u32.val[0]),
                       vreinterpret_u8_u32(d0x2u32.val[1]));

      d2u32 = vreinterpret_u32_u8(d0x2u8.val[0]);
      d3u32 = vreinterpret_u32_u8(d0x2u8.val[1]);

      d = pdst;
      vst1_lane_u32((uint32_t *)d, d2u32, 0);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d3u32, 0);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d2u32, 1);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d3u32, 1);

      q8u16 = q9u16;
      d20s16 = d23s16;
      q11u16 = q12u16;
      q9u16 = q13u16;
      d23s16 = vreinterpret_s16_u16(vget_high_u16(q11u16));
    }
  }
  return;
}

void vpx_convolve8_vert_neon_org(const uint8_t *src, ptrdiff_t src_stride,
                                 uint8_t *dst, ptrdiff_t dst_stride,
                                 const int16_t *filter_x,  // unused
                                 int x_step_q4,            // unused
                                 const int16_t *filter_y, int y_step_q4, int w,
                                 int h) {
  int height;
  const uint8_t *s;
  uint8_t *d;
  uint32x2_t d2u32, d3u32;
  uint32x2_t d16u32, d18u32, d20u32, d22u32, d24u32, d26u32;
  int16x4_t d16s16, d17s16, d18s16, d19s16, d20s16, d21s16, d22s16;
  int16x4_t d24s16, d25s16, d26s16, d27s16;
  uint16x4_t d2u16, d3u16, d4u16, d5u16;
  int16x8_t q0s16;
  uint16x8_t q1u16, q2u16, q8u16, q9u16, q10u16, q11u16, q12u16, q13u16;
  int32x4_t q1s32, q2s32, q14s32, q15s32;

  assert(y_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_x;

  src -= src_stride * 3;
  q0s16 = vld1q_s16(filter_y);
  for (; w > 0; w -= 4, src += 4, dst += 4) {  // loop_vert_h
    s = src;
    d16u32 = vld1_lane_u32((const uint32_t *)s, d16u32, 0);
    s += src_stride;
    d16u32 = vld1_lane_u32((const uint32_t *)s, d16u32, 1);
    s += src_stride;
    d18u32 = vld1_lane_u32((const uint32_t *)s, d18u32, 0);
    s += src_stride;
    d18u32 = vld1_lane_u32((const uint32_t *)s, d18u32, 1);
    s += src_stride;
    d20u32 = vld1_lane_u32((const uint32_t *)s, d20u32, 0);
    s += src_stride;
    d20u32 = vld1_lane_u32((const uint32_t *)s, d20u32, 1);
    s += src_stride;
    d22u32 = vld1_lane_u32((const uint32_t *)s, d22u32, 0);
    s += src_stride;

    q8u16 = vmovl_u8(vreinterpret_u8_u32(d16u32));
    q9u16 = vmovl_u8(vreinterpret_u8_u32(d18u32));
    q10u16 = vmovl_u8(vreinterpret_u8_u32(d20u32));
    q11u16 = vmovl_u8(vreinterpret_u8_u32(d22u32));

    d18s16 = vreinterpret_s16_u16(vget_low_u16(q9u16));
    d19s16 = vreinterpret_s16_u16(vget_high_u16(q9u16));
    d22s16 = vreinterpret_s16_u16(vget_low_u16(q11u16));
    d = dst;
    for (height = h; height > 0; height -= 4) {  // loop_vert
      d24u32 = vld1_lane_u32((const uint32_t *)s, d24u32, 0);
      s += src_stride;
      d26u32 = vld1_lane_u32((const uint32_t *)s, d26u32, 0);
      s += src_stride;
      d26u32 = vld1_lane_u32((const uint32_t *)s, d26u32, 1);
      s += src_stride;
      d24u32 = vld1_lane_u32((const uint32_t *)s, d24u32, 1);
      s += src_stride;

      q12u16 = vmovl_u8(vreinterpret_u8_u32(d24u32));
      q13u16 = vmovl_u8(vreinterpret_u8_u32(d26u32));

      d16s16 = vreinterpret_s16_u16(vget_low_u16(q8u16));
      d17s16 = vreinterpret_s16_u16(vget_high_u16(q8u16));
      d20s16 = vreinterpret_s16_u16(vget_low_u16(q10u16));
      d21s16 = vreinterpret_s16_u16(vget_high_u16(q10u16));
      d24s16 = vreinterpret_s16_u16(vget_low_u16(q12u16));
      d25s16 = vreinterpret_s16_u16(vget_high_u16(q12u16));
      d26s16 = vreinterpret_s16_u16(vget_low_u16(q13u16));
      d27s16 = vreinterpret_s16_u16(vget_high_u16(q13u16));

      __builtin_prefetch(d);
      __builtin_prefetch(d + dst_stride);
      q1s32 = MULTIPLY_BY_Q0(d16s16, d17s16, d18s16, d19s16, d20s16, d21s16,
                             d22s16, d24s16, q0s16);
      __builtin_prefetch(d + dst_stride * 2);
      __builtin_prefetch(d + dst_stride * 3);
      q2s32 = MULTIPLY_BY_Q0(d17s16, d18s16, d19s16, d20s16, d21s16, d22s16,
                             d24s16, d26s16, q0s16);
      __builtin_prefetch(s);
      __builtin_prefetch(s + src_stride);
      q14s32 = MULTIPLY_BY_Q0(d18s16, d19s16, d20s16, d21s16, d22s16, d24s16,
                              d26s16, d27s16, q0s16);
      __builtin_prefetch(s + src_stride * 2);
      __builtin_prefetch(s + src_stride * 3);
      q15s32 = MULTIPLY_BY_Q0(d19s16, d20s16, d21s16, d22s16, d24s16, d26s16,
                              d27s16, d25s16, q0s16);

      d2u16 = vqrshrun_n_s32(q1s32, 7);
      d3u16 = vqrshrun_n_s32(q2s32, 7);
      d4u16 = vqrshrun_n_s32(q14s32, 7);
      d5u16 = vqrshrun_n_s32(q15s32, 7);

      q1u16 = vcombine_u16(d2u16, d3u16);
      q2u16 = vcombine_u16(d4u16, d5u16);

      d2u32 = vreinterpret_u32_u8(vqmovn_u16(q1u16));
      d3u32 = vreinterpret_u32_u8(vqmovn_u16(q2u16));

      vst1_lane_u32((uint32_t *)d, d2u32, 0);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d2u32, 1);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d3u32, 0);
      d += dst_stride;
      vst1_lane_u32((uint32_t *)d, d3u32, 1);
      d += dst_stride;

      q8u16 = q10u16;
      d18s16 = d22s16;
      d19s16 = d24s16;
      q10u16 = q13u16;
      d22s16 = d25s16;
    }
  }
  return;
}

static INLINE void load_8x4(const uint8_t *s, const int p, uint8x8_t *s0,
                            uint8x8_t *s1, uint8x8_t *s2, uint8x8_t *s3) {
  *s0 = vld1_u8(s);
  s += p;
  *s1 = vld1_u8(s);
  s += p;
  *s2 = vld1_u8(s);
  s += p;
  *s3 = vld1_u8(s);
}

static INLINE void load_8x8(const uint8_t *s, const int p, uint8x8_t *s0,
                            uint8x8_t *s1, uint8x8_t *s2, uint8x8_t *s3,
                            uint8x8_t *s4, uint8x8_t *s5, uint8x8_t *s6,
                            uint8x8_t *s7) {
  *s0 = vld1_u8(s);
  s += p;
  *s1 = vld1_u8(s);
  s += p;
  *s2 = vld1_u8(s);
  s += p;
  *s3 = vld1_u8(s);
  s += p;
  *s4 = vld1_u8(s);
  s += p;
  *s5 = vld1_u8(s);
  s += p;
  *s6 = vld1_u8(s);
  s += p;
  *s7 = vld1_u8(s);
}

static INLINE void store_8x8(uint8_t *s, const int p, const uint8x8_t s0,
                             const uint8x8_t s1, const uint8x8_t s2,
                             const uint8x8_t s3, const uint8x8_t s4,
                             const uint8x8_t s5, const uint8x8_t s6,
                             const uint8x8_t s7) {
  vst1_u8(s, s0);
  s += p;
  vst1_u8(s, s1);
  s += p;
  vst1_u8(s, s2);
  s += p;
  vst1_u8(s, s3);
  s += p;
  vst1_u8(s, s4);
  s += p;
  vst1_u8(s, s5);
  s += p;
  vst1_u8(s, s6);
  s += p;
  vst1_u8(s, s7);
}

static INLINE int16x4_t convolve8_4(int16x4_t s0, int16x4_t s1, int16x4_t s2,
                                    int16x4_t s3, int16x4_t s4, int16x4_t s5,
                                    int16x4_t s6, int16x4_t s7,
                                    int16x8_t filters, int16x4_t filter3,
                                    int16x4_t filter4) {
  const int16x4_t filters_lo = vget_low_s16(filters);
  const int16x4_t filters_hi = vget_high_s16(filters);
  int16x4_t sum = vdup_n_s16(0);

  sum = vmla_lane_s16(sum, s0, filters_lo, 0);
  sum = vmla_lane_s16(sum, s1, filters_lo, 1);
  sum = vmla_lane_s16(sum, s2, filters_lo, 2);
  sum = vmla_lane_s16(sum, s5, filters_hi, 1);
  sum = vmla_lane_s16(sum, s6, filters_hi, 2);
  sum = vmla_lane_s16(sum, s7, filters_hi, 3);
  sum = vqadd_s16(sum, vmul_s16(s3, filter3));
  sum = vqadd_s16(sum, vmul_s16(s4, filter4));
  return sum;
}

static INLINE int16x8_t convolve8_8(int16x8_t s0, int16x8_t s1, int16x8_t s2,
                                    int16x8_t s3, int16x8_t s4, int16x8_t s5,
                                    int16x8_t s6, int16x8_t s7,
                                    int16x8_t filters, int16x8_t filter3,
                                    int16x8_t filter4) {
  const int16x4_t filters_lo = vget_low_s16(filters);
  const int16x4_t filters_hi = vget_high_s16(filters);
  int16x8_t sum = vdupq_n_s16(0);

  sum = vmlaq_lane_s16(sum, s0, filters_lo, 0);
  sum = vmlaq_lane_s16(sum, s1, filters_lo, 1);
  sum = vmlaq_lane_s16(sum, s2, filters_lo, 2);
  sum = vmlaq_lane_s16(sum, s5, filters_hi, 1);
  sum = vmlaq_lane_s16(sum, s6, filters_hi, 2);
  sum = vmlaq_lane_s16(sum, s7, filters_hi, 3);
  sum = vqaddq_s16(sum, vmulq_s16(s3, filter3));
  sum = vqaddq_s16(sum, vmulq_s16(s4, filter4));
  return sum;
}

void vpx_convolve8_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y,  // unused
                              int y_step_q4,            // unused
                              int w, int h) {
  const int16x8_t filters = vld1q_s16(filter_x);
  uint8x8_t t0, t1, t2, t3;

  assert(x_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_y;

  src -= 3;

  if (h == 4) {
    uint8x8_t d01, d23;
    int16x4_t filter3, filter4, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0,
        d1, d2, d3;
    int16x8_t tt0, tt1, tt2, tt3;

    __builtin_prefetch(src + 0 * src_stride);
    __builtin_prefetch(src + 1 * src_stride);
    __builtin_prefetch(src + 2 * src_stride);
    __builtin_prefetch(src + 3 * src_stride);
    filter3 = vdup_lane_s16(vget_low_s16(filters), 3);
    filter4 = vdup_lane_s16(vget_high_s16(filters), 0);
    load_8x4(src, src_stride, &t0, &t1, &t2, &t3);
    transpose_u8_8x4(&t0, &t1, &t2, &t3);
    tt0 = vreinterpretq_s16_u16(vmovl_u8(t0));
    tt1 = vreinterpretq_s16_u16(vmovl_u8(t1));
    tt2 = vreinterpretq_s16_u16(vmovl_u8(t2));
    tt3 = vreinterpretq_s16_u16(vmovl_u8(t3));
    s0 = vget_low_s16(tt0);
    s1 = vget_low_s16(tt1);
    s2 = vget_low_s16(tt2);
    s3 = vget_low_s16(tt3);
    s4 = vget_high_s16(tt0);
    s5 = vget_high_s16(tt1);
    s6 = vget_high_s16(tt2);
    __builtin_prefetch(dst + 0 * dst_stride);
    __builtin_prefetch(dst + 1 * dst_stride);
    __builtin_prefetch(dst + 2 * dst_stride);
    __builtin_prefetch(dst + 3 * dst_stride);
    src += 7;

    do {
      load_8x4(src, src_stride, &t0, &t1, &t2, &t3);
      transpose_u8_8x4(&t0, &t1, &t2, &t3);
      tt0 = vreinterpretq_s16_u16(vmovl_u8(t0));
      tt1 = vreinterpretq_s16_u16(vmovl_u8(t1));
      tt2 = vreinterpretq_s16_u16(vmovl_u8(t2));
      tt3 = vreinterpretq_s16_u16(vmovl_u8(t3));
      s7 = vget_low_s16(tt0);
      s8 = vget_low_s16(tt1);
      s9 = vget_low_s16(tt2);
      s10 = vget_low_s16(tt3);

      d0 = convolve8_4(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                       filter4);
      d1 = convolve8_4(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                       filter4);
      d2 = convolve8_4(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                       filter4);
      d3 = convolve8_4(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                       filter4);

      d01 = vqrshrun_n_s16(vcombine_s16(d0, d1), 7);
      d23 = vqrshrun_n_s16(vcombine_s16(d2, d3), 7);
      transpose_u8_4x4(&d01, &d23);

      vst1_lane_u32((uint32_t *)(dst + 0 * dst_stride),
                    vreinterpret_u32_u8(d01), 0);
      vst1_lane_u32((uint32_t *)(dst + 1 * dst_stride),
                    vreinterpret_u32_u8(d23), 0);
      vst1_lane_u32((uint32_t *)(dst + 2 * dst_stride),
                    vreinterpret_u32_u8(d01), 1);
      vst1_lane_u32((uint32_t *)(dst + 3 * dst_stride),
                    vreinterpret_u32_u8(d23), 1);

      s0 = s4;
      s1 = s5;
      s2 = s6;
      s3 = s7;
      s4 = s8;
      s5 = s9;
      s6 = s10;
      src += 4;
      dst += 4;
      w -= 4;
    } while (w > 0);
  } else {
    const int16x8_t filter3 = vdupq_lane_s16(vget_low_s16(filters), 3);
    const int16x8_t filter4 = vdupq_lane_s16(vget_high_s16(filters), 0);
    int width;
    const uint8_t *s;
    uint8x8_t t4, t5, t6, t7;
    int16x8_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;

    if (w == 4) {
      do {
        load_8x8(src, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s0 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s1 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s2 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s3 = vreinterpretq_s16_u16(vmovl_u8(t3));
        s4 = vreinterpretq_s16_u16(vmovl_u8(t4));
        s5 = vreinterpretq_s16_u16(vmovl_u8(t5));
        s6 = vreinterpretq_s16_u16(vmovl_u8(t6));

        load_8x8(src + 7, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        src += 8 * src_stride;
        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(dst + 4 * dst_stride);
        __builtin_prefetch(dst + 5 * dst_stride);
        __builtin_prefetch(dst + 6 * dst_stride);
        __builtin_prefetch(dst + 7 * dst_stride);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s7 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s8 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s9 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s10 = vreinterpretq_s16_u16(vmovl_u8(t3));

        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        __builtin_prefetch(src + 4 * src_stride);
        __builtin_prefetch(src + 5 * src_stride);
        __builtin_prefetch(src + 6 * src_stride);
        __builtin_prefetch(src + 7 * src_stride);
        d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                         filter4);
        d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                         filter4);
        d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                         filter4);
        d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                         filter4);

        t0 = vqrshrun_n_s16(d0, 7);
        t1 = vqrshrun_n_s16(d1, 7);
        t2 = vqrshrun_n_s16(d2, 7);
        t3 = vqrshrun_n_s16(d3, 7);
        transpose_u8_8x4(&t0, &t1, &t2, &t3);
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t0), 0);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t1), 0);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t2), 0);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t3), 0);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t0), 1);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t1), 1);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t2), 1);
        dst += dst_stride;
        vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(t3), 1);
        dst += dst_stride;
        h -= 8;
      } while (h > 0);
    } else {
      uint8_t *d;
      int16x8_t s11, s12, s13, s14, d4, d5, d6, d7;

      do {
        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        __builtin_prefetch(src + 4 * src_stride);
        __builtin_prefetch(src + 5 * src_stride);
        __builtin_prefetch(src + 6 * src_stride);
        __builtin_prefetch(src + 7 * src_stride);
        load_8x8(src, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s0 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s1 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s2 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s3 = vreinterpretq_s16_u16(vmovl_u8(t3));
        s4 = vreinterpretq_s16_u16(vmovl_u8(t4));
        s5 = vreinterpretq_s16_u16(vmovl_u8(t5));
        s6 = vreinterpretq_s16_u16(vmovl_u8(t6));

        width = w;
        s = src + 7;
        d = dst;
        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(dst + 4 * dst_stride);
        __builtin_prefetch(dst + 5 * dst_stride);
        __builtin_prefetch(dst + 6 * dst_stride);
        __builtin_prefetch(dst + 7 * dst_stride);

        do {
          load_8x8(s, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
          transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
          s7 = vreinterpretq_s16_u16(vmovl_u8(t0));
          s8 = vreinterpretq_s16_u16(vmovl_u8(t1));
          s9 = vreinterpretq_s16_u16(vmovl_u8(t2));
          s10 = vreinterpretq_s16_u16(vmovl_u8(t3));
          s11 = vreinterpretq_s16_u16(vmovl_u8(t4));
          s12 = vreinterpretq_s16_u16(vmovl_u8(t5));
          s13 = vreinterpretq_s16_u16(vmovl_u8(t6));
          s14 = vreinterpretq_s16_u16(vmovl_u8(t7));

          d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                           filter4);
          d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                           filter4);
          d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                           filter4);
          d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                           filter4);
          d4 = convolve8_8(s4, s5, s6, s7, s8, s9, s10, s11, filters, filter3,
                           filter4);
          d5 = convolve8_8(s5, s6, s7, s8, s9, s10, s11, s12, filters, filter3,
                           filter4);
          d6 = convolve8_8(s6, s7, s8, s9, s10, s11, s12, s13, filters, filter3,
                           filter4);
          d7 = convolve8_8(s7, s8, s9, s10, s11, s12, s13, s14, filters,
                           filter3, filter4);

          t0 = vqrshrun_n_s16(d0, 7);
          t1 = vqrshrun_n_s16(d1, 7);
          t2 = vqrshrun_n_s16(d2, 7);
          t3 = vqrshrun_n_s16(d3, 7);
          t4 = vqrshrun_n_s16(d4, 7);
          t5 = vqrshrun_n_s16(d5, 7);
          t6 = vqrshrun_n_s16(d6, 7);
          t7 = vqrshrun_n_s16(d7, 7);
          transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
          store_8x8(d, dst_stride, t0, t1, t2, t3, t4, t5, t6, t7);

          s0 = s8;
          s1 = s9;
          s2 = s10;
          s3 = s11;
          s4 = s12;
          s5 = s13;
          s6 = s14;
          s += 8;
          d += 8;
          width -= 8;
        } while (width > 0);
        src += 8 * src_stride;
        dst += 8 * dst_stride;
        h -= 8;
      } while (h > 0);
    }
  }
}

void vpx_convolve8_avg_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                                  uint8_t *dst, ptrdiff_t dst_stride,
                                  const int16_t *filter_x, int x_step_q4,
                                  const int16_t *filter_y,  // unused
                                  int y_step_q4,            // unused
                                  int w, int h) {
  const int16x8_t filters = vld1q_s16(filter_x);
  uint8x8_t t0, t1, t2, t3;

  assert(x_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_y;

  src -= 3;

  if (h == 4) {
    uint8x8_t d01, d23;
    int16x4_t filter3, filter4, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0,
        d1, d2, d3;
    int16x8_t tt0, tt1, tt2, tt3;
    uint32x4_t dd0;

    __builtin_prefetch(src + 0 * src_stride);
    __builtin_prefetch(src + 1 * src_stride);
    __builtin_prefetch(src + 2 * src_stride);
    __builtin_prefetch(src + 3 * src_stride);
    filter3 = vdup_lane_s16(vget_low_s16(filters), 3);
    filter4 = vdup_lane_s16(vget_high_s16(filters), 0);
    load_8x4(src, src_stride, &t0, &t1, &t2, &t3);
    transpose_u8_8x4(&t0, &t1, &t2, &t3);
    tt0 = vreinterpretq_s16_u16(vmovl_u8(t0));
    tt1 = vreinterpretq_s16_u16(vmovl_u8(t1));
    tt2 = vreinterpretq_s16_u16(vmovl_u8(t2));
    tt3 = vreinterpretq_s16_u16(vmovl_u8(t3));
    s0 = vget_low_s16(tt0);
    s1 = vget_low_s16(tt1);
    s2 = vget_low_s16(tt2);
    s3 = vget_low_s16(tt3);
    s4 = vget_high_s16(tt0);
    s5 = vget_high_s16(tt1);
    s6 = vget_high_s16(tt2);
    __builtin_prefetch(dst + 0 * dst_stride);
    __builtin_prefetch(dst + 1 * dst_stride);
    __builtin_prefetch(dst + 2 * dst_stride);
    __builtin_prefetch(dst + 3 * dst_stride);
    src += 7;

    do {
      load_8x4(src, src_stride, &t0, &t1, &t2, &t3);
      transpose_u8_8x4(&t0, &t1, &t2, &t3);
      tt0 = vreinterpretq_s16_u16(vmovl_u8(t0));
      tt1 = vreinterpretq_s16_u16(vmovl_u8(t1));
      tt2 = vreinterpretq_s16_u16(vmovl_u8(t2));
      tt3 = vreinterpretq_s16_u16(vmovl_u8(t3));
      s7 = vget_low_s16(tt0);
      s8 = vget_low_s16(tt1);
      s9 = vget_low_s16(tt2);
      s10 = vget_low_s16(tt3);

      d0 = convolve8_4(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                       filter4);
      d1 = convolve8_4(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                       filter4);
      d2 = convolve8_4(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                       filter4);
      d3 = convolve8_4(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                       filter4);

      d01 = vqrshrun_n_s16(vcombine_s16(d0, d1), 7);
      d23 = vqrshrun_n_s16(vcombine_s16(d2, d3), 7);
      transpose_u8_4x4(&d01, &d23);

      dd0 = vld1q_lane_u32((uint32_t *)(dst + 0 * dst_stride), dd0, 0);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 1 * dst_stride), dd0, 2);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 2 * dst_stride), dd0, 1);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 3 * dst_stride), dd0, 3);
      dd0 = vreinterpretq_u32_u8(
          vrhaddq_u8(vreinterpretq_u8_u32(dd0), vcombine_u8(d01, d23)));

      vst1q_lane_u32((uint32_t *)(dst + 0 * dst_stride), dd0, 0);
      vst1q_lane_u32((uint32_t *)(dst + 1 * dst_stride), dd0, 2);
      vst1q_lane_u32((uint32_t *)(dst + 2 * dst_stride), dd0, 1);
      vst1q_lane_u32((uint32_t *)(dst + 3 * dst_stride), dd0, 3);

      s0 = s4;
      s1 = s5;
      s2 = s6;
      s3 = s7;
      s4 = s8;
      s5 = s9;
      s6 = s10;
      src += 4;
      dst += 4;
      w -= 4;
    } while (w > 0);
  } else {
    const int16x8_t filter3 = vdupq_lane_s16(vget_low_s16(filters), 3);
    const int16x8_t filter4 = vdupq_lane_s16(vget_high_s16(filters), 0);
    int width;
    const uint8_t *s;
    uint8x8_t t4, t5, t6, t7;
    int16x8_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;

    if (w == 4) {
      uint32x4_t dd0, dd1;
      do {
        load_8x8(src, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s0 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s1 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s2 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s3 = vreinterpretq_s16_u16(vmovl_u8(t3));
        s4 = vreinterpretq_s16_u16(vmovl_u8(t4));
        s5 = vreinterpretq_s16_u16(vmovl_u8(t5));
        s6 = vreinterpretq_s16_u16(vmovl_u8(t6));

        load_8x8(src + 7, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        src += 8 * src_stride;
        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(dst + 4 * dst_stride);
        __builtin_prefetch(dst + 5 * dst_stride);
        __builtin_prefetch(dst + 6 * dst_stride);
        __builtin_prefetch(dst + 7 * dst_stride);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s7 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s8 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s9 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s10 = vreinterpretq_s16_u16(vmovl_u8(t3));

        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        __builtin_prefetch(src + 4 * src_stride);
        __builtin_prefetch(src + 5 * src_stride);
        __builtin_prefetch(src + 6 * src_stride);
        __builtin_prefetch(src + 7 * src_stride);
        d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                         filter4);
        d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                         filter4);
        d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                         filter4);
        d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                         filter4);

        t0 = vqrshrun_n_s16(d0, 7);
        t1 = vqrshrun_n_s16(d1, 7);
        t2 = vqrshrun_n_s16(d2, 7);
        t3 = vqrshrun_n_s16(d3, 7);
        transpose_u8_8x4(&t0, &t1, &t2, &t3);

        dd0 = vld1q_lane_u32((uint32_t *)(dst + 0 * dst_stride), dd0, 0);
        dd0 = vld1q_lane_u32((uint32_t *)(dst + 1 * dst_stride), dd0, 2);
        dd1 = vld1q_lane_u32((uint32_t *)(dst + 2 * dst_stride), dd1, 0);
        dd1 = vld1q_lane_u32((uint32_t *)(dst + 3 * dst_stride), dd1, 2);
        dd0 = vld1q_lane_u32((uint32_t *)(dst + 4 * dst_stride), dd0, 1);
        dd0 = vld1q_lane_u32((uint32_t *)(dst + 5 * dst_stride), dd0, 3);
        dd1 = vld1q_lane_u32((uint32_t *)(dst + 6 * dst_stride), dd1, 1);
        dd1 = vld1q_lane_u32((uint32_t *)(dst + 7 * dst_stride), dd1, 3);
        dd0 = vreinterpretq_u32_u8(
            vrhaddq_u8(vreinterpretq_u8_u32(dd0), vcombine_u8(t0, t1)));
        dd1 = vreinterpretq_u32_u8(
            vrhaddq_u8(vreinterpretq_u8_u32(dd1), vcombine_u8(t2, t3)));

        vst1q_lane_u32((uint32_t *)dst, dd0, 0);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd0, 2);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd1, 0);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd1, 2);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd0, 1);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd0, 3);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd1, 1);
        dst += dst_stride;
        vst1q_lane_u32((uint32_t *)dst, dd1, 3);
        dst += dst_stride;
        h -= 8;
      } while (h > 0);
    } else {
      uint8_t *d;
      int16x8_t s11, s12, s13, s14, d4, d5, d6, d7;
      uint64x2_t dd0, dd1, dd2, dd3;

      do {
        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        __builtin_prefetch(src + 4 * src_stride);
        __builtin_prefetch(src + 5 * src_stride);
        __builtin_prefetch(src + 6 * src_stride);
        __builtin_prefetch(src + 7 * src_stride);
        load_8x8(src, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
        s0 = vreinterpretq_s16_u16(vmovl_u8(t0));
        s1 = vreinterpretq_s16_u16(vmovl_u8(t1));
        s2 = vreinterpretq_s16_u16(vmovl_u8(t2));
        s3 = vreinterpretq_s16_u16(vmovl_u8(t3));
        s4 = vreinterpretq_s16_u16(vmovl_u8(t4));
        s5 = vreinterpretq_s16_u16(vmovl_u8(t5));
        s6 = vreinterpretq_s16_u16(vmovl_u8(t6));

        width = w;
        s = src + 7;
        d = dst;
        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(dst + 4 * dst_stride);
        __builtin_prefetch(dst + 5 * dst_stride);
        __builtin_prefetch(dst + 6 * dst_stride);
        __builtin_prefetch(dst + 7 * dst_stride);

        do {
          load_8x8(s, src_stride, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
          transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
          s7 = vreinterpretq_s16_u16(vmovl_u8(t0));
          s8 = vreinterpretq_s16_u16(vmovl_u8(t1));
          s9 = vreinterpretq_s16_u16(vmovl_u8(t2));
          s10 = vreinterpretq_s16_u16(vmovl_u8(t3));
          s11 = vreinterpretq_s16_u16(vmovl_u8(t4));
          s12 = vreinterpretq_s16_u16(vmovl_u8(t5));
          s13 = vreinterpretq_s16_u16(vmovl_u8(t6));
          s14 = vreinterpretq_s16_u16(vmovl_u8(t7));

          d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                           filter4);
          d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                           filter4);
          d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                           filter4);
          d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                           filter4);
          d4 = convolve8_8(s4, s5, s6, s7, s8, s9, s10, s11, filters, filter3,
                           filter4);
          d5 = convolve8_8(s5, s6, s7, s8, s9, s10, s11, s12, filters, filter3,
                           filter4);
          d6 = convolve8_8(s6, s7, s8, s9, s10, s11, s12, s13, filters, filter3,
                           filter4);
          d7 = convolve8_8(s7, s8, s9, s10, s11, s12, s13, s14, filters,
                           filter3, filter4);

          t0 = vqrshrun_n_s16(d0, 7);
          t1 = vqrshrun_n_s16(d1, 7);
          t2 = vqrshrun_n_s16(d2, 7);
          t3 = vqrshrun_n_s16(d3, 7);
          t4 = vqrshrun_n_s16(d4, 7);
          t5 = vqrshrun_n_s16(d5, 7);
          t6 = vqrshrun_n_s16(d6, 7);
          t7 = vqrshrun_n_s16(d7, 7);
          transpose_u8_8x8(&t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);

          dd0 = vld1q_lane_u64((uint64_t *)(d + 0 * dst_stride), dd0, 0);
          dd0 = vld1q_lane_u64((uint64_t *)(d + 1 * dst_stride), dd0, 1);
          dd1 = vld1q_lane_u64((uint64_t *)(d + 2 * dst_stride), dd1, 0);
          dd1 = vld1q_lane_u64((uint64_t *)(d + 3 * dst_stride), dd1, 1);
          dd2 = vld1q_lane_u64((uint64_t *)(d + 4 * dst_stride), dd2, 0);
          dd2 = vld1q_lane_u64((uint64_t *)(d + 5 * dst_stride), dd2, 1);
          dd3 = vld1q_lane_u64((uint64_t *)(d + 6 * dst_stride), dd3, 0);
          dd3 = vld1q_lane_u64((uint64_t *)(d + 7 * dst_stride), dd3, 1);
          dd0 = vreinterpretq_u64_u8(
              vrhaddq_u8(vreinterpretq_u8_u64(dd0), vcombine_u8(t0, t1)));
          dd1 = vreinterpretq_u64_u8(
              vrhaddq_u8(vreinterpretq_u8_u64(dd1), vcombine_u8(t2, t3)));
          dd2 = vreinterpretq_u64_u8(
              vrhaddq_u8(vreinterpretq_u8_u64(dd2), vcombine_u8(t4, t5)));
          dd3 = vreinterpretq_u64_u8(
              vrhaddq_u8(vreinterpretq_u8_u64(dd3), vcombine_u8(t6, t7)));

          store_8x8(d, dst_stride, vget_low_u8(vreinterpretq_u8_u64(dd0)),
                    vget_high_u8(vreinterpretq_u8_u64(dd0)),
                    vget_low_u8(vreinterpretq_u8_u64(dd1)),
                    vget_high_u8(vreinterpretq_u8_u64(dd1)),
                    vget_low_u8(vreinterpretq_u8_u64(dd2)),
                    vget_high_u8(vreinterpretq_u8_u64(dd2)),
                    vget_low_u8(vreinterpretq_u8_u64(dd3)),
                    vget_high_u8(vreinterpretq_u8_u64(dd3)));

          s0 = s8;
          s1 = s9;
          s2 = s10;
          s3 = s11;
          s4 = s12;
          s5 = s13;
          s6 = s14;
          s += 8;
          d += 8;
          width -= 8;
        } while (width > 0);
        src += 8 * src_stride;
        dst += 8 * dst_stride;
        h -= 8;
      } while (h > 0);
    }
  }
}

void vpx_convolve8_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                             uint8_t *dst, ptrdiff_t dst_stride,
                             const int16_t *filter_x,  // unused
                             int x_step_q4,            // unused
                             const int16_t *filter_y, int y_step_q4, int w,
                             int h) {
  const int16x8_t filters = vld1q_s16(filter_y);

  assert(y_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_x;

  src -= 3 * src_stride;

  if (w == 4) {
    const int16x4_t filter3 = vdup_lane_s16(vget_low_s16(filters), 3);
    const int16x4_t filter4 = vdup_lane_s16(vget_high_s16(filters), 0);
    uint8x8_t d01, d23;
    int16x4_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;

    s0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s2 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s3 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s4 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s5 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s6 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;

    do {
      s7 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s8 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s9 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s10 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;

      __builtin_prefetch(dst + 0 * dst_stride);
      __builtin_prefetch(dst + 1 * dst_stride);
      __builtin_prefetch(dst + 2 * dst_stride);
      __builtin_prefetch(dst + 3 * dst_stride);
      __builtin_prefetch(src + 0 * src_stride);
      __builtin_prefetch(src + 1 * src_stride);
      __builtin_prefetch(src + 2 * src_stride);
      __builtin_prefetch(src + 3 * src_stride);
      d0 = convolve8_4(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                       filter4);
      d1 = convolve8_4(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                       filter4);
      d2 = convolve8_4(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                       filter4);
      d3 = convolve8_4(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                       filter4);

      d01 = vqrshrun_n_s16(vcombine_s16(d0, d1), 7);
      d23 = vqrshrun_n_s16(vcombine_s16(d2, d3), 7);
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(d01), 0);
      dst += dst_stride;
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(d01), 1);
      dst += dst_stride;
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(d23), 0);
      dst += dst_stride;
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(d23), 1);
      dst += dst_stride;

      s0 = s4;
      s1 = s5;
      s2 = s6;
      s3 = s7;
      s4 = s8;
      s5 = s9;
      s6 = s10;
      h -= 4;
    } while (h > 0);
  } else {
    const int16x8_t filter3 = vdupq_lane_s16(vget_low_s16(filters), 3);
    const int16x8_t filter4 = vdupq_lane_s16(vget_high_s16(filters), 0);
    int height;
    const uint8_t *s;
    uint8_t *d;
    int16x8_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;

    do {
      __builtin_prefetch(src + 0 * src_stride);
      __builtin_prefetch(src + 1 * src_stride);
      __builtin_prefetch(src + 2 * src_stride);
      __builtin_prefetch(src + 3 * src_stride);
      __builtin_prefetch(src + 4 * src_stride);
      __builtin_prefetch(src + 5 * src_stride);
      __builtin_prefetch(src + 6 * src_stride);
      s = src;
      s0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s1 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s2 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s3 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s4 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s5 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s6 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      d = dst;
      height = h;

      do {
        s7 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s8 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s9 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;

        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                         filter4);
        d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                         filter4);
        d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                         filter4);
        d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                         filter4);

        vst1_u8(d, vqrshrun_n_s16(d0, 7));
        d += dst_stride;
        vst1_u8(d, vqrshrun_n_s16(d1, 7));
        d += dst_stride;
        vst1_u8(d, vqrshrun_n_s16(d2, 7));
        d += dst_stride;
        vst1_u8(d, vqrshrun_n_s16(d3, 7));
        d += dst_stride;

        s0 = s4;
        s1 = s5;
        s2 = s6;
        s3 = s7;
        s4 = s8;
        s5 = s9;
        s6 = s10;
        height -= 4;
      } while (height > 0);
      src += 8;
      dst += 8;
      w -= 8;
    } while (w > 0);
  }
}

void vpx_convolve8_avg_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                                 uint8_t *dst, ptrdiff_t dst_stride,
                                 const int16_t *filter_x,  // unused
                                 int x_step_q4,            // unused
                                 const int16_t *filter_y, int y_step_q4, int w,
                                 int h) {
  const int16x8_t filters = vld1q_s16(filter_y);

  assert(y_step_q4 == 16);

  (void)x_step_q4;
  (void)y_step_q4;
  (void)filter_x;

  src -= 3 * src_stride;

  if (w == 4) {
    const int16x4_t filter3 = vdup_lane_s16(vget_low_s16(filters), 3);
    const int16x4_t filter4 = vdup_lane_s16(vget_high_s16(filters), 0);
    uint8x8_t d01, d23;
    int16x4_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;
    uint32x4_t dd0;

    s0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s2 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s3 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s4 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s5 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;
    s6 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
    src += src_stride;

    do {
      s7 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s8 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s9 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;
      s10 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vld1_u8(src))));
      src += src_stride;

      __builtin_prefetch(dst + 0 * dst_stride);
      __builtin_prefetch(dst + 1 * dst_stride);
      __builtin_prefetch(dst + 2 * dst_stride);
      __builtin_prefetch(dst + 3 * dst_stride);
      __builtin_prefetch(src + 0 * src_stride);
      __builtin_prefetch(src + 1 * src_stride);
      __builtin_prefetch(src + 2 * src_stride);
      __builtin_prefetch(src + 3 * src_stride);
      d0 = convolve8_4(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                       filter4);
      d1 = convolve8_4(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                       filter4);
      d2 = convolve8_4(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                       filter4);
      d3 = convolve8_4(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                       filter4);

      d01 = vqrshrun_n_s16(vcombine_s16(d0, d1), 7);
      d23 = vqrshrun_n_s16(vcombine_s16(d2, d3), 7);

      dd0 = vld1q_lane_u32((uint32_t *)(dst + 0 * dst_stride), dd0, 0);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 1 * dst_stride), dd0, 1);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 2 * dst_stride), dd0, 2);
      dd0 = vld1q_lane_u32((uint32_t *)(dst + 3 * dst_stride), dd0, 3);
      dd0 = vreinterpretq_u32_u8(
          vrhaddq_u8(vreinterpretq_u8_u32(dd0), vcombine_u8(d01, d23)));

      vst1q_lane_u32((uint32_t *)dst, dd0, 0);
      dst += dst_stride;
      vst1q_lane_u32((uint32_t *)dst, dd0, 1);
      dst += dst_stride;
      vst1q_lane_u32((uint32_t *)dst, dd0, 2);
      dst += dst_stride;
      vst1q_lane_u32((uint32_t *)dst, dd0, 3);
      dst += dst_stride;

      s0 = s4;
      s1 = s5;
      s2 = s6;
      s3 = s7;
      s4 = s8;
      s5 = s9;
      s6 = s10;
      h -= 4;
    } while (h > 0);
  } else {
    const int16x8_t filter3 = vdupq_lane_s16(vget_low_s16(filters), 3);
    const int16x8_t filter4 = vdupq_lane_s16(vget_high_s16(filters), 0);
    int height;
    const uint8_t *s;
    uint8_t *d;
    uint8x16_t d01, d23;
    int16x8_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, d0, d1, d2, d3;
    uint64x2_t dd0, dd1;

    do {
      __builtin_prefetch(src + 0 * src_stride);
      __builtin_prefetch(src + 1 * src_stride);
      __builtin_prefetch(src + 2 * src_stride);
      __builtin_prefetch(src + 3 * src_stride);
      __builtin_prefetch(src + 4 * src_stride);
      __builtin_prefetch(src + 5 * src_stride);
      __builtin_prefetch(src + 6 * src_stride);
      s = src;
      s0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s1 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s2 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s3 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s4 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s5 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      s6 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
      s += src_stride;
      d = dst;
      height = h;

      do {
        s7 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s8 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s9 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;
        s10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(s)));
        s += src_stride;

        __builtin_prefetch(dst + 0 * dst_stride);
        __builtin_prefetch(dst + 1 * dst_stride);
        __builtin_prefetch(dst + 2 * dst_stride);
        __builtin_prefetch(dst + 3 * dst_stride);
        __builtin_prefetch(src + 0 * src_stride);
        __builtin_prefetch(src + 1 * src_stride);
        __builtin_prefetch(src + 2 * src_stride);
        __builtin_prefetch(src + 3 * src_stride);
        d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                         filter4);
        d1 = convolve8_8(s1, s2, s3, s4, s5, s6, s7, s8, filters, filter3,
                         filter4);
        d2 = convolve8_8(s2, s3, s4, s5, s6, s7, s8, s9, filters, filter3,
                         filter4);
        d3 = convolve8_8(s3, s4, s5, s6, s7, s8, s9, s10, filters, filter3,
                         filter4);

        d01 = vcombine_u8(vqrshrun_n_s16(d0, 7), vqrshrun_n_s16(d1, 7));
        d23 = vcombine_u8(vqrshrun_n_s16(d2, 7), vqrshrun_n_s16(d3, 7));
        dd0 = vld1q_lane_u64((uint64_t *)(d + 0 * dst_stride), dd0, 0);
        dd0 = vld1q_lane_u64((uint64_t *)(d + 1 * dst_stride), dd0, 1);
        dd1 = vld1q_lane_u64((uint64_t *)(d + 2 * dst_stride), dd1, 0);
        dd1 = vld1q_lane_u64((uint64_t *)(d + 3 * dst_stride), dd1, 1);
        dd0 = vreinterpretq_u64_u8(vrhaddq_u8(vreinterpretq_u8_u64(dd0), d01));
        dd1 = vreinterpretq_u64_u8(vrhaddq_u8(vreinterpretq_u8_u64(dd1), d23));

        vst1q_lane_u64((uint64_t *)d, dd0, 0);
        d += dst_stride;
        vst1q_lane_u64((uint64_t *)d, dd0, 1);
        d += dst_stride;
        vst1q_lane_u64((uint64_t *)d, dd1, 0);
        d += dst_stride;
        vst1q_lane_u64((uint64_t *)d, dd1, 1);
        d += dst_stride;

        s0 = s4;
        s1 = s5;
        s2 = s6;
        s3 = s7;
        s4 = s8;
        s5 = s9;
        s6 = s10;
        height -= 4;
      } while (height > 0);
      src += 8;
      dst += 8;
      w -= 8;
    } while (w > 0);
  }
}

#if 0
// This optimization is much faster in speed unit test, but slowed down the whole decoder by 5%.
void vpx_convolve8_horiz_neon_slow(const uint8_t *src, ptrdiff_t src_stride,
                                   uint8_t *dst, ptrdiff_t dst_stride,
                                   const int16_t *filter_x, int x_step_q4,
                                   const int16_t *filter_y,  // unused
                                   int y_step_q4,            // unused
                                   int w, int h) {
  int16x8_t filters;

  src -= 3;
  __builtin_prefetch(src);
  __builtin_prefetch(dst);
  filters = vld1q_s16(filter_x);

  if (w == 4) {
    // uint8x8_t t0, t1, t2, t3;
    // int16x8_t tt0, tt1, tt2, tt3;
    int16x4_t filter3, filter4, s0, s1, s2, s3, s4, s5, s6, s7, d0, d1;
    uint32x2_t d01;

    filter3 = vdup_lane_s16(vget_low_s16(filters), 3);
    filter4 = vdup_lane_s16(vget_high_s16(filters), 0);

    do {
      const uint8x16_t ss = vld1q_u8(src);
      const int16x8_t t0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(ss)));
      const int16x8_t t1 = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(ss)));
      src += src_stride;
      __builtin_prefetch(src);
      s0 = vget_low_s16(t0);
      s1 = vext_s16(vget_low_s16(t0), vget_high_s16(t0), 1);
      s2 = vext_s16(vget_low_s16(t0), vget_high_s16(t0), 2);
      s3 = vext_s16(vget_low_s16(t0), vget_high_s16(t0), 3);
      s4 = vget_high_s16(t0);
      s5 = vext_s16(vget_high_s16(t0), vget_low_s16(t1), 1);
      s6 = vext_s16(vget_high_s16(t0), vget_low_s16(t1), 2);
      s7 = vext_s16(vget_high_s16(t0), vget_low_s16(t1), 3);
      // t0 = vld1_u8(src + 0);
      // t1 = vld1_u8(src + 1);
      // t2 = vld1_u8(src + 2);
      // t3 = vld1_u8(src + 3);
      // tt0 = vreinterpretq_s16_u16(vmovl_u8(t0));
      // tt1 = vreinterpretq_s16_u16(vmovl_u8(t1));
      // tt2 = vreinterpretq_s16_u16(vmovl_u8(t2));
      // tt3 = vreinterpretq_s16_u16(vmovl_u8(t3));
      // s0 = vget_low_s16(tt0);
      // s1 = vget_low_s16(tt1);
      // s2 = vget_low_s16(tt2);
      // s3 = vget_low_s16(tt3);
      // s4 = vget_high_s16(tt0);
      // s5 = vget_high_s16(tt1);
      // s6 = vget_high_s16(tt2);
      // s7 = vget_high_s16(tt3);
      d0 = convolve8_4(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                       filter4);
      d01 = vreinterpret_u32_u8(vqrshrun_n_s16(vcombine_s16(d0, d1), 7));
      vst1_lane_u32((uint32_t *)dst, d01, 0);
      dst += dst_stride;
      __builtin_prefetch(dst);
    } while (--h);
  } else {
    const uint8_t /**s,*/ *psrc;
    uint8_t /**d,*/ *pdst;
    int16x8_t filter3, filter4, s0, s1, s2, s3, s4, s5, s6, s7, d0;

    filter3 = vdupq_lane_s16(vget_low_s16(filters), 3);
    filter4 = vdupq_lane_s16(vget_high_s16(filters), 0);
    do {
      int width = w;
      psrc = src;
      pdst = dst;
      do {
        const uint8x16_t ss = vld1q_u8(psrc);
        const int16x8_t t = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(ss)));
        s0 = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(ss)));
        s1 = vextq_s16(s0, t, 1);
        s2 = vextq_s16(s0, t, 2);
        s3 = vextq_s16(s0, t, 3);
        s4 = vextq_s16(s0, t, 4);
        s5 = vextq_s16(s0, t, 5);
        s6 = vextq_s16(s0, t, 6);
        s7 = vextq_s16(s0, t, 7);
        d0 = convolve8_8(s0, s1, s2, s3, s4, s5, s6, s7, filters, filter3,
                         filter4);
        vst1_u8(pdst, vqrshrun_n_s16(d0, 7));
        width -= 8;
        psrc += 8;
        pdst += 8;
      } while (width > 0);
      src += src_stride;
      __builtin_prefetch(src);
      dst += dst_stride;
      __builtin_prefetch(dst);
    } while (--h);
  }
}
#endif
