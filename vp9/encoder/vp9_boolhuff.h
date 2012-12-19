/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


/****************************************************************************
*
*   Module Title :     vp9_boolhuff.h
*
*   Description  :     Bool Coder header file.
*
****************************************************************************/
#ifndef VP9_ENCODER_VP9_BOOLHUFF_H_
#define VP9_ENCODER_VP9_BOOLHUFF_H_

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "vpx_ports/mem.h"

#if CONFIG_MULTISYMBOL
#include "vpx_ports/vpx_clz.h"

/* MRC namespace: Multisymbol Range Coder */
typedef struct mrc_enc mrc_enc;
typedef struct mrc_enc BOOL_CODER;

#define MRC_TREE_FTB (15)
#define MRC_TREE_FT  (1<<MRC_TREE_FTB)

struct mrc_enc {
  unsigned short *buf;
  size_t          cbuf;
  size_t          nbuf;
  unsigned        low;
  short           cnt;
  unsigned short  rng;
  unsigned char  *out;
};

static int mrc_encode(mrc_enc *br, unsigned _fl, unsigned _fh, unsigned _ftb) {
  unsigned l;
  unsigned r;
  int      c;
  int      s;
  unsigned d;
  unsigned u;
  unsigned v;
  l = br->low;
  c = br->cnt;
  r = br->rng;
  u = r * _fl >> _ftb;
  v = r * _fh >> _ftb;
  r = v - u;
  l += u;
  d = 16 - VPX_ILOGNZ_32(r);
  s = c + d;
  /* TODO: Right now we flush every time we have at least one byte available.
   * Instead we should use an mrc_window and flush right before we're about to
   * shift bits off the end of the window.
   * For a 32-bit window this is about the same amount of work, but for a 64-bit
   * window it should be a fair win.
   */
  if (s >= 0) {
    unsigned short *buf;
    size_t          cbuf;
    size_t          nbuf;
    unsigned        m;
    buf = br->buf;
    cbuf = br->cbuf;
    nbuf = br->nbuf;
    if (nbuf + 2 >= cbuf) {
      cbuf = (cbuf << 1) + 2;
      buf = (unsigned short *)realloc(buf, cbuf * sizeof(*buf));
      if (buf == NULL)return -1;
    }
    c += 16;
    m = (1 << c) - 1;
    if (s >= 8) {
      buf[nbuf++] = (unsigned short)(l >> c);
      l &= m;
      c -= 8;
      m >>= 8;
    }
    buf[nbuf++] = (unsigned short)(l >> c);
    s = c + d - 24;
    l &= m;
    br->buf = buf;
    br->cbuf = cbuf;
    br->nbuf = nbuf;
  }
  br->low = l << d;
  br->cnt = s;
  br->rng = r << d;
  return 0;
}

static void encode_bool(BOOL_CODER *br, int bit, int prob) {
  unsigned int split = (MRC_TREE_FT * prob + 128) >> 8;

  assert(prob || bit == 1);
  mrc_encode(br, bit ? split : 0, bit ? MRC_TREE_FT : split, MRC_TREE_FTB);
}

#else

typedef struct {
  unsigned int lowvalue;
  unsigned int range;
  unsigned int value;
  int count;
  unsigned int pos;
  unsigned char *buffer;

  // Variables used to track bit costs without outputing to the bitstream
  unsigned int  measure_cost;
  unsigned long bit_counter;
} BOOL_CODER;

DECLARE_ALIGNED(16, extern const unsigned char, vp9_norm[256]);


static void encode_bool(BOOL_CODER *br, int bit, int probability) {
  unsigned int split;
  int count = br->count;
  unsigned int range = br->range;
  unsigned int lowvalue = br->lowvalue;
  register unsigned int shift;

#ifdef ENTROPY_STATS
#if defined(SECTIONBITS_OUTPUT)

  if (bit)
    Sectionbits[active_section] += vp9_prob_cost[255 - probability];
  else
    Sectionbits[active_section] += vp9_prob_cost[probability];

#endif
#endif

  split = 1 + (((range - 1) * probability) >> 8);

  range = split;

  if (bit) {
    lowvalue += split;
    range = br->range - split;
  }

  shift = vp9_norm[range];

  range <<= shift;
  count += shift;

  if (count >= 0) {
    int offset = shift - count;

    if ((lowvalue << (offset - 1)) & 0x80000000) {
      int x = br->pos - 1;

      while (x >= 0 && br->buffer[x] == 0xff) {
        br->buffer[x] = (unsigned char)0;
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

#endif

extern void vp9_start_encode(BOOL_CODER *bc, unsigned char *buffer);
extern int vp9_stop_encode(BOOL_CODER *bc);
extern void vp9_encode_value(BOOL_CODER *br, int data, int bits);
extern void vp9_encode_unsigned_max(BOOL_CODER *br, int data, int max);
extern const unsigned int vp9_prob_cost[256];

extern void vp9_encode_uniform(BOOL_CODER *bc, int v, int n);
extern void vp9_encode_term_subexp(BOOL_CODER *bc, int v, int k, int n);
extern int vp9_count_uniform(int v, int n);
extern int vp9_count_term_subexp(int v, int k, int n);
extern int vp9_recenter_nonneg(int v, int m);

#endif  // VP9_ENCODER_VP9_BOOLHUFF_H_
