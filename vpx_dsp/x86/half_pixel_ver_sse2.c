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

static INLINE int ver_half_pixel4_sse2(const uint8_t *src, const int src_stride,
                                       const int h, uint8_t *dst) {
  const int dst_stride = 4;
  __m128i s, d;
  int i;

  s = _mm_cvtsi32_si128(*(const int *)src);

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    const __m128i s1 = _mm_cvtsi32_si128(*(const int *)(src + 1 * src_stride));
    const __m128i s2 = _mm_cvtsi32_si128(*(const int *)(src + 2 * src_stride));
    const __m128i s3 = _mm_cvtsi32_si128(*(const int *)(src + 3 * src_stride));
    const __m128i s4 = _mm_cvtsi32_si128(*(const int *)(src + 4 * src_stride));
    const __m128i a0 = _mm_avg_epu8(s, s1);
    const __m128i a1 = _mm_avg_epu8(s1, s2);
    const __m128i a2 = _mm_avg_epu8(s2, s3);
    const __m128i a3 = _mm_avg_epu8(s3, s4);
    const __m128i aa0 = _mm_unpacklo_epi32(a0, a1);
    const __m128i aa1 = _mm_unpacklo_epi32(a2, a3);
    d = _mm_unpacklo_epi64(aa0, aa1);
    _mm_store_si128((__m128i *)dst, d);
    s = s4;
  }

  d = _mm_cvtsi32_si128(*(const int *)(src + 1 * src_stride));
  d = _mm_avg_epu8(s, d);
  *(int *)dst = _mm_cvtsi128_si32(d);

  return dst_stride;
}

int vpx_ver_half_pixel4x4_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return ver_half_pixel4_sse2(src, src_stride, 4, dst);
}

int vpx_ver_half_pixel4x8_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return ver_half_pixel4_sse2(src, src_stride, 8, dst);
}

//------------------------------------------------------------------------------

static INLINE __m128i avg8x2(const uint8_t *const src, const int src_stride,
                             const __m128i s0, uint8_t *const dst) {
  const __m128i s1 = _mm_loadl_epi64((const __m128i *)(src + 0 * src_stride));
  const __m128i s2 = _mm_loadl_epi64((const __m128i *)(src + 1 * src_stride));
  const __m128i ss0 = _mm_unpacklo_epi64(s0, s1);
  const __m128i ss1 = _mm_unpacklo_epi64(s1, s2);
  const __m128i d = _mm_avg_epu8(ss0, ss1);
  _mm_store_si128((__m128i *)dst, d);
  return s2;
}

static INLINE int ver_half_pixel8_sse2(const uint8_t *src, const int src_stride,
                                       const int h, uint8_t *dst) {
  const int dst_stride = 8;
  __m128i s, d;
  int i;

  s = _mm_loadl_epi64((const __m128i *)src);

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    s = avg8x2(src + 1 * src_stride, src_stride, s, dst + 0 * dst_stride);
    s = avg8x2(src + 3 * src_stride, src_stride, s, dst + 2 * dst_stride);
  }

  d = _mm_loadl_epi64((const __m128i *)(src + 1 * src_stride));
  d = _mm_avg_epu8(s, d);
  _mm_storel_epi64((__m128i *)dst, d);

  return dst_stride;
}

int vpx_ver_half_pixel8x4_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return ver_half_pixel8_sse2(src, src_stride, 4, dst);
}

int vpx_ver_half_pixel8x8_sse2(const uint8_t *const src, const int src_stride,
                               uint8_t *const dst) {
  return ver_half_pixel8_sse2(src, src_stride, 8, dst);
}

int vpx_ver_half_pixel8x16_sse2(const uint8_t *const src, const int src_stride,
                                uint8_t *const dst) {
  return ver_half_pixel8_sse2(src, src_stride, 16, dst);
}

//------------------------------------------------------------------------------

static INLINE __m128i avg16(const uint8_t *const src, const __m128i s,
                            uint8_t *const dst) {
  const __m128i n = _mm_loadu_si128((const __m128i *)src);
  const __m128i d = _mm_avg_epu8(s, n);
  _mm_store_si128((__m128i *)dst, d);
  return n;
}

static INLINE int ver_half_pixel16_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 16;
  __m128i s;
  int i;

  s = _mm_loadu_si128((const __m128i *)src);

  for (i = 0; i < h; i += 8, src += 8 * src_stride, dst += 8 * dst_stride) {
    s = avg16(src + 1 * src_stride, s, dst + 0 * dst_stride);
    s = avg16(src + 2 * src_stride, s, dst + 1 * dst_stride);
    s = avg16(src + 3 * src_stride, s, dst + 2 * dst_stride);
    s = avg16(src + 4 * src_stride, s, dst + 3 * dst_stride);
    s = avg16(src + 5 * src_stride, s, dst + 4 * dst_stride);
    s = avg16(src + 6 * src_stride, s, dst + 5 * dst_stride);
    s = avg16(src + 7 * src_stride, s, dst + 6 * dst_stride);
    s = avg16(src + 8 * src_stride, s, dst + 7 * dst_stride);
  }

  avg16(src + 1 * src_stride, s, dst);

  return dst_stride;
}

int vpx_ver_half_pixel16x8_sse2(const uint8_t *const src, const int src_stride,
                                uint8_t *const dst) {
  return ver_half_pixel16_sse2(src, src_stride, 8, dst);
}

int vpx_ver_half_pixel16x16_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel16_sse2(src, src_stride, 16, dst);
}

int vpx_ver_half_pixel16x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel16_sse2(src, src_stride, 32, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg32(const uint8_t *const src, uint8_t *const dst,
                         __m128i *const s /*[2]*/) {
  s[0] = avg16(src + 0, s[0], dst + 0);
  s[1] = avg16(src + 16, s[1], dst + 16);
}

static INLINE int ver_half_pixel32_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 32;
  __m128i s[2];
  int i;

  s[0] = _mm_loadu_si128((const __m128i *)(src + 0));
  s[1] = _mm_loadu_si128((const __m128i *)(src + 16));

  for (i = 0; i < h; i += 8, src += 8 * src_stride, dst += 8 * dst_stride) {
    avg32(src + 1 * src_stride, dst + 0 * dst_stride, s);
    avg32(src + 2 * src_stride, dst + 1 * dst_stride, s);
    avg32(src + 3 * src_stride, dst + 2 * dst_stride, s);
    avg32(src + 4 * src_stride, dst + 3 * dst_stride, s);
    avg32(src + 5 * src_stride, dst + 4 * dst_stride, s);
    avg32(src + 6 * src_stride, dst + 5 * dst_stride, s);
    avg32(src + 7 * src_stride, dst + 6 * dst_stride, s);
    avg32(src + 8 * src_stride, dst + 7 * dst_stride, s);
  }

  avg32(src + 1 * src_stride, dst, s);

  return dst_stride;
}

int vpx_ver_half_pixel32x16_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel32_sse2(src, src_stride, 16, dst);
}

int vpx_ver_half_pixel32x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel32_sse2(src, src_stride, 32, dst);
}

int vpx_ver_half_pixel32x64_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel32_sse2(src, src_stride, 64, dst);
}

//------------------------------------------------------------------------------

static INLINE void avg64(const uint8_t *const src, uint8_t *const dst,
                         __m128i *const s /*[4]*/) {
  avg32(src + 0, dst + 0, s + 0);
  avg32(src + 32, dst + 32, s + 2);
}

static INLINE int ver_half_pixel64_sse2(const uint8_t *src,
                                        const int src_stride, const int h,
                                        uint8_t *dst) {
  const int dst_stride = 64;
  __m128i s[4];
  int i;

  s[0] = _mm_loadu_si128((const __m128i *)(src + 0));
  s[1] = _mm_loadu_si128((const __m128i *)(src + 16));
  s[2] = _mm_loadu_si128((const __m128i *)(src + 32));
  s[3] = _mm_loadu_si128((const __m128i *)(src + 48));

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    avg64(src + 1 * src_stride, dst + 0 * dst_stride, s);
    avg64(src + 2 * src_stride, dst + 1 * dst_stride, s);
    avg64(src + 3 * src_stride, dst + 2 * dst_stride, s);
    avg64(src + 4 * src_stride, dst + 3 * dst_stride, s);
  }

  avg64(src + 1 * src_stride, dst, s);

  return dst_stride;
}

int vpx_ver_half_pixel64x32_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel64_sse2(src, src_stride, 32, dst);
}

int vpx_ver_half_pixel64x64_sse2(const uint8_t *const src, const int src_stride,
                                 uint8_t *const dst) {
  return ver_half_pixel64_sse2(src, src_stride, 64, dst);
}
