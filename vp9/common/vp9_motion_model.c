/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be
 *  found  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#include "vp9/common/vp9_motion_model.h"

INLINE projectPointsType get_projectPointsType(TransformationType type) {
  switch (type) {
    case HOMOGRAPHY:
      return projectPointsHomography;
    case AFFINE:
      return projectPointsAffine;
    case ROTZOOM:
      return projectPointsRotZoom;
    case TRANSLATION:
      return projectPointsTranslation;
    default:
      assert(0);
      return NULL;
  }
}

void projectPointsTranslation(double *mat, double *points, double *proj,
                              const int n,
                              const int stride_points,
                              const int stride_proj) {
  int i;
  for (i = 0; i < n; ++i) {
    const double x = *(points++), y = *(points++);
    *(proj++) = x + mat[0];
    *(proj++) = y + mat[1];
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

void projectPointsRotZoom(double *mat, double *points,
                          double *proj, const int n,
                          const int stride_points, const int stride_proj) {
  int i;
  for (i = 0; i < n; ++i) {
    const double x = *(points++), y = *(points++);
    *(proj++) =  mat[0] * x + mat[1] * y + mat[2];
    *(proj++) = -mat[1] * x + mat[0] * y + mat[3];
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

void projectPointsAffine(double *mat, double *points,
                         double *proj, const int n,
                         const int stride_points, const int stride_proj) {
  int i;
  for (i = 0; i < n; ++i) {
    const double x = *(points++), y = *(points++);
    *(proj++) = mat[0] * x + mat[1] * y + mat[4];
    *(proj++) = mat[2] * x + mat[3] * y + mat[5];
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

void projectPointsHomography(double *mat, double *points,
                             double *proj, const int n,
                             const int stride_points, const int stride_proj) {
  int i;
  double x, y, Z;
  for (i = 0; i < n; ++i) {
    x = *(points++), y = *(points++);
    Z = 1. / (mat[6] * x + mat[7] * y + mat[8]);
    *(proj++) = (mat[0] * x + mat[1] * y + mat[2]) * Z;
    *(proj++) = (mat[3] * x + mat[4] * y + mat[5]) * Z;
    points += stride_points - 2;
    proj += stride_proj - 2;
  }
}

#define clip_pixel(v) ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

static unsigned char bilinear(unsigned char *ref, double x, double y,
                              int width, int height, int stride) {
  if (x < 0 && y < 0) return ref[0];
  else if (x < 0 && y > height - 1)
    return ref[(height - 1) * stride];
  else if (x > width - 1 && y < 0)
    return ref[width - 1];
  else if (x > width - 1 && y > height - 1)
    return ref[(height - 1) * stride + (width - 1)];
  else if (x < 0) {
    int i = (int) y;
    double a = y - i;
    int v = (int)(ref[i * stride] * (1 - a) + ref[(i + 1) * stride] * a + 0.5);
    return clip_pixel(v);
  } else if (y < 0) {
    int j = (int) x;
    double b = x - j;
    int v = (int)(ref[j] * (1 - b) + ref[j + 1] * b + 0.5);
    return clip_pixel(v);
  } else if (x > width - 1) {
    int i = (int) y;
    double a = y - i;
    int v = (int)(ref[i * stride + width - 1] * (1 - a) +
                  ref[(i + 1) * stride + width - 1] * a + 0.5);
    return clip_pixel(v);
  } else if (y > height - 1) {
    int j = (int) x;
    double b = x - j;
    int v = (int)(ref[(height - 1) * stride + j] * (1 - b) +
                  ref[(height - 1) * stride + j + 1] * b + 0.5);
    return clip_pixel(v);
  } else {
    int i = (int) y;
    int j = (int) x;
    double a = y - i;
    double b = x - j;
    int v = (int)(ref[i * stride + j] * (1 - a) * (1 - b) +
                  ref[i * stride + j + 1] * (1 - a) * b +
                  ref[(i + 1) * stride + j] * a * (1 - b) +
                  ref[(i + 1) * stride + j + 1] * a * b);
    return clip_pixel(v);
  }
}

void WarpImage(TransformationType type, double *H,
               unsigned char *ref,
               int width, int height, int stride,
               unsigned char *pred,
               int p_col, int p_row,
               int p_width, int p_height,
               int subsampling_col, int subsampling_row,
               int p_stride) {
  int i, j;
  projectPointsType projectPoints = get_projectPointsType(type);
  if (projectPoints == NULL)
    return;
  for (i = p_row; i < p_row + p_height; ++i) {
    for (j = p_col; j < p_col + p_width; ++j) {
      double in[2], out[2];
      in[0] = subsampling_col ? 2 * j + 0.5 : j;
      in[1] = subsampling_row ? 2 * i + 0.5 : i;
      projectPoints(H, in, out, 1, 2, 2);
      out[0] = subsampling_col ? (out[0] - 0.5) / 2.0 : out[0];
      out[1] = subsampling_row ? (out[1] - 0.5) / 2.0 : out[1];
      pred[j + i * p_stride] =
          bilinear(ref, out[0], out[1], width, height, stride);
    }
  }
}
