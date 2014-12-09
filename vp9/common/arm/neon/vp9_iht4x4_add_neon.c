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
#include <assert.h>

static int16_t sinpi_1_9 = 0x14a3;
static int16_t sinpi_2_9 = 0x26c9;
static int16_t sinpi_3_9 = 0x3441;
static int16_t sinpi_4_9 = 0x3b6c;
static int16_t cospi_8_64 = 0x3b21;
static int16_t cospi_16_64 = 0x2d41;
static int16_t cospi_24_64 = 0x187e;

static inline void transpose4x4(int16x8_t *ouput0,
                                int16x8_t *output1) {
  int32x4_t v_s32x4_0, v_s32x4_1;
  int16x4x2_t m_s16x4x2_0, m_s16x4x2_1;
  int32x4x2_t m_s32x4x2;

  m_s16x4x2_0 = vtrn_s16(vget_low_s16(*ouput0), vget_high_s16(*ouput0));
  m_s16x4x2_1 = vtrn_s16(vget_low_s16(*output1), vget_high_s16(*output1));

  v_s32x4_0 = vreinterpretq_s32_s16(vcombine_s16(m_s16x4x2_0.val[0],
                                                 m_s16x4x2_0.val[1]));
  v_s32x4_1 = vreinterpretq_s32_s16(vcombine_s16(m_s16x4x2_1.val[0],
                                                 m_s16x4x2_1.val[1]));
  m_s32x4x2 = vtrnq_s32(v_s32x4_0, v_s32x4_1);

  *ouput0 = vreinterpretq_s16_s32(m_s32x4x2.val[0]);
  *output1 = vreinterpretq_s16_s32(m_s32x4x2.val[1]);
}

static inline void generate_cosine_constants(int16x4_t *const0,
                                             int16x4_t *const1,
                                             int16x4_t *const2) {
  *const0 = vdup_n_s16(cospi_8_64);
  *const1 = vdup_n_s16(cospi_16_64);
  *const2 = vdup_n_s16(cospi_24_64);
}

static inline void generate_sine_constants(int16x4_t *const0,
                                           int16x4_t *const1,
                                           int16x4_t *const2,
                                           int16x8_t *const3) {
  *const0 = vdup_n_s16(sinpi_1_9);
  *const1 = vdup_n_s16(sinpi_2_9);
  *const3 = vdupq_n_s16(sinpi_3_9);
  *const2 = vdup_n_s16(sinpi_4_9);
}

static inline void idct4x4_1d(int16x4_t *const0,
                              int16x4_t *const1,
                              int16x4_t *const2,
                              int16x8_t *output0,
                              int16x8_t *output1) {
  int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3, v_s16x4_4,
    v_s16x4_5, v_s16x4_6, v_s16x4_7, v_s16x4_8, v_s16x4_9;
  int32x4_t v_s32x4_0, v_s32x4_1, v_s32x4_2, v_s32x4_3;
  int16x8_t v_s16x8_0, v_s16x8_1;

  v_s16x4_0 = vget_low_s16(*output0);
  v_s16x4_1 = vget_high_s16(*output0);
  v_s16x4_2 = vget_low_s16(*output1);
  v_s16x4_3 = vget_high_s16(*output1);

  v_s16x4_4 = vadd_s16(v_s16x4_0, v_s16x4_2);
  v_s16x4_5 = vsub_s16(v_s16x4_0, v_s16x4_2);

  v_s32x4_3 = vmull_s16(v_s16x4_1, *const2);
  v_s32x4_0 = vmull_s16(v_s16x4_1, *const0);
  v_s32x4_1 = vmull_s16(v_s16x4_4, *const1);
  v_s32x4_2 = vmull_s16(v_s16x4_5, *const1);
  v_s32x4_3 = vmlsl_s16(v_s32x4_3, v_s16x4_3, *const0);
  v_s32x4_0 = vmlal_s16(v_s32x4_0, v_s16x4_3, *const2);

  v_s16x4_6 = vqrshrn_n_s32(v_s32x4_1, 14);
  v_s16x4_7 = vqrshrn_n_s32(v_s32x4_2, 14);
  v_s16x4_9 = vqrshrn_n_s32(v_s32x4_3, 14);
  v_s16x4_8 = vqrshrn_n_s32(v_s32x4_0, 14);

  v_s16x8_0 = vcombine_s16(v_s16x4_6, v_s16x4_7);
  v_s16x8_1 = vcombine_s16(v_s16x4_8, v_s16x4_9);
  *output0 = vaddq_s16(v_s16x8_0, v_s16x8_1);
  *output1 = vsubq_s16(v_s16x8_0, v_s16x8_1);
  *output1 = vcombine_s16(vget_high_s16(*output1), vget_low_s16(*output1));
}

static inline void iadst4x4_1d(int16x4_t *const0,
                               int16x4_t *const1,
                               int16x4_t *const2,
                               int16x4_t *const3,
                               int16x8_t *output0,
                               int16x8_t *output1) {
  int16x4_t v_d_0, v_d_1, v_d_2, v_d_3;
  int32x4_t v_q_0, v_q_1, v_q_2, v_q_3, v_q_4, v_q_5, v_q_6, v_q_7;

  v_d_0 = vget_low_s16(*output0);
  v_d_1 = vget_high_s16(*output0);
  v_d_2 = vget_low_s16(*output1);
  v_d_3 = vget_high_s16(*output1);

  v_q_2 = vmull_s16(*const0, v_d_0);
  v_q_3 = vmull_s16(*const1, v_d_0);
  v_q_4 = vmull_s16(*const3, v_d_1);
  v_q_5 = vmull_s16(*const2, v_d_2);
  v_q_6 = vmull_s16(*const0, v_d_2);
  v_q_7 = vmovl_s16(v_d_0);
  v_q_7 = vaddw_s16(v_q_7, v_d_3);
  v_q_0  = vmull_s16(*const1, v_d_3);
  v_q_7 = vsubw_s16(v_q_7, v_d_2);
  v_q_1  = vmull_s16(*const2, v_d_3);

  v_q_2 = vaddq_s32(v_q_2, v_q_5);
  v_q_2 = vaddq_s32(v_q_2, v_q_0);
  v_q_3 = vsubq_s32(v_q_3, v_q_6);
  v_q_0  = vdupq_n_s32(sinpi_3_9);
  v_q_3 = vsubq_s32(v_q_3, v_q_1);
  v_q_7 = vmulq_s32(v_q_7, v_q_0);

  v_q_5 = vaddq_s32(v_q_2, v_q_4);
  v_q_2 = vaddq_s32(v_q_2, v_q_3);
  v_q_6 = vaddq_s32(v_q_3, v_q_4);
  v_q_2 = vsubq_s32(v_q_2, v_q_4);

  v_d_0 = vqrshrn_n_s32(v_q_5, 14);
  v_d_1 = vqrshrn_n_s32(v_q_6, 14);
  v_d_2 = vqrshrn_n_s32(v_q_7, 14);
  v_d_3 = vqrshrn_n_s32(v_q_2, 14);

  *output0 = vcombine_s16(v_d_0, v_d_1);
  *output1 = vcombine_s16(v_d_2, v_d_3);
}

/*
 * This function will only handle tx_type of 1, 2, 3.
 */
void vp9_iht4x4_16_add_neon(int16_t *src,
                            uint8_t *dst,
                            int dst_stride,
                            int tx_type) {
  uint8x8_t v_u8x8_0, v_u8x8_1;
  int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3,
    v_s16x4_4, v_s16x4_5, v_s16x4_6;
  uint32x2_t v_u32x2_0, v_u32x2_1;
  int16x8_t v_s16x8_0, v_s16x8_1, v_s16x8_2;
  uint16x8_t v_u16x8_0, v_u16x8_1;

  v_u32x2_0 = v_u32x2_1 = vdup_n_u32(0);

  v_s16x8_1 = vld1q_s16(src);
  v_s16x8_2 = vld1q_s16(src + 8);

  transpose4x4(&v_s16x8_1, &v_s16x8_2);

  switch (tx_type) {
    case 0:  // idct_idct is not supported. Fall back to C
      vp9_iht4x4_16_add_c(src, dst, dst_stride, tx_type);
      return;
      break;
    case 1:  // iadst_idct
      // generate constants
      generate_cosine_constants(&v_s16x4_0, &v_s16x4_1, &v_s16x4_2);
      generate_sine_constants(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x8_0);

      // first transform rows
      idct4x4_1d(&v_s16x4_0, &v_s16x4_1, &v_s16x4_2, &v_s16x8_1, &v_s16x8_2);

      // transpose the matrix
      transpose4x4(&v_s16x8_1, &v_s16x8_2);

      // then transform columns
      v_s16x4_6 = vget_low_s16(v_s16x8_0);
      iadst4x4_1d(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x4_6,
                  &v_s16x8_1, &v_s16x8_2);
      break;
    case 2:  // idct_iadst
      // generate constantsyy
      generate_cosine_constants(&v_s16x4_0, &v_s16x4_1, &v_s16x4_2);
      generate_sine_constants(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x8_0);

      // first transform rows
      v_s16x4_6 = vget_low_s16(v_s16x8_0);
      iadst4x4_1d(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x4_6,
                  &v_s16x8_1, &v_s16x8_2);

      // transpose the matrix
      transpose4x4(&v_s16x8_1, &v_s16x8_2);

      // then transform columns
      idct4x4_1d(&v_s16x4_0, &v_s16x4_1, &v_s16x4_2, &v_s16x8_1, &v_s16x8_2);
      break;
    case 3:  // iadst_iadst
      // generate constants
      generate_sine_constants(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x8_0);

      // first transform rows
      v_s16x4_6 = vget_low_s16(v_s16x8_0);
      iadst4x4_1d(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x4_6,
                  &v_s16x8_1, &v_s16x8_2);

      // transpose the matrix
      transpose4x4(&v_s16x8_1, &v_s16x8_2);

      // then transform columns
      v_s16x4_6 = vget_low_s16(v_s16x8_0);
      iadst4x4_1d(&v_s16x4_3, &v_s16x4_4, &v_s16x4_5, &v_s16x4_6,
                  &v_s16x8_1, &v_s16x8_2);
      break;
    default:
      assert(0);
      break;
  }

  v_s16x8_1 = vrshrq_n_s16(v_s16x8_1, 4);
  v_s16x8_2 = vrshrq_n_s16(v_s16x8_2, 4);

  v_u32x2_0 = vld1_lane_u32((const uint32_t *)dst, v_u32x2_0, 0);
  dst += dst_stride;
  v_u32x2_0 = vld1_lane_u32((const uint32_t *)dst, v_u32x2_0, 1);
  dst += dst_stride;
  v_u32x2_1 = vld1_lane_u32((const uint32_t *)dst, v_u32x2_1, 0);
  dst += dst_stride;
  v_u32x2_1 = vld1_lane_u32((const uint32_t *)dst, v_u32x2_1, 1);

  v_u16x8_0 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_1),
                       vreinterpret_u8_u32(v_u32x2_0));
  v_u16x8_1 = vaddw_u8(vreinterpretq_u16_s16(v_s16x8_2),
                       vreinterpret_u8_u32(v_u32x2_1));

  v_u8x8_0 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_0));
  v_u8x8_1 = vqmovun_s16(vreinterpretq_s16_u16(v_u16x8_1));

  vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(v_u8x8_1), 1);
  dst -= dst_stride;
  vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(v_u8x8_1), 0);
  dst -= dst_stride;
  vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(v_u8x8_0), 1);
  dst -= dst_stride;
  vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(v_u8x8_0), 0);
}
