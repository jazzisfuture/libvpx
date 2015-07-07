/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdlib.h>

#include "vpx_ports/mem.h"
#include "vpx_mem/vpx_mem.h"

#include "./vpx_config.h"
#include "vp9/decoder/vp9_reader.h"

int vp9_reader_init(vp9_reader *r,
                    const uint8_t *buffer,
                    size_t size,
                    vpx_decrypt_cb decrypt_cb,
                    void *decrypt_state) {
  if (size && !buffer) {
    return 1;
  } else {
    r->buffer_end = buffer + size;
    r->buffer = buffer;
    r->value = 0;
    r->count = -8;
    r->range = 255;
    r->decrypt_cb = decrypt_cb;
    r->decrypt_state = decrypt_state;
    vp9_reader_fill(r);
    return vp9_read_bit(r) != 0;  // marker bit
  }
}
#if CONFIG_BIG_ENDIAN
#define BIGENDIFY64(X) (X)
#define BIGENDIFY32(X) (X)
#else
#if defined(__GNUC__)
  #define BIGENDIFY64(X) __builtin_bswap64(X)
#elif defined(_MSC_VER)
  #define BIGENDIFY64(X) _byteswap_uint64(X)
#else
  #define BIGENDIFY64(X) bswap_64(X)
#endif

#if defined(__GNUC__)
  #define BIGENDIFY32(X) __builtin_bswap32(X)
#elif defined(_MSC_VER)
  #define BIGENDIFY32(X) _byteswap_uint32(X)
#else
  #define BIGENDIFY32(X) bswap_32(X)
#endif
#endif
uint64_t bswap_64(uint64_t v) {
  uint64_t nv = 0;
  int bytes = 8;
  while (bytes--) {
    nv <<= 8;
    nv |= (v & 0xff);
    v >>= 8;
  }
  return nv;
}
uint32_t bswap_32(uint32_t v) {
  uint32_t nv = 0;
  int bytes = 4;
  // This while loop can be replaced with a single pshuffb command on intel.
  while (bytes--) {
    nv <<= 8;
    nv |= (v & 0xff);
    v >>= 8;
  }
  return nv;
}

void vp9_reader_fill(vp9_reader *r) {
  const uint8_t *const buffer_end = r->buffer_end;
  const uint8_t *buffer = r->buffer;
  const uint8_t *buffer_start = buffer;
  BD_VALUE value = r->value;
  int count = r->count;
  const size_t bytes_left = buffer_end - buffer;
  const size_t bits_left = bytes_left * CHAR_BIT;
  int shift = BD_VALUE_SIZE - CHAR_BIT - (count + CHAR_BIT);
  const int bits_over = (int)(shift + CHAR_BIT - bits_left);

  if (r->decrypt_cb) {
    size_t n = MIN(sizeof(r->clear_buffer), bytes_left);
    r->decrypt_cb(r->decrypt_state, buffer, r->clear_buffer, (int)n);
    buffer = r->clear_buffer;
    buffer_start = r->clear_buffer;
  }

  if (bits_over < 0 ) {
  #if UINTPTR_MAX == 0xffffffffffffffff
      BD_VALUE big_endian_values = BIGENDIFY64(*((const BD_VALUE *) buffer));
  #else
      BD_VALUE big_endian_values = BIGENDIFY32(*((const BD_VALUE *) buffer));
  #endif
      int bits = (shift & 0xfffffff8) + CHAR_BIT;
      BD_VALUE nv = big_endian_values >> (BD_VALUE_SIZE - bits);
      count += bits;
      buffer += (bits >> 3);
      value = r->value | (nv << (shift & 0x7));
  } else {
    count += LOTS_OF_BITS;
    while (shift >= bits_over) {
      count += CHAR_BIT;
      value |= (BD_VALUE)*buffer++ << shift;
      shift -= CHAR_BIT;
    }
  }

  // NOTE: Variable 'buffer' may not relate to 'r->buffer' after decryption,
  // so we increase 'r->buffer' by the amount that 'buffer' moved, rather than
  // assign 'buffer' to 'r->buffer'.
  r->buffer += buffer - buffer_start;
  r->value = value;
  r->count = count;
}

const uint8_t *vp9_reader_find_end(vp9_reader *r) {
  // Find the end of the coded buffer
  while (r->count > CHAR_BIT && r->count < BD_VALUE_SIZE) {
    r->count -= CHAR_BIT;
    r->buffer--;
  }
  return r->buffer;
}
