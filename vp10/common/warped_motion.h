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

#include "./vpx_config.h"
#include "vpx_ports/mem.h"

#define WARPEDMODEL_PREC_BITS    8
#define WARPEDPIXEL_PREC_BITS    6
#define WARPEDPIXEL_PREC_SHIFTS  (1 << WARPEDPIXEL_PREC_BITS)

#define WARPEDPIXEL_FILTER_TAPS  6
#define WARPEDPIXEL_FILTER_BITS  7

#define WARPEDDIFF_PREC_BITS  (WARPEDMODEL_PREC_BITS - WARPEDPIXEL_PREC_BITS)


typedef enum {
  UNKNOWN_TRANSFORM = -1,
  HOMOGRAPHY,      // homography, 8-parameter
  AFFINE,          // affine, 6-parameter
  ROTZOOM,         // simplified affine with rotation and zoom only, 4-parameter
  TRANSLATION      // translational motion 2-parameter
} TransformationType;

typedef struct {
  TransformationType wmtype;
  int wmmat[9];
} WarpedMotionParams;

void vp10_warp_plane(WarpedMotionParams *wm,
                     uint8_t *ref,
                     int width, int height, int stride,
                     uint8_t *pred,
                     int p_col, int p_row,
                     int p_width, int p_height, int p_stride,
                     int subsampling_col, int subsampling_row,
                     int x_scale, int y_scale);

#endif  // VP10_COMMON_WARPED_MOTION_H
