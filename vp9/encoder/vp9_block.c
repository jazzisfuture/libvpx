/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/encoder/vp9_block.h"

// TODO(jingning): the variables used here are little complicated. need further
// refactoring on organizing the temporary buffers, when recursive
// partition down to 4x4 block size is enabled.
PICK_MODE_CONTEXT *vp9_get_block_context(MACROBLOCK *x, BLOCK_SIZE bsize) {
  switch (bsize) {
    case BLOCK_64X64:
      return &x->sb64_context;
    case BLOCK_64X32:
      return &x->sb64x32_context[x->sb_index];
    case BLOCK_32X64:
      return &x->sb32x64_context[x->sb_index];
    case BLOCK_32X32:
      return &x->sb32_context[x->sb_index];
    case BLOCK_32X16:
      return &x->sb32x16_context[x->sb_index][x->mb_index];
    case BLOCK_16X32:
      return &x->sb16x32_context[x->sb_index][x->mb_index];
    case BLOCK_16X16:
      return &x->mb_context[x->sb_index][x->mb_index];
    case BLOCK_16X8:
      return &x->sb16x8_context[x->sb_index][x->mb_index][x->b_index];
    case BLOCK_8X16:
      return &x->sb8x16_context[x->sb_index][x->mb_index][x->b_index];
    case BLOCK_8X8:
      return &x->sb8x8_context[x->sb_index][x->mb_index][x->b_index];
    case BLOCK_8X4:
      return &x->sb8x4_context[x->sb_index][x->mb_index][x->b_index];
    case BLOCK_4X8:
      return &x->sb4x8_context[x->sb_index][x->mb_index][x->b_index];
    case BLOCK_4X4:
      return &x->ab4x4_context[x->sb_index][x->mb_index][x->b_index];
    default:
      assert(0);
      return NULL;
  }
}
