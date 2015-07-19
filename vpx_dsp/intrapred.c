/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_dsp/intrapred.h"

#define DST(x, y) dst[(x) + (y) * stride]
#define AVG3(a, b, c) (((a) + 2 * (b) + (c) + 2) >> 2)
#define AVG2(a, b) (((a) + (b) + 1) >> 1)

INLINE void d207_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left) {
  int r, c;
  (void) above;
  // first column
  for (r = 0; r < bs - 1; ++r)
    dst[r * stride] = AVG2(left[r], left[r + 1]);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // second column
  for (r = 0; r < bs - 2; ++r)
    dst[r * stride] = AVG3(left[r], left[r + 1], left[r + 2]);
  dst[(bs - 2) * stride] = AVG3(left[bs - 2], left[bs - 1], left[bs - 1]);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // rest of last row
  for (c = 0; c < bs - 2; ++c)
    dst[(bs - 1) * stride + c] = left[bs - 1];

  for (r = bs - 2; r >= 0; --r)
    for (c = 0; c < bs - 2; ++c)
      dst[r * stride + c] = dst[(r + 1) * stride + c - 2];
}

INLINE void d63_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left) {
  int r, c;
  int size;
  (void)left;
  for (c = 0; c < bs; ++c) {
    dst[c] = AVG2(above[c], above[c + 1]);
    dst[stride + c] = AVG3(above[c], above[c + 1], above[c + 2]);
  }
  for (r = 2, size = bs - 2; r < bs; r += 2, --size) {
    memcpy(dst + (r + 0) * stride, dst + (r >> 1), size);
    memset(dst + (r + 0) * stride + size, above[bs - 1], bs - size);
    memcpy(dst + (r + 1) * stride, dst + stride + (r >> 1), size);
    memset(dst + (r + 1) * stride + size, above[bs - 1], bs - size);
  }
}

INLINE void d45_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left) {
  const uint8_t above_right = above[bs - 1];
  const uint8_t *const dst_row0 = dst;
  int x, size;
  (void)left;

  for (x = 0; x < bs - 1; ++x) {
    dst[x] = AVG3(above[x], above[x + 1], above[x + 2]);
  }
  dst[bs - 1] = above_right;
  dst += stride;
  for (x = 1, size = bs - 2; x < bs; ++x, --size) {
    memcpy(dst, dst_row0 + x, size);
    memset(dst + size, above_right, x + 1);
    dst += stride;
  }
}

INLINE void d117_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left) {
  int r, c;

  // first row
  for (c = 0; c < bs; c++)
    dst[c] = AVG2(above[c - 1], above[c]);
  dst += stride;

  // second row
  dst[0] = AVG3(left[0], above[-1], above[0]);
  for (c = 1; c < bs; c++)
    dst[c] = AVG3(above[c - 2], above[c - 1], above[c]);
  dst += stride;

  // the rest of first col
  dst[0] = AVG3(above[-1], left[0], left[1]);
  for (r = 3; r < bs; ++r)
    dst[(r - 2) * stride] = AVG3(left[r - 3], left[r - 2], left[r - 1]);

  // the rest of the block
  for (r = 2; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-2 * stride + c - 1];
    dst += stride;
  }
}

INLINE void d135_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left) {
  int r, c;
  dst[0] = AVG3(left[0], above[-1], above[0]);
  for (c = 1; c < bs; c++)
    dst[c] = AVG3(above[c - 2], above[c - 1], above[c]);

  dst[stride] = AVG3(above[-1], left[0], left[1]);
  for (r = 2; r < bs; ++r)
    dst[r * stride] = AVG3(left[r - 2], left[r - 1], left[r]);

  dst += stride;
  for (r = 1; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-stride + c - 1];
    dst += stride;
  }
}

INLINE void d153_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left) {
  int r, c;
  dst[0] = AVG2(above[-1], left[0]);
  for (r = 1; r < bs; r++)
    dst[r * stride] = AVG2(left[r - 1], left[r]);
  dst++;

  dst[0] = AVG3(left[0], above[-1], above[0]);
  dst[stride] = AVG3(above[-1], left[0], left[1]);
  for (r = 2; r < bs; r++)
    dst[r * stride] = AVG3(left[r - 2], left[r - 1], left[r]);
  dst++;

  for (c = 0; c < bs - 2; c++)
    dst[c] = AVG3(above[c - 1], above[c], above[c + 1]);
  dst += stride;

  for (r = 1; r < bs; ++r) {
    for (c = 0; c < bs - 2; c++)
      dst[c] = dst[-stride + c - 2];
    dst += stride;
  }
}

INLINE void v_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left) {
  int r;
  (void) left;

  for (r = 0; r < bs; r++) {
    memcpy(dst, above, bs);
    dst += stride;
  }
}

INLINE void h_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left) {
  int r;
  (void) above;

  for (r = 0; r < bs; r++) {
    memset(dst, left[r], bs);
    dst += stride;
  }
}

INLINE void tm_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left) {
  int r, c;
  int ytop_left = above[-1];

  for (r = 0; r < bs; r++) {
    for (c = 0; c < bs; c++)
      dst[c] = clip_pixel(left[r] + above[c] - ytop_left);
    dst += stride;
  }
}

INLINE void dc_128_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left) {
  int r;
  (void) above;
  (void) left;

  for (r = 0; r < bs; r++) {
    memset(dst, 128, bs);
    dst += stride;
  }
}

INLINE void dc_left_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                              const uint8_t *above,
                              const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  (void) above;

  for (i = 0; i < bs; i++)
    sum += left[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    memset(dst, expected_dc, bs);
    dst += stride;
  }
}

INLINE void dc_top_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  (void) left;

  for (i = 0; i < bs; i++)
    sum += above[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    memset(dst, expected_dc, bs);
    dst += stride;
  }
}

INLINE void dc_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  const int count = 2 * bs;

  for (i = 0; i < bs; i++) {
    sum += above[i];
    sum += left[i];
  }

  expected_dc = (sum + (count >> 1)) / count;

  for (r = 0; r < bs; r++) {
    memset(dst, expected_dc, bs);
    dst += stride;
  }
}


