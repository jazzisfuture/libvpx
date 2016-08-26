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

//#include "./vp10_rtcd.h"
#include "./vpx_dsp_rtcd.h"

void vpx_fdct16x16_1_avx2(const int16_t *input, tran_low_t *output,
                          int stride) {
  __m256i r0, r1, r2, r3, u0, u1, sum;
  __m256i zero = _mm256_setzero_si256();
  const int16_t *blockBound = input + (stride << 4);

  while (input < blockBound) {
    r0 = _mm256_loadu_si256((__m256i const *)input);
    r1 = _mm256_loadu_si256((__m256i const *)(input + stride));
    r2 = _mm256_loadu_si256((__m256i const *)(input + 2 * stride));
    r3 = _mm256_loadu_si256((__m256i const *)(input + 3 * stride));

    u0 = _mm256_add_epi16(r0, r1);
    u1 = _mm256_add_epi16(r2, r3);
    sum = _mm256_add_epi16(u0, u1);

    input += stride << 2;
  }

  // unpack 16 int16_t into 2x8 int32_t
  u0 = _mm256_unpacklo_epi16(zero, sum);
  u1 = _mm256_unpackhi_epi16(zero, sum);
  u0 = _mm256_srai_epi32(u0, 16);
  u1 = _mm256_srai_epi32(u1, 16);
  sum = _mm256_add_epi32(u0, u1);

  // unpack 8 int32_t into 2x4 int64_t
  u0 = _mm256_unpacklo_epi32(sum, zero);
  u1 = _mm256_unpackhi_epi32(sum, zero);
  sum = _mm256_add_epi64(u0, u1);

  // unpack 4 int64_t into 2x2 int128_t
  u0 = _mm256_unpacklo_epi64(sum, zero);
  u1 = _mm256_unpackhi_epi64(sum, zero);
  sum = _mm256_add_epi64(u0, u1);

  u0 = _mm256_srli_si256(sum, 16);
  sum = _mm256_add_epi64(u0, sum);
  sum = _mm256_srai_epi32(sum, 1);
  output[0] = (tran_low_t)_mm256_extract_epi32(sum, 0);
}

//void vp10_fht16x16_avx2(const int16_t *input, tran_low_t *output, int stride,
//                        int tx_type) {
//}
