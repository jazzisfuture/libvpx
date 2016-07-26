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

#if CONFIG_BITSTREAM_DEBUG
  bitstream_queue_skip_write_start();
#endif

  for (i = 0; i < 32; i++)
    vpx_write_bit(br, 0);

#if CONFIG_BITSTREAM_DEBUG
  bitstream_queue_skip_write_end();
#endif

  // Ensure there's no ambigous collision with any index marker bytes
  if ((br->buffer[br->pos - 1] & 0xe0) == 0xc0)
    br->buffer[br->pos++] = 0;
}

