/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./bitwriter.h"

static void tree2tok(struct vpx_token *tokens, const vpx_tree_index *tree,
                     int i, int v, int l) {
  v += v;
  ++l;

  do {
    const vpx_tree_index j = tree[i++];
    if (j <= 0) {
      tokens[-j].value = v;
      tokens[-j].len = l;
    } else {
      tree2tok(tokens, tree, j, v, l);
    }
  } while (++v & 1);
}

void vpx_tokens_from_tree(struct vpx_token *tokens,
                          const vpx_tree_index *tree) {
  tree2tok(tokens, tree, 0, 0, 0);
}

static unsigned int convert_distribution(unsigned int i, vpx_tree tree,
                                         unsigned int branch_ct[][2],
                                         const unsigned int num_events[]) {
  unsigned int left, right;

  if (tree[i] <= 0)
    left = num_events[-tree[i]];
  else
    left = convert_distribution(tree[i], tree, branch_ct, num_events);

  if (tree[i + 1] <= 0)
    right = num_events[-tree[i + 1]];
  else
    right = convert_distribution(tree[i + 1], tree, branch_ct, num_events);

  branch_ct[i >> 1][0] = left;
  branch_ct[i >> 1][1] = right;
  return left + right;
}

void vpx_tree_probs_from_distribution(vpx_tree tree,
                                      unsigned int branch_ct[/* n-1 */][2],
                                      const unsigned int num_events[/* n */]) {
  convert_distribution(0, tree, branch_ct, num_events);
}

void vpx_start_encode(vpx_writer *br, uint8_t *source) {
  br->lowvalue = 0;
  br->range    = 255;
  br->count    = -24;
  br->buffer   = source;
  br->pos      = 0;
  vpx_write_bit(br, 0);
}

void vpx_stop_encode(vpx_writer *br) {
  int i;

  for (i = 0; i < 32; i++)
    vpx_write_bit(br, 0);

  // Ensure there's no ambigous collision with any index marker bytes
  if ((br->buffer[br->pos - 1] & 0xe0) == 0xc0)
    br->buffer[br->pos++] = 0;
}
