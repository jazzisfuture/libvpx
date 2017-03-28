/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <immintrin.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/x86/convolve.h"

// -----------------------------------------------------------------------------
// Copy and average

static INLINE void convolve_copy_row(const uint16_t *src, uint16_t *dst,
                                     int width) {
  assert(width % 4 == 0);

  while (width >= 16) {
    const __m256i p = _mm256_loadu_si256((const __m256i *)src);
    _mm256_storeu_si256((__m256i *)dst, p);
    src += 16;
    dst += 16;
    width -= 16;
  }
  if (width == 8) {
    const __m128i p = _mm_loadu_si128((const __m128i *)src);
    _mm_storeu_si128((__m128i *)dst, p);
  } else {  // (width == 4)
    const __m128i p = _mm_loadl_epi64((const __m128i *)src);
    _mm_storel_epi64((__m128i *)dst, p);
  }
}

#define DO_TWO_ROWS_IS_FASTER (0)
void vpx_highbd_convolve_copy_avx2(const uint8_t *src8, ptrdiff_t src_stride,
                                   uint8_t *dst8, ptrdiff_t dst_stride,
                                   const int16_t *filter_x, int filter_x_stride,
                                   const int16_t *filter_y, int filter_y_stride,
                                   int width, int h, int bd) {
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  (void)filter_x;
  (void)filter_y;
  (void)filter_x_stride;
  (void)filter_y_stride;
  (void)bd;
#if DO_TWO_ROWS_IS_FASTER
  do {
    convolve_copy_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    convolve_copy_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    h -= 2;
  } while (h > 0);
#else
  do {
    convolve_copy_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    h -= 1;
  } while (h > 0);
#endif
}

static INLINE void convolve_avg_row(const uint16_t *src, uint16_t *dst,
                                    int width) {
  assert(width % 4 == 0);

  while (width >= 16) {
    const __m256i p = _mm256_loadu_si256((const __m256i *)src);
    const __m256i u = _mm256_loadu_si256((const __m256i *)dst);
    const __m256i avg = _mm256_avg_epu16(p, u);
    _mm256_storeu_si256((__m256i *)dst, avg);
    src += 16;
    dst += 16;
    width -= 16;
  }
  if (width == 8) {
    const __m128i p = _mm_loadu_si128((const __m128i *)src);
    const __m128i u = _mm_loadu_si128((const __m128i *)dst);
    const __m128i avg = _mm_avg_epu16(p, u);
    _mm_storeu_si128((__m128i *)dst, avg);
  } else {  // (width == 4)
    const __m128i p = _mm_loadl_epi64((const __m128i *)src);
    const __m128i u = _mm_loadl_epi64((const __m128i *)dst);
    const __m128i avg = _mm_avg_epu16(p, u);
    _mm_storel_epi64((__m128i *)dst, avg);
  }
}

void vpx_highbd_convolve_avg_avx2(const uint8_t *src8, ptrdiff_t src_stride,
                                  uint8_t *dst8, ptrdiff_t dst_stride,
                                  const int16_t *filter_x, int filter_x_stride,
                                  const int16_t *filter_y, int filter_y_stride,
                                  int width, int height, int bd) {
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  (void)filter_x;
  (void)filter_y;
  (void)filter_x_stride;
  (void)filter_y_stride;
  (void)bd;
#if DO_TWO_ROWS_IS_FASTER
  do {
    convolve_avg_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    convolve_avg_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    height -= 2;
  } while (height > 0);
#else
  do {
    convolve_avg_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    height -= 1;
  } while (height > 0);
#endif
}
#undef DO_TWO_ROWS_IS_FASTER
