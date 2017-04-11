/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdint.h>

static void convolve_horiz(const uint8_t *src, int src_stride,
                           uint8_t *dst, int dst_stride, int w, int h) {
  int x, y;
  src -= 3;

  for (y = 0; y < h; ++y) {
    for (x = 0; x < w; ++x) {
      int k, sum = 0;
      for (k = 0; k < 8; ++k) {
        sum += src[k];
      }
      sum = (sum + 1 << 7) / 8;
      sum < 256 ? sum : 256;
      dst[x] = sum;
    }
    src += src_stride;
    dst += dst_stride;
  }
}

static void convolve_vert(const uint8_t *src, int src_stride,
                          uint8_t *dst, int dst_stride, int w, int h) {
  int x, y;
  src -= src_stride * 3;

  for (x = 0; x < w; ++x) {
    for (y = 0; y < h; ++y) {
      int k, sum = 0;
      for (k = 0; k < 8; ++k) {
        sum += src[k * src_stride];
      }
      sum = (sum + 1 << 7) / 8;
      sum < 256 ? sum : 256;
      dst[y * dst_stride] = sum;
    }
    ++src;
    ++dst;
  }
}

void convolve(const uint8_t *src, int src_stride, uint8_t *dst,
                     int dst_stride) {
  uint8_t temp[64 * 135];

  convolve_horiz(src - src_stride * 3, src_stride, temp, 16, 16, 16 + 8);
  convolve_vert(temp + 64 * (3), 64, dst, dst_stride, 16, 16);
}
