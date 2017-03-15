/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <tmmintrin.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

// Load 16 values each from 'a' and 'b'. Compute the difference squared and sum
// neighboring pixels such that:
// sum[1] = (a[0]-b[0])^2 + (a[1]-b[1])^2 + (a[2]-b[2])^2
// The values are returned in sum_0 and sum_1 as *unsigned* 16 bit values.
static void sum_16(const uint8_t *a, const uint8_t *b, __m128i *sum_0,
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
static void sum_8(const uint8_t *a, const uint8_t *b, __m128i *sum_0) {
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

// Average the value based on the number of neighbors summed (9 for pixels away
// from the border, 4 for pixels in corners, and 6 for other edge values).
//
// Add in the rounding factor and shift, clamp to 16, invert and shift.
static __m128i average_8(__m128i sum, const __m128i mul_constants,
                         const __m128i strength, const __m128i rounding,
                         const __m128i weight) {
  const __m128i sixteen = _mm_set1_epi16(16);

  sum = _mm_mulhi_epu16(sum, mul_constants);

  // or adjust the multiply value so the mul/high half takes care of this ...
  sum = _mm_adds_epu16(sum, rounding);
  sum = _mm_srl_epi16(sum, strength);

  sum = _mm_min_epi16(sum, sixteen);

  sum = _mm_sub_epi16(sixteen, sum);

  return _mm_mullo_epi16(sum, weight);
}

static void average_16(__m128i *sum_0_u16, __m128i *sum_1_u16,
                       const __m128i mul_constants_0,
                       const __m128i mul_constants_1, const __m128i strength,
                       const __m128i rounding_u16, const __m128i weight_u16) {
  const __m128i sixteen = _mm_set1_epi16(16);
  __m128i input_0, input_1;

  input_0 = _mm_mulhi_epu16(*sum_0_u16, mul_constants_0);
  input_0 = _mm_adds_epu16(input_0, rounding_u16);

  input_1 = _mm_mulhi_epu16(*sum_1_u16, mul_constants_1);
  input_1 = _mm_adds_epu16(input_1, rounding_u16);

  input_0 = _mm_srl_epi16(input_0, strength);
  input_1 = _mm_srl_epi16(input_1, strength);

  input_0 = _mm_min_epi16(input_0, sixteen);
  input_1 = _mm_min_epi16(input_1, sixteen);
  input_0 = _mm_sub_epi16(sixteen, input_0);
  input_1 = _mm_sub_epi16(sixteen, input_1);

  *sum_0_u16 = _mm_mullo_epi16(input_0, weight_u16);
  *sum_1_u16 = _mm_mullo_epi16(input_1, weight_u16);
}

// Add the value to 'count'. Multiply by pred and add to 'accumulator.'
static void accumulate_and_store_8(const __m128i sum_u16, const uint8_t *pred,
                                   uint16_t *count, unsigned int *accumulator) {
  const __m128i pred_u8 = _mm_loadl_epi64((const __m128i *)pred);
  const __m128i zero = _mm_setzero_si128();
  __m128i count_u16 = _mm_loadu_si128((const __m128i *)count);
  __m128i pred_u16 = _mm_unpacklo_epi8(pred_u8, zero);
  __m128i pred_0_u32, pred_1_u32;
  __m128i accum_0_u32, accum_1_u32;

  count_u16 = _mm_adds_epu16(count_u16, sum_u16);
  _mm_storeu_si128((__m128i *)count, count_u16);

  pred_u16 = _mm_mullo_epi16(sum_u16, pred_u16);

  pred_0_u32 = _mm_unpacklo_epi16(pred_u16, zero);
  pred_1_u32 = _mm_unpackhi_epi16(pred_u16, zero);

  accum_0_u32 = _mm_loadu_si128((const __m128i *)accumulator);
  accum_1_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 4));

  accum_0_u32 = _mm_add_epi32(pred_0_u32, accum_0_u32);
  accum_1_u32 = _mm_add_epi32(pred_1_u32, accum_1_u32);

  _mm_storeu_si128((__m128i *)accumulator, accum_0_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 4), accum_1_u32);
}

static void accumulate_and_store_16(const __m128i sum_0_u16,
                                    const __m128i sum_1_u16,
                                    const uint8_t *pred, uint16_t *count,
                                    unsigned int *accumulator) {
  const __m128i pred_u8 = _mm_loadu_si128((const __m128i *)pred);
  const __m128i zero = _mm_setzero_si128();
  __m128i count_0_u16 = _mm_loadu_si128((const __m128i *)count),
          count_1_u16 = _mm_loadu_si128((const __m128i *)(count + 8));
  __m128i pred_0_u16 = _mm_unpacklo_epi8(pred_u8, zero),
          pred_1_u16 = _mm_unpackhi_epi8(pred_u8, zero);
  __m128i pred_0_u32, pred_1_u32, pred_2_u32, pred_3_u32;
  __m128i accum_0_u32, accum_1_u32, accum_2_u32, accum_3_u32;

  count_0_u16 = _mm_adds_epu16(count_0_u16, sum_0_u16);
  _mm_storeu_si128((__m128i *)count, count_0_u16);

  count_1_u16 = _mm_adds_epu16(count_1_u16, sum_1_u16);
  _mm_storeu_si128((__m128i *)(count + 8), count_1_u16);

  pred_0_u16 = _mm_mullo_epi16(sum_0_u16, pred_0_u16);
  pred_1_u16 = _mm_mullo_epi16(sum_1_u16, pred_1_u16);

  pred_0_u32 = _mm_unpacklo_epi16(pred_0_u16, zero);
  pred_1_u32 = _mm_unpackhi_epi16(pred_0_u16, zero);
  pred_2_u32 = _mm_unpacklo_epi16(pred_1_u16, zero);
  pred_3_u32 = _mm_unpackhi_epi16(pred_1_u16, zero);

  accum_0_u32 = _mm_loadu_si128((const __m128i *)accumulator);
  accum_1_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 4));
  accum_2_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 8));
  accum_3_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 12));

  accum_0_u32 = _mm_add_epi32(pred_0_u32, accum_0_u32);
  accum_1_u32 = _mm_add_epi32(pred_1_u32, accum_1_u32);
  accum_2_u32 = _mm_add_epi32(pred_2_u32, accum_2_u32);
  accum_3_u32 = _mm_add_epi32(pred_3_u32, accum_3_u32);

  _mm_storeu_si128((__m128i *)accumulator, accum_0_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 4), accum_1_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 8), accum_2_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 12), accum_3_u32);
}

void vp9_temporal_filter_apply_ssse3(const uint8_t *a, unsigned int stride,
                                     const uint8_t *b, unsigned int width,
                                     unsigned int height, int strength,
                                     int weight, unsigned int *accumulator,
                                     uint16_t *count) {
  unsigned int h;
  const int rounding = strength > 0 ? 1 << (strength - 1) : 0;
  const __m128i rounding_u16 = _mm_set1_epi16(rounding);
  // _mm_srl_epi16 uses a single 64 bit element for the shift. It can't shift
  // different values by different amounts.
  const __m128i strength_u128 = _mm_set_epi32(0, 0, 0, strength);
  const __m128i weight_u16 = _mm_set1_epi16(weight);

  const __m128i zero = _mm_setzero_si128();

  assert(strength >= 0);
  assert(strength <= 6);

  assert(weight >= 0);
  assert(weight <= 2);

  assert(width == 8 || width == 16);

  // TODO(johannkoenig) Use uint32_t for accumulator.
  assert(sizeof(unsigned int) == sizeof(uint32_t));

  if (width == 8) {
    __m128i sum_row_a, sum_row_b, sum_row_c;

    sum_row_a = zero;
    sum_8(a, b, &sum_row_b);
    a += stride;

    for (h = 0; h < height; ++h) {
      __m128i mul_constants;
      if (h != height - 1) {
        // Pass b+width here because the current row must be used for
        // finish_row.
        sum_8(a, b + width, &sum_row_c);
      } else {
        sum_row_c = zero;
      }

      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_b);
      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_c);

      if (h == 0 || h == (height - 1)) {
        mul_constants = _mm_setr_epi16(
            (int16_t)49152, (int16_t)32768, (int16_t)32768, (int16_t)32768,
            (int16_t)32768, (int16_t)32768, (int16_t)32768, (int16_t)49152);
      } else {
        mul_constants = _mm_setr_epi16(
            (int16_t)32768, (int16_t)21846, (int16_t)21846, (int16_t)21846,
            (int16_t)21846, (int16_t)21846, (int16_t)21846, (int16_t)32768);
      }

      sum_row_a = average_8(sum_row_a, mul_constants, strength_u128,
                            rounding_u16, weight_u16);

      accumulate_and_store_8(sum_row_a, b, count, accumulator);

      a += stride;
      b += width;
      count += width;
      accumulator += width;

      sum_row_a = sum_row_b;
      sum_row_b = sum_row_c;
    }
  } else if (width == 16) {
    __m128i sum_row_a_0, sum_row_a_1;
    __m128i sum_row_b_0, sum_row_b_1;
    __m128i sum_row_c_0, sum_row_c_1;

    sum_row_a_0 = zero;
    sum_row_a_1 = zero;
    sum_16(a, b, &sum_row_b_0, &sum_row_b_1);
    a += stride;
    for (h = 0; h < height; ++h) {
      __m128i mul_constants_0, mul_constants_1;
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

      if (h == 0 || h == (height - 1)) {
        mul_constants_0 = _mm_setr_epi16(
            (int16_t)49152, (int16_t)32768, (int16_t)32768, (int16_t)32768,
            (int16_t)32768, (int16_t)32768, (int16_t)32768, (int16_t)32768);
        mul_constants_1 = _mm_setr_epi16(
            (int16_t)32768, (int16_t)32768, (int16_t)32768, (int16_t)32768,
            (int16_t)32768, (int16_t)32768, (int16_t)32768, (int16_t)49152);
      } else {
        mul_constants_0 = _mm_setr_epi16(
            (int16_t)32768, (int16_t)21846, (int16_t)21846, (int16_t)21846,
            (int16_t)21846, (int16_t)21846, (int16_t)21846, (int16_t)21846);
        mul_constants_1 = _mm_setr_epi16(
            (int16_t)21846, (int16_t)21846, (int16_t)21846, (int16_t)21846,
            (int16_t)21846, (int16_t)21846, (int16_t)21846, (int16_t)32768);
      }

      average_16(&sum_row_a_0, &sum_row_a_1, mul_constants_0, mul_constants_1,
                 strength_u128, rounding_u16, weight_u16);
      accumulate_and_store_16(sum_row_a_0, sum_row_a_1, b, count, accumulator);

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
