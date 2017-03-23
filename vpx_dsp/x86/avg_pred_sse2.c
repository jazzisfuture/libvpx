/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
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
  if (width > 8) {
    int w, h;
    for (h = 0; h < height; ++h) {
      for (w = 0; w < width; w += 16) {
        const __m128i p = _mm_load_si128((const __m128i *)(pred + w));
        const __m128i r = _mm_loadu_si128((const __m128i *)(ref + w));
        const __m128i avg = _mm_avg_epu8(p, r);
        _mm_store_si128((__m128i *)(comp + w), avg);
      }
      comp += width;
      pred += width;
      ref += ref_stride;
    }
  } else {  // width must be 4 or 8.
    int i;
    // Process 16 elements at a time. comp and pred have width == stride and
    // therefore live in contigious memory. 4*4, 4*8, 8*4, 8*8, and 8*16 are all
    // divisible by 16 so just ref needs to be massaged when loading.
    const int elements = width * height;
    for (i = 0; i < elements; i += 16) {
      const __m128i p = _mm_load_si128((const __m128i *)pred);
      __m128i r;
      __m128i avg;
      if (width == ref_stride) {
        r = _mm_loadu_si128((const __m128i *)ref);
        ref += 16;
      } else if (width == 4) {
        const __m128i r_0 = _mm_loadl_epi64((const __m128i *)ref);
        const __m128i r_0_2 = (__m128i)_mm_loadh_pi(
            (__m128)r_0, (const __m64 *)(ref + 2 * ref_stride));
        const __m128i r_1 =
            _mm_loadl_epi64((const __m128i *)(ref + ref_stride));
        const __m128i r_1_3 = (__m128i)_mm_loadh_pi(
            (__m128)r_1, (const __m64 *)(ref + 3 * ref_stride));

        r = _mm_slli_epi64(r_1_3, 32);
        r = _mm_or_si128(r, _mm_srli_epi64(_mm_slli_epi64(r_0_2, 32), 32));

        ref += 4 * ref_stride;
      } else {  // width must be 8.
        const __m128i r_0 = _mm_loadl_epi64((const __m128i *)ref);
        r = (__m128i)_mm_loadh_pi((__m128)r_0,
                                  (const __m64 *)(ref + ref_stride));

        ref += 2 * ref_stride;
      }
      avg = _mm_avg_epu8(p, r);
      _mm_store_si128((__m128i *)comp, avg);

      pred += 16;
      comp += 16;
    }
  }
}
