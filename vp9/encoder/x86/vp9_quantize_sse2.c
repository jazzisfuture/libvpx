/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <xmmintrin.h>

#include "vpx/vpx_integer.h"

void vp9_quantize_b_q0_sse2(const int16_t* coeff_ptr,
                            intptr_t count,
                            int skip_block,
                            int16_t* qcoeff_ptr,
                            int16_t* dqcoeff_ptr,
                            uint16_t* eob_ptr,
                            const int16_t* scan,
                            const int16_t* iscan) {
  __m128i zero;
  (void)scan;

  zero = _mm_set1_epi16(0);
  if (!skip_block) {
    __m128i c0, c1;
    __m128i qc0, qc1;
    __m128i z0, z1;
    __m128i nz0, nz1;
    __m128i is0, is1;
    __m128i eob0, eob1;
    c0 = _mm_load_si128((const __m128i*)coeff_ptr);
    c1 = _mm_load_si128((const __m128i*)coeff_ptr + 1);
    qc0 = _mm_srai_epi16(c0, 2);
    qc1 = _mm_srai_epi16(c1, 2);
    _mm_store_si128((__m128i*)dqcoeff_ptr, c0);
    _mm_store_si128((__m128i*)dqcoeff_ptr + 1, c1);
    _mm_store_si128((__m128i*)qcoeff_ptr, qc0);
    _mm_store_si128((__m128i*)qcoeff_ptr + 1, qc1);
    z0 = _mm_cmpeq_epi16(qc0, zero);
    z1 = _mm_cmpeq_epi16(qc1, zero);
    nz0 = _mm_cmpeq_epi16(z0, zero);
    nz1 = _mm_cmpeq_epi16(z1, zero);
    is0 = _mm_load_si128((const __m128i*)iscan);
    is1 = _mm_load_si128((const __m128i*)iscan + 1);
    is0 = _mm_sub_epi16(is0, nz0);
    is1 = _mm_sub_epi16(is1, nz1);
    eob0 = _mm_and_si128(is0, nz0);
    eob1 = _mm_and_si128(is1, nz1);

    eob0 = _mm_max_epi16(eob0, eob1);
    eob1 = _mm_shuffle_epi32(eob0, 0xe);
    eob0 = _mm_max_epi16(eob0, eob1);
    eob1 = _mm_shufflelo_epi16(eob0, 0xe);
    eob0 = _mm_max_epi16(eob0, eob1);
    eob1 = _mm_shufflelo_epi16(eob0, 0x1);
    eob0 = _mm_max_epi16(eob0, eob1);
    *eob_ptr = _mm_extract_epi16(eob0, 1);
  } else {
    _mm_store_si128((__m128i*)dqcoeff_ptr, zero);
    _mm_store_si128((__m128i*)dqcoeff_ptr + 1, zero);
    _mm_store_si128((__m128i*)qcoeff_ptr, zero);
    _mm_store_si128((__m128i*)qcoeff_ptr + 1, zero);
    *eob_ptr = 0;
  }
}
