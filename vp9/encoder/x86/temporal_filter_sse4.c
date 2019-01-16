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
#include <smmintrin.h>

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

// Division using multiplication and shifting. The C implementation does:
// modifier *= 3;
// modifier /= index;
// where 'modifier' is a set of summed values and 'index' is the number of
// summed values.
//
// This equation works out to (m * 3) / i which reduces to:
// m * 3/4
// m * 1/2
// m * 1/3
//
// By pairing the multiply with a down shift by 16 (_mm_mulhi_epu16):
// m * C / 65536
// we can create a C to replicate the division.
//
// m * 49152 / 65536 = m * 3/4
// m * 32758 / 65536 = m * 1/2
// m * 21846 / 65536 = m * 0.3333
//
// These are loaded using an instruction expecting int16_t values but are used
// with _mm_mulhi_epu16(), which treats them as unsigned.
#define NEIGHBOR_CONSTANT_4 (int16_t)49152
#define NEIGHBOR_CONSTANT_5 (int16_t)39322
#define NEIGHBOR_CONSTANT_6 (int16_t)32768
#define NEIGHBOR_CONSTANT_7 (int16_t)28087
#define NEIGHBOR_CONSTANT_8 (int16_t)24576
#define NEIGHBOR_CONSTANT_9 (int16_t)21846
#define NEIGHBOR_CONSTANT_10 (int16_t)19661
#define NEIGHBOR_CONSTANT_11 (int16_t)17874
#define NEIGHBOR_CONSTANT_13 (int16_t)15124

// Load values from 'a' and 'b'. Compute the difference squared and sum
// neighboring values such that:
// sum[1] = (a[0]-b[0])^2 + (a[1]-b[1])^2 + (a[2]-b[2])^2
// Values to the left and right of the row are set to 0.
// The values are returned in sum_0 and sum_1 as *unsigned* 16 bit values.
static void sum_8(const uint8_t *a, const uint8_t *b, __m128i *sum) {
  const __m128i a_u8 = _mm_loadl_epi64((const __m128i *)a);
  const __m128i b_u8 = _mm_loadl_epi64((const __m128i *)b);

  const __m128i a_u16 = _mm_cvtepu8_epi16(a_u8);
  const __m128i b_u16 = _mm_cvtepu8_epi16(b_u8);

  const __m128i diff_s16 = _mm_sub_epi16(a_u16, b_u16);
  const __m128i diff_sq_u16 = _mm_mullo_epi16(diff_s16, diff_s16);

  // Shift all the values one place to the left/right so we can efficiently sum
  // diff_sq_u16[i - 1] + diff_sq_u16[i] + diff_sq_u16[i + 1].
  const __m128i shift_left = _mm_slli_si128(diff_sq_u16, 2);
  const __m128i shift_right = _mm_srli_si128(diff_sq_u16, 2);

  // It becomes necessary to treat the values as unsigned at this point. The
  // 255^2 fits in uint16_t but not int16_t. Use saturating adds from this point
  // forward since the filter is only applied to smooth small pixel changes.
  // Once the value has saturated to uint16_t it is well outside the useful
  // range.
  __m128i sum_u16 = _mm_adds_epu16(diff_sq_u16, shift_left);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum = sum_u16;
}

static void sum_16(const uint8_t *a, const uint8_t *b, __m128i *sum_0,
                   __m128i *sum_1) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_u8 = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_u8 = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_0_u16 = _mm_cvtepu8_epi16(a_u8);
  const __m128i a_1_u16 = _mm_unpackhi_epi8(a_u8, zero);
  const __m128i b_0_u16 = _mm_cvtepu8_epi16(b_u8);
  const __m128i b_1_u16 = _mm_unpackhi_epi8(b_u8, zero);

  const __m128i diff_0_s16 = _mm_sub_epi16(a_0_u16, b_0_u16);
  const __m128i diff_1_s16 = _mm_sub_epi16(a_1_u16, b_1_u16);
  const __m128i diff_sq_0_u16 = _mm_mullo_epi16(diff_0_s16, diff_0_s16);
  const __m128i diff_sq_1_u16 = _mm_mullo_epi16(diff_1_s16, diff_1_s16);

  __m128i shift_left = _mm_slli_si128(diff_sq_0_u16, 2);
  // Use _mm_alignr_epi8() to "shift in" diff_sq_u16[8].
  __m128i shift_right = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 2);

  __m128i sum_u16 = _mm_adds_epu16(diff_sq_0_u16, shift_left);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_0 = sum_u16;

  shift_left = _mm_alignr_epi8(diff_sq_1_u16, diff_sq_0_u16, 14);
  shift_right = _mm_srli_si128(diff_sq_1_u16, 2);

  sum_u16 = _mm_adds_epu16(diff_sq_1_u16, shift_left);
  sum_u16 = _mm_adds_epu16(sum_u16, shift_right);

  *sum_1 = sum_u16;
}

static INLINE void store_dist_16(const uint8_t *a, const uint8_t *b,
                                 uint16_t *dst) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i a_reg = _mm_loadu_si128((const __m128i *)a);
  const __m128i b_reg = _mm_loadu_si128((const __m128i *)b);

  const __m128i a_first = _mm_cvtepu8_epi16(a_reg);
  const __m128i a_second = _mm_unpackhi_epi8(a_reg, zero);
  const __m128i b_first = _mm_cvtepu8_epi16(b_reg);
  const __m128i b_second = _mm_unpackhi_epi8(b_reg, zero);

  __m128i dist_first, dist_second;

  dist_first = _mm_sub_epi16(a_first, b_first);
  dist_second = _mm_sub_epi16(a_second, b_second);
  dist_first = _mm_mullo_epi16(dist_first, dist_first);
  dist_second = _mm_mullo_epi16(dist_second, dist_second);

  _mm_storeu_si128((__m128i *)dst, dist_first);
  _mm_storeu_si128((__m128i *)(dst + 8), dist_second);
}

static INLINE void store_dist_32(const uint8_t *a, const uint8_t *b,
                                 uint16_t *dst) {
  store_dist_16(a, b, dst);
  store_dist_16(a + 16, b + 16, dst + 16);
}

static INLINE void read_dist_8(const uint16_t *dist, __m128i *dist_reg) {
  *dist_reg = _mm_loadu_si128((const __m128i *)dist);
}

static INLINE void read_dist_16(const uint16_t *dist, __m128i *reg_first,
                                __m128i *reg_second) {
  read_dist_8(dist, reg_first);
  read_dist_8(dist + 8, reg_second);
}

// Average the value based on the number of values summed (9 for pixels away
// from the border, 4 for pixels in corners, and 6 for other edge values).
//
// Add in the rounding factor and shift, clamp to 16, invert and shift. Multiply
// by weight.
static __m128i average_8(__m128i sum, const __m128i mul_constants,
                         const int strength, const int rounding,
                         const int weight) {
  // _mm_srl_epi16 uses the lower 64 bit value for the shift.
  const __m128i strength_u128 = _mm_set_epi32(0, 0, 0, strength);
  const __m128i rounding_u16 = _mm_set1_epi16(rounding);
  const __m128i weight_u16 = _mm_set1_epi16(weight);
  const __m128i sixteen = _mm_set1_epi16(16);

  // modifier * 3 / index;
  sum = _mm_mulhi_epu16(sum, mul_constants);

  sum = _mm_adds_epu16(sum, rounding_u16);
  sum = _mm_srl_epi16(sum, strength_u128);

  // The maximum input to this comparison is UINT16_MAX * NEIGHBOR_CONSTANT_4
  // >> 16 (also NEIGHBOR_CONSTANT_4 -1) which is 49151 / 0xbfff / -16385
  // So this needs to use the epu16 version which did not come until SSE4.
  sum = _mm_min_epu16(sum, sixteen);

  sum = _mm_sub_epi16(sixteen, sum);

  return _mm_mullo_epi16(sum, weight_u16);
}

static INLINE void average_16(__m128i *sum_0_u16, __m128i *sum_1_u16,
                              const __m128i mul_constants_0,
                              const __m128i mul_constants_1, const int strength,
                              const int rounding, const int weight) {
  const __m128i strength_u128 = _mm_set_epi32(0, 0, 0, strength);
  const __m128i rounding_u16 = _mm_set1_epi16(rounding);
  const __m128i weight_u16 = _mm_set1_epi16(weight);
  const __m128i sixteen = _mm_set1_epi16(16);
  __m128i input_0, input_1;

  input_0 = _mm_mulhi_epu16(*sum_0_u16, mul_constants_0);
  input_0 = _mm_adds_epu16(input_0, rounding_u16);

  input_1 = _mm_mulhi_epu16(*sum_1_u16, mul_constants_1);
  input_1 = _mm_adds_epu16(input_1, rounding_u16);

  input_0 = _mm_srl_epi16(input_0, strength_u128);
  input_1 = _mm_srl_epi16(input_1, strength_u128);

  input_0 = _mm_min_epu16(input_0, sixteen);
  input_1 = _mm_min_epu16(input_1, sixteen);
  input_0 = _mm_sub_epi16(sixteen, input_0);
  input_1 = _mm_sub_epi16(sixteen, input_1);

  *sum_0_u16 = _mm_mullo_epi16(input_0, weight_u16);
  *sum_1_u16 = _mm_mullo_epi16(input_1, weight_u16);
}

// Add 'sum_u16' to 'count'. Multiply by 'pred' and add to 'accumulator.'
static void accumulate_and_store_8(const __m128i sum_u16, const uint8_t *pred,
                                   uint16_t *count, uint32_t *accumulator) {
  const __m128i pred_u8 = _mm_loadl_epi64((const __m128i *)pred);
  const __m128i zero = _mm_setzero_si128();
  __m128i count_u16 = _mm_loadu_si128((const __m128i *)count);
  __m128i pred_u16 = _mm_cvtepu8_epi16(pred_u8);
  __m128i pred_0_u32, pred_1_u32;
  __m128i accum_0_u32, accum_1_u32;

  count_u16 = _mm_adds_epu16(count_u16, sum_u16);
  _mm_storeu_si128((__m128i *)count, count_u16);

  pred_u16 = _mm_mullo_epi16(sum_u16, pred_u16);

  pred_0_u32 = _mm_cvtepu16_epi32(pred_u16);
  pred_1_u32 = _mm_unpackhi_epi16(pred_u16, zero);

  accum_0_u32 = _mm_loadu_si128((const __m128i *)accumulator);
  accum_1_u32 = _mm_loadu_si128((const __m128i *)(accumulator + 4));

  accum_0_u32 = _mm_add_epi32(pred_0_u32, accum_0_u32);
  accum_1_u32 = _mm_add_epi32(pred_1_u32, accum_1_u32);

  _mm_storeu_si128((__m128i *)accumulator, accum_0_u32);
  _mm_storeu_si128((__m128i *)(accumulator + 4), accum_1_u32);
}

static INLINE void accumulate_and_store_16(const __m128i sum_0_u16,
                                           const __m128i sum_1_u16,
                                           const uint8_t *pred, uint16_t *count,
                                           uint32_t *accumulator) {
  const __m128i pred_u8 = _mm_loadu_si128((const __m128i *)pred);
  const __m128i zero = _mm_setzero_si128();
  __m128i count_0_u16 = _mm_loadu_si128((const __m128i *)count),
          count_1_u16 = _mm_loadu_si128((const __m128i *)(count + 8));
  __m128i pred_0_u16 = _mm_cvtepu8_epi16(pred_u8),
          pred_1_u16 = _mm_unpackhi_epi8(pred_u8, zero);
  __m128i pred_0_u32, pred_1_u32, pred_2_u32, pred_3_u32;
  __m128i accum_0_u32, accum_1_u32, accum_2_u32, accum_3_u32;

  count_0_u16 = _mm_adds_epu16(count_0_u16, sum_0_u16);
  _mm_storeu_si128((__m128i *)count, count_0_u16);

  count_1_u16 = _mm_adds_epu16(count_1_u16, sum_1_u16);
  _mm_storeu_si128((__m128i *)(count + 8), count_1_u16);

  pred_0_u16 = _mm_mullo_epi16(sum_0_u16, pred_0_u16);
  pred_1_u16 = _mm_mullo_epi16(sum_1_u16, pred_1_u16);

  pred_0_u32 = _mm_cvtepu16_epi32(pred_0_u16);
  pred_1_u32 = _mm_unpackhi_epi16(pred_0_u16, zero);
  pred_2_u32 = _mm_cvtepu16_epi32(pred_1_u16);
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

// Read in 8 pixels from y_dist. For each index i, compute y_dist[i-1] +
// y_dist[i] + y_dist[i+1] and store in sum as 16-bit unsigned int.
static INLINE void get_sum_8(const uint16_t *y_dist, __m128i *sum) {
  __m128i dist_reg, dist_left, dist_right;

  dist_reg = _mm_loadu_si128((const __m128i *)y_dist);
  dist_left = _mm_loadu_si128((const __m128i *)(y_dist - 1));
  dist_right = _mm_loadu_si128((const __m128i *)(y_dist + 1));

  *sum = _mm_adds_epu16(dist_reg, dist_left);
  *sum = _mm_adds_epu16(*sum, dist_right);
}

// Read in 16 pixels from y_dist. For each index i, compute y_dist[i-1] +
// y_dist[i] + y_dist[i+1]. Store the result for first 8 pixels in sum_first and
// the rest in sum_second.
static INLINE void get_sum_16(const uint16_t *y_dist, __m128i *sum_first,
                              __m128i *sum_second) {
  get_sum_8(y_dist, sum_first);
  get_sum_8(y_dist + 8, sum_second);
}

void vp9_temporal_filter_apply_sse4_1(const uint8_t *a, unsigned int stride,
                                      const uint8_t *b, unsigned int width,
                                      unsigned int height, int strength,
                                      int weight, uint32_t *accumulator,
                                      uint16_t *count) {
  unsigned int h;
  const int rounding = (1 << strength) >> 1;

  assert(strength >= 0);
  assert(strength <= 6);

  assert(weight >= 0);
  assert(weight <= 2);

  assert(width == 8 || width == 16);

  if (width == 8) {
    __m128i sum_row_a, sum_row_b, sum_row_c;
    __m128i mul_constants = _mm_setr_epi16(
        NEIGHBOR_CONSTANT_4, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
        NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
        NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_4);

    sum_8(a, b, &sum_row_a);
    sum_8(a + stride, b + width, &sum_row_b);
    sum_row_c = _mm_adds_epu16(sum_row_a, sum_row_b);
    sum_row_c = average_8(sum_row_c, mul_constants, strength, rounding, weight);
    accumulate_and_store_8(sum_row_c, b, count, accumulator);

    a += stride + stride;
    b += width;
    count += width;
    accumulator += width;

    mul_constants = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_9,
                                   NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                   NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                   NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_6);

    for (h = 0; h < height - 2; ++h) {
      sum_8(a, b + width, &sum_row_c);
      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_b);
      sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_c);
      sum_row_a =
          average_8(sum_row_a, mul_constants, strength, rounding, weight);
      accumulate_and_store_8(sum_row_a, b, count, accumulator);

      a += stride;
      b += width;
      count += width;
      accumulator += width;

      sum_row_a = sum_row_b;
      sum_row_b = sum_row_c;
    }

    mul_constants = _mm_setr_epi16(NEIGHBOR_CONSTANT_4, NEIGHBOR_CONSTANT_6,
                                   NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                   NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                   NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_4);
    sum_row_a = _mm_adds_epu16(sum_row_a, sum_row_b);
    sum_row_a = average_8(sum_row_a, mul_constants, strength, rounding, weight);
    accumulate_and_store_8(sum_row_a, b, count, accumulator);

  } else {  // width == 16
    __m128i sum_row_a_0, sum_row_a_1;
    __m128i sum_row_b_0, sum_row_b_1;
    __m128i sum_row_c_0, sum_row_c_1;
    __m128i mul_constants_0 = _mm_setr_epi16(
                NEIGHBOR_CONSTANT_4, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6),
            mul_constants_1 = _mm_setr_epi16(
                NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_4);

    sum_16(a, b, &sum_row_a_0, &sum_row_a_1);
    sum_16(a + stride, b + width, &sum_row_b_0, &sum_row_b_1);

    sum_row_c_0 = _mm_adds_epu16(sum_row_a_0, sum_row_b_0);
    sum_row_c_1 = _mm_adds_epu16(sum_row_a_1, sum_row_b_1);

    average_16(&sum_row_c_0, &sum_row_c_1, mul_constants_0, mul_constants_1,
               strength, rounding, weight);
    accumulate_and_store_16(sum_row_c_0, sum_row_c_1, b, count, accumulator);

    a += stride + stride;
    b += width;
    count += width;
    accumulator += width;

    mul_constants_0 = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9);
    mul_constants_1 = _mm_setr_epi16(NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_9,
                                     NEIGHBOR_CONSTANT_9, NEIGHBOR_CONSTANT_6);
    for (h = 0; h < height - 2; ++h) {
      sum_16(a, b + width, &sum_row_c_0, &sum_row_c_1);

      sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_b_0);
      sum_row_a_0 = _mm_adds_epu16(sum_row_a_0, sum_row_c_0);
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_b_1);
      sum_row_a_1 = _mm_adds_epu16(sum_row_a_1, sum_row_c_1);

      average_16(&sum_row_a_0, &sum_row_a_1, mul_constants_0, mul_constants_1,
                 strength, rounding, weight);
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

    mul_constants_0 = _mm_setr_epi16(NEIGHBOR_CONSTANT_4, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6);
    mul_constants_1 = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_6,
                                     NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_4);
    sum_row_c_0 = _mm_adds_epu16(sum_row_a_0, sum_row_b_0);
    sum_row_c_1 = _mm_adds_epu16(sum_row_a_1, sum_row_b_1);

    average_16(&sum_row_c_0, &sum_row_c_1, mul_constants_0, mul_constants_1,
               strength, rounding, weight);
    accumulate_and_store_16(sum_row_c_0, sum_row_c_1, b, count, accumulator);
  }
}

// Read in a row of chroma values corresponds to a row of 16 luma values.
static INLINE void read_chroma_dist_row_16(int ss_x, const uint16_t *u_dist,
                                           const uint16_t *v_dist,
                                           __m128i *u_first, __m128i *u_second,
                                           __m128i *v_first,
                                           __m128i *v_second) {
  if (ss_x == 0) {
    // If there is no chroma subsampling in the horizaontal direction, then we
    // need to load 16 entries from chroma.
    read_dist_16(u_dist, u_first, u_second);
    read_dist_16(v_dist, v_first, v_second);
  } else {  // ss_x == 1
    // Otherwise, we only need to load 8 entries
    __m128i u_reg, v_reg;

    read_dist_8(u_dist, &u_reg);

    *u_first = _mm_unpacklo_epi16(u_reg, u_reg);
    *u_second = _mm_unpackhi_epi16(u_reg, u_reg);

    read_dist_8(v_dist, &v_reg);

    *v_first = _mm_unpacklo_epi16(v_reg, v_reg);
    *v_second = _mm_unpackhi_epi16(v_reg, v_reg);
  }
}

// Horizonta add unsigned 16-bit ints in src and store them as signed 32-bit int
// in dst.
static INLINE void hadd_epu16(__m128i *src, __m128i *dst) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i shift_right = _mm_srli_si128(*src, 2);

  const __m128i odd = _mm_blend_epi16(shift_right, zero, 170);
  const __m128i even = _mm_blend_epi16(*src, zero, 170);

  *dst = _mm_add_epi32(even, odd);
}

// Sum add a row of 32 luma distortion to the corresponding chroma values.
static INLINE void add_32_luma_to_chroma_mod(const uint16_t *y_dist,
                                             unsigned int dist_stride, int ss_y,
                                             __m128i *u_mod_first,
                                             __m128i *v_mod_first,
                                             __m128i *u_mod_second,
                                             __m128i *v_mod_second) {
  __m128i y_first, y_second;
  __m128i y_tmp_0, y_tmp_1;

  // First 16 luma pixels
  read_dist_16(y_dist, &y_tmp_0, &y_tmp_1);
  if (ss_y == 1) {
    __m128i y_tmp_0_1, y_tmp_1_1;
    read_dist_16(y_dist + dist_stride, &y_tmp_0_1, &y_tmp_1_1);

    y_tmp_0 = _mm_adds_epu16(y_tmp_0, y_tmp_0_1);
    y_tmp_1 = _mm_adds_epu16(y_tmp_1, y_tmp_1_1);
  }

  hadd_epu16(&y_tmp_0, &y_tmp_0);
  hadd_epu16(&y_tmp_1, &y_tmp_1);

  y_first = _mm_packus_epi32(y_tmp_0, y_tmp_1);

  *u_mod_first = _mm_adds_epu16(*u_mod_first, y_first);
  *v_mod_first = _mm_adds_epu16(*v_mod_first, y_first);

  // Next 16 luma pixels
  read_dist_16(y_dist + 16, &y_tmp_0, &y_tmp_1);
  if (ss_y == 1) {
    __m128i y_tmp_0_1, y_tmp_1_1;
    read_dist_16(y_dist + 16 + dist_stride, &y_tmp_0_1, &y_tmp_1_1);

    y_tmp_0 = _mm_adds_epu16(y_tmp_0, y_tmp_0_1);
    y_tmp_1 = _mm_adds_epu16(y_tmp_1, y_tmp_1_1);
  }

  hadd_epu16(&y_tmp_0, &y_tmp_0);
  hadd_epu16(&y_tmp_1, &y_tmp_1);

  y_second = _mm_packus_epi32(y_tmp_0, y_tmp_1);

  *u_mod_second = _mm_adds_epu16(*u_mod_second, y_second);
  *v_mod_second = _mm_adds_epu16(*v_mod_second, y_second);
}

// Sum add a row of 16 luma distortion to the corresponding chroma values.
static INLINE void add_16_luma_to_chroma_mod(const uint16_t *y_dist,
                                             unsigned int dist_stride, int ss_y,
                                             __m128i *u_mod_first,
                                             __m128i *v_mod_first,
                                             __m128i *u_mod_second,
                                             __m128i *v_mod_second) {
  __m128i y_first, y_second;

  read_dist_16(y_dist, &y_first, &y_second);
  if (ss_y == 1) {
    __m128i y_tmp_0, y_tmp_1;
    read_dist_16(y_dist + dist_stride, &y_tmp_0, &y_tmp_1);

    y_first = _mm_adds_epu16(y_first, y_tmp_0);
    y_second = _mm_adds_epu16(y_second, y_tmp_1);
  }

  *u_mod_first = _mm_adds_epu16(*u_mod_first, y_first);
  *v_mod_first = _mm_adds_epu16(*v_mod_first, y_first);

  *u_mod_second = _mm_adds_epu16(*u_mod_second, y_second);
  *v_mod_second = _mm_adds_epu16(*v_mod_second, y_second);
}

// Apply temporal filter to the luma components. This performs temporal
// filtering on a luma block of 16 X uv_height. The variable loc indicates
// whether the current block if on the left or right half of the 32x32 block.
// loc == 0 indicates left, 1 indicates right.
static void vp9_apply_temporal_filter_luma_16(
    const uint8_t *y_src, int y_src_stride, const uint8_t *y_pre,
    int y_pre_stride, const uint8_t *u_src, const uint8_t *v_src,
    int uv_src_stride, const uint8_t *u_pre, const uint8_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw, int use_32x32,
    uint32_t *y_accum, uint16_t *y_count, const uint16_t *y_dist,
    const uint16_t *u_dist, const uint16_t *v_dist, const int loc) {
  const int rounding = (1 << strength) >> 1;
  const int dist_stride = 34;
  int weight = blk_fw[0];

  __m128i mul_first, mul_second;

  __m128i sum_row_1_first, sum_row_1_second;
  __m128i sum_row_2_first, sum_row_2_second;
  __m128i sum_row_3_first, sum_row_3_second;

  __m128i u_first, u_second;
  __m128i v_first, v_second;

  __m128i sum_row_first;
  __m128i sum_row_second;

  assert(strength >= 0);
  assert(strength <= 6);

  assert(blk_fw[0] >= 0 && (use_32x32 || blk_fw[2] >= 0));
  assert(blk_fw[0] <= 2 && (use_32x32 || blk_fw[2] <= 2));

  assert(block_width == 16);

  assert((ss_x == 0 || ss_x == 1) && (ss_y == 0 || ss_y == 1));

  (void)block_width;

  // First row
  if (loc == 0) {  // On the left
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
  } else {  // On the right
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
  }

  // Add luma values
  get_sum_16(y_dist, &sum_row_2_first, &sum_row_2_second);
  get_sum_16(y_dist + dist_stride, &sum_row_3_first, &sum_row_3_second);

  sum_row_first = _mm_adds_epu16(sum_row_2_first, sum_row_3_first);
  sum_row_second = _mm_adds_epu16(sum_row_2_second, sum_row_3_second);

  // Add chroma values
  read_chroma_dist_row_16(ss_x, u_dist, v_dist, &u_first, &u_second, &v_first,
                          &v_second);

  sum_row_first = _mm_adds_epu16(sum_row_first, u_first);
  sum_row_second = _mm_adds_epu16(sum_row_second, u_second);

  sum_row_first = _mm_adds_epu16(sum_row_first, v_first);
  sum_row_second = _mm_adds_epu16(sum_row_second, v_second);

  // Get modifier and store result
  average_16(&sum_row_first, &sum_row_second, mul_first, mul_second, strength,
             rounding, weight);
  accumulate_and_store_16(sum_row_first, sum_row_second, y_pre, y_count,
                          y_accum);

  y_src += y_src_stride;
  y_pre += y_pre_stride;
  y_count += y_pre_stride;
  y_accum += y_pre_stride;
  y_dist += dist_stride;

  u_src += uv_src_stride;
  u_pre += uv_pre_stride;
  u_dist += dist_stride;
  v_src += uv_src_stride;
  v_pre += uv_pre_stride;
  v_dist += dist_stride;

  // Then all the rows except the last one
  if (loc == 0) {  // On the left
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
  } else {  // On the right
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                               NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_8);
  }

  for (unsigned int h = 1; h < block_height - 1; ++h) {
    // Move the weight pointer to the bottom half of the blocks
    if (!use_32x32 && h == block_height / 2) {
      weight = blk_fw[2];
    }
    // Shift the rows up
    sum_row_1_first = sum_row_2_first;
    sum_row_1_second = sum_row_2_second;
    sum_row_2_first = sum_row_3_first;
    sum_row_2_second = sum_row_3_second;

    // Add luma values to the modifier
    sum_row_first = _mm_adds_epu16(sum_row_1_first, sum_row_2_first);
    sum_row_second = _mm_adds_epu16(sum_row_1_second, sum_row_2_second);

    get_sum_16(y_dist + dist_stride, &sum_row_3_first, &sum_row_3_second);

    sum_row_first = _mm_adds_epu16(sum_row_first, sum_row_3_first);
    sum_row_second = _mm_adds_epu16(sum_row_second, sum_row_3_second);

    // Add chroma values to the modifier
    if (ss_y == 0 || h % 2 == 0) {
      // Only calculate the new chroma distortion if we are at a pixel that
      // corresponds to a new chroma row
      read_chroma_dist_row_16(ss_x, u_dist, v_dist, &u_first, &u_second,
                              &v_first, &v_second);

      u_src += uv_src_stride;
      u_pre += uv_pre_stride;
      u_dist += dist_stride;
      v_src += uv_src_stride;
      v_pre += uv_pre_stride;
      v_dist += dist_stride;
    }

    sum_row_first = _mm_adds_epu16(sum_row_first, u_first);
    sum_row_second = _mm_adds_epu16(sum_row_second, u_second);
    sum_row_first = _mm_adds_epu16(sum_row_first, v_first);
    sum_row_second = _mm_adds_epu16(sum_row_second, v_second);

    // Get modifier and store result
    average_16(&sum_row_first, &sum_row_second, mul_first, mul_second, strength,
               rounding, weight);
    accumulate_and_store_16(sum_row_first, sum_row_second, y_pre, y_count,
                            y_accum);

    y_src += y_src_stride;
    y_pre += y_pre_stride;
    y_count += y_pre_stride;
    y_accum += y_pre_stride;
    y_dist += dist_stride;
  }

  // The last row
  if (loc == 0) {  // On the left
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
  } else {  // On the right
    mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                               NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

    mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
  }

  // Shift the rows up
  sum_row_1_first = sum_row_2_first;
  sum_row_1_second = sum_row_2_second;
  sum_row_2_first = sum_row_3_first;
  sum_row_2_second = sum_row_3_second;

  // Add luma values to the modifier
  sum_row_first = _mm_adds_epu16(sum_row_1_first, sum_row_2_first);
  sum_row_second = _mm_adds_epu16(sum_row_1_second, sum_row_2_second);

  // Add chroma values to the modifier
  if (ss_y == 0) {
    // Only calculate the new chroma distortion if we are at a pixel that
    // corresponds to a new chroma row
    read_chroma_dist_row_16(ss_x, u_dist, v_dist, &u_first, &u_second, &v_first,
                            &v_second);
  }

  sum_row_first = _mm_adds_epu16(sum_row_first, u_first);
  sum_row_second = _mm_adds_epu16(sum_row_second, u_second);
  sum_row_first = _mm_adds_epu16(sum_row_first, v_first);
  sum_row_second = _mm_adds_epu16(sum_row_second, v_second);

  // Get modifier and store result
  average_16(&sum_row_first, &sum_row_second, mul_first, mul_second, strength,
             rounding, weight);
  accumulate_and_store_16(sum_row_first, sum_row_second, y_pre, y_count,
                          y_accum);
}

// Perform temporal filter for the luma component.
static void vp9_apply_temporal_filter_luma(
    const uint8_t *y_src, int y_src_stride, const uint8_t *y_pre,
    int y_pre_stride, const uint8_t *u_src, const uint8_t *v_src,
    int uv_src_stride, const uint8_t *u_pre, const uint8_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw, int use_32x32,
    uint32_t *y_accum, uint16_t *y_count, const uint16_t *y_dist,
    const uint16_t *u_dist, const uint16_t *v_dist) {
  const unsigned int uv_width = block_width >> ss_x;
  const int *weights = blk_fw;

  vp9_apply_temporal_filter_luma_16(
      y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
      u_pre, v_pre, uv_pre_stride, block_width / 2, block_height, ss_x, ss_y,
      strength, weights, use_32x32, y_accum, y_count, y_dist, u_dist, v_dist,
      0);

  if (!use_32x32) {
    weights = blk_fw + 1;
  }

  vp9_apply_temporal_filter_luma_16(
      y_src + 16, y_src_stride, y_pre + 16, y_pre_stride, u_src + uv_width / 2,
      v_src + uv_width / 2, uv_src_stride, u_pre + uv_width / 2,
      v_pre + uv_width / 2, uv_pre_stride, block_width / 2, block_height, ss_x,
      ss_y, strength, weights, use_32x32, y_accum + 16, y_count + 16,
      y_dist + 16, u_dist + uv_width / 2, v_dist + uv_width / 2, 1);
}

// TODO(chiyotsai@google.com): There a lot of repeated code in this function. It
// is probably better to divide this function into 4 depending on the chroma
// subsampling setting, and use MACRo to generate the functions.

// Apply temporal filter to the chroma components. This performs temporal
// filtering on a chroma block of 16 X uv_height. In the case we are not doing
// chroma subsampling, loc indicates whether the current block if on the left or
// right half of the 32x32 block. loc == 0 indicates left, 1 indicates right.
static void vp9_apply_temporal_filter_chroma_16(
    const uint8_t *y_src, int y_src_stride, const uint8_t *y_pre,
    int y_pre_stride, const uint8_t *u_src, const uint8_t *v_src,
    int uv_src_stride, const uint8_t *u_pre, const uint8_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw, int use_32x32,
    uint32_t *u_accum, uint16_t *u_count, uint32_t *v_accum, uint16_t *v_count,
    const uint16_t *y_dist, const uint16_t *u_dist, const uint16_t *v_dist,
    int loc) {
  const int rounding = (1 << strength) >> 1;
  const int dist_stride = 34;
  int weight_first, weight_second;

  const unsigned int uv_block_height = block_height >> ss_y;

  __m128i mul_first, mul_second;

  __m128i u_sum_row_1_first, u_sum_row_1_second;
  __m128i u_sum_row_2_first, u_sum_row_2_second;
  __m128i u_sum_row_3_first, u_sum_row_3_second;

  __m128i v_sum_row_1_first, v_sum_row_1_second;
  __m128i v_sum_row_2_first, v_sum_row_2_second;
  __m128i v_sum_row_3_first, v_sum_row_3_second;

  __m128i u_sum_row_first, u_sum_row_second;
  __m128i v_sum_row_first, v_sum_row_second;

  (void)block_width;

  assert((ss_x == 0 || ss_x == 1) && (ss_y == 0 || ss_y == 1));
  assert(strength >= 0 && strength <= 6);
  assert(blk_fw[0] >= 0 &&
         (use_32x32 || (blk_fw[1] >= 0 && blk_fw[2] >= 0 && blk_fw[3] >= 0)));
  assert(blk_fw[0] <= 2 &&
         (use_32x32 || (blk_fw[1] <= 2 && blk_fw[2] <= 2 && blk_fw[3] <= 2)));

  if (ss_x == 1) {  // First consider the case where we subsample in x dir
    weight_first = blk_fw[0];
    weight_second = use_32x32 ? blk_fw[0] : blk_fw[1];

    // First row
    if (ss_y == 1) {
      // If chroma is subsampled by 1:2 in each direction, then each chroma
      // pixel corresponds to 4 luma pixels. So we have 4+4 neighbors on the
      // corner, and 6+4 neighbors on the edge.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_8);
    } else {
      // If chroma is subsampled by 1:2 in x direction only, so each chroma
      // pixel corresponds to 2 luma pixels. So we have 4+2 neighbors on the
      // corner, and 6+2 neighbors on the edge.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
    }

    // Add chroma values
    get_sum_16(u_dist, &u_sum_row_2_first, &u_sum_row_2_second);
    get_sum_16(u_dist + dist_stride, &u_sum_row_3_first, &u_sum_row_3_second);

    u_sum_row_first = _mm_adds_epu16(u_sum_row_2_first, u_sum_row_3_first);
    u_sum_row_second = _mm_adds_epu16(u_sum_row_2_second, u_sum_row_3_second);

    get_sum_16(v_dist, &v_sum_row_2_first, &v_sum_row_2_second);
    get_sum_16(v_dist + dist_stride, &v_sum_row_3_first, &v_sum_row_3_second);

    v_sum_row_first = _mm_adds_epu16(v_sum_row_2_first, v_sum_row_3_first);
    v_sum_row_second = _mm_adds_epu16(v_sum_row_2_second, v_sum_row_3_second);

    // Add luma values
    add_32_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                              &v_sum_row_first, &u_sum_row_second,
                              &v_sum_row_second);

    // Get modifier and store result
    u_sum_row_first =
        average_8(u_sum_row_first, mul_first, strength, rounding, weight_first);
    u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                            u_accum);

    v_sum_row_first =
        average_8(v_sum_row_first, mul_first, strength, rounding, weight_first);
    v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                            v_accum);

    u_src += uv_src_stride;
    u_pre += uv_pre_stride;
    u_dist += dist_stride;
    v_src += uv_src_stride;
    v_pre += uv_pre_stride;
    v_dist += dist_stride;
    u_count += uv_pre_stride;
    u_accum += uv_pre_stride;
    v_count += uv_pre_stride;
    v_accum += uv_pre_stride;

    y_src += y_src_stride * (1 + ss_y);
    y_pre += y_pre_stride * (1 + ss_y);
    y_dist += dist_stride * (1 + ss_y);

    // Then all the rows except the last one
    if (ss_y == 1) {
      // If chroma is subsampled by 1:2 in each direction, then each chroma
      // pixel corresponds to 4 luma pixels. So we have 6+4 neighbors on the
      // edge, and 9+4 neighbors in the center.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_13,
                                 NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
                                 NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
                                 NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
                                  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
                                  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_13,
                                  NEIGHBOR_CONSTANT_13, NEIGHBOR_CONSTANT_10);
    } else {
      // If chroma is subsampled by 1:2 in x direction only, so each chroma
      // pixel corresponds to 2 luma pixels. So we have 6+2 neighbors on the
      // edge, and 9+2 neighbors in the center.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_8);
    }

    for (unsigned int h = 1; h < uv_block_height - 1; ++h) {
      // Move the weight pointer to the bottom half of the blocks
      if (!use_32x32 && h == uv_block_height / 2) {
        weight_first = blk_fw[2];
        weight_second = blk_fw[3];
      }

      // Shift the rows up
      u_sum_row_1_first = u_sum_row_2_first;
      u_sum_row_1_second = u_sum_row_2_second;
      u_sum_row_2_first = u_sum_row_3_first;
      u_sum_row_2_second = u_sum_row_3_second;

      v_sum_row_1_first = v_sum_row_2_first;
      v_sum_row_1_second = v_sum_row_2_second;
      v_sum_row_2_first = v_sum_row_3_first;
      v_sum_row_2_second = v_sum_row_3_second;

      // Add chroma values
      u_sum_row_first = _mm_adds_epu16(u_sum_row_1_first, u_sum_row_2_first);
      u_sum_row_second = _mm_adds_epu16(u_sum_row_1_second, u_sum_row_2_second);

      get_sum_16(u_dist + dist_stride, &u_sum_row_3_first, &u_sum_row_3_second);

      u_sum_row_first = _mm_adds_epu16(u_sum_row_first, u_sum_row_3_first);
      u_sum_row_second = _mm_adds_epu16(u_sum_row_second, u_sum_row_3_second);

      v_sum_row_first = _mm_adds_epu16(v_sum_row_1_first, v_sum_row_2_first);
      v_sum_row_second = _mm_adds_epu16(v_sum_row_1_second, v_sum_row_2_second);

      get_sum_16(v_dist + dist_stride, &v_sum_row_3_first, &v_sum_row_3_second);

      v_sum_row_first = _mm_adds_epu16(v_sum_row_first, v_sum_row_3_first);
      v_sum_row_second = _mm_adds_epu16(v_sum_row_second, v_sum_row_3_second);

      // Add luma values
      add_32_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                                &v_sum_row_first, &u_sum_row_second,
                                &v_sum_row_second);

      // Get modifier and store result
      u_sum_row_first = average_8(u_sum_row_first, mul_first, strength,
                                  rounding, weight_first);
      u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                   rounding, weight_second);
      accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                              u_accum);

      v_sum_row_first = average_8(v_sum_row_first, mul_first, strength,
                                  rounding, weight_first);
      v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                   rounding, weight_second);
      accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                              v_accum);

      u_src += uv_src_stride;
      u_pre += uv_pre_stride;
      u_dist += dist_stride;
      v_src += uv_src_stride;
      v_pre += uv_pre_stride;
      v_dist += dist_stride;
      u_count += uv_pre_stride;
      u_accum += uv_pre_stride;
      v_count += uv_pre_stride;
      v_accum += uv_pre_stride;

      y_src += y_src_stride * (1 + ss_y);
      y_pre += y_pre_stride * (1 + ss_y);
      y_dist += dist_stride * (1 + ss_y);
    }

    // The last row
    if (ss_y == 1) {
      // If chroma is subsampled by 1:2 in each direction, then each chroma
      // pixel corresponds to 4 luma pixels. So we have 4+4 neighbors on the
      // corner, and 6+4 neighbors on the edge.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_8);
    } else {
      // If chroma is subsampled by 1:2 in x direction only, so each chroma
      // pixel corresponds to 2 luma pixels. So we have 4+2 neighbors on the
      // corner, and 6+2 neighbors on the edge.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
    }

    // Shift the rows up
    u_sum_row_1_first = u_sum_row_2_first;
    u_sum_row_1_second = u_sum_row_2_second;
    u_sum_row_2_first = u_sum_row_3_first;
    u_sum_row_2_second = u_sum_row_3_second;

    v_sum_row_1_first = v_sum_row_2_first;
    v_sum_row_1_second = v_sum_row_2_second;
    v_sum_row_2_first = v_sum_row_3_first;
    v_sum_row_2_second = v_sum_row_3_second;

    // Add chroma values
    u_sum_row_first = _mm_adds_epu16(u_sum_row_1_first, u_sum_row_2_first);
    u_sum_row_second = _mm_adds_epu16(u_sum_row_1_second, u_sum_row_2_second);

    v_sum_row_first = _mm_adds_epu16(v_sum_row_1_first, v_sum_row_2_first);
    v_sum_row_second = _mm_adds_epu16(v_sum_row_1_second, v_sum_row_2_second);

    // Add luma values
    add_32_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                              &v_sum_row_first, &u_sum_row_second,
                              &v_sum_row_second);

    // Get modifier and store result
    u_sum_row_first =
        average_8(u_sum_row_first, mul_first, strength, rounding, weight_first);
    u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                            u_accum);

    v_sum_row_first =
        average_8(v_sum_row_first, mul_first, strength, rounding, weight_first);
    v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                            v_accum);
  } else {  // We are not subsampling in x dir. Need loc to know where we are
    weight_first = weight_second = !use_32x32 && loc ? blk_fw[1] : blk_fw[0];

    // First row
    if (loc == 0 && ss_y == 1) {
      // On the left and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
    } else if (loc == 1 && ss_y == 1) {
      // On the right and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
    } else if (loc == 0 && ss_y == 0) {
      // On the left and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_5, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
    } else {  // loc == 1 && ss_y == 0
      // On the right and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_5);
    }

    // Add chroma values
    get_sum_16(u_dist, &u_sum_row_2_first, &u_sum_row_2_second);
    get_sum_16(u_dist + dist_stride, &u_sum_row_3_first, &u_sum_row_3_second);

    u_sum_row_first = _mm_adds_epu16(u_sum_row_2_first, u_sum_row_3_first);
    u_sum_row_second = _mm_adds_epu16(u_sum_row_2_second, u_sum_row_3_second);

    get_sum_16(v_dist, &v_sum_row_2_first, &v_sum_row_2_second);
    get_sum_16(v_dist + dist_stride, &v_sum_row_3_first, &v_sum_row_3_second);

    v_sum_row_first = _mm_adds_epu16(v_sum_row_2_first, v_sum_row_3_first);
    v_sum_row_second = _mm_adds_epu16(v_sum_row_2_second, v_sum_row_3_second);

    // Add luma values
    add_16_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                              &v_sum_row_first, &u_sum_row_second,
                              &v_sum_row_second);

    // Get modifier and store result
    u_sum_row_first =
        average_8(u_sum_row_first, mul_first, strength, rounding, weight_first);
    u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                            u_accum);

    v_sum_row_first =
        average_8(v_sum_row_first, mul_first, strength, rounding, weight_first);
    v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                            v_accum);

    u_src += uv_src_stride;
    u_pre += uv_pre_stride;
    u_dist += dist_stride;
    v_src += uv_src_stride;
    v_pre += uv_pre_stride;
    v_dist += dist_stride;
    u_count += uv_pre_stride;
    u_accum += uv_pre_stride;
    v_count += uv_pre_stride;
    v_accum += uv_pre_stride;

    y_src += y_src_stride * (1 + ss_y);
    y_pre += y_pre_stride * (1 + ss_y);
    y_dist += dist_stride * (1 + ss_y);

    // Then all the rows except the last one
    if (loc == 0 && ss_y == 1) {
      // On the left and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
    } else if (loc == 1 && ss_y == 1) {
      // On the right and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                 NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_11,
                                  NEIGHBOR_CONSTANT_11, NEIGHBOR_CONSTANT_8);
    } else if (loc == 0 && ss_y == 0) {
      // On the left and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10);
    } else {  // loc == 1 && ss_y == 0
      // On the right and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                 NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_10,
                                  NEIGHBOR_CONSTANT_10, NEIGHBOR_CONSTANT_7);
    }

    for (unsigned int h = 1; h < uv_block_height - 1; ++h) {
      // Move the weight pointer to the bottom half of the blocks
      if (!use_32x32 && h == uv_block_height / 2) {
        weight_first = weight_second = loc ? blk_fw[3] : blk_fw[2];
      }

      // Shift the rows up
      u_sum_row_1_first = u_sum_row_2_first;
      u_sum_row_1_second = u_sum_row_2_second;
      u_sum_row_2_first = u_sum_row_3_first;
      u_sum_row_2_second = u_sum_row_3_second;

      v_sum_row_1_first = v_sum_row_2_first;
      v_sum_row_1_second = v_sum_row_2_second;
      v_sum_row_2_first = v_sum_row_3_first;
      v_sum_row_2_second = v_sum_row_3_second;

      // Add chroma values
      u_sum_row_first = _mm_adds_epu16(u_sum_row_1_first, u_sum_row_2_first);
      u_sum_row_second = _mm_adds_epu16(u_sum_row_1_second, u_sum_row_2_second);

      get_sum_16(u_dist + dist_stride, &u_sum_row_3_first, &u_sum_row_3_second);

      u_sum_row_first = _mm_adds_epu16(u_sum_row_first, u_sum_row_3_first);
      u_sum_row_second = _mm_adds_epu16(u_sum_row_second, u_sum_row_3_second);

      v_sum_row_first = _mm_adds_epu16(v_sum_row_1_first, v_sum_row_2_first);
      v_sum_row_second = _mm_adds_epu16(v_sum_row_1_second, v_sum_row_2_second);

      get_sum_16(v_dist + dist_stride, &v_sum_row_3_first, &v_sum_row_3_second);

      v_sum_row_first = _mm_adds_epu16(v_sum_row_first, v_sum_row_3_first);
      v_sum_row_second = _mm_adds_epu16(v_sum_row_second, v_sum_row_3_second);

      // Add luma values
      add_16_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                                &v_sum_row_first, &u_sum_row_second,
                                &v_sum_row_second);

      // Get modifier and store result
      u_sum_row_first = average_8(u_sum_row_first, mul_first, strength,
                                  rounding, weight_first);
      u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                   rounding, weight_second);
      accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                              u_accum);

      v_sum_row_first = average_8(v_sum_row_first, mul_first, strength,
                                  rounding, weight_first);
      v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                   rounding, weight_second);
      accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                              v_accum);

      u_src += uv_src_stride;
      u_pre += uv_pre_stride;
      u_dist += dist_stride;
      v_src += uv_src_stride;
      v_pre += uv_pre_stride;
      v_dist += dist_stride;
      u_count += uv_pre_stride;
      u_accum += uv_pre_stride;
      v_count += uv_pre_stride;
      v_accum += uv_pre_stride;

      y_src += y_src_stride * (1 + ss_y);
      y_pre += y_pre_stride * (1 + ss_y);
      y_dist += dist_stride * (1 + ss_y);
    }

    // The last row
    if (loc == 0 && ss_y == 1) {
      // On the left and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_6, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);
    } else if (loc == 1 && ss_y == 1) {
      // On the right and subsampling chroma in y_direction. We get 2
      // extra_neighbors
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                 NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8);

      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_8,
                                  NEIGHBOR_CONSTANT_8, NEIGHBOR_CONSTANT_6);
    } else if (loc == 0 && ss_y == 0) {
      // On the left and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_5, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
    } else {  // loc == 1 && ss_y == 0
      // On the right and no subsampling. We get 1 extra neighbor.
      mul_first = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                 NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7);
      mul_second = _mm_setr_epi16(NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_7,
                                  NEIGHBOR_CONSTANT_7, NEIGHBOR_CONSTANT_5);
    }

    // Shift the rows up
    u_sum_row_1_first = u_sum_row_2_first;
    u_sum_row_1_second = u_sum_row_2_second;
    u_sum_row_2_first = u_sum_row_3_first;
    u_sum_row_2_second = u_sum_row_3_second;

    v_sum_row_1_first = v_sum_row_2_first;
    v_sum_row_1_second = v_sum_row_2_second;
    v_sum_row_2_first = v_sum_row_3_first;
    v_sum_row_2_second = v_sum_row_3_second;

    // Add chroma values
    u_sum_row_first = _mm_adds_epu16(u_sum_row_1_first, u_sum_row_2_first);
    u_sum_row_second = _mm_adds_epu16(u_sum_row_1_second, u_sum_row_2_second);

    v_sum_row_first = _mm_adds_epu16(v_sum_row_1_first, v_sum_row_2_first);
    v_sum_row_second = _mm_adds_epu16(v_sum_row_1_second, v_sum_row_2_second);

    // Add luma values
    add_16_luma_to_chroma_mod(y_dist, dist_stride, ss_y, &u_sum_row_first,
                              &v_sum_row_first, &u_sum_row_second,
                              &v_sum_row_second);

    // Get modifier and store result
    u_sum_row_first =
        average_8(u_sum_row_first, mul_first, strength, rounding, weight_first);
    u_sum_row_second = average_8(u_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(u_sum_row_first, u_sum_row_second, u_pre, u_count,
                            u_accum);

    v_sum_row_first =
        average_8(v_sum_row_first, mul_first, strength, rounding, weight_first);
    v_sum_row_second = average_8(v_sum_row_second, mul_second, strength,
                                 rounding, weight_second);
    accumulate_and_store_16(v_sum_row_first, v_sum_row_second, v_pre, v_count,
                            v_accum);
  }
}

// Perform temporal filter for the chroma components.
static void vp9_apply_temporal_filter_chroma(
    const uint8_t *y_src, int y_src_stride, const uint8_t *y_pre,
    int y_pre_stride, const uint8_t *u_src, const uint8_t *v_src,
    int uv_src_stride, const uint8_t *u_pre, const uint8_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw, int use_32x32,
    uint32_t *u_accum, uint16_t *u_count, uint32_t *v_accum, uint16_t *v_count,
    const uint16_t *y_dist, const uint16_t *u_dist, const uint16_t *v_dist) {
  const unsigned int uv_width = block_width >> ss_x;

  if (!ss_x) {
    vp9_apply_temporal_filter_chroma_16(
        y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
        u_pre, v_pre, uv_pre_stride, block_width / 2, block_height, ss_x, ss_y,
        strength, blk_fw, use_32x32, u_accum, u_count, v_accum, v_count, y_dist,
        u_dist, v_dist, 0);

    vp9_apply_temporal_filter_chroma_16(
        y_src + 16, y_src_stride, y_pre + 16, y_pre_stride,
        u_src + uv_width / 2, v_src + uv_width / 2, uv_src_stride,
        u_pre + uv_width / 2, v_pre + uv_width / 2, uv_pre_stride,
        block_width / 2, block_height, ss_x, ss_y, strength, blk_fw, use_32x32,
        u_accum + 16, u_count + 16, v_accum + 16, v_count + 16, y_dist + 16,
        u_dist + uv_width / 2, v_dist + uv_width / 2, 1);
  } else {
    vp9_apply_temporal_filter_chroma_16(
        y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
        u_pre, v_pre, uv_pre_stride, block_width / 2, block_height, ss_x, ss_y,
        strength, blk_fw, use_32x32, u_accum, u_count, v_accum, v_count, y_dist,
        u_dist, v_dist, 0);
  }
}

void vp9_apply_temporal_filter_sse4_1(
    const uint8_t *y_src, int y_src_stride, const uint8_t *y_pre,
    int y_pre_stride, const uint8_t *u_src, const uint8_t *v_src,
    int uv_src_stride, const uint8_t *u_pre, const uint8_t *v_pre,
    int uv_pre_stride, unsigned int block_width, unsigned int block_height,
    int ss_x, int ss_y, int strength, const int *const blk_fw, int use_32x32,
    uint32_t *y_accum, uint16_t *y_count, uint32_t *u_accum, uint16_t *u_count,
    uint32_t *v_accum, uint16_t *v_count) {
  const int dist_stride = 34;

  DECLARE_ALIGNED(16, uint16_t, y_dist[32 * 34]) = { 0 };
  DECLARE_ALIGNED(16, uint16_t, u_dist[32 * 34]) = { 0 };
  DECLARE_ALIGNED(16, uint16_t, v_dist[32 * 34]) = { 0 };

  uint16_t *y_dist_ptr = y_dist + 1, *u_dist_ptr = u_dist + 1,
           *v_dist_ptr = v_dist + 1;
  const uint8_t *y_src_ptr = y_src, *u_src_ptr = u_src, *v_src_ptr = v_src;
  const uint8_t *y_pre_ptr = y_pre, *u_pre_ptr = u_pre, *v_pre_ptr = v_pre;

  // Precompute the difference sqaured
  for (unsigned int row = 0; row < block_height; row++) {
    store_dist_32(y_src_ptr, y_pre_ptr, y_dist_ptr);

    y_src_ptr += y_src_stride;
    y_pre_ptr += y_pre_stride;
    y_dist_ptr += dist_stride;
  }

  for (unsigned int row = 0; row < (block_height >> ss_y); row++) {
    if (ss_x) {  // ss_x == 1
      store_dist_16(u_src_ptr, u_pre_ptr, u_dist_ptr);
      store_dist_16(v_src_ptr, v_pre_ptr, v_dist_ptr);
    } else {  // ss_x == 0
      store_dist_32(u_src_ptr, u_pre_ptr, u_dist_ptr);
      store_dist_32(v_src_ptr, v_pre_ptr, v_dist_ptr);
    }

    u_src_ptr += uv_src_stride;
    u_pre_ptr += uv_pre_stride;
    u_dist_ptr += dist_stride;
    v_src_ptr += uv_src_stride;
    v_pre_ptr += uv_pre_stride;
    v_dist_ptr += dist_stride;
  }

  y_dist_ptr = y_dist + 1;
  u_dist_ptr = u_dist + 1;
  v_dist_ptr = v_dist + 1;

  vp9_apply_temporal_filter_luma(y_src, y_src_stride, y_pre, y_pre_stride,
                                 u_src, v_src, uv_src_stride, u_pre, v_pre,
                                 uv_pre_stride, block_width, block_height, ss_x,
                                 ss_y, strength, blk_fw, use_32x32, y_accum,
                                 y_count, y_dist_ptr, u_dist_ptr, v_dist_ptr);

  vp9_apply_temporal_filter_chroma(
      y_src, y_src_stride, y_pre, y_pre_stride, u_src, v_src, uv_src_stride,
      u_pre, v_pre, uv_pre_stride, block_width, block_height, ss_x, ss_y,
      strength, blk_fw, use_32x32, u_accum, u_count, v_accum, v_count,
      y_dist_ptr, u_dist_ptr, v_dist_ptr);
}
