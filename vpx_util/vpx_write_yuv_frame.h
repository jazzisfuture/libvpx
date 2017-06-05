/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_UTIL_VPX_WRITE_YUV_FRAME_H_
#define VPX_UTIL_VPX_WRITE_YUV_FRAME_H_

#include <stdio.h>
#include "vpx_dsp/skin_detection.h"
#include "vpx_scale/yv12config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(OUTPUT_YUV_SRC) || defined(OUTPUT_YUV_DENOISED) || \
    defined(OUTPUT_YUV_SKINMAP)

void vpx_write_yuv_frame(FILE *yuv_file, YV12_BUFFER_CONFIG *s);

#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_UTIL_VPX_WRITE_YUV_FRAME_H_
