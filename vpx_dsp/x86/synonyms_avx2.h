/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_X86_SYNONYMS_AVX2_H_
#define VPX_VPX_DSP_X86_SYNONYMS_AVX2_H_

#include <immintrin.h>

#include "vpx/vpx_integer.h"

/**
 * Various reusable shorthands for x86 SIMD intrinsics.
 *
 * Intrinsics prefixed with xx_ operate on or return 128bit XMM registers.
 * Intrinsics prefixed with yy_ operate on or return 256bit YMM registers.
 */

// Loads and stores to do away with the tedium of casting the address
// to the right type.
static INLINE __m256i yy_loadu_256(const void *a) {
  return _mm256_loadu_si256((const __m256i *)a);
}

// Some compilers don't have _mm256_set_m128i defined in immintrin.h. We
// therefore define an equivalent function using a different intrinsic.
// ([ hi ], [ lo ]) -> [ hi ][ lo ]
static INLINE __m256i yy_set_m128i(__m128i hi, __m128i lo) {
  return _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
}

static INLINE __m256i yy_loadu2_128(const void *hi, const void *lo) {
  __m128i mhi = _mm_loadu_si128((const __m128i *)(hi));
  __m128i mlo = _mm_loadu_si128((const __m128i *)(lo));
  return yy_set_m128i(mhi, mlo);
}

#endif  // VPX_VPX_DSP_X86_SYNONYMS_AVX2_H_
