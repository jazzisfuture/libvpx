/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp9/encoder/vp9_context_tree.h"

const BLOCK_SIZE square[] = {
    BLOCK_8X8,
    BLOCK_16X16,
    BLOCK_32X32,
    BLOCK_64X64,
};

static void alloc_mode_context(VP9_COMMON *cm, int num_4x4_blk,
                               PICK_MODE_CONTEXT *ctx) {
  const int num_blk = (num_4x4_blk < 4 ? 4 : num_4x4_blk);
  const int num_pix = num_blk << 4;
  int i, k;
  ctx->num_4x4_blk = num_blk;

  CHECK_MEM_ERROR(cm, ctx->zcoeff_blk,
                  vpx_calloc(num_4x4_blk, sizeof(uint8_t)));
  for (i = 0; i < MAX_MB_PLANE; ++i) {
    for (k = 0; k < 3; ++k) {
      CHECK_MEM_ERROR(cm, ctx->coeff[i][k],
                      vpx_memalign(16, num_pix * sizeof(int16_t)));
      CHECK_MEM_ERROR(cm, ctx->qcoeff[i][k],
                      vpx_memalign(16, num_pix * sizeof(int16_t)));
      CHECK_MEM_ERROR(cm, ctx->dqcoeff[i][k],
                      vpx_memalign(16, num_pix * sizeof(int16_t)));
      CHECK_MEM_ERROR(cm, ctx->eobs[i][k],
                      vpx_memalign(16, num_pix * sizeof(uint16_t)));
      ctx->coeff_pbuf[i][k]   = ctx->coeff[i][k];
      ctx->qcoeff_pbuf[i][k]  = ctx->qcoeff[i][k];
      ctx->dqcoeff_pbuf[i][k] = ctx->dqcoeff[i][k];
      ctx->eobs_pbuf[i][k]    = ctx->eobs[i][k];
    }
  }
}

static void free_mode_context(PICK_MODE_CONTEXT *ctx) {
  int i, k;
  vpx_free(ctx->zcoeff_blk);
  ctx->zcoeff_blk = 0;
  for (i = 0; i < MAX_MB_PLANE; ++i) {
    for (k = 0; k < 3; ++k) {
      vpx_free(ctx->coeff[i][k]);
      ctx->coeff[i][k] = 0;
      vpx_free(ctx->qcoeff[i][k]);
      ctx->qcoeff[i][k] = 0;
      vpx_free(ctx->dqcoeff[i][k]);
      ctx->dqcoeff[i][k] = 0;
      vpx_free(ctx->eobs[i][k]);
      ctx->eobs[i][k] = 0;
    }
  }
}
static void free_tree_contexts(PC_TREE *this_pc) {
  free_mode_context(&this_pc->none);
  free_mode_context(&this_pc->horizontal[0]);
  free_mode_context(&this_pc->horizontal[1]);
  free_mode_context(&this_pc->vertical[0]);
  free_mode_context(&this_pc->vertical[1]);
}
static void alloc_tree_contexts(VP9_COMMON *cm, PC_TREE *this_pc,
                                int num_4x4_blk) {
  alloc_mode_context(cm, num_4x4_blk, &this_pc->none);
  alloc_mode_context(cm, num_4x4_blk/2, &this_pc->horizontal[0]);
  alloc_mode_context(cm, num_4x4_blk/2, &this_pc->horizontal[1]);
  alloc_mode_context(cm, num_4x4_blk/2, &this_pc->vertical[0]);
  alloc_mode_context(cm, num_4x4_blk/2, &this_pc->vertical[1]);
}


// This function sets up a tree of contexts such that at each square
// partition level. There are contexts for none, horizontal, vertical, and
// split.  Along with a block_size value and a selected block_size which
// represents the state of our search...
void vp9_setup_pc_tree(VP9_COMP *cpi) {
  int i,j;
  const int leaf_nodes = 64;
  const int tree_nodes = 64 + 16 + 4 + 1;
  int pc_tree_index = 0;
  PC_TREE *this_pc;
  PICK_MODE_CONTEXT *this_leaf;
  int square_index = 1;
  int nodes;
  VP9_COMMON *cm=&cpi->common;
  MACROBLOCK *m=&cpi->mb;

  vpx_free(m->leaf_tree);
  CHECK_MEM_ERROR(cm, m->leaf_tree, vpx_calloc(leaf_nodes * 4,
                                               sizeof(PICK_MODE_CONTEXT)));
  vpx_free(m->pc_tree);
  CHECK_MEM_ERROR(cm, m->pc_tree, vpx_calloc(tree_nodes, sizeof(PC_TREE)));

  this_pc = &m->pc_tree[0];
  this_leaf = &m->leaf_tree[0];

  // Set up all 4x4 mode contexts
  for (i = 0; i < leaf_nodes * 4; ++i)
    alloc_mode_context(cm, 1, &m->leaf_tree[i]);

  // Sets up all the leaf nodes in the tree.
  for (pc_tree_index = 0; pc_tree_index < leaf_nodes; ++pc_tree_index) {
    m->pc_tree[pc_tree_index].block_size = square[0];
    alloc_tree_contexts(cm, &m->pc_tree[pc_tree_index], 4);
    for (j = 0; j < 4; j++) {
      m->pc_tree[pc_tree_index].leaf_split[j] = this_leaf++;
    }
  }

  // Each node has 4 leaf nodes, fill each block_size level of the tree
  // from leafs to the root.
  for (nodes = 16; nodes > 0; nodes >>= 2, ++square_index) {
    for (i = 0; i < nodes; ++pc_tree_index,  ++i) {
      alloc_tree_contexts(cm, &m->pc_tree[pc_tree_index],
                          4 << (2 * square_index));
      m->pc_tree[pc_tree_index].block_size = square[square_index];
      for (j = 0; j < 4; j++) {
        m->pc_tree[pc_tree_index].split[j] = this_pc++;
      }
    }
  }
  m->pc_root = &m->pc_tree[tree_nodes-1];
  m->pc_root[0].none.best_mode_index = 2;
}

void vp9_free_pc_tree(VP9_COMP *cpi) {
  MACROBLOCK *m=&cpi->mb;
  const int tree_nodes = 64 + 16 + 4 + 1;
  int i;

  // Set up all 4x4 mode contexts
  for (i = 0; i < 64 * 4; ++i)
    free_mode_context(&m->leaf_tree[i]);

  // Sets up all the leaf nodes in the tree.
  for (i = 0; i < tree_nodes; i++) {
    free_tree_contexts(&m->pc_tree[i]);
  }
  vpx_free(m->pc_tree);
  m->pc_tree = 0;
  vpx_free(m->leaf_tree);
  m->leaf_tree = 0;
}
