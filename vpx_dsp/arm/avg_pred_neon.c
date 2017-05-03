/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
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

// Prevent clang from adding alignment hints to load instructions:
// https://bugs.llvm.org//show_bug.cgi?id=24421
typedef uint32_t __attribute__((aligned(1))) uint32_unaligned;

// TODO(johannkoenig): move these to a common header.
static INLINE uint8x16_t load_32x4_from_u8(const uint8_t *a, int a_stride) {
  uint32x4_t a_u32 = vdupq_n_u32(0);
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 0);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 1);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 2);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 3);
  return vreinterpretq_u8_u32(a_u32);
}

static INLINE uint8x16_t load_64x2_from_u8(const uint8_t *a, int a_stride) {
  uint32x4_t a_u32 = vdupq_n_u32(0);
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 0);
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)(a + 4), a_u32, 1);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 2);
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)(a + 4), a_u32, 3);
  return vreinterpretq_u8_u32(a_u32);
}

void vpx_comp_avg_pred_neon(uint8_t *comp, const uint8_t *pred, int width,
                            int height, const uint8_t *ref, int ref_stride) {
  if (width > 8) {
    int x, y;
    for (y = 0; y < height; ++y) {
      for (x = 0; x < width; x += 16) {
        const uint8x16_t p = vld1q_u8(pred + x);
        const uint8x16_t r = vld1q_u8(ref + x);
        const uint8x16_t avg = vrhaddq_u8(p, r);
        vst1q_u8(comp + x, avg);
      }
      comp += width;
      pred += width;
      ref += ref_stride;
    }
  } else {
    int i;
    for (i = 0; i < width * height; i += 16) {
      const uint8x16_t p = vld1q_u8(pred);
      uint8x16_t r;

      if (width == ref_stride) {
        r = vld1q_u8(ref);
        ref += 16;
      } else if (width == 4) {
        r = load_32x4_from_u8(ref, ref_stride);
        ref += 4 * ref_stride;
      } else {
        assert(width == 8);
        r = load_64x2_from_u8(ref, ref_stride);
        ref += 2 * ref_stride;
      }
      r = vrhaddq_u8(r, p);
      vst1q_u8(comp, r);

      pred += 16;
      comp += 16;
    }
  }
}
