/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdint.h>

void box_filter(const uint8_t *src, int src_stride,
                uint8_t *dst, int dst_stride) {
  int h, w;
  const int width = 16, height = 16;

  for (h = 0; h < height; h++) {
    for (w = 0; w < width; w++) {
      // non-local mean approach
      int sum = 0;
      int idx, idy;
      const uint8_t *center_pixel = src + h * src_stride + w;

      for (idy = -1; idy <= 1; ++idy) {
        for (idx = -1; idx <= 1; ++idx) {
          int row = h + idy;
          int col = w + idx;

          if (row >= 0 && row < height && col >= 0 && col < width) {
            sum += center_pixel[idy * src_stride + idx];
          } else {
            // intent is to pad out the borders ...
            sum += *src;
          }
        }
      }

      // ... so that this is always 9
      // maybe there should be rounding?
      sum /= 9;

      dst[h * dst_stride + w] = sum;
    }
  }
}
