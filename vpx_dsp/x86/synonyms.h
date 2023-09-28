/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_X86_SYNONYMS_H_
#define VPX_VPX_DSP_X86_SYNONYMS_H_

#include <immintrin.h>
#include <string.h>

#include "vpx/vpx_integer.h"

/**
 * Various reusable shorthands for x86 SIMD intrinsics.
 *
 * Intrinsics prefixed with xx_ operate on or return 128bit XMM registers.
 * Intrinsics prefixed with yy_ operate on or return 256bit YMM registers.
 */

// Loads and stores to do away with the tedium of casting the address
// to the right type.
static INLINE __m128i xx_loadl_32(const void *a) {
  int val;
  memcpy(&val, a, sizeof(val));
  return _mm_cvtsi32_si128(val);
}

static INLINE __m128i xx_loadl_64(const void *a) {
  return _mm_loadl_epi64((const __m128i *)a);
}

static INLINE __m128i xx_loadu_128(const void *a) {
  return _mm_loadu_si128((const __m128i *)a);
}

static INLINE void xx_storel_64(void *const a, const __m128i v) {
  _mm_storel_epi64((__m128i *)a, v);
}

#endif  // VPX_VPX_DSP_X86_SYNONYMS_H_
