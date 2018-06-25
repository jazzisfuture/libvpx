/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_ARM_VPX_CONVOLVE8_NEON_ASM_H_
#define VPX_DSP_ARM_VPX_CONVOLVE8_NEON_ASM_H_

void vpx_convolve8_horiz_filter_type1_neon(const uint8_t *src,
                                           ptrdiff_t src_stride, uint8_t *dst,
                                           ptrdiff_t dst_stride,
                                           const InterpKernel *filter,
                                           int x0_q4, int x_step_q4, int y0_q4,
                                           int y_step_q4, int w, int h);
void vpx_convolve8_horiz_filter_type2_neon(const uint8_t *src,
                                           ptrdiff_t src_stride, uint8_t *dst,
                                           ptrdiff_t dst_stride,
                                           const InterpKernel *filter,
                                           int x0_q4, int x_step_q4, int y0_q4,
                                           int y_step_q4, int w, int h);
void vpx_convolve8_vert_filter_type1_neon(const uint8_t *src,
                                          ptrdiff_t src_stride, uint8_t *dst,
                                          ptrdiff_t dst_stride,
                                          const InterpKernel *filter, int x0_q4,
                                          int x_step_q4, int y0_q4,
                                          int y_step_q4, int w, int h);
void vpx_convolve8_vert_filter_type2_neon(const uint8_t *src,
                                          ptrdiff_t src_stride, uint8_t *dst,
                                          ptrdiff_t dst_stride,
                                          const InterpKernel *filter, int x0_q4,
                                          int x_step_q4, int y0_q4,
                                          int y_step_q4, int w, int h);
void vpx_convolve8_avg_horiz_filter_type1_neon(
    const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst,
    ptrdiff_t dst_stride, const InterpKernel *filter, int x0_q4, int x_step_q4,
    int y0_q4, int y_step_q4, int w, int h);
void vpx_convolve8_avg_horiz_filter_type2_neon(
    const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst,
    ptrdiff_t dst_stride, const InterpKernel *filter, int x0_q4, int x_step_q4,
    int y0_q4, int y_step_q4, int w, int h);
void vpx_convolve8_avg_vert_filter_type1_neon(
    const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst,
    ptrdiff_t dst_stride, const InterpKernel *filter, int x0_q4, int x_step_q4,
    int y0_q4, int y_step_q4, int w, int h);
void vpx_convolve8_avg_vert_filter_type2_neon(
    const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst,
    ptrdiff_t dst_stride, const InterpKernel *filter, int x0_q4, int x_step_q4,
    int y0_q4, int y_step_q4, int w, int h);

#endif /* VPX_DSP_ARM_VPX_CONVOLVE8_NEON_ASM_H_ */
