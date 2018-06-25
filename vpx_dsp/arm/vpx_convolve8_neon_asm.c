/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_dsp_rtcd.h"
#include "vp9/common/vp9_filter.h"
#include "vpx_dsp/arm/vpx_convolve8_neon_asm.h"

void vpx_convolve8_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const InterpKernel *filter, int x0_q4,
                              int x_step_q4, int y0_q4, int y_step_q4, int w,
                              int h) {
  if (0 != x0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_horiz_filter_type1_neon(src, src_stride, dst, dst_stride,
                                          filter, x0_q4, x_step_q4, y0_q4,
                                          y_step_q4, w, h);
  } else {
    vpx_convolve8_horiz_filter_type2_neon(src, src_stride, dst, dst_stride,
                                          filter, x0_q4, x_step_q4, y0_q4,
                                          y_step_q4, w, h);
  }
}

void vpx_convolve8_avg_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                                  uint8_t *dst, ptrdiff_t dst_stride,
                                  const InterpKernel *filter, int x0_q4,
                                  int x_step_q4, int y0_q4, int y_step_q4,
                                  int w, int h) {
  if (0 != x0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_avg_horiz_filter_type1_neon(src, src_stride, dst, dst_stride,
                                              filter, x0_q4, x_step_q4, y0_q4,
                                              y_step_q4, w, h);
  } else {
    vpx_convolve8_avg_horiz_filter_type2_neon(src, src_stride, dst, dst_stride,
                                              filter, x0_q4, x_step_q4, y0_q4,
                                              y_step_q4, w, h);
  }
}

void vpx_convolve8_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                             uint8_t *dst, ptrdiff_t dst_stride,
                             const InterpKernel *filter, int x0_q4,
                             int x_step_q4, int y0_q4, int y_step_q4, int w,
                             int h) {
  if (0 != y0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_vert_filter_type1_neon(src, src_stride, dst, dst_stride,
                                         filter, x0_q4, x_step_q4, y0_q4,
                                         y_step_q4, w, h);
  } else {
    vpx_convolve8_vert_filter_type2_neon(src, src_stride, dst, dst_stride,
                                         filter, x0_q4, x_step_q4, y0_q4,
                                         y_step_q4, w, h);
  }
}

void vpx_convolve8_avg_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                                 uint8_t *dst, ptrdiff_t dst_stride,
                                 const InterpKernel *filter, int x0_q4,
                                 int x_step_q4, int y0_q4, int y_step_q4, int w,
                                 int h) {
  if (0 != y0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_avg_vert_filter_type1_neon(src, src_stride, dst, dst_stride,
                                             filter, x0_q4, x_step_q4, y0_q4,
                                             y_step_q4, w, h);
  } else {
    vpx_convolve8_avg_vert_filter_type2_neon(src, src_stride, dst, dst_stride,
                                             filter, x0_q4, x_step_q4, y0_q4,
                                             y_step_q4, w, h);
  }
}

void vpx_scaled_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const InterpKernel *filter, int x0_q4, int x_step_q4,
                           int y0_q4, int y_step_q4, int w, int h) {
  if (0 != x0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_horiz_filter_type1_neon(src, src_stride, dst, dst_stride,
                                          filter, x0_q4, x_step_q4, y0_q4,
                                          y_step_q4, w, h);
  } else {
    vpx_convolve8_horiz_filter_type2_neon(src, src_stride, dst, dst_stride,
                                          filter, x0_q4, x_step_q4, y0_q4,
                                          y_step_q4, w, h);
  }
}

void vpx_scaled_avg_horiz_neon(const uint8_t *src, ptrdiff_t src_stride,
                               uint8_t *dst, ptrdiff_t dst_stride,
                               const InterpKernel *filter, int x0_q4,
                               int x_step_q4, int y0_q4, int y_step_q4, int w,
                               int h) {
  if (0 != x0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_avg_horiz_filter_type1_neon(src, src_stride, dst, dst_stride,
                                              filter, x0_q4, x_step_q4, y0_q4,
                                              y_step_q4, w, h);
  } else {
    vpx_convolve8_avg_horiz_filter_type2_neon(src, src_stride, dst, dst_stride,
                                              filter, x0_q4, x_step_q4, y0_q4,
                                              y_step_q4, w, h);
  }
}

void vpx_scaled_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                          uint8_t *dst, ptrdiff_t dst_stride,
                          const InterpKernel *filter, int x0_q4, int x_step_q4,
                          int y0_q4, int y_step_q4, int w, int h) {
  if (0 != y0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_vert_filter_type1_neon(src, src_stride, dst, dst_stride,
                                         filter, x0_q4, x_step_q4, y0_q4,
                                         y_step_q4, w, h);
  } else {
    vpx_convolve8_vert_filter_type2_neon(src, src_stride, dst, dst_stride,
                                         filter, x0_q4, x_step_q4, y0_q4,
                                         y_step_q4, w, h);
  }
}

void vpx_scaled_avg_vert_neon(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const InterpKernel *filter, int x0_q4,
                              int x_step_q4, int y0_q4, int y_step_q4, int w,
                              int h) {
  if (0 != y0_q4 && filter == vp9_filter_kernels[1]) {
    vpx_convolve8_avg_vert_filter_type1_neon(src, src_stride, dst, dst_stride,
                                             filter, x0_q4, x_step_q4, y0_q4,
                                             y_step_q4, w, h);
  } else {
    vpx_convolve8_avg_vert_filter_type2_neon(src, src_stride, dst, dst_stride,
                                             filter, x0_q4, x_step_q4, y0_q4,
                                             y_step_q4, w, h);
  }
}
