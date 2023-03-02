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
#include "vp9/encoder/vp9_block.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DISABLE_TRELLIS_OPT,  // Disable trellis optimization
  ENABLE_TRELLIS_OPT,   // Enable Trellis optimization
  TX_RD_TRELLIS_OPT,    // Trellis optimization in transform RD
} TRELLIS_OPT_TYPE;

struct encode_b_args {
  const struct VP9_COMP *cpi;
  MACROBLOCK *x;
  TRELLIS_OPT_TYPE trellis_opt_type;
  ENTROPY_CONTEXT *ta;
  ENTROPY_CONTEXT *tl;
  int8_t *skip;
#if CONFIG_MISMATCH_DEBUG
  int mi_row;
  int mi_col;
  int output_enabled;
#endif
};
int vp9_optimize_b(MACROBLOCK *mb, int plane, int block, TX_SIZE tx_size,
                   int ctx);
void vp9_encode_sb(const struct VP9_COMP *cpi, MACROBLOCK *x, BLOCK_SIZE bsize,
                   int mi_row, int mi_col, int output_enabled);
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

void vp9_encode_intra_block_plane(const struct VP9_COMP *cpi, MACROBLOCK *x,
                                  BLOCK_SIZE bsize, int plane,
                                  int trellis_opt_type);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_VP9_ENCODER_VP9_ENCODEMB_H_
