/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_COMMON_MOTION_MODEL_H_
#define VP10_COMMON_MOTION_MODEL_H_

int findHomography(const int np, double *pts1, double *pts2, double *mat);
int findAffine(const int np, double *pts1, double *pts2, double *mat);

void projectPointsHomography(double *mat, double *points, double *proj,
                             const int n, const int stride_points,
                             const int stride_proj);
void projectPointsAffine(double *mat, double *points,
                         double *proj, const int n,
                         const int stride_points, const int stride_proj);

void warpImage(double *H,
               unsigned char *ref,
               int width, int height, int stride,
               unsigned char *pred,
               int p_col, int p_row,
               int p_width, int p_height, int p_stride,
               int subsampling_col, int subsampling_row,
               int x_scale, int y_scale);

unsigned char interpolate(unsigned char *ref, double x, double y,
                          int width, int height, int stride);

void warpImage2(int mi_row, int mi_col,
                double *H,
                unsigned char *ref,
                int width, int height, int stride,
                unsigned char *pred,
                int p_col, int p_row,
                int p_width, int p_height, int p_stride,
                int subsampling_col, int subsampling_row,
                int x_scale, int y_scale);

#endif  // VP10_COMMON_MOTION_MODEL_H_
