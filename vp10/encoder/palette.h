/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_ENCODER_PALETTE_H_
#define VP10_ENCODER_PALETTE_H_

#include "vp10/common/blockd.h"

#if CONFIG_PALETTE
void vp10_insertion_sort(double *data, int n);
void vp10_calc_indices(const double *data, const double *centroids,
                       int *indices, int n, int k, int dim);
int vp10_k_means(const double *data, double *centroids, int *indices,
                 int n, int k, int dim, int max_itr);
int vp10_count_colors(const uint8_t *src, int stride, int rows, int cols);
#if CONFIG_VP9_HIGHBITDEPTH
int vp10_count_colors_highbd(const uint8_t *src8, int stride, int rows,
                             int cols, int bit_depth);
#endif  // CONFIG_vp9_HIGHBITDEPTH
#endif  // CONFIG_PALETTE

#endif /* VP10_ENCODER_PALETTE_H_ */
