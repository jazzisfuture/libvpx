/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"

static INLINE void avg5(const uint8_t *const src, uint8_t *const dst) {
  const __m128i s0 = _mm_loadl_epi64((const __m128i *)src);
  const __m128i s1 = _mm_srli_si128(s0, 1);
  const __m128i d0 = _mm_avg_epu8(s0, s1);
  const __m128i d1 = _mm_srli_si128(d0, 4);
  *(int *)dst = _mm_cvtsi128_si32(d0);
  dst[4] = (uint8_t)_mm_cvtsi128_si32(d1);
}

static INLINE int hor_half_pixel5_sse2(const uint8_t *src, const int src_stride,
                                       const int h, uint8_t *dst) {
  const int dst_stride = 32;
  int i;

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    avg5(src + 0 * src_stride, dst + 0 * dst_stride);
    avg5(src + 1 * src_stride, dst + 1 * dst_stride);
    avg5(src + 2 * src_stride, dst + 2 * dst_stride);
    avg5(src + 3 * src_stride, dst + 3 * dst_stride);
  }

  return dst_stride;
}

int vpx_hor_half_pixel4x4_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return hor_half_pixel5_sse2(src, src_stride, 4, dst);
}

int vpx_hor_half_pixel4x8_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return hor_half_pixel5_sse2(src, src_stride, 8, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg9(const uint8_t *const src, uint8_t *const dst) {
  const __m128i s0 = _mm_loadu_si128((const __m128i *)src);
  const __m128i s1 = _mm_srli_si128(s0, 1);
  const __m128i d0 = _mm_avg_epu8(s0, s1);
  const __m128i d1 = _mm_srli_si128(d0, 8);
  _mm_storel_epi64((__m128i *)dst, d0);
  dst[8] = (uint8_t)_mm_cvtsi128_si32(d1);
}

static INLINE int hor_half_pixel9_sse2(const uint8_t *src, const int src_stride,
                                       const int h, uint8_t *dst) {
  const int dst_stride = 32;
  int i;

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    avg9(src + 0 * src_stride, dst + 0 * dst_stride);
    avg9(src + 1 * src_stride, dst + 1 * dst_stride);
    avg9(src + 2 * src_stride, dst + 2 * dst_stride);
    avg9(src + 3 * src_stride, dst + 3 * dst_stride);
  }

  return dst_stride;
}

int vpx_hor_half_pixel8x4_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return hor_half_pixel9_sse2(src, src_stride, 4, dst);
}

int vpx_hor_half_pixel8x8_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return hor_half_pixel9_sse2(src, src_stride, 8, dst);
}

int vpx_hor_half_pixel8x16_sse2(const uint8_t *const src, const int src_stride,
                                uint8_t *const dst) {
  return hor_half_pixel9_sse2(src, src_stride, 16, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg16(const uint8_t *const src, uint8_t *const dst) {
  const __m128i s0 = _mm_loadu_si128((const __m128i *)(src + 0));
  const __m128i s1 = _mm_loadu_si128((const __m128i *)(src + 1));
  const __m128i d0 = _mm_avg_epu8(s0, s1);
  _mm_store_si128((__m128i *)dst, d0);
}

static INLINE void avg17(const uint8_t *const src, uint8_t *const dst) {
  avg16(src, dst);
  dst[16] = ROUND_POWER_OF_TWO(src[16] + src[17], 1);
}

static INLINE int hor_half_pixel17_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 32;
  int i;

  for (i = 0; i < h; i += 8, src += 8 * src_stride, dst += 8 * dst_stride) {
    avg17(src + 0 * src_stride, dst + 0 * dst_stride);
    avg17(src + 1 * src_stride, dst + 1 * dst_stride);
    avg17(src + 2 * src_stride, dst + 2 * dst_stride);
    avg17(src + 3 * src_stride, dst + 3 * dst_stride);
    avg17(src + 4 * src_stride, dst + 4 * dst_stride);
    avg17(src + 5 * src_stride, dst + 5 * dst_stride);
    avg17(src + 6 * src_stride, dst + 6 * dst_stride);
    avg17(src + 7 * src_stride, dst + 7 * dst_stride);
  }

  return dst_stride;
}

int vpx_hor_half_pixel16x8_sse2(const uint8_t *const src, const int src_stride,
                                uint8_t *const dst) {
  return hor_half_pixel17_sse2(src, src_stride, 8, dst);
}

int vpx_hor_half_pixel16x16_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel17_sse2(src, src_stride, 16, dst);
}

int vpx_hor_half_pixel16x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel17_sse2(src, src_stride, 32, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg33(const uint8_t *const src, uint8_t *const dst) {
  avg16(src + 0, dst + 0);
  avg17(src + 16, dst + 16);
}

static INLINE int hor_half_pixel33_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 64;
  int i;

  for (i = 0; i < h; i += 8, src += 8 * src_stride, dst += 8 * dst_stride) {
    avg33(src + 0 * src_stride, dst + 0 * dst_stride);
    avg33(src + 1 * src_stride, dst + 1 * dst_stride);
    avg33(src + 2 * src_stride, dst + 2 * dst_stride);
    avg33(src + 3 * src_stride, dst + 3 * dst_stride);
    avg33(src + 4 * src_stride, dst + 4 * dst_stride);
    avg33(src + 5 * src_stride, dst + 5 * dst_stride);
    avg33(src + 6 * src_stride, dst + 6 * dst_stride);
    avg33(src + 7 * src_stride, dst + 7 * dst_stride);
  }

  return dst_stride;
}

int vpx_hor_half_pixel32x16_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel33_sse2(src, src_stride, 16, dst);
}

int vpx_hor_half_pixel32x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel33_sse2(src, src_stride, 32, dst);
}

int vpx_hor_half_pixel32x64_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel33_sse2(src, src_stride, 64, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg65(const uint8_t *const src, uint8_t *const dst) {
  avg16(src + 0, dst + 0);
  avg16(src + 16, dst + 16);
  avg33(src + 32, dst + 32);
}

static INLINE int hor_half_pixel65_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 96;
  int i;

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    avg65(src + 0 * src_stride, dst + 0 * dst_stride);
    avg65(src + 1 * src_stride, dst + 1 * dst_stride);
    avg65(src + 2 * src_stride, dst + 2 * dst_stride);
    avg65(src + 3 * src_stride, dst + 3 * dst_stride);
  }

  return dst_stride;
}

int vpx_hor_half_pixel64x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel65_sse2(src, src_stride, 32, dst);
}

int vpx_hor_half_pixel64x64_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return hor_half_pixel65_sse2(src, src_stride, 64, dst);
}
