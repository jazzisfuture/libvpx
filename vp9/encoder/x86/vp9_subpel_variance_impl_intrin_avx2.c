/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <immintrin.h>  // AVX2
#include "vpx_ports/mem.h"
#include "vp9/encoder/vp9_variance.h"

DECLARE_ALIGNED(32, static const uint8_t, bilinear_filters_avx2[512])= {
  16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0,
  16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0,
  15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1,
  15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1,
  14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2,
  14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2,
  13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3,
  13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3,
  12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4,
  12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4,
  11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5,
  11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5,
  10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6,
  10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6,
  9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7,
  9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9,
  7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9, 7, 9,
  6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10,
  6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10, 6, 10,
  5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11,
  5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11, 5, 11,
  4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12,
  4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12, 4, 12,
  3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13,
  3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13, 3, 13,
  2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14,
  2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14, 2, 14,
  1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15,
  1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15, 1, 15};

#define FILTER_X(filter) \
  /* filter the source */ \
  exp_src_lo = _mm256_maddubs_epi16(exp_src_lo, filter); \
  exp_src_hi = _mm256_maddubs_epi16(exp_src_hi, filter); \
  \
  /* add 8 to source */ \
  exp_src_lo = _mm256_add_epi16(exp_src_lo, pw8); \
  exp_src_hi = _mm256_add_epi16(exp_src_hi, pw8); \
  \
  /* divide source by 16 */ \
  exp_src_lo = _mm256_srai_epi16(exp_src_lo, 4); \
  exp_src_hi = _mm256_srai_epi16(exp_src_hi, 4);

#define X_ZERO_Y_ZERO_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst));

#define X_ZERO_Y_HALF_PRE \
  /* load source + next source + destination */ \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) \
                                  (src + src_stride)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  /* average between current and next stride source */ \
  src_reg = _mm256_avg_epu8(src_reg, src_next_reg);

#define X_ZERO_Y_BILIN_PRE \
  /* load current and next source + destination */ \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) \
                                  (src + src_stride)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  \
  /* merge current and next source */ \
  exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg); \
  \
  FILTER_X(filter)

#define X_HALF_Y_ZERO_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  /* average between source and the next byte following source */ \
  src_reg = _mm256_avg_epu8(src_reg, src_next_reg); \

#define X_HALF_Y_HALF_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  /* average between source and the next byte following source */ \
  src_reg = _mm256_avg_epu8(src_reg, src_next_reg); \
  /* average between previous average to current average */ \
  src_avg = _mm256_avg_epu8(src_avg, src_reg); \

#define X_HALF_Y_BILIN_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  /* average between source and the next byte following source */ \
  src_reg = _mm256_avg_epu8(src_reg, src_next_reg); \
  \
  /* merge previous average and current average */ \
  exp_src_lo = _mm256_unpacklo_epi8(src_avg, src_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_avg, src_reg); \
  \
  FILTER_X(filter)

#define X_BILIN_Y_ZERO_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  /* merge current and next stride source */ \
  exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg); \
  FILTER_X(filter)

#define X_BILIN_Y_HALF_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg); \
  FILTER_X(filter) \
  src_reg =  _mm256_packus_epi16(exp_src_lo, exp_src_hi); \
  /* average between previous pack to the current */ \
  src_pack = _mm256_avg_epu8(src_pack, src_reg);

#define X_BILIN_Y_BILIN_PRE \
  src_reg = _mm256_loadu_si256((__m256i const *) (src)); \
  src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1)); \
  dst_reg = _mm256_load_si256((__m256i const *) (dst)); \
  \
  /* merge current and next stride source */ \
  exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg); \
  \
  FILTER_X(xfilter) \
  src_reg = _mm256_packus_epi16(exp_src_lo, exp_src_hi); \
  /* merge previous pack to current pack source */ \
  exp_src_lo = _mm256_unpacklo_epi8(src_pack, src_reg); \
  exp_src_hi = _mm256_unpackhi_epi8(src_pack, src_reg); \
  /* filter the source */ \
  FILTER_X(yfilter)

#define X_ANY_Y_ANY_SUF \
  /* expand each byte to 2 bytes */ \
  exp_dst_lo = _mm256_unpacklo_epi8(dst_reg, zero_reg); \
  exp_dst_hi = _mm256_unpackhi_epi8(dst_reg, zero_reg); \
  /* source - dest */ \
  exp_src_lo = _mm256_sub_epi16(exp_src_lo, exp_dst_lo); \
  exp_src_hi = _mm256_sub_epi16(exp_src_hi, exp_dst_hi); \
  /* caculate sum */ \
  sum_reg = _mm256_add_epi16(sum_reg, exp_src_lo); \
  exp_src_lo = _mm256_madd_epi16(exp_src_lo, exp_src_lo); \
  sum_reg = _mm256_add_epi16(sum_reg, exp_src_hi); \
  exp_src_hi = _mm256_madd_epi16(exp_src_hi, exp_src_hi); \
  /* calculate sse */ \
  sse_reg = _mm256_add_epi32(sse_reg, exp_src_lo); \
  sse_reg = _mm256_add_epi32(sse_reg, exp_src_hi);

// calculate sum and sse
// expand sum to 16 bytes
// add each 8 bytes in the sum
// add each 4 bytes in the sse
#define CALC_SUM_AND_SSE \
  res_cmp = _mm256_cmpgt_epi16(zero_reg, sum_reg); \
  sse_reg_hi = _mm256_srli_si256(sse_reg, 8); \
  sum_reg_lo = _mm256_unpacklo_epi16(sum_reg, res_cmp); \
  sum_reg_hi = _mm256_unpackhi_epi16(sum_reg, res_cmp); \
  sse_reg = _mm256_add_epi32(sse_reg, sse_reg_hi); \
  sum_reg = _mm256_add_epi32(sum_reg_lo, sum_reg_hi); \
  \
  sse_reg_hi = _mm256_srli_si256(sse_reg, 4); \
  sum_reg_hi = _mm256_srli_si256(sum_reg, 8); \
  \
  sse_reg = _mm256_add_epi32(sse_reg, sse_reg_hi); \
  sum_reg = _mm256_add_epi32(sum_reg, sum_reg_hi); \
  *((int*)sse)= _mm_cvtsi128_si32(_mm256_castsi256_si128(sse_reg)) + \
                _mm_cvtsi128_si32(_mm256_extractf128_si256(sse_reg, 1)); \
  sum_reg_hi = _mm256_srli_si256(sum_reg, 4); \
  sum_reg = _mm256_add_epi32(sum_reg, sum_reg_hi); \
  sum = _mm_cvtsi128_si32(_mm256_castsi256_si128(sum_reg)) + \
        _mm_cvtsi128_si32(_mm256_extractf128_si256(sum_reg, 1));


unsigned int vp9_sub_pixel_variance32xh_avx2(const uint8_t *src,
                                             int src_stride,
                                             int x_offset,
                                             int y_offset,
                                             const uint8_t *dst,
                                             int dst_stride,
                                             int height,
                                             unsigned int *sse) {
  __m256i src_reg, dst_reg, exp_src_lo, exp_src_hi, exp_dst_lo, exp_dst_hi;
  __m256i sse_reg, sum_reg, sse_reg_hi, res_cmp, sum_reg_lo, sum_reg_hi;
  __m256i zero_reg;
  int i, sum;
  sum_reg = _mm256_set1_epi16(0);
  sse_reg = _mm256_set1_epi16(0);
  zero_reg = _mm256_set1_epi16(0);

  // x_offset = 0 and y_offset = 0
  if (x_offset == 0) {
    if (y_offset == 0) {
      for (i = 0; i < height ; i++) {
        // load source and destination
        X_ZERO_Y_ZERO_PRE
        // expend each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = 0 and y_offset = 8
    } else if (y_offset == 8) {
      __m256i src_next_reg;
      for (i = 0; i < height ; i++) {
        X_ZERO_Y_HALF_PRE
        // expend each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = 0 and y_offset = bilin interpolation
    } else {
      __m256i filter, pw8, src_next_reg;

      y_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      for (i = 0; i < height ; i++) {
        X_ZERO_Y_BILIN_PRE
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    }
  // x_offset = 8  and y_offset = 0
  } else if (x_offset == 8) {
    if (y_offset == 0) {
      __m256i src_next_reg;
      for (i = 0; i < height ; i++) {
        X_HALF_Y_ZERO_PRE
        // expand each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = 8  and y_offset = 8
    } else if (y_offset == 8) {
      __m256i src_next_reg, src_avg;
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // average between source and the next byte following source
      src_avg = _mm256_avg_epu8(src_reg, src_next_reg);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_HALF_Y_HALF_PRE
        // expand each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_avg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_avg, zero_reg);
        // save current source average
        src_avg = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    // x_offset = 8  and y_offset = bilin interpolation
    } else {
      __m256i filter, pw8, src_next_reg, src_avg;
      y_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // average between source and the next byte following source
      src_avg = _mm256_avg_epu8(src_reg, src_next_reg);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_HALF_Y_BILIN_PRE
        // save current source average
        src_avg = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    }
  // x_offset = bilin interpolation and y_offset = 0
  } else {
    if (y_offset == 0) {
      __m256i filter, pw8, src_next_reg;
      x_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + x_offset));
      pw8 = _mm256_set1_epi16(8);
      for (i = 0; i < height ; i++) {
        X_BILIN_Y_ZERO_PRE
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = bilin interpolation and y_offset = 8
    } else if (y_offset == 8) {
      __m256i filter, pw8, src_next_reg, src_pack;
      x_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + x_offset));
      pw8 = _mm256_set1_epi16(8);
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // merge current and next stride source
      exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg);
      exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg);
      FILTER_X(filter)
      // convert each 16 bit to 8 bit to each low and high lane source
      src_pack =  _mm256_packus_epi16(exp_src_lo, exp_src_hi);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_BILIN_Y_HALF_PRE
        exp_src_lo = _mm256_unpacklo_epi8(src_pack, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_pack, zero_reg);
        src_pack = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    // x_offset = bilin interpolation and y_offset = bilin interpolation
    } else {
      __m256i xfilter, yfilter, pw8, src_next_reg, src_pack;
      x_offset <<= 5;
      xfilter = _mm256_load_si256((__m256i const *)
                (bilinear_filters_avx2 + x_offset));
      y_offset <<= 5;
      yfilter = _mm256_load_si256((__m256i const *)
                (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));

      // merge current and next stride source
      exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg);
      exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg);

      FILTER_X(xfilter)
      // convert each 16 bit to 8 bit to each low and high lane source
      src_pack = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_BILIN_Y_BILIN_PRE
        src_pack = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    }
  }
  CALC_SUM_AND_SSE
  return sum;
}

unsigned int vp9_sub_pixel_avg_variance32xh_avx2(const uint8_t *src,
                                             int src_stride,
                                             int x_offset,
                                             int y_offset,
                                             const uint8_t *dst,
                                             int dst_stride,
                                             const uint8_t *sec,
                                             int sec_stride,
                                             int height,
                                             unsigned int *sse) {
  __m256i sec_reg;
  __m256i src_reg, dst_reg, exp_src_lo, exp_src_hi, exp_dst_lo, exp_dst_hi;
  __m256i sse_reg, sum_reg, sse_reg_hi, res_cmp, sum_reg_lo, sum_reg_hi;
  __m256i zero_reg;
  int i, sum;
  sum_reg = _mm256_set1_epi16(0);
  sse_reg = _mm256_set1_epi16(0);
  zero_reg = _mm256_set1_epi16(0);

  // x_offset = 0 and y_offset = 0
  if (x_offset == 0) {
    if (y_offset == 0) {
      for (i = 0; i < height ; i++) {
        // load source and destination
        X_ZERO_Y_ZERO_PRE
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_reg = _mm256_avg_epu8(src_reg, sec_reg);
        sec+= sec_stride;
        // expend each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    } else if (y_offset == 8) {
      __m256i src_next_reg;
      for (i = 0; i < height ; i++) {
        X_ZERO_Y_HALF_PRE
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_reg = _mm256_avg_epu8(src_reg, sec_reg);
        sec+= sec_stride;
        // expend each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = 0 and y_offset = bilin interpolation
    } else {
      __m256i filter, pw8, src_next_reg;

      y_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
                 (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      for (i = 0; i < height ; i++) {
        X_ZERO_Y_BILIN_PRE
        src_reg = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_reg = _mm256_avg_epu8(src_reg, sec_reg);
        sec+= sec_stride;
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    }
  // x_offset = 8  and y_offset = 0
  } else if (x_offset == 8) {
    if (y_offset == 0) {
      __m256i src_next_reg;
      for (i = 0; i < height ; i++) {
        X_HALF_Y_ZERO_PRE
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_reg = _mm256_avg_epu8(src_reg, sec_reg);
        sec+= sec_stride;
        // expand each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = 8  and y_offset = 8
    } else if (y_offset == 8) {
      __m256i src_next_reg, src_avg;
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // average between source and the next byte following source
      src_avg = _mm256_avg_epu8(src_reg, src_next_reg);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_HALF_Y_HALF_PRE
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_avg = _mm256_avg_epu8(src_avg, sec_reg);
        sec+= sec_stride;
        // expand each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_avg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_avg, zero_reg);
        // save current source average
        src_avg = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    // x_offset = 8  and y_offset = bilin interpolation
    } else {
      __m256i filter, pw8, src_next_reg, src_avg;
      y_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // average between source and the next byte following source
      src_avg = _mm256_avg_epu8(src_reg, src_next_reg);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_HALF_Y_BILIN_PRE
        src_avg = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_avg = _mm256_avg_epu8(src_avg, sec_reg);
        // expand each byte to 2 bytes
        exp_src_lo = _mm256_unpacklo_epi8(src_avg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_avg, zero_reg);
        sec+= sec_stride;
        // save current source average
        src_avg = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    }
  // x_offset = bilin interpolation and y_offset = 0
  } else {
    if (y_offset == 0) {
      __m256i filter, pw8, src_next_reg;
      x_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + x_offset));
      pw8 = _mm256_set1_epi16(8);
      for (i = 0; i < height ; i++) {
        X_BILIN_Y_ZERO_PRE
        src_reg = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_reg = _mm256_avg_epu8(src_reg, sec_reg);
        exp_src_lo = _mm256_unpacklo_epi8(src_reg, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_reg, zero_reg);
        sec+= sec_stride;
        X_ANY_Y_ANY_SUF
        src+= src_stride;
        dst+= dst_stride;
      }
    // x_offset = bilin interpolation and y_offset = 8
    } else if (y_offset == 8) {
      __m256i filter, pw8, src_next_reg, src_pack;
      x_offset <<= 5;
      filter = _mm256_load_si256((__m256i const *)
               (bilinear_filters_avx2 + x_offset));
      pw8 = _mm256_set1_epi16(8);
       src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));
      // merge current and next stride source
      exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg);
      exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg);
      FILTER_X(filter)
      // convert each 16 bit to 8 bit to each low and high lane source
      src_pack =  _mm256_packus_epi16(exp_src_lo, exp_src_hi);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_BILIN_Y_HALF_PRE
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_pack = _mm256_avg_epu8(src_pack, sec_reg);
        sec+= sec_stride;
        exp_src_lo = _mm256_unpacklo_epi8(src_pack, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_pack, zero_reg);
        src_pack = src_reg;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    // x_offset = bilin interpolation and y_offset = bilin interpolation
    } else {
      __m256i xfilter, yfilter, pw8, src_next_reg, src_pack;
      x_offset <<= 5;
      xfilter = _mm256_load_si256((__m256i const *)
                (bilinear_filters_avx2 + x_offset));
      y_offset <<= 5;
      yfilter = _mm256_load_si256((__m256i const *)
                (bilinear_filters_avx2 + y_offset));
      pw8 = _mm256_set1_epi16(8);
      // load source and another source starting from the next
      // following byte
      src_reg = _mm256_loadu_si256((__m256i const *) (src));
      src_next_reg = _mm256_loadu_si256((__m256i const *) (src + 1));

      // merge current and next stride source
      exp_src_lo = _mm256_unpacklo_epi8(src_reg, src_next_reg);
      exp_src_hi = _mm256_unpackhi_epi8(src_reg, src_next_reg);

      FILTER_X(xfilter)
      // convert each 16 bit to 8 bit to each low and high lane source
      src_pack = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
      for (i = 0; i < height ; i++) {
        src+= src_stride;
        X_BILIN_Y_BILIN_PRE
        src_pack = _mm256_packus_epi16(exp_src_lo, exp_src_hi);
        sec_reg = _mm256_load_si256((__m256i const *) (sec));
        src_pack = _mm256_avg_epu8(src_pack, sec_reg);
        exp_src_lo = _mm256_unpacklo_epi8(src_pack, zero_reg);
        exp_src_hi = _mm256_unpackhi_epi8(src_pack, zero_reg);
        src_pack = src_reg;
        sec+= sec_stride;
        X_ANY_Y_ANY_SUF
        dst+= dst_stride;
      }
    }
  }
  CALC_SUM_AND_SSE
  return sum;
}
