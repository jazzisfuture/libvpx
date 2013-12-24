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
#include "vpx_scale/yv12config.h"

void vp8_yv12_extend_frame_borders_neon(
        YV12_BUFFER_CONFIG *dst_ybc) {
    int32_t i, j, uv, border, pln_stride;
    uint8_t *s_ptr1, *s_ptr2;
    uint8_t *d_ptr1, *d_ptr2;
    uint8x16_t q0u8, q1u8, q2u8, q3u8, q4u8, q5u8, q6u8, q7u8;
    uint8x16_t q8u8, q9u8, q10u8, q11u8, q12u8, q13u8, q14u8, q15u8;
    uint8x8_t d0u8, d8u8;

    border = 32;
    pln_stride = dst_ybc->y_stride;
    s_ptr1 = dst_ybc->y_buffer;
    d_ptr2 = s_ptr1 + dst_ybc->y_width;
    s_ptr2 = d_ptr2 - 1;
    d_ptr1 = s_ptr1 - 32;

    // copy_left_right_y
    i = dst_ybc->y_height >> 2;
    for (; i > 0; i--) {
        q1u8 = q0u8 = vld1q_dup_u8(s_ptr1);
        s_ptr1 += pln_stride;
        q3u8 = q2u8 = vld1q_dup_u8(s_ptr2);
        s_ptr2 += pln_stride;
        q5u8 = q4u8 = vld1q_dup_u8(s_ptr1);
        s_ptr1 += pln_stride;
        q7u8 = q6u8 = vld1q_dup_u8(s_ptr2);
        s_ptr2 += pln_stride;
        q9u8 = q8u8 = vld1q_dup_u8(s_ptr1);
        s_ptr1 += pln_stride;
        q11u8 = q10u8 = vld1q_dup_u8(s_ptr2);
        s_ptr2 += pln_stride;
        q13u8 = q12u8 = vld1q_dup_u8(s_ptr1);
        s_ptr1 += pln_stride;
        q15u8 = q14u8 = vld1q_dup_u8(s_ptr2);
        s_ptr2 += pln_stride;

        vst1q_u8(d_ptr1, q0u8);
        vst1q_u8(d_ptr1 + 16, q1u8);
        d_ptr1 += pln_stride;
        vst1q_u8(d_ptr2, q2u8);
        vst1q_u8(d_ptr2 + 16, q3u8);
        d_ptr2 += pln_stride;
        vst1q_u8(d_ptr1, q4u8);
        vst1q_u8(d_ptr1 + 16, q5u8);
        d_ptr1 += pln_stride;
        vst1q_u8(d_ptr2, q6u8);
        vst1q_u8(d_ptr2 + 16, q7u8);
        d_ptr2 += pln_stride;
        vst1q_u8(d_ptr1, q8u8);
        vst1q_u8(d_ptr1 + 16, q9u8);
        d_ptr1 += pln_stride;
        vst1q_u8(d_ptr2, q10u8);
        vst1q_u8(d_ptr2 + 16, q11u8);
        d_ptr2 += pln_stride;
        vst1q_u8(d_ptr1, q12u8);
        vst1q_u8(d_ptr1 + 16, q13u8);
        d_ptr1 += pln_stride;
        vst1q_u8(d_ptr2, q14u8);
        vst1q_u8(d_ptr2 + 16, q15u8);
        d_ptr2 += pln_stride;
    }

    // Now copy the top and bottom source lines into each
    // line of the respective borders
    s_ptr1 = dst_ybc->y_buffer - border;
    d_ptr2 = s_ptr1 + (dst_ybc->y_height * pln_stride);
    s_ptr2 = d_ptr2 - pln_stride;
    d_ptr1 = s_ptr1 - (border * pln_stride);

    i = pln_stride >> 7;
    for (; i > 0; i--) {  // if plane stride > 128
        q0u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q1u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q2u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q3u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q4u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q5u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q6u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q7u8  = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q8u8  = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q9u8  = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q10u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q11u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q12u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q13u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q14u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        q15u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        for (j = 0; j < border; j++) {
            vst1q_u8(d_ptr1, q0u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q1u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q2u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q3u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q4u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q5u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q6u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr1, q7u8);
            d_ptr1 += 16;
            vst1q_u8(d_ptr2, q8u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q9u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q10u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q11u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q12u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q13u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q14u8);
            d_ptr2 += 16;
            vst1q_u8(d_ptr2, q15u8);
            d_ptr2 += 16;

            d_ptr1 += pln_stride - 128;
            d_ptr2 += pln_stride - 128;
        }
        d_ptr1 = s_ptr1 - (border * pln_stride);
        d_ptr2 = s_ptr2 + pln_stride;
    }

    // extra_top_bottom_y
    i = (pln_stride >> 4) & 0x7;
    for (; i > 0; i--) {
        q0u8 = vld1q_u8(s_ptr1);
        s_ptr1 += 16;
        q2u8 = vld1q_u8(s_ptr2);
        s_ptr2 += 16;
        for (j = 0; j < 32; j++) {
            vst1q_u8(d_ptr1, q0u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q2u8);
            d_ptr2 += pln_stride;
        }
        d_ptr1 = s_ptr1 - (border * pln_stride);
        d_ptr2 = s_ptr2 + pln_stride;
    }

    // Border copy for U, V planes
    border = 16;
    pln_stride = dst_ybc->uv_stride;
    for (uv = 0; uv < 2; uv++) {  // for u and v plane
        if (uv == 0)
            s_ptr1 = dst_ybc->u_buffer;
        else if (uv == 1)
            s_ptr1 = dst_ybc->v_buffer;

        d_ptr1 = s_ptr1 - border;
        d_ptr2 = s_ptr1 + dst_ybc->uv_width;
        s_ptr2 = d_ptr2 - 1;
        i = dst_ybc->uv_height >> 3;
        for (; i > 0; i--) {
            // copy_left_right_uv
            q0u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q1u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q2u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q3u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q4u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q5u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q6u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q7u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q8u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q9u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q10u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q11u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q12u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q13u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;
            q14u8 = vld1q_dup_u8(s_ptr1);
            s_ptr1 += pln_stride;
            q15u8 = vld1q_dup_u8(s_ptr2);
            s_ptr2 += pln_stride;

            vst1q_u8(d_ptr1, q0u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q1u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q2u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q3u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q4u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q5u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q6u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q7u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q8u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q9u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q10u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q11u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q12u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q13u8);
            d_ptr2 += pln_stride;
            vst1q_u8(d_ptr1, q14u8);
            d_ptr1 += pln_stride;
            vst1q_u8(d_ptr2, q15u8);
            d_ptr2 += pln_stride;
        }

        // Now copy the top and bottom source lines into each line
        // of the respective borders
        if (uv == 0)
            s_ptr1 = dst_ybc->u_buffer - border;
        else if (uv == 1)
            s_ptr1 = dst_ybc->v_buffer - border;

        d_ptr2 = s_ptr1 + (dst_ybc->uv_height * pln_stride);
        s_ptr2 = d_ptr2 - pln_stride;
        d_ptr1 = s_ptr1 - (border * pln_stride);

        i = (pln_stride >> 6);
        for (; i > 0; i--) {  // if plane stride > 64
            q0u8 = vld1q_u8(s_ptr1);
            s_ptr1 += 16;
            q1u8 = vld1q_u8(s_ptr1);
            s_ptr1 += 16;
            q2u8 = vld1q_u8(s_ptr1);
            s_ptr1 += 16;
            q3u8 = vld1q_u8(s_ptr1);
            s_ptr1 += 16;
            q8u8 = vld1q_u8(s_ptr2);
            s_ptr2 += 16;
            q9u8 = vld1q_u8(s_ptr2);
            s_ptr2 += 16;
            q10u8 = vld1q_u8(s_ptr2);
            s_ptr2 += 16;
            q11u8 = vld1q_u8(s_ptr2);
            s_ptr2 += 16;
            for (j = 0; j < border; j++) {
                vst1q_u8(d_ptr1, q0u8);
                d_ptr1 += 16;
                vst1q_u8(d_ptr1, q1u8);
                d_ptr1 += 16;
                vst1q_u8(d_ptr1, q2u8);
                d_ptr1 += 16;
                vst1q_u8(d_ptr1, q3u8);
                d_ptr1 += 16;
                vst1q_u8(d_ptr2, q8u8);
                d_ptr2 += 16;
                vst1q_u8(d_ptr2, q9u8);
                d_ptr2 += 16;
                vst1q_u8(d_ptr2, q10u8);
                d_ptr2 += 16;
                vst1q_u8(d_ptr2, q11u8);
                d_ptr2 += 16;

                d_ptr1 += pln_stride - 64;
                d_ptr2 += pln_stride - 64;
            }
            d_ptr1 = s_ptr1 - (border * pln_stride);
            d_ptr2 = s_ptr2 + pln_stride;
        }

        // extra_top_bottom_uv
        i = (pln_stride >> 3) & 0x7;
        for (; i > 0; i--) {
            d0u8 = vld1_u8(s_ptr1);
            s_ptr1 += 8;
            d8u8 = vld1_u8(s_ptr2);
            s_ptr2 += 8;
            for (j = 0; j < 16; j++) {
                vst1_u8(d_ptr1, d0u8);
                d_ptr1 += pln_stride;
                vst1_u8(d_ptr2, d8u8);
                d_ptr2 += pln_stride;
            }
            d_ptr1 = s_ptr1 - (border * pln_stride);
            d_ptr2 = s_ptr2 + pln_stride;
        }
    }
    return;
}
