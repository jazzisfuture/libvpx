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
#include "vpx_dsp/fwd_txfm.h"
#include "vpx_dsp/ppc/types_vsx.h"
#include "vpx_dsp/ppc/transpose_vsx.h"

static const uint32x4_t dct_const_0 = { 13, 13, 13, 13 };
static const uint32x4_t dct_const_1 = { 14, 14, 14, 14 };

static const uint32x4_t vec_twos_u32 = { 2, 2, 2, 2 };
static const uint16x8_t vec_fours_u16 = { 4, 4, 4, 4, 4, 4, 4, 4 };

static const uint8x16_t perm_1_v = { 0x00, 0x01, 0x02, 0x03, 0x08, 0x09,
                                     0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13,
                                     0x18, 0x19, 0x1A, 0x1B };
static const uint8x16_t perm_2_v = { 0x06, 0x07, 0x04, 0x05, 0x0E, 0x0F,
                                     0x0C, 0x0D, 0x16, 0x17, 0x14, 0x15,
                                     0x1E, 0x1F, 0x1C, 0x1D };
static const uint8x16_t perm_3_v = { 0x02, 0x03, 0x10, 0x11, 0x02, 0x03,
                                     0x10, 0x11, 0x06, 0x07, 0x14, 0x15,
                                     0x06, 0x07, 0x14, 0x15 };
static const uint8x16_t perm_4_v = { 0x0A, 0x0B, 0x18, 0x19, 0x0A, 0x0B,
                                     0x18, 0x19, 0x0E, 0x0F, 0x1C, 0x1D,
                                     0x0E, 0x0F, 0x1C, 0x1D };
static const uint8x16_t perm_5_v = { 0x00, 0x01, 0x12, 0x13, 0x00, 0x01,
                                     0x12, 0x13, 0x04, 0x05, 0x16, 0x17,
                                     0x04, 0x05, 0x16, 0x17 };
static const uint8x16_t perm_6_v = { 0x08, 0x09, 0x1A, 0x1B, 0x08, 0x09,
                                     0x1A, 0x1B, 0x0C, 0x0D, 0x1E, 0x1F,
                                     0x0C, 0x0D, 0x1E, 0x1F };

static const int16x8_t cospi_1_v = { 11585, 15137, -11585, 6270,
                                     11585, 15137, -11585, 6270 };
static const int16x8_t cospi_2_v = { 11585, 6270, 11585, -15137,
                                     11585, 6270, 11585, -15137 };

static INLINE void vpx_round_shift_s32_4x4(const int32x4_t *input,
                                           int32x4_t *output) {
  int32x4_t tmp;
  int32x4_t sum[4];

  tmp = vec_sl(vec_ones_s32, dct_const_0);

  sum[0] = vec_add(input[0], tmp);
  sum[1] = vec_add(input[1], tmp);
  sum[2] = vec_add(input[2], tmp);
  sum[3] = vec_add(input[3], tmp);

  output[0] = vec_sra(sum[0], dct_const_1);
  output[1] = vec_sra(sum[1], dct_const_1);
  output[2] = vec_sra(sum[2], dct_const_1);
  output[3] = vec_sra(sum[3], dct_const_1);
}

static INLINE void vpx_fdct4x4_one_pass(const int16x8_t *input,
                                        int32x4_t *output) {
  int16x8_t v[2];
  int16x8_t s[2];
  int16x8_t a[2];
  int16x8_t b[2];

  int32x4_t b_e[2];
  int32x4_t b_o[2];
  int32x4_t a_e[2];
  int32x4_t a_o[2];
  int32x4_t b_h[2];
  int32x4_t b_l[2];
  int32x4_t a_h[2];
  int32x4_t a_l[2];

  v[0] = vec_perm(input[0], input[1], perm_1_v);
  v[1] = vec_perm(input[0], input[1], perm_2_v);

  s[0] = vec_add(v[0], v[1]);
  s[1] = vec_sub(v[0], v[1]);

  b[0] = vec_perm(s[0], s[1], perm_3_v);
  b[1] = vec_perm(s[0], s[1], perm_4_v);

  a[0] = vec_perm(s[0], s[1], perm_5_v);
  a[1] = vec_perm(s[0], s[1], perm_6_v);

  b_e[0] = vec_mule(b[0], cospi_1_v);
  b_o[0] = vec_mulo(b[0], cospi_1_v);
  b_h[0] = vec_mergeh(b_e[0], b_o[0]);
  b_l[0] = vec_mergel(b_e[0], b_o[0]);

  b_e[1] = vec_mule(b[1], cospi_1_v);
  b_o[1] = vec_mulo(b[1], cospi_1_v);
  b_h[1] = vec_mergeh(b_e[1], b_o[1]);
  b_l[1] = vec_mergel(b_e[1], b_o[1]);

  a_e[0] = vec_mule(a[0], cospi_2_v);
  a_o[0] = vec_mulo(a[0], cospi_2_v);
  a_h[0] = vec_mergeh(a_e[0], a_o[0]);
  a_l[0] = vec_mergel(a_e[0], a_o[0]);

  a_e[1] = vec_mule(a[1], cospi_2_v);
  a_o[1] = vec_mulo(a[1], cospi_2_v);
  a_h[1] = vec_mergeh(a_e[1], a_o[1]);
  a_l[1] = vec_mergel(a_e[1], a_o[1]);

  output[0] = vec_add(a_h[0], b_h[0]);
  output[1] = vec_add(a_l[0], b_l[0]);
  output[2] = vec_add(a_h[1], b_h[1]);
  output[3] = vec_add(a_l[1], b_l[1]);
}

void vpx_fdct4x4_vsx(const int16_t *input, tran_low_t *output, int stride) {
  int16x8_t v[2];
  int32x4_t v_out[4];
  int32x4_t loaded[4];
  int32x4_t rounded[4];
  int32x4_t transposed[4];
  int32x4_t transformed[4];

  loaded[0] = unpack_u16_to_s32_h(vec_vsx_ld(0, input));
  loaded[1] = unpack_u16_to_s32_h(vec_vsx_ld(0, input + stride));
  loaded[2] = unpack_u16_to_s32_h(vec_vsx_ld(0, input + stride * 2));
  loaded[3] = unpack_u16_to_s32_h(vec_vsx_ld(0, input + stride * 3));

  vpx_transpose_s32_4x4(loaded, transposed);

#ifdef WORDS_BIGENDIAN
  v[0] = vec_pack(transposed[1], transposed[0]);
  v[1] = vec_pack(transposed[3], transposed[2]);
#else
  v[0] = vec_pack(transposed[0], transposed[1]);
  v[1] = vec_pack(transposed[2], transposed[3]);
#endif  // WORDS_BIGENDIAN

  v[0] = vec_sl(v[0], vec_fours_u16);
  v[1] = vec_sl(v[1], vec_fours_u16);

  if (v[0][0]) {
    ++v[0][0];
  }

  vpx_fdct4x4_one_pass(v, transformed);

  vpx_round_shift_s32_4x4(transformed, rounded);

  vpx_transpose_s32_4x4(rounded, transposed);

#ifdef WORDS_BIGENDIAN
  v[0] = vec_pack(transposed[1], transposed[0]);
  v[1] = vec_pack(transposed[3], transposed[2]);
#else
  v[0] = vec_pack(transposed[0], transposed[1]);
  v[1] = vec_pack(transposed[2], transposed[3]);
#endif  // WORDS_BIGENDIAN

  vpx_fdct4x4_one_pass(v, transformed);

  vpx_round_shift_s32_4x4(transformed, rounded);

  v_out[0] = vec_add(rounded[0], vec_ones_s32);
  v_out[1] = vec_add(rounded[1], vec_ones_s32);
  v_out[2] = vec_add(rounded[2], vec_ones_s32);
  v_out[3] = vec_add(rounded[3], vec_ones_s32);

  v_out[0] = vec_sra(v_out[0], vec_twos_u32);
  v_out[1] = vec_sra(v_out[1], vec_twos_u32);
  v_out[2] = vec_sra(v_out[2], vec_twos_u32);
  v_out[3] = vec_sra(v_out[3], vec_twos_u32);

#ifdef WORDS_BIGENDIAN
  v[0] = vec_pack(v_out[1], v_out[0]);
  v[1] = vec_pack(v_out[3], v_out[2]);
#else
  v[0] = vec_pack(v_out[0], v_out[1]);
  v[1] = vec_pack(v_out[2], v_out[3]);
#endif  // WORDS_BIGENDIAN

  vec_vsx_st(v[0], 0, output);
  vec_vsx_st(v[1], 0, output + 8);
}
