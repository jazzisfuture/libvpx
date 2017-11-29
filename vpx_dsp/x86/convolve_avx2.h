/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_X86_CONVOLVE_AVX2_H_
#define VPX_DSP_X86_CONVOLVE_AVX2_H_

#include <immintrin.h>  // AVX2

#include "./vpx_config.h"
#include "vpx_ports/avx2_compat.h"

static INLINE void shuffle_filter_avx2(const int16_t *const filter,
                                       __m256i *const f) {
  const __m256i f_values =
      MM256_BROADCASTSI128_SI256(_mm_load_si128((const __m128i *)filter));
  // pack and duplicate the filter values
  f[0] = _mm256_shuffle_epi8(f_values, _mm256_set1_epi16(0x0200u));
  f[1] = _mm256_shuffle_epi8(f_values, _mm256_set1_epi16(0x0604u));
  f[2] = _mm256_shuffle_epi8(f_values, _mm256_set1_epi16(0x0a08u));
  f[3] = _mm256_shuffle_epi8(f_values, _mm256_set1_epi16(0x0e0cu));
}

static INLINE __m256i convolve8_16_avx2(const __m256i *const s,
                                        const __m256i *const f) {
  // multiply 2 adjacent elements with the filter and add the result
  const __m256i k_64 = _mm256_set1_epi16(1 << 6);
  const __m256i x0 = _mm256_maddubs_epi16(s[0], f[0]);
  const __m256i x1 = _mm256_maddubs_epi16(s[1], f[1]);
  const __m256i x2 = _mm256_maddubs_epi16(s[2], f[2]);
  const __m256i x3 = _mm256_maddubs_epi16(s[3], f[3]);
  __m256i sum1, sum2;

  // sum the results together, saturating only on the final step
  // adding x0 with x2 and x1 with x3 is the only order that prevents
  // outranges for all filters
  sum1 = _mm256_add_epi16(x0, x2);
  sum2 = _mm256_add_epi16(x1, x3);
  // add the rounding offset early to avoid another saturated add
  sum1 = _mm256_add_epi16(sum1, k_64);
  sum1 = _mm256_adds_epi16(sum1, sum2);
  // round and shift by 7 bit each 16 bit
  sum1 = _mm256_srai_epi16(sum1, 7);
  return sum1;
}

static INLINE __m128i convolve8_8_avx2(const __m256i *const s,
                                       const __m256i *const f) {
  // multiply 2 adjacent elements with the filter and add the result
  const __m128i k_64 = _mm_set1_epi16(1 << 6);
  const __m128i x0 = _mm_maddubs_epi16(_mm256_castsi256_si128(s[0]),
                                       _mm256_castsi256_si128(f[0]));
  const __m128i x1 = _mm_maddubs_epi16(_mm256_castsi256_si128(s[1]),
                                       _mm256_castsi256_si128(f[1]));
  const __m128i x2 = _mm_maddubs_epi16(_mm256_castsi256_si128(s[2]),
                                       _mm256_castsi256_si128(f[2]));
  const __m128i x3 = _mm_maddubs_epi16(_mm256_castsi256_si128(s[3]),
                                       _mm256_castsi256_si128(f[3]));
  __m128i sum1, sum2;

  // sum the results together, saturating only on the final step
  // adding x0 with x2 and x1 with x3 is the only order that prevents
  // outranges for all filters
  sum1 = _mm_add_epi16(x0, x2);
  sum2 = _mm_add_epi16(x1, x3);
  // add the rounding offset early to avoid another saturated add
  sum1 = _mm_add_epi16(sum1, k_64);
  sum1 = _mm_adds_epi16(sum1, sum2);
  // shift by 7 bit each 16 bit
  sum1 = _mm_srai_epi16(sum1, 7);
  return sum1;
}

#endif  // VPX_DSP_X86_CONVOLVE_AVX2_H_
