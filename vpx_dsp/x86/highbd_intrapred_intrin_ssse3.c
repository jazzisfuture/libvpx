/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <tmmintrin.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

// -----------------------------------------------------------------------------
/*
; ------------------------------------------
; input: x, y, z, result
;
; trick from pascal
; (x+2y+z+2)>>2 can be calculated as:
; result = avg(x,z)
; result -= xor(x,z) & 1
; result = avg(result,y)
; ------------------------------------------
*/
static INLINE __m128i avg3_epu16(const __m128i *x, const __m128i *y,
                                 const __m128i *z) {
  const __m128i one = _mm_set1_epi16(1);
  const __m128i a = _mm_avg_epu16(*x, *z);
  const __m128i b =
      _mm_subs_epu16(a, _mm_and_si128(_mm_xor_si128(*x, *z), one));
  return _mm_avg_epu16(b, *y);
}

void vpx_highbd_d45_predictor_4x4_ssse3(uint16_t *dst, ptrdiff_t stride,
                                        const uint16_t *above,
                                        const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const __m128i ABCDHHHH = _mm_shufflehi_epi16(ABCDEFGH, 0xff);
  const __m128i HHHHHHHH = _mm_unpackhi_epi64(ABCDHHHH, ABCDHHHH);
  const __m128i BCDEFGHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 2);
  const __m128i CDEFGHHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 4);
  const __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGHH, &CDEFGHHH);
  (void)left;
  (void)bd;
  _mm_storel_epi64((__m128i *)dst, avg3);
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 2));
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 4));
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 6));
}

static INLINE void d45_store_8(uint16_t **dst, const ptrdiff_t stride,
                               __m128i *row, const __m128i *ar) {
  *row = _mm_alignr_epi8(*ar, *row, 2);
  _mm_store_si128((__m128i *)*dst, *row);
  *dst += stride;
}

void vpx_highbd_d45_predictor_8x8_ssse3(uint16_t *dst, ptrdiff_t stride,
                                        const uint16_t *above,
                                        const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const __m128i ABCDHHHH = _mm_shufflehi_epi16(ABCDEFGH, 0xff);
  const __m128i HHHHHHHH = _mm_unpackhi_epi64(ABCDHHHH, ABCDHHHH);
  const __m128i BCDEFGHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 2);
  const __m128i CDEFGHHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 4);
  __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGHH, &CDEFGHHH);
  (void)left;
  (void)bd;
  _mm_store_si128((__m128i *)dst, avg3);
  dst += stride;
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
  d45_store_8(&dst, stride, &avg3, &HHHHHHHH);
}

static INLINE void d45_store_16(uint16_t **dst, const ptrdiff_t stride,
                                __m128i *row_0, __m128i *row_1,
                                const __m128i *ar) {
  *row_0 = _mm_alignr_epi8(*row_1, *row_0, 2);
  *row_1 = _mm_alignr_epi8(*ar, *row_1, 2);
  _mm_store_si128((__m128i *)*dst, *row_0);
  _mm_store_si128((__m128i *)(*dst + 8), *row_1);
  *dst += stride;
}

void vpx_highbd_d45_predictor_16x16_ssse3(uint16_t *dst, ptrdiff_t stride,
                                          const uint16_t *above,
                                          const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const __m128i AR0 = _mm_shufflehi_epi16(A1, 0xff);
  const __m128i AR = _mm_unpackhi_epi64(AR0, AR0);
  const __m128i B0 = _mm_alignr_epi8(A1, A0, 2);
  const __m128i B1 = _mm_alignr_epi8(AR, A1, 2);
  const __m128i C0 = _mm_alignr_epi8(A1, A0, 4);
  const __m128i C1 = _mm_alignr_epi8(AR, A1, 4);
  __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  (void)left;
  (void)bd;
  _mm_store_si128((__m128i *)dst, avg3_0);
  _mm_store_si128((__m128i *)(dst + 8), avg3_1);
  dst += stride;
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
  d45_store_16(&dst, stride, &avg3_0, &avg3_1, &AR);
}

void vpx_highbd_d45_predictor_32x32_ssse3(uint16_t *dst, ptrdiff_t stride,
                                          const uint16_t *above,
                                          const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const __m128i A2 = _mm_load_si128((const __m128i *)(above + 16));
  const __m128i A3 = _mm_load_si128((const __m128i *)(above + 24));
  const __m128i AR0 = _mm_shufflehi_epi16(A3, 0xff);
  const __m128i AR = _mm_unpackhi_epi64(AR0, AR0);
  const __m128i B0 = _mm_alignr_epi8(A1, A0, 2);
  const __m128i B1 = _mm_alignr_epi8(A2, A1, 2);
  const __m128i B2 = _mm_alignr_epi8(A3, A2, 2);
  const __m128i B3 = _mm_alignr_epi8(AR, A3, 2);
  const __m128i C0 = _mm_alignr_epi8(A1, A0, 4);
  const __m128i C1 = _mm_alignr_epi8(A2, A1, 4);
  const __m128i C2 = _mm_alignr_epi8(A3, A2, 4);
  const __m128i C3 = _mm_alignr_epi8(AR, A3, 4);
  __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  __m128i avg3_2 = avg3_epu16(&A2, &B2, &C2);
  __m128i avg3_3 = avg3_epu16(&A3, &B3, &C3);
  int i;
  (void)left;
  (void)bd;
  _mm_store_si128((__m128i *)dst, avg3_0);
  _mm_store_si128((__m128i *)(dst + 8), avg3_1);
  _mm_store_si128((__m128i *)(dst + 16), avg3_2);
  _mm_store_si128((__m128i *)(dst + 24), avg3_3);
  dst += stride;
  for (i = 1; i < 32; ++i) {
    avg3_0 = _mm_alignr_epi8(avg3_1, avg3_0, 2);
    avg3_1 = _mm_alignr_epi8(avg3_2, avg3_1, 2);
    avg3_2 = _mm_alignr_epi8(avg3_3, avg3_2, 2);
    avg3_3 = _mm_alignr_epi8(AR, avg3_3, 2);
    _mm_store_si128((__m128i *)dst, avg3_0);
    _mm_store_si128((__m128i *)(dst + 8), avg3_1);
    _mm_store_si128((__m128i *)(dst + 16), avg3_2);
    _mm_store_si128((__m128i *)(dst + 24), avg3_3);
    dst += stride;
  }
}
