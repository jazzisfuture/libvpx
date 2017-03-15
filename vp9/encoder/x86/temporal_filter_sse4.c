/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string.h>     // memset
#include <smmintrin.h>  // SSE4.1
#include <stdio.h>      // printf

#include "./vpx_config.h"
#include "./vp9_rtcd.h"

static INLINE void dump_reg(char *name, __m128i a) {
  printf("%6s %5d %5d %5d %5d\n", name, _mm_extract_epi32(a, 0),
         _mm_extract_epi32(a, 1), _mm_extract_epi32(a, 2),
         _mm_extract_epi32(a, 3));
}

static INLINE void sum_16(const uint8_t *a, const uint8_t *b, __m128i *sum_0,
                          __m128i *sum_1, __m128i *sum_2, __m128i *sum_3) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_u8 = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_0_16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i a_1_16 = _mm_unpackhi_epi8(a_u8, zero);
  const __m128i b_0_16 = _mm_unpacklo_epi8(b_u8, zero);
  const __m128i b_1_16 = _mm_unpackhi_epi8(b_u8, zero);

  const __m128i a_0_32 = _mm_unpacklo_epi16(a_0_16, zero);
  const __m128i a_1_32 = _mm_unpackhi_epi16(a_0_16, zero);
  const __m128i a_2_32 = _mm_unpacklo_epi16(a_1_16, zero);
  const __m128i a_3_32 = _mm_unpackhi_epi16(a_1_16, zero);

  const __m128i b_0_32 = _mm_unpacklo_epi16(b_0_16, zero);
  const __m128i b_1_32 = _mm_unpackhi_epi16(b_0_16, zero);
  const __m128i b_2_32 = _mm_unpacklo_epi16(b_1_16, zero);
  const __m128i b_3_32 = _mm_unpackhi_epi16(b_1_16, zero);

  const __m128i diff_0 = _mm_sub_epi32(a_0_32, b_0_32);
  const __m128i diff_1 = _mm_sub_epi32(a_1_32, b_1_32);
  const __m128i diff_2 = _mm_sub_epi32(a_2_32, b_2_32);
  const __m128i diff_3 = _mm_sub_epi32(a_3_32, b_3_32);

  const __m128i diff_sq_0 = _mm_mullo_epi32(diff_0, diff_0);
  const __m128i diff_sq_1 = _mm_mullo_epi32(diff_1, diff_1);
  const __m128i diff_sq_2 = _mm_mullo_epi32(diff_2, diff_2);
  const __m128i diff_sq_3 = _mm_mullo_epi32(diff_3, diff_3);

  __m128i shift_right = _mm_slli_si128(diff_sq_0, 4);
  __m128i shift_left = _mm_alignr_epi8(diff_sq_1, diff_sq_0, 4);

  *sum_0 = _mm_add_epi32(shift_left, diff_sq_0);
  *sum_0 = _mm_add_epi32(*sum_0, shift_right);

  shift_right = _mm_alignr_epi8(diff_sq_1, diff_sq_0, 12);
  shift_left = _mm_alignr_epi8(diff_sq_2, diff_sq_1, 4);

  *sum_1 = _mm_add_epi32(shift_left, diff_sq_1);
  *sum_1 = _mm_add_epi32(*sum_1, shift_right);

  shift_right = _mm_alignr_epi8(diff_sq_2, diff_sq_1, 12);
  shift_left = _mm_alignr_epi8(diff_sq_3, diff_sq_2, 4);

  *sum_2 = _mm_add_epi32(shift_left, diff_sq_2);
  *sum_2 = _mm_add_epi32(*sum_2, shift_right);

  shift_right = _mm_alignr_epi8(diff_sq_3, diff_sq_2, 12);
  shift_left = _mm_srli_si128(diff_sq_3, 4);

  *sum_3 = _mm_add_epi32(shift_left, diff_sq_3);
  *sum_3 = _mm_add_epi32(*sum_3, shift_right);
}

static INLINE void sum_8(const uint8_t *a, const uint8_t *b, __m128i *sum_0,
                         __m128i *sum_1) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_u8 = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_0_16 = _mm_unpacklo_epi8(a_u8, zero);
  const __m128i b_0_16 = _mm_unpacklo_epi8(b_u8, zero);

  const __m128i a_0_32 = _mm_unpacklo_epi16(a_0_16, zero);
  const __m128i a_1_32 = _mm_unpackhi_epi16(a_0_16, zero);

  const __m128i b_0_32 = _mm_unpacklo_epi16(b_0_16, zero);
  const __m128i b_1_32 = _mm_unpackhi_epi16(b_0_16, zero);

  const __m128i diff_0 = _mm_sub_epi32(a_0_32, b_0_32);
  const __m128i diff_1 = _mm_sub_epi32(a_1_32, b_1_32);

  const __m128i diff_sq_0 = _mm_mullo_epi32(diff_0, diff_0);
  const __m128i diff_sq_1 = _mm_mullo_epi32(diff_1, diff_1);

  __m128i shift_right = _mm_slli_si128(diff_sq_0, 4);
  __m128i shift_left = _mm_alignr_epi8(diff_sq_1, diff_sq_0, 4);

  *sum_0 = _mm_add_epi32(shift_left, diff_sq_0);
  *sum_0 = _mm_add_epi32(*sum_0, shift_right);

  shift_right = _mm_alignr_epi8(diff_sq_1, diff_sq_0, 12);
  shift_left = _mm_srli_si128(diff_sq_1, 4);

  *sum_1 = _mm_add_epi32(shift_left, diff_sq_1);
  *sum_1 = _mm_add_epi32(*sum_1, shift_right);
}
// block_width and block_height are for macroblocks. When applied to the Y
// buffer they will always be 16. For U and V, they will be 8 or 16 depending on
// downsampling.
void vp9_temporal_filter_apply_sse4_1(
    const uint8_t *a, unsigned int stride, const uint8_t *b,
    unsigned int block_width, unsigned int block_height, int strength,
    int filter_weight, unsigned int *accumulator, uint16_t *count) {
  unsigned int i, j;
  int sum_row[16];
  const int rounding = strength > 0 ? 1 << (strength - 1) : 0;
  // One empty block between each row for shifting and adding.
  const uint8_t *a_, *b_;

  const __m128i zero = _mm_setzero_si128();
  const __m128i rounding_32 = _mm_set1_epi32(rounding);
  const __m128i strength_32 = _mm_set1_epi32(strength);
  const __m128i weight_32 = _mm_set1_epi32(filter_weight);
  const __m128i three = _mm_set1_epi32(3);
  __m128i sum_row_a_0, sum_row_a_1, sum_row_a_2, sum_row_a_3;
  __m128i sum_row_b_0, sum_row_b_1, sum_row_b_2, sum_row_b_3;
  __m128i sum_row_c_0, sum_row_c_1, sum_row_c_2, sum_row_c_3;

  sum_row_a_0 = zero;
  sum_row_a_1 = zero;
  sum_row_a_2 = zero;
  sum_row_a_3 = zero;

  if (block_width == 16) {
    sum_16(a, b, &sum_row_b_0, &sum_row_b_1, &sum_row_b_2, &sum_row_b_3);
  } else {
    sum_8(a, b, &sum_row_b_0, &sum_row_b_1);
  }

  a_ = a + stride;
  b_ = b + block_width;

  for (j = 0; j < block_height; ++j) {
    if (j != block_height - 1) {
      if (block_width == 16) {
        sum_16(a_, b_, &sum_row_c_0, &sum_row_c_1, &sum_row_c_2, &sum_row_c_3);
      } else {
        sum_8(a_, b_, &sum_row_c_0, &sum_row_c_1);
      }

      a_ += stride;
      b_ += block_width;
    } else {
      sum_row_c_0 = zero;
      sum_row_c_1 = zero;
      sum_row_c_2 = zero;
      sum_row_c_3 = zero;
    }

    sum_row_a_0 = _mm_add_epi32(sum_row_a_0, sum_row_b_0);
    sum_row_a_0 = _mm_add_epi32(sum_row_a_0, sum_row_c_0);
    sum_row_a_1 = _mm_add_epi32(sum_row_a_1, sum_row_b_1);
    sum_row_a_1 = _mm_add_epi32(sum_row_a_1, sum_row_c_1);
    if (block_width == 16) {
      sum_row_a_2 = _mm_add_epi32(sum_row_a_2, sum_row_b_2);
      sum_row_a_2 = _mm_add_epi32(sum_row_a_2, sum_row_c_2);
      sum_row_a_3 = _mm_add_epi32(sum_row_a_3, sum_row_b_3);
      sum_row_a_3 = _mm_add_epi32(sum_row_a_3, sum_row_c_3);
    }

    // Multiply by 3.
    //sum_row_a_0 = _mm_mullo_epi32(sum_row_a_0, three);
    //sum_row_a_1 = _mm_mullo_epi32(sum_row_a_1, three);
    //sum_row_a_2 = _mm_mullo_epi32(sum_row_a_2, three);
    //sum_row_a_3 = _mm_mullo_epi32(sum_row_a_3, three);

    _mm_storeu_si128((__m128i *)sum_row, sum_row_a_0);
    _mm_storeu_si128((__m128i *)(sum_row + 4), sum_row_a_1);
    if (block_width == 16) {
      _mm_storeu_si128((__m128i *)(sum_row + 8), sum_row_a_2);
      _mm_storeu_si128((__m128i *)(sum_row + 12), sum_row_a_3);
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

      diff_sq = sum_row[i];

      diff_sq *= 3;
      diff_sq /= num_values;

      if (num_values == 4) {
      } else if (num_values == 6) {
        diff_sq = ((int64_t)sum_row[i] << 15) >> 16;
      } else if (num_values == 9) {
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
    sum_row_a_2 = sum_row_b_2;
    sum_row_a_3 = sum_row_b_3;
    sum_row_b_0 = sum_row_c_0;
    sum_row_b_1 = sum_row_c_1;
    sum_row_b_2 = sum_row_c_2;
    sum_row_b_3 = sum_row_c_3;
  }
}
