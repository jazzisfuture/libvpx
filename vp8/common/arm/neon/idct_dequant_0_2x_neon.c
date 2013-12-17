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

void idct_dequant_0_2x_neon(
        int16_t *q,
        int16_t dq,
        unsigned char *dst,
        int stride) {
    unsigned char *dst0, *dst1;
    int a0, a1;
    int16x8x2_t q2Add;
    int32x2_t d2s32, d4s32, d8s32, d10s32;
    uint8x8_t d2u8, d4u8, d8u8, d10u8;
    uint16x8_t q1u16, q2u16, q4u16, q5u16;

    d2s32 = d4s32 = d8s32 = d10s32 = vdup_n_s32(0);

    dst0 = dst;
    dst1 = dst + 4;
    d2s32 = vld1_lane_s32((const int32_t *)dst0, d2s32, 0); dst0 += stride;
    d2s32 = vld1_lane_s32((const int32_t *)dst0, d2s32, 1); dst0 += stride;
    d4s32 = vld1_lane_s32((const int32_t *)dst0, d4s32, 0); dst0 += stride;
    d4s32 = vld1_lane_s32((const int32_t *)dst0, d4s32, 1);

    d8s32 = vld1_lane_s32((const int32_t *)dst1, d8s32, 0); dst1 += stride;
    d8s32 = vld1_lane_s32((const int32_t *)dst1, d8s32, 1); dst1 += stride;
    d10s32 = vld1_lane_s32((const int32_t *)dst1, d10s32, 0); dst1 += stride;
    d10s32 = vld1_lane_s32((const int32_t *)dst1, d10s32, 1);

    a0 = ((q[0] * dq) + 4) >> 3;
    a1 = ((q[16] * dq) + 4) >> 3;
    q[0] = q[16] = 0;
    q2Add.val[0] = vdupq_n_s16((int16_t)a0);
    q2Add.val[1] = vdupq_n_s16((int16_t)a1);

    q1u16 = vaddw_u8(vreinterpretq_u16_s16(q2Add.val[0]),
                     vreinterpret_u8_s32(d2s32));
    q2u16 = vaddw_u8(vreinterpretq_u16_s16(q2Add.val[0]),
                     vreinterpret_u8_s32(d4s32));
    q4u16 = vaddw_u8(vreinterpretq_u16_s16(q2Add.val[1]),
                     vreinterpret_u8_s32(d8s32));
    q5u16 = vaddw_u8(vreinterpretq_u16_s16(q2Add.val[1]),
                     vreinterpret_u8_s32(d10s32));

    d2u8 = vqmovun_s16(vreinterpretq_s16_u16(q1u16));
    d4u8 = vqmovun_s16(vreinterpretq_s16_u16(q2u16));
    d8u8 = vqmovun_s16(vreinterpretq_s16_u16(q4u16));
    d10u8 = vqmovun_s16(vreinterpretq_s16_u16(q5u16));

    d2s32 = vreinterpret_s32_u8(d2u8);
    d4s32 = vreinterpret_s32_u8(d4u8);
    d8s32 = vreinterpret_s32_u8(d8u8);
    d10s32 = vreinterpret_s32_u8(d10u8);

    dst0 = dst;
    dst1 = dst + 4;
    vst1_lane_s32((int32_t *)dst0, d2s32, 0); dst0 += stride;
    vst1_lane_s32((int32_t *)dst0, d2s32, 1); dst0 += stride;
    vst1_lane_s32((int32_t *)dst0, d4s32, 0); dst0 += stride;
    vst1_lane_s32((int32_t *)dst0, d4s32, 1);

    vst1_lane_s32((int32_t *)dst1, d8s32, 0); dst1 += stride;
    vst1_lane_s32((int32_t *)dst1, d8s32, 1); dst1 += stride;
    vst1_lane_s32((int32_t *)dst1, d10s32, 0); dst1 += stride;
    vst1_lane_s32((int32_t *)dst1, d10s32, 1);
    return;
}
