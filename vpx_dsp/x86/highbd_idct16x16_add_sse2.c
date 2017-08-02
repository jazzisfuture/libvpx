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

static INLINE __m128i load_pack_8_32bit(const tran_low_t *const input) {
  const __m128i t0 = _mm_load_si128((const __m128i *)(input + 0));
  const __m128i t1 = _mm_load_si128((const __m128i *)(input + 4));
  return _mm_packs_epi32(t0, t1);
}

static INLINE void highbd_write_buffer_8x1(uint16_t *dest, const __m128i in,
                                           const int bd) {
  const __m128i final_rounding = _mm_set1_epi16(1 << 5);
  __m128i out;

  out = _mm_adds_epi16(in, final_rounding);
  out = _mm_srai_epi16(out, 6);
  recon_and_store_8_kernel(out, &dest, 0, bd);
}

static INLINE void recon_and_store_4_kernel(const __m128i in,
                                            uint16_t *const dest,
                                            const int bd) {
  __m128i d;

  d = _mm_loadl_epi64((const __m128i *)dest);
  d = add_clamp(d, in, bd);
  _mm_storel_epi64((__m128i *)dest, d);
}

static INLINE void highbd_write_buffer_4x1(uint16_t *const dest,
                                           const __m128i in, const int bd) {
  const __m128i final_rounding = _mm_set1_epi32(1 << 5);
  __m128i out;

  out = _mm_add_epi32(in, final_rounding);
  out = _mm_srai_epi32(out, 6);
  out = _mm_packs_epi32(out, out);
  recon_and_store_4_kernel(out, dest, bd);
}

static INLINE void highbd_idct16_4col_stage5(const __m128i *const in,
                                             __m128i *const out) {
  __m128i temp1[2], temp2, sign[2];
  // stage 5
  out[0] = _mm_add_epi32(in[0], in[3]);
  out[1] = _mm_add_epi32(in[1], in[2]);
  out[2] = _mm_sub_epi32(in[1], in[2]);
  out[3] = _mm_sub_epi32(in[0], in[3]);
  temp2 = _mm_sub_epi32(in[6], in[5]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[5] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  temp2 = _mm_add_epi32(in[6], in[5]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[6] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  out[8] = _mm_add_epi32(in[8], in[11]);
  out[9] = _mm_add_epi32(in[9], in[10]);
  out[10] = _mm_sub_epi32(in[9], in[10]);
  out[11] = _mm_sub_epi32(in[8], in[11]);
  out[12] = _mm_sub_epi32(in[15], in[12]);
  out[13] = _mm_sub_epi32(in[14], in[13]);
  out[14] = _mm_add_epi32(in[14], in[13]);
  out[15] = _mm_add_epi32(in[15], in[12]);
}

static INLINE void highbd_idct16_4col_stage6(const __m128i *const in,
                                             __m128i *const out) {
  __m128i temp1[2], temp2, sign[2];
  out[0] = _mm_add_epi32(in[0], in[7]);
  out[1] = _mm_add_epi32(in[1], in[6]);
  out[2] = _mm_add_epi32(in[2], in[5]);
  out[3] = _mm_add_epi32(in[3], in[4]);
  out[4] = _mm_sub_epi32(in[3], in[4]);
  out[5] = _mm_sub_epi32(in[2], in[5]);
  out[6] = _mm_sub_epi32(in[1], in[6]);
  out[7] = _mm_sub_epi32(in[0], in[7]);
  out[8] = in[8];
  out[9] = in[9];
  temp2 = _mm_sub_epi32(in[13], in[10]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[10] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  temp2 = _mm_add_epi32(in[13], in[10]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[13] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);

  temp2 = _mm_sub_epi32(in[12], in[11]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[11] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  temp2 = _mm_add_epi32(in[12], in[11]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  out[12] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  out[14] = in[14];
  out[15] = in[15];
}

static INLINE void highbd_idct16_4col_stage7(const __m128i *const in,
                                             __m128i *const out) {
  out[0] = _mm_add_epi32(in[0], in[15]);
  out[1] = _mm_add_epi32(in[1], in[14]);
  out[2] = _mm_add_epi32(in[2], in[13]);
  out[3] = _mm_add_epi32(in[3], in[12]);
  out[4] = _mm_add_epi32(in[4], in[11]);
  out[5] = _mm_add_epi32(in[5], in[10]);
  out[6] = _mm_add_epi32(in[6], in[9]);
  out[7] = _mm_add_epi32(in[7], in[8]);
  out[8] = _mm_sub_epi32(in[7], in[8]);
  out[9] = _mm_sub_epi32(in[6], in[9]);
  out[10] = _mm_sub_epi32(in[5], in[10]);
  out[11] = _mm_sub_epi32(in[4], in[11]);
  out[12] = _mm_sub_epi32(in[3], in[12]);
  out[13] = _mm_sub_epi32(in[2], in[13]);
  out[14] = _mm_sub_epi32(in[1], in[14]);
  out[15] = _mm_sub_epi32(in[0], in[15]);
}

static INLINE void highbd_idct16_4col(__m128i *const io /*io[16]*/) {
  __m128i step1[16], step2[16];
  __m128i temp1[4], temp2, sign[2];

  // stage 2
  highbd_multiplication_and_add_sse2(io[1], io[15], (int)cospi_30_64,
                                     (int)cospi_2_64, &step2[8], &step2[15]);
  highbd_multiplication_and_add_sse2(io[9], io[7], (int)cospi_14_64,
                                     (int)cospi_18_64, &step2[9], &step2[14]);
  highbd_multiplication_and_add_sse2(io[5], io[11], (int)cospi_22_64,
                                     (int)cospi_10_64, &step2[10], &step2[13]);
  highbd_multiplication_and_add_sse2(io[13], io[3], (int)cospi_6_64,
                                     (int)cospi_26_64, &step2[11], &step2[12]);

  // stage 3
  highbd_multiplication_and_add_sse2(io[2], io[14], (int)cospi_28_64,
                                     (int)cospi_4_64, &step1[4], &step1[7]);
  highbd_multiplication_and_add_sse2(io[10], io[6], (int)cospi_12_64,
                                     (int)cospi_20_64, &step1[5], &step1[6]);
  step1[8] = _mm_add_epi32(step2[8], step2[9]);
  step1[9] = _mm_sub_epi32(step2[8], step2[9]);
  step1[10] = _mm_sub_epi32(step2[10], step2[11]);  // step1[10] = -step1[10]
  step1[11] = _mm_add_epi32(step2[10], step2[11]);
  step1[12] = _mm_add_epi32(step2[13], step2[12]);
  step1[13] = _mm_sub_epi32(step2[13], step2[12]);  // step1[13] = -step1[13]
  step1[14] = _mm_sub_epi32(step2[15], step2[14]);
  step1[15] = _mm_add_epi32(step2[15], step2[14]);

  // stage 4
  temp2 = _mm_add_epi32(io[0], io[8]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  step2[0] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  temp2 = _mm_sub_epi32(io[0], io[8]);
  abs_extend_64bit_sse2(temp2, temp1, sign);
  step2[1] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  highbd_multiplication_and_add_sse2(io[4], io[12], (int)cospi_24_64,
                                     (int)cospi_8_64, &step2[2], &step2[3]);
  highbd_multiplication_and_add_sse2(step1[14], step1[9], (int)cospi_24_64,
                                     (int)cospi_8_64, &step2[9], &step2[14]);
  highbd_multiplication_and_add_sse2(step1[10], step1[13], (int)cospi_8_64,
                                     (int)cospi_24_64, &step2[13], &step2[10]);
  step2[5] = _mm_sub_epi32(step1[4], step1[5]);
  step1[4] = _mm_add_epi32(step1[4], step1[5]);
  step2[6] = _mm_sub_epi32(step1[7], step1[6]);
  step1[7] = _mm_add_epi32(step1[7], step1[6]);
  step2[8] = step1[8];
  step2[11] = step1[11];
  step2[12] = step1[12];
  step2[15] = step1[15];

  highbd_idct16_4col_stage5(step2, step1);
  highbd_idct16_4col_stage6(step1, step2);
  highbd_idct16_4col_stage7(step2, io);
}

static INLINE void highbd_idct16x16_38_4col(__m128i *const io /*io[16]*/) {
  __m128i step1[16], step2[16];
  __m128i temp1[2], sign[2];

  // stage 2
  highbd_multiplication_sse2(io[1], (int)cospi_30_64, (int)cospi_2_64,
                             &step2[8], &step2[15]);
  highbd_multiplication_neg_sse2(io[7], (int)cospi_14_64, (int)cospi_18_64,
                                 &step2[9], &step2[14]);
  highbd_multiplication_sse2(io[5], (int)cospi_22_64, (int)cospi_10_64,
                             &step2[10], &step2[13]);
  highbd_multiplication_neg_sse2(io[3], (int)cospi_6_64, (int)cospi_26_64,
                                 &step2[11], &step2[12]);

  // stage 3
  highbd_multiplication_sse2(io[2], (int)cospi_28_64, (int)cospi_4_64,
                             &step1[4], &step1[7]);
  highbd_multiplication_neg_sse2(io[6], (int)cospi_12_64, (int)cospi_20_64,
                                 &step1[5], &step1[6]);
  step1[8] = _mm_add_epi32(step2[8], step2[9]);
  step1[9] = _mm_sub_epi32(step2[8], step2[9]);
  step1[10] = _mm_sub_epi32(step2[10], step2[11]);  // step1[10] = -step1[10]
  step1[11] = _mm_add_epi32(step2[10], step2[11]);
  step1[12] = _mm_add_epi32(step2[13], step2[12]);
  step1[13] = _mm_sub_epi32(step2[13], step2[12]);  // step1[13] = -step1[13]
  step1[14] = _mm_sub_epi32(step2[15], step2[14]);
  step1[15] = _mm_add_epi32(step2[15], step2[14]);

  // stage 4
  abs_extend_64bit_sse2(io[0], temp1, sign);
  step2[0] = multiplication_round_shift_sse2(temp1, sign, (int)cospi_16_64);
  step2[1] = step2[0];
  highbd_multiplication_sse2(io[4], (int)cospi_24_64, (int)cospi_8_64,
                             &step2[2], &step2[3]);
  highbd_multiplication_and_add_sse2(step1[14], step1[9], (int)cospi_24_64,
                                     (int)cospi_8_64, &step2[9], &step2[14]);
  highbd_multiplication_and_add_sse2(step1[10], step1[13], (int)cospi_8_64,
                                     (int)cospi_24_64, &step2[13], &step2[10]);
  step2[5] = _mm_sub_epi32(step1[4], step1[5]);
  step1[4] = _mm_add_epi32(step1[4], step1[5]);
  step2[6] = _mm_sub_epi32(step1[7], step1[6]);
  step1[7] = _mm_add_epi32(step1[7], step1[6]);
  step2[8] = step1[8];
  step2[11] = step1[11];
  step2[12] = step1[12];
  step2[15] = step1[15];

  highbd_idct16_4col_stage5(step2, step1);
  highbd_idct16_4col_stage6(step1, step2);
  highbd_idct16_4col_stage7(step2, io);
}

static INLINE void highbd_idct16x16_10_4col(__m128i *const io /*io[16]*/) {
  __m128i step1[16], step2[16];
  __m128i temp[2], sign[2];

  // stage 2
  highbd_multiplication_sse2(io[1], (int)cospi_30_64, (int)cospi_2_64,
                             &step2[8], &step2[15]);
  highbd_multiplication_neg_sse2(io[3], (int)cospi_6_64, (int)cospi_26_64,
                                 &step2[11], &step2[12]);

  // stage 3
  highbd_multiplication_sse2(io[2], (int)cospi_28_64, (int)cospi_4_64,
                             &step1[4], &step1[7]);
  step1[8] = step2[8];
  step1[9] = step2[8];
  step1[10] =
      _mm_sub_epi32(_mm_setzero_si128(), step2[11]);  // step1[10] = -step1[10]
  step1[11] = step2[11];
  step1[12] = step2[12];
  step1[13] =
      _mm_sub_epi32(_mm_setzero_si128(), step2[12]);  // step1[13] = -step1[13]
  step1[14] = step2[15];
  step1[15] = step2[15];

  // stage 4
  abs_extend_64bit_sse2(io[0], temp, sign);
  step2[0] = multiplication_round_shift_sse2(temp, sign, (int)cospi_16_64);
  step2[1] = step2[0];
  step2[2] = _mm_setzero_si128();
  step2[3] = _mm_setzero_si128();
  highbd_multiplication_and_add_sse2(step1[14], step1[9], (int)cospi_24_64,
                                     (int)cospi_8_64, &step2[9], &step2[14]);
  highbd_multiplication_and_add_sse2(step1[10], step1[13], (int)cospi_8_64,
                                     (int)cospi_24_64, &step2[13], &step2[10]);
  step2[5] = step1[4];
  step2[6] = step1[7];
  step2[8] = step1[8];
  step2[11] = step1[11];
  step2[12] = step1[12];
  step2[15] = step1[15];

  highbd_idct16_4col_stage5(step2, step1);
  highbd_idct16_4col_stage6(step1, step2);
  highbd_idct16_4col_stage7(step2, io);
}

void vpx_highbd_idct16x16_256_add_sse2(const tran_low_t *input, uint16_t *dest,
                                       int stride, int bd) {
  int i;
  __m128i out[16], *in;

  if (bd == 8) {
    __m128i l[16], r[16];

    in = l;
    for (i = 0; i < 2; i++) {
      in[0] = load_pack_8_32bit(input + 0 * 16);
      in[1] = load_pack_8_32bit(input + 1 * 16);
      in[2] = load_pack_8_32bit(input + 2 * 16);
      in[3] = load_pack_8_32bit(input + 3 * 16);
      in[4] = load_pack_8_32bit(input + 4 * 16);
      in[5] = load_pack_8_32bit(input + 5 * 16);
      in[6] = load_pack_8_32bit(input + 6 * 16);
      in[7] = load_pack_8_32bit(input + 7 * 16);
      transpose_16bit_8x8(in, in);

      in[8] = load_pack_8_32bit(input + 0 * 16 + 8);
      in[9] = load_pack_8_32bit(input + 1 * 16 + 8);
      in[10] = load_pack_8_32bit(input + 2 * 16 + 8);
      in[11] = load_pack_8_32bit(input + 3 * 16 + 8);
      in[12] = load_pack_8_32bit(input + 4 * 16 + 8);
      in[13] = load_pack_8_32bit(input + 5 * 16 + 8);
      in[14] = load_pack_8_32bit(input + 6 * 16 + 8);
      in[15] = load_pack_8_32bit(input + 7 * 16 + 8);
      transpose_16bit_8x8(in + 8, in + 8);
      idct16_8col(in);
      in = r;
      input += 128;
    }

    for (i = 0; i < 2; i++) {
      int j;
      transpose_16bit_8x8(l + i * 8, out);
      transpose_16bit_8x8(r + i * 8, out + 8);
      idct16_8col(out);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_8x1(dest + j * stride, out[j], bd);
      }
      dest += 8;
    }
  } else {
    __m128i all[4][16];

    for (i = 0; i < 4; i++) {
      in = all[i];
      in[0] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 0));
      in[1] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 4));
      in[2] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 0));
      in[3] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 4));
      in[4] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 0));
      in[5] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 4));
      in[6] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 0));
      in[7] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 4));
      transpose_32bit_8x4(in, in);

      in[8] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 8));
      in[9] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 12));
      in[10] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 8));
      in[11] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 12));
      in[12] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 8));
      in[13] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 12));
      in[14] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 8));
      in[15] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 12));
      transpose_32bit_8x4(in + 8, in + 8);

      highbd_idct16_4col(in);
      input += 4 * 16;
    }

    for (i = 0; i < 4; i++) {
      int j;
      out[0] = all[0][4 * i + 0];
      out[1] = all[1][4 * i + 0];
      out[2] = all[0][4 * i + 1];
      out[3] = all[1][4 * i + 1];
      out[4] = all[0][4 * i + 2];
      out[5] = all[1][4 * i + 2];
      out[6] = all[0][4 * i + 3];
      out[7] = all[1][4 * i + 3];
      transpose_32bit_8x4(out, out);

      out[8] = all[2][4 * i + 0];
      out[9] = all[3][4 * i + 0];
      out[10] = all[2][4 * i + 1];
      out[11] = all[3][4 * i + 1];
      out[12] = all[2][4 * i + 2];
      out[13] = all[3][4 * i + 2];
      out[14] = all[2][4 * i + 3];
      out[15] = all[3][4 * i + 3];
      transpose_32bit_8x4(out + 8, out + 8);

      highbd_idct16_4col(out);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_4x1(dest + j * stride, out[j], bd);
      }
      dest += 4;
    }
  }
}

void vpx_highbd_idct16x16_38_add_sse2(const tran_low_t *input, uint16_t *dest,
                                      int stride, int bd) {
  int i;
  __m128i out[16];

  if (bd == 8) {
    __m128i in[16];

    in[0] = load_pack_8_32bit(input + 0 * 16);
    in[1] = load_pack_8_32bit(input + 1 * 16);
    in[2] = load_pack_8_32bit(input + 2 * 16);
    in[3] = load_pack_8_32bit(input + 3 * 16);
    in[4] = load_pack_8_32bit(input + 4 * 16);
    in[5] = load_pack_8_32bit(input + 5 * 16);
    in[6] = load_pack_8_32bit(input + 6 * 16);
    in[7] = load_pack_8_32bit(input + 7 * 16);
    transpose_16bit_8x8(in, in);

    in[8] = _mm_setzero_si128();
    in[9] = _mm_setzero_si128();
    in[10] = _mm_setzero_si128();
    in[11] = _mm_setzero_si128();
    in[12] = _mm_setzero_si128();
    in[13] = _mm_setzero_si128();
    in[14] = _mm_setzero_si128();
    in[15] = _mm_setzero_si128();
    idct16_8col(in);

    for (i = 0; i < 2; i++) {
      int j;
      transpose_16bit_8x8(in + i * 8, out);
      out[8] = _mm_setzero_si128();
      out[9] = _mm_setzero_si128();
      out[10] = _mm_setzero_si128();
      out[11] = _mm_setzero_si128();
      out[12] = _mm_setzero_si128();
      out[13] = _mm_setzero_si128();
      out[14] = _mm_setzero_si128();
      out[15] = _mm_setzero_si128();
      idct16_8col(out);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_8x1(dest + j * stride, out[j], bd);
      }
      dest += 8;
    }
  } else {
    __m128i all[2][16], *in;

    for (i = 0; i < 2; i++) {
      in = all[i];
      in[0] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 0));
      in[1] = _mm_load_si128((const __m128i *)(input + 0 * 16 + 4));
      in[2] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 0));
      in[3] = _mm_load_si128((const __m128i *)(input + 1 * 16 + 4));
      in[4] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 0));
      in[5] = _mm_load_si128((const __m128i *)(input + 2 * 16 + 4));
      in[6] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 0));
      in[7] = _mm_load_si128((const __m128i *)(input + 3 * 16 + 4));
      transpose_32bit_8x4(in, in);
      highbd_idct16x16_38_4col(in);
      input += 4 * 16;
    }

    for (i = 0; i < 4; i++) {
      int j;
      out[0] = all[0][4 * i + 0];
      out[1] = all[1][4 * i + 0];
      out[2] = all[0][4 * i + 1];
      out[3] = all[1][4 * i + 1];
      out[4] = all[0][4 * i + 2];
      out[5] = all[1][4 * i + 2];
      out[6] = all[0][4 * i + 3];
      out[7] = all[1][4 * i + 3];
      transpose_32bit_8x4(out, out);
      highbd_idct16x16_38_4col(out);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_4x1(dest + j * stride, out[j], bd);
      }
      dest += 4;
    }
  }
}

void vpx_highbd_idct16x16_10_add_sse2(const tran_low_t *input, uint16_t *dest,
                                      int stride, int bd) {
  int i;
  __m128i out[16];

  if (bd == 8) {
    __m128i in[16], l[16];

    in[0] = load_pack_8_32bit(input + 0 * 16);
    in[1] = load_pack_8_32bit(input + 1 * 16);
    in[2] = load_pack_8_32bit(input + 2 * 16);
    in[3] = load_pack_8_32bit(input + 3 * 16);

    idct16x16_10_pass1(in, l);

    for (i = 0; i < 2; i++) {
      int j;
      idct16x16_10_pass2(l + 8 * i, in);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_8x1(dest + j * stride, in[j], bd);
      }
      dest += 8;
    }
  } else {
    __m128i all[2][16], *in;

    for (i = 0; i < 2; i++) {
      in = all[i];
      in[0] = _mm_load_si128((const __m128i *)(input + 0 * 16));
      in[1] = _mm_load_si128((const __m128i *)(input + 1 * 16));
      in[2] = _mm_load_si128((const __m128i *)(input + 2 * 16));
      in[3] = _mm_load_si128((const __m128i *)(input + 3 * 16));
      transpose_32bit_4x4(in, in);
      highbd_idct16x16_10_4col(in);
      input += 4 * 16;
    }

    for (i = 0; i < 4; i++) {
      int j;
      transpose_32bit_4x4(&all[0][4 * i], out);
      highbd_idct16x16_10_4col(out);

      for (j = 0; j < 16; ++j) {
        highbd_write_buffer_4x1(dest + j * stride, out[j], bd);
      }
      dest += 4;
    }
  }
}

void vpx_highbd_idct16x16_1_add_sse2(const tran_low_t *input, uint16_t *dest,
                                     int stride, int bd) {
  highbd_idct_1_add_kernel(input, dest, stride, bd, 16);
}
