/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <smmintrin.h>  // SSE4.1

#include <assert.h>

#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"
#include "vpx_dsp/vpx_dsp_common.h"

#include "./vpx_dsp_rtcd.h"

#define MASK_BITS 6

static INLINE __m128i mm_loadl_32(const void *a) {
  return _mm_cvtsi32_si128(*(const uint32_t*)a);
}

static INLINE __m128i mm_loadl_64(const void *a) {
  return _mm_loadl_epi64((const __m128i*)a);
}

static INLINE __m128i mm_loadu_128(const void *a) {
  return _mm_loadu_si128((const __m128i*)a);
}

static INLINE void mm_storel_32(void *const a, const __m128i v) {
  *(uint32_t*)a = _mm_cvtsi128_si32(v);
}

static INLINE void mm_storel_64(void *const a, const __m128i v) {
  _mm_storel_epi64((__m128i*)a, v);
}

static INLINE void mm_storeu_128(void *const a, const __m128i v) {
  _mm_storeu_si128((__m128i*)a, v);
}

static INLINE __m128i mm_round_epu16(__m128i v_val_w, int bits) {
  const __m128i v_r_w = _mm_set1_epi16(1 << (bits - 1));
  const __m128i v_sum_w = _mm_add_epi16(v_val_w, v_r_w);
  return _mm_srli_epi16(v_sum_w, bits);
}

//////////////////////////////////////////////////////////////////////////////
// Common kernels
//////////////////////////////////////////////////////////////////////////////

static INLINE __m128i blend_4(uint8_t*src0, uint8_t *src1,
                              const __m128i v_m0_w, const __m128i v_m1_w) {
  const __m128i v_s0_b = mm_loadl_32(src0);
  const __m128i v_s1_b = mm_loadl_32(src1);
  const __m128i v_s0_w = _mm_cvtepu8_epi16(v_s0_b);
  const __m128i v_s1_w = _mm_cvtepu8_epi16(v_s1_b);

  const __m128i v_p0_w = _mm_mullo_epi16(v_s0_w, v_m0_w);
  const __m128i v_p1_w = _mm_mullo_epi16(v_s1_w, v_m1_w);

  const __m128i v_sum_w = _mm_add_epi16(v_p0_w, v_p1_w);

  const __m128i v_res_w = mm_round_epu16(v_sum_w, MASK_BITS);

  return v_res_w;
}

static INLINE __m128i blend_8(uint8_t*src0, uint8_t *src1,
                              const __m128i v_m0_w, const __m128i v_m1_w) {
  const __m128i v_s0_b = mm_loadl_64(src0);
  const __m128i v_s1_b = mm_loadl_64(src1);
  const __m128i v_s0_w = _mm_cvtepu8_epi16(v_s0_b);
  const __m128i v_s1_w = _mm_cvtepu8_epi16(v_s1_b);

  const __m128i v_p0_w = _mm_mullo_epi16(v_s0_w, v_m0_w);
  const __m128i v_p1_w = _mm_mullo_epi16(v_s1_w, v_m1_w);

  const __m128i v_sum_w = _mm_add_epi16(v_p0_w, v_p1_w);

  const __m128i v_res_w = mm_round_epu16(v_sum_w, MASK_BITS);

  return v_res_w;
}

//////////////////////////////////////////////////////////////////////////////
// No sub-sampling
//////////////////////////////////////////////////////////////////////////////

static void blend_mask6_w4_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_m0_b = mm_loadl_32(mask);
    const __m128i v_m0_w = _mm_cvtepu8_epi16(v_m0_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_4(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_32(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

static void blend_mask6_w8_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_m0_b = mm_loadl_64(mask);
    const __m128i v_m0_w = _mm_cvtepu8_epi16(v_m0_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_8(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_64(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

static void blend_mask6_w16n_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  do {
    int c;
    for (c = 0; c < w; c += 16) {
      const __m128i v_m0l_b = mm_loadl_64(mask + c);
      const __m128i v_m0h_b = mm_loadl_64(mask + c + 8);
      const __m128i v_m0l_w = _mm_cvtepu8_epi16(v_m0l_b);
      const __m128i v_m0h_w = _mm_cvtepu8_epi16(v_m0h_b);
      const __m128i v_m1l_w = _mm_sub_epi16(v_maxval_w, v_m0l_w);
      const __m128i v_m1h_w = _mm_sub_epi16(v_maxval_w, v_m0h_w);

      const __m128i v_resl_w = blend_8(src0 + c, src1 + c,
                                       v_m0l_w, v_m1l_w);
      const __m128i v_resh_w = blend_8(src0 + c + 8, src1 + c + 8,
                                       v_m0h_w, v_m1h_w);

      const __m128i v_res_b = _mm_packus_epi16(v_resl_w, v_resh_w);

      mm_storeu_128(dst + c, v_res_b);
    }
    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

//////////////////////////////////////////////////////////////////////////////
// Horizontal sub-sampling
//////////////////////////////////////////////////////////////////////////////

static void blend_mask6_sx_w4_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_r_b = mm_loadl_64(mask);
    const __m128i v_a_b = _mm_avg_epu8(v_r_b, _mm_bsrli_si128(v_r_b, 1));

    const __m128i v_m0_w = _mm_and_si128(v_a_b, v_zmask_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_4(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_32(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

static void blend_mask6_sx_w8_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_r_b = mm_loadu_128(mask);
    const __m128i v_a_b = _mm_avg_epu8(v_r_b, _mm_bsrli_si128(v_r_b, 1));

    const __m128i v_m0_w = _mm_and_si128(v_a_b, v_zmask_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_8(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_64(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

static void blend_mask6_sx_w16n_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  do {
    int c;
    for (c = 0; c < w; c += 16) {
      const __m128i v_rl_b = mm_loadu_128(mask + 2 * c);
      const __m128i v_rh_b = mm_loadu_128(mask + 2 * c + 16);
      const __m128i v_al_b = _mm_avg_epu8(v_rl_b, _mm_bsrli_si128(v_rl_b, 1));
      const __m128i v_ah_b = _mm_avg_epu8(v_rh_b, _mm_bsrli_si128(v_rh_b, 1));

      const __m128i v_m0l_w = _mm_and_si128(v_al_b, v_zmask_b);
      const __m128i v_m0h_w = _mm_and_si128(v_ah_b, v_zmask_b);
      const __m128i v_m1l_w = _mm_sub_epi16(v_maxval_w, v_m0l_w);
      const __m128i v_m1h_w = _mm_sub_epi16(v_maxval_w, v_m0h_w);

      const __m128i v_resl_w = blend_8(src0 + c, src1 + c,
                                       v_m0l_w, v_m1l_w);
      const __m128i v_resh_w = blend_8(src0 + c + 8, src1 + c + 8,
                                       v_m0h_w, v_m1h_w);

      const __m128i v_res_b = _mm_packus_epi16(v_resl_w, v_resh_w);

      mm_storeu_128(dst + c, v_res_b);
    }
    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += mask_stride;
  } while (--h);
}

//////////////////////////////////////////////////////////////////////////////
// Vertical sub-sampling
//////////////////////////////////////////////////////////////////////////////

static void blend_mask6_sy_w4_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_ra_b = mm_loadl_32(mask);
    const __m128i v_rb_b = mm_loadl_32(mask + mask_stride);
    const __m128i v_a_b = _mm_avg_epu8(v_ra_b, v_rb_b);

    const __m128i v_m0_w = _mm_cvtepu8_epi16(v_a_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_4(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_32(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

static void blend_mask6_sy_w8_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_ra_b = mm_loadl_64(mask);
    const __m128i v_rb_b = mm_loadl_64(mask + mask_stride);
    const __m128i v_a_b = _mm_avg_epu8(v_ra_b, v_rb_b);

    const __m128i v_m0_w = _mm_cvtepu8_epi16(v_a_b);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_8(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_64(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

static void blend_mask6_sy_w16n_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zero = _mm_setzero_si128();
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  do {
    int c;
    for (c = 0; c < w; c += 16) {
      const __m128i v_ra_b = mm_loadu_128(mask + c);
      const __m128i v_rb_b = mm_loadu_128(mask + c + mask_stride);
      const __m128i v_a_b = _mm_avg_epu8(v_ra_b, v_rb_b);

      const __m128i v_m0l_w = _mm_cvtepu8_epi16(v_a_b);
      const __m128i v_m0h_w = _mm_unpackhi_epi8(v_a_b, v_zero);
      const __m128i v_m1l_w = _mm_sub_epi16(v_maxval_w, v_m0l_w);
      const __m128i v_m1h_w = _mm_sub_epi16(v_maxval_w, v_m0h_w);

      const __m128i v_resl_w = blend_8(src0 + c, src1 + c,
                                       v_m0l_w, v_m1l_w);
      const __m128i v_resh_w = blend_8(src0 + c + 8, src1 + c + 8,
                                       v_m0h_w, v_m1h_w);

      const __m128i v_res_b = _mm_packus_epi16(v_resl_w, v_resh_w);

      mm_storeu_128(dst + c, v_res_b);
    }
    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

//////////////////////////////////////////////////////////////////////////////
// Horizontal and Vertical sub-sampling
//////////////////////////////////////////////////////////////////////////////

static void blend_mask6_sx_sy_w4_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_ra_b = mm_loadl_64(mask);
    const __m128i v_rb_b = mm_loadl_64(mask + mask_stride);
    const __m128i v_rvs_b = _mm_add_epi8(v_ra_b, v_rb_b);
    const __m128i v_rvsa_w = _mm_and_si128(v_rvs_b, v_zmask_b);
    const __m128i v_rvsb_w = _mm_and_si128(_mm_bsrli_si128(v_rvs_b, 1),
                                           v_zmask_b);
    const __m128i v_rs_w = _mm_add_epi16(v_rvsa_w, v_rvsb_w);

    const __m128i v_m0_w = mm_round_epu16(v_rs_w, 2);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_4(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_32(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

static void blend_mask6_sx_sy_w8_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  (void)w;

  do {
    const __m128i v_ra_b = mm_loadu_128(mask);
    const __m128i v_rb_b = mm_loadu_128(mask + mask_stride);
    const __m128i v_rvs_b = _mm_add_epi8(v_ra_b, v_rb_b);
    const __m128i v_rvsa_w = _mm_and_si128(v_rvs_b, v_zmask_b);
    const __m128i v_rvsb_w = _mm_and_si128(_mm_bsrli_si128(v_rvs_b, 1),
                                           v_zmask_b);
    const __m128i v_rs_w = _mm_add_epi16(v_rvsa_w, v_rvsb_w);

    const __m128i v_m0_w = mm_round_epu16(v_rs_w, 2);
    const __m128i v_m1_w = _mm_sub_epi16(v_maxval_w, v_m0_w);

    const __m128i v_res_w = blend_8(src0, src1, v_m0_w, v_m1_w);

    const __m128i v_res_b = _mm_packus_epi16(v_res_w, v_res_w);

    mm_storel_64(dst, v_res_b);

    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

static void blend_mask6_sx_sy_w16n_sse4_1(
    uint8_t *dst, uint32_t dst_stride,
    uint8_t *src0, uint32_t src0_stride,
    uint8_t *src1, uint32_t src1_stride,
    const uint8_t *mask, uint32_t mask_stride,
    int h, int w) {
  const __m128i v_zmask_b = _mm_set_epi8(0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff,
                                         0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff);
  const __m128i v_maxval_w = _mm_set1_epi16(1 << MASK_BITS);

  do {
    int c;
    for (c = 0; c < w; c += 16) {
      const __m128i v_ral_b = mm_loadu_128(mask + 2 * c);
      const __m128i v_rah_b = mm_loadu_128(mask + 2 * c + 16);
      const __m128i v_rbl_b = mm_loadu_128(mask + mask_stride + 2 * c);
      const __m128i v_rbh_b = mm_loadu_128(mask + mask_stride + 2 * c + 16);
      const __m128i v_rvsl_b = _mm_add_epi8(v_ral_b, v_rbl_b);
      const __m128i v_rvsh_b = _mm_add_epi8(v_rah_b, v_rbh_b);
      const __m128i v_rvsal_w = _mm_and_si128(v_rvsl_b, v_zmask_b);
      const __m128i v_rvsah_w = _mm_and_si128(v_rvsh_b, v_zmask_b);
      const __m128i v_rvsbl_w = _mm_and_si128(_mm_bsrli_si128(v_rvsl_b, 1),
                                              v_zmask_b);
      const __m128i v_rvsbh_w = _mm_and_si128(_mm_bsrli_si128(v_rvsh_b, 1),
                                              v_zmask_b);
      const __m128i v_rsl_w = _mm_add_epi16(v_rvsal_w, v_rvsbl_w);
      const __m128i v_rsh_w = _mm_add_epi16(v_rvsah_w, v_rvsbh_w);

      const __m128i v_m0l_w = mm_round_epu16(v_rsl_w, 2);
      const __m128i v_m0h_w = mm_round_epu16(v_rsh_w, 2);
      const __m128i v_m1l_w = _mm_sub_epi16(v_maxval_w, v_m0l_w);
      const __m128i v_m1h_w = _mm_sub_epi16(v_maxval_w, v_m0h_w);

      const __m128i v_resl_w = blend_8(src0 + c, src1 + c,
                                       v_m0l_w, v_m1l_w);
      const __m128i v_resh_w = blend_8(src0 + c + 8, src1 + c + 8,
                                       v_m0h_w, v_m1h_w);

      const __m128i v_res_b = _mm_packus_epi16(v_resl_w, v_resh_w);

      mm_storeu_128(dst + c, v_res_b);
    }
    dst += dst_stride;
    src0 += src0_stride;
    src1 += src1_stride;
    mask += 2 * mask_stride;
  } while (--h);
}

//////////////////////////////////////////////////////////////////////////////
// Dispatch
//////////////////////////////////////////////////////////////////////////////

void vpx_blend_mask6_sse4_1(uint8_t *dst, uint32_t dst_stride,
                            uint8_t *src0, uint32_t src0_stride,
                            uint8_t *src1, uint32_t src1_stride,
                            const uint8_t *mask, uint32_t mask_stride,
                            int h, int w, int suby, int subx) {
  typedef  void (*blend_fn)(uint8_t *dst, uint32_t dst_stride,
                            uint8_t *src0, uint32_t src0_stride,
                            uint8_t *src1, uint32_t src1_stride,
                            const uint8_t *mask, uint32_t mask_stride,
                            int h, int w);

  static blend_fn blend[3][2][2] = {  // width_index X subx X suby
    {     // w % 16 == 0
      {blend_mask6_w16n_sse4_1, blend_mask6_sy_w16n_sse4_1},
      {blend_mask6_sx_w16n_sse4_1, blend_mask6_sx_sy_w16n_sse4_1}
    }, {  // w == 4
      {blend_mask6_w4_sse4_1, blend_mask6_sy_w4_sse4_1},
      {blend_mask6_sx_w4_sse4_1, blend_mask6_sx_sy_w4_sse4_1}
    }, {  // w == 8
      {blend_mask6_w8_sse4_1, blend_mask6_sy_w8_sse4_1},
      {blend_mask6_sx_w8_sse4_1, blend_mask6_sx_sy_w8_sse4_1}
    }
  };

  assert(IMPLIES(src0 == dst, src0_stride == dst_stride));
  assert(IMPLIES(src1 == dst, src1_stride == dst_stride));

  assert(h >= 4);
  assert(w >= 4);
  assert(IS_POWER_OF_TWO(h));
  assert(IS_POWER_OF_TWO(w));

  blend[(w >> 2) & 3][subx != 0][suby != 0](dst, dst_stride,
                                            src0, src0_stride,
                                            src1, src1_stride,
                                            mask, mask_stride,
                                            h, w);
}
