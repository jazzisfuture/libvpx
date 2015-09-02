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
//#include "vp10/common/entropymode.h"

#if CONFIG_SCREEN_CONTENT
/*
int vp10_count_colors(const uint8_t *src, int stride, int rows, int cols);
#if CONFIG_vp9_HIGHBITDEPTH
int vp10_count_colors_highbd(const uint8_t *src8, int stride, int rows,
                             int cols, int bit_depth);
void vp10_palette_color_insertion(uint16_t *old_colors, int *m, int *count,
                                  const MB_MODE_INFO *mbmi);
int vp10_palette_color_lookup(uint16_t *dic, int n, uint16_t val, int bits);
#else
void vp10_palette_color_insertion(uint8_t *old_colors, int *m, int *count,
                                  const MB_MODE_INFO *mbmi);
int vp10_palette_color_lookup(uint8_t *dic, int n, uint8_t val, int bits);
#endif  // CONFIG_vp9_HIGHBITDEPTH
void vp10_insertion_sort(double *data, int n);
int vp10_ceil_log2(int n);
int vp10_k_means(const double *data, double *centroids, int *indices,
                 int n, int k, int dim, int max_itr);
void vp10_calc_indices(const double *data, const double *centroids, int *indices,
                       int n, int k, int dim);
void vp10_update_palette_counts(FRAME_COUNTS *counts, const MB_MODE_INFO *mbmi,
                                BLOCK_SIZE bsize, int palette_ctx);
int vp10_get_palette_color_context(const uint8_t *color_map, int cols,
                                   int r, int c, int n, int *color_order);*/

void vp10_insertion_sort(double *data, int n);
int vp10_count_colors(const uint8_t *src, int stride, int rows, int cols);
void vp10_calc_indices(const double *data, const double *centroids,
                       int *indices, int n, int k, int dim);
int vp10_k_means(const double *data, double *centroids, int *indices,
                 int n, int k, int dim, int max_itr);
#endif  // CONFIG_SCREEN_CONTENT

#endif /* VP10_ENCODER_PALETTE_H_ */
