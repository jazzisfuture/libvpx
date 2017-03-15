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
#include <stdio.h>

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

  __m128i shift_left = _mm_slli_si128(diff_sq_0_u16, 2);
  __m128i shift_right = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 2);

  // It becomes necessary to treat the values as unsigned at this point. The
  // 255^2 fits in uint16_t but not int16_t. Use saturating adds from this point
  // foward since the filter is only applied to smooth small pixel changes. Once
  // the value has saturated to uint16_t it is well outside the useful range.
  __m128i sum_u16 = _mm_adds_epu16(shift_left, diff_sq_0_u16);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_0 = sum_u16;

  shift_left = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 14);
  shift_right = _mm_srli_si128(diff_sq_1_u16, 2);

  sum_u16 = _mm_adds_epu16(shift_left, diff_sq_1_u16);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_1 = sum_u16;
}

// Same but for 8 input values.
static INLINE void sum_8(const uint8_t *a, const uint8_t *b, __m128i *sum_0) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadl_epi64((const __m128i *)a);
  const __m128i b_u8 = _mm_loadl_epi64((const __m128i *)b);

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

static void finish_row(const uint16_t *a, const uint8_t *b, const int width,
                       const int top_bottom, const int strength,
                       const int rounding, const int weight,
                       unsigned int *accumulator, uint16_t *count) {
  int w;
  for (w = 0; w < width; ++w) {
    int num_values = 9;
    int diff_sq;

    if (top_bottom) num_values = 6;

    if ((w == 0) || (w == width - 1)) {
      if (num_values == 6)
        num_values = 4;
      else
        num_values = 6;
    }

    // Instead of multiplying by 3 then dividing by the number of blocks being
    // summed, attempt to do this with integer math. This appears to be the
    // most likely path for getting this into SIMD.
    if (num_values == 4) {
      //diff_sq = ((uint32_t)a[w] * 3) >> 2;
      diff_sq = ((uint32_t)a[w] * 384) >> 9;
    } else if (num_values == 6) {
      //diff_sq = ((uint32_t)a[w] * 2) >> 2;
      diff_sq = ((uint32_t)a[w] * 256) >> 9;
    } else {  // if (num_values == 9) {
      // For results < 16 this approximation of division by 3 is sufficient.
      diff_sq = ((uint32_t)a[w] * 171) >> 9;
    }

    // The minimum value for saturation is 2976 because strength maxes
    // out at 6.
    // 2976 * 3 / 9 = 992
    // 992 + rounding = 1024
    // 1024 >> 6 = 16
    // Due to this, uint8_t can not be used as a saturation target.

    diff_sq += rounding;
    diff_sq >>= strength;
    if (diff_sq > 16) diff_sq = 16;
    diff_sq = 16 - diff_sq;
    diff_sq *= weight;
    count[w] += diff_sq;
    accumulator[w] += diff_sq * b[w];
  }
}

void vp9_temporal_filter_apply_ssse3(const uint8_t *a, unsigned int stride,
                                     const uint8_t *b, unsigned int width,
                                     unsigned int height, int strength,
                                     int weight, unsigned int *accumulator,
                                     uint16_t *count) {
  unsigned int h;
  uint16_t sum_row[16];
  const int rounding = strength > 0 ? 1 << (strength - 1) : 0;

  const __m128i zero = _mm_setzero_si128();

  if (width == 8) {
    __m128i sum_row_a;
    __m128i sum_row_b;
    __m128i sum_row_c;

    sum_row_a = zero;
    sum_8(a, b, &sum_row_b);
    a += stride;

    for (h = 0; h < height; ++h) {
      if (h != height - 1) {
        // Pass b+width here because the current row must be used for finish_row.
        sum_8(a, b + width, &sum_row_c);
      } else {
        sum_row_c = zero;
      }

      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_b);
      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_c);

      _mm_store_si128((__m128i *)sum_row, sum_row_a);

      finish_row(sum_row, b, width, h == 0 || h == (height - 1), strength, rounding,
                 weight, accumulator, count);

      a += stride;
      b += width;
      count += width;
      accumulator += width;

      sum_row_a = sum_row_b;
      sum_row_b = sum_row_c;
    }
  } else {
    __m128i sum_row_a_0, sum_row_a_1;
    __m128i sum_row_b_0, sum_row_b_1;
    __m128i sum_row_c_0, sum_row_c_1;

    sum_row_a_0 = zero;
    sum_row_a_1 = zero;
    sum_16(a, b, &sum_row_b_0, &sum_row_b_1);
    a += stride;
    for (h = 0; h < height; ++h) {
      if (h != height - 1) {
        sum_16(a, b + width, &sum_row_c_0, &sum_row_c_1);
      } else {
        sum_row_c_0 = zero;
        sum_row_c_1 = zero;
      }

      sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_b_0);
      sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_c_0);
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_b_1);
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_c_1);

      _mm_store_si128((__m128i *)sum_row, sum_row_a_0);
      _mm_store_si128((__m128i *)(sum_row + 8), sum_row_a_1);

      finish_row(sum_row, b, width, h == 0 || h == (height - 1), strength,
                 rounding, weight, accumulator, count);

      a += stride;
      b += width;
      count += width;
      accumulator += width;

      sum_row_a_0 = sum_row_b_0;
      sum_row_a_1 = sum_row_b_1;
      sum_row_b_0 = sum_row_c_0;
      sum_row_b_1 = sum_row_c_1;
    }
  }
}
