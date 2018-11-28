/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_DSP_X86_QUANTIZE_SSSE3_H_
#define VPX_VPX_DSP_X86_QUANTIZE_SSSE3_H_

#include <emmintrin.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/x86/quantize_sse2.h"

static INLINE void calculate_dqcoeff_and_store_32x32(const __m128i qcoeff,
                                                     const __m128i dequant,
                                                     tran_low_t *dqcoeff) {
  const __m128i low = _mm_mullo_epi16(qcoeff, dequant);
  const __m128i high = _mm_mulhi_epi16(qcoeff, dequant);
  __m128i dqcoeff32_0 = _mm_unpacklo_epi16(low, high);
  __m128i dqcoeff32_1 = _mm_unpackhi_epi16(low, high);

  // Divide by 2.
  dqcoeff32_0 = _mm_srai_epi32(dqcoeff32_0, 1);
  dqcoeff32_1 = _mm_srai_epi32(dqcoeff32_1, 1);

#if CONFIG_VP9_HIGHBITDEPTH
  _mm_store_si128((__m128i *)(dqcoeff), dqcoeff32_0);
  _mm_store_si128((__m128i *)(dqcoeff + 4), dqcoeff32_1);
#else
  // Truncate 32 bit values to 16 bits. Match casting behavior in C.
  dqcoeff32_0 = _mm_shufflelo_epi16(dqcoeff32_0, _MM_SHUFFLE(0, 0, 2, 0));
  dqcoeff32_0 = _mm_shufflehi_epi16(dqcoeff32_0, _MM_SHUFFLE(0, 0, 2, 0));
  dqcoeff32_0 = _mm_shuffle_epi32(dqcoeff32_0, _MM_SHUFFLE(0, 0, 2, 0));

  dqcoeff32_1 = _mm_shufflelo_epi16(dqcoeff32_1, _MM_SHUFFLE(0, 0, 2, 0));
  dqcoeff32_1 = _mm_shufflehi_epi16(dqcoeff32_1, _MM_SHUFFLE(0, 0, 2, 0));
  dqcoeff32_1 = _mm_shuffle_epi32(dqcoeff32_1, _MM_SHUFFLE(2, 0, 2, 0));

  _mm_store_si128((__m128i *)(dqcoeff),
                  _mm_unpacklo_epi64(dqcoeff32_0, dqcoeff32_1));
#endif  // CONFIG_VP9_HIGHBITDEPTH
}

#endif  // VPX_VPX_DSP_X86_QUANTIZE_SSSE3_H_
