/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp9/common/vp9_blockd.h"

void vp9_build_block_doffsets(MACROBLOCKD *xd) {
  int block;
  BLOCKD *blockd = xd->block;
  int stride;

  stride = xd->dst.y_stride;
  for (block = 0; block < 16; block++) { /* y blocks */
    blockd[block].offset = (block >> 2) * 4 * stride + (block & 3) * 4;
  }

  stride = xd->dst.uv_stride;
  for (block = 16; block < 20; block++) { /* U and V blocks */
    blockd[block].offset = ((block - 16) >> 1) * 4 * stride + (block & 1) * 4;
    blockd[block + 4].offset = blockd[block].offset;
  }
}

void vp9_setup_block_dptrs(MACROBLOCKD *xd) {
  int r, c;
  BLOCKD *blockd = xd->block;

  for (r = 0; r < 4; r++) {
    for (c = 0; c < 4; c++) {
      blockd[r * 4 + c].diff = &xd->diff[r * 4 * 16 + c * 4];
      blockd[r * 4 + c].predictor = xd->predictor + r * 4 * 16 + c * 4;
    }
  }

  for (r = 0; r < 2; r++) {
    for (c = 0; c < 2; c++) {
      blockd[16 + r * 2 + c].diff = &xd->diff[256 + r * 4 * 8 + c * 4];
      blockd[16 + r * 2 + c].predictor =
        xd->predictor + 256 + r * 4 * 8 + c * 4;

    }
  }

  for (r = 0; r < 2; r++) {
    for (c = 0; c < 2; c++) {
      blockd[20 + r * 2 + c].diff = &xd->diff[320 + r * 4 * 8 + c * 4];
      blockd[20 + r * 2 + c].predictor =
        xd->predictor + 320 + r * 4 * 8 + c * 4;

    }
  }

  blockd[24].diff = &xd->diff[384];

  for (r = 0; r < 25; r++) {
    blockd[r].qcoeff  = xd->qcoeff  + r * 16;
    blockd[r].dqcoeff = xd->dqcoeff + r * 16;
  }
}
