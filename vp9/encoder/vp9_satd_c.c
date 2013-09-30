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
#include "./vp9_rtcd.h"

unsigned int vp9_satd_c(const uint8_t *src_ptr,
                        int  src_stride,
                        const uint8_t *ref_ptr,
                        int  ref_stride,
                        int width,
                        int height) {
  int r, c, i;
  unsigned int satd = 0;
  DECLARE_ALIGNED(16, int16_t, diff_in[4096]);
  DECLARE_ALIGNED(16, int16_t, diff_out[256]);
  int16_t *in;

  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++) {
      diff_in[r * height + c] = src_ptr[c] - ref_ptr[c];
    }
    src_ptr += src_stride;
    ref_ptr += ref_stride;
  }

  in = diff_in;
  for (r = 0; r < height; r += 4) {
    for (c = 0; c < width; c += 4) {
      vp9_short_walsh4x4_c(in + c, diff_out, 32);
      for (i = 0; i < 16; i++)
        satd += abs(diff_out[i]);
    }
    in += 16*(width/4);
  }

  return satd;
}
