/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <tmmintrin.h>  // SSSE3 for _mm_alignr_epi8()

#include "./vpx_config.h"
#include "./vp9_rtcd.h"

// Load 16 values each from 'a' and 'b'. Compute the difference squared and sum
// neighboring pixels such that:
// sum[1] = (a[0]-b[0])^2 + (a[1]-b[1])^2 + (a[2]-b[2])^2
// The values are returned in sum_0 and sum_1 as *unsigned* 16 bit values.
static INLINE void sum_16(const uint8_t *a, const uint8_t *b, __m128i *sum_0,
                          __m128i *sum_1) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_u8 = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_0_s16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i a_1_s16 = _mm_unpackhi_epi8(a_u8, zero);
  const __m128i b_0_s16 = _mm_unpacklo_epi8(b_u8, zero);
  const __m128i b_1_s16 = _mm_unpackhi_epi8(b_u8, zero);

  const __m128i diff_0_s16 = _mm_sub_epi16(a_0_s16, b_0_s16);
  const __m128i diff_1_s16 = _mm_sub_epi16(a_1_s16, b_1_s16);

  const __m128i diff_sq_0_u16 = _mm_mullo_epi16(diff_0_s16, diff_0_s16);
  const __m128i diff_sq_1_u16 = _mm_mullo_epi16(diff_1_s16, diff_1_s16);

  __m128i shift_right = _mm_slli_si128(diff_sq_0_u16, 2);
  __m128i shift_left = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 2);

  // It becomes necessary to treat the values as unsigned at this point. The
  // 255^2 fits in uint16_t but not int16_t. Use saturating adds from this point
  // foward since the filter is only applied to smooth small pixel changes. Once
  // the value has saturated to uint16_t it is well outside the useful range.
  __m128i sum_u16 = _mm_adds_epu16(shift_left, diff_sq_0_u16);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_0 = sum_u16;

  shift_right = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 14);
  shift_left = _mm_srli_si128(diff_sq_1_u16, 2);

  sum_u16 = _mm_adds_epu16(shift_left, diff_sq_1_u16);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_1 = sum_u16;
}

// Same but for 8 input values.
static INLINE void sum_8(const uint8_t *a, const uint8_t *b, __m128i *sum_0) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_u8 = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_0_16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i b_0_16 = _mm_unpacklo_epi8(b_u8, zero);

  const __m128i diff = _mm_sub_epi16(a_0_16, b_0_16);

  const __m128i diff_sq_s16 = _mm_mullo_epi16(diff, diff);

  __m128i shift_right = _mm_slli_si128(diff_sq_s16, 2);
  __m128i shift_left = _mm_srli_si128(diff_sq_s16, 2);

  __m128i sum_s16 = _mm_adds_epu16(shift_left, diff_sq_s16);
  sum_s16 = _mm_adds_epu16(sum_s16, shift_right);

  *sum_0 = sum_s16;
}

void vp9_temporal_filter_apply_ssse3(const uint8_t *a, unsigned int stride,
                                     const uint8_t *b, unsigned int block_width,
                                     unsigned int block_height, int strength,
                                     int filter_weight,
                                     unsigned int *accumulator,
                                     uint16_t *count) {
  unsigned int i, j;
  uint16_t sum_row[16];
  const int rounding = strength > 0 ? 1 << (strength - 1) : 0;
  // One empty block between each row for shifting and adding.
  const uint8_t *a_, *b_;

  const __m128i zero = _mm_setzero_si128();
  __m128i sum_row_a_0, sum_row_a_1;
  __m128i sum_row_b_0, sum_row_b_1;
  __m128i sum_row_c_0, sum_row_c_1;

  sum_row_a_0 = zero;
  sum_row_a_1 = zero;

  if (block_width == 16) {
    sum_16(a, b, &sum_row_b_0, &sum_row_b_1);
  } else {
    sum_8(a, b, &sum_row_b_0);
  }

  a_ = a + stride;
  b_ = b + block_width;

  for (j = 0; j < block_height; ++j) {
    if (j != block_height - 1) {
      if (block_width == 16) {
        sum_16(a_, b_, &sum_row_c_0, &sum_row_c_1);
      } else {
        sum_8(a_, b_, &sum_row_c_0);
      }

      a_ += stride;
      b_ += block_width;
    } else {
      sum_row_c_0 = zero;
      sum_row_c_1 = zero;
    }

    sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_b_0);
    sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_c_0);
    if (block_width == 16) {
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_b_1);
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_c_1);
    }

    _mm_storeu_si128((__m128i *)sum_row, sum_row_a_0);
    if (block_width == 16) {
      _mm_storeu_si128((__m128i *)(sum_row + 8), sum_row_a_1);
    }

    for (i = 0; i < block_width; ++i) {
      int num_values = 9;
      int diff_sq;

      if (j == 0 || j == block_height - 1) num_values = 6;

      if ((i == 0) || (i == block_width - 1)) {
        if (num_values == 6)
          num_values = 4;
        else
          num_values = 6;
      }

      // Instead of multiplying by 3 then dividing by the number of blocks being
      // summed, attempt to do this with integer math. This appears to be the
      // most likely path for getting this into SIMD.
      if (num_values == 4) {
        diff_sq = (sum_row[i] * 3) >> 2;
      } else if (num_values == 6) {
        diff_sq = (sum_row[i] * 2) >> 2;
      } else if (num_values == 9) {
        diff_sq = ((uint32_t)sum_row[i] * 11) >> 5;
      }

      diff_sq += rounding;
      diff_sq >>= strength;
      if (diff_sq > 16) diff_sq = 16;
      diff_sq = 16 - diff_sq;
      diff_sq *= filter_weight;
      count[j * block_width + i] += diff_sq;
      accumulator[j * block_width + i] += diff_sq * b[j * block_width + i];
    }

    sum_row_a_0 = sum_row_b_0;
    sum_row_a_1 = sum_row_b_1;
    sum_row_b_0 = sum_row_c_0;
    sum_row_b_1 = sum_row_c_1;
  }
}
