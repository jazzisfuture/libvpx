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

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "vpx_ports/mem.h"

#if CONFIG_MULTISYMBOL

typedef struct ec_enc ec_enc;
typedef struct ec_enc BOOL_CODER;
extern void vp9_encode_value(BOOL_CODER *br, int data, int bits);
extern void vp9_encode_unsigned_max(BOOL_CODER *br, int data, int max);
extern const unsigned int vp9_prob_cost[256];

extern void vp9_encode_uniform(BOOL_CODER *bc, int v, int n);
extern void vp9_encode_term_subexp(BOOL_CODER *bc, int v, int k, int n);
extern int vp9_count_uniform(int v, int n);
extern int vp9_count_term_subexp(int v, int k, int n);
extern int vp9_recenter_nonneg(int v, int m);

#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
         ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

# ifdef __GNUC_PREREQ
#  if __GNUC_PREREQ(3, 4)
/*Note the casts to (int) below: this prevents EC_CLZ{32|64}_OFFS from
   "upgrading" the type of an entire expression to an (unsigned) size_t.*/
#   if INT_MAX >= 2147483647
#    define EC_CLZ32_OFFS ((int)sizeof(unsigned)*CHAR_BIT)
#    define EC_CLZ32(_x) (__builtin_clz(_x))
#   elif LONG_MAX >= 2147483647L
#    define EC_CLZ32_OFFS ((int)sizeof(unsigned long)*CHAR_BIT)
#    define EC_CLZ32(_x) (__builtin_clzl(_x))
#   endif
#  endif
# endif

# if defined(EC_CLZ32)
#  define EC_ILOGNZ_32(_v) (EC_CLZ32_OFFS-EC_CLZ32(_v))
#  define EC_ILOG_32(_v)   (EC_ILOGNZ_32(_v)&-!!(_v))
# else
#  error "Need __builtin_clz or equivalent."
# endif

#define EC_TREE_FTB (15)
#define EC_TREE_FT  (1<<EC_TREE_FTB)

struct ec_enc {
  unsigned short *buf;
  size_t          cbuf;
  size_t          nbuf;
  unsigned        low;
  short           cnt;
  unsigned short  rng;
  unsigned char  *out;
};

static void ec_enc_init(ec_enc *_this) {
  _this->buf = NULL;
  _this->cbuf = 0;
  _this->nbuf = 0;
  _this->low = 0;
  _this->cnt = -9;
  _this->rng = 0x8000;
}

static int ec_encode(ec_enc *_this, unsigned _fl, unsigned _fh, unsigned _ftb) {
  unsigned l;
  unsigned r;
  int      c;
  int      s;
  unsigned d;
  unsigned u;
  unsigned v;
#if defined(EC_MULT_FREE)
  unsigned ft;
#endif
  /*printf("0x%08X %2i 0x%04X  [0x%04X,0x%04X) {0x%04X}\n",
   _this->low,_this->cnt,_this->rng,_fl,_fh,1<<_ftb);*/
  l = _this->low;
  c = _this->cnt;
  r = _this->rng;
#if defined(EC_MULT_FREE)
  ft = 1 << _ftb;
  s = r >= ft << 1;
  if (s)ft <<= 1;
  _fl <<= s;
  _fh <<= s;
  d = r - ft;
  u = _fl + EC_MINI(_fl, d);
  v = _fh + EC_MINI(_fh, d);
#else
  u = r * _fl >> _ftb;
  v = r * _fh >> _ftb;
#endif
  r = v - u;
  l += u;
  d = 16 - EC_ILOGNZ_32(r);
  s = c + d;
  /*TODO: Right now we flush every time we have at least one byte available.
    Instead we should use an ec_window and flush right before we're about to
     shift bits off the end of the window.
    For a 32-bit window this is about the same amount of work, but for a 64-bit
     window it should be a fair win.*/
  if (s >= 0) {
    unsigned short *buf;
    size_t          cbuf;
    size_t          nbuf;
    unsigned        m;
    buf = _this->buf;
    cbuf = _this->cbuf;
    nbuf = _this->nbuf;
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
    _this->buf = buf;
    _this->cbuf = cbuf;
    _this->nbuf = nbuf;
  }
  _this->low = l << d;
  _this->cnt = s;
  _this->rng = r << d;
  return 0;
}

static int ec_enc_done(ec_enc *_this, unsigned char **_out) {
  unsigned short *buf;
  size_t          cbuf;
  size_t          nbuf;
  unsigned char  *out;
  unsigned        mask;
  unsigned        end;
  unsigned        l;
  unsigned        r;
  int             c;
  int             d;
  int             s;
  size_t          i;
  buf = _this->buf;
  cbuf = _this->cbuf;
  nbuf = _this->nbuf;
  l = _this->low;
  c = _this->cnt;
  r = _this->rng;
  /*Figure out the minimum number of bits that ensures that the symbols encoded
     thus far will be decoded correctly regardless of the bits that follow.*/
  d = 0;
  mask = 0x7FFF;
  end = (l + mask)&~mask;
  if ((end | mask) >= l + r) {
    d++;
    mask >>= 1;
    end = (l + mask)&~mask;
  }
  /*Flush all remaining bits into the output buffer.*/
  s = c + d + 8;
  if (s >= 0) {
    unsigned m;
    if (nbuf + 1 + s / 8 >= cbuf) {
      cbuf += 1 + s / 8;
      cbuf = (cbuf << 1) + 2;
      buf = (unsigned short *)realloc(buf, cbuf * sizeof(*buf));
      if (buf == NULL)return -1;
    }
    m = (1 << (c + 16)) - 1;
    do {
      buf[nbuf++] = (unsigned short)(end >> (c + 16));
      end &= m;
      s -= 8;
      c -= 8;
      m >>= 8;
    } while (s >= 0);
    _this->buf = buf;
    _this->cbuf = cbuf;
  }
  /*Perform carry propagation.*/
  out = (unsigned char *)malloc(nbuf * sizeof(*out));
  if (out == NULL)return -1;
  l = 0;
  for (i = nbuf; i-- > 0;) {
    l = buf[i] + l;
    out[i] = (unsigned char)l;
    l >>= 8;
  }
  *_out = out;
  return nbuf;
}

static void vp9_start_encode(BOOL_CODER *br, unsigned char *source) {
  ec_enc_init(br);
  br->out = source;
}

static int vp9_stop_encode(BOOL_CODER *br) {
  unsigned char *out = NULL;
  int sz;

  sz = ec_enc_done(br, &out);
  memcpy(br->out, out, sz);
  free(br->buf);
  return sz;
}

static void encode_bool(BOOL_CODER *br, int bit, int prob) {
  unsigned int split = (EC_TREE_FT * prob + 128) >> 8;

  ec_encode(br, bit ? split : 0, bit ? EC_TREE_FT : split, EC_TREE_FTB);
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

extern void vp9_start_encode(BOOL_CODER *bc, unsigned char *buffer);

extern void vp9_encode_value(BOOL_CODER *br, int data, int bits);
extern void vp9_encode_unsigned_max(BOOL_CODER *br, int data, int max);
extern int vp9_stop_encode(BOOL_CODER *bc);
extern const unsigned int vp9_prob_cost[256];

extern void vp9_encode_uniform(BOOL_CODER *bc, int v, int n);
extern void vp9_encode_term_subexp(BOOL_CODER *bc, int v, int k, int n);
extern int vp9_count_uniform(int v, int n);
extern int vp9_count_term_subexp(int v, int k, int n);
extern int vp9_recenter_nonneg(int v, int m);

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
#endif  // VP9_ENCODER_VP9_BOOLHUFF_H_
