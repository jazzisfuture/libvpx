/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/x86/highbd_inv_txfm_sse2.h"
#include "vpx_dsp/x86/inv_txfm_sse2.h"
#include "vpx_dsp/x86/transpose_sse2.h"
#include "vpx_dsp/x86/txfm_common_sse2.h"

// Note: There is no 32-bit signed multiply SIMD instruction in SSE2.
//       _mm_mul_epu32() is used which can only guarantee the lower 32-bit
//       (signed) result is meaningful, which is enough for 4x4 idct.

static INLINE __m128i dct_const_round_shift_4_sse2(const __m128i in0,
                                                   const __m128i in1) {
  const __m128i t0 = _mm_unpacklo_epi32(in0, in1);  // 0, 1
  const __m128i t1 = _mm_unpackhi_epi32(in0, in1);  // 2, 3
  const __m128i t2 = _mm_unpacklo_epi64(t0, t1);    // 0, 1, 2, 3
  return dct_const_round_shift_sse2(t2);
}

static INLINE __m128i wraplow_16bit_sse2(const __m128i in0, const __m128i in1,
                                         const __m128i rounding) {
  __m128i temp[2];
  temp[0] = _mm_add_epi32(in0, rounding);
  temp[1] = _mm_add_epi32(in1, rounding);
  temp[0] = _mm_srai_epi32(temp[0], 4);
  temp[1] = _mm_srai_epi32(temp[1], 4);
  return _mm_packs_epi32(temp[0], temp[1]);
}

static INLINE void vpx_highbd_idct4_sse2(__m128i *const io) {
  const __m128i cospi_p16_p16 = _mm_setr_epi32(cospi_16_64, 0, cospi_16_64, 0);
  const __m128i cospi_p08_p08 = _mm_setr_epi32(cospi_8_64, 0, cospi_8_64, 0);
  const __m128i cospi_p24_p24 = _mm_setr_epi32(cospi_24_64, 0, cospi_24_64, 0);
  __m128i temp1[4], temp2[4], step[4];

  transpose_32bit_4x4(&io[0], &io[1], &io[2], &io[3]);

  // stage 1
  temp1[0] = _mm_add_epi32(io[0], io[2]);             // input[0] + input[2]
  temp2[0] = _mm_sub_epi32(io[0], io[2]);             // input[0] - input[2]
  temp1[1] = _mm_srli_si128(temp1[0], 4);             // 1, 3
  temp2[1] = _mm_srli_si128(temp2[0], 4);             // 1, 3
  temp1[0] = _mm_mul_epu32(temp1[0], cospi_p16_p16);  // ([0] + [2])*cospi_16_64
  temp1[1] = _mm_mul_epu32(temp1[1], cospi_p16_p16);  // ([0] + [2])*cospi_16_64
  temp2[0] = _mm_mul_epu32(temp2[0], cospi_p16_p16);  // ([0] - [2])*cospi_16_64
  temp2[1] = _mm_mul_epu32(temp2[1], cospi_p16_p16);  // ([0] - [2])*cospi_16_64
  step[0] = dct_const_round_shift_4_sse2(temp1[0], temp1[1]);
  step[1] = dct_const_round_shift_4_sse2(temp2[0], temp2[1]);

  temp1[3] = _mm_srli_si128(io[1], 4);
  temp2[3] = _mm_srli_si128(io[3], 4);
  temp1[0] = _mm_mul_epu32(io[1], cospi_p24_p24);     // input[1] * cospi_24_64
  temp1[1] = _mm_mul_epu32(temp1[3], cospi_p24_p24);  // input[1] * cospi_24_64
  temp2[0] = _mm_mul_epu32(io[1], cospi_p08_p08);     // input[1] * cospi_8_64
  temp2[1] = _mm_mul_epu32(temp1[3], cospi_p08_p08);  // input[1] * cospi_8_64
  temp1[2] = _mm_mul_epu32(io[3], cospi_p08_p08);     // input[3] * cospi_8_64
  temp1[3] = _mm_mul_epu32(temp2[3], cospi_p08_p08);  // input[3] * cospi_8_64
  temp2[2] = _mm_mul_epu32(io[3], cospi_p24_p24);     // input[3] * cospi_24_64
  temp2[3] = _mm_mul_epu32(temp2[3], cospi_p24_p24);  // input[3] * cospi_24_64
  temp1[0] = _mm_sub_epi64(temp1[0], temp1[2]);  // [1]*cospi_24 - [3]*cospi_8
  temp1[1] = _mm_sub_epi64(temp1[1], temp1[3]);  // [1]*cospi_24 - [3]*cospi_8
  temp2[0] = _mm_add_epi64(temp2[0], temp2[2]);  // [1]*cospi_8 + [3]*cospi_24
  temp2[1] = _mm_add_epi64(temp2[1], temp2[3]);  // [1]*cospi_8 + [3]*cospi_24
  step[2] = dct_const_round_shift_4_sse2(temp1[0], temp1[1]);
  step[3] = dct_const_round_shift_4_sse2(temp2[0], temp2[1]);

  // stage 2
  io[0] = _mm_add_epi32(step[0], step[3]);  // step[0] + step[3]
  io[1] = _mm_add_epi32(step[1], step[2]);  // step[1] + step[2]
  io[2] = _mm_sub_epi32(step[1], step[2]);  // step[1] - step[2]
  io[3] = _mm_sub_epi32(step[0], step[3]);  // step[0] - step[3]
}

void vpx_highbd_idct4x4_16_add_sse2(const tran_low_t *input, uint16_t *dest,
                                    int stride, int bd) {
  __m128i in[4];
  in[0] = _mm_load_si128((const __m128i *)(input + 0));
  in[1] = _mm_load_si128((const __m128i *)(input + 4));
  in[2] = _mm_load_si128((const __m128i *)(input + 8));
  in[3] = _mm_load_si128((const __m128i *)(input + 12));

  if (bd == 8) {
    in[0] = _mm_packs_epi32(in[0], in[1]);
    in[1] = _mm_packs_epi32(in[2], in[3]);
    idct4_sse2(in);
    idct4_sse2(in);
    in[0] = _mm_add_epi16(in[0], _mm_set1_epi16(8));
    in[1] = _mm_add_epi16(in[1], _mm_set1_epi16(8));
    in[0] = _mm_srai_epi16(in[0], 4);
    in[1] = _mm_srai_epi16(in[1], 4);
  } else {
    if (bd == 10) {
      __m128i step[4];
      in[0] = _mm_packs_epi32(in[0], in[1]);
      in[1] = _mm_packs_epi32(in[2], in[3]);
      idct4_kernel_sse2(in, step);
      in[0] = _mm_add_epi32(step[0], step[3]);  // step[0] + step[3]
      in[1] = _mm_add_epi32(step[1], step[2]);  // step[1] + step[2]
      in[2] = _mm_sub_epi32(step[1], step[2]);  // step[1] - step[2]
      in[3] = _mm_sub_epi32(step[0], step[3]);  // step[0] - step[3]
    } else {
      vpx_highbd_idct4_sse2(in);
    }
    vpx_highbd_idct4_sse2(in);
    in[0] = wraplow_16bit_sse2(in[0], in[1], _mm_set1_epi32(8));
    in[1] = wraplow_16bit_sse2(in[2], in[3], _mm_set1_epi32(8));
  }

  // Reconstruction and Store
  {
    __m128i d0 = _mm_loadl_epi64((const __m128i *)dest);
    __m128i d2 = _mm_loadl_epi64((const __m128i *)(dest + stride * 2));
    d0 = _mm_unpacklo_epi64(d0,
                            _mm_loadl_epi64((const __m128i *)(dest + stride)));
    d2 = _mm_unpacklo_epi64(
        d2, _mm_loadl_epi64((const __m128i *)(dest + stride * 3)));
    d0 = clamp_high_sse2(_mm_adds_epi16(d0, in[0]), bd);
    d2 = clamp_high_sse2(_mm_adds_epi16(d2, in[1]), bd);
    // store input0
    _mm_storel_epi64((__m128i *)dest, d0);
    // store input1
    d0 = _mm_srli_si128(d0, 8);
    _mm_storel_epi64((__m128i *)(dest + stride), d0);
    // store input2
    _mm_storel_epi64((__m128i *)(dest + stride * 2), d2);
    // store input3
    d2 = _mm_srli_si128(d2, 8);
    _mm_storel_epi64((__m128i *)(dest + stride * 3), d2);
  }
}

void vpx_highbd_idct4x4_1_add_sse2(const tran_low_t *input, uint16_t *dest,
                                   int stride, int bd) {
  const __m128i zero = _mm_setzero_si128();
  // Faster than _mm_set1_epi16((1 << bd) - 1).
  const __m128i one = _mm_set1_epi16(1);
  const __m128i max = _mm_sub_epi16(_mm_slli_epi16(one, bd), one);
  int a1, i;
  tran_low_t out;
  __m128i dc, d;

  out = HIGHBD_WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64), bd);
  out = HIGHBD_WRAPLOW(dct_const_round_shift(out * cospi_16_64), bd);
  a1 = ROUND_POWER_OF_TWO(out, 4);
  dc = _mm_set1_epi16(a1);

  for (i = 0; i < 4; ++i) {
    d = _mm_loadl_epi64((const __m128i *)dest);
    d = add_dc_clamp(&zero, &max, &dc, &d);
    _mm_storel_epi64((__m128i *)dest, d);
    dest += stride;
  }
}
