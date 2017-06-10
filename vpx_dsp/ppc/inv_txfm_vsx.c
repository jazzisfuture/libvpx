/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "vpx_dsp/ppc/types_vsx.h"

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/inv_txfm.h"

static int16x8_t cospi24_v = { 6270, 6270, 6270, 6270, 6270, 6270, 6270, 6270 };
static int16x8_t cospi16_v = { 11585, 11585, 11585, 11585,
                               11585, 11585, 11585, 11585 };
static int16x8_t cospi8_v = { 15137, 15137, 15137, 15137,
                              15137, 15137, 15137, 15137 };

#define IDCT4(in0, in1, out0, out1)                                           \
  t0 = vec_add(in0, in1);                                                     \
  t1 = vec_sub(in0, in1);                                                     \
  tmp16_0 = vec_mergeh(t0, t1);                                               \
  temp1 = vec_sra(vec_add(vec_mule(tmp16_0, cospi16_v), shift), shift14);     \
  temp2 = vec_sra(vec_add(vec_mulo(tmp16_0, cospi16_v), shift), shift14);     \
                                                                              \
  tmp16_0 = vec_mergel(in0, in1);                                             \
  temp3 = vec_sub(vec_mule(tmp16_0, cospi24_v), vec_mulo(tmp16_0, cospi8_v)); \
  temp3 = vec_sra(vec_add(temp3, shift), shift14);                            \
  temp4 = vec_add(vec_mule(tmp16_0, cospi8_v), vec_mulo(tmp16_0, cospi24_v)); \
  temp4 = vec_sra(vec_add(temp4, shift), shift14);                            \
                                                                              \
  step0 = vec_packs(temp1, temp2);                                            \
  step1 = vec_packs(temp4, temp3);                                            \
  out0 = vec_add(step0, step1);                                               \
  out1 = vec_sub(step0, step1);                                               \
  out1 = vec_perm(out1, out1, mask0);

void vpx_idct4x4_16_add_vsx(const tran_low_t *input, uint8_t *dest,
                            int stride) {
  int32x4_t temp1, temp2, temp3, temp4;
  int16x8_t step0, step1, tmp16_0, tmp16_1, t_out0, t_out1;
  int32x4_t shift = vec_sl(vec_splat_s32(1), vec_splat_u32(13));
  uint32x4_t shift14 = vec_splat_u32(14);
  uint8x16_t mask0 = { 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
                       0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };
  uint8x16_t mask1 = { 0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,
                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
  uint8x16_t dst_mask0 = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  uint8x16_t dst_mask1 = { 0x4,  0x5,  0x6,  0x7,  0x14, 0x15, 0x16, 0x17,
                           0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
  uint8x16_t dst_mask2 = { 0x8,  0x9,  0xA,  0xB,  0x14, 0x15, 0x16, 0x17,
                           0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
  uint8x16_t dst_mask3 = { 0xC,  0xD,  0xE,  0xF,  0x14, 0x15, 0x16, 0x17,
                           0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
  int16x8_t v0 = vec_vsx_ld(0, input);
  int16x8_t v1 = vec_vsx_ld(16, input);
  int16x8_t t0 = vec_mergeh(v0, v1);
  int16x8_t t1 = vec_mergel(v0, v1);

  uint8x16_t dest0 = vec_vsx_ld(0, dest);
  uint8x16_t dest1 = vec_vsx_ld(stride, dest);
  uint8x16_t dest2 = vec_vsx_ld(2 * stride, dest);
  uint8x16_t dest3 = vec_vsx_ld(3 * stride, dest);
  uint8x16_t zerov = vec_splat_u8(0);
  int16x8_t d_u0 = (int16x8_t)vec_mergeh(dest0, zerov);
  int16x8_t d_u1 = (int16x8_t)vec_mergeh(dest1, zerov);
  int16x8_t d_u2 = (int16x8_t)vec_mergeh(dest2, zerov);
  int16x8_t d_u3 = (int16x8_t)vec_mergeh(dest3, zerov);
  int16x8_t add8 = vec_splat_s16(8);
  uint16x8_t shift4 = vec_splat_u16(4);
  uint8x16_t output_v;

  v0 = vec_mergeh(t0, t1);
  v1 = vec_mergel(t0, t1);

  IDCT4(v0, v1, t_out0, t_out1);
  // transpose
  t0 = vec_mergeh(t_out0, t_out1);
  t1 = vec_mergel(t_out0, t_out1);
  v0 = vec_mergeh(t0, t1);
  v1 = vec_mergel(t0, t1);
  IDCT4(v0, v1, t_out0, t_out1);

  v0 = vec_sra(vec_add(t_out0, add8), shift4);
  v1 = vec_sra(vec_add(t_out1, add8), shift4);
  tmp16_0 = vec_add(vec_perm(d_u0, d_u1, mask1), v0);
  tmp16_1 = vec_add(vec_perm(d_u2, d_u3, mask1), v1);
  output_v = vec_packsu(tmp16_0, tmp16_1);

  dest0 = vec_sel(output_v, dest0, dst_mask0);
  dest1 = vec_perm(output_v, dest1, dst_mask1);
  dest2 = vec_perm(output_v, dest2, dst_mask2);
  dest3 = vec_perm(output_v, dest3, dst_mask3);

  vec_vsx_st(dest0, 0, dest);
  vec_vsx_st(dest1, stride, dest);
  vec_vsx_st(dest2, 2 * stride, dest);
  vec_vsx_st(dest3, 3 * stride, dest);
}
