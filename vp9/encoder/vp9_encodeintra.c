/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "vp9_rtcd.h"
#include "vp9/encoder/vp9_quantize.h"
#include "vp9/common/vp9_reconintra.h"
#include "vp9/encoder/vp9_encodemb.h"
#include "vp9/common/vp9_invtrans.h"
#include "vp9/encoder/vp9_encodeintra.h"

static void encode_intra4x4block(MACROBLOCK *x, int ib, BLOCK_SIZE_TYPE bs);

int vp9_encode_intra(VP9_COMP *cpi, MACROBLOCK *x, int use_16x16_pred) {
  MB_MODE_INFO * mbmi = &x->e_mbd.mode_info_context->mbmi;
  (void) cpi;

  if (use_16x16_pred) {
    mbmi->mode = DC_PRED;
    mbmi->uv_mode = DC_PRED;
    mbmi->ref_frame = INTRA_FRAME;

    vp9_encode_intra16x16mby(&cpi->common, x);
  } else {
    int i;

    for (i = 0; i < 16; i++) {
      x->e_mbd.mode_info_context->bmi[i].as_mode.first = B_DC_PRED;
      encode_intra4x4block(x, i, BLOCK_SIZE_MB16X16);
    }
  }

  return vp9_get_mb_ss(x->plane[0].src_diff);
}

static void encode_intra_block(MACROBLOCK *x, int ib,
                               BLOCK_SIZE_TYPE bsize,
                               TX_SIZE tx_size) {
  MACROBLOCKD * const xd = &x->e_mbd;
  TX_TYPE tx_type;
  const int txfm_b_size = 4 << tx_size;
  const int txfm_b_size_2 = txfm_b_size * txfm_b_size;

  uint8_t* const src =
      raster_txblock_offset_uint8(xd, bsize, tx_size, 0, ib,
                                x->plane[0].src.buf, x->plane[0].src.stride);
  uint8_t* const dst =
      raster_txblock_offset_uint8(xd, bsize, tx_size, 0, ib,
                                xd->plane[0].dst.buf, xd->plane[0].dst.stride);
  int16_t* const src_diff =
      raster_txblock_offset_int16(xd, bsize, tx_size, 0, ib,
                                x->plane[0].src_diff);
  int16_t* const diff =
      raster_txblock_offset_int16(xd, bsize, tx_size, 0, ib,
                                xd->plane[0].diff);
  int16_t* const coeff = BLOCK_OFFSET(x->plane[0].coeff, ib, txfm_b_size_2);
  const int bwl = b_width_log2(bsize), bhl = b_height_log2(bsize);

  assert(ib < (1 << (bwl - tx_size + bhl - tx_size)));

  vp9_predict_intra_block(&x->e_mbd, ib, bsize, tx_size,
                          xd->mode_info_context->bmi[ib].as_mode.first,
                          dst, xd->plane[0].dst.stride);
  vp9_subtract_block(txfm_b_size, txfm_b_size,
                     src_diff, 4 << bwl,
                     src, x->plane[0].src.stride,
                     dst, xd->plane[0].dst.stride);

  if (tx_size <= TX_16X16)
    tx_type = txfm_map(xd->mode_info_context->bmi[ib].as_mode.first);
  else
    tx_type = DCT_DCT;
  switch (tx_size) {
    case TX_4X4:
      if (tx_type != DCT_DCT) {
        vp9_short_fht4x4(src_diff, coeff, 4 << bwl, tx_type);
        x->quantize_b_4x4(x, ib, tx_type, txfm_b_size_2);
        vp9_short_iht4x4(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                         diff, 4 << bwl, tx_type);
      } else {
        x->fwd_txm4x4(src_diff, coeff, 8 << bwl);
        x->quantize_b_4x4(x, ib, tx_type, txfm_b_size_2);
        vp9_inverse_transform_b_4x4(&x->e_mbd, xd->plane[0].eobs[ib],
                                    BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                                    diff, 8 << bwl);
      }
      break;
    case TX_8X8:
      if (tx_type != DCT_DCT) {
        vp9_short_fht8x8(src_diff, coeff, 4 << bwl, tx_type);
        vp9_quantize(x, 0, ib, txfm_b_size_2, tx_type);
        vp9_short_iht8x8(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                         diff, 4 << bwl, tx_type);
      } else {
        x->fwd_txm8x8(src_diff, coeff, 8 << bwl);
        vp9_quantize(x, 0, ib, txfm_b_size_2, tx_type);
        vp9_short_idct8x8(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                          diff, 8 << bwl);
      }
      break;
    case TX_16X16:
      if (tx_type != DCT_DCT) {
        vp9_short_fht16x16(src_diff, coeff, 4 << bwl, tx_type);
        vp9_quantize(x, 0, ib, txfm_b_size_2, tx_type);
        vp9_short_iht16x16(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                           diff, 4 << bwl, tx_type);
      } else {
        x->fwd_txm16x16(src_diff, coeff, 8 << bwl);
        vp9_quantize(x, 0, ib, txfm_b_size_2, tx_type);
        vp9_short_idct16x16(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                            diff, 8 << bwl);
      }
      break;
    case TX_32X32:
      vp9_short_fdct32x32(src_diff, coeff, 8 << bwl);
      vp9_quantize(x, 0, ib, txfm_b_size_2, tx_type);
      vp9_short_idct32x32(BLOCK_OFFSET(xd->plane[0].dqcoeff, ib, txfm_b_size_2),
                          diff, 8 << bwl);
      break;
  }
  vp9_recon_txb_c(txfm_b_size, dst,
                  diff, 4 << bwl,
                  dst, xd->plane[0].dst.stride);

}
static void encode_intra4x4block(MACROBLOCK *x, int ib,
                                 BLOCK_SIZE_TYPE bsize) {
  encode_intra_block(x, ib, bsize, TX_4X4);
}

void vp9_encode_intra4x4mby(MACROBLOCK *mb, BLOCK_SIZE_TYPE bsize) {
  int i;
  int bwl = b_width_log2(bsize), bhl = b_height_log2(bsize);
  int bc = 1 << (bwl + bhl);

  for (i = 0; i < bc; i++)
    encode_intra4x4block(mb, i, bsize);
}

void vp9_encode_intra16x16mby(VP9_COMMON *const cm, MACROBLOCK *x) {
  MACROBLOCKD *xd = &x->e_mbd;

  vp9_build_intra_predictors_sby_s(xd, BLOCK_SIZE_MB16X16);
  vp9_encode_sby(cm, x, BLOCK_SIZE_MB16X16);
}

void vp9_encode_intra16x16mbuv(VP9_COMMON *const cm, MACROBLOCK *x) {
  MACROBLOCKD *xd = &x->e_mbd;

  vp9_build_intra_predictors_sbuv_s(xd, BLOCK_SIZE_MB16X16);
  vp9_encode_sbuv(cm, x, BLOCK_SIZE_MB16X16);
}


void vp9_encode_intra_sb(VP9_COMMON *const cm, MACROBLOCK *x,
                        BLOCK_SIZE_TYPE bsize) {
  int i;
  const MACROBLOCKD *xd = &x->e_mbd;
  const int bwl = b_width_log2(bsize);
  const int bhl = b_height_log2(bsize);
  const TX_SIZE tx_size = xd->mode_info_context->mbmi.txfm_size;
  const int bc = 1 << (bwl - tx_size + bwl - tx_size);

  for (i = 0; i < bc; i++)
    encode_intra_block(x, i, bsize, tx_size);
}

