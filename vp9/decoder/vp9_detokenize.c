/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"

#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_common.h"

#include "vp9/decoder/vp9_detokenize.h"

#define EOB_CONTEXT_NODE            0
#define ZERO_CONTEXT_NODE           1
#define ONE_CONTEXT_NODE            2
#define LOW_VAL_CONTEXT_NODE        0
#define TWO_CONTEXT_NODE            1
#define THREE_CONTEXT_NODE          2
#define HIGH_LOW_CONTEXT_NODE       3
#define CAT_ONE_CONTEXT_NODE        4
#define CAT_THREEFOUR_CONTEXT_NODE  5
#define CAT_THREE_CONTEXT_NODE      6
#define CAT_FIVE_CONTEXT_NODE       7

#define CAT1_MIN_VAL    5
#define CAT2_MIN_VAL    7
#define CAT3_MIN_VAL   11
#define CAT4_MIN_VAL   19
#define CAT5_MIN_VAL   35
#define CAT6_MIN_VAL   67

#define CAT1_PROB0    159
static const vp9_prob cat1_probs[] = { CAT1_PROB0 };

#define CAT2_PROB0    145
#define CAT2_PROB1    165
static const vp9_prob cat2_probs[] = { CAT2_PROB1, CAT2_PROB0 };

#define CAT3_PROB0 140
#define CAT3_PROB1 148
#define CAT3_PROB2 173
static const vp9_prob cat3_probs[] = { CAT3_PROB2, CAT3_PROB1, CAT3_PROB0 };

#define CAT4_PROB0 135
#define CAT4_PROB1 140
#define CAT4_PROB2 155
#define CAT4_PROB3 176
static const vp9_prob cat4_probs[] = { CAT4_PROB3, CAT4_PROB2, CAT4_PROB1,
                                       CAT4_PROB0 };

#define CAT5_PROB0 130
#define CAT5_PROB1 134
#define CAT5_PROB2 141
#define CAT5_PROB3 157
#define CAT5_PROB4 180
static const vp9_prob cat5_probs[] = { CAT5_PROB4, CAT5_PROB3, CAT5_PROB2,
                                       CAT5_PROB1, CAT5_PROB0 };

static const vp9_prob cat6_probs[15] = {
  254, 254, 254, 252, 249, 243, 230, 196, 177, 153, 140, 133, 130, 129, 0
};

#define INCREMENT_COUNT(token)                              \
  do {                                                      \
     if (!cm->frame_parallel_decoding_mode)                 \
       ++coef_counts[band][ctx][token];                      \
  } while (0)

static INLINE int read_coeff(const vp9_prob *probs, int n, vp9_reader *r) {
  int i, val = 0;
  for (i = 0; i < n; ++i)
    val = (val << 1) | vp9_read(r, probs[i]);
  return val;
}

const vp9_tree_index tree[TREE_SIZE(ENTROPY_TOKENS)] = {
  2, 6,                                         /* 0 = LOW_VAL */
  -TWO_TOKEN, 4,                                /* 1 = TWO */
  -THREE_TOKEN, -FOUR_TOKEN,                    /* 2 = THREE */
  8, 10,                                        /* 3 = HIGH_LOW */
  -CATEGORY1_TOKEN, -CATEGORY2_TOKEN,           /* 4 = CAT_ONE */
  12, 14,                                       /* 5 = CAT_THREEFOUR */
  -CATEGORY3_TOKEN, -CATEGORY4_TOKEN,           /* 6 = CAT_THREE */
  -CATEGORY5_TOKEN, -CATEGORY6_TOKEN            /* 7 = CAT_FIVE */
};

static int decode_coefs(VP9_COMMON *cm, const MACROBLOCKD *xd, PLANE_TYPE type,
                        int16_t *dqcoeff, TX_SIZE tx_size, const int16_t *dq,
                        int ctx, const int16_t *scan, const int16_t *nb,
                        vp9_reader *r) {
  const int max_eob = 16 << (tx_size << 1);
  const FRAME_CONTEXT *const fc = &cm->fc;
  FRAME_COUNTS *const counts = &cm->counts;
  const int ref = is_inter_block(&xd->mi[0]->mbmi);
  int band, c = 0;
  const vp9_prob (*coef_probs)[COEFF_CONTEXTS][UNCONSTRAINED_NODES] =
      fc->coef_probs[tx_size][type][ref];
  const vp9_prob *prob;
  unsigned int (*coef_counts)[COEFF_CONTEXTS][UNCONSTRAINED_NODES + 1] =
      counts->coef[tx_size][type][ref];
  unsigned int (*eob_branch_count)[COEFF_CONTEXTS] =
      counts->eob_branch[tx_size][type][ref];
  uint8_t token_cache[32 * 32];
  const uint8_t *band_translate = get_band_translate(tx_size);
  const int dq_shift = (tx_size == TX_32X32);
  int v, token;
  int16_t dqv = dq[0];

  while (c < max_eob) {
    int val = -1;
    band = *band_translate++;
    prob = coef_probs[band][ctx];
    if (!cm->frame_parallel_decoding_mode)
      ++eob_branch_count[band][ctx];
    if (!vp9_read(r, prob[EOB_CONTEXT_NODE])) {
      INCREMENT_COUNT(EOB_MODEL_TOKEN);
      break;
    }

    while (!vp9_read(r, prob[ZERO_CONTEXT_NODE])) {
      INCREMENT_COUNT(ZERO_TOKEN);
      dqv = dq[1];
      token_cache[scan[c]] = 0;
      ++c;
      if (c >= max_eob)
        return c;  // zero tokens at the end (no eob token)
      ctx = get_coef_context(nb, token_cache, c);
      band = *band_translate++;
      prob = coef_probs[band][ctx];
    }

    if (!vp9_read(r, prob[ONE_CONTEXT_NODE])) {
      INCREMENT_COUNT(ONE_TOKEN);
      token = ONE_TOKEN;
      val = 1;
    } else {
      INCREMENT_COUNT(TWO_TOKEN);
      token = vp9_read_tree(r, tree, vp9_pareto8_full[prob[PIVOT_NODE] - 1]);
      switch (token) {
        case TWO_TOKEN:
        case THREE_TOKEN:
        case FOUR_TOKEN:
          val = token;
          break;
        case CATEGORY1_TOKEN:
          val = CAT1_MIN_VAL + read_coeff(cat1_probs, 1, r);
          break;
        case CATEGORY2_TOKEN:
          val = CAT2_MIN_VAL + read_coeff(cat2_probs, 2, r);
          break;
        case CATEGORY3_TOKEN:
          val = CAT3_MIN_VAL + read_coeff(cat3_probs, 3, r);
          break;
        case CATEGORY4_TOKEN:
          val = CAT4_MIN_VAL + read_coeff(cat4_probs, 4, r);
          break;
        case CATEGORY5_TOKEN:
          val = CAT5_MIN_VAL + read_coeff(cat5_probs, 5, r);
          break;
        case CATEGORY6_TOKEN:
          val = CAT6_MIN_VAL + read_coeff(cat6_probs, 14, r);
          break;
      }
    }
    v = (val * dqv) >> dq_shift;
    dqcoeff[scan[c]] = vp9_read_bit(r) ? -v : v;
    token_cache[scan[c]] = vp9_pt_energy_class[token];
    ++c;
    ctx = get_coef_context(nb, token_cache, c);
    dqv = dq[1];
  }

  return c;
}

int vp9_decode_block_tokens(VP9_COMMON *cm, MACROBLOCKD *xd,
                            int plane, int block, BLOCK_SIZE plane_bsize,
                            int x, int y, TX_SIZE tx_size, vp9_reader *r) {
  struct macroblockd_plane *const pd = &xd->plane[plane];
  const int ctx = get_entropy_context(tx_size, pd->above_context + x,
                                               pd->left_context + y);
  const scan_order *so = get_scan(xd, tx_size, pd->plane_type, block);
  const int eob = decode_coefs(cm, xd, pd->plane_type,
                               BLOCK_OFFSET(pd->dqcoeff, block), tx_size,
                               pd->dequant, ctx, so->scan, so->neighbors, r);
  vp9_set_contexts(xd, pd, plane_bsize, tx_size, eob > 0, x, y);
  return eob;
}


