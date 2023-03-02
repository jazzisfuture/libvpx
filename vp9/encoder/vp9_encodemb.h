/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_ENCODEMB_H_
#define VPX_VP9_ENCODER_VP9_ENCODEMB_H_

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vp9/encoder/vp9_block.h"

#ifdef __cplusplus
extern "C" {
#endif

struct encode_b_args {
  MACROBLOCK *x;
  int enable_coeff_opt;
  ENTROPY_CONTEXT *ta;
  ENTROPY_CONTEXT *tl;
  int8_t *skip;
  int64_t *sse;
  int *sse_calc_done;
  int skip_coeff_opt;
#if CONFIG_MISMATCH_DEBUG
  int mi_row;
  int mi_col;
  int output_enabled;
#endif
};
int vp9_optimize_b(MACROBLOCK *mb, int plane, int block, TX_SIZE tx_size,
                   int ctx);
void vp9_encode_sb(MACROBLOCK *x, BLOCK_SIZE bsize, int mi_row, int mi_col,
                   int output_enabled);
void vp9_encode_sby_pass1(MACROBLOCK *x, BLOCK_SIZE bsize);
void vp9_xform_quant_fp(MACROBLOCK *x, int plane, int block, int row, int col,
                        BLOCK_SIZE plane_bsize, TX_SIZE tx_size);
void vp9_xform_quant_dc(MACROBLOCK *x, int plane, int block, int row, int col,
                        BLOCK_SIZE plane_bsize, TX_SIZE tx_size);
void vp9_xform_quant(MACROBLOCK *x, int plane, int block, int row, int col,
                     BLOCK_SIZE plane_bsize, TX_SIZE tx_size);

void vp9_subtract_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane);

void vp9_encode_block_intra(int plane, int block, int row, int col,
                            BLOCK_SIZE plane_bsize, TX_SIZE tx_size, void *arg);

void vp9_encode_intra_block_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane,
                                  int enable_optimize_b);

static INLINE int num_4x4_to_edge(int plane_4x4_dim, int mb_to_edge_dim,
                                  int subsampling_dim, int blk_dim) {
  return plane_4x4_dim + (mb_to_edge_dim >> (5 + subsampling_dim)) - blk_dim;
}

// Compute the sum of squares on all visible 4x4s in the transform block.
static int64_t sum_squares_visible(const MACROBLOCKD *xd,
                                   const struct macroblockd_plane *const pd,
                                   const int16_t *diff, const int diff_stride,
                                   int blk_row, int blk_col,
                                   const BLOCK_SIZE plane_bsize,
                                   const BLOCK_SIZE tx_bsize,
                                   int *visible_width, int *visible_height) {
  int64_t sse;
  const int plane_4x4_w = num_4x4_blocks_wide_lookup[plane_bsize];
  const int plane_4x4_h = num_4x4_blocks_high_lookup[plane_bsize];
  const int tx_4x4_w = num_4x4_blocks_wide_lookup[tx_bsize];
  const int tx_4x4_h = num_4x4_blocks_high_lookup[tx_bsize];
  int b4x4s_to_right_edge = num_4x4_to_edge(plane_4x4_w, xd->mb_to_right_edge,
                                            pd->subsampling_x, blk_col);
  int b4x4s_to_bottom_edge = num_4x4_to_edge(plane_4x4_h, xd->mb_to_bottom_edge,
                                             pd->subsampling_y, blk_row);
  if (tx_bsize == BLOCK_4X4 ||
      (b4x4s_to_right_edge >= tx_4x4_w && b4x4s_to_bottom_edge >= tx_4x4_h)) {
    assert(tx_4x4_w == tx_4x4_h);
    sse = (int64_t)vpx_sum_squares_2d_i16(diff, diff_stride, tx_4x4_w << 2);
    *visible_width = tx_4x4_w << 2;
    *visible_height = tx_4x4_h << 2;
  } else {
    int r, c;
    int max_r = VPXMIN(b4x4s_to_bottom_edge, tx_4x4_h);
    int max_c = VPXMIN(b4x4s_to_right_edge, tx_4x4_w);
    sse = 0;
    // if we are in the unrestricted motion border.
    for (r = 0; r < max_r; ++r) {
      // Skip visiting the sub blocks that are wholly within the UMV.
      for (c = 0; c < max_c; ++c) {
        sse += (int64_t)vpx_sum_squares_2d_i16(
            diff + r * diff_stride * 4 + c * 4, diff_stride, 4);
      }
    }
    *visible_width = max_c << 2;
    *visible_height = max_r << 2;
  }
  return sse;
}

static INLINE int skip_coeff_optimize(const MACROBLOCKD *xd,
                                      const struct macroblockd_plane *pd,
                                      const int16_t *src_diff, int diff_stride,
                                      int blk_row, int blk_col,
                                      BLOCK_SIZE plane_bsize, TX_SIZE tx_size,
                                      int skip_coeff_opt, int64_t *sse,
                                      int *sse_calc_done) {
  int skip_optimize_b = 0;
  const BLOCK_SIZE tx_bsize = txsize_to_bsize[tx_size];
#if CONFIG_VP9_HIGHBITDEPTH
  const int dequant_shift =
      (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) ? xd->bd - 5 : 3;
#else
  const int dequant_shift = 3;
#endif  // CONFIG_VP9_HIGHBITDEPTH
  const int qstep = pd->dequant[1] >> dequant_shift;
  int visible_width, visible_height;
  const int thresh = skip_coeff_opt + 1;

  if (skip_coeff_opt < 2 || !sse || !sse_calc_done) return 0;

  *sse = sum_squares_visible(xd, pd, src_diff, diff_stride, blk_row, blk_col,
                             plane_bsize, tx_bsize, &visible_width,
                             &visible_height);
  *sse_calc_done = 1;

  skip_optimize_b = (*(sse) > (int64_t)visible_width * visible_height * qstep *
                                  qstep * thresh);
  return skip_optimize_b;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_ENCODEMB_H_
