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
#include "vp8/encoder/block.h"

static const uint16_t inv_zig_zag[16] = {
    0x0001, 0x0002, 0x0006, 0x0007,
    0x0003, 0x0005, 0x0008, 0x000d,
    0x0004, 0x0009, 0x000c, 0x000e,
    0x000a, 0x000b, 0x000f, 0x0010
};

void vp8_fast_quantize_b_neon(
        BLOCK *b,
        BLOCKD *d) {
    int16_t *coeff_p, *round_p, *quant_p, *dequant_p, *qcoeff_p, *dqcoeff_p;
    uint32_t z1, z2;
    uint16x4_t d0u16;
    int16x4_t d28s16;
    uint32x2_t d0u32;
    int16x8_t q0s16, q1s16, q2s16, q3s16;
    int16x8_t q8s16, q9s16, q12s16, q13s16, q14s16, q15s16;
    uint16x8_t q0u16, q8u16, q10u16, q11u16, q14u16, q15u16;
    uint32x4_t q0u32;

    coeff_p = b->coeff;
    q0s16 = vld1q_s16(coeff_p);
    q1s16 = vld1q_s16(coeff_p + 8);

    q14s16 = vorrq_s16(q0s16, q1s16);
    d28s16 = vorr_s16(vget_low_s16(q14s16), vget_high_s16(q14s16));
    z1 = vget_lane_u32(vreinterpret_u32_s16(d28s16), 0);
    z2 = vget_lane_u32(vreinterpret_u32_s16(d28s16), 1);
    if ((z1 | z2) == 0) {  // zero_output
        *d->eob = 0;
        qcoeff_p = d->qcoeff;
        vst1q_s16(qcoeff_p, q0s16);
        vst1q_s16(qcoeff_p + 8, q1s16);
        dqcoeff_p = d->dqcoeff;
        vst1q_s16(dqcoeff_p, q0s16);
        vst1q_s16(dqcoeff_p + 8, q1s16);
        return;
    }

    q12s16 = vabsq_s16(q0s16);
    q13s16 = vabsq_s16(q1s16);
    q2s16 = vshrq_n_s16(q0s16, 15);
    q3s16 = vshrq_n_s16(q1s16, 15);

    round_p = b->round;
    q14s16 = vld1q_s16(round_p);
    q15s16 = vld1q_s16(round_p + 8);
    quant_p = b->quant_fast;
    q8s16 = vld1q_s16(quant_p);
    q9s16 = vld1q_s16(quant_p + 8);

    q12s16 = vaddq_s16(q12s16, q14s16);
    q13s16 = vaddq_s16(q13s16, q15s16);
    q12s16 = vqdmulhq_s16(q12s16, q8s16);
    q13s16 = vqdmulhq_s16(q13s16, q9s16);

    q10u16 = vld1q_u16(inv_zig_zag);  // load inverse scan order
    q11u16 = vld1q_u16(inv_zig_zag + 8);

    q8u16 = vceqq_s16(q8s16, q8s16);

    q12s16 = vshrq_n_s16(q12s16, 1);
    q13s16 = vshrq_n_s16(q13s16, 1);

    q12s16 = veorq_s16(q12s16, q2s16);
    q13s16 = veorq_s16(q13s16, q3s16);
    q12s16 = vsubq_s16(q12s16, q2s16);
    q13s16 = vsubq_s16(q13s16, q3s16);

    dequant_p = d->dequant;
    q2s16 = vld1q_s16(dequant_p);
    q3s16 = vld1q_s16(dequant_p + 8);

    q14u16 = vtstq_s16(q12s16, vreinterpretq_s16_u16(q8u16));
    q15u16 = vtstq_s16(q13s16, vreinterpretq_s16_u16(q8u16));

    qcoeff_p = d->qcoeff;
    vst1q_s16(qcoeff_p, q12s16);
    vst1q_s16(qcoeff_p + 8, q13s16);

    q10u16 = vandq_u16(q10u16, q14u16);
    q11u16 = vandq_u16(q11u16, q15u16);

    q0u16 = vmaxq_u16(q10u16, q11u16);
    d0u16 = vmax_u16(vget_low_u16(q0u16), vget_high_u16(q0u16));
    q0u32 = vmovl_u16(d0u16);

    q2s16 = vmulq_s16(q2s16, q12s16);
    q3s16 = vmulq_s16(q3s16, q13s16);

    d0u32 = vmax_u32(vget_low_u32(q0u32), vget_high_u32(q0u32));
    d0u32 = vpmax_u32(d0u32, d0u32);

    dqcoeff_p = d->dqcoeff;
    vst1q_s16(dqcoeff_p, q2s16);
    vst1q_s16(dqcoeff_p + 8, q3s16);

    vst1_lane_s8((int8_t *)d->eob, vreinterpret_s8_u32(d0u32), 0);
    return;
}

void vp8_fast_quantize_b_pair_neon(
        BLOCK *b1,
        BLOCK *b2,
        BLOCKD *d1,
        BLOCKD *d2) {
    int16_t *coeff_p, *round_p, *quant_p, *dequant_p, *qcoeff_p, *dqcoeff_p;
    uint16x4_t d0u16, d20u16;
    uint32x2_t d0u32, d20u32;
    int16x8_t q0s16, q1s16, q2s16, q3s16, q4s16, q5s16, q6s16, q7s16;
    int16x8_t q8s16, q9s16, q10s16, q11s16, q12s16, q13s16;
    uint16x8_t q0u16, q1u16, q2u16, q3u16, q6u16, q7u16, q8u16;
    uint16x8_t q10u16, q11u16, q14u16, q15u16;
    uint32x4_t q0u32, q10u32;

    coeff_p = b1->coeff;
    q0s16 = vld1q_s16(coeff_p);
    q1s16 = vld1q_s16(coeff_p + 8);
    round_p = b1->round;
    q6s16 = vld1q_s16(round_p);
    q7s16 = vld1q_s16(round_p + 8);
    quant_p = b1->quant_fast;
    q8s16 = vld1q_s16(quant_p);
    q9s16 = vld1q_s16(quant_p + 8);

    q4s16 = vabsq_s16(q0s16);
    q5s16 = vabsq_s16(q1s16);
    q2s16 = vshrq_n_s16(q0s16, 15);
    q3s16 = vshrq_n_s16(q1s16, 15);

    q4s16 = vaddq_s16(q4s16, q6s16);
    q5s16 = vaddq_s16(q5s16, q7s16);
    q4s16 = vqdmulhq_s16(q4s16, q8s16);
    q5s16 = vqdmulhq_s16(q5s16, q9s16);

    coeff_p = b2->coeff;
    q0s16 = vld1q_s16(coeff_p);
    q1s16 = vld1q_s16(coeff_p + 8);

    q10s16 = vabsq_s16(q0s16);
    q11s16 = vabsq_s16(q1s16);
    q12s16 = vshrq_n_s16(q0s16, 15);
    q13s16 = vshrq_n_s16(q1s16, 15);

    q4s16 = veorq_s16(q4s16, q2s16);
    q5s16 = veorq_s16(q5s16, q3s16);

    q10s16 = vaddq_s16(q10s16, q6s16);
    q11s16 = vaddq_s16(q11s16, q7s16);
    q10s16 = vqdmulhq_s16(q10s16, q8s16);
    q11s16 = vqdmulhq_s16(q11s16, q9s16);

    q4s16 = vshrq_n_s16(q4s16, 1);
    q5s16 = vshrq_n_s16(q5s16, 1);

    dequant_p = d1->dequant;
    q6s16 = vld1q_s16(dequant_p);
    q7s16 = vld1q_s16(dequant_p + 8);

    q4s16 = vsubq_s16(q4s16, q2s16);
    q5s16 = vsubq_s16(q5s16, q3s16);

    q10s16 = vshrq_n_s16(q10s16, 1);
    q11s16 = vshrq_n_s16(q11s16, 1);

    q10s16 = veorq_s16(q10s16, q12s16);
    q11s16 = veorq_s16(q11s16, q13s16);

    qcoeff_p = d1->qcoeff;
    vst1q_s16(qcoeff_p, q4s16);
    vst1q_s16(qcoeff_p + 8, q5s16);

    q10s16 = vsubq_s16(q10s16, q12s16);
    q11s16 = vsubq_s16(q11s16, q13s16);

    q2s16 = vmulq_s16(q6s16, q4s16);
    q3s16 = vmulq_s16(q7s16, q5s16);

    q8u16 = vceqq_s16(q8s16, q8s16);

    qcoeff_p = d2->qcoeff;
    vst1q_s16(qcoeff_p, q10s16);
    vst1q_s16(qcoeff_p + 8, q11s16);

    q12s16 = vmulq_s16(q6s16, q10s16);
    q13s16 = vmulq_s16(q7s16, q11s16);

    q14u16 = vtstq_s16(q4s16, vreinterpretq_s16_u16(q8u16));
    q15u16 = vtstq_s16(q5s16, vreinterpretq_s16_u16(q8u16));

    dqcoeff_p = d1->dqcoeff;
    vst1q_s16(dqcoeff_p, q2s16);
    vst1q_s16(dqcoeff_p + 8, q3s16);

    q6u16 = vld1q_u16(inv_zig_zag);  // load inverse scan order
    q7u16 = vld1q_u16(inv_zig_zag + 8);

    q0u16 = vandq_u16(q6u16, q14u16);
    q1u16 = vandq_u16(q7u16, q15u16);

    dqcoeff_p = d2->dqcoeff;
    vst1q_s16(dqcoeff_p, q12s16);
    vst1q_s16(dqcoeff_p + 8, q13s16);

    q2u16 = vtstq_s16(q10s16, vreinterpretq_s16_u16(q8u16));
    q3u16 = vtstq_s16(q11s16, vreinterpretq_s16_u16(q8u16));

    q0u16 = vmaxq_u16(q0u16, q1u16);
    q10u16 = vandq_u16(q6u16, q2u16);
    q11u16 = vandq_u16(q7u16, q3u16);
    q10u16 = vmaxq_u16(q10u16, q11u16);

    d0u16  = vmax_u16(vget_low_u16(q0u16), vget_high_u16(q0u16));
    d20u16 = vmax_u16(vget_low_u16(q10u16), vget_high_u16(q10u16));
    q0u32  = vmovl_u16(d0u16);
    q10u32 = vmovl_u16(d20u16);

    d0u32  = vmax_u32(vget_low_u32(q0u32), vget_high_u32(q0u32));
    d20u32 = vmax_u32(vget_low_u32(q10u32), vget_high_u32(q10u32));
    d0u32  = vpmax_u32(d0u32, d0u32);
    d20u32 = vpmax_u32(d20u32, d20u32);

    vst1_lane_s8((int8_t *)d1->eob, vreinterpret_s8_u32(d0u32), 0);
    vst1_lane_s8((int8_t *)d2->eob, vreinterpret_s8_u32(d20u32), 0);
    return;
}
