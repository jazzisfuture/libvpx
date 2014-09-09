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

#include "vpx_ports/mem.h"

static int16_t sinpi_1_9 = 0x14a3;
static int16_t sinpi_2_9 = 0x26c9;
static int16_t sinpi_3_9 = 0x3441;
static int16_t sinpi_4_9 = 0x3b6c;
static int16_t cospi_8_64 = 0x3b21;
static int16_t cospi_16_64 = 0x2d41;
static int16_t cospi_24_64 = 0x187e;

static inline void transpose4x4(int16x8_t *matrix0,
                                int16x8_t *matrix1)
{
    int32x4_t v_s32x4_0, v_s32x4_1;
    int16x4x2_t m_s16x4x2_0, m_s16x4x2_1;
    int32x4x2_t m_s32x4x2;

    m_s16x4x2_0 = vtrn_s16(vget_low_s16(*matrix0), vget_high_s16(*matrix0));
    m_s16x4x2_1 = vtrn_s16(vget_low_s16(*matrix1), vget_high_s16(*matrix1));

    v_s32x4_0 = vreinterpretq_s32_s16(vcombine_s16(m_s16x4x2_0.val[0], m_s16x4x2_0.val[1]));
    v_s32x4_1 = vreinterpretq_s32_s16(vcombine_s16(m_s16x4x2_1.val[0], m_s16x4x2_1.val[1]));
    m_s32x4x2 = vtrnq_s32(v_s32x4_0, v_s32x4_1);

    *matrix0 = vreinterpretq_s16_s32(m_s32x4x2.val[0]);
    *matrix1 = vreinterpretq_s16_s32(m_s32x4x2.val[1]);
}

static inline void generate_cosine_constants(int16x4_t *const0,
                                             int16x4_t *const1,
                                             int16x4_t *const2)
{
    *const0 = vdup_n_s16(cospi_8_64);
    *const1 = vdup_n_s16(cospi_16_64);
    *const2 = vdup_n_s16(cospi_24_64);
}

static inline void generate_sine_constants(int16x4_t *const0,
                                           int16x4_t *const1,
                                           int16x4_t *const2,
                                           int16x8_t *const3)
{
    *const0 = vdup_n_s16(sinpi_1_9);
    *const1 = vdup_n_s16(sinpi_2_9);
    *const3 = vdupq_n_s16(sinpi_3_9);
    *const2 = vdup_n_s16(sinpi_4_9);
}

static inline void idct4x4_1d(int16x4_t *const0,
                              int16x4_t *const1,
                              int16x4_t *const2,
                              int16x8_t *matrix0,
                              int16x8_t *matrix1)
{
    int16x4_t v_s16x4_0, v_s16x4_1, v_s16x4_2, v_s16x4_3, v_s16x4_4,
      v_s16x4_5, v_s16x4_6, v_s16x4_7, v_s16x4_8, v_s16x4_9;
    int32x4_t v_s32x4_0, v_s32x4_1, v_s32x4_2, v_s32x4_3;
    int16x8_t v_s16x8_0, v_s16x8_1;

    v_s16x4_0 = vget_low_s16(*matrix0);
    v_s16x4_1 = vget_high_s16(*matrix0);
    v_s16x4_2 = vget_low_s16(*matrix1);
    v_s16x4_3 = vget_high_s16(*matrix1);

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
    *matrix0 = vaddq_s16(v_s16x8_0, v_s16x8_1);
    *matrix1 = vsubq_s16(v_s16x8_0, v_s16x8_1);
    *matrix1 = vcombine_s16(vget_high_s16(*matrix1), vget_low_s16(*matrix1));
}

static inline void iadst4x4_1d(int16x4_t *d3s16,
                               int16x4_t *d4s16,
                               int16x4_t *d5s16,
                               int16x8_t *q3s16,
                               int16x8_t *q8s16,
                               int16x8_t *q9s16)
{
    int16x4_t d6s16, d16s16, d17s16, d18s16, d19s16;
    int32x4_t q8s32, q9s32, q10s32, q11s32, q12s32, q13s32, q14s32, q15s32;

    d6s16 = vget_low_s16(*q3s16);

    d16s16 = vget_low_s16(*q8s16);
    d17s16 = vget_high_s16(*q8s16);
    d18s16 = vget_low_s16(*q9s16);
    d19s16 = vget_high_s16(*q9s16);

    q10s32 = vmull_s16(*d3s16, d16s16);
    q11s32 = vmull_s16(*d4s16, d16s16);
    q12s32 = vmull_s16(d6s16, d17s16);
    q13s32 = vmull_s16(*d5s16, d18s16);
    q14s32 = vmull_s16(*d3s16, d18s16);
    q15s32 = vmovl_s16(d16s16);
    q15s32 = vaddw_s16(q15s32, d19s16);
    q8s32  = vmull_s16(*d4s16, d19s16);
    q15s32 = vsubw_s16(q15s32, d18s16);
    q9s32  = vmull_s16(*d5s16, d19s16);

    q10s32 = vaddq_s32(q10s32, q13s32);
    q10s32 = vaddq_s32(q10s32, q8s32);
    q11s32 = vsubq_s32(q11s32, q14s32);
    q8s32  = vdupq_n_s32(sinpi_3_9);
    q11s32 = vsubq_s32(q11s32, q9s32);
    q15s32 = vmulq_s32(q15s32, q8s32);

    q13s32 = vaddq_s32(q10s32, q12s32);
    q10s32 = vaddq_s32(q10s32, q11s32);
    q14s32 = vaddq_s32(q11s32, q12s32);
    q10s32 = vsubq_s32(q10s32, q12s32);

    d16s16 = vqrshrn_n_s32(q13s32, 14);
    d17s16 = vqrshrn_n_s32(q14s32, 14);
    d18s16 = vqrshrn_n_s32(q15s32, 14);
    d19s16 = vqrshrn_n_s32(q10s32, 14);

    *q8s16 = vcombine_s16(d16s16, d17s16);
    *q9s16 = vcombine_s16(d18s16, d19s16);
}

void vp9_iht4x4_16_add_neon(int16_t *input,
                            uint8_t *dest,
                            int dest_stride,
                            int tx_type)
{
    uint8x8_t d26u8, d27u8;
    int16x4_t d0s16, d1s16, d2s16, d3s16, d4s16, d5s16;
    uint32x2_t d26u32, d27u32;
    int16x8_t q3s16, q8s16, q9s16;
    uint16x8_t q8u16, q9u16;

    d26u32 = d27u32 = vdup_n_u32(0);

    q8s16 = vld1q_s16(input); input += 8;
    q9s16 = vld1q_s16(input);

    transpose4x4(&q8s16, &q9s16);

    switch (tx_type) {
    case 2: {  // idct_iadst
        // generate constantsyy
        generate_cosine_constants(&d0s16, &d1s16, &d2s16);
        generate_sine_constants(&d3s16, &d4s16, &d5s16, &q3s16);

        // first transform rows
        iadst4x4_1d(&d3s16, &d4s16, &d5s16, &q3s16, &q8s16, &q9s16);

        // transpose the matrix
        transpose4x4(&q8s16, &q9s16);

        // then transform columns
        idct4x4_1d(&d0s16, &d1s16, &d2s16, &q8s16, &q9s16);
        break;
    }
    case 3: {  // iadst_iadst
        // generate constants
        generate_sine_constants(&d3s16, &d4s16, &d5s16, &q3s16);

        // first transform rows
        iadst4x4_1d(&d3s16, &d4s16, &d5s16, &q3s16, &q8s16, &q9s16);

        // transpose the matrix
        transpose4x4(&q8s16, &q9s16);

        // then transform columns
        iadst4x4_1d(&d3s16, &d4s16, &d5s16, &q3s16, &q8s16, &q9s16);
        break;
    }
    default: {  // iadst_idct
        // generate constants
        generate_cosine_constants(&d0s16, &d1s16, &d2s16);
        generate_sine_constants(&d3s16, &d4s16, &d5s16, &q3s16);

        // first transform rows
        idct4x4_1d(&d0s16, &d1s16, &d2s16, &q8s16, &q9s16);

        // transpose the matrix
        transpose4x4(&q8s16, &q9s16);

        // then transform columns
        iadst4x4_1d(&d3s16, &d4s16, &d5s16, &q3s16, &q8s16, &q9s16);
        break;
    }
    }

    q8s16 = vrshrq_n_s16(q8s16, 4);
    q9s16 = vrshrq_n_s16(q9s16, 4);

    d26u32 = vld1_lane_u32((const uint32_t *)dest, d26u32, 0);
    dest += dest_stride;
    d26u32 = vld1_lane_u32((const uint32_t *)dest, d26u32, 1);
    dest += dest_stride;
    d27u32 = vld1_lane_u32((const uint32_t *)dest, d27u32, 0);
    dest += dest_stride;
    d27u32 = vld1_lane_u32((const uint32_t *)dest, d27u32, 1);

    q8u16 = vaddw_u8(vreinterpretq_u16_s16(q8s16), vreinterpret_u8_u32(d26u32));
    q9u16 = vaddw_u8(vreinterpretq_u16_s16(q9s16), vreinterpret_u8_u32(d27u32));

    d26u8 = vqmovun_s16(vreinterpretq_s16_u16(q8u16));
    d27u8 = vqmovun_s16(vreinterpretq_s16_u16(q9u16));

    vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(d27u8), 1);
    dest -= dest_stride;
    vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(d27u8), 0);
    dest -= dest_stride;
    vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(d26u8), 1);
    dest -= dest_stride;
    vst1_lane_u32((uint32_t *)dest, vreinterpret_u32_u8(d26u8), 0);
}
