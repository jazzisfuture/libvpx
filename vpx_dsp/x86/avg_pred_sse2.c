/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

void vpx_comp_avg_pred_sse2(uint8_t *comp, const uint8_t *pred, int width,
                            int height, const uint8_t *ref, int ref_stride) {
  int w, h;
  for (h = 0; h < height; ++h) {
    // In use, comp is over specified (always 64x64) so over-writing on the 4x4
    // and 8x8 (and 8x4, 4x8, 8x16) is fine. Supporting small sizes does require
    // using loadu/storeu unfortunately.
    for (w = 0; w < width; w += 16) {
      const __m128i p = _mm_loadu_si128((const __m128i *)(pred + w));
      const __m128i r = _mm_loadu_si128((const __m128i *)(ref + w));
      const __m128i avg = _mm_avg_epu8(p, r);
      _mm_storeu_si128((__m128i *)(comp + w), avg);
    }
    comp += width;
    pred += width;
    ref += ref_stride;
  }
}
