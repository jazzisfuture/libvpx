/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <immintrin.h>

#include "./vpx_config.h"
#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"

#include "vpx_dsp/x86/synonyms.h"
#include "vpx_dsp/vpx_filter.h"

////////////////////////////////////////////////////////////////////////////////
// 8 bit
////////////////////////////////////////////////////////////////////////////////

static INLINE void obmc_variance_w4(const uint8_t *a,
                                    const int a_stride,
                                    const int32_t *b,
                                    const int32_t *m,
                                    unsigned int *const sse,
                                    int *const sum,
                                    const int height) {
  const int a_step = a_stride - 4;
  int n = 0;
  __m128i v_sum_d = _mm_setzero_si128();
  __m128i v_sse_d = _mm_setzero_si128();

  do {
    const __m128i v_a_b = xx_loadl_32(a + n);
    const __m128i v_m_d = xx_load_128(m + n);
    const __m128i v_b_d = xx_load_128(b + n);

    const __m128i v_a_d = _mm_cvtepu8_epi32(v_a_b);

    // Values in both a and m fit in 15 bits, and are packed at 32 bit
    // boundaries. We use pmaddwd, as it has lower latency on Haswell
    // than pmulld but produces the same result with these inputs.
    const __m128i v_am_d = _mm_madd_epi16(v_a_d, v_m_d);

    const __m128i v_diff_d = _mm_sub_epi32(v_b_d, v_am_d);
    const __m128i v_rdiff_d = xx_roundn_epi32(v_diff_d, 12);
    const __m128i v_sqrdiff_d = _mm_mullo_epi32(v_rdiff_d, v_rdiff_d);

    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff_d);
    v_sse_d = _mm_add_epi32(v_sse_d, v_sqrdiff_d);

    n += 4;

    if (n % 4 == 0)
      a += a_step;
  } while (n < 4 * height);

  *sum = xx_hsum_epi32_si32(v_sum_d);
  *sse = xx_hsum_epi32_si32(v_sse_d);
}

static INLINE void obmc_variance_w8n(const uint8_t *a,
                                     const int a_stride,
                                     const int32_t *b,
                                     const int32_t *m,
                                     unsigned int *const sse,
                                     int *const sum,
                                     const int width,
                                     const int height) {
  const int a_step = a_stride - width;
  int n = 0;
  __m128i v_sum_d = _mm_setzero_si128();
  __m128i v_sse_d = _mm_setzero_si128();

  do {
    const __m128i v_a1_b = xx_loadl_32(a + n + 4);
    const __m128i v_m1_d = xx_load_128(m + n + 4);
    const __m128i v_b1_d = xx_load_128(b + n + 4);
    const __m128i v_a0_b = xx_loadl_32(a + n);
    const __m128i v_m0_d = xx_load_128(m + n);
    const __m128i v_b0_d = xx_load_128(b + n);

    const __m128i v_a0_d = _mm_cvtepu8_epi32(v_a0_b);
    const __m128i v_a1_d = _mm_cvtepu8_epi32(v_a1_b);

    // Values in both a and m fit in 15 bits, and are packed at 32 bit
    // boundaries. We use pmaddwd, as it has lower latency on Haswell
    // than pmulld but produces the same result with these inputs.
    const __m128i v_am0_d = _mm_madd_epi16(v_a0_d, v_m0_d);
    const __m128i v_am1_d = _mm_madd_epi16(v_a1_d, v_m1_d);

    const __m128i v_diff0_d = _mm_sub_epi32(v_b0_d, v_am0_d);
    const __m128i v_diff1_d = _mm_sub_epi32(v_b1_d, v_am1_d);

    const __m128i v_rdiff0_d = xx_roundn_epi32(v_diff0_d, 12);
    const __m128i v_rdiff1_d = xx_roundn_epi32(v_diff1_d, 12);
    const __m128i v_rdiff01_w = _mm_packs_epi32(v_rdiff0_d, v_rdiff1_d);
    const __m128i v_sqrdiff_d = _mm_madd_epi16(v_rdiff01_w, v_rdiff01_w);

    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff0_d);
    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff1_d);
    v_sse_d = _mm_add_epi32(v_sse_d, v_sqrdiff_d);

    n += 8;

    if (n % width == 0)
      a += a_step;
  } while (n < width * height);

  *sum = xx_hsum_epi32_si32(v_sum_d);
  *sse = xx_hsum_epi32_si32(v_sse_d);
}

#define OBMCVARWXH(W, H)                                                      \
unsigned int vpx_obmc_variance##W##x##H##_sse4_1(const uint8_t *a,            \
                                                 int a_stride,                \
                                                 const int32_t *b,            \
                                                 const int32_t *m,            \
                                                 unsigned int *sse) {         \
  int sum;                                                                    \
  if (W == 4)                                                                 \
    obmc_variance_w4(a, a_stride, b, m, sse, &sum, H);                        \
  else                                                                        \
    obmc_variance_w8n(a, a_stride, b, m, sse, &sum, W, H);                    \
  return *sse - (((int64_t)sum * sum) / (W * H));                             \
}

#if CONFIG_EXT_PARTITION
OBMCVARWXH(128, 128)
OBMCVARWXH(128, 64)
OBMCVARWXH(64, 128)
#endif  // CONFIG_EXT_PARTITION
OBMCVARWXH(64, 64)
OBMCVARWXH(64, 32)
OBMCVARWXH(32, 64)
OBMCVARWXH(32, 32)
OBMCVARWXH(32, 16)
OBMCVARWXH(16, 32)
OBMCVARWXH(16, 16)
OBMCVARWXH(16, 8)
OBMCVARWXH(8, 16)
OBMCVARWXH(8, 8)
OBMCVARWXH(8, 4)
OBMCVARWXH(4, 8)
OBMCVARWXH(4, 4)

////////////////////////////////////////////////////////////////////////////////
// High bit-depth
////////////////////////////////////////////////////////////////////////////////

#if CONFIG_VP9_HIGHBITDEPTH
static INLINE void hbd_obmc_variance_w4(const uint8_t *a8,
                                        const int a_stride,
                                        const int32_t *b,
                                        const int32_t *m,
                                        uint64_t *const sse,
                                        int64_t *const sum,
                                        const int height) {
  const uint16_t *a = CONVERT_TO_SHORTPTR(a8);
  const int a_step = a_stride - 4;
  int n = 0;
  __m128i v_sum_d = _mm_setzero_si128();
  __m128i v_sse_d = _mm_setzero_si128();

  do {
    const __m128i v_a_w = xx_loadl_64(a + n);
    const __m128i v_m_d = xx_load_128(m + n);
    const __m128i v_b_d = xx_load_128(b + n);

    const __m128i v_a_d = _mm_cvtepu16_epi32(v_a_w);

    // Values in both a and m fit in 15 bits, and are packed at 32 bit
    // boundaries. We use pmaddwd, as it has lower latency on Haswell
    // than pmulld but produces the same result with these inputs.
    const __m128i v_am_d = _mm_madd_epi16(v_a_d, v_m_d);

    const __m128i v_diff_d = _mm_sub_epi32(v_b_d, v_am_d);
    const __m128i v_rdiff_d = xx_roundn_epi32(v_diff_d, 12);
    const __m128i v_sqrdiff_d = _mm_mullo_epi32(v_rdiff_d, v_rdiff_d);

    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff_d);
    v_sse_d = _mm_add_epi32(v_sse_d, v_sqrdiff_d);

    n += 4;

    if (n % 4 == 0)
      a += a_step;
  } while (n < 4 * height);

  *sum = xx_hsum_epi32_si32(v_sum_d);
  *sse = xx_hsum_epi32_si32(v_sse_d);
}

static INLINE void hbd_obmc_variance_w8n(const uint8_t *a8,
                                         const int a_stride,
                                         const int32_t *b,
                                         const int32_t *m,
                                         uint64_t *const sse,
                                         int64_t *const sum,
                                         const int width,
                                         const int height) {
  const uint16_t *a = CONVERT_TO_SHORTPTR(a8);
  const int a_step = a_stride - width;
  int n = 0;
  __m128i v_sum_d = _mm_setzero_si128();
  __m128i v_sse_d = _mm_setzero_si128();

  do {
    const __m128i v_a1_w = xx_loadl_64(a + n + 4);
    const __m128i v_m1_d = xx_load_128(m + n + 4);
    const __m128i v_b1_d = xx_load_128(b + n + 4);
    const __m128i v_a0_w = xx_loadl_64(a + n);
    const __m128i v_m0_d = xx_load_128(m + n);
    const __m128i v_b0_d = xx_load_128(b + n);

    const __m128i v_a0_d = _mm_cvtepu16_epi32(v_a0_w);
    const __m128i v_a1_d = _mm_cvtepu16_epi32(v_a1_w);

    // Values in both a and m fit in 15 bits, and are packed at 32 bit
    // boundaries. We use pmaddwd, as it has lower latency on Haswell
    // than pmulld but produces the same result with these inputs.
    const __m128i v_am0_d = _mm_madd_epi16(v_a0_d, v_m0_d);
    const __m128i v_am1_d = _mm_madd_epi16(v_a1_d, v_m1_d);

    const __m128i v_diff0_d = _mm_sub_epi32(v_b0_d, v_am0_d);
    const __m128i v_diff1_d = _mm_sub_epi32(v_b1_d, v_am1_d);

    const __m128i v_rdiff0_d = xx_roundn_epi32(v_diff0_d, 12);
    const __m128i v_rdiff1_d = xx_roundn_epi32(v_diff1_d, 12);
    const __m128i v_rdiff01_w = _mm_packs_epi32(v_rdiff0_d, v_rdiff1_d);
    const __m128i v_sqrdiff_d = _mm_madd_epi16(v_rdiff01_w, v_rdiff01_w);

    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff0_d);
    v_sum_d = _mm_add_epi32(v_sum_d, v_rdiff1_d);
    v_sse_d = _mm_add_epi32(v_sse_d, v_sqrdiff_d);

    n += 8;

    if (n % width == 0)
      a += a_step;
  } while (n < width * height);

  *sum += xx_hsum_epi32_si64(v_sum_d);
  *sse += xx_hsum_epi32_si64(v_sse_d);
}

static INLINE void highbd_obmc_variance(const uint8_t *a8, int  a_stride,
                                        const int32_t *b, const int32_t *m,
                                        int w, int h,
                                        unsigned int *sse, int *sum) {
  int64_t sum64 = 0;
  uint64_t sse64 = 0;
  if (w == 4)
    hbd_obmc_variance_w4(a8, a_stride, b, m, &sse64, &sum64, h);
  else
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, w, h);
  *sum = (int)sum64;
  *sse = (unsigned int)sse64;
}

static INLINE void highbd_10_obmc_variance(const uint8_t *a8, int  a_stride,
                                           const int32_t *b, const int32_t *m,
                                           int w, int h,
                                           unsigned int *sse, int *sum) {
  int64_t sum64 = 0;
  uint64_t sse64 = 0;
  if (w == 4)
    hbd_obmc_variance_w4(a8, a_stride, b, m, &sse64, &sum64, h);
  else
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, w, h);
  *sum = (int)ROUND_POWER_OF_TWO(sum64, 2);
  *sse = (unsigned int)ROUND_POWER_OF_TWO(sse64, 4);
}

static INLINE void highbd_12_obmc_variance(const uint8_t *a8, int  a_stride,
                                           const int32_t *b, const int32_t *m,
                                           int w, int h,
                                           unsigned int *sse, int *sum) {
  int64_t sum64 = 0;
  uint64_t sse64 = 0;
  if (w == 128 && h == 128) {
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
    a8 += 32 * a_stride; b += 32 * 128; m += 32 * 128;
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
    a8 += 32 * a_stride; b += 32 * 128; m += 32 * 128;
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
    a8 += 32 * a_stride; b += 32 * 128; m += 32 * 128;
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
  } else if (w == 128 && h == 64) {
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
    a8 += 32 * a_stride; b += 32 * 128; m += 32 * 128;
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 128, 32);
  } else if (w == 64 && h == 128) {
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 64, 64);
    a8 += 64 * a_stride; b += 64 * 64; m += 64 * 64;
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, 64, 64);
  } else if (w == 4) {
    hbd_obmc_variance_w4(a8, a_stride, b, m, &sse64, &sum64, h);
  } else {
    hbd_obmc_variance_w8n(a8, a_stride, b, m, &sse64, &sum64, w, h);
  }
  *sum = (int)ROUND_POWER_OF_TWO(sum64, 4);
  *sse = (unsigned int)ROUND_POWER_OF_TWO(sse64, 8);
}

#define HBD_OBMCVARWXH(W, H)                                                  \
unsigned int vpx_highbd_obmc_variance##W##x##H##_sse4_1(                      \
    const uint8_t *a,                                                         \
    int a_stride,                                                             \
    const int32_t *b,                                                         \
    const int32_t *m,                                                         \
    unsigned int *sse) {                                                      \
  int sum;                                                                    \
  highbd_obmc_variance(a, a_stride, b, m, W, H, sse, &sum);                   \
  return *sse - (((int64_t)sum * sum) / (W * H));                             \
}                                                                             \
                                                                              \
unsigned int vpx_highbd_10_obmc_variance##W##x##H##_sse4_1(                   \
    const uint8_t *a,                                                         \
    int a_stride,                                                             \
    const int32_t *b,                                                         \
    const int32_t *m,                                                         \
    unsigned int *sse) {                                                      \
  int sum;                                                                    \
  highbd_10_obmc_variance(a, a_stride, b, m, W, H, sse, &sum);                \
  return *sse - (((int64_t)sum * sum) / (W * H));                             \
}                                                                             \
                                                                              \
unsigned int vpx_highbd_12_obmc_variance##W##x##H##_sse4_1(                   \
    const uint8_t *a,                                                         \
    int a_stride,                                                             \
    const int32_t *b,                                                         \
    const int32_t *m,                                                         \
    unsigned int *sse) {                                                      \
  int sum;                                                                    \
  highbd_12_obmc_variance(a, a_stride, b, m, W, H, sse, &sum);                \
  return *sse - (((int64_t)sum * sum) / (W * H));                             \
}

#if CONFIG_EXT_PARTITION
HBD_OBMCVARWXH(128, 128)
HBD_OBMCVARWXH(128, 64)
HBD_OBMCVARWXH(64, 128)
#endif  // CONFIG_EXT_PARTITION
HBD_OBMCVARWXH(64, 64)
HBD_OBMCVARWXH(64, 32)
HBD_OBMCVARWXH(32, 64)
HBD_OBMCVARWXH(32, 32)
HBD_OBMCVARWXH(32, 16)
HBD_OBMCVARWXH(16, 32)
HBD_OBMCVARWXH(16, 16)
HBD_OBMCVARWXH(16, 8)
HBD_OBMCVARWXH(8, 16)
HBD_OBMCVARWXH(8, 8)
HBD_OBMCVARWXH(8, 4)
HBD_OBMCVARWXH(4, 8)
HBD_OBMCVARWXH(4, 4)
#endif  // CONFIG_VP9_HIGHBITDEPTH
