/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_PALETTE_H_
#define VP9_COMMON_VP9_PALETTE_H_

#include "vp9/common/vp9_blockd.h"

#if CONFIG_PALETTE
void sort_array_double(double *data, int n);
int generate_palette(const uint8_t *src, int stride, int rows, int cols,
                     uint8_t *palette, int *count, uint8_t *map);
int nearest_number(int num, int denom, int l_bound, int r_bound);
void reduce_palette(uint8_t *palette, int *count, int n, uint8_t *map,
                    int rows, int cols);
int run_lengh_encoding(uint8_t *seq, int n, uint16_t *runs, int max_run);
int run_lengh_decoding(uint16_t *runs, int l, uint8_t *seq);
void transpose_block(uint8_t *seq_in, uint8_t *seq_out, int rows, int cols);
/*void palette_color_insersion(uint8_t *old_colors, int *m, uint8_t *new_colors,
                             int n, int *count);*/
void palette_color_insertion(uint8_t *old_colors, int *m, int *count,
                             MB_MODE_INFO *mbmi);
int palette_color_lookup(uint8_t *dic, int n, uint8_t val, int bits);
int palette_max_run(BLOCK_SIZE bsize);
int get_bit_depth(int n);
int k_means(double *data, double *centroids, int *indices,
             int n, int k, int dim, int max_itr);
void calc_indices(double *data, double *centroids, int *indices,
                  int n, int k, int dim);
#endif

#endif  // VP9_COMMON_VP9_PALETTE_H_
