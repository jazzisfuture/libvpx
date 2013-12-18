/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * ARMv8 support by rewrite NEON assembly to NEON intrinsics.
 * Enable with -mfpu=neon compile option.
 *
 * This file includes below assembly code.
 * vp8_subpixelvariance16x16s_neon.asm
 * vp8_subpixelvariance16x16_neon.asm
 * vp8_subpixelvariance8x8_neon.as
 *
 * <james.yu at linaro.org>
 * SEEMS UNUSED. I didn't rewrite it to useing NEON intrinsics.
 * I will rewrite it if needed.
 */

#include <arm_neon.h>
#include <stdio.h>

unsigned int vp8_sub_pixel_variance8x8_neon(
        const unsigned char *src_ptr,
        int src_pixels_per_line,
        int xoffset,
        int yoffset,
        const unsigned char *dst_ptr,
        int dst_pixels_per_line,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_sub_pixel_variance16x16_neon_func(
        const unsigned char *src_ptr,
        int src_pixels_per_line,
        int xoffset,
        int yoffset,
        const unsigned char *dst_ptr,
        int dst_pixels_per_line,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_sub_pixel_variance16x16s_neon(
        const unsigned char *src_ptr,
        int src_pixels_per_line,
        int xoffset,
        int yoffset,
        const unsigned char *dst_ptr,
        int dst_pixels_per_line,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance_halfpixvar16x16_v_neon(
        const unsigned char *src_ptr,
        int  source_stride,
        const unsigned char *ref_ptr,
        int  recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance_halfpixvar16x16_hv_neon(
        const unsigned char *src_ptr,
        int  source_stride,
        const unsigned char *ref_ptr,
        int  recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance_halfpixvar16x16_h_neon(
        const unsigned char *src_ptr,
        int  source_stride,
        const unsigned char *ref_ptr,
        int  recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}
