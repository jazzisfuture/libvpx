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
#include "vpx_dsp/x86/mem_sse2.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"

static INLINE __m128i highbd_avg4x2(const uint16_t *const src,
                                    const int src_stride, const __m128i s0,
                                    uint16_t *const dst, const int dst_stride) {
  const __m128i s1 = _mm_loadl_epi64((const __m128i *)(src + 0 * src_stride));
  const __m128i s2 = _mm_loadl_epi64((const __m128i *)(src + 1 * src_stride));
  const __m128i ss0 = _mm_unpacklo_epi64(s0, s1);
  const __m128i ss1 = _mm_unpacklo_epi64(s1, s2);
  const __m128i d = _mm_avg_epu16(ss0, ss1);
  _mm_storel_epi64((__m128i *)(dst + 0 * dst_stride), d);
  _mm_storeh_epi64((__m128i *)(dst + 1 * dst_stride), d);
  return s2;
}

static INLINE int highbd_ver_half_pixel4_sse2(const uint8_t *const src8,
                                              const int src_stride, const int h,
                                              uint8_t *const dst8) {
  const int dst_stride = 32;
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  __m128i s, d;
  int i;

  s = _mm_loadl_epi64((const __m128i *)src);

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    s = highbd_avg4x2(src + 1 * src_stride, src_stride, s, dst + 0 * dst_stride,
                      dst_stride);
    s = highbd_avg4x2(src + 3 * src_stride, src_stride, s, dst + 2 * dst_stride,
                      dst_stride);
  }

  d = _mm_loadl_epi64((const __m128i *)(src + 1 * src_stride));
  d = _mm_avg_epu16(s, d);
  _mm_storel_epi64((__m128i *)dst, d);

  return dst_stride;
}

int vpx_highbd_ver_half_pixel4x4_sse2(const uint8_t *const src8,
                                      const int src_stride,
                                      uint8_t *const dst8) {
  return highbd_ver_half_pixel4_sse2(src8, src_stride, 4, dst8);
}

int vpx_highbd_ver_half_pixel4x8_sse2(const uint8_t *const src8,
                                      const int src_stride,
                                      uint8_t *const dst8) {
  return highbd_ver_half_pixel4_sse2(src8, src_stride, 8, dst8);
}

//------------------------------------------------------------------------------

static INLINE __m128i highbd_avg8(const uint16_t *const src, const __m128i s0,
                                  uint16_t *const dst) {
  const __m128i s1 = _mm_loadu_si128((const __m128i *)src);
  const __m128i d = _mm_avg_epu16(s0, s1);
  _mm_store_si128((__m128i *)dst, d);
  return s1;
}

static INLINE int highbd_ver_half_pixel8_sse2(const uint8_t *const src8,
                                              const int src_stride, const int h,
                                              uint8_t *const dst8) {
  const int dst_stride = 32;
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  __m128i s;
  int i;

  s = _mm_loadu_si128((const __m128i *)src);

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    s = highbd_avg8(src + 1 * src_stride, s, dst + 0 * dst_stride);
    s = highbd_avg8(src + 2 * src_stride, s, dst + 1 * dst_stride);
    s = highbd_avg8(src + 3 * src_stride, s, dst + 2 * dst_stride);
    s = highbd_avg8(src + 4 * src_stride, s, dst + 3 * dst_stride);
  }

  highbd_avg8(src + 1 * src_stride, s, dst);

  return dst_stride;
}

int vpx_highbd_ver_half_pixel8x4_sse2(const uint8_t *const src8,
                                      const int src_stride,
                                      uint8_t *const dst8) {
  return highbd_ver_half_pixel8_sse2(src8, src_stride, 4, dst8);
}

int vpx_highbd_ver_half_pixel8x8_sse2(const uint8_t *const src8,
                                      const int src_stride,
                                      uint8_t *const dst8) {
  return highbd_ver_half_pixel8_sse2(src8, src_stride, 8, dst8);
}

int vpx_highbd_ver_half_pixel8x16_sse2(const uint8_t *const src8,
                                       const int src_stride,
                                       uint8_t *const dst8) {
  return highbd_ver_half_pixel8_sse2(src8, src_stride, 16, dst8);
}

//------------------------------------------------------------------------------

static INLINE void highbd_avg16(const uint16_t *const src, uint16_t *const dst,
                                __m128i *const s /*[2]*/) {
  s[0] = highbd_avg8(src + 0, s[0], dst + 0);
  s[1] = highbd_avg8(src + 8, s[1], dst + 8);
}

static INLINE int highbd_ver_half_pixel16_sse2(const uint8_t *const src8,
                                               const int src_stride,
                                               const int h,
                                               uint8_t *const dst8) {
  const int dst_stride = 32;
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  __m128i s[2];
  int i;

  s[0] = _mm_loadu_si128((const __m128i *)(src + 0));
  s[1] = _mm_loadu_si128((const __m128i *)(src + 8));

  for (i = 0; i < h; i += 8, src += 8 * src_stride, dst += 8 * dst_stride) {
    highbd_avg16(src + 1 * src_stride, dst + 0 * dst_stride, s);
    highbd_avg16(src + 2 * src_stride, dst + 1 * dst_stride, s);
    highbd_avg16(src + 3 * src_stride, dst + 2 * dst_stride, s);
    highbd_avg16(src + 4 * src_stride, dst + 3 * dst_stride, s);
    highbd_avg16(src + 5 * src_stride, dst + 4 * dst_stride, s);
    highbd_avg16(src + 6 * src_stride, dst + 5 * dst_stride, s);
    highbd_avg16(src + 7 * src_stride, dst + 6 * dst_stride, s);
    highbd_avg16(src + 8 * src_stride, dst + 7 * dst_stride, s);
  }

  highbd_avg16(src + 1 * src_stride, dst, s);

  return dst_stride;
}

int vpx_highbd_ver_half_pixel16x8_sse2(const uint8_t *const src8,
                                       const int src_stride,
                                       uint8_t *const dst8) {
  return highbd_ver_half_pixel16_sse2(src8, src_stride, 8, dst8);
}

int vpx_highbd_ver_half_pixel16x16_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel16_sse2(src8, src_stride, 16, dst8);
}

int vpx_highbd_ver_half_pixel16x32_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel16_sse2(src8, src_stride, 32, dst8);
}

//------------------------------------------------------------------------------

static INLINE void highbd_avg32(const uint16_t *const src, uint16_t *const dst,
                                __m128i *const s /*[4]*/) {
  highbd_avg16(src + 0, dst + 0, s + 0);
  highbd_avg16(src + 16, dst + 16, s + 2);
}

static INLINE int highbd_ver_half_pixel32_sse2(const uint8_t *const src8,
                                               const int src_stride,
                                               const int h,
                                               uint8_t *const dst8) {
  const int dst_stride = 64;
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  __m128i s[4];
  int i;

  s[0] = _mm_loadu_si128((const __m128i *)(src + 0));
  s[1] = _mm_loadu_si128((const __m128i *)(src + 8));
  s[2] = _mm_loadu_si128((const __m128i *)(src + 16));
  s[3] = _mm_loadu_si128((const __m128i *)(src + 24));

  for (i = 0; i < h; i += 4, src += 4 * src_stride, dst += 4 * dst_stride) {
    highbd_avg32(src + 1 * src_stride, dst + 0 * dst_stride, s);
    highbd_avg32(src + 2 * src_stride, dst + 1 * dst_stride, s);
    highbd_avg32(src + 3 * src_stride, dst + 2 * dst_stride, s);
    highbd_avg32(src + 4 * src_stride, dst + 3 * dst_stride, s);
  }

  highbd_avg32(src + 1 * src_stride, dst, s);

  return dst_stride;
}

int vpx_highbd_ver_half_pixel32x16_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel32_sse2(src8, src_stride, 16, dst8);
}

int vpx_highbd_ver_half_pixel32x32_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel32_sse2(src8, src_stride, 32, dst8);
}

int vpx_highbd_ver_half_pixel32x64_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel32_sse2(src8, src_stride, 64, dst8);
}

//------------------------------------------------------------------------------

static INLINE void highbd_avg64(const uint16_t *const src, uint16_t *const dst,
                                __m128i *const s /*[8]*/) {
  highbd_avg32(src + 0, dst + 0, s + 0);
  highbd_avg32(src + 32, dst + 32, s + 4);
}

static INLINE int highbd_ver_half_pixel64_sse2(const uint8_t *const src8,
                                               const int src_stride,
                                               const int h,
                                               uint8_t *const dst8) {
  const int dst_stride = 96;
  const uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  __m128i s[8];
  int i;

  s[0] = _mm_loadu_si128((const __m128i *)(src + 0));
  s[1] = _mm_loadu_si128((const __m128i *)(src + 8));
  s[2] = _mm_loadu_si128((const __m128i *)(src + 16));
  s[3] = _mm_loadu_si128((const __m128i *)(src + 24));
  s[4] = _mm_loadu_si128((const __m128i *)(src + 32));
  s[5] = _mm_loadu_si128((const __m128i *)(src + 40));
  s[6] = _mm_loadu_si128((const __m128i *)(src + 48));
  s[7] = _mm_loadu_si128((const __m128i *)(src + 56));

  for (i = 0; i < h; i += 2, src += 2 * src_stride, dst += 2 * dst_stride) {
    highbd_avg64(src + 1 * src_stride, dst + 0 * dst_stride, s);
    highbd_avg64(src + 2 * src_stride, dst + 1 * dst_stride, s);
  }

  highbd_avg64(src + 1 * src_stride, dst, s);

  return dst_stride;
}

int vpx_highbd_ver_half_pixel64x32_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel64_sse2(src8, src_stride, 32, dst8);
}

int vpx_highbd_ver_half_pixel64x64_sse2(const uint8_t *const src8,
                                        const int src_stride,
                                        uint8_t *const dst8) {
  return highbd_ver_half_pixel64_sse2(src8, src_stride, 64, dst8);
}
