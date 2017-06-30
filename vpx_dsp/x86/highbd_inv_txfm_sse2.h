/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_X86_HIGHBD_INV_TXFM_SSE2_H_
#define VPX_DSP_X86_HIGHBD_INV_TXFM_SSE2_H_

#include <emmintrin.h>  // SSE2

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/inv_txfm.h"
#include "vpx_dsp/x86/txfm_common_sse2.h"

static INLINE void extend_64bit(const __m128i in,
                                __m128i *const out /*out[2]*/) {
  out[0] = _mm_unpacklo_epi32(in, in);  // 0, 0, 1, 1
  out[1] = _mm_unpackhi_epi32(in, in);  // 2, 2, 3, 3
}

static INLINE __m128i wraplow_16bit_shift4(const __m128i in0, const __m128i in1,
                                           const __m128i rounding) {
  __m128i temp[2];
  temp[0] = _mm_add_epi32(in0, rounding);
  temp[1] = _mm_add_epi32(in1, rounding);
  temp[0] = _mm_srai_epi32(temp[0], 4);
  temp[1] = _mm_srai_epi32(temp[1], 4);
  return _mm_packs_epi32(temp[0], temp[1]);
}

static INLINE __m128i wraplow_16bit_shift5(const __m128i in0, const __m128i in1,
                                           const __m128i rounding) {
  __m128i temp[2];
  temp[0] = _mm_add_epi32(in0, rounding);
  temp[1] = _mm_add_epi32(in1, rounding);
  temp[0] = _mm_srai_epi32(temp[0], 5);
  temp[1] = _mm_srai_epi32(temp[1], 5);
  return _mm_packs_epi32(temp[0], temp[1]);
}

static INLINE __m128i dct_const_round_shift_64bit(const __m128i in) {
  const __m128i t = _mm_add_epi64(
      in,
      _mm_setr_epi32(DCT_CONST_ROUNDING << 2, 0, DCT_CONST_ROUNDING << 2, 0));
  return _mm_srli_si128(t, 2);
}

static INLINE __m128i pack_4(const __m128i in0, const __m128i in1) {
  const __m128i t0 = _mm_unpacklo_epi32(in0, in1);  // 0, 2
  const __m128i t1 = _mm_unpackhi_epi32(in0, in1);  // 1, 3
  return _mm_unpacklo_epi32(t0, t1);                // 0, 1, 2, 3
}

static INLINE void abs_extend_64bit_sse2(const __m128i in,
                                         __m128i *const out /*out[2]*/,
                                         __m128i *const sign /*sign[2]*/) {
  sign[0] = _mm_srai_epi32(in, 31);
  out[0] = _mm_xor_si128(in, sign[0]);
  out[0] = _mm_sub_epi32(out[0], sign[0]);
  sign[1] = _mm_unpackhi_epi32(sign[0], sign[0]);  // 64-bit sign of 2, 3
  sign[0] = _mm_unpacklo_epi32(sign[0], sign[0]);  // 64-bit sign of 0, 1
  out[1] = _mm_unpackhi_epi32(out[0], out[0]);     // 2, 3
  out[0] = _mm_unpacklo_epi32(out[0], out[0]);     // 0, 1
}

// Note: cospi must be non negative.
static INLINE __m128i multiply_apply_sign_sse2(const __m128i in,
                                               const __m128i sign,
                                               const __m128i cospi) {
  __m128i out = _mm_mul_epu32(in, cospi);
  out = _mm_xor_si128(out, sign);
  return _mm_sub_epi64(out, sign);
}

// Note: cospi must be non negative.
static INLINE __m128i multiplication_round_shift_sse2(
    const __m128i *const in /*in[2]*/, const __m128i *const sign /*sign[2]*/,
    const __m128i cospi) {
  __m128i t0, t1;
  t0 = multiply_apply_sign_sse2(in[0], sign[0], cospi);
  t1 = multiply_apply_sign_sse2(in[1], sign[1], cospi);
  t0 = dct_const_round_shift_64bit(t0);
  t1 = dct_const_round_shift_64bit(t1);
  return pack_4(t0, t1);
}

// Note: cst0 and cst1 must be non negative.
static INLINE void multiplication_and_add_2_sse2(
    const __m128i in0, const __m128i in1, const __m128i *const cst0,
    const __m128i *const cst1, __m128i *const out0, __m128i *const out1) {
  __m128i temp1[4], temp2[4], sign1[4], sign2[4];

  abs_extend_64bit_sse2(in0, temp1, sign1);
  abs_extend_64bit_sse2(in1, temp2, sign2);
  temp1[2] = multiply_apply_sign_sse2(temp1[0], sign1[0], *cst1);
  temp1[3] = multiply_apply_sign_sse2(temp1[1], sign1[1], *cst1);
  temp1[0] = multiply_apply_sign_sse2(temp1[0], sign1[0], *cst0);
  temp1[1] = multiply_apply_sign_sse2(temp1[1], sign1[1], *cst0);
  temp2[2] = multiply_apply_sign_sse2(temp2[0], sign2[0], *cst0);
  temp2[3] = multiply_apply_sign_sse2(temp2[1], sign2[1], *cst0);
  temp2[0] = multiply_apply_sign_sse2(temp2[0], sign2[0], *cst1);
  temp2[1] = multiply_apply_sign_sse2(temp2[1], sign2[1], *cst1);
  temp1[0] = _mm_sub_epi64(temp1[0], temp2[0]);
  temp1[1] = _mm_sub_epi64(temp1[1], temp2[1]);
  temp2[0] = _mm_add_epi64(temp1[2], temp2[2]);
  temp2[1] = _mm_add_epi64(temp1[3], temp2[3]);
  temp1[0] = dct_const_round_shift_64bit(temp1[0]);
  temp1[1] = dct_const_round_shift_64bit(temp1[1]);
  temp2[0] = dct_const_round_shift_64bit(temp2[0]);
  temp2[1] = dct_const_round_shift_64bit(temp2[1]);
  *out0 = pack_4(temp1[0], temp1[1]);
  *out1 = pack_4(temp2[0], temp2[1]);
}

static INLINE void highbd_idct8_stage4(const __m128i *const in,
                                       __m128i *const out) {
  out[0] = _mm_add_epi32(in[0], in[7]);
  out[1] = _mm_add_epi32(in[1], in[6]);
  out[2] = _mm_add_epi32(in[2], in[5]);
  out[3] = _mm_add_epi32(in[3], in[4]);
  out[4] = _mm_sub_epi32(in[3], in[4]);
  out[5] = _mm_sub_epi32(in[2], in[5]);
  out[6] = _mm_sub_epi32(in[1], in[6]);
  out[7] = _mm_sub_epi32(in[0], in[7]);
}

static INLINE void highbd_idct8x8_final_round(__m128i *const io) {
  io[0] = wraplow_16bit_shift5(io[0], io[8], _mm_set1_epi32(16));
  io[1] = wraplow_16bit_shift5(io[1], io[9], _mm_set1_epi32(16));
  io[2] = wraplow_16bit_shift5(io[2], io[10], _mm_set1_epi32(16));
  io[3] = wraplow_16bit_shift5(io[3], io[11], _mm_set1_epi32(16));
  io[4] = wraplow_16bit_shift5(io[4], io[12], _mm_set1_epi32(16));
  io[5] = wraplow_16bit_shift5(io[5], io[13], _mm_set1_epi32(16));
  io[6] = wraplow_16bit_shift5(io[6], io[14], _mm_set1_epi32(16));
  io[7] = wraplow_16bit_shift5(io[7], io[15], _mm_set1_epi32(16));
}
static INLINE __m128i add_clamp(const __m128i in0, const __m128i in1,
                                const int bd) {
  const __m128i zero = _mm_set1_epi16(0);
  // Faster than _mm_set1_epi16((1 << bd) - 1).
  const __m128i one = _mm_set1_epi16(1);
  const __m128i max = _mm_sub_epi16(_mm_slli_epi16(one, bd), one);
  __m128i d;

  d = _mm_adds_epi16(in0, in1);
  d = _mm_max_epi16(d, zero);
  d = _mm_min_epi16(d, max);

  return d;
}

static INLINE void highbd_idct_1_add_kernel(const tran_low_t *input,
                                            uint16_t *dest, int stride, int bd,
                                            const int size) {
  int a1, i, j;
  tran_low_t out;
  __m128i dc, d;

  out = HIGHBD_WRAPLOW(dct_const_round_shift(input[0] * cospi_16_64), bd);
  out = HIGHBD_WRAPLOW(dct_const_round_shift(out * cospi_16_64), bd);
  a1 = ROUND_POWER_OF_TWO(out, (size == 8) ? 5 : 6);
  dc = _mm_set1_epi16(a1);

  for (i = 0; i < size; ++i) {
    for (j = 0; j < (size >> 3); ++j) {
      d = _mm_load_si128((const __m128i *)(&dest[j * 8]));
      d = add_clamp(d, dc, bd);
      _mm_store_si128((__m128i *)(&dest[j * 8]), d);
    }
    dest += stride;
  }
}

static INLINE void recon_and_store_4_dual(const __m128i in,
                                          uint16_t *const dest,
                                          const int stride, const int bd) {
  __m128i d;

  d = _mm_loadl_epi64((const __m128i *)(dest + 0 * stride));
  d = _mm_castps_si128(
      _mm_loadh_pi(_mm_castsi128_ps(d), (const __m64 *)(dest + 1 * stride)));
  d = add_clamp(d, in, bd);
  _mm_storel_epi64((__m128i *)(dest + 0 * stride), d);
  _mm_storeh_pi((__m64 *)(dest + 1 * stride), _mm_castsi128_ps(d));
}

static INLINE void recon_and_store_4(const __m128i *const in, uint16_t *dest,
                                     const int stride, const int bd) {
  recon_and_store_4_dual(in[0], dest, stride, bd);
  dest += 2 * stride;
  recon_and_store_4_dual(in[1], dest, stride, bd);
}

static INLINE void recon_and_store_8_kernel(const __m128i in,
                                            uint16_t **const dest,
                                            const int stride, const int bd) {
  __m128i d;

  d = _mm_load_si128((const __m128i *)(*dest));
  d = add_clamp(d, in, bd);
  _mm_store_si128((__m128i *)(*dest), d);
  *dest += stride;
}

static INLINE void recon_and_store_8(const __m128i *const in, uint16_t *dest,
                                     const int stride, const int bd) {
  recon_and_store_8_kernel(in[0], &dest, stride, bd);
  recon_and_store_8_kernel(in[1], &dest, stride, bd);
  recon_and_store_8_kernel(in[2], &dest, stride, bd);
  recon_and_store_8_kernel(in[3], &dest, stride, bd);
  recon_and_store_8_kernel(in[4], &dest, stride, bd);
  recon_and_store_8_kernel(in[5], &dest, stride, bd);
  recon_and_store_8_kernel(in[6], &dest, stride, bd);
  recon_and_store_8_kernel(in[7], &dest, stride, bd);
}

#endif  // VPX_DSP_X86_HIGHBD_INV_TXFM_SSE2_H_
