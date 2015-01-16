/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>
#include "./vp9_rtcd.h"
#include "./vpx_config.h"

#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_idct.h"

void vp9_fdct8x8_1_neon(const int16_t *input, int16_t *output, int stride) {
  int r;
  int16x8_t sum = vld1q_s16(&input[0]);
  for (r = 1; r < 8; ++r) {
    const int16x8_t input_00 = vld1q_s16(&input[r * stride]);
    sum = vaddq_s16(sum, input_00);
  }
  {
    const int32x4_t a = vpaddlq_s16(sum);
    const int64x2_t b = vpaddlq_s32(a);
    const int32x2_t c = vadd_s32(vreinterpret_s32_s64(vget_low_s64(b)),
                                 vreinterpret_s32_s64(vget_high_s64(b)));
    output[0] = vget_lane_s16(vreinterpret_s16_s32(c), 0);
    output[1] = 0;
  }
}

// TODO(fgalligan): Re-factor this function, vp9_quantize_fp_neon, and
// vp9_fdct8x8_neon.
void vp9_fdct8x8_quant_neon(const int16_t *input, int stride,
                            int16_t* coeff_ptr, intptr_t n_coeffs,
                            int skip_block, const int16_t* zbin_ptr,
                            const int16_t* round_ptr, const int16_t* quant_ptr,
                            const int16_t* quant_shift_ptr,
                            int16_t* qcoeff_ptr, int16_t* dqcoeff_ptr,
                            const int16_t* dequant_ptr, uint16_t* eob_ptr,
                            const int16_t* scan_ptr,
                            const int16_t* iscan_ptr) {
  int i;

  // stage 1
  int16x8_t input_0 = vshlq_n_s16(vld1q_s16(&input[0 * stride]), 2);
  int16x8_t input_1 = vshlq_n_s16(vld1q_s16(&input[1 * stride]), 2);
  int16x8_t input_2 = vshlq_n_s16(vld1q_s16(&input[2 * stride]), 2);
  int16x8_t input_3 = vshlq_n_s16(vld1q_s16(&input[3 * stride]), 2);
  int16x8_t input_4 = vshlq_n_s16(vld1q_s16(&input[4 * stride]), 2);
  int16x8_t input_5 = vshlq_n_s16(vld1q_s16(&input[5 * stride]), 2);
  int16x8_t input_6 = vshlq_n_s16(vld1q_s16(&input[6 * stride]), 2);
  int16x8_t input_7 = vshlq_n_s16(vld1q_s16(&input[7 * stride]), 2);

  int16x8_t *in[8];
  in[0] = &input_0;
  in[1] = &input_1;
  in[2] = &input_2;
  in[3] = &input_3;
  in[4] = &input_4;
  in[5] = &input_5;
  in[6] = &input_6;
  in[7] = &input_7;

  // TODO(jingning) Decide the need of these arguments after the
  // quantization process is completed.
  (void)zbin_ptr;
  (void)quant_shift_ptr;
  (void)scan_ptr;
  (void)coeff_ptr;

  for (i = 0; i < 2; ++i) {
    int16x8_t out_0, out_1, out_2, out_3, out_4, out_5, out_6, out_7;
    const int16x8_t v_s0 = vaddq_s16(input_0, input_7);
    const int16x8_t v_s1 = vaddq_s16(input_1, input_6);
    const int16x8_t v_s2 = vaddq_s16(input_2, input_5);
    const int16x8_t v_s3 = vaddq_s16(input_3, input_4);
    const int16x8_t v_s4 = vsubq_s16(input_3, input_4);
    const int16x8_t v_s5 = vsubq_s16(input_2, input_5);
    const int16x8_t v_s6 = vsubq_s16(input_1, input_6);
    const int16x8_t v_s7 = vsubq_s16(input_0, input_7);

    // fdct4(step, step);
    int16x8_t v_x0 = vaddq_s16(v_s0, v_s3);
    int16x8_t v_x1 = vaddq_s16(v_s1, v_s2);
    int16x8_t v_x2 = vsubq_s16(v_s1, v_s2);
    int16x8_t v_x3 = vsubq_s16(v_s0, v_s3);

    // fdct4(step, step);
    int32x4_t v_t0_lo = vaddl_s16(vget_low_s16(v_x0), vget_low_s16(v_x1));
    int32x4_t v_t0_hi = vaddl_s16(vget_high_s16(v_x0), vget_high_s16(v_x1));
    int32x4_t v_t1_lo = vsubl_s16(vget_low_s16(v_x0), vget_low_s16(v_x1));
    int32x4_t v_t1_hi = vsubl_s16(vget_high_s16(v_x0), vget_high_s16(v_x1));
    int32x4_t v_t2_lo = vmull_n_s16(vget_low_s16(v_x2), (int16_t)cospi_24_64);
    int32x4_t v_t2_hi = vmull_n_s16(vget_high_s16(v_x2), (int16_t)cospi_24_64);
    int32x4_t v_t3_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_24_64);
    int32x4_t v_t3_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_24_64);
    v_t2_lo = vmlal_n_s16(v_t2_lo, vget_low_s16(v_x3), (int16_t)cospi_8_64);
    v_t2_hi = vmlal_n_s16(v_t2_hi, vget_high_s16(v_x3), (int16_t)cospi_8_64);
    v_t3_lo = vmlsl_n_s16(v_t3_lo, vget_low_s16(v_x2), (int16_t)cospi_8_64);
    v_t3_hi = vmlsl_n_s16(v_t3_hi, vget_high_s16(v_x2), (int16_t)cospi_8_64);
    v_t0_lo = vmulq_n_s32(v_t0_lo, cospi_16_64);
    v_t0_hi = vmulq_n_s32(v_t0_hi, cospi_16_64);
    v_t1_lo = vmulq_n_s32(v_t1_lo, cospi_16_64);
    v_t1_hi = vmulq_n_s32(v_t1_hi, cospi_16_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x4_t e = vrshrn_n_s32(v_t2_lo, DCT_CONST_BITS);
      const int16x4_t f = vrshrn_n_s32(v_t2_hi, DCT_CONST_BITS);
      const int16x4_t g = vrshrn_n_s32(v_t3_lo, DCT_CONST_BITS);
      const int16x4_t h = vrshrn_n_s32(v_t3_hi, DCT_CONST_BITS);
      out_0 = vcombine_s16(a, c);  // 00 01 02 03 40 41 42 43
      out_2 = vcombine_s16(e, g);  // 20 21 22 23 60 61 62 63
      out_4 = vcombine_s16(b, d);  // 04 05 06 07 44 45 46 47
      out_6 = vcombine_s16(f, h);  // 24 25 26 27 64 65 66 67
    }

    // Stage 2
    v_x0 = vsubq_s16(v_s6, v_s5);
    v_x1 = vaddq_s16(v_s6, v_s5);
    v_t0_lo = vmull_n_s16(vget_low_s16(v_x0), (int16_t)cospi_16_64);
    v_t0_hi = vmull_n_s16(vget_high_s16(v_x0), (int16_t)cospi_16_64);
    v_t1_lo = vmull_n_s16(vget_low_s16(v_x1), (int16_t)cospi_16_64);
    v_t1_hi = vmull_n_s16(vget_high_s16(v_x1), (int16_t)cospi_16_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x8_t ab = vcombine_s16(a, b);
      const int16x8_t cd = vcombine_s16(c, d);

      // Stage 3
      v_x0 = vaddq_s16(v_s4, ab);
      v_x1 = vsubq_s16(v_s4, ab);
      v_x2 = vsubq_s16(v_s7, cd);
      v_x3 = vaddq_s16(v_s7, cd);
    }

    // Stage 4
    v_t0_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_4_64);
    v_t0_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_4_64);
    v_t0_lo = vmlal_n_s16(v_t0_lo, vget_low_s16(v_x0), (int16_t)cospi_28_64);
    v_t0_hi = vmlal_n_s16(v_t0_hi, vget_high_s16(v_x0), (int16_t)cospi_28_64);
    v_t1_lo = vmull_n_s16(vget_low_s16(v_x1), (int16_t)cospi_12_64);
    v_t1_hi = vmull_n_s16(vget_high_s16(v_x1), (int16_t)cospi_12_64);
    v_t1_lo = vmlal_n_s16(v_t1_lo, vget_low_s16(v_x2), (int16_t)cospi_20_64);
    v_t1_hi = vmlal_n_s16(v_t1_hi, vget_high_s16(v_x2), (int16_t)cospi_20_64);
    v_t2_lo = vmull_n_s16(vget_low_s16(v_x2), (int16_t)cospi_12_64);
    v_t2_hi = vmull_n_s16(vget_high_s16(v_x2), (int16_t)cospi_12_64);
    v_t2_lo = vmlsl_n_s16(v_t2_lo, vget_low_s16(v_x1), (int16_t)cospi_20_64);
    v_t2_hi = vmlsl_n_s16(v_t2_hi, vget_high_s16(v_x1), (int16_t)cospi_20_64);
    v_t3_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_28_64);
    v_t3_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_28_64);
    v_t3_lo = vmlsl_n_s16(v_t3_lo, vget_low_s16(v_x0), (int16_t)cospi_4_64);
    v_t3_hi = vmlsl_n_s16(v_t3_hi, vget_high_s16(v_x0), (int16_t)cospi_4_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x4_t e = vrshrn_n_s32(v_t2_lo, DCT_CONST_BITS);
      const int16x4_t f = vrshrn_n_s32(v_t2_hi, DCT_CONST_BITS);
      const int16x4_t g = vrshrn_n_s32(v_t3_lo, DCT_CONST_BITS);
      const int16x4_t h = vrshrn_n_s32(v_t3_hi, DCT_CONST_BITS);
      out_1 = vcombine_s16(a, c);  // 10 11 12 13 50 51 52 53
      out_3 = vcombine_s16(e, g);  // 30 31 32 33 70 71 72 73
      out_5 = vcombine_s16(b, d);  // 14 15 16 17 54 55 56 57
      out_7 = vcombine_s16(f, h);  // 34 35 36 37 74 75 76 77
    }

    // transpose 8x8
    {
      // 00 01 02 03 40 41 42 43
      // 10 11 12 13 50 51 52 53
      // 20 21 22 23 60 61 62 63
      // 30 31 32 33 70 71 72 73
      // 04 05 06 07 44 45 46 47
      // 14 15 16 17 54 55 56 57
      // 24 25 26 27 64 65 66 67
      // 34 35 36 37 74 75 76 77
      const int32x4x2_t r02_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_0),
                                            vreinterpretq_s32_s16(out_2));
      const int32x4x2_t r13_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_1),
                                            vreinterpretq_s32_s16(out_3));
      const int32x4x2_t r46_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_4),
                                            vreinterpretq_s32_s16(out_6));
      const int32x4x2_t r57_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_5),
                                            vreinterpretq_s32_s16(out_7));
      const int16x8x2_t r01_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r02_s32.val[0]),
                    vreinterpretq_s16_s32(r13_s32.val[0]));
      const int16x8x2_t r23_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r02_s32.val[1]),
                    vreinterpretq_s16_s32(r13_s32.val[1]));
      const int16x8x2_t r45_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r46_s32.val[0]),
                    vreinterpretq_s16_s32(r57_s32.val[0]));
      const int16x8x2_t r67_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r46_s32.val[1]),
                    vreinterpretq_s16_s32(r57_s32.val[1]));
      input_0 = r01_s16.val[0];
      input_1 = r01_s16.val[1];
      input_2 = r23_s16.val[0];
      input_3 = r23_s16.val[1];
      input_4 = r45_s16.val[0];
      input_5 = r45_s16.val[1];
      input_6 = r67_s16.val[0];
      input_7 = r67_s16.val[1];
      // 00 10 20 30 40 50 60 70
      // 01 11 21 31 41 51 61 71
      // 02 12 22 32 42 52 62 72
      // 03 13 23 33 43 53 63 73
      // 04 14 24 34 44 54 64 74
      // 05 15 25 35 45 55 65 75
      // 06 16 26 36 46 56 66 76
      // 07 17 27 37 47 57 67 77
    }
  }  // for
  {
    // from vp9_dct_sse2.c
    // Post-condition (division by two)
    //    division of two 16 bits signed numbers using shifts
    //    n / 2 = (n - (n >> 15)) >> 1
    const int16x8_t sign_in0 = vshrq_n_s16(input_0, 15);
    const int16x8_t sign_in1 = vshrq_n_s16(input_1, 15);
    const int16x8_t sign_in2 = vshrq_n_s16(input_2, 15);
    const int16x8_t sign_in3 = vshrq_n_s16(input_3, 15);
    const int16x8_t sign_in4 = vshrq_n_s16(input_4, 15);
    const int16x8_t sign_in5 = vshrq_n_s16(input_5, 15);
    const int16x8_t sign_in6 = vshrq_n_s16(input_6, 15);
    const int16x8_t sign_in7 = vshrq_n_s16(input_7, 15);
    input_0 = vhsubq_s16(input_0, sign_in0);
    input_1 = vhsubq_s16(input_1, sign_in1);
    input_2 = vhsubq_s16(input_2, sign_in2);
    input_3 = vhsubq_s16(input_3, sign_in3);
    input_4 = vhsubq_s16(input_4, sign_in4);
    input_5 = vhsubq_s16(input_5, sign_in5);
    input_6 = vhsubq_s16(input_6, sign_in6);
    input_7 = vhsubq_s16(input_7, sign_in7);
  }

  if (!skip_block) {
    // Quantization pass: All coefficients with index >= zero_flag are
    // skippable. Note: zero_flag can be zero.
    int i;
    int index = 1;
    const int16x8_t v_zero = vdupq_n_s16(0);
    const int16x8_t v_one = vdupq_n_s16(1);
    int16x8_t v_eobmax_76543210 = vdupq_n_s16(-1);
    int16x8_t v_round = vmovq_n_s16(round_ptr[1]);
    int16x8_t v_quant = vmovq_n_s16(quant_ptr[1]);
    int16x8_t v_dequant = vmovq_n_s16(dequant_ptr[1]);

    // adjust for dc
    v_round = vsetq_lane_s16(round_ptr[0], v_round, 0);
    v_quant = vsetq_lane_s16(quant_ptr[0], v_quant, 0);
    v_dequant = vsetq_lane_s16(dequant_ptr[0], v_dequant, 0);

    // process dc and the first seven ac coeffs
    {
      const int16x8_t v_iscan = vld1q_s16(&iscan_ptr[0]);
      const int16x8_t v_coeff = *in[0];
      const int16x8_t v_coeff_sign = vshrq_n_s16(v_coeff, 15);
      const int16x8_t v_tmp = vabaq_s16(v_round, v_coeff, v_zero);
      const int32x4_t v_tmp_lo = vmull_s16(vget_low_s16(v_tmp),
                                           vget_low_s16(v_quant));
      const int32x4_t v_tmp_hi = vmull_s16(vget_high_s16(v_tmp),
                                           vget_high_s16(v_quant));
      const int16x8_t v_tmp2 = vcombine_s16(vshrn_n_s32(v_tmp_lo, 16),
                                            vshrn_n_s32(v_tmp_hi, 16));
      const uint16x8_t v_nz_mask = vceqq_s16(v_tmp2, v_zero);
      const int16x8_t v_iscan_plus1 = vaddq_s16(v_iscan, v_one);
      const int16x8_t v_nz_iscan = vbslq_s16(v_nz_mask, v_zero, v_iscan_plus1);
      const int16x8_t v_qcoeff_a = veorq_s16(v_tmp2, v_coeff_sign);
      const int16x8_t v_qcoeff = vsubq_s16(v_qcoeff_a, v_coeff_sign);
      const int16x8_t v_dqcoeff = vmulq_s16(v_qcoeff, v_dequant);
      v_eobmax_76543210 = vmaxq_s16(v_eobmax_76543210, v_nz_iscan);
      vst1q_s16(&qcoeff_ptr[0], v_qcoeff);
      vst1q_s16(&dqcoeff_ptr[0], v_dqcoeff);
      v_round = vmovq_n_s16(round_ptr[1]);
      v_quant = vmovq_n_s16(quant_ptr[1]);
      v_dequant = vmovq_n_s16(dequant_ptr[1]);
    }

    // now process the rest of the ac coeffs
    for (i = 8; i < n_coeffs; i += 8) {
      const int16x8_t v_iscan = vld1q_s16(&iscan_ptr[i]);
      const int16x8_t v_coeff = *in[index];
      const int16x8_t v_coeff_sign = vshrq_n_s16(v_coeff, 15);
      const int16x8_t v_tmp = vabaq_s16(v_round, v_coeff, v_zero);
      const int32x4_t v_tmp_lo = vmull_s16(vget_low_s16(v_tmp),
                                           vget_low_s16(v_quant));
      const int32x4_t v_tmp_hi = vmull_s16(vget_high_s16(v_tmp),
                                           vget_high_s16(v_quant));
      const int16x8_t v_tmp2 = vcombine_s16(vshrn_n_s32(v_tmp_lo, 16),
                                            vshrn_n_s32(v_tmp_hi, 16));
      const uint16x8_t v_nz_mask = vceqq_s16(v_tmp2, v_zero);
      const int16x8_t v_iscan_plus1 = vaddq_s16(v_iscan, v_one);
      const int16x8_t v_nz_iscan = vbslq_s16(v_nz_mask, v_zero, v_iscan_plus1);
      const int16x8_t v_qcoeff_a = veorq_s16(v_tmp2, v_coeff_sign);
      const int16x8_t v_qcoeff = vsubq_s16(v_qcoeff_a, v_coeff_sign);
      const int16x8_t v_dqcoeff = vmulq_s16(v_qcoeff, v_dequant);
      v_eobmax_76543210 = vmaxq_s16(v_eobmax_76543210, v_nz_iscan);
      vst1q_s16(&qcoeff_ptr[i], v_qcoeff);
      vst1q_s16(&dqcoeff_ptr[i], v_dqcoeff);
      index++;
    }
    {
      const int16x4_t v_eobmax_3210 =
          vmax_s16(vget_low_s16(v_eobmax_76543210),
                   vget_high_s16(v_eobmax_76543210));
      const int64x1_t v_eobmax_xx32 =
          vshr_n_s64(vreinterpret_s64_s16(v_eobmax_3210), 32);
      const int16x4_t v_eobmax_tmp =
          vmax_s16(v_eobmax_3210, vreinterpret_s16_s64(v_eobmax_xx32));
      const int64x1_t v_eobmax_xxx3 =
          vshr_n_s64(vreinterpret_s64_s16(v_eobmax_tmp), 16);
      const int16x4_t v_eobmax_final =
          vmax_s16(v_eobmax_tmp, vreinterpret_s16_s64(v_eobmax_xxx3));

      *eob_ptr = (uint16_t)vget_lane_s16(v_eobmax_final, 0);
    }
  } else {
    vpx_memset(qcoeff_ptr, 0, n_coeffs * sizeof(*qcoeff_ptr));
    vpx_memset(dqcoeff_ptr, 0, n_coeffs * sizeof(*dqcoeff_ptr));
    *eob_ptr = 0;
  }
}

void vp9_fdct8x8_neon(const int16_t *input, int16_t *final_output, int stride) {
  int i;
  // stage 1
  int16x8_t input_0 = vshlq_n_s16(vld1q_s16(&input[0 * stride]), 2);
  int16x8_t input_1 = vshlq_n_s16(vld1q_s16(&input[1 * stride]), 2);
  int16x8_t input_2 = vshlq_n_s16(vld1q_s16(&input[2 * stride]), 2);
  int16x8_t input_3 = vshlq_n_s16(vld1q_s16(&input[3 * stride]), 2);
  int16x8_t input_4 = vshlq_n_s16(vld1q_s16(&input[4 * stride]), 2);
  int16x8_t input_5 = vshlq_n_s16(vld1q_s16(&input[5 * stride]), 2);
  int16x8_t input_6 = vshlq_n_s16(vld1q_s16(&input[6 * stride]), 2);
  int16x8_t input_7 = vshlq_n_s16(vld1q_s16(&input[7 * stride]), 2);
  for (i = 0; i < 2; ++i) {
    int16x8_t out_0, out_1, out_2, out_3, out_4, out_5, out_6, out_7;
    const int16x8_t v_s0 = vaddq_s16(input_0, input_7);
    const int16x8_t v_s1 = vaddq_s16(input_1, input_6);
    const int16x8_t v_s2 = vaddq_s16(input_2, input_5);
    const int16x8_t v_s3 = vaddq_s16(input_3, input_4);
    const int16x8_t v_s4 = vsubq_s16(input_3, input_4);
    const int16x8_t v_s5 = vsubq_s16(input_2, input_5);
    const int16x8_t v_s6 = vsubq_s16(input_1, input_6);
    const int16x8_t v_s7 = vsubq_s16(input_0, input_7);
    // fdct4(step, step);
    int16x8_t v_x0 = vaddq_s16(v_s0, v_s3);
    int16x8_t v_x1 = vaddq_s16(v_s1, v_s2);
    int16x8_t v_x2 = vsubq_s16(v_s1, v_s2);
    int16x8_t v_x3 = vsubq_s16(v_s0, v_s3);
    // fdct4(step, step);
    int32x4_t v_t0_lo = vaddl_s16(vget_low_s16(v_x0), vget_low_s16(v_x1));
    int32x4_t v_t0_hi = vaddl_s16(vget_high_s16(v_x0), vget_high_s16(v_x1));
    int32x4_t v_t1_lo = vsubl_s16(vget_low_s16(v_x0), vget_low_s16(v_x1));
    int32x4_t v_t1_hi = vsubl_s16(vget_high_s16(v_x0), vget_high_s16(v_x1));
    int32x4_t v_t2_lo = vmull_n_s16(vget_low_s16(v_x2), (int16_t)cospi_24_64);
    int32x4_t v_t2_hi = vmull_n_s16(vget_high_s16(v_x2), (int16_t)cospi_24_64);
    int32x4_t v_t3_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_24_64);
    int32x4_t v_t3_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_24_64);
    v_t2_lo = vmlal_n_s16(v_t2_lo, vget_low_s16(v_x3), (int16_t)cospi_8_64);
    v_t2_hi = vmlal_n_s16(v_t2_hi, vget_high_s16(v_x3), (int16_t)cospi_8_64);
    v_t3_lo = vmlsl_n_s16(v_t3_lo, vget_low_s16(v_x2), (int16_t)cospi_8_64);
    v_t3_hi = vmlsl_n_s16(v_t3_hi, vget_high_s16(v_x2), (int16_t)cospi_8_64);
    v_t0_lo = vmulq_n_s32(v_t0_lo, cospi_16_64);
    v_t0_hi = vmulq_n_s32(v_t0_hi, cospi_16_64);
    v_t1_lo = vmulq_n_s32(v_t1_lo, cospi_16_64);
    v_t1_hi = vmulq_n_s32(v_t1_hi, cospi_16_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x4_t e = vrshrn_n_s32(v_t2_lo, DCT_CONST_BITS);
      const int16x4_t f = vrshrn_n_s32(v_t2_hi, DCT_CONST_BITS);
      const int16x4_t g = vrshrn_n_s32(v_t3_lo, DCT_CONST_BITS);
      const int16x4_t h = vrshrn_n_s32(v_t3_hi, DCT_CONST_BITS);
      out_0 = vcombine_s16(a, c);  // 00 01 02 03 40 41 42 43
      out_2 = vcombine_s16(e, g);  // 20 21 22 23 60 61 62 63
      out_4 = vcombine_s16(b, d);  // 04 05 06 07 44 45 46 47
      out_6 = vcombine_s16(f, h);  // 24 25 26 27 64 65 66 67
    }
    // Stage 2
    v_x0 = vsubq_s16(v_s6, v_s5);
    v_x1 = vaddq_s16(v_s6, v_s5);
    v_t0_lo = vmull_n_s16(vget_low_s16(v_x0), (int16_t)cospi_16_64);
    v_t0_hi = vmull_n_s16(vget_high_s16(v_x0), (int16_t)cospi_16_64);
    v_t1_lo = vmull_n_s16(vget_low_s16(v_x1), (int16_t)cospi_16_64);
    v_t1_hi = vmull_n_s16(vget_high_s16(v_x1), (int16_t)cospi_16_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x8_t ab = vcombine_s16(a, b);
      const int16x8_t cd = vcombine_s16(c, d);
      // Stage 3
      v_x0 = vaddq_s16(v_s4, ab);
      v_x1 = vsubq_s16(v_s4, ab);
      v_x2 = vsubq_s16(v_s7, cd);
      v_x3 = vaddq_s16(v_s7, cd);
    }
    // Stage 4
    v_t0_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_4_64);
    v_t0_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_4_64);
    v_t0_lo = vmlal_n_s16(v_t0_lo, vget_low_s16(v_x0), (int16_t)cospi_28_64);
    v_t0_hi = vmlal_n_s16(v_t0_hi, vget_high_s16(v_x0), (int16_t)cospi_28_64);
    v_t1_lo = vmull_n_s16(vget_low_s16(v_x1), (int16_t)cospi_12_64);
    v_t1_hi = vmull_n_s16(vget_high_s16(v_x1), (int16_t)cospi_12_64);
    v_t1_lo = vmlal_n_s16(v_t1_lo, vget_low_s16(v_x2), (int16_t)cospi_20_64);
    v_t1_hi = vmlal_n_s16(v_t1_hi, vget_high_s16(v_x2), (int16_t)cospi_20_64);
    v_t2_lo = vmull_n_s16(vget_low_s16(v_x2), (int16_t)cospi_12_64);
    v_t2_hi = vmull_n_s16(vget_high_s16(v_x2), (int16_t)cospi_12_64);
    v_t2_lo = vmlsl_n_s16(v_t2_lo, vget_low_s16(v_x1), (int16_t)cospi_20_64);
    v_t2_hi = vmlsl_n_s16(v_t2_hi, vget_high_s16(v_x1), (int16_t)cospi_20_64);
    v_t3_lo = vmull_n_s16(vget_low_s16(v_x3), (int16_t)cospi_28_64);
    v_t3_hi = vmull_n_s16(vget_high_s16(v_x3), (int16_t)cospi_28_64);
    v_t3_lo = vmlsl_n_s16(v_t3_lo, vget_low_s16(v_x0), (int16_t)cospi_4_64);
    v_t3_hi = vmlsl_n_s16(v_t3_hi, vget_high_s16(v_x0), (int16_t)cospi_4_64);
    {
      const int16x4_t a = vrshrn_n_s32(v_t0_lo, DCT_CONST_BITS);
      const int16x4_t b = vrshrn_n_s32(v_t0_hi, DCT_CONST_BITS);
      const int16x4_t c = vrshrn_n_s32(v_t1_lo, DCT_CONST_BITS);
      const int16x4_t d = vrshrn_n_s32(v_t1_hi, DCT_CONST_BITS);
      const int16x4_t e = vrshrn_n_s32(v_t2_lo, DCT_CONST_BITS);
      const int16x4_t f = vrshrn_n_s32(v_t2_hi, DCT_CONST_BITS);
      const int16x4_t g = vrshrn_n_s32(v_t3_lo, DCT_CONST_BITS);
      const int16x4_t h = vrshrn_n_s32(v_t3_hi, DCT_CONST_BITS);
      out_1 = vcombine_s16(a, c);  // 10 11 12 13 50 51 52 53
      out_3 = vcombine_s16(e, g);  // 30 31 32 33 70 71 72 73
      out_5 = vcombine_s16(b, d);  // 14 15 16 17 54 55 56 57
      out_7 = vcombine_s16(f, h);  // 34 35 36 37 74 75 76 77
    }
    // transpose 8x8
    {
      // 00 01 02 03 40 41 42 43
      // 10 11 12 13 50 51 52 53
      // 20 21 22 23 60 61 62 63
      // 30 31 32 33 70 71 72 73
      // 04 05 06 07 44 45 46 47
      // 14 15 16 17 54 55 56 57
      // 24 25 26 27 64 65 66 67
      // 34 35 36 37 74 75 76 77
      const int32x4x2_t r02_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_0),
                                            vreinterpretq_s32_s16(out_2));
      const int32x4x2_t r13_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_1),
                                            vreinterpretq_s32_s16(out_3));
      const int32x4x2_t r46_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_4),
                                            vreinterpretq_s32_s16(out_6));
      const int32x4x2_t r57_s32 = vtrnq_s32(vreinterpretq_s32_s16(out_5),
                                            vreinterpretq_s32_s16(out_7));
      const int16x8x2_t r01_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r02_s32.val[0]),
                    vreinterpretq_s16_s32(r13_s32.val[0]));
      const int16x8x2_t r23_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r02_s32.val[1]),
                    vreinterpretq_s16_s32(r13_s32.val[1]));
      const int16x8x2_t r45_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r46_s32.val[0]),
                    vreinterpretq_s16_s32(r57_s32.val[0]));
      const int16x8x2_t r67_s16 =
          vtrnq_s16(vreinterpretq_s16_s32(r46_s32.val[1]),
                    vreinterpretq_s16_s32(r57_s32.val[1]));
      input_0 = r01_s16.val[0];
      input_1 = r01_s16.val[1];
      input_2 = r23_s16.val[0];
      input_3 = r23_s16.val[1];
      input_4 = r45_s16.val[0];
      input_5 = r45_s16.val[1];
      input_6 = r67_s16.val[0];
      input_7 = r67_s16.val[1];
      // 00 10 20 30 40 50 60 70
      // 01 11 21 31 41 51 61 71
      // 02 12 22 32 42 52 62 72
      // 03 13 23 33 43 53 63 73
      // 04 14 24 34 44 54 64 74
      // 05 15 25 35 45 55 65 75
      // 06 16 26 36 46 56 66 76
      // 07 17 27 37 47 57 67 77
    }
  }  // for
  {
    // from vp9_dct_sse2.c
    // Post-condition (division by two)
    //    division of two 16 bits signed numbers using shifts
    //    n / 2 = (n - (n >> 15)) >> 1
    const int16x8_t sign_in0 = vshrq_n_s16(input_0, 15);
    const int16x8_t sign_in1 = vshrq_n_s16(input_1, 15);
    const int16x8_t sign_in2 = vshrq_n_s16(input_2, 15);
    const int16x8_t sign_in3 = vshrq_n_s16(input_3, 15);
    const int16x8_t sign_in4 = vshrq_n_s16(input_4, 15);
    const int16x8_t sign_in5 = vshrq_n_s16(input_5, 15);
    const int16x8_t sign_in6 = vshrq_n_s16(input_6, 15);
    const int16x8_t sign_in7 = vshrq_n_s16(input_7, 15);
    input_0 = vhsubq_s16(input_0, sign_in0);
    input_1 = vhsubq_s16(input_1, sign_in1);
    input_2 = vhsubq_s16(input_2, sign_in2);
    input_3 = vhsubq_s16(input_3, sign_in3);
    input_4 = vhsubq_s16(input_4, sign_in4);
    input_5 = vhsubq_s16(input_5, sign_in5);
    input_6 = vhsubq_s16(input_6, sign_in6);
    input_7 = vhsubq_s16(input_7, sign_in7);
    // store results
    vst1q_s16(&final_output[0 * 8], input_0);
    vst1q_s16(&final_output[1 * 8], input_1);
    vst1q_s16(&final_output[2 * 8], input_2);
    vst1q_s16(&final_output[3 * 8], input_3);
    vst1q_s16(&final_output[4 * 8], input_4);
    vst1q_s16(&final_output[5 * 8], input_5);
    vst1q_s16(&final_output[6 * 8], input_6);
    vst1q_s16(&final_output[7 * 8], input_7);
  }
}

