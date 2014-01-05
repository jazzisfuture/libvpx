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
 * vp8_subpixelvariance8x8_neon.asm
 *
 * <james.yu at linaro.org>
 * SEEMS UNUSED, but no effect. I didn't rewrite it to useing NEON intrinsics.
 * I will rewrite it if needed.
 */

#include <arm_neon.h>
#include <stdio.h>

void vp8_push_neon(int64_t *store) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return;
}

void vp8_pop_neon(int64_t *store) {
    // printf("[%s:%d]\n", __func__, __LINE__);
    return;
}
