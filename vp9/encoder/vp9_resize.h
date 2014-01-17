/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
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

void vp9_resize_plane(uint8_t *input,
                      int height,
                      int width,
                      int in_stride,
                      uint8_t *output,
                      int height2,
                      int width2,
                      int out_stride);
void vp9_resize_frame420(uint8_t *y,
                         int y_stride,
                         uint8_t *u,
                         uint8_t *v,
                         int uv_stride,
                         int height,
                         int width,
                         uint8_t *oy,
                         int oy_stride,
                         uint8_t *ou,
                         uint8_t *ov,
                         int ouv_stride,
                         int oheight,
                         int owidth);
void vp9_resize_frame422(uint8_t *y,
                         int y_stride,
                         uint8_t *u,
                         uint8_t *v,
                         int uv_stride,
                         int height,
                         int width,
                         uint8_t *oy,
                         int oy_stride,
                         uint8_t *ou,
                         uint8_t *ov,
                         int ouv_stride,
                         int oheight,
                         int owidth);
void vp9_resize_frame444(uint8_t *y,
                         int y_stride,
                         uint8_t *u,
                         uint8_t *v,
                         int uv_stride,
                         int height,
                         int width,
                         uint8_t *oy,
                         int oy_stride,
                         uint8_t *ou,
                         uint8_t *ov,
                         int ouv_stride,
                         int oheight,
                         int owidth);

#endif    // VP9_ENCODER_VP9_RESIZE_H_
