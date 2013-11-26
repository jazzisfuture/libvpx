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

#include "./vpx_config.h"
#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"

typedef size_t BD_VALUE;

#define BD_VALUE_SIZE ((int)sizeof(BD_VALUE) * CHAR_BIT)

typedef struct {
  const uint8_t *buffer_end;
  const uint8_t *buffer;
  BD_VALUE value;
  int count;
  unsigned int range;
} vp9_reader;

DECLARE_ALIGNED(16, extern const uint8_t, vp9_norm[256]);

int vp9_reader_init(vp9_reader *r, const uint8_t *buffer, size_t size);

void vp9_reader_fill(vp9_reader *r);

const uint8_t *vp9_reader_find_end(vp9_reader *r);

static int vp9_read(vp9_reader *r, int prob) {
  BD_VALUE value, big_split;
  int shift, bit;
  unsigned int range, split;

  if (r->count < 0)
    vp9_reader_fill(r);

  split = (r->range * prob + (256 - prob)) >> CHAR_BIT;
  big_split = (BD_VALUE)split << (BD_VALUE_SIZE - CHAR_BIT);
  value = r->value;
  bit = (value >= big_split);
  if (bit)
    value -= big_split;
  range = bit ? r->range - split : split;
  shift = vp9_norm[range];
  r->count -= shift;
  r->value = value << shift;
  r->range = range << shift;

  return bit;
}

static int vp9_read_bit(vp9_reader *r) {
  return vp9_read(r, 128);  // vp9_prob_half
}

static int vp9_read_literal(vp9_reader *br, int bits) {
  int z = 0, bit;

  for (bit = bits - 1; bit >= 0; bit--)
    z |= vp9_read_bit(br) << bit;

  return z;
}

int vp9_reader_has_error(vp9_reader *r);

#endif  // VP9_DECODER_VP9_DBOOLHUFF_H_
