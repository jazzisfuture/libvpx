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

// Copy and average
static INLINE void convolve_copy_row(const uint16_t *src, uint16_t *dst,
                                     int width) {
  while (width >= 16) {
    const __m256i p = _mm256_loadu_si256((const __m256i *)src);
    _mm256_storeu_si256((__m256i *)dst, p);
    src += 16;
    dst += 16;
    width -= 16;
  }
  while (width >= 8) {
    const __m128i p = _mm_loadu_si128((const __m128i *)src);
    _mm_storeu_si128((__m128i *)dst, p);
    src += 8;
    dst += 8;
    width -= 8;
  }
  while (width >= 4) {
    const __m128i p = _mm_loadl_epi64((const __m128i *)src);
    _mm_storel_epi64((__m128i *)dst, p);
    src += 4;
    dst += 4;
    width -= 4;
  }
  // Note: width is a multiple of 4
}

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
  do {
    convolve_copy_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    convolve_copy_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    h -= 2;
  } while (h > 1);

  if (h) {
    convolve_copy_row(src, dst, width);
  }
}

static INLINE void convolve_avg_row(const uint16_t *src, uint16_t *dst,
                                    int width) {
  int i = 0;
  while (width >= 16) {
    __m256i p = _mm256_loadu_si256((const __m256i *)src);
    const __m256i u = _mm256_loadu_si256((const __m256i *)dst);
    p = _mm256_avg_epu16(p, u);
    _mm256_storeu_si256((__m256i *)dst, p);
    src += 16;
    dst += 16;
    width -= 16;
  }
  while (width >= 8) {
    __m128i p = _mm_loadu_si128((const __m128i *)src);
    const __m128i u = _mm_loadu_si128((const __m128i *)dst);
    p = _mm_avg_epu16(p, u);
    _mm_storeu_si128((__m128i *)dst, p);
    src += 8;
    dst += 8;
    width -= 8;
  }
  while (width >= 4) {
    __m128i p = _mm_loadl_epi64((const __m128i *)src);
    const __m128i u = _mm_loadl_epi64((const __m128i *)dst);
    p = _mm_avg_epu16(p, u);
    _mm_storel_epi64((__m128i *)dst, p);
    src += 4;
    dst += 4;
    width -= 4;
  }
  while (width) {
    dst[i] = ROUND_POWER_OF_TWO(dst[i] + src[i], 1);
    i++;
    width--;
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
  do {
    convolve_avg_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    convolve_avg_row(src, dst, width);
    src += src_stride;
    dst += dst_stride;
    height -= 2;
  } while (height > 1);
  if (height) {
    convolve_avg_row(src, dst, width);
  }
}
