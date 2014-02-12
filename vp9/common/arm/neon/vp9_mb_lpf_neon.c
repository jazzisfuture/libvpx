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

static int vp9_wide_mbfilter_neon(
        uint8x8_t dblimit,    // flimit
        uint8x8_t dlimit,     // limit
        uint8x8_t dthresh,    // thresh
        uint8x8_t d0u8,       // p7
        uint8x8_t d1u8,       // p6
        uint8x8_t d2u8,       // p5
        uint8x8_t d3u8,       // p4
        uint8x8_t d4u8,       // p3
        uint8x8_t d5u8,       // p2
        uint8x8_t d6u8,       // p1
        uint8x8_t d7u8,       // p0
        uint8x8_t d8u8,       // q0
        uint8x8_t d9u8,       // q1
        uint8x8_t d10u8,      // q2
        uint8x8_t d11u8,      // q3
        uint8x8_t d12u8,      // q4
        uint8x8_t d13u8,      // q5
        uint8x8_t d14u8,      // q6
        uint8x8_t d15u8,      // q7
        uint8x8_t *d1ru8,
        uint8x8_t *d2ru8,
        uint8x8_t *d3ru8,
        uint8x8_t *d16ru8,
        uint8x8_t *d18ru8,
        uint8x8_t *d19ru8,
        uint8x8_t *d20ru8,
        uint8x8_t *d21ru8,
        uint8x8_t *d22ru8,
        uint8x8_t *d23ru8,
        uint8x8_t *d24ru8,
        uint8x8_t *d25ru8,
        uint8x8_t *d26ru8,
        uint8x8_t *d27ru8) {
    int32_t r5, r6;
    uint8x8_t d16u8, d17u8, d18u8, d19u8, d20u8, d21u8, d22u8, d23u8, d24u8;
    uint8x8_t d25u8, d26u8, d27u8, d28u8, d29u8, d30u8;
    int16x8_t q15s16;
    uint16x8_t q4u16, q10u16, q12u16, q13u16, q14u16, q15u16;
    int8x8_t d23s8, d24s8, d25s8, d26s8, d27s8, d28s8, d29s8, d30s8;

    d19u8 = vabd_u8(d4u8, d5u8);
    d20u8 = vabd_u8(d5u8, d6u8);
    d21u8 = vabd_u8(d6u8, d7u8);
    d22u8 = vabd_u8(d9u8, d8u8);
    d23u8 = vabd_u8(d10u8, d9u8);
    d24u8 = vabd_u8(d11u8, d10u8);

    d19u8 = vmax_u8(d19u8, d20u8);
    d20u8 = vmax_u8(d21u8, d22u8);
    d23u8 = vmax_u8(d23u8, d24u8);
    d19u8 = vmax_u8(d19u8, d20u8);

    d24u8 = vabd_u8(d7u8, d8u8);

    d19u8 = vmax_u8(d19u8, d23u8);

    d23u8 = vabd_u8(d6u8, d9u8);
    d24u8 = vqadd_u8(d24u8, d24u8);

    d19u8 = vcge_u8(dlimit, d19u8);

    d25u8 = vabd_u8(d7u8, d5u8);
    d26u8 = vabd_u8(d8u8, d10u8);
    d27u8 = vabd_u8(d4u8, d7u8);
    d28u8 = vabd_u8(d11u8, d8u8);

    d25u8 = vmax_u8(d25u8, d26u8);
    d26u8 = vmax_u8(d27u8, d28u8);
    d25u8 = vmax_u8(d25u8, d26u8);
    d20u8 = vmax_u8(d20u8, d25u8);

    d23u8 = vshr_n_u8(d23u8, 1);
    d24u8 = vqadd_u8(d24u8, d23u8);

    d30u8 = vdup_n_u8(1);
    d24u8 = vcge_u8(dblimit, d24u8);

    d20u8 = vcge_u8(d30u8, d20u8);

    d19u8 = vand_u8(d19u8, d24u8);

    d21u8 = vcgt_u8(d21u8, dthresh);
    d22u8 = vcgt_u8(d22u8, dthresh);
    d21u8 = vorr_u8(d21u8, d22u8);

    d16u8 = vand_u8(d20u8, d19u8);
    r5 = vget_lane_u32(vreinterpret_u32_u8(d16u8), 0);
    r6 = vget_lane_u32(vreinterpret_u32_u8(d16u8), 1);

    d22u8 = vabd_u8(d3u8, d7u8);
    d23u8 = vabd_u8(d12u8, d8u8);
    d24u8 = vabd_u8(d7u8, d2u8);
    d25u8 = vabd_u8(d8u8, d13u8);
    d26u8 = vabd_u8(d1u8, d7u8);
    d27u8 = vabd_u8(d14u8, d8u8);
    d28u8 = vabd_u8(d0u8, d7u8);
    d29u8 = vabd_u8(d15u8, d8u8);

    d22u8 = vmax_u8(d22u8, d23u8);
    d23u8 = vmax_u8(d24u8, d25u8);
    d24u8 = vmax_u8(d26u8, d27u8);
    d25u8 = vmax_u8(d28u8, d29u8);

    d26u8 = vmax_u8(d22u8, d23u8);
    d27u8 = vmax_u8(d24u8, d25u8);
    d23u8 = vmax_u8(d26u8, d27u8);

    d18u8 = vcge_u8(d30u8, d23u8);

    d22u8 = vdup_n_u8(0x80);
    d23u8 = veor_u8(d8u8, d22u8);
    d24u8 = veor_u8(d7u8, d22u8);
    d25u8 = veor_u8(d6u8, d22u8);
    d26u8 = veor_u8(d9u8, d22u8);

    d27s8 = vdup_n_s8(3);

    d28s8 = vsub_s8(vreinterpret_s8_u8(d23u8),
                    vreinterpret_s8_u8(d24u8));
    d29s8 = vqsub_s8(vreinterpret_s8_u8(d25u8),
                     vreinterpret_s8_u8(d26u8));
    q15s16 = vmull_s8(d28s8, d27s8);
    d29s8 = vand_s8(d29s8, vreinterpret_s8_u8(d21u8));
    q15s16 = vaddw_s8(q15s16, d29s8);
    d29s8 = vdup_n_s8(4);

    d28s8 = vqmovn_s16(q15s16);

    d28s8 = vand_s8(d28s8, vreinterpret_s8_u8(d19u8));

    d30s8 = vqadd_s8(d28s8, d27s8);
    d29s8 = vqadd_s8(d28s8, d29s8);
    d30s8 = vshr_n_s8(d30s8, 3);
    d29s8 = vshr_n_s8(d29s8, 3);

    d24s8 = vqadd_s8(vreinterpret_s8_u8(d24u8), d30s8);
    d23s8 = vqsub_s8(vreinterpret_s8_u8(d23u8), d29s8);

    d29s8 = vrshr_n_s8(d29s8, 1);
    d29s8 = vbic_s8(d29s8, vreinterpret_s8_u8(d21u8));

    d25s8 = vqadd_s8(vreinterpret_s8_u8(d25u8), d29s8);
    d26s8 = vqsub_s8(vreinterpret_s8_u8(d26u8), d29s8);

    d24u8 = veor_u8(vreinterpret_u8_s8(d24s8), d22u8);
    d23u8 = veor_u8(vreinterpret_u8_s8(d23s8), d22u8);
    d25u8 = veor_u8(vreinterpret_u8_s8(d25s8), d22u8);
    d26u8 = veor_u8(vreinterpret_u8_s8(d26s8), d22u8);

    if (r5 == 0 && r6 == 0) {
        *d23ru8 = d23u8;
        *d24ru8 = d24u8;
        *d25ru8 = d25u8;
        *d26ru8 = d26u8;
        return 1;
    }

    d17u8 = vand_u8(d18u8, d16u8);
    r5 = vget_lane_u32(vreinterpret_u32_u8(d17u8), 0);
    r6 = vget_lane_u32(vreinterpret_u32_u8(d17u8), 1);

    d29u8 = vdup_n_u8(2);
    q15u16 = vaddl_u8(d7u8, d8u8);
    q15u16 = vmlal_u8(q15u16, d4u8, vreinterpret_u8_s8(d27s8));
    q15u16 = vmlal_u8(q15u16, d5u8, d29u8);
    q10u16 = vaddl_u8(d4u8, d5u8);
    q15u16 = vaddw_u8(q15u16, d6u8);
    q14u16 = vaddl_u8(d6u8, d9u8);
    d18u8 = vqrshrn_n_u16(q15u16, 3);

    q15u16 = vsubq_u16(q15u16, q10u16);
    q10u16 = vaddl_u8(d4u8, d6u8);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d7u8, d10u8);
    d19u8 = vqrshrn_n_u16(q15u16, 3);

    q15u16 = vsubq_u16(q15u16, q10u16);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d8u8, d11u8);
    d20u8 = vqrshrn_n_u16(q15u16, 3);

    q15u16 = vsubw_u8(q15u16, d4u8);
    q15u16 = vsubw_u8(q15u16, d7u8);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d9u8, d11u8);
    d21u8 = vqrshrn_n_u16(q15u16, 3);

    q15u16 = vsubw_u8(q15u16, d5u8);
    q15u16 = vsubw_u8(q15u16, d8u8);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d10u8, d11u8);
    d22u8 = vqrshrn_n_u16(q15u16, 3);

    q15u16 = vsubw_u8(q15u16, d6u8);
    q15u16 = vsubw_u8(q15u16, d9u8);
    q15u16 = vaddq_u16(q15u16, q14u16);
    d27u8 = vqrshrn_n_u16(q15u16, 3);

    d18u8 = vbsl_u8(d16u8, d18u8, d5u8);
    d19u8 = vbsl_u8(d16u8, d19u8, d25u8);
    d20u8 = vbsl_u8(d16u8, d20u8, d24u8);
    d21u8 = vbsl_u8(d16u8, d21u8, d23u8);
    d22u8 = vbsl_u8(d16u8, d22u8, d26u8);

    d23u8 = vbsl_u8(d16u8, d27u8, d23u8);
    d23u8 = vbsl_u8(d16u8, d23u8, d10u8);

    if (r5 == 0 && r6 == 0) {
        *d18ru8 = d18u8;
        *d19ru8 = d19u8;
        *d20ru8 = d20u8;
        *d21ru8 = d21u8;
        *d22ru8 = d22u8;
        *d23ru8 = d23u8;
        return 2;
    }

    d16u8 = vdup_n_u8(7);
    q15u16 = vaddl_u8(d7u8, d8u8);
    q12u16 = vaddl_u8(d2u8, d3u8);
    q13u16 = vaddl_u8(d4u8, d5u8);
    q14u16 = vaddl_u8(d1u8, d6u8);
    q15u16 = vmlal_u8(q15u16, d0u8, d16u8);
    q12u16 = vaddq_u16(q12u16, q13u16);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d2u8, d9u8);
    q15u16 = vaddq_u16(q15u16, q12u16);
    q12u16 = vaddl_u8(d0u8, d1u8);
    q15u16 = vaddw_u8(q15u16, d1u8);
    q13u16 = vaddl_u8(d0u8, d2u8);
    q14u16 = vaddq_u16(q15u16, q14u16);
    d16u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q14u16, q12u16);
    q14u16 = vaddl_u8(d3u8, d10u8);
    d24u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q13u16);
    q13u16 = vaddl_u8(d0u8, d3u8);
    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d4u8, d11u8);
    d25u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vaddq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d0u8, d4u8);
    q15u16 = vsubq_u16(q15u16, q13u16);
    q14u16 = vsubq_u16(q15u16, q14u16);
    d26u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vaddw_u8(q14u16, d5u8);
    q14u16 = vaddl_u8(d0u8, d5u8);
    q15u16 = vaddw_u8(q15u16, d12u8);
    d26u8 = vbsl_u8(d17u8, d26u8, d4u8);
    d27u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d0u8, d6u8);
    q15u16 = vaddw_u8(q15u16, d6u8);
    q15u16 = vaddw_u8(q15u16, d13u8);
    d27u8 = vbsl_u8(d17u8, d27u8, d18u8);
    d18u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d0u8, d7u8);
    q15u16 = vaddw_u8(q15u16, d7u8);
    q15u16 = vaddw_u8(q15u16, d14u8);
    d18u8 = vbsl_u8(d17u8, d18u8, d19u8);
    d19u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d1u8, d8u8);
    q15u16 = vaddw_u8(q15u16, d8u8);
    q15u16 = vaddw_u8(q15u16, d15u8);
    d19u8 = vbsl_u8(d17u8, d19u8, d20u8);
    d20u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d2u8, d9u8);
    q15u16 = vaddw_u8(q15u16, d9u8);
    q4u16 = vaddl_u8(d10u8, d15u8);
    q15u16 = vaddw_u8(q15u16, d15u8);
    d20u8 = vbsl_u8(d17u8, d20u8, d21u8);
    d21u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d3u8, d10u8);
    q15u16 = vaddq_u16(q15u16, q4u16);
    q4u16 = vaddl_u8(d11u8, d15u8);
    d21u8 = vbsl_u8(d17u8, d21u8, d22u8);
    d22u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d4u8, d11u8);
    q15u16 = vaddq_u16(q15u16, q4u16);
    q4u16 = vaddl_u8(d12u8, d15u8);
    d22u8 = vbsl_u8(d17u8, d22u8, d23u8);
    d23u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d5u8, d12u8);
    q15u16 = vaddq_u16(q15u16, q4u16);
    q4u16 = vaddl_u8(d13u8, d15u8);
    d16u8 = vbsl_u8(d17u8, d16u8, d1u8);
    d1u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    q14u16 = vaddl_u8(d6u8, d13u8);
    q15u16 = vaddq_u16(q15u16, q4u16);
    q4u16 = vaddl_u8(d14u8, d15u8);
    d24u8 = vbsl_u8(d17u8, d24u8, d2u8);
    d2u8 = vqrshrn_n_u16(q15u16, 4);

    q15u16 = vsubq_u16(q15u16, q14u16);
    d25u8 = vbsl_u8(d17u8, d25u8, d3u8);
    q15u16 = vaddq_u16(q15u16, q4u16);
    d23u8 = vbsl_u8(d17u8, d23u8, d11u8);
    d3u8 = vqrshrn_n_u16(q15u16, 4);
    d1u8 = vbsl_u8(d17u8, d1u8, d12u8);
    d2u8 = vbsl_u8(d17u8, d2u8, d13u8);
    d3u8 = vbsl_u8(d17u8, d3u8, d14u8);

    *d1ru8 = d1u8;
    *d2ru8 = d2u8;
    *d3ru8 = d3u8;
    *d16ru8 = d16u8;
    *d18ru8 = d18u8;
    *d19ru8 = d19u8;
    *d20ru8 = d20u8;
    *d21ru8 = d21u8;
    *d22ru8 = d22u8;
    *d23ru8 = d23u8;
    *d24ru8 = d24u8;
    *d25ru8 = d25u8;
    *d26ru8 = d26u8;
    *d27ru8 = d27u8;
    return 0;
}

void vp9_lpf_horizontal_16_neon(
        uint8_t *src,
        int p,
        const uint8_t *blimit,
        const uint8_t *limit,
        const uint8_t *thresh,
        int count) {
    int ret;
    uint8_t *s, *st;
    uint8x8_t dblimit, dlimit, dthresh;
    uint8x8_t d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8, d8u8, d9u8;
    uint8x8_t d10u8, d11u8, d12u8, d13u8, d14u8, d15u8, d16u8, d18u8;
    uint8x8_t d19u8, d20u8, d21u8, d22u8, d23u8, d24u8, d25u8, d26u8, d27u8;

    dblimit = vld1_dup_u8(blimit);
    dlimit  = vld1_dup_u8(limit);
    dthresh = vld1_dup_u8(thresh);
    for (; count > 0; count--, src += 8) {
        s = src - (p << 3);  // move src pointer down by 8 lines
        d0u8  = vld1_u8(s); s += p;
        d1u8  = vld1_u8(s); s += p;
        d2u8  = vld1_u8(s); s += p;
        d3u8  = vld1_u8(s); s += p;
        d4u8  = vld1_u8(s); s += p;
        d5u8  = vld1_u8(s); s += p;
        d6u8  = vld1_u8(s); s += p;
        d7u8  = vld1_u8(s); s += p;
        d8u8  = vld1_u8(s); s += p;
        d9u8  = vld1_u8(s); s += p;
        d10u8 = vld1_u8(s); s += p;
        d11u8 = vld1_u8(s); s += p;
        d12u8 = vld1_u8(s); s += p;
        d13u8 = vld1_u8(s); s += p;
        d14u8 = vld1_u8(s); s += p;
        d15u8 = vld1_u8(s);

        ret = vp9_wide_mbfilter_neon(dblimit, dlimit, dthresh,
                  d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8,
                  d8u8, d9u8, d10u8, d11u8, d12u8, d13u8, d14u8, d15u8,
                  &d1u8, &d2u8, &d3u8, &d16u8, &d18u8, &d19u8, &d20u8,
                  &d21u8, &d22u8, &d23u8, &d24u8, &d25u8, &d26u8, &d27u8);

        switch (ret) {
        case 1: {
            st = src - (p << 1);
            vst1_u8(st, d25u8); st += p;
            vst1_u8(st, d24u8); st += p;
            vst1_u8(st, d23u8); st += p;
            vst1_u8(st, d26u8);
            break;
        }
        case 2: {  // h_mbfilter
            st = src - p * 3;
            vst1_u8(st, d18u8); st += p;
            vst1_u8(st, d19u8); st += p;
            vst1_u8(st, d20u8); st += p;
            vst1_u8(st, d21u8); st += p;
            vst1_u8(st, d22u8); st += p;
            vst1_u8(st, d23u8);
            break;
        }
        default: {  // h_wide_mbfilter
            st = src - p * 7;
            vst1_u8(st, d16u8); st += p;
            vst1_u8(st, d24u8); st += p;
            vst1_u8(st, d25u8); st += p;
            vst1_u8(st, d26u8); st += p;
            vst1_u8(st, d27u8); st += p;
            vst1_u8(st, d18u8); st += p;
            vst1_u8(st, d19u8); st += p;
            vst1_u8(st, d20u8); st += p;
            vst1_u8(st, d21u8); st += p;
            vst1_u8(st, d22u8); st += p;
            vst1_u8(st, d23u8); st += p;
            vst1_u8(st, d1u8);  st += p;
            vst1_u8(st, d2u8);  st += p;
            vst1_u8(st, d3u8);
            break;
        }
        }
    }
    return;
}

void vp9_lpf_vertical_16_neon(
        uint8_t *src,
        int p,
        const uint8_t *blimit,
        const uint8_t *limit,
        const uint8_t *thresh) {
    int ret;
    uint8_t *s, *st;
    uint8x8_t dblimit, dlimit, dthresh;
    uint8x8_t d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8, d8u8, d9u8;
    uint8x8_t d10u8, d11u8, d12u8, d13u8, d14u8, d15u8, d16u8, d18u8;
    uint8x8_t d19u8, d20u8, d21u8, d22u8, d23u8, d24u8, d25u8, d26u8, d27u8;
    uint8x16_t q0u8, q1u8, q2u8, q3u8, q4u8, q5u8, q6u8, q7u8;
    uint8x8x2_t d0x2u8, d1x2u8, d2x2u8, d3x2u8, d4x2u8, d5x2u8, d6x2u8, d7x2u8;
    uint8x8x3_t d0x3u8, d1x3u8;
    uint8x8x4_t d0x4u8;
    uint16x4x2_t d0x2u16, d1x2u16, d2x2u16, d3x2u16;
    uint32x2x2_t d0x2u32, d1x2u32, d2x2u32, d3x2u32;
    uint32x4x2_t q0x2u32, q1x2u32, q2x2u32, q3x2u32;
    uint16x8x2_t q0x2u16, q1x2u16, q2x2u16, q3x2u16;

    dblimit = vld1_dup_u8(blimit);
    dlimit  = vld1_dup_u8(limit);
    dthresh = vld1_dup_u8(thresh);

    s = src - 8;
    d0u8  = vld1_u8(s);
    d8u8  = vld1_u8(s + 8); s += p;
    d1u8  = vld1_u8(s);
    d9u8  = vld1_u8(s + 8); s += p;
    d2u8  = vld1_u8(s);
    d10u8 = vld1_u8(s + 8); s += p;
    d3u8  = vld1_u8(s);
    d11u8 = vld1_u8(s + 8); s += p;
    d4u8  = vld1_u8(s);
    d12u8 = vld1_u8(s + 8); s += p;
    d5u8  = vld1_u8(s);
    d13u8 = vld1_u8(s + 8); s += p;
    d6u8  = vld1_u8(s);
    d14u8 = vld1_u8(s + 8); s += p;
    d7u8  = vld1_u8(s);
    d15u8 = vld1_u8(s + 8);

    q0u8 = vcombine_u8(d0u8, d1u8);
    q1u8 = vcombine_u8(d2u8, d3u8);
    q2u8 = vcombine_u8(d4u8, d5u8);
    q3u8 = vcombine_u8(d6u8, d7u8);
    q4u8 = vcombine_u8(d8u8, d9u8);
    q5u8 = vcombine_u8(d10u8, d11u8);
    q6u8 = vcombine_u8(d12u8, d13u8);
    q7u8 = vcombine_u8(d14u8, d15u8);

    q0x2u32 = vtrnq_u32(vreinterpretq_u32_u8(q0u8),
                        vreinterpretq_u32_u8(q2u8));
    q1x2u32 = vtrnq_u32(vreinterpretq_u32_u8(q1u8),
                        vreinterpretq_u32_u8(q3u8));
    q2x2u32 = vtrnq_u32(vreinterpretq_u32_u8(q4u8),
                        vreinterpretq_u32_u8(q6u8));
    q3x2u32 = vtrnq_u32(vreinterpretq_u32_u8(q5u8),
                        vreinterpretq_u32_u8(q7u8));

    q0x2u16 = vtrnq_u16(vreinterpretq_u16_u32(q0x2u32.val[0]),
                        vreinterpretq_u16_u32(q1x2u32.val[0]));
    q1x2u16 = vtrnq_u16(vreinterpretq_u16_u32(q0x2u32.val[1]),
                        vreinterpretq_u16_u32(q1x2u32.val[1]));
    q2x2u16 = vtrnq_u16(vreinterpretq_u16_u32(q2x2u32.val[0]),
                        vreinterpretq_u16_u32(q3x2u32.val[0]));
    q3x2u16 = vtrnq_u16(vreinterpretq_u16_u32(q2x2u32.val[1]),
                        vreinterpretq_u16_u32(q3x2u32.val[1]));

    d0x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q0x2u16.val[0])),
                    vget_high_u8(vreinterpretq_u8_u16(q0x2u16.val[0])));
    d1x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q0x2u16.val[1])),
                    vget_high_u8(vreinterpretq_u8_u16(q0x2u16.val[1])));
    d2x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q1x2u16.val[0])),
                    vget_high_u8(vreinterpretq_u8_u16(q1x2u16.val[0])));
    d3x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q1x2u16.val[1])),
                    vget_high_u8(vreinterpretq_u8_u16(q1x2u16.val[1])));
    d4x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q2x2u16.val[0])),
                    vget_high_u8(vreinterpretq_u8_u16(q2x2u16.val[0])));
    d5x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q2x2u16.val[1])),
                    vget_high_u8(vreinterpretq_u8_u16(q2x2u16.val[1])));
    d6x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q3x2u16.val[0])),
                    vget_high_u8(vreinterpretq_u8_u16(q3x2u16.val[0])));
    d7x2u8 = vtrn_u8(vget_low_u8(vreinterpretq_u8_u16(q3x2u16.val[1])),
                    vget_high_u8(vreinterpretq_u8_u16(q3x2u16.val[1])));

    d0u8  = d0x2u8.val[0];
    d1u8  = d0x2u8.val[1];
    d2u8  = d1x2u8.val[0];
    d3u8  = d1x2u8.val[1];
    d4u8  = d2x2u8.val[0];
    d5u8  = d2x2u8.val[1];
    d6u8  = d3x2u8.val[0];
    d7u8  = d3x2u8.val[1];
    d8u8  = d4x2u8.val[0];
    d9u8  = d4x2u8.val[1];
    d10u8 = d5x2u8.val[0];
    d11u8 = d5x2u8.val[1];
    d12u8 = d6x2u8.val[0];
    d13u8 = d6x2u8.val[1];
    d14u8 = d7x2u8.val[0];
    d15u8 = d7x2u8.val[1];

    ret = vp9_wide_mbfilter_neon(dblimit, dlimit, dthresh,
              d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8,
              d8u8, d9u8, d10u8, d11u8, d12u8, d13u8, d14u8, d15u8,
              &d1u8, &d2u8, &d3u8, &d16u8, &d18u8, &d19u8, &d20u8,
              &d21u8, &d22u8, &d23u8, &d24u8, &d25u8, &d26u8, &d27u8);

    switch (ret) {
    case 1: {
        st = src - 2;
        d0x4u8.val[0] = d25u8;  // vswp  d23, d25
        d0x4u8.val[1] = d24u8;
        d0x4u8.val[2] = d23u8;
        d0x4u8.val[3] = d26u8;

        vst4_lane_u8(st, d0x4u8, 0); st += p;
        vst4_lane_u8(st, d0x4u8, 1); st += p;
        vst4_lane_u8(st, d0x4u8, 2); st += p;
        vst4_lane_u8(st, d0x4u8, 3); st += p;
        vst4_lane_u8(st, d0x4u8, 4); st += p;
        vst4_lane_u8(st, d0x4u8, 5); st += p;
        vst4_lane_u8(st, d0x4u8, 6); st += p;
        vst4_lane_u8(st, d0x4u8, 7);
        break;
    }
    case 2: {  // v_mbfilter
        st = src - 3;
        d0x3u8.val[0] = d18u8; 
        d0x3u8.val[1] = d19u8; 
        d0x3u8.val[2] = d20u8; 
        d1x3u8.val[0] = d21u8; 
        d1x3u8.val[1] = d22u8; 
        d1x3u8.val[2] = d23u8; 

        vst3_lane_u8(st, d0x3u8, 0);
        vst3_lane_u8(st + 3, d1x3u8, 0); st += p;
        vst3_lane_u8(st, d0x3u8, 1);
        vst3_lane_u8(st + 3, d1x3u8, 1); st += p;
        vst3_lane_u8(st, d0x3u8, 2);
        vst3_lane_u8(st + 3, d1x3u8, 2); st += p;
        vst3_lane_u8(st, d0x3u8, 3);
        vst3_lane_u8(st + 3, d1x3u8, 3); st += p;
        vst3_lane_u8(st, d0x3u8, 4);
        vst3_lane_u8(st + 3, d1x3u8, 4); st += p;
        vst3_lane_u8(st, d0x3u8, 5);
        vst3_lane_u8(st + 3, d1x3u8, 5); st += p;
        vst3_lane_u8(st, d0x3u8, 6);
        vst3_lane_u8(st + 3, d1x3u8, 6); st += p;
        vst3_lane_u8(st, d0x3u8, 7);
        vst3_lane_u8(st + 3, d1x3u8, 7);
        break;
    }
    default: {  // v_wide_mbfilter
        st = src - 8;
        d0x2u32 = vtrn_u32(vreinterpret_u32_u8(d0u8),
                           vreinterpret_u32_u8(d26u8));
        d1x2u32 = vtrn_u32(vreinterpret_u32_u8(d16u8),
                           vreinterpret_u32_u8(d27u8));
        d2x2u32 = vtrn_u32(vreinterpret_u32_u8(d24u8),
                           vreinterpret_u32_u8(d18u8));
        d3x2u32 = vtrn_u32(vreinterpret_u32_u8(d25u8),
                           vreinterpret_u32_u8(d19u8));

        d0x2u16 = vtrn_u16(vreinterpret_u16_u32(d0x2u32.val[0]),
                           vreinterpret_u16_u32(d2x2u32.val[0]));
        d1x2u16 = vtrn_u16(vreinterpret_u16_u32(d1x2u32.val[0]),
                           vreinterpret_u16_u32(d3x2u32.val[0]));
        d2x2u16 = vtrn_u16(vreinterpret_u16_u32(d0x2u32.val[1]),
                           vreinterpret_u16_u32(d2x2u32.val[1]));
        d3x2u16 = vtrn_u16(vreinterpret_u16_u32(d1x2u32.val[1]),
                           vreinterpret_u16_u32(d3x2u32.val[1]));

        d0x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[0]),
                         vreinterpret_u8_u16(d1x2u16.val[0]));
        d1x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[1]),
                         vreinterpret_u8_u16(d1x2u16.val[1]));
        d2x2u8 = vtrn_u8(vreinterpret_u8_u16(d2x2u16.val[0]),
                         vreinterpret_u8_u16(d3x2u16.val[0]));
        d3x2u8 = vtrn_u8(vreinterpret_u8_u16(d2x2u16.val[1]),
                         vreinterpret_u8_u16(d3x2u16.val[1]));

        d0u8  = d0x2u8.val[0];
        d16u8 = d0x2u8.val[1];
        d24u8 = d1x2u8.val[0];
        d25u8 = d1x2u8.val[1];
        d26u8 = d2x2u8.val[0];
        d27u8 = d2x2u8.val[1];
        d18u8 = d3x2u8.val[0];
        d19u8 = d3x2u8.val[1];

        d0x2u32 = vtrn_u32(vreinterpret_u32_u8(d20u8),
                           vreinterpret_u32_u8(d1u8));
        d1x2u32 = vtrn_u32(vreinterpret_u32_u8(d21u8),
                           vreinterpret_u32_u8(d2u8));
        d2x2u32 = vtrn_u32(vreinterpret_u32_u8(d22u8),
                           vreinterpret_u32_u8(d3u8));
        d3x2u32 = vtrn_u32(vreinterpret_u32_u8(d23u8),
                           vreinterpret_u32_u8(d15u8));

        d0x2u16 = vtrn_u16(vreinterpret_u16_u32(d0x2u32.val[0]),
                           vreinterpret_u16_u32(d2x2u32.val[0]));
        d1x2u16 = vtrn_u16(vreinterpret_u16_u32(d1x2u32.val[0]),
                           vreinterpret_u16_u32(d3x2u32.val[0]));
        d2x2u16 = vtrn_u16(vreinterpret_u16_u32(d0x2u32.val[1]),
                           vreinterpret_u16_u32(d2x2u32.val[1]));
        d3x2u16 = vtrn_u16(vreinterpret_u16_u32(d1x2u32.val[1]),
                           vreinterpret_u16_u32(d3x2u32.val[1]));

        d0x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[0]),
                         vreinterpret_u8_u16(d1x2u16.val[0]));
        d1x2u8 = vtrn_u8(vreinterpret_u8_u16(d0x2u16.val[1]),
                         vreinterpret_u8_u16(d1x2u16.val[1]));
        d2x2u8 = vtrn_u8(vreinterpret_u8_u16(d2x2u16.val[0]),
                         vreinterpret_u8_u16(d3x2u16.val[0]));
        d3x2u8 = vtrn_u8(vreinterpret_u8_u16(d2x2u16.val[1]),
                         vreinterpret_u8_u16(d3x2u16.val[1]));

        d20u8 = d0x2u8.val[0];
        d21u8 = d0x2u8.val[1];
        d22u8 = d1x2u8.val[0];
        d23u8 = d1x2u8.val[1];
        d1u8  = d2x2u8.val[0];
        d2u8  = d2x2u8.val[1];
        d3u8  = d3x2u8.val[0];
        d15u8 = d3x2u8.val[1];

        vst1_u8(st, d0u8);
        vst1_u8(st + 8, d20u8); st += p;
        vst1_u8(st, d16u8);
        vst1_u8(st + 8, d21u8); st += p;
        vst1_u8(st, d24u8);
        vst1_u8(st + 8, d22u8); st += p;
        vst1_u8(st, d25u8);
        vst1_u8(st + 8, d23u8); st += p;
        vst1_u8(st, d26u8);
        vst1_u8(st + 8, d1u8); st += p;
        vst1_u8(st, d27u8);
        vst1_u8(st + 8, d2u8); st += p;
        vst1_u8(st, d18u8);
        vst1_u8(st + 8, d3u8); st += p;
        vst1_u8(st, d19u8);
        vst1_u8(st + 8, d15u8);
        break;
    }
    }
}
