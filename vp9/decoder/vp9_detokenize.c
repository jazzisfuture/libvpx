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
#include "vp9/common/vp9_seg_common.h"

#include "vp9/decoder/vp9_dboolhuff.h"
#include "vp9/decoder/vp9_detokenize.h"
#include "vp9/decoder/vp9_onyxd_int.h"

#include "vp9/encoder/vp9_boolhuff.h"

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
#define CAT2_PROB0    145
#define CAT2_PROB1    165

#define CAT3_PROB0 140
#define CAT3_PROB1 148
#define CAT3_PROB2 173

#define CAT4_PROB0 135
#define CAT4_PROB1 140
#define CAT4_PROB2 155
#define CAT4_PROB3 176

#define CAT5_PROB0 130
#define CAT5_PROB1 134
#define CAT5_PROB2 141
#define CAT5_PROB3 157
#define CAT5_PROB4 180


typedef struct {
  uint8_t range;
  uint8_t count;
} bool_coder_state;

typedef struct {
  uint32_t lowvalue[3];
  bool_coder_state state[4];
} bw_state;

bw_state full_set[TX_SIZES][BLOCK_TYPES][REF_TYPES][COEF_BANDS]
                 [PREV_COEF_CONTEXTS][2][128];
static INLINE void set_state(bool_coder_state *fs, int range, int count) {
  fs->range = range;
  fs->count = count;
}

static INLINE void take_right(const unsigned int split, unsigned int *lowvalue,
                         int *range, int *count) {
  unsigned int shift;
  *lowvalue += split;
  *range -= split;
  shift = vp9_norm[*range];
  *range <<= shift;
  *count += shift;
  *lowvalue <<= shift;
}
static INLINE void one_bit(const int prob, bool_coder_state *bcs,
                           unsigned int *lowvalue,
                         int *range, int *count) {
  unsigned int split = 1 + (((*range - 1) * prob) >> 8);
  unsigned int shift = vp9_norm[split];
  set_state(bcs, split << shift, *count + shift);
  take_right(split, lowvalue, range, count);
}

#define BD_VALUEIZE(V, C) ((BD_VALUE)(V)) << (32 - 8 - (C));
static INLINE void node_one_bit(const int prob, bw_state *fs, int node,
                                unsigned int *lowvalue, int *range,
                                int *count) {
  unsigned int split = 1 + (((*range - 1) * prob) >> 8);
  unsigned int shift = vp9_norm[split];
  set_state(&fs->state[node+1], split << shift, *count + shift);
  fs->lowvalue[node] = BD_VALUEIZE((*lowvalue) << shift,
                                   fs->state[node+1].count);
  take_right(split, lowvalue, range, count);
}

static INLINE bw_state * fill_table(PLANE_TYPE type, int is_inter, int band,
                      TX_SIZE tx_size, int context, int range,
                      int last_is_zero,
                      int prob1, int prob2, int prob3) {
  int count = 0;
  unsigned int lowvalue = 0;
  bw_state *fs = &full_set[tx_size][type][is_inter][band][context]
                          [last_is_zero][range-128];

  if (fs->state[0].range)
    return fs;

  if (last_is_zero)
    fs->state[0].range = 1;
  else
    one_bit(prob1, &fs->state[0], &lowvalue, &range, &count);

  // bit 1
  node_one_bit(prob2, fs, 0, &lowvalue, &range, &count);

  // bit 2
  node_one_bit(prob3, fs, 1, &lowvalue, &range, &count);

  // bit 2 / 1 branch
  set_state(&fs->state[3], range, count);
  fs->lowvalue[2] = BD_VALUEIZE(lowvalue, fs->state[3].count);

  return fs;
}

void empty_one_range(PLANE_TYPE type, int is_inter, int band,
                     TX_SIZE tx_size, int context) {
  int k;
  for (k = 0; k < 128; ++k) {
    full_set[tx_size][type][is_inter][band][context][0][k].state[0].range = 0;
    full_set[tx_size][type][is_inter][band][context][1][k].state[0].range = 0;
  }
}

void empty_entire_table(void) {
  int h, i, j, k, l;
  for (h = 0; h < TX_SIZES; ++h)
    for (i = 0; i < BLOCK_TYPES; ++i)
      for (j = 0; j < REF_TYPES; ++j)
        for (k = 0; k < COEF_BANDS; ++k)
          for (l = 0; l < PREV_COEF_CONTEXTS; ++l)
            empty_one_range(i, j, k, h, l);
}

static const vp9_prob cat6_prob[15] = {
  254, 254, 254, 252, 249, 243, 230, 196, 177, 153, 140, 133, 130, 129, 0
};

static const int token_to_counttoken[MAX_ENTROPY_TOKENS] = {
  ZERO_TOKEN, ONE_TOKEN, TWO_TOKEN, TWO_TOKEN,
  TWO_TOKEN, TWO_TOKEN, TWO_TOKEN, TWO_TOKEN,
  TWO_TOKEN, TWO_TOKEN, TWO_TOKEN, DCT_EOB_MODEL_TOKEN
};

#define INCREMENT_COUNT(token)                              \
  do {                                                      \
     if (!cm->frame_parallel_decoding_mode)                 \
       ++coef_counts[band][pt][token_to_counttoken[token]]; \
  } while (0)


#define WRITE_COEF_CONTINUE(val, token)                  \
  {                                                      \
    v = (val * dqv) >> dq_shift; \
    dqcoeff_ptr[scan[c]] = (vp9_read_bit(r) ? -v : v); \
    INCREMENT_COUNT(token);                              \
    token_cache[scan[c]] = vp9_pt_energy_class[token];   \
    ++c;                                                 \
    pt = get_coef_context(nb, token_cache, c);           \
    dqv = dq[1];                                          \
    continue;                                            \
  }


#define ADJUST_COEF(prob, bits_count)                   \
  do {                                                  \
    val += (vp9_read(r, prob) << bits_count);           \
  } while (0)
#define ADJUSTED_LOW(V) (((BD_VALUE)V) << (BD_VALUE_SIZE - 32))

inline void adjust_state(vp9_reader *r, bw_state *fs, int node) {
  if (node) r->value -= ADJUSTED_LOW(fs->lowvalue[node-1]);
  r->value <<= fs->state[node].count;
  r->count -= fs->state[node].count;
  r->range = fs->state[node].range;
}

inline int parse_bit(vp9_reader *r, bw_state *fs, int node) {
  if (r->value < ADJUSTED_LOW(fs->lowvalue[node])) {
    adjust_state(r, fs, node);
    return 1;
  }
  return 0;
}
static int decode_coefs(VP9_COMMON *cm, const MACROBLOCKD *xd,
                        vp9_reader *r, int block_idx,
                        PLANE_TYPE type, int seg_eob, int16_t *dqcoeff_ptr,
                        TX_SIZE tx_size, const int16_t *dq, int pt,
                        uint8_t *token_cache) {
  const FRAME_CONTEXT *const fc = &cm->fc;
  FRAME_COUNTS *const counts = &cm->counts;
  const int ref = is_inter_block(&xd->mi_8x8[0]->mbmi);
  int band, c = 0;
  const vp9_prob (*coef_probs)[PREV_COEF_CONTEXTS][UNCONSTRAINED_NODES] =
      fc->coef_probs[tx_size][type][ref];
  const vp9_prob *prob;
  unsigned int (*coef_counts)[PREV_COEF_CONTEXTS][UNCONSTRAINED_NODES + 1] =
      counts->coef[tx_size][type][ref];
  unsigned int (*eob_branch_count)[PREV_COEF_CONTEXTS] =
      counts->eob_branch[tx_size][type][ref];
  const uint8_t *cat6;
  const uint8_t *band_translate = get_band_translate(tx_size);
  const int dq_shift = (tx_size == TX_32X32);
  const scan_order *so = get_scan(xd, tx_size, type, block_idx);
  const int16_t *scan = so->scan;
  const int16_t *nb = so->neighbors;
  int v;
  int16_t dqv = dq[0];
  bw_state *fs;

  while (c < seg_eob) {
    int val;

    band = *band_translate++;
    prob = coef_probs[band][pt];
    fs = fill_table(type, ref, band, tx_size, pt, r->range, 0,
                    prob[EOB_CONTEXT_NODE], prob[ZERO_CONTEXT_NODE],
                    prob[ONE_CONTEXT_NODE]);

    if (!cm->frame_parallel_decoding_mode)
      ++eob_branch_count[band][pt];

    if (r->count < 0)
      vp9_reader_fill(r);
    if (parse_bit(r, fs, EOB_CONTEXT_NODE))
      break;

  DECODE_ZERO:
    if (r->count < fs->state[3].count)
      vp9_reader_fill(r);

    if (parse_bit(r, fs, ZERO_CONTEXT_NODE)) {
      INCREMENT_COUNT(ZERO_TOKEN);
      dqv = dq[1];
      ++c;
      if (c >= seg_eob)
        break;
      pt = get_coef_context(nb, token_cache, c);
      band = *band_translate++;
      prob = coef_probs[band][pt];
      fs = fill_table(type, ref, band, tx_size, pt, r->range, 1,
                      prob[EOB_CONTEXT_NODE], prob[ZERO_CONTEXT_NODE],
                      prob[ONE_CONTEXT_NODE]);
      goto DECODE_ZERO;
    }
    // ONE_CONTEXT_NODE_0_
    if (parse_bit(r, fs, ONE_CONTEXT_NODE)) {
      WRITE_COEF_CONTINUE(1, ONE_TOKEN);
    }
    adjust_state(r, fs, 3);
    prob = vp9_pareto8_full[coef_probs[band][pt][PIVOT_NODE]-1];

    // LOW_VAL_CONTEXT_NODE_0_
    if (!vp9_read(r, prob[LOW_VAL_CONTEXT_NODE])) {
      if (!vp9_read(r, prob[TWO_CONTEXT_NODE])) {
        WRITE_COEF_CONTINUE(2, TWO_TOKEN);
      }
      if (!vp9_read(r, prob[THREE_CONTEXT_NODE])) {
        WRITE_COEF_CONTINUE(3, THREE_TOKEN);
      }
      WRITE_COEF_CONTINUE(4, FOUR_TOKEN);
    }
    // HIGH_LOW_CONTEXT_NODE_0_
    if (!vp9_read(r, prob[HIGH_LOW_CONTEXT_NODE])) {
      if (!vp9_read(r, prob[CAT_ONE_CONTEXT_NODE])) {
        val = CAT1_MIN_VAL;
        ADJUST_COEF(CAT1_PROB0, 0);
        WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY1);
      }
      val = CAT2_MIN_VAL;
      ADJUST_COEF(CAT2_PROB1, 1);
      ADJUST_COEF(CAT2_PROB0, 0);
      WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY2);
    }
    // CAT_THREEFOUR_CONTEXT_NODE_0_
    if (!vp9_read(r, prob[CAT_THREEFOUR_CONTEXT_NODE])) {
      if (!vp9_read(r, prob[CAT_THREE_CONTEXT_NODE])) {
        val = CAT3_MIN_VAL;
        ADJUST_COEF(CAT3_PROB2, 2);
        ADJUST_COEF(CAT3_PROB1, 1);
        ADJUST_COEF(CAT3_PROB0, 0);
        WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY3);
      }
      val = CAT4_MIN_VAL;
      ADJUST_COEF(CAT4_PROB3, 3);
      ADJUST_COEF(CAT4_PROB2, 2);
      ADJUST_COEF(CAT4_PROB1, 1);
      ADJUST_COEF(CAT4_PROB0, 0);
      WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY4);
    }
    // CAT_FIVE_CONTEXT_NODE_0_:
    if (!vp9_read(r, prob[CAT_FIVE_CONTEXT_NODE])) {
      val = CAT5_MIN_VAL;
      ADJUST_COEF(CAT5_PROB4, 4);
      ADJUST_COEF(CAT5_PROB3, 3);
      ADJUST_COEF(CAT5_PROB2, 2);
      ADJUST_COEF(CAT5_PROB1, 1);
      ADJUST_COEF(CAT5_PROB0, 0);
      WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY5);
    }
    val = 0;
    cat6 = cat6_prob;
    while (*cat6)
      val = (val << 1) | vp9_read(r, *cat6++);
    val += CAT6_MIN_VAL;

    WRITE_COEF_CONTINUE(val, DCT_VAL_CATEGORY6);
  }

  if (c < seg_eob) {
    if (!cm->frame_parallel_decoding_mode)
      ++coef_counts[band][pt][DCT_EOB_MODEL_TOKEN];
  }

  return c;
}

int vp9_decode_block_tokens(VP9_COMMON *cm, MACROBLOCKD *xd,
                            int plane, int block, BLOCK_SIZE plane_bsize,
                            int x, int y, TX_SIZE tx_size, vp9_reader *r,
                            uint8_t *token_cache) {
  struct macroblockd_plane *const pd = &xd->plane[plane];
  const int seg_eob = get_tx_eob(&cm->seg, xd->mi_8x8[0]->mbmi.segment_id,
                                 tx_size);
  const int pt = get_entropy_context(tx_size, pd->above_context + x,
                                              pd->left_context + y);
  const int eob = decode_coefs(cm, xd, r, block, pd->plane_type, seg_eob,
                               BLOCK_OFFSET(pd->dqcoeff, block), tx_size,
                               pd->dequant, pt, token_cache);
  set_contexts(xd, pd, plane_bsize, tx_size, eob > 0, x, y);
  pd->eobs[block] = eob;
  return eob;
}


