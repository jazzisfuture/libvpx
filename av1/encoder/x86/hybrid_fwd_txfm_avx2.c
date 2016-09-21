/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include <immintrin.h>  // avx2

#include "./av1_rtcd.h"
#include "./aom_dsp_rtcd.h"

#include "aom_dsp/txfm_common.h"
//  #include "aom_dsp/x86/txfm_common_avx2.h"

static INLINE void mm256_reverse_epi16(__m256i *u) {
  const __m256i control = _mm256_set_epi16(0x0100, 0x0302, 0x0504, 0x0706,
                                           0x0908, 0x0B0A, 0x0D0C, 0x0F0E,
                                           0x0100, 0x0302, 0x0504, 0x0706,
                                           0x0908, 0x0B0A, 0x0D0C, 0x0F0E);
  __m256i v = _mm256_shuffle_epi8(*u, control);
  *u = _mm256_permute2x128_si256(v, v, 1);
}

void aom_fdct16x16_1_avx2(const int16_t *input, tran_low_t *output,
                          int stride) {
  __m256i r0, r1, r2, r3, u0, u1;
  __m256i zero = _mm256_setzero_si256();
  __m256i sum = _mm256_setzero_si256();
  const int16_t *blockBound = input + (stride << 4);
  __m128i v0, v1;

  while (input < blockBound) {
    r0 = _mm256_loadu_si256((__m256i const *)input);
    r1 = _mm256_loadu_si256((__m256i const *)(input + stride));
    r2 = _mm256_loadu_si256((__m256i const *)(input + 2 * stride));
    r3 = _mm256_loadu_si256((__m256i const *)(input + 3 * stride));

    u0 = _mm256_add_epi16(r0, r1);
    u1 = _mm256_add_epi16(r2, r3);
    sum = _mm256_add_epi16(sum, u0);
    sum = _mm256_add_epi16(sum, u1);

    input += stride << 2;
  }

  // unpack 16 int16_t into 2x8 int32_t
  u0 = _mm256_unpacklo_epi16(zero, sum);
  u1 = _mm256_unpackhi_epi16(zero, sum);
  u0 = _mm256_srai_epi32(u0, 16);
  u1 = _mm256_srai_epi32(u1, 16);
  sum = _mm256_add_epi32(u0, u1);

  u0 = _mm256_srli_si256(sum, 8);
  u1 = _mm256_add_epi32(sum, u0);

  v0 = _mm_add_epi32(_mm256_extracti128_si256(u1, 1),
                     _mm256_castsi256_si128(u1));
  v1 = _mm_srli_si128(v0, 4);
  v0 = _mm_add_epi32(v0, v1);
  v0 = _mm_srai_epi32(v0, 1);

  output[0] = (tran_low_t)_mm_extract_epi32(v0, 0);
}
