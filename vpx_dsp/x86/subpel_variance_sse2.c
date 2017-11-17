/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>  // SSE2

#include "./vpx_dsp_rtcd.h"

#define SUBPEL_OPT 2
#include "vpx_dsp/x86/subpel_variance_x86.h"
#undef SUBPEL_OPT

#define SUBPIX_VAR(W, H)                                         \
  uint32_t vpx_sub_pixel_variance##W##x##H##_sse2(               \
      const uint8_t *a, int a_stride, int xoffset, int yoffset,  \
      const uint8_t *b, int b_stride, uint32_t *sse) {           \
    DECLARE_ALIGNED(16, uint8_t, c[(H + 1) * W]);                \
                                                                 \
    first_pass(a, a_stride, c, xoffset, W, H);                   \
    second_pass(c, yoffset, W, H);                               \
                                                                 \
    return vpx_variance##W##x##H##_sse2(c, W, b, b_stride, sse); \
  }

#define SUBPIX_AVG_VAR(W, H)                                     \
  uint32_t vpx_sub_pixel_avg_variance##W##x##H##_sse2(           \
      const uint8_t *a, int a_stride, int xoffset, int yoffset,  \
      const uint8_t *b, int b_stride, uint32_t *sse,             \
      const uint8_t *second_pred) {                              \
    DECLARE_ALIGNED(16, uint8_t, c[(H + 1) * W]);                \
                                                                 \
    first_pass(a, a_stride, c, xoffset, W, H);                   \
    second_pass(c, yoffset, W, H);                               \
                                                                 \
    avg_pred(c, second_pred, W, H);                              \
                                                                 \
    return vpx_variance##W##x##H##_sse2(c, W, b, b_stride, sse); \
  }

SUBPIX_VAR(4, 4)
SUBPIX_VAR(4, 8)
SUBPIX_VAR(8, 4)
SUBPIX_VAR(8, 8)
SUBPIX_VAR(8, 16)
SUBPIX_VAR(16, 8)
SUBPIX_VAR(16, 16)
SUBPIX_VAR(16, 32)
SUBPIX_VAR(32, 16)
SUBPIX_VAR(32, 32)
SUBPIX_VAR(32, 64)
SUBPIX_VAR(64, 32)
SUBPIX_VAR(64, 64)

SUBPIX_AVG_VAR(64, 64)
SUBPIX_AVG_VAR(64, 32)
SUBPIX_AVG_VAR(32, 64)
SUBPIX_AVG_VAR(32, 32)
SUBPIX_AVG_VAR(32, 16)
SUBPIX_AVG_VAR(16, 32)
SUBPIX_AVG_VAR(16, 16)
SUBPIX_AVG_VAR(16, 8)
SUBPIX_AVG_VAR(8, 16)
SUBPIX_AVG_VAR(8, 8)
SUBPIX_AVG_VAR(8, 4)
SUBPIX_AVG_VAR(4, 8)
SUBPIX_AVG_VAR(4, 4)
