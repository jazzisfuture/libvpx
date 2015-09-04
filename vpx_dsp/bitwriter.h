/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_BITWRITER_H_
#define VPX_DSP_BITWRITER_H_

#include "vpx_ports/mem.h"

#include "vpx_dsp/prob.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vpx_token {
  int value;
  int len;
};

typedef struct vpx_writer {
  unsigned int lowvalue;
  unsigned int range;
  int count;
  unsigned int pos;
  uint8_t *buffer;
} vpx_writer;

static INLINE void vpx_write(vpx_writer *br, int bit, int probability) {
  unsigned int split;
  int count = br->count;
  unsigned int range = br->range;
  unsigned int lowvalue = br->lowvalue;
  register unsigned int shift;

  split = 1 + (((range - 1) * probability) >> 8);

  range = split;

  if (bit) {
    lowvalue += split;
    range = br->range - split;
  }

  shift = vpx_norm[range];

  range <<= shift;
  count += shift;

  if (count >= 0) {
    int offset = shift - count;

    if ((lowvalue << (offset - 1)) & 0x80000000) {
      int x = br->pos - 1;

      while (x >= 0 && br->buffer[x] == 0xff) {
        br->buffer[x] = 0;
        x--;
      }

      br->buffer[x] += 1;
    }

    br->buffer[br->pos++] = (lowvalue >> (24 - offset));
    lowvalue <<= offset;
    shift = count;
    lowvalue &= 0xffffff;
    count -= 8;
  }

  lowvalue <<= shift;
  br->count = count;
  br->lowvalue = lowvalue;
  br->range = range;
}

static INLINE void vpx_write_bit(vpx_writer *w, int bit) {
  vpx_write(w, bit, 128);  // vpx_prob_half
}

static INLINE void vpx_write_literal(vpx_writer *w, int data, int bits) {
  int bit;

  for (bit = bits - 1; bit >= 0; bit--)
    vpx_write_bit(w, 1 & (data >> bit));
}

static INLINE void vpx_write_tree(vpx_writer *w, const vpx_tree_index *tree,
                                  const vpx_prob *probs, int bits, int len,
                                  vpx_tree_index i) {
  do {
    const int bit = (bits >> --len) & 1;
    vpx_write(w, bit, probs[i >> 1]);
    i = tree[i + bit];
  } while (len);
}

void vpx_tree_probs_from_distribution(vpx_tree tree,
                                      unsigned int branch_ct[ /* n - 1 */ ][2],
                                      const unsigned int num_events[ /* n */ ]);
void vpx_tokens_from_tree(struct vpx_token*, const vpx_tree_index *);
void vpx_start_encode(vpx_writer *bc, uint8_t *buffer);
void vpx_stop_encode(vpx_writer *bc);

#define vpx_write_prob(w, v) vpx_write_literal((w), (v), 8)

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_DSP_BITWRITER_H_
