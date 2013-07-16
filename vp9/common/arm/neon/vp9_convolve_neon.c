/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vp9_rtcd.h"
#include "vp9/common/vp9_common.h"

void vp9_convolve8_neon(const uint8_t *src, ptrdiff_t src_stride,
                        uint8_t *dst, ptrdiff_t dst_stride,
                        const int16_t *filter_x, int x_step_q4,
                        const int16_t *filter_y, int y_step_q4,
                        int w, int h) {
  uint8_t temp[64 * 135];
  int intermediate_height = MAX(((h * y_step_q4) >> 4), 1) + 8 - 1;

  if (intermediate_height < h)
    intermediate_height = h;

  vp9_convolve8_horiz_neon(src - src_stride * (8 / 2 - 1), src_stride,
                           temp, 64,
                           filter_x, x_step_q4, filter_y, y_step_q4,
                           w, intermediate_height);
  vp9_convolve8_vert_neon(temp + 64 * (8 / 2 - 1), 64, dst, dst_stride,
                          filter_x, x_step_q4, filter_y, y_step_q4,
                          w, h);
}

void vp9_convolve8_avg_neon(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h) {
  uint8_t temp[64 * 135];
  int intermediate_height = MAX(((h * y_step_q4) >> 4), 1) + 8 - 1;

  if (intermediate_height < h)
    intermediate_height = h;

  vp9_convolve8_horiz_neon(src - src_stride * (8 / 2 - 1), src_stride,
                           temp, 64,
                           filter_x, x_step_q4, filter_y, y_step_q4,
                           w, intermediate_height);
  vp9_convolve8_avg_vert_neon(temp + 64 * (8 / 2 - 1), 64, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h);
}
