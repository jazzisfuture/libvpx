/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_ENCODER_VP9_TOKENIZE_H_
#define VP9_ENCODER_VP9_TOKENIZE_H_

#include "vp9/common/vp9_entropy.h"

#include "vp9/encoder/vp9_block.h"
#include "vp9/encoder/vp9_treewriter.h"

#ifdef __cplusplus
extern "C" {
#endif

void vp9_tokenize_initialize();

#define EOSB_TOKEN 127     // Not signalled, encoder only

typedef struct {
  int16_t token;
#if CONFIG_VP9_HIGHBITDEPTH
  int32_t extra;
#else
  int16_t extra;
#endif
} TOKENVALUE;

typedef struct {
  const vp9_prob *context_tree;
#if CONFIG_VP9_HIGHBITDEPTH
  int32_t extra;
#else
  int16_t         extra;
#endif
  uint8_t         token;
  uint8_t         skip_eob_node;
#if CONFIG_CODE_ZEROGROUP
  uint8_t         skip_coef_val;
#endif  // CONFIG_CODE_ZEROGROUP
#if CONFIG_TX_SKIP
  uint8_t         is_pxd_token;
#endif  // CONFIG_TX_SKIP
} TOKENEXTRA;

extern const vp9_tree_index vp9_coef_tree[];
extern const vp9_tree_index vp9_coef_con_tree[];
extern struct vp9_token vp9_coef_encodings[];

int vp9_is_skippable_in_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane);
int vp9_has_high_freq_in_plane(MACROBLOCK *x, BLOCK_SIZE bsize, int plane);

struct VP9_COMP;

void vp9_tokenize_sb(struct VP9_COMP *cpi, TOKENEXTRA **t, int dry_run,
                     BLOCK_SIZE bsize);
#if CONFIG_SUPERTX
void vp9_tokenize_sb_supertx(struct VP9_COMP *cpi, TOKENEXTRA **t, int dry_run,
                             BLOCK_SIZE bsize);
#endif

#if CONFIG_CODE_ZEROGROUP
int vp9_is_eoo(int c, int eob, const int16_t *scan, TX_SIZE tx_size,
               int *last_nz_pos);
#endif  // CONFIG_CODE_ZEROGROUP

extern const int16_t *vp9_dct_value_cost_ptr;
/* TODO: The Token field should be broken out into a separate char array to
 *  improve cache locality, since it's needed for costing when the rest of the
 *  fields are not.
 */
extern const TOKENVALUE *vp9_dct_value_tokens_ptr;
#if CONFIG_VP9_HIGHBITDEPTH
extern const int16_t *vp9_dct_value_cost_high10_ptr;
extern const TOKENVALUE *vp9_dct_value_tokens_high10_ptr;
extern const int16_t *vp9_dct_value_cost_high12_ptr;
extern const TOKENVALUE *vp9_dct_value_tokens_high12_ptr;
#endif  // CONFIG_VP9_HIGHBITDEPTH

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_TOKENIZE_H_
