/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_DECODER_VP9_DBOOLHUFF_H_
#define VP9_DECODER_VP9_DBOOLHUFF_H_

#include <limits.h>
#include <stddef.h>
#include "vpx_ports/config.h"
#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"

#if CONFIG_MULTISYMBOL
#include "vpx_ports/vpx_clz.h"

/* MRC namespace: Multisymbol Range Coder */
#define MRC_TREE_FTB (15)
#define MRC_TREE_FT  (1<<MRC_TREE_FTB)

typedef size_t mrc_window;
typedef struct mrc_dec mrc_dec;
typedef struct mrc_dec BOOL_DECODER;

#define MRC_WINDOW_SZ ((int)sizeof(mrc_window)*CHAR_BIT)
/* This is meant to be a large, positive constant that can still be efficiently
 * loaded as an immediate (on platforms like ARM, for example).
 * Even relatively modest values like 100 would work fine.
 */
#define MRC_LOTS_OF_BITS (0x4000)

struct mrc_dec {
  const uint8_t *buf;
  const uint8_t *end;
  mrc_window     diff;
  int16_t        count;
  uint16_t       range;
};

/* MRC_WINDOW_SZ - 1 bits available in the window
 * -8                adjustment to get to the LSB of the incoming byte
 * -(c + 15)         fractional bits already read
 */
#define MRC_SHIFT_BITS(c) (MRC_WINDOW_SZ - 9 - (c + 15))

static int mrc_decode(mrc_dec *br, const uint16_t f[], uint32_t ftb) {
  mrc_window diff;
  uint32_t   r;
  uint32_t   d;
  int        c;
  int        s;
  uint32_t   u;
  uint32_t   v;
  uint32_t   q;
  int        l;
  diff = br->diff;
  c = br->count;
  r = br->range;
  q = diff >> (MRC_WINDOW_SZ - 16);
  u = 0;
  v = (f[0] * r) >> ftb;
  for (l = 0; v <= q;) {
    u = v;
    v = (f[++l] * r) >> ftb;
  }
  r = v - u;
  diff -= (mrc_window)u << (MRC_WINDOW_SZ - 16);
  d = 16 - VPX_ILOGNZ_32(r);
  s = c - d;
  if (s < 0) {
    do {
      const uint8_t *buf = br->buf;
      const uint8_t *end = br->end;
      int loop_end, last_bit, fill;
      int bits_left = (int)(end-buf)*CHAR_BIT;

      fill = MRC_SHIFT_BITS(c);
      last_bit = fill + CHAR_BIT - bits_left;
      loop_end = 0;
      if (last_bit >= 0) {
        c += MRC_LOTS_OF_BITS;
        loop_end = last_bit;
        if (!bits_left)
          break;
      }
      while (fill >= loop_end) {
        c += 8;
        diff |= ((mrc_window)buf[0]) << fill;
        ++buf;
        fill -= 8;
      }
      br->buf = buf;
    } while (0);
    s = c - d;
  }
  br->diff = diff << d;
  br->count = s;
  br->range = r << d;
  return l;
}

static int decode_bool(BOOL_DECODER *br, int prob) {
  uint16_t   split = (MRC_TREE_FT * prob + 128) >> 8;
  mrc_window diff;
  uint32_t   r;
  uint32_t   d;
  int        c;
  int        s;
  uint32_t   v;
  uint32_t   q;
  int        l;
  diff = br->diff;
  c = br->count;
  r = br->range;
  q = diff >> (MRC_WINDOW_SZ - 16);
  v = split * r >> MRC_TREE_FTB;
  if (v <= q) {
    l = 1;
    r -= v;
    diff -= (mrc_window)v << (MRC_WINDOW_SZ - 16);
  } else {
    l = 0;
    r = v;
  }
  d = 16 - VPX_ILOGNZ_32(r);
  s = c - d;
  if (s < 0) {
    do {
      const uint8_t *buf = br->buf;
      const uint8_t *end = br->end;
      int loop_end, last_bit, fill;
      int bits_left = (int)(end-buf)*CHAR_BIT;

      fill = MRC_SHIFT_BITS(c);
      last_bit = fill + CHAR_BIT - bits_left;
      loop_end = 0;
      if (last_bit >= 0) {
        c += MRC_LOTS_OF_BITS;
        loop_end = last_bit;
        if (!bits_left)
          break;
      }
      while (fill >= loop_end) {
        c += 8;
        diff |= ((mrc_window)buf[0]) << fill;
        ++buf;
        fill -= 8;
      }
      br->buf = buf;
    } while (0);
    s = c - d;
  }
  br->diff = diff << d;
  br->count = s;
  br->range = r << d;
  return l;
}

static int decode_value(BOOL_DECODER *br, int bits) {
  int z = 0;
  int bit;

  for (bit = bits - 1; bit >= 0; bit--) {
    z |= decode_bool(br, 0x80) << bit;
  }
  return z;
}

static int bool_error(BOOL_DECODER *br) {
 /* Check if we have reached the end of the buffer.
   *
   * Variable 'count' stores the number of bits in the 'diff' buffer, minus
   * 15. The top two bytes are part of the algorithm (the sign bit is
   * unused)  and the remainder is buffered to be shifted into it.
   * So if count == 8, the top 24 bits of 'value' are
   * occupied, 1 unused, 15 for the algorithm and 8 in the buffer.
   *
   * When reading a byte from the user's buffer, count is filled with 8 and
   * one byte is filled into the value buffer. When we reach the end of the
   * data, count is additionally filled with VP9_LOTS_OF_BITS. So when
   * count == VP9_LOTS_OF_BITS - 1, the user's data has been exhausted.
   *
   * TODO(jkoleszar): Validate this logic more thoroughly, since the encoder
   * will terminate optimally, relying on the "infinte trailing zeros"
   * provided by the LOTS_OF_BITS implementation. It's only used for
   * error concealment, which is currently unimplemented for VP9.
   */
  if ((br->count > MRC_WINDOW_SZ) && (br->count+15 < MRC_LOTS_OF_BITS)) {
    /* We have tried to decode bits after the end of
     * stream was encountered.
     */
    return 1;
  }

  /* No error. */
  return 0;
}

#else

typedef size_t VP9_BD_VALUE;

# define VP9_BD_VALUE_SIZE ((int)sizeof(VP9_BD_VALUE)*CHAR_BIT)
/*This is meant to be a large, positive constant that can still be efficiently
   loaded as an immediate (on platforms like ARM, for example).
  Even relatively modest values like 100 would work fine.*/
# define VP9_LOTS_OF_BITS (0x40000000)

typedef struct {
  const unsigned char *user_buffer_end;
  const unsigned char *user_buffer;
  VP9_BD_VALUE         value;
  int                  count;
  unsigned int         range;
} BOOL_DECODER;

DECLARE_ALIGNED(16, extern const uint8_t, vp9_norm[256]);

void vp9_bool_decoder_fill(BOOL_DECODER *br);

/*The refill loop is used in several places, so define it in a macro to make
   sure they're all consistent.
  An inline function would be cleaner, but has a significant penalty, because
   multiple BOOL_DECODER fields must be modified, and the compiler is not smart
   enough to eliminate the stores to those fields and the subsequent reloads
   from them when inlining the function.*/
#define VP9DX_BOOL_DECODER_FILL(_count,_value,_bufptr,_bufend) \
  do \
  { \
    int shift = VP9_BD_VALUE_SIZE - 8 - ((_count) + 8); \
    int loop_end, x; \
    int bits_left = (int)(((_bufend)-(_bufptr))*CHAR_BIT); \
    \
    x = shift + CHAR_BIT - bits_left; \
    loop_end = 0; \
    if(x >= 0) \
    { \
      (_count) += VP9_LOTS_OF_BITS; \
      loop_end = x; \
      if(!bits_left) break; \
    } \
    while(shift >= loop_end) \
    { \
      (_count) += CHAR_BIT; \
      (_value) |= (VP9_BD_VALUE)*(_bufptr)++ << shift; \
      shift -= CHAR_BIT; \
    } \
  } \
  while(0) \


static int decode_bool(BOOL_DECODER *br, int probability) {
  unsigned int bit = 0;
  VP9_BD_VALUE value;
  unsigned int split;
  VP9_BD_VALUE bigsplit;
  int count;
  unsigned int range;

  split = 1 + (((br->range - 1) * probability) >> 8);

  if (br->count < 0)
    vp9_bool_decoder_fill(br);

  value = br->value;
  count = br->count;

  bigsplit = (VP9_BD_VALUE)split << (VP9_BD_VALUE_SIZE - 8);

  range = split;

  if (value >= bigsplit) {
    range = br->range - split;
    value = value - bigsplit;
    bit = 1;
  }

  {
    register unsigned int shift = vp9_norm[range];
    range <<= shift;
    value <<= shift;
    count -= shift;
  }
  br->value = value;
  br->count = count;
  br->range = range;

  return bit;
}

static int decode_value(BOOL_DECODER *br, int bits) {
  int z = 0;
  int bit;

  for (bit = bits - 1; bit >= 0; bit--) {
    z |= (decode_bool(br, 0x80) << bit);
  }

  return z;
}

static int bool_error(BOOL_DECODER *br) {
  /* Check if we have reached the end of the buffer.
   *
   * Variable 'count' stores the number of bits in the 'value' buffer, minus
   * 8. The top byte is part of the algorithm, and the remainder is buffered
   * to be shifted into it. So if count == 8, the top 16 bits of 'value' are
   * occupied, 8 for the algorithm and 8 in the buffer.
   *
   * When reading a byte from the user's buffer, count is filled with 8 and
   * one byte is filled into the value buffer. When we reach the end of the
   * data, count is additionally filled with VP9_LOTS_OF_BITS. So when
   * count == VP9_LOTS_OF_BITS - 1, the user's data has been exhausted.
   */
  if ((br->count > VP9_BD_VALUE_SIZE) && (br->count < VP9_LOTS_OF_BITS)) {
    /* We have tried to decode bits after the end of
     * stream was encountered.
     */
    return 1;
  }

  /* No error. */
  return 0;
}

#endif /* CONFIG_MULTISYMBOL */

int vp9_start_decode(BOOL_DECODER *br,
                     const uint8_t *source,
                     size_t source_sz);
const uint8_t* vp9_stop_decode(BOOL_DECODER *br);

extern int vp9_decode_unsigned_max(BOOL_DECODER *br, int max);
int vp9_decode_uniform(BOOL_DECODER *br, int n);
int vp9_decode_term_subexp(BOOL_DECODER *br, int k, int num_syms);
int vp9_inv_recenter_nonneg(int v, int m);

#endif  // VP9_DECODER_VP9_DBOOLHUFF_H_
