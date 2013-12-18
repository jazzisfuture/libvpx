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
 * variance_neon.asm
 *
 * <james.yu at linaro.org>
 * SEEMS UNUSED. I didn't rewrite it to useing NEON intrinsics.
 * I will rewrite it if needed.
 */

#include <arm_neon.h>
#include <stdio.h>

unsigned int vp8_variance16x16_neon(
        const unsigned char *src_ptr,
        int source_stride,
        const unsigned char *ref_ptr,
        int recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance16x8_neon(
        const unsigned char *src_ptr,
        int source_stride,
        const unsigned char *ref_ptr,
        int recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance8x16_neon(
        const unsigned char *src_ptr,
        int source_stride,
        const unsigned char *ref_ptr,
        int recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}

unsigned int vp8_variance8x8_neon(
        const unsigned char *src_ptr,
        int source_stride,
        const unsigned char *ref_ptr,
        int recon_stride,
        unsigned int *sse) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return 0;
}
