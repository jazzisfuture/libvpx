/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef VP9_COMMON_CONVOLVE_H_
#define VP9_COMMON_CONVOLVE_H_

#include "vpx/vpx_integer.h"

typedef void (*convolve_fn_t)(const uint8_t *src, int src_stride,
                              uint8_t *dst, int dst_stride,
                              const int16_t *filter_x, int filter_x_stride,
                              const int16_t *filter_y, int filter_y_stride,
                              int w, int h);

// Not a convolution, a block copy conforming to the convolution prototype
void vp9_convolve_copy(const uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride,
                       const int16_t *filter_x, int filter_x_stride,
                       const int16_t *filter_y, int filter_y_stride,
                       int w, int h);

// Not a convolution, a block average conforming to the convolution prototype
void vp9_convolve_avg(const uint8_t *src, int src_stride,
                      uint8_t *dst, int dst_stride,
                      const int16_t *filter_x, int filter_x_stride,
                      const int16_t *filter_y, int filter_y_stride,
                      int w, int h);

struct subpix_fn_table {
  convolve_fn_t predict[2][2][2]; // horiz, vert, avg
  const int16_t (*filter_x)[8];
  const int16_t (*filter_y)[8];
  int filter_x_step;
  int filter_y_step;
};

#endif  // VP9_COMMON_CONVOLVE_H_