/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_dsp/mips/fwd_txfm_msa.h"

void vp9_fdct4x4_msa(const int16_t *input, int16_t *output,
                     int32_t src_stride) {
  v8i16 in0, in1, in2, in3;

  LD_SH4(input, src_stride, in0, in1, in2, in3);

  /* fdct4 pre-process */
  {
    v8i16 vec, mask;
    v16i8 zero = { 0 };
    v16i8 one = __msa_ldi_b(1);

    mask = (v8i16)__msa_sldi_b(zero, one, 15);
    SLLI_4V(in0, in1, in2, in3, 4);
    vec = __msa_ceqi_h(in0, 0);
    vec = vec ^ 255;
    vec = mask & vec;
    in0 += vec;
  }

  VP9_FDCT4(in0, in1, in2, in3, in0, in1, in2, in3);
  TRANSPOSE4x4_SH_SH(in0, in1, in2, in3, in0, in1, in2, in3);
  VP9_FDCT4(in0, in1, in2, in3, in0, in1, in2, in3);
  TRANSPOSE4x4_SH_SH(in0, in1, in2, in3, in0, in1, in2, in3);
  ADD4(in0, 1, in1, 1, in2, 1, in3, 1, in0, in1, in2, in3);
  SRA_4V(in0, in1, in2, in3, 2);
  PCKEV_D2_SH(in1, in0, in3, in2, in0, in2);
  ST_SH2(in0, in2, output, 8);
}

void vp9_fdct8x8_msa(const int16_t *input, int16_t *output,
                     int32_t src_stride) {
  v8i16 in0, in1, in2, in3, in4, in5, in6, in7;

  LD_SH8(input, src_stride, in0, in1, in2, in3, in4, in5, in6, in7);
  SLLI_4V(in0, in1, in2, in3, 2);
  SLLI_4V(in4, in5, in6, in7, 2);
  VP9_FDCT8(in0, in1, in2, in3, in4, in5, in6, in7,
            in0, in1, in2, in3, in4, in5, in6, in7);
  TRANSPOSE8x8_SH_SH(in0, in1, in2, in3, in4, in5, in6, in7,
                     in0, in1, in2, in3, in4, in5, in6, in7);
  VP9_FDCT8(in0, in1, in2, in3, in4, in5, in6, in7,
            in0, in1, in2, in3, in4, in5, in6, in7);
  TRANSPOSE8x8_SH_SH(in0, in1, in2, in3, in4, in5, in6, in7,
                     in0, in1, in2, in3, in4, in5, in6, in7);
  SRLI_AVE_S_4V_H(in0, in1, in2, in3, in4, in5, in6, in7);
  ST_SH8(in0, in1, in2, in3, in4, in5, in6, in7, output, 8);
}

void vp9_fdct16x16_msa(const int16_t *input, int16_t *output,
                       int32_t src_stride) {
  int32_t i;
  DECLARE_ALIGNED(32, int16_t, tmp_buf[16 * 16]);

  /* column transform */
  for (i = 0; i < 2; ++i) {
    fdct8x16_1d_column((input + 8 * i), (&tmp_buf[0] + 8 * i), src_stride);
  }

  /* row transform */
  for (i = 0; i < 2; ++i) {
    fdct16x8_1d_row((&tmp_buf[0] + (128 * i)), (output + (128 * i)));
  }
}
