/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <smmintrin.h> /* SSE4.1 */

#include "./vp8_rtcd.h"
#include "vp8/common/entropy.h" /* vp8_default_inv_zig_zag */
#include "vp8/encoder/block.h"

// use GNU builtins where available.
#if defined(__GNUC__) && \
    ((__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || __GNUC__ >= 4)
static INLINE int get_lsb(int n) {
  return __builtin_ctz(n);
}
#elif defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
static INLINE int get_lsb(int n) {
  unsigned long first_set_bit;  // NOLINT(runtime/int)
  _BitScanForward(&first_set_bit, n);
  return first_set_bit;
}
#else
static INLINE int get_lsb(int n) {
  int i;
  for (i = 0; i < 32 && !(n & 1); ++i) n >>= 1;
  return i;
}
#endif

void vp8_regular_quantize_b_sse4_1(BLOCK *b, BLOCKD *d) {
  int eob = -1;
  char *zbin_boost_ptr = (char *)b->zrun_zbin_boost;
  __m128i zbin_boost0 = _mm_load_si128((__m128i *)zbin_boost_ptr);
  __m128i zbin_boost1 = _mm_load_si128((__m128i *)zbin_boost_ptr + 1);
  const __m128i idx0 =
      _mm_setr_epi8(2, 3, 4, 5, 10, 11, 0, 1, 12, 13, 6, 7, 8, 9, 14, 15);
  const __m128i idx1 =
      _mm_setr_epi8(0, 1, 6, 7, 8, 9, 2, 3, 14, 15, 4, 5, 10, 11, 12, 13);
  DECLARE_ALIGNED(16, static const uint8_t,
                  zig_zag_mask[16]) = { 0, 1,  4,  8,  5, 2,  3,  6,
                                        9, 12, 13, 10, 7, 11, 14, 15 };
  DECLARE_ALIGNED(16, int16_t, qcoeff[16]) = { 0 };
  uint32_t mask, mask2;

  __m128i x0, x1, y0, y1, x_minus_zbin0, x_minus_zbin1, dqcoeff0, dqcoeff1;
  __m128i quant_shift0 = _mm_load_si128((__m128i *)(b->quant_shift));
  __m128i quant_shift1 = _mm_load_si128((__m128i *)(b->quant_shift + 8));
  __m128i z0 = _mm_load_si128((__m128i *)(b->coeff));
  __m128i z1 = _mm_load_si128((__m128i *)(b->coeff + 8));
  __m128i zbin_extra = _mm_cvtsi32_si128(b->zbin_extra);
  __m128i zbin0 = _mm_load_si128((__m128i *)(b->zbin));
  __m128i zbin1 = _mm_load_si128((__m128i *)(b->zbin + 8));
  __m128i round0 = _mm_load_si128((__m128i *)(b->round));
  __m128i round1 = _mm_load_si128((__m128i *)(b->round + 8));
  __m128i quant0 = _mm_load_si128((__m128i *)(b->quant));
  __m128i quant1 = _mm_load_si128((__m128i *)(b->quant + 8));
  __m128i dequant0 = _mm_load_si128((__m128i *)(d->dequant));
  __m128i dequant1 = _mm_load_si128((__m128i *)(d->dequant + 8));
  __m128i qcoeff0, qcoeff1, t0, t1, x_shuf0, x_shuf1;

  /* Duplicate to all lanes. */
  zbin_extra = _mm_shufflelo_epi16(zbin_extra, 0);
  zbin_extra = _mm_unpacklo_epi16(zbin_extra, zbin_extra);

  /* x = abs(z) */
  x0 = _mm_abs_epi16(z0);
  x1 = _mm_abs_epi16(z1);

  /* zbin[] + zbin_extra */
  zbin0 = _mm_add_epi16(zbin0, zbin_extra);
  zbin1 = _mm_add_epi16(zbin1, zbin_extra);

  /* In C x is compared to zbin where zbin = zbin[] + boost + extra. Rebalance
   * the equation because boost is the only value which can change:
   * x - (zbin[] + extra) >= boost */
  x_minus_zbin0 = _mm_sub_epi16(x0, zbin0);
  x_minus_zbin1 = _mm_sub_epi16(x1, zbin1);

  /* All the remaining calculations are valid whether they are done now with
   * simd or later inside the loop one at a time. */
  x0 = _mm_add_epi16(x0, round0);
  x1 = _mm_add_epi16(x1, round1);

  y0 = _mm_mulhi_epi16(x0, quant0);
  y1 = _mm_mulhi_epi16(x1, quant1);

  y0 = _mm_add_epi16(y0, x0);
  y1 = _mm_add_epi16(y1, x1);

  /* Instead of shifting each value independently we convert the scaling
   * factor with 1 << (16 - shift) so we can use multiply/return high half. */
  y0 = _mm_mulhi_epi16(y0, quant_shift0);
  y1 = _mm_mulhi_epi16(y1, quant_shift1);

  /* Restore the sign. */
  y0 = _mm_sign_epi16(y0, z0);
  y1 = _mm_sign_epi16(y1, z1);

  t0 = _mm_bslli_si128(x_minus_zbin0, 2);
  t1 = _mm_bsrli_si128(x_minus_zbin1, 2);
  t0 = _mm_blend_epi16(t0, x_minus_zbin1, 0x01);
  t1 = _mm_blend_epi16(t1, x_minus_zbin0, 0x80);
  x_shuf0 = _mm_shuffle_epi8(t0, idx0);
  x_shuf1 = _mm_shuffle_epi8(t1, idx1);

  t0 = _mm_packs_epi16(y0, y1);
  t0 = _mm_cmpeq_epi8(t0, _mm_setzero_si128());
  t0 = _mm_shuffle_epi8(t0, _mm_load_si128((const __m128i *)zig_zag_mask));
  mask2 = _mm_movemask_epi8(t0) ^ 0xffff;

  for (;;) {
    t0 = _mm_cmpgt_epi16(zbin_boost0, x_shuf0);
    t1 = _mm_cmpgt_epi16(zbin_boost1, x_shuf1);
    t0 = _mm_packs_epi16(t0, t1);
    mask = _mm_movemask_epi8(t0);
    mask = ~mask & mask2;
    if (!mask) break;
    eob = get_lsb(mask);
    mask2 &= ~1U << eob;
    /* It's safe to read ahead of this buffer if struct VP8_COMP has at
     * least 32 bytes before the zrun_zbin_boost_* fields (it has 384). */
    zbin_boost0 = _mm_loadu_si128((__m128i *)&zbin_boost_ptr[-eob * 2 - 2]);
    zbin_boost1 = _mm_loadu_si128((__m128i *)&zbin_boost_ptr[-eob * 2 + 14]);
    qcoeff[zig_zag_mask[eob]] = -1;
  }

  qcoeff0 = _mm_load_si128((__m128i *)(qcoeff));
  qcoeff1 = _mm_load_si128((__m128i *)(qcoeff + 8));
  qcoeff0 = _mm_and_si128(qcoeff0, y0);
  qcoeff1 = _mm_and_si128(qcoeff1, y1);

  _mm_store_si128((__m128i *)(d->qcoeff), qcoeff0);
  _mm_store_si128((__m128i *)(d->qcoeff + 8), qcoeff1);

  dqcoeff0 = _mm_mullo_epi16(qcoeff0, dequant0);
  dqcoeff1 = _mm_mullo_epi16(qcoeff1, dequant1);

  _mm_store_si128((__m128i *)(d->dqcoeff), dqcoeff0);
  _mm_store_si128((__m128i *)(d->dqcoeff + 8), dqcoeff1);

  *d->eob = eob + 1;
}
