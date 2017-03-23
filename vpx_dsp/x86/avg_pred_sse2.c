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
#include <string.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

void vpx_comp_avg_pred_sse2(uint8_t *comp, const uint8_t *pred, int width,
                            int height, const uint8_t *ref, int ref_stride) {
  if (width > 8) {
    int w, h;
    for (h = 0; h < height; ++h) {
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
  } else {  // width must be 4 or 8.
    int elements = width * height;
    int i;
    // Process 16 elements at a time. comp and pred have width == stride and
    // therefore live in contigious memory. 4*4, 4*8, 8*4, 8*8, and 8*16 are all
    // divisible by 16 so just ref needs to be massaged when loading.
    for (i = 0; i < elements; i += 16) {
      const __m128i p = _mm_loadu_si128((const __m128i *)pred);
      __m128i r;
      __m128i avg;
      if (width == 4) {
        uint8_t r_concat[16];
        memcpy(r_concat, ref, 4);
        memcpy(r_concat + 4, ref + ref_stride, 4);
        memcpy(r_concat + 8, ref + 2 * ref_stride, 4);
        memcpy(r_concat + 12, ref + 3 * ref_stride, 4);

        r = _mm_loadu_si128((const __m128i *)r_concat);

        ref += 4 * ref_stride;
      } else {  // width must be 8.
        const __m128i r_0 = _mm_loadl_epi64((const __m128i *)ref);
        const __m128i r_1 =
            _mm_loadl_epi64((const __m128i *)(ref + ref_stride));

        r = _mm_or_si128(r_0, _mm_slli_si128(r_1, 8));

        ref += 2 * ref_stride;
      }
      avg = _mm_avg_epu8(p, r);
      _mm_storeu_si128((__m128i *)comp, avg);

      pred += 16;
      comp += 16;
    }
  }
}
