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

void vpx_yv12_copy_y_neon(
        const YV12_BUFFER_CONFIG *src_ybc,
        YV12_BUFFER_CONFIG *dst_ybc) {
    int32_t i, j, src_stride, dst_stride, src_height, src_width, r11;
    uint8_t *src_buff, *dst_buff;
    uint8_t *src_tmp, *dst_tmp;
    uint8x16_t q0u8, q1u8, q2u8, q3u8, q4u8, q5u8, q6u8, q7u8;

    src_height = src_ybc->y_height;
    src_width = src_ybc->y_width;
    src_stride = src_ybc->y_stride;
    dst_stride = dst_ybc->y_stride;
    src_buff = src_ybc->y_buffer;
    dst_buff = dst_ybc->y_buffer;

    if (src_width > 128) {
        for (j = 0; j < src_height; j++) {  // cp_src_to_dst_height_loop1
            src_tmp = src_buff + j * src_stride;
            dst_tmp = dst_buff + j * dst_stride;
            for (i = 0; i < (src_width >> 7); i++) {
                // cp_src_to_dst_width_loop1
                q0u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q1u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q2u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q3u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q4u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q5u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q6u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                q7u8 = vld1q_u8(src_tmp);
                src_tmp += 16;

                vst1q_u8(dst_tmp, q0u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q1u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q2u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q3u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q4u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q5u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q6u8);
                dst_tmp += 16;
                vst1q_u8(dst_tmp, q7u8);
                dst_tmp += 16;
            }
        }
    }

    if ((src_width & 0x7F) > 0) {  // extra_copy_needed
        r11 = src_width - (src_width & 0x7F);
        src_buff += r11;
        dst_buff += r11;
        for (j = 0; j < src_height; j++) {
            // extra_cp_src_to_dst_height_loop1
            src_tmp = src_buff + j * src_stride;
            dst_tmp = dst_buff + j * dst_stride;
            for (i = 0; i < (src_width % 128); i += 16) {
                // extra_cp_src_to_dst_width_loop1
                q0u8 = vld1q_u8(src_tmp);
                src_tmp += 16;
                vst1q_u8(dst_tmp, q0u8);
                dst_tmp += 16;
            }
        }
    }
    return;
}
