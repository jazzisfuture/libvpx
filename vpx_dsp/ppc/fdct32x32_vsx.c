/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_dsp_rtcd.h"

#include "vpx_dsp/ppc/types_vsx.h"
#include "vpx_dsp/ppc/txfm_common_vsx.h"

static const int32x4_t vec_dct_const_rounding = { 8192, 8192, 8192, 8192 };
static const uint32x4_t vec_dct_const_bits = { 14, 14, 14, 14 };
static const uint16x8_t vec_dct_scale_log2 = { 2, 2, 2, 2, 2, 2, 2, 2 };

// Negate 16-bit integers
static INLINE int16x8_t vec_neg(int16x8_t a) {
  return vec_add(vec_xor(a, vec_splats((int16_t)-1)), vec_ones_s16);
}

static INLINE int16x8_t vec_sign_s16(int16x8_t a) {
  return vec_sr(a, vec_shift_sign_s16);
}

// Returns ((a + b) * c + (2 << 13)) >> 14, since a + b can overflow 16 bits,
// the multiplication is distributed a * c + b * c. Both multiplications are
// done in 32-bit lanes.
static INLINE int16x8_t single_butterfly(int16x8_t a, int16x8_t b,
                                         int16x8_t c) {
  const int32x4_t ac_e = vec_mule(a, c);
  const int32x4_t ac_o = vec_mulo(a, c);
  const int32x4_t bc_e = vec_mule(b, c);
  const int32x4_t bc_o = vec_mulo(b, c);

  const int32x4_t sum_e = vec_add(ac_e, bc_e);
  const int32x4_t sum_o = vec_add(ac_o, bc_o);
  const int32x4_t rsum_o = vec_add(sum_o, vec_dct_const_rounding);
  const int32x4_t rsum_e = vec_add(sum_e, vec_dct_const_rounding);

  const int32x4_t ssum_o = vec_sra(rsum_o, vec_dct_const_bits);
  const int32x4_t ssum_e = vec_sra(rsum_e, vec_dct_const_bits);

  const int16x8_t pack = vec_pack(ssum_e, ssum_o);
  return vec_perm(pack, pack, vec_perm_merge);
}

// TODO(ltrudeau) use two out parameters
static INLINE int16x8_t double_butterfly(int16x8_t in1, int16x8_t cos1,
                                         int16x8_t in2, int16x8_t cos2) {
  const int32x4_t mulo_1 = vec_mulo(in1, cos1);
  const int32x4_t mule_1 = vec_mule(in1, cos1);
  const int32x4_t mulo_2 = vec_mulo(in2, cos2);
  const int32x4_t mule_2 = vec_mule(in2, cos2);

  const int32x4_t sum_o = vec_add(mulo_1, mulo_2);
  const int32x4_t sum_e = vec_add(mule_1, mule_2);
  const int32x4_t rsum_o = vec_add(sum_o, vec_dct_const_rounding);
  const int32x4_t rsum_e = vec_add(sum_e, vec_dct_const_rounding);

  const int32x4_t ssum_o = vec_sra(rsum_o, vec_dct_const_bits);
  const int32x4_t ssum_e = vec_sra(rsum_e, vec_dct_const_bits);
  const int16x8_t pack = vec_pack(ssum_e, ssum_o);

  return vec_perm(pack, pack, vec_perm_merge);
}

static INLINE void load(const int16_t *a, int stride, int16x8_t *b) {
  // TODO(ltrudeau) Unroll loop
  for (int i = 0; i < 32; i++) {
    b[i] = vec_vsx_ld(0, a + i * stride);
    b[i] = vec_sl(b[i], vec_dct_scale_log2);
  }
}

static INLINE void store(tran_low_t *a, const int16x8_t *b) {
  for (int i = 0; i < 8; i++) {
    vec_vsx_st(b[0], 0, a);
    vec_vsx_st(b[8], 0, a + 8);
    vec_vsx_st(b[16], 0, a + 16);
    vec_vsx_st(b[24], 0, a + 24);
    a += 32;
    b++;
  }
}

// Add 2 if positive, 1 if negative, and shift by 2.
static INLINE int16x8_t sub_round_shift(const int16x8_t a) {
  const int16x8_t sign = vec_sign_s16(a);
  return vec_sra(vec_sub(vec_add(a, vec_twos_s16), sign), vec_dct_scale_log2);
}

// Add 1 if positive, 2 if negative, and shift by 2.
// In practice, add 1, then add the sign bit, then shift without rounding.
static INLINE int16x8_t add_round_shift_s16(const int16x8_t a) {
  const int16x8_t sign = vec_sign_s16(a);
  return vec_sra(vec_add(vec_add(a, vec_ones_s16), sign), vec_dct_scale_log2);
}

void vpx_fdct32_vsx(const int16x8_t *in, int16x8_t *out, int pass) {
  int16x8_t step[32];
  int16x8_t output[32];

  // Stage 1
  for (int i = 0; i < 16; i++) {
    step[i] = vec_add(in[i], in[31 - i]);
    step[i + 16] = vec_sub(in[15 - i], in[i + 16]);
  }

  // Stage 2
  for (int i = 0; i < 8; i++) {
    output[i] = vec_add(step[i], step[15 - i]);
    output[i + 8] = vec_sub(step[7 - i], step[i + 8]);
  }

  output[16] = step[16];
  output[17] = step[17];
  output[18] = step[18];
  output[19] = step[19];

  output[20] = single_butterfly(vec_neg(step[20]), step[27], cospi16_v);
  output[21] = single_butterfly(vec_neg(step[21]), step[26], cospi16_v);
  output[22] = single_butterfly(vec_neg(step[22]), step[25], cospi16_v);
  output[23] = single_butterfly(vec_neg(step[23]), step[24], cospi16_v);

  output[24] = single_butterfly(step[24], step[23], cospi16_v);
  output[25] = single_butterfly(step[25], step[22], cospi16_v);
  output[26] = single_butterfly(step[26], step[21], cospi16_v);
  output[27] = single_butterfly(step[27], step[20], cospi16_v);

  output[28] = step[28];
  output[29] = step[29];
  output[30] = step[30];
  output[31] = step[31];

  // dump the magnitude by 4, hence the intermediate values are within
  // the range of 16 bits.
  if (pass) {
    output[0] = add_round_shift_s16(output[0]);
    output[1] = add_round_shift_s16(output[1]);
    output[2] = add_round_shift_s16(output[2]);
    output[3] = add_round_shift_s16(output[3]);
    output[4] = add_round_shift_s16(output[4]);
    output[5] = add_round_shift_s16(output[5]);
    output[6] = add_round_shift_s16(output[6]);
    output[7] = add_round_shift_s16(output[7]);
    output[8] = add_round_shift_s16(output[8]);
    output[9] = add_round_shift_s16(output[9]);
    output[10] = add_round_shift_s16(output[10]);
    output[11] = add_round_shift_s16(output[11]);
    output[12] = add_round_shift_s16(output[12]);
    output[13] = add_round_shift_s16(output[13]);
    output[14] = add_round_shift_s16(output[14]);
    output[15] = add_round_shift_s16(output[15]);

    output[16] = add_round_shift_s16(output[16]);
    output[17] = add_round_shift_s16(output[17]);
    output[18] = add_round_shift_s16(output[18]);
    output[19] = add_round_shift_s16(output[19]);
    output[20] = add_round_shift_s16(output[20]);
    output[21] = add_round_shift_s16(output[21]);
    output[22] = add_round_shift_s16(output[22]);
    output[23] = add_round_shift_s16(output[23]);
    output[24] = add_round_shift_s16(output[24]);
    output[25] = add_round_shift_s16(output[25]);
    output[26] = add_round_shift_s16(output[26]);
    output[27] = add_round_shift_s16(output[27]);
    output[28] = add_round_shift_s16(output[28]);
    output[29] = add_round_shift_s16(output[29]);
    output[30] = add_round_shift_s16(output[30]);
    output[31] = add_round_shift_s16(output[31]);
  }

  // Stage 3
  step[0] = vec_add(output[0], output[7]);
  step[1] = vec_add(output[1], output[6]);
  step[2] = vec_add(output[2], output[5]);
  step[3] = vec_add(output[3], output[4]);
  step[4] = vec_sub(output[3], output[4]);
  step[5] = vec_sub(output[2], output[5]);
  step[6] = vec_sub(output[1], output[6]);
  step[7] = vec_sub(output[0], output[7]);
  step[8] = output[8];
  step[9] = output[9];
  step[10] = single_butterfly(vec_neg(output[10]), output[13], cospi16_v);
  step[11] = single_butterfly(vec_neg(output[11]), output[12], cospi16_v);
  step[12] = single_butterfly(output[12], output[11], cospi16_v);
  step[13] = single_butterfly(output[13], output[10], cospi16_v);
  step[14] = output[14];
  step[15] = output[15];

  step[16] = vec_add(output[16], output[23]);
  step[17] = vec_add(output[17], output[22]);
  step[18] = vec_add(output[18], output[21]);
  step[19] = vec_add(output[19], output[20]);

  step[20] = vec_sub(output[19], output[20]);
  step[21] = vec_sub(output[18], output[21]);
  step[22] = vec_sub(output[17], output[22]);
  step[23] = vec_sub(output[16], output[23]);
  step[24] = vec_sub(output[31], output[24]);
  step[25] = vec_sub(output[30], output[25]);
  step[26] = vec_sub(output[29], output[26]);
  step[27] = vec_sub(output[28], output[27]);

  step[28] = vec_add(output[28], output[27]);
  step[29] = vec_add(output[29], output[26]);
  step[30] = vec_add(output[30], output[25]);
  step[31] = vec_add(output[31], output[24]);

  // Stage 4
  output[0] = vec_add(step[0], step[3]);
  output[1] = vec_add(step[1], step[2]);
  output[2] = vec_sub(step[1], step[2]);
  output[3] = vec_sub(step[0], step[3]);
  output[4] = step[4];
  output[5] = single_butterfly(vec_neg(step[5]), step[6], cospi16_v);
  output[6] = single_butterfly(step[6], step[5], cospi16_v);
  output[7] = step[7];
  output[8] = vec_add(step[8], step[11]);
  output[9] = vec_add(step[9], step[10]);
  output[10] = vec_sub(step[9], step[10]);
  output[11] = vec_sub(step[8], step[11]);
  output[12] = vec_sub(step[15], step[12]);
  output[13] = vec_sub(step[14], step[13]);
  output[14] = vec_add(step[14], step[13]);
  output[15] = vec_add(step[15], step[12]);

  output[16] = step[16];
  output[17] = step[17];
  output[18] =
      double_butterfly(step[18], vec_neg(cospi8_v), step[29], cospi24_v);
  output[19] =
      double_butterfly(step[19], vec_neg(cospi8_v), step[28], cospi24_v);
  output[20] = double_butterfly(step[20], vec_neg(cospi24_v), step[27],
                                vec_neg(cospi8_v));
  output[21] = double_butterfly(step[21], vec_neg(cospi24_v), step[26],
                                vec_neg(cospi8_v));
  output[22] = step[22];
  output[23] = step[23];
  output[24] = step[24];
  output[25] = step[25];
  output[26] =
      double_butterfly(step[26], cospi24_v, step[21], vec_neg(cospi8_v));
  output[27] =
      double_butterfly(step[27], cospi24_v, step[20], vec_neg(cospi8_v));
  output[28] = double_butterfly(step[28], cospi8_v, step[19], cospi24_v);
  output[29] = double_butterfly(step[29], cospi8_v, step[18], cospi24_v);
  output[30] = step[30];
  output[31] = step[31];

  // Stage 5
  step[0] = single_butterfly(output[0], output[1], cospi16_v);
  step[1] = single_butterfly(vec_neg(output[1]), output[0], cospi16_v);
  step[2] = double_butterfly(output[2], cospi24_v, output[3], cospi8_v);
  step[3] =
      double_butterfly(output[3], cospi24_v, output[2], vec_neg(cospi8_v));
  step[4] = vec_add(output[4], output[5]);
  step[5] = vec_sub(output[4], output[5]);
  step[6] = vec_sub(output[7], output[6]);
  step[7] = vec_add(output[7], output[6]);
  step[8] = output[8];
  step[9] =
      double_butterfly(output[9], vec_neg(cospi8_v), output[14], cospi24_v);
  step[10] = double_butterfly(output[10], vec_neg(cospi24_v), output[13],
                              vec_neg(cospi8_v));
  step[11] = output[11];
  step[12] = output[12];
  step[13] =
      double_butterfly(output[13], cospi24_v, output[10], vec_neg(cospi8_v));
  step[14] = double_butterfly(output[14], cospi8_v, output[9], cospi24_v);
  step[15] = output[15];

  step[16] = vec_add(output[16], output[19]);
  step[17] = vec_add(output[17], output[18]);
  step[18] = vec_sub(output[17], output[18]);
  step[19] = vec_sub(output[16], output[19]);
  step[20] = vec_sub(output[23], output[20]);
  step[21] = vec_sub(output[22], output[21]);
  step[22] = vec_add(output[22], output[21]);
  step[23] = vec_add(output[23], output[20]);
  step[24] = vec_add(output[24], output[27]);
  step[25] = vec_add(output[25], output[26]);
  step[26] = vec_sub(output[25], output[26]);
  step[27] = vec_sub(output[24], output[27]);
  step[28] = vec_sub(output[31], output[28]);
  step[29] = vec_sub(output[30], output[29]);
  step[30] = vec_add(output[30], output[29]);
  step[31] = vec_add(output[31], output[28]);

  // Stage 6
  output[0] = step[0];
  output[1] = step[1];
  output[2] = step[2];
  output[3] = step[3];
  output[4] = double_butterfly(step[4], cospi28_v, step[7], cospi4_v);
  output[5] = double_butterfly(step[5], cospi12_v, step[6], cospi20_v);
  output[6] = double_butterfly(step[6], cospi12_v, step[5], vec_neg(cospi20_v));
  output[7] = double_butterfly(step[7], cospi28_v, step[4], vec_neg(cospi4_v));
  output[8] = vec_add(step[8], step[9]);
  output[9] = vec_sub(step[8], step[9]);
  output[10] = vec_sub(step[11], step[10]);
  output[11] = vec_add(step[11], step[10]);
  output[12] = vec_add(step[12], step[13]);
  output[13] = vec_sub(step[12], step[13]);
  output[14] = vec_sub(step[15], step[14]);
  output[15] = vec_add(step[15], step[14]);

  output[16] = step[16];
  output[17] =
      double_butterfly(step[17], vec_neg(cospi4_v), step[30], cospi28_v);
  output[18] = double_butterfly(step[18], vec_neg(cospi28_v), step[29],
                                vec_neg(cospi4_v));
  output[19] = step[19];
  output[20] = step[20];
  output[21] =
      double_butterfly(step[21], vec_neg(cospi20_v), step[26], cospi12_v);
  output[22] = double_butterfly(step[22], vec_neg(cospi12_v), step[25],
                                vec_neg(cospi20_v));
  output[23] = step[23];
  output[24] = step[24];
  output[25] =
      double_butterfly(step[25], cospi12_v, step[22], vec_neg(cospi20_v));
  output[26] = double_butterfly(step[26], cospi20_v, step[21], cospi12_v);
  output[27] = step[27];
  output[28] = step[28];
  output[29] =
      double_butterfly(step[29], cospi28_v, step[18], vec_neg(cospi4_v));
  output[30] = double_butterfly(step[30], cospi4_v, step[17], cospi28_v);
  output[31] = step[31];

  // Stage 7
  step[0] = output[0];
  step[1] = output[1];
  step[2] = output[2];
  step[3] = output[3];
  step[4] = output[4];
  step[5] = output[5];
  step[6] = output[6];
  step[7] = output[7];
  step[8] = double_butterfly(output[8], cospi30_v, output[15], cospi2_v);
  step[9] = double_butterfly(output[9], cospi14_v, output[14], cospi18_v);
  step[10] = double_butterfly(output[10], cospi22_v, output[13], cospi10_v);
  step[11] = double_butterfly(output[11], cospi6_v, output[12], cospi26_v);
  step[12] =
      double_butterfly(output[12], cospi6_v, output[11], vec_neg(cospi26_v));
  step[13] =
      double_butterfly(output[13], cospi22_v, output[10], vec_neg(cospi10_v));
  step[14] =
      double_butterfly(output[14], cospi14_v, output[9], vec_neg(cospi18_v));
  step[15] =
      double_butterfly(output[15], cospi30_v, output[8], vec_neg(cospi2_v));

  step[16] = vec_add(output[16], output[17]);
  step[17] = vec_sub(output[16], output[17]);
  step[18] = vec_sub(output[19], output[18]);
  step[19] = vec_add(output[19], output[18]);
  step[20] = vec_add(output[20], output[21]);
  step[21] = vec_sub(output[20], output[21]);
  step[22] = vec_sub(output[23], output[22]);
  step[23] = vec_add(output[23], output[22]);
  step[24] = vec_add(output[24], output[25]);
  step[25] = vec_sub(output[24], output[25]);
  step[26] = vec_sub(output[27], output[26]);
  step[27] = vec_add(output[27], output[26]);
  step[28] = vec_add(output[28], output[29]);
  step[29] = vec_sub(output[28], output[29]);
  step[30] = vec_sub(output[31], output[30]);
  step[31] = vec_add(output[31], output[30]);

  // Final stage --- outputs indices are bit-reversed.
  out[0] = step[0];
  out[16] = step[1];
  out[8] = step[2];
  out[24] = step[3];
  out[4] = step[4];
  out[20] = step[5];
  out[12] = step[6];
  out[28] = step[7];
  out[2] = step[8];
  out[18] = step[9];
  out[10] = step[10];
  out[26] = step[11];
  out[6] = step[12];
  out[22] = step[13];
  out[14] = step[14];
  out[30] = step[15];

  out[1] = double_butterfly(step[16], cospi31_v, step[31], cospi1_v);
  out[17] = double_butterfly(step[17], cospi15_v, step[30], cospi17_v);
  out[9] = double_butterfly(step[18], cospi23_v, step[29], cospi9_v);
  out[25] = double_butterfly(step[19], cospi7_v, step[28], cospi25_v);
  out[5] = double_butterfly(step[20], cospi27_v, step[27], cospi5_v);
  out[21] = double_butterfly(step[21], cospi11_v, step[26], cospi21_v);
  out[13] = double_butterfly(step[22], cospi19_v, step[25], cospi13_v);
  out[29] = double_butterfly(step[23], cospi3_v, step[24], cospi29_v);
  out[3] = double_butterfly(step[24], cospi3_v, step[23], vec_neg(cospi29_v));
  out[19] = double_butterfly(step[25], cospi19_v, step[22], vec_neg(cospi13_v));
  out[11] = double_butterfly(step[26], cospi11_v, step[21], vec_neg(cospi21_v));
  out[27] = double_butterfly(step[27], cospi27_v, step[20], vec_neg(cospi5_v));
  out[7] = double_butterfly(step[28], cospi7_v, step[19], vec_neg(cospi25_v));
  out[23] = double_butterfly(step[29], cospi23_v, step[18], vec_neg(cospi9_v));
  out[15] = double_butterfly(step[30], cospi15_v, step[17], vec_neg(cospi17_v));
  out[31] = double_butterfly(step[31], cospi31_v, step[16], vec_neg(cospi1_v));

  if (pass == 0) {
    for (int i = 0; i < 32; i++) {
      out[i] = sub_round_shift(out[i]);
    }
  }
}

void vpx_fdct32x32_rd_vsx(const int16_t *input, tran_low_t *out, int stride) {
  int16x8_t temp0[32];
  int16x8_t temp1[32];
  int16x8_t temp2[32];
  int16x8_t temp3[32];
  int16x8_t temp4[32];
  int16x8_t temp5[32];
  int16x8_t temp6[32];

  // Process in 8x32 columns.
  load(input, stride, temp0);
  vpx_fdct32_vsx(temp0, temp1, 0);

  load(input + 8, stride, temp0);
  vpx_fdct32_vsx(temp0, temp2, 0);

  load(input + 16, stride, temp0);
  vpx_fdct32_vsx(temp0, temp3, 0);

  load(input + 24, stride, temp0);
  vpx_fdct32_vsx(temp0, temp4, 0);

  // Generate the top row by munging the first set of 8 from each one together.
  transpose_8x8(&temp1[0], &temp0[0]);
  transpose_8x8(&temp2[0], &temp0[8]);
  transpose_8x8(&temp3[0], &temp0[16]);
  transpose_8x8(&temp4[0], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out, temp6);

  // Second row of 8x32.
  transpose_8x8(&temp1[8], &temp0[0]);
  transpose_8x8(&temp2[8], &temp0[8]);
  transpose_8x8(&temp3[8], &temp0[16]);
  transpose_8x8(&temp4[8], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 8 * 32, temp6);

  // Third row of 8x32
  transpose_8x8(&temp1[16], &temp0[0]);
  transpose_8x8(&temp2[16], &temp0[8]);
  transpose_8x8(&temp3[16], &temp0[16]);
  transpose_8x8(&temp4[16], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 16 * 32, temp6);

  // Final row of 8x32.
  transpose_8x8(&temp1[24], &temp0[0]);
  transpose_8x8(&temp2[24], &temp0[8]);
  transpose_8x8(&temp3[24], &temp0[16]);
  transpose_8x8(&temp4[24], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 24 * 32, temp6);
}
