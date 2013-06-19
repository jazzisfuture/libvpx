/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_RECONINTRA_H_
#define VP9_COMMON_VP9_RECONINTRA_H_

#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_blockd.h"

void vp9_build_intra_predictors(uint8_t *src, int src_stride,
                                uint8_t *ypred_ptr,
                                int y_stride, int mode,
                                int bw, int bh,
                                int up_available, int left_available,
                                int right_available);

void vp9_predict_intra_block(MACROBLOCKD *xd,
                            int block_idx,
                            int bwl_in,
                            TX_SIZE tx_size,
                            int mode,
                            uint8_t *predictor, int pre_stride);

void vp9_intra4x4_predict(MACROBLOCKD *xd,
                          int block_idx,
                          BLOCK_SIZE_TYPE bsize,
                          int mode,
                          uint8_t *predictor, int pre_stride);

#endif  // VP9_COMMON_VP9_RECONINTRA_H_
