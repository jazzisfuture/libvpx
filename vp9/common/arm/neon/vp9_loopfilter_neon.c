/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vp9_rtcd.h"
#include "vp9/common/vp9_common.h"

void vp9_loop_filter_horizontal_edge_neon(uint8_t *s, int p /* pitch */,
                                          const uint8_t *blimit,
                                          const uint8_t *limit,
                                          const uint8_t *thresh,
                                          int count) {
  if (count == 2)
    vp9_loop_filter_horizontal_edge_neon_16(s, p, *blimit, *limit, *thresh);
  else
    vp9_loop_filter_horizontal_edge_neon_8(s, p, blimit, limit, thresh, count);
}
