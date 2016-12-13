/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdlib.h>
#include <arm_neon.h>
#include "vpx/vpx_integer.h"

#include "vpx_dsp/arm/transpose_neon.h"

static const int16_t vpx_rv_neon[] = {
  8,  5,  2,  2,  8,  12, 4,  9,  8,  3,  0,  3,  9,  0,  0,  0,  8,  3,  14,
  4,  10, 1,  11, 14, 1,  14, 9,  6,  12, 11, 8,  6,  10, 0,  0,  8,  9,  0,
  3,  14, 8,  11, 13, 4,  2,  9,  0,  3,  9,  6,  1,  2,  3,  14, 13, 1,  8,
  2,  9,  7,  3,  3,  1,  13, 13, 6,  6,  5,  2,  7,  11, 9,  11, 8,  7,  3,
  2,  0,  13, 13, 14, 4,  12, 5,  12, 10, 8,  10, 13, 10, 4,  14, 4,  10, 0,
  8,  11, 1,  13, 7,  7,  14, 6,  14, 13, 2,  13, 5,  4,  4,  0,  10, 0,  5,
  13, 2,  12, 7,  11, 13, 8,  0,  4,  10, 7,  2,  7,  2,  2,  5,  3,  4,  7,
  3,  3,  14, 14, 5,  9,  13, 3,  14, 3,  6,  3,  0,  11, 8,  13, 1,  13, 1,
  12, 0,  10, 9,  7,  6,  2,  8,  5,  2,  13, 7,  1,  13, 14, 7,  6,  7,  9,
  6,  10, 11, 7,  8,  7,  5,  14, 8,  4,  4,  0,  8,  7,  10, 0,  8,  14, 11,
  3,  12, 5,  7,  14, 3,  14, 5,  2,  6,  11, 12, 12, 8,  0,  11, 13, 1,  2,
  0,  5,  10, 14, 7,  8,  0,  4,  11, 0,  8,  0,  3,  10, 5,  8,  0,  11, 6,
  7,  8,  10, 7,  13, 9,  2,  5,  1,  5,  10, 2,  4,  3,  5,  6,  10, 8,  9,
  4,  11, 14, 0,  10, 0,  5,  13, 2,  12, 7,  11, 13, 8,  0,  4,  10, 7,  2,
  7,  2,  2,  5,  3,  4,  7,  3,  3,  14, 14, 5,  9,  13, 3,  14, 3,  6,  3,
  0,  11, 8,  13, 1,  13, 1,  12, 0,  10, 9,  7,  6,  2,  8,  5,  2,  13, 7,
  1,  13, 14, 7,  6,  7,  9,  6,  10, 11, 7,  8,  7,  5,  14, 8,  4,  4,  0,
  8,  7,  10, 0,  8,  14, 11, 3,  12, 5,  7,  14, 3,  14, 5,  2,  6,  11, 12,
  12, 8,  0,  11, 13, 1,  2,  0,  5,  10, 14, 7,  8,  0,  4,  11, 0,  8,  0,
  3,  10, 5,  8,  0,  11, 6,  7,  8,  10, 7,  13, 9,  2,  5,  1,  5,  10, 2,
  4,  3,  5,  6,  10, 8,  9,  4,  11, 14, 3,  8,  3,  7,  8,  5,  11, 4,  12,
  3,  11, 9,  14, 8,  14, 13, 4,  3,  1,  2,  14, 6,  5,  4,  4,  11, 4,  6,
  2,  1,  5,  8,  8,  12, 13, 5,  14, 10, 12, 13, 0,  9,  5,  5,  11, 10, 13,
  9,  10, 13,
};

static uint8x8_t average_k_out(uint8x8_t a2, uint8x8_t a1, uint8x8_t v0,
                               uint8x8_t b1, uint8x8_t b2) {
  const uint8x8_t k1 = vrhadd_u8(a2, a1);
  const uint8x8_t k2 = vrhadd_u8(b2, b1);
  const uint8x8_t k3 = vrhadd_u8(k1, k2);
  return vrhadd_u8(k3, v0);
}

static uint8x8_t generate_mask(uint8x8_t a2, uint8x8_t a1, uint8x8_t v0,
                               uint8x8_t b1, uint8x8_t b2,
                               const uint8x8_t filter) {
  const uint8x8_t a2_v0 = vabd_u8(a2, v0);
  const uint8x8_t a1_v0 = vabd_u8(a1, v0);
  const uint8x8_t b1_v0 = vabd_u8(b1, v0);
  const uint8x8_t b2_v0 = vabd_u8(b2, v0);

  uint8x8_t mask = vand_u8(vclt_u8(a2_v0, filter), vclt_u8(a1_v0, filter));
  mask = vand_u8(mask, vclt_u8(b1_v0, filter));
  return vand_u8(mask, vclt_u8(b2_v0, filter));
}

static uint8x8_t generate_output(uint8x8_t a2, uint8x8_t a1, uint8x8_t v0,
                                 uint8x8_t b1, uint8x8_t b2,
                                 const uint8x8_t filter) {
  uint8x8_t k_out = average_k_out(a2, a1, v0, b1, b2);
  uint8x8_t mask = generate_mask(a2, a1, v0, b1, b2, filter);
  uint8x8_t v_out;

  k_out = vand_u8(k_out, mask);
  v_out = vand_u8(v0, vmvn_u8(mask));
  return vorr_u8(v_out, k_out);
}

// Same functions but for uint8x16_t.
static uint8x16_t average_k_outq(uint8x16_t a2, uint8x16_t a1, uint8x16_t v0,
                                 uint8x16_t b1, uint8x16_t b2) {
  const uint8x16_t k1 = vrhaddq_u8(a2, a1);
  const uint8x16_t k2 = vrhaddq_u8(b2, b1);
  const uint8x16_t k3 = vrhaddq_u8(k1, k2);
  return vrhaddq_u8(k3, v0);
}

static uint8x16_t generate_maskq(uint8x16_t a2, uint8x16_t a1, uint8x16_t v0,
                                 uint8x16_t b1, uint8x16_t b2,
                                 const uint8x16_t filter) {
  const uint8x16_t a2_v0 = vabdq_u8(a2, v0);
  const uint8x16_t a1_v0 = vabdq_u8(a1, v0);
  const uint8x16_t b1_v0 = vabdq_u8(b1, v0);
  const uint8x16_t b2_v0 = vabdq_u8(b2, v0);

  uint8x16_t mask = vandq_u8(vcltq_u8(a2_v0, filter), vcltq_u8(a1_v0, filter));
  mask = vandq_u8(mask, vcltq_u8(b1_v0, filter));
  return vandq_u8(mask, vcltq_u8(b2_v0, filter));
}

static uint8x16_t generate_outputq(uint8x16_t a2, uint8x16_t a1, uint8x16_t v0,
                                   uint8x16_t b1, uint8x16_t b2,
                                   const uint8x16_t filter) {
  uint8x16_t k_out = average_k_outq(a2, a1, v0, b1, b2);
  uint8x16_t mask = generate_maskq(a2, a1, v0, b1, b2, filter);
  uint8x16_t v_out;

  k_out = vandq_u8(k_out, mask);
  v_out = vandq_u8(v0, vmvnq_u8(mask));
  return vorrq_u8(v_out, k_out);
}

void vpx_post_proc_down_and_across_mb_row_neon(uint8_t *src_ptr,
                                               uint8_t *dst_ptr, int src_stride,
                                               int dst_stride, int cols,
                                               uint8_t *f, int size) {
  uint8_t *p_src, *p_dst;
  int row;
  int col;

  // Process a stripe of macroblocks. The stripe will be a multiple of 16 (for
  // Y) or 8 (for U/V) wide (cols) and the height (size) will be 16 (for Y) or 8
  // (for U/V).
  assert((size == 8 && cols % 8 == 0) || (size == 16 && cols % 16 == 0));

  // While columns of length 16 can be processed, load them.
  for (col = 0; col < cols - 8; col += 16) {
    uint8x16_t a0, a1, a2, a3, a4, a5, a6, a7;
    p_src = src_ptr - 2 * src_stride;
    p_dst = dst_ptr;

    // The next phase extends the borders to avoid using the random buffer
    // values. Are the input frames already extended or should we be considering
    // that here? In the tests the border is not extended but rather random
    // values. In production code I believe it is extended, somewhat mitigating
    // this issue.
    a0 = vld1q_u8(p_src);
    p_src += src_stride;
    a1 = vld1q_u8(p_src);
    p_src += src_stride;
    a2 = vld1q_u8(p_src);
    p_src += src_stride;
    a3 = vld1q_u8(p_src);
    p_src += src_stride;

    for (row = 0; row < size; row += 4) {
      uint8x16_t v_out_0, v_out_1, v_out_2, v_out_3;
      const uint8x16_t filterq = vld1q_u8(f + col);

      a4 = vld1q_u8(p_src);
      p_src += src_stride;
      a5 = vld1q_u8(p_src);
      p_src += src_stride;
      a6 = vld1q_u8(p_src);
      p_src += src_stride;
      a7 = vld1q_u8(p_src);
      p_src += src_stride;

      v_out_0 = generate_outputq(a0, a1, a2, a3, a4, filterq);
      v_out_1 = generate_outputq(a1, a2, a3, a4, a5, filterq);
      v_out_2 = generate_outputq(a2, a3, a4, a5, a6, filterq);
      v_out_3 = generate_outputq(a3, a4, a5, a6, a7, filterq);

      vst1q_u8(p_dst, v_out_0);
      p_dst += dst_stride;
      vst1q_u8(p_dst, v_out_1);
      p_dst += dst_stride;
      vst1q_u8(p_dst, v_out_2);
      p_dst += dst_stride;
      vst1q_u8(p_dst, v_out_3);
      p_dst += dst_stride;

      // Rotate over to the next slot.
      a0 = a4;
      a1 = a5;
      a2 = a6;
      a3 = a7;
    }

    src_ptr += 16;
    dst_ptr += 16;
  }

  // Clean up any left over column of length 8.
  if (col != cols) {
    uint8x8_t a0, a1, a2, a3, a4, a5, a6, a7;
    p_src = src_ptr - 2 * src_stride;
    p_dst = dst_ptr;

    a0 = vld1_u8(p_src);
    p_src += src_stride;
    a1 = vld1_u8(p_src);
    p_src += src_stride;
    a2 = vld1_u8(p_src);
    p_src += src_stride;
    a3 = vld1_u8(p_src);
    p_src += src_stride;

    for (row = 0; row < size; row += 4) {
      uint8x8_t v_out_0, v_out_1, v_out_2, v_out_3;
      const uint8x8_t filter = vld1_u8(f + col);

      a4 = vld1_u8(p_src);
      p_src += src_stride;
      a5 = vld1_u8(p_src);
      p_src += src_stride;
      a6 = vld1_u8(p_src);
      p_src += src_stride;
      a7 = vld1_u8(p_src);
      p_src += src_stride;

      v_out_0 = generate_output(a0, a1, a2, a3, a4, filter);
      v_out_1 = generate_output(a1, a2, a3, a4, a5, filter);
      v_out_2 = generate_output(a2, a3, a4, a5, a6, filter);
      v_out_3 = generate_output(a3, a4, a5, a6, a7, filter);

      vst1_u8(p_dst, v_out_0);
      p_dst += dst_stride;
      vst1_u8(p_dst, v_out_1);
      p_dst += dst_stride;
      vst1_u8(p_dst, v_out_2);
      p_dst += dst_stride;
      vst1_u8(p_dst, v_out_3);
      p_dst += dst_stride;

      // Rotate over to the next slot.
      a0 = a4;
      a1 = a5;
      a2 = a6;
      a3 = a7;
    }

    // Not strictly necessary but makes resetting dst_ptr easier.
    dst_ptr += 8;
  }

  dst_ptr -= cols;

  for (row = 0; row < size; row += 8) {
    uint8x8_t a0, a1, a2, a3;
    uint8x8_t b0, b1, b2, b3, b4, b5, b6, b7;

    p_src = dst_ptr;
    p_dst = dst_ptr;

    // Load 8 values, transpose 4 of them, and discard 2 because they will be
    // reloaded later.
    load_and_transpose_u8_4x8(p_src, dst_stride, &a0, &a1, &a2, &a3);
    a3 = a1;
    a2 = a1 = a0;  // Extend left border.

    p_src += 2;

    for (col = 0; col < cols; col += 8) {
      uint8x8_t v_out_0, v_out_1, v_out_2, v_out_3, v_out_4, v_out_5, v_out_6,
          v_out_7;
      // Although the filter is meant to be applied vertically and is instead
      // being applied horizontally here it's OK because it's set in blocks of 8
      // (or 16).
      const uint8x8_t filter = vld1_u8(f + col);

      load_and_transpose_u8_8x8(p_src, dst_stride, &b0, &b1, &b2, &b3, &b4, &b5,
                                &b6, &b7);

      if (col + 8 == cols) {
        // Last row. Extend border (b5).
        b6 = b7 = b5;
      }

      v_out_0 = generate_output(a0, a1, a2, a3, b0, filter);
      v_out_1 = generate_output(a1, a2, a3, b0, b1, filter);
      v_out_2 = generate_output(a2, a3, b0, b1, b2, filter);
      v_out_3 = generate_output(a3, b0, b1, b2, b3, filter);
      v_out_4 = generate_output(b0, b1, b2, b3, b4, filter);
      v_out_5 = generate_output(b1, b2, b3, b4, b5, filter);
      v_out_6 = generate_output(b2, b3, b4, b5, b6, filter);
      v_out_7 = generate_output(b3, b4, b5, b6, b7, filter);

      transpose_and_store_u8_8x8(p_dst, dst_stride, v_out_0, v_out_1, v_out_2,
                                 v_out_3, v_out_4, v_out_5, v_out_6, v_out_7);

      a0 = b4;
      a1 = b5;
      a2 = b6;
      a3 = b7;

      p_src += 8;
      p_dst += 8;
    }

    dst_ptr += 8 * dst_stride;
  }
}
