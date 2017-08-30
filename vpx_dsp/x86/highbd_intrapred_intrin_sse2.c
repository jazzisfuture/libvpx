/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>  // SSE2

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

// -----------------------------------------------------------------------------

void vpx_highbd_h_predictor_4x4_sse2(uint16_t *dst, ptrdiff_t stride,
                                     const uint16_t *above,
                                     const uint16_t *left, int bd) {
  const __m128i left_u16 = _mm_loadl_epi64((const __m128i *)left);
  const __m128i row0 = _mm_shufflelo_epi16(left_u16, 0x0);
  const __m128i row1 = _mm_shufflelo_epi16(left_u16, 0x55);
  const __m128i row2 = _mm_shufflelo_epi16(left_u16, 0xaa);
  const __m128i row3 = _mm_shufflelo_epi16(left_u16, 0xff);
  (void)above;
  (void)bd;
  _mm_storel_epi64((__m128i *)dst, row0);
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, row1);
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, row2);
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, row3);
}

void vpx_highbd_h_predictor_8x8_sse2(uint16_t *dst, ptrdiff_t stride,
                                     const uint16_t *above,
                                     const uint16_t *left, int bd) {
  const __m128i left_u16 = _mm_load_si128((const __m128i *)left);
  const __m128i row0 = _mm_shufflelo_epi16(left_u16, 0x0);
  const __m128i row1 = _mm_shufflelo_epi16(left_u16, 0x55);
  const __m128i row2 = _mm_shufflelo_epi16(left_u16, 0xaa);
  const __m128i row3 = _mm_shufflelo_epi16(left_u16, 0xff);
  const __m128i row4 = _mm_shufflehi_epi16(left_u16, 0x0);
  const __m128i row5 = _mm_shufflehi_epi16(left_u16, 0x55);
  const __m128i row6 = _mm_shufflehi_epi16(left_u16, 0xaa);
  const __m128i row7 = _mm_shufflehi_epi16(left_u16, 0xff);
  (void)above;
  (void)bd;
  _mm_store_si128((__m128i *)dst, _mm_unpacklo_epi64(row0, row0));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpacklo_epi64(row1, row1));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpacklo_epi64(row2, row2));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpacklo_epi64(row3, row3));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpackhi_epi64(row4, row4));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpackhi_epi64(row5, row5));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpackhi_epi64(row6, row6));
  dst += stride;
  _mm_store_si128((__m128i *)dst, _mm_unpackhi_epi64(row7, row7));
}

static INLINE void h_store_16_unpacklo(uint16_t **dst, const ptrdiff_t stride,
                                       const __m128i *row) {
  const __m128i val = _mm_unpacklo_epi64(*row, *row);
  _mm_store_si128((__m128i *)*dst, val);
  _mm_store_si128((__m128i *)(*dst + 8), val);
  *dst += stride;
}

static INLINE void h_store_16_unpackhi(uint16_t **dst, const ptrdiff_t stride,
                                       const __m128i *row) {
  const __m128i val = _mm_unpackhi_epi64(*row, *row);
  _mm_store_si128((__m128i *)(*dst), val);
  _mm_store_si128((__m128i *)(*dst + 8), val);
  *dst += stride;
}

void vpx_highbd_h_predictor_16x16_sse2(uint16_t *dst, ptrdiff_t stride,
                                       const uint16_t *above,
                                       const uint16_t *left, int bd) {
  int i;
  (void)above;
  (void)bd;

  for (i = 0; i < 2; i++, left += 8) {
    const __m128i left_u16 = _mm_load_si128((const __m128i *)left);
    const __m128i row0 = _mm_shufflelo_epi16(left_u16, 0x0);
    const __m128i row1 = _mm_shufflelo_epi16(left_u16, 0x55);
    const __m128i row2 = _mm_shufflelo_epi16(left_u16, 0xaa);
    const __m128i row3 = _mm_shufflelo_epi16(left_u16, 0xff);
    const __m128i row4 = _mm_shufflehi_epi16(left_u16, 0x0);
    const __m128i row5 = _mm_shufflehi_epi16(left_u16, 0x55);
    const __m128i row6 = _mm_shufflehi_epi16(left_u16, 0xaa);
    const __m128i row7 = _mm_shufflehi_epi16(left_u16, 0xff);
    h_store_16_unpacklo(&dst, stride, &row0);
    h_store_16_unpacklo(&dst, stride, &row1);
    h_store_16_unpacklo(&dst, stride, &row2);
    h_store_16_unpacklo(&dst, stride, &row3);
    h_store_16_unpackhi(&dst, stride, &row4);
    h_store_16_unpackhi(&dst, stride, &row5);
    h_store_16_unpackhi(&dst, stride, &row6);
    h_store_16_unpackhi(&dst, stride, &row7);
  }
}

static INLINE void h_store_32_unpacklo(uint16_t **dst, const ptrdiff_t stride,
                                       const __m128i *row) {
  const __m128i val = _mm_unpacklo_epi64(*row, *row);
  _mm_store_si128((__m128i *)(*dst), val);
  _mm_store_si128((__m128i *)(*dst + 8), val);
  _mm_store_si128((__m128i *)(*dst + 16), val);
  _mm_store_si128((__m128i *)(*dst + 24), val);
  *dst += stride;
}

static INLINE void h_store_32_unpackhi(uint16_t **dst, const ptrdiff_t stride,
                                       const __m128i *row) {
  const __m128i val = _mm_unpackhi_epi64(*row, *row);
  _mm_store_si128((__m128i *)(*dst), val);
  _mm_store_si128((__m128i *)(*dst + 8), val);
  _mm_store_si128((__m128i *)(*dst + 16), val);
  _mm_store_si128((__m128i *)(*dst + 24), val);
  *dst += stride;
}

void vpx_highbd_h_predictor_32x32_sse2(uint16_t *dst, ptrdiff_t stride,
                                       const uint16_t *above,
                                       const uint16_t *left, int bd) {
  int i;
  (void)above;
  (void)bd;

  for (i = 0; i < 4; i++, left += 8) {
    const __m128i left_u16 = _mm_load_si128((const __m128i *)left);
    const __m128i row0 = _mm_shufflelo_epi16(left_u16, 0x0);
    const __m128i row1 = _mm_shufflelo_epi16(left_u16, 0x55);
    const __m128i row2 = _mm_shufflelo_epi16(left_u16, 0xaa);
    const __m128i row3 = _mm_shufflelo_epi16(left_u16, 0xff);
    const __m128i row4 = _mm_shufflehi_epi16(left_u16, 0x0);
    const __m128i row5 = _mm_shufflehi_epi16(left_u16, 0x55);
    const __m128i row6 = _mm_shufflehi_epi16(left_u16, 0xaa);
    const __m128i row7 = _mm_shufflehi_epi16(left_u16, 0xff);
    h_store_32_unpacklo(&dst, stride, &row0);
    h_store_32_unpacklo(&dst, stride, &row1);
    h_store_32_unpacklo(&dst, stride, &row2);
    h_store_32_unpacklo(&dst, stride, &row3);
    h_store_32_unpackhi(&dst, stride, &row4);
    h_store_32_unpackhi(&dst, stride, &row5);
    h_store_32_unpackhi(&dst, stride, &row6);
    h_store_32_unpackhi(&dst, stride, &row7);
  }
}

//------------------------------------------------------------------------------
// DC 4x4

static INLINE __m128i dc_sum_4(const uint16_t *ref) {
  const __m128i _dcba = _mm_loadl_epi64((const __m128i *)ref);
  const __m128i _xxdc = _mm_shufflelo_epi16(_dcba, 0xe);
  const __m128i a = _mm_add_epi16(_dcba, _xxdc);
  return _mm_add_epi16(a, _mm_shufflelo_epi16(a, 0x1));
}

static INLINE void dc_store_4x4(uint16_t *dst, ptrdiff_t stride,
                                const __m128i *dc) {
  const __m128i dc_dup = _mm_shufflelo_epi16(*dc, 0x0);
  int i;
  for (i = 0; i < 4; ++i, dst += stride) {
    _mm_storel_epi64((__m128i *)dst, dc_dup);
  }
}

void vpx_highbd_dc_left_predictor_4x4_sse2(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i two = _mm_cvtsi32_si128(2);
  const __m128i sum = dc_sum_4(left);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, two), 2);
  (void)above;
  (void)bd;
  dc_store_4x4(dst, stride, &dc);
}

void vpx_highbd_dc_top_predictor_4x4_sse2(uint16_t *dst, ptrdiff_t stride,
                                          const uint16_t *above,
                                          const uint16_t *left, int bd) {
  const __m128i two = _mm_cvtsi32_si128(2);
  const __m128i sum = dc_sum_4(above);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, two), 2);
  (void)left;
  (void)bd;
  dc_store_4x4(dst, stride, &dc);
}

//------------------------------------------------------------------------------
// DC 8x8

static INLINE __m128i dc_sum_8(const uint16_t *ref) {
  const __m128i ref_u16 = _mm_load_si128((const __m128i *)ref);
  const __m128i _dcba = _mm_add_epi16(ref_u16, _mm_srli_si128(ref_u16, 8));
  const __m128i _xxdc = _mm_shufflelo_epi16(_dcba, 0xe);
  const __m128i a = _mm_add_epi16(_dcba, _xxdc);

  return _mm_add_epi16(a, _mm_shufflelo_epi16(a, 0x1));
}

static INLINE void dc_store_8x8(uint16_t *dst, ptrdiff_t stride,
                                const __m128i *dc) {
  const __m128i dc_dup_lo = _mm_shufflelo_epi16(*dc, 0);
  const __m128i dc_dup = _mm_unpacklo_epi64(dc_dup_lo, dc_dup_lo);
  int i;
  for (i = 0; i < 8; ++i, dst += stride) {
    _mm_store_si128((__m128i *)dst, dc_dup);
  }
}

void vpx_highbd_dc_left_predictor_8x8_sse2(uint16_t *dst, ptrdiff_t stride,
                                           const uint16_t *above,
                                           const uint16_t *left, int bd) {
  const __m128i four = _mm_cvtsi32_si128(4);
  const __m128i sum = dc_sum_8(left);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, four), 3);
  (void)above;
  (void)bd;
  dc_store_8x8(dst, stride, &dc);
}

void vpx_highbd_dc_top_predictor_8x8_sse2(uint16_t *dst, ptrdiff_t stride,
                                          const uint16_t *above,
                                          const uint16_t *left, int bd) {
  const __m128i four = _mm_cvtsi32_si128(4);
  const __m128i sum = dc_sum_8(above);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, four), 3);
  (void)left;
  (void)bd;
  dc_store_8x8(dst, stride, &dc);
}

//------------------------------------------------------------------------------
// DC 16x16

static INLINE __m128i dc_sum_16(const uint16_t *ref) {
  const __m128i sum_lo = dc_sum_8(ref);
  const __m128i sum_hi = dc_sum_8(ref + 8);
  return _mm_add_epi16(sum_lo, sum_hi);
}

static INLINE void dc_store_16x16(uint16_t *dst, ptrdiff_t stride,
                                  const __m128i *dc) {
  const __m128i dc_dup_lo = _mm_shufflelo_epi16(*dc, 0);
  const __m128i dc_dup = _mm_unpacklo_epi64(dc_dup_lo, dc_dup_lo);
  int i;
  for (i = 0; i < 16; ++i, dst += stride) {
    _mm_store_si128((__m128i *)dst, dc_dup);
    _mm_store_si128((__m128i *)(dst + 8), dc_dup);
  }
}

void vpx_highbd_dc_left_predictor_16x16_sse2(uint16_t *dst, ptrdiff_t stride,
                                             const uint16_t *above,
                                             const uint16_t *left, int bd) {
  const __m128i eight = _mm_cvtsi32_si128(8);
  const __m128i sum = dc_sum_16(left);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, eight), 4);
  (void)above;
  (void)bd;
  dc_store_16x16(dst, stride, &dc);
}

void vpx_highbd_dc_top_predictor_16x16_sse2(uint16_t *dst, ptrdiff_t stride,
                                            const uint16_t *above,
                                            const uint16_t *left, int bd) {
  const __m128i eight = _mm_cvtsi32_si128(8);
  const __m128i sum = dc_sum_16(above);
  const __m128i dc = _mm_srli_epi16(_mm_add_epi16(sum, eight), 4);
  (void)left;
  (void)bd;
  dc_store_16x16(dst, stride, &dc);
}

//------------------------------------------------------------------------------
// DC 32x32

static INLINE __m128i dc_sum_32(const uint16_t *ref) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i sum_a = dc_sum_16(ref);
  const __m128i sum_b = dc_sum_16(ref + 16);
  // 12 bit bd will outrange, so expand to 32 bit before adding final total
  return _mm_add_epi32(_mm_unpacklo_epi16(sum_a, zero),
                       _mm_unpacklo_epi16(sum_b, zero));
}

static INLINE void dc_store_32x32(uint16_t *dst, ptrdiff_t stride,
                                  const __m128i *dc) {
  const __m128i dc_dup_lo = _mm_shufflelo_epi16(*dc, 0);
  const __m128i dc_dup = _mm_unpacklo_epi64(dc_dup_lo, dc_dup_lo);
  int i;
  for (i = 0; i < 32; ++i, dst += stride) {
    _mm_store_si128((__m128i *)dst, dc_dup);
    _mm_store_si128((__m128i *)(dst + 8), dc_dup);
    _mm_store_si128((__m128i *)(dst + 16), dc_dup);
    _mm_store_si128((__m128i *)(dst + 24), dc_dup);
  }
}

void vpx_highbd_dc_left_predictor_32x32_sse2(uint16_t *dst, ptrdiff_t stride,
                                             const uint16_t *above,
                                             const uint16_t *left, int bd) {
  const __m128i sixteen = _mm_cvtsi32_si128(16);
  const __m128i sum = dc_sum_32(left);
  const __m128i dc = _mm_srli_epi32(_mm_add_epi32(sum, sixteen), 5);
  (void)above;
  (void)bd;
  dc_store_32x32(dst, stride, &dc);
}

void vpx_highbd_dc_top_predictor_32x32_sse2(uint16_t *dst, ptrdiff_t stride,
                                            const uint16_t *above,
                                            const uint16_t *left, int bd) {
  const __m128i sixteen = _mm_cvtsi32_si128(16);
  const __m128i sum = dc_sum_32(above);
  const __m128i dc = _mm_srli_epi32(_mm_add_epi32(sum, sixteen), 5);
  (void)left;
  (void)bd;
  dc_store_32x32(dst, stride, &dc);
}

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

void vpx_highbd_d45_predictor_4x4_sse2(uint16_t *dst, ptrdiff_t stride,
                                       const uint16_t *above,
                                       const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const __m128i BCDEFGH0 = _mm_srli_si128(ABCDEFGH, 2);
  const __m128i CDEFGH00 = _mm_srli_si128(ABCDEFGH, 4);
  const __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGH0, &CDEFGH00);
  (void)left;
  (void)bd;
  _mm_storel_epi64((__m128i *)dst, avg3);
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 2));
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 4));
  dst += stride;
  _mm_storel_epi64((__m128i *)dst, _mm_srli_si128(avg3, 6));
  dst[3] = above[7];
}

static INLINE void d45_store_8(uint16_t **dst, const ptrdiff_t stride,
                               const int above_right, __m128i *row) {
  *row = _mm_srli_si128(*row, 2);
  *row = _mm_insert_epi16(*row, above_right, 7);
  _mm_store_si128((__m128i *)*dst, *row);
  *dst += stride;
}

void vpx_highbd_d45_predictor_8x8_sse2(uint16_t *dst, ptrdiff_t stride,
                                       const uint16_t *above,
                                       const uint16_t *left, int bd) {
  const __m128i ABCDEFGH = _mm_load_si128((const __m128i *)above);
  const int above_right = _mm_extract_epi16(ABCDEFGH, 7);
  const __m128i BCDEFGH0 = _mm_srli_si128(ABCDEFGH, 2);
  const __m128i BCDEFGHH = _mm_insert_epi16(BCDEFGH0, above_right, 7);
  const __m128i CDEFGHH0 = _mm_srli_si128(BCDEFGHH, 2);
  const __m128i CDEFGHHH = _mm_insert_epi16(CDEFGHH0, above_right, 7);
  __m128i avg3 = avg3_epu16(&ABCDEFGH, &BCDEFGHH, &CDEFGHHH);
  (void)left;
  (void)bd;
  _mm_store_si128((__m128i *)dst, avg3);
  dst += stride;
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
  d45_store_8(&dst, stride, above_right, &avg3);
}

static INLINE void d45_store_16(uint16_t **dst, const ptrdiff_t stride,
                                const int above_right, __m128i *row_0,
                                __m128i *row_1) {
  const int above8_0 = _mm_extract_epi16(*row_1, 0);
  *row_0 = _mm_srli_si128(*row_0, 2);
  *row_0 = _mm_insert_epi16(*row_0, above8_0, 7);
  *row_1 = _mm_srli_si128(*row_1, 2);
  *row_1 = _mm_insert_epi16(*row_1, above_right, 7);
  _mm_store_si128((__m128i *)*dst, *row_0);
  _mm_store_si128((__m128i *)(*dst + 8), *row_1);
  *dst += stride;
}

static INLINE __m128i ext_shift_ins(__m128i a, int *above_right) {
  const int new_above_right = _mm_extract_epi16(a, 0);
  const __m128i b = _mm_srli_si128(a, 2);
  const __m128i c = _mm_insert_epi16(b, *above_right, 7);
  *above_right = new_above_right;
  return c;
}

void vpx_highbd_d45_predictor_16x16_sse2(uint16_t *dst, ptrdiff_t stride,
                                         const uint16_t *above,
                                         const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const int above_right = _mm_extract_epi16(A1, 7);
  int ar = above_right;
  const __m128i B1 = ext_shift_ins(A1, &ar);
  const __m128i B0 = ext_shift_ins(A0, &ar);
  int ar2 = _mm_extract_epi16(A1, 7);
  const __m128i C1 = ext_shift_ins(B1, &ar2);
  const __m128i C0 = ext_shift_ins(B0, &ar2);
  __m128i avg3_0 = avg3_epu16(&A0, &B0, &C0);
  __m128i avg3_1 = avg3_epu16(&A1, &B1, &C1);
  (void)left;
  (void)bd;
  _mm_store_si128((__m128i *)dst, avg3_0);
  _mm_store_si128((__m128i *)(dst + 8), avg3_1);
  dst += stride;
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
  d45_store_16(&dst, stride, above_right, &avg3_0, &avg3_1);
}

void vpx_highbd_d45_predictor_32x32_sse2(uint16_t *dst, ptrdiff_t stride,
                                         const uint16_t *above,
                                         const uint16_t *left, int bd) {
  const __m128i A0 = _mm_load_si128((const __m128i *)above);
  const __m128i A1 = _mm_load_si128((const __m128i *)(above + 8));
  const __m128i A2 = _mm_load_si128((const __m128i *)(above + 16));
  const __m128i A3 = _mm_load_si128((const __m128i *)(above + 24));
  const int above_right = _mm_extract_epi16(A3, 7);
  int ar = above_right;
  const __m128i B3 = ext_shift_ins(A3, &ar);
  const __m128i B2 = ext_shift_ins(A2, &ar);
  const __m128i B1 = ext_shift_ins(A1, &ar);
  const __m128i B0 = ext_shift_ins(A0, &ar);
  int ar2 = above_right;
  const __m128i C3 = ext_shift_ins(B3, &ar2);
  const __m128i C2 = ext_shift_ins(B2, &ar2);
  const __m128i C1 = ext_shift_ins(B1, &ar2);
  const __m128i C0 = ext_shift_ins(B0, &ar2);
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
    ar2 = above_right;
    avg3_3 = ext_shift_ins(avg3_3, &ar2);
    avg3_2 = ext_shift_ins(avg3_2, &ar2);
    avg3_1 = ext_shift_ins(avg3_1, &ar2);
    avg3_0 = ext_shift_ins(avg3_0, &ar2);
    _mm_store_si128((__m128i *)dst, avg3_0);
    _mm_store_si128((__m128i *)(dst + 8), avg3_1);
    _mm_store_si128((__m128i *)(dst + 16), avg3_2);
    _mm_store_si128((__m128i *)(dst + 24), avg3_3);
    dst += stride;
  }
}
