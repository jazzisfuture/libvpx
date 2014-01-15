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
#include <stdio.h>

void vp9_dc_only_idct_add_neon(
        int16_t input_dc,
        unsigned char *pred_ptr,
        int pred_stride,
        unsigned char *dst_ptr,
        int dst_stride) {
    uint32_t a1;
    uint32x2_t d2u32 = vdup_n_u32(0);
    uint32x2_t d4u32 = vdup_n_u32(0);
    uint8x8_t d2u8, d4u8;
    uint16x8_t q1u16, q2u16;
    uint16x8_t qAdd;

    printf("%s\n", __func__);

    a1 = ((input_dc * 11585 + 0x2000) >> 14);
    a1 = ((a1 * 11585 + 0x2000) >> 14);
    a1 = (a1 + 8) >> 4;

    qAdd = vdupq_n_u16((int16_t)a1);

    d2u32 = vld1_lane_u32((const uint32_t *)pred_ptr, d2u32, 0);
    pred_ptr += pred_stride;
    d2u32 = vld1_lane_u32((const uint32_t *)pred_ptr, d2u32, 1);
    pred_ptr += pred_stride;
    d4u32 = vld1_lane_u32((const uint32_t *)pred_ptr, d4u32, 0);
    pred_ptr += pred_stride;
    d4u32 = vld1_lane_u32((const uint32_t *)pred_ptr, d4u32, 1);

    q1u16 = vaddw_u8(qAdd, vreinterpret_u8_u32(d2u32));
    q2u16 = vaddw_u8(qAdd, vreinterpret_u8_u32(d4u32));

    d2u8 = vqmovun_s16(vreinterpretq_s16_u16(q1u16));
    d4u8 = vqmovun_s16(vreinterpretq_s16_u16(q2u16));

    vst1_lane_u32((uint32_t *)dst_ptr, vreinterpret_u32_u8(d2u8), 0);
    dst_ptr += dst_stride;
    vst1_lane_u32((uint32_t *)dst_ptr, vreinterpret_u32_u8(d2u8), 1);
    dst_ptr += dst_stride;
    vst1_lane_u32((uint32_t *)dst_ptr, vreinterpret_u32_u8(d4u8), 0);
    dst_ptr += dst_stride;
    vst1_lane_u32((uint32_t *)dst_ptr, vreinterpret_u32_u8(d4u8), 1);
}
