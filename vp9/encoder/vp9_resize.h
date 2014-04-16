/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_RESIZE_H_
#define VP9_ENCODER_VP9_RESIZE_H_

#include <stdio.h>
#include "vpx/vpx_integer.h"

#if CONFIG_B10_EXT
void vp9_resize_plane(const uint16_t *const input,
#else
void vp9_resize_plane(const uint8_t *const input,
#endif
                      int height,
                      int width,
                      int in_stride,
#if CONFIG_B10_EXT
                      uint16_t *output,
#else
                      uint8_t *output,
#endif
                      int height2,
                      int width2,
                      int out_stride);
#if CONFIG_B10_EXT
void vp9_resize_frame420(const uint16_t *const y,
#else
void vp9_resize_frame420(const uint8_t *const y,
#endif
                         int y_stride,
#if CONFIG_B10_EXT
                         const uint16_t *const u,
                         const uint16_t *const v,
#else
                         const uint8_t *const u,
                         const uint8_t *const v,
#endif
                         int uv_stride,
                         int height,
                         int width,
#if CONFIG_B10_EXT
                         uint16_t *oy,
#else
                         uint8_t *oy,
#endif
                         int oy_stride,
#if CONFIG_B10_EXT
                         uint16_t *ou,
                         uint16_t *ov,
#else
                         uint8_t *ou,
                         uint8_t *ov,
#endif
                         int ouv_stride,
                         int oheight,
                         int owidth);
#if CONFIG_B10_EXT
void vp9_resize_frame422(const uint16_t *const y,
#else
void vp9_resize_frame422(const uint8_t *const y,
#endif
                         int y_stride,
#if CONFIG_B10_EXT
                         const uint16_t *const u,
                         const uint16_t *const v,
#else
                         const uint8_t *const u,
                         const uint8_t *const v,
#endif
                         int uv_stride,
                         int height,
                         int width,
#if CONFIG_B10_EXT
                         uint16_t *oy,
#else
                         uint8_t *oy,
#endif
                         int oy_stride,
#if CONFIG_B10_EXT
                         uint16_t *ou,
                         uint16_t *ov,
#else
                         uint8_t *ou,
                         uint8_t *ov,
#endif
                         int ouv_stride,
                         int oheight,
                         int owidth);
#if CONFIG_B10_EXT
void vp9_resize_frame444(const uint16_t *const y,
#else
void vp9_resize_frame444(const uint8_t *const y,
#endif
                         int y_stride,
#if CONFIG_B10_EXT
                         const uint16_t *const u,
                         const uint16_t *const v,
#else
                         const uint8_t *const u,
                         const uint8_t *const v,
#endif
                         int uv_stride,
                         int height,
                         int width,
#if CONFIG_B10_EXT
                         uint16_t *oy,
#else
                         uint8_t *oy,
#endif
                         int oy_stride,
#if CONFIG_B10_EXT
                         uint16_t *ou,
                         uint16_t *ov,
#else
                         uint8_t *ou,
                         uint8_t *ov,
#endif
                         int ouv_stride,
                         int oheight,
                         int owidth);

#endif    // VP9_ENCODER_VP9_RESIZE_H_
