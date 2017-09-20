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

static INLINE void d207_store_4x8(uint16_t **dst, const ptrdiff_t stride,
                                  const __m128i *a, const __m128i *b) {
  _mm_store_si128((__m128i *)*dst, *a);
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 4));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 8));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 12));
  *dst += stride;
}

void vpx_highbd_d207_predictor_8x8_ssse3(uint16_t *dst, ptrdiff_t stride,
                                         const uint16_t *above,
                                         const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)left);
  const __m128i ABCDHHHH = _mm_shufflehi_epi16(ABCDEFGH, 0xff);
  const __m128i HHHHHHHH = _mm_unpackhi_epi64(ABCDHHHH, ABCDHHHH);
  const __m128i BCDEFGHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 2);
  const __m128i CDEFGHHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 4);
  const __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGHH, &CDEFGHHH);
  const __m128i avg2 = _mm_avg_epu16(ABCDEFGH, BCDEFGHH);
  const __m128i out_a = _mm_unpacklo_epi16(avg2, avg3);
  const __m128i out_b = _mm_unpackhi_epi16(avg2, avg3);
  (void)above;
  (void)bd;
  d207_store_4x8(&dst, stride, &out_a, &out_b);
  d207_store_4x8(&dst, stride, &out_b, &HHHHHHHH);
}

static INLINE void d207_store_4x16(uint16_t **dst, const ptrdiff_t stride,
                                   const __m128i *a, const __m128i *b,
                                   const __m128i *c) {
  _mm_store_si128((__m128i *)*dst, *a);
  _mm_store_si128((__m128i *)(*dst + 8), *b);
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 4));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 4));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 8));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 8));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 12));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 12));
  *dst += stride;
}

void vpx_highbd_d207_predictor_16x16_ssse3(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)left);
  const __m128i A1 = _mm_load_si128((const __m128i *)(left + 8));
  const __m128i LR0 = _mm_shufflehi_epi16(A1, 0xff);
  const __m128i LR = _mm_unpackhi_epi64(LR0, LR0);
  const __m128i B0 = _mm_alignr_epi8(A1, A0, 2);
  const __m128i B1 = _mm_alignr_epi8(LR, A1, 2);
  const __m128i C0 = _mm_alignr_epi8(A1, A0, 4);
  const __m128i C1 = _mm_alignr_epi8(LR, A1, 4);
  const __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  const __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  const __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  const __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  const __m128i out_a = _mm_unpacklo_epi16(avg2_0, avg3_0);
  const __m128i out_b = _mm_unpackhi_epi16(avg2_0, avg3_0);
  const __m128i out_c = _mm_unpacklo_epi16(avg2_1, avg3_1);
  const __m128i out_d = _mm_unpackhi_epi16(avg2_1, avg3_1);
  (void)above;
  (void)bd;
  d207_store_4x16(&dst, stride, &out_a, &out_b, &out_c);
  d207_store_4x16(&dst, stride, &out_b, &out_c, &out_d);
  d207_store_4x16(&dst, stride, &out_c, &out_d, &LR);
  d207_store_4x16(&dst, stride, &out_d, &LR, &LR);
}

static INLINE void d207_store_4x32(uint16_t **dst, const ptrdiff_t stride,
                                   const __m128i *a, const __m128i *b,
                                   const __m128i *c, const __m128i *d,
                                   const __m128i *e) {
  _mm_store_si128((__m128i *)*dst, *a);
  _mm_store_si128((__m128i *)(*dst + 8), *b);
  _mm_store_si128((__m128i *)(*dst + 16), *c);
  _mm_store_si128((__m128i *)(*dst + 24), *d);
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 4));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 4));
  _mm_store_si128((__m128i *)(*dst + 16), _mm_alignr_epi8(*d, *c, 4));
  _mm_store_si128((__m128i *)(*dst + 24), _mm_alignr_epi8(*e, *d, 4));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 8));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 8));
  _mm_store_si128((__m128i *)(*dst + 16), _mm_alignr_epi8(*d, *c, 8));
  _mm_store_si128((__m128i *)(*dst + 24), _mm_alignr_epi8(*e, *d, 8));
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, _mm_alignr_epi8(*b, *a, 12));
  _mm_store_si128((__m128i *)(*dst + 8), _mm_alignr_epi8(*c, *b, 12));
  _mm_store_si128((__m128i *)(*dst + 16), _mm_alignr_epi8(*d, *c, 12));
  _mm_store_si128((__m128i *)(*dst + 24), _mm_alignr_epi8(*e, *d, 12));
  *dst += stride;
}

void vpx_highbd_d207_predictor_32x32_ssse3(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)left);
  const __m128i A1 = _mm_load_si128((const __m128i *)(left + 8));
  const __m128i A2 = _mm_load_si128((const __m128i *)(left + 16));
  const __m128i A3 = _mm_load_si128((const __m128i *)(left + 24));
  const __m128i LR0 = _mm_shufflehi_epi16(A3, 0xff);
  const __m128i LR = _mm_unpackhi_epi64(LR0, LR0);
  const __m128i B0 = _mm_alignr_epi8(A1, A0, 2);
  const __m128i B1 = _mm_alignr_epi8(A2, A1, 2);
  const __m128i B2 = _mm_alignr_epi8(A3, A2, 2);
  const __m128i B3 = _mm_alignr_epi8(LR, A3, 2);
  const __m128i C0 = _mm_alignr_epi8(A1, A0, 4);
  const __m128i C1 = _mm_alignr_epi8(A2, A1, 4);
  const __m128i C2 = _mm_alignr_epi8(A3, A2, 4);
  const __m128i C3 = _mm_alignr_epi8(LR, A3, 4);
  const __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  const __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  const __m128i avg3_2 = avg3_epu16(&A2, &B2, &C2);
  const __m128i avg3_3 = avg3_epu16(&A3, &B3, &C3);
  const __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  const __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  const __m128i avg2_2 = _mm_avg_epu16(A2, B2);
  const __m128i avg2_3 = _mm_avg_epu16(A3, B3);
  const __m128i out_a = _mm_unpacklo_epi16(avg2_0, avg3_0);
  const __m128i out_b = _mm_unpackhi_epi16(avg2_0, avg3_0);
  const __m128i out_c = _mm_unpacklo_epi16(avg2_1, avg3_1);
  const __m128i out_d = _mm_unpackhi_epi16(avg2_1, avg3_1);
  const __m128i out_e = _mm_unpacklo_epi16(avg2_2, avg3_2);
  const __m128i out_f = _mm_unpackhi_epi16(avg2_2, avg3_2);
  const __m128i out_g = _mm_unpacklo_epi16(avg2_3, avg3_3);
  const __m128i out_h = _mm_unpackhi_epi16(avg2_3, avg3_3);
  (void)above;
  (void)bd;
  d207_store_4x32(&dst, stride, &out_a, &out_b, &out_c, &out_d, &out_e);
  d207_store_4x32(&dst, stride, &out_b, &out_c, &out_d, &out_e, &out_f);
  d207_store_4x32(&dst, stride, &out_c, &out_d, &out_e, &out_f, &out_g);
  d207_store_4x32(&dst, stride, &out_d, &out_e, &out_f, &out_g, &out_h);
  d207_store_4x32(&dst, stride, &out_e, &out_f, &out_g, &out_h, &LR);
  d207_store_4x32(&dst, stride, &out_f, &out_g, &out_h, &LR, &LR);
  d207_store_4x32(&dst, stride, &out_g, &out_h, &LR, &LR, &LR);
  d207_store_4x32(&dst, stride, &out_h, &LR, &LR, &LR, &LR);
}

static INLINE void d63_store_4x8(uint16_t **dst, const ptrdiff_t stride,
                                 __m128i *a, __m128i *b, const __m128i *ar) {
  _mm_store_si128((__m128i *)*dst, *a);
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, *b);
  *dst += stride;
  *a = _mm_alignr_epi8(*ar, *a, 2);
  *b = _mm_alignr_epi8(*ar, *b, 2);
  _mm_store_si128((__m128i *)*dst, *a);
  *dst += stride;
  _mm_store_si128((__m128i *)*dst, *b);
  *dst += stride;
  *a = _mm_alignr_epi8(*ar, *a, 2);
  *b = _mm_alignr_epi8(*ar, *b, 2);
}

void vpx_highbd_d63_predictor_8x8_ssse3(uint16_t *dst, ptrdiff_t stride,
                                        const uint16_t *above,
                                        const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const __m128i ABCDHHHH = _mm_shufflehi_epi16(ABCDEFGH, 0xff);
  const __m128i HHHHHHHH = _mm_unpackhi_epi64(ABCDHHHH, ABCDHHHH);
  const __m128i BCDEFGHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 2);
  const __m128i CDEFGHHH = _mm_alignr_epi8(HHHHHHHH, ABCDEFGH, 4);
  __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGHH, &CDEFGHHH);
  __m128i avg2 = _mm_avg_epu16(ABCDEFGH, BCDEFGHH);
  (void)left;
  (void)bd;
  d63_store_4x8(&dst, stride, &avg2, &avg3, &HHHHHHHH);
  d63_store_4x8(&dst, stride, &avg2, &avg3, &HHHHHHHH);
}

void vpx_highbd_d63_predictor_16x16_ssse3(uint16_t *dst, ptrdiff_t stride,
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
  __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  int i;
  (void)left;
  (void)bd;
  for (i = 0; i < 14; i += 2) {
    _mm_store_si128((__m128i *)dst, avg2_0);
    _mm_store_si128((__m128i *)(dst + 8), avg2_1);
    dst += stride;
    _mm_store_si128((__m128i *)dst, avg3_0);
    _mm_store_si128((__m128i *)(dst + 8), avg3_1);
    dst += stride;
    avg2_0 = _mm_alignr_epi8(avg2_1, avg2_0, 2);
    avg2_1 = _mm_alignr_epi8(AR, avg2_1, 2);
    avg3_0 = _mm_alignr_epi8(avg3_1, avg3_0, 2);
    avg3_1 = _mm_alignr_epi8(AR, avg3_1, 2);
  }
  _mm_store_si128((__m128i *)dst, avg2_0);
  _mm_store_si128((__m128i *)(dst + 8), avg2_1);
  dst += stride;
  _mm_store_si128((__m128i *)dst, avg3_0);
  _mm_store_si128((__m128i *)(dst + 8), avg3_1);
}

void vpx_highbd_d63_predictor_32x32_ssse3(uint16_t *dst, ptrdiff_t stride,
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
  __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  __m128i avg2_2 = _mm_avg_epu16(A2, B2);
  __m128i avg2_3 = _mm_avg_epu16(A3, B3);
  int i;
  (void)left;
  (void)bd;
  for (i = 0; i < 30; i += 2) {
    _mm_store_si128((__m128i *)dst, avg2_0);
    _mm_store_si128((__m128i *)(dst + 8), avg2_1);
    _mm_store_si128((__m128i *)(dst + 16), avg2_2);
    _mm_store_si128((__m128i *)(dst + 24), avg2_3);
    dst += stride;
    _mm_store_si128((__m128i *)dst, avg3_0);
    _mm_store_si128((__m128i *)(dst + 8), avg3_1);
    _mm_store_si128((__m128i *)(dst + 16), avg3_2);
    _mm_store_si128((__m128i *)(dst + 24), avg3_3);
    dst += stride;
    avg2_0 = _mm_alignr_epi8(avg2_1, avg2_0, 2);
    avg2_1 = _mm_alignr_epi8(avg2_2, avg2_1, 2);
    avg2_2 = _mm_alignr_epi8(avg2_3, avg2_2, 2);
    avg2_3 = _mm_alignr_epi8(AR, avg2_3, 2);
    avg3_0 = _mm_alignr_epi8(avg3_1, avg3_0, 2);
    avg3_1 = _mm_alignr_epi8(avg3_2, avg3_1, 2);
    avg3_2 = _mm_alignr_epi8(avg3_3, avg3_2, 2);
    avg3_3 = _mm_alignr_epi8(AR, avg3_3, 2);
  }
  _mm_store_si128((__m128i *)dst, avg2_0);
  _mm_store_si128((__m128i *)(dst + 8), avg2_1);
  _mm_store_si128((__m128i *)(dst + 16), avg2_2);
  _mm_store_si128((__m128i *)(dst + 24), avg2_3);
  dst += stride;
  _mm_store_si128((__m128i *)dst, avg3_0);
  _mm_store_si128((__m128i *)(dst + 8), avg3_1);
  _mm_store_si128((__m128i *)(dst + 16), avg3_2);
  _mm_store_si128((__m128i *)(dst + 24), avg3_3);
}

static const uint8_t rotate_right_epu16[16] = {2,  3,  4,  5,  6,  7,  8, 9,
                                               10, 11, 12, 13, 14, 15, 0, 1};

static INLINE __m128i rotr_epu16(__m128i *a, const __m128i *rotrw) {
  *a = _mm_shuffle_epi8(*a, *rotrw);
  return *a;
}

void vpx_highbd_d117_predictor_8x8_ssse3(uint16_t *dst, ptrdiff_t stride,
                                         const uint16_t *above,
                                         const uint16_t *left, int bd) {
  const __m128i rotrw = _mm_loadu_si128((const __m128i *)rotate_right_epu16);
  const __m128i XABCDEFG = _mm_loadu_si128((const __m128i *)(above - 1));
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const __m128i IJKLMNOP = _mm_load_si128((const __m128i *)left);
  const __m128i IXABCDEF =
      _mm_alignr_epi8(XABCDEFG, _mm_slli_si128(IJKLMNOP, 14), 14);
  const __m128i avg3 = avg3_epu16(&ABCDEFGH, &XABCDEFG, &IXABCDEF);
  const __m128i avg2 = _mm_avg_epu16(ABCDEFGH, XABCDEFG);
  const __m128i XIJKLMNO =
      _mm_alignr_epi8(IJKLMNOP, _mm_slli_si128(XABCDEFG, 14), 14);
  const __m128i JKLMNOP0 = _mm_srli_si128(IJKLMNOP, 2);
  __m128i avg3_left = avg3_epu16(&XIJKLMNO, &IJKLMNOP, &JKLMNOP0);
  __m128i rowa = avg2;
  __m128i rowb = avg3;
  int i;
  (void)bd;
  for (i = 0; i < 8; i += 2) {
    _mm_store_si128((__m128i *)dst, rowa);
    dst += stride;
    _mm_store_si128((__m128i *)dst, rowb);
    dst += stride;
    rowa = _mm_alignr_epi8(rowa, rotr_epu16(&avg3_left, &rotrw), 14);
    rowb = _mm_alignr_epi8(rowb, rotr_epu16(&avg3_left, &rotrw), 14);
  }
}

void vpx_highbd_d117_predictor_16x16_ssse3(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i rotrw = _mm_loadu_si128((const __m128i *)rotate_right_epu16);
  const __m128i B0 = _mm_loadu_si128((const __m128i *)(above - 1));
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i B1 = _mm_loadu_si128((const __m128i *)(above + 7));
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  const __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  const __m128i L0 = _mm_load_si128((const __m128i *)left);
  const __m128i L1 = _mm_load_si128((const __m128i *)(left + 8));
  const __m128i C0 = _mm_alignr_epi8(B0, _mm_slli_si128(L0, 14), 14);
  const __m128i C1 = _mm_alignr_epi8(B1, B0, 14);
  const __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  const __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  const __m128i XL0 = _mm_alignr_epi8(L0, _mm_slli_si128(B0, 14), 14);
  const __m128i XL1 = _mm_alignr_epi8(L1, L0, 14);
  const __m128i L0_ = _mm_alignr_epi8(L1, L0, 2);
  const __m128i L1_ = _mm_srli_si128(L1, 2);
  const __m128i avg3_left_0 = avg3_epu16(&XL0, &L0, &L0_);
  const __m128i avg3_left_1 = avg3_epu16(&XL1, &L1, &L1_);
  __m128i rowa_0 = avg2_0;
  __m128i rowa_1 = avg2_1;
  __m128i rowb_0 = avg3_0;
  __m128i rowb_1 = avg3_1;
  __m128i avg_left = avg3_left_0;
  int i, j;
  (void)bd;
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 8; j += 2) {
      _mm_store_si128((__m128i *)dst, rowa_0);
      _mm_store_si128((__m128i *)(dst + 8), rowa_1);
      dst += stride;
      _mm_store_si128((__m128i *)dst, rowb_0);
      _mm_store_si128((__m128i *)(dst + 8), rowb_1);
      dst += stride;
      rowa_1 = _mm_alignr_epi8(rowa_1, rowa_0, 14);
      rowa_0 = _mm_alignr_epi8(rowa_0, rotr_epu16(&avg_left, &rotrw), 14);
      rowb_1 = _mm_alignr_epi8(rowb_1, rowb_0, 14);
      rowb_0 = _mm_alignr_epi8(rowb_0, rotr_epu16(&avg_left, &rotrw), 14);
    }
    avg_left = avg3_left_1;
  }
}

void vpx_highbd_d117_predictor_32x32_ssse3(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i rotrw = _mm_loadu_si128((const __m128i *)rotate_right_epu16);
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const __m128i A2 = _mm_load_si128((const __m128i *)(above + 16));
  const __m128i A3 = _mm_load_si128((const __m128i *)(above + 24));
  const __m128i B0 = _mm_loadu_si128((const __m128i *)(above - 1));
  const __m128i B1 = _mm_loadu_si128((const __m128i *)(above + 7));
  const __m128i B2 = _mm_loadu_si128((const __m128i *)(above + 15));
  const __m128i B3 = _mm_loadu_si128((const __m128i *)(above + 23));
  const __m128i avg2_0 = _mm_avg_epu16(A0, B0);
  const __m128i avg2_1 = _mm_avg_epu16(A1, B1);
  const __m128i avg2_2 = _mm_avg_epu16(A2, B2);
  const __m128i avg2_3 = _mm_avg_epu16(A3, B3);
  const __m128i L0 = _mm_load_si128((const __m128i *)left);
  const __m128i L1 = _mm_load_si128((const __m128i *)(left + 8));
  const __m128i L2 = _mm_load_si128((const __m128i *)(left + 16));
  const __m128i L3 = _mm_load_si128((const __m128i *)(left + 24));
  const __m128i C0 = _mm_alignr_epi8(B0, _mm_slli_si128(L0, 14), 14);
  const __m128i C1 = _mm_alignr_epi8(B1, B0, 14);
  const __m128i C2 = _mm_alignr_epi8(B2, B1, 14);
  const __m128i C3 = _mm_alignr_epi8(B3, B2, 14);
  const __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  const __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  const __m128i avg3_2 = avg3_epu16(&A2, &B2, &C2);
  const __m128i avg3_3 = avg3_epu16(&A3, &B3, &C3);
  const __m128i XL0 = _mm_alignr_epi8(L0, _mm_slli_si128(B0, 14), 14);
  const __m128i XL1 = _mm_alignr_epi8(L1, L0, 14);
  const __m128i XL2 = _mm_alignr_epi8(L2, L1, 14);
  const __m128i XL3 = _mm_alignr_epi8(L3, L2, 14);
  const __m128i L0_ = _mm_alignr_epi8(L1, L0, 2);
  const __m128i L1_ = _mm_alignr_epi8(L2, L1, 2);
  const __m128i L2_ = _mm_alignr_epi8(L3, L2, 2);
  const __m128i L3_ = _mm_srli_si128(L3, 2);
  const __m128i avg3_left_0 = avg3_epu16(&XL0, &L0, &L0_);
  const __m128i avg3_left_1 = avg3_epu16(&XL1, &L1, &L1_);
  const __m128i avg3_left_2 = avg3_epu16(&XL2, &L2, &L2_);
  const __m128i avg3_left_3 = avg3_epu16(&XL3, &L3, &L3_);
  __m128i rowa_0 = avg2_0;
  __m128i rowa_1 = avg2_1;
  __m128i rowa_2 = avg2_2;
  __m128i rowa_3 = avg2_3;
  __m128i rowb_0 = avg3_0;
  __m128i rowb_1 = avg3_1;
  __m128i rowb_2 = avg3_2;
  __m128i rowb_3 = avg3_3;
  __m128i avg_left = avg3_left_0;
  int i, j;
  (void)bd;
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 8; j += 2) {
      _mm_store_si128((__m128i *)dst, rowa_0);
      _mm_store_si128((__m128i *)(dst + 8), rowa_1);
      _mm_store_si128((__m128i *)(dst + 16), rowa_2);
      _mm_store_si128((__m128i *)(dst + 24), rowa_3);
      dst += stride;
      _mm_store_si128((__m128i *)dst, rowb_0);
      _mm_store_si128((__m128i *)(dst + 8), rowb_1);
      _mm_store_si128((__m128i *)(dst + 16), rowb_2);
      _mm_store_si128((__m128i *)(dst + 24), rowb_3);
      dst += stride;
      rowa_3 = _mm_alignr_epi8(rowa_3, rowa_2, 14);
      rowa_2 = _mm_alignr_epi8(rowa_2, rowa_1, 14);
      rowa_1 = _mm_alignr_epi8(rowa_1, rowa_0, 14);
      rowa_0 = _mm_alignr_epi8(rowa_0, rotr_epu16(&avg_left, &rotrw), 14);
      rowb_3 = _mm_alignr_epi8(rowb_3, rowb_2, 14);
      rowb_2 = _mm_alignr_epi8(rowb_2, rowb_1, 14);
      rowb_1 = _mm_alignr_epi8(rowb_1, rowb_0, 14);
      rowb_0 = _mm_alignr_epi8(rowb_0, rotr_epu16(&avg_left, &rotrw), 14);
    }
    if (i == 0) avg_left = avg3_left_1;
    if (i == 1) avg_left = avg3_left_2;
    if (i == 2) avg_left = avg3_left_3;
  }
}
