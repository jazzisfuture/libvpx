/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_COMMON_WARPED_MOTION_H
#define VP10_COMMON_WARPED_MOTION_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#include "vp10/common/mv.h"

#include "./vpx_config.h"
#include "vpx_ports/mem.h"
#include "vpx_dsp/vpx_dsp_common.h"

typedef void (*projectPointsType)(int16_t *mat,
                                  int *points,
                                  int *proj,
                                  const int n,
                                  const int stride_points,
                                  const int stride_proj,
                                  const int subsampling_x,
                                  const int subsampling_y);

void projectPointsHomography(int16_t *mat,
                             int *points,
                             int *proj,
                             const int n,
                             const int stride_points,
                             const int stride_proj,
                             const int subsampling_x,
                             const int subsampling_y);

void projectPointsAffine(int16_t *mat,
                         int *points,
                         int *proj,
                         const int n,
                         const int stride_points,
                         const int stride_proj,
                         const int subsampling_x,
                         const int subsampling_y);

void projectPointsRotZoom(int16_t *mat,
                          int *points,
                          int *proj,
                          const int n,
                          const int stride_points,
                          const int stride_proj,
                          const int subsampling_x,
                          const int subsampling_y);

void projectPointsTranslation(int16_t *mat,
                              int *points,
                              int *proj,
                              const int n,
                              const int stride_points,
                              const int stride_proj,
                              const int subsampling_x,
                              const int subsampling_y);

double warp_erroradv(WarpedMotionParams *wm,
#if CONFIG_VP9_HIGHBITDEPTH
                     int use_hbd, int bd,
#endif  // CONFIG_VP9_HIGHBITDEPTH
                     uint8_t *ref, int width, int height, int stride,
                     uint8_t *dst, int p_col, int p_row, int p_width,
                     int p_height, int p_stride, int subsampling_x,
                     int subsampling_y, int x_scale, int y_scale);

void vp10_warp_plane(WarpedMotionParams *wm,
#if CONFIG_VP9_HIGHBITDEPTH
                     int use_hbd, int bd,
#endif  // CONFIG_VP9_HIGHBITDEPTH
                     uint8_t *ref, int width, int height, int stride,
                     uint8_t *pred, int p_col, int p_row, int p_width,
                     int p_height, int p_stride, int subsampling_x,
                     int subsampling_y, int x_scale, int y_scale);

// Integerize model into the WarpedMotionParams structure
void vp10_integerize_model(const double *model, TransformationType wmtype,
                           WarpedMotionParams *wm);
#endif  // VP10_COMMON_WARPED_MOTION_H
