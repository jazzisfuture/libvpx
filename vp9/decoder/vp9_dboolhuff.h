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

#include <stddef.h>
#include <limits.h>
#include "vpx_ports/config.h"
#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"

#if CONFIG_MULTISYMBOL

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

typedef size_t ec_window;
typedef struct ec_dec ec_dec;
typedef struct ec_dec BOOL_DECODER;

#define EC_WINDOW_SZ ((int)sizeof(ec_window)*CHAR_BIT)
/*This is meant to be a large, positive constant that can still be efficiently
   loaded as an immediate (on platforms like ARM, for example).
  Even relatively modest values like 100 would work fine.*/
#define EC_LOTS_OF_BITS (0x4000)

struct ec_dec {
  const unsigned char *buf;
  const unsigned char *end;
  ec_window            dif;
  short                cnt;
  unsigned short       rng;
};

static void ec_dec_init(ec_dec *_this, const unsigned char *_buf, size_t _sz) {
  const unsigned char *end;
  ec_window            dif;
  int                  c;
  int                  s;
  end = _buf + _sz;
  dif = 0;
  c = -15;
  for (s = EC_WINDOW_SZ - 9; s >= 0;) {
    if (_buf >= end) {
      c = EC_LOTS_OF_BITS;
      break;
    }
    c += 8;
    dif |= (ec_window) * _buf++ << s;
    s -= 8;
  }
  _this->buf = _buf;
  _this->end = end;
  _this->dif = dif;
  _this->cnt = c;
  _this->rng = 0x8000;
}

static int ec_decode(ec_dec *_this, const unsigned short _f[16],
                     unsigned _ftb) {
  ec_window dif;
  unsigned  r;
  unsigned  d;
  int       c;
  int       s;
  unsigned  u;
  unsigned  v;
  unsigned  q;
  int       l;
#if defined(EC_MULT_FREE)
  unsigned  fl;
  unsigned  fh;
  unsigned  ft;
#endif
  dif = _this->dif;
  c = _this->cnt;
  r = _this->rng;
#if defined(EC_MULT_FREE)
  ft = 1 << _ftb;
  s = r >= ft << 1;
  if (s)ft <<= 1;
  d = r - ft;
  q = EC_MAXI((int)(dif >> EC_WINDOW_SZ - 16 + 1),
              (int)((dif >> EC_WINDOW_SZ - 16) - d)) >> s;
  fl = 0;
  for (l = 0; _f[l] <= q; l++)fl = _f[l];
  fh = _f[l];
  /*printf("0x%08X %2i 0x%04X  [0x%04X,0x%04X) {0x%04X}\n",
   (int)(_this->low>>EC_WINDOW_SZ-16),_this->cnt,_this->rng,fl,fh,1<<_ftb);*/
  fl <<= s;
  fh <<= s;
  u = fl + EC_MINI(fl, d);
  v = fh + EC_MINI(fh, d);
#else
  q = (unsigned)(dif >> (EC_WINDOW_SZ - 16));
  u = 0;
  v = _f[0] * r >> _ftb;
  for (l = 0; v <= q;) {
    u = v;
    v = _f[++l] * r >> _ftb;
  }
#endif
  r = v - u;
  dif -= (ec_window)u << (EC_WINDOW_SZ - 16);
  d = 16 - EC_ILOGNZ_32(r);
  s = c - d;
  if (s < 0) {
    const unsigned char *buf;
    const unsigned char *end;
    buf = _this->buf;
    end = _this->end;
    for (s = EC_WINDOW_SZ - 9 - (c + 15); s >= 0;) {
      if (buf >= end) {
        c = EC_LOTS_OF_BITS;
        break;
      }
      c += 8;
      dif |= (ec_window) * buf++ << s;
      s -= 8;
    }
    s = c - d;
    _this->buf = buf;
  }
  _this->dif = dif << d;
  _this->cnt = s;
  _this->rng = r << d;
  return l;
}

static int vp9_start_decode(BOOL_DECODER *br,
                            const unsigned char *source,
                            unsigned int source_sz) {
  ec_dec_init(br, source, source_sz);
  return (source_sz && !source);
}

static int decode_bool(BOOL_DECODER *br, int prob) {
  unsigned short f[2];

  f[0] = (EC_TREE_FT * prob + 128) >> 8;
  f[1] = EC_TREE_FT;
  return ec_decode(br, f, EC_TREE_FTB);
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
  return 0; /* TODO: implement this. */
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

int vp9_start_decode(BOOL_DECODER *br,
                     const unsigned char *source,
                     unsigned int source_sz);

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

extern int vp9_decode_unsigned_max(BOOL_DECODER *br, int max);
int vp9_decode_uniform(BOOL_DECODER *br, int n);
int vp9_decode_term_subexp(BOOL_DECODER *br, int k, int num_syms);
int vp9_inv_recenter_nonneg(int v, int m);

#endif  // VP9_DECODER_VP9_DBOOLHUFF_H_
