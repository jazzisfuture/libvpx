/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/ppc/types_vsx.h"

// Negate 16-bit integers in a when the corresponding signed 16-bit
// integer in b is negative.
static INLINE int16x8_t vec_sign(int16x8_t a, int16x8_t b) {
  const int16x8_t mask = vec_sra(b, vec_shift_sign_s16);
  return vec_xor(vec_add(a, mask), mask);
}

// Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit
// integers, and return the high 16 bits of the intermediate integers.
static INLINE int16x8_t vec_mulhi(int16x8_t a, int16x8_t b) {
  // madds does ((A * B) >>15) + C, we need >> 16, so we perform an extra right
  // shift.
  return vec_sra(vec_madds(a, b, vec_zeros_s16), vec_ones_s16);
}

static INLINE int16x8_t quantize_coeff(int16x8_t coeff, int16x8_t coeff_abs,
                                       int16x8_t round, int16x8_t quant,
                                       int16x8_t quant_shift, bool16x8_t mask) {
  int16x8_t rounded, qcoeff;
  rounded = vec_vaddshs(coeff_abs, round);
  qcoeff = vec_mulhi(rounded, quant);
  qcoeff = vec_add(qcoeff, rounded);
  qcoeff = vec_mulhi(qcoeff, quant_shift);
  qcoeff = vec_sign(qcoeff, coeff);
  return vec_and(qcoeff, mask);
}

static INLINE int16x8_t nonzero_scanindex(int16x8_t qcoeff0, bool16x8_t mask0,
                                          int16x8_t qcoeff1, bool16x8_t mask1,
                                          const int16_t *iscan_ptr) {
  bool16x8_t zero_coeff0, zero_coeff1;
  int16x8_t eob0, eob1;
  int16x8_t scan0 = vec_vsx_ld(0, iscan_ptr);
  int16x8_t scan1 = vec_vsx_ld(16, iscan_ptr);
  zero_coeff0 = vec_cmpeq(qcoeff0, vec_zeros_s16);
  zero_coeff1 = vec_cmpeq(qcoeff1, vec_zeros_s16);
  scan0 = vec_sub(scan0, mask0);
  scan1 = vec_sub(scan1, mask1);
  eob0 = vec_andc(scan0, zero_coeff0);
  eob1 = vec_andc(scan1, zero_coeff1);
  return vec_max(eob0, eob1);
}

// Compare packed 16-bit integers across a, and return the maximum value in
// every element. Returns a vector containing the biggest value across vector a.
static INLINE int16x8_t vec_max_across(int16x8_t a) {
  a = vec_max(a, vec_perm(a, a, vec_perm64));
  a = vec_max(a, vec_perm(a, a, vec_perm32));
  return vec_max(a, vec_perm(a, a, vec_perm16));
}

void vpx_quantize_b_vsx(const tran_low_t *coeff_ptr, intptr_t n_coeffs,
                        int skip_block, const int16_t *zbin_ptr,
                        const int16_t *round_ptr, const int16_t *quant_ptr,
                        const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr,
                        tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr,
                        uint16_t *eob_ptr, const int16_t *scan_ptr,
                        const int16_t *iscan_ptr) {
  int16x8_t qcoeff0, qcoeff1, dqcoeff0, dqcoeff1, eob;
  bool16x8_t zero_mask0, zero_mask1;

  // First set of 8 coeff starts with DC + 7 AC
  int16x8_t zbin = vec_vsx_ld(0, zbin_ptr);
  int16x8_t round = vec_vsx_ld(0, round_ptr);
  int16x8_t quant = vec_vsx_ld(0, quant_ptr);
  int16x8_t dequant = vec_vsx_ld(0, dequant_ptr);
  int16x8_t quant_shift = vec_vsx_ld(0, quant_shift_ptr);

  int16x8_t coeff0 = vec_vsx_ld(0, coeff_ptr);
  int16x8_t coeff1 = vec_vsx_ld(16, coeff_ptr);

  int16x8_t coeff0_abs = vec_abs(coeff0);
  int16x8_t coeff1_abs = vec_abs(coeff1);

  zero_mask0 = vec_cmpge(coeff0_abs, zbin);
  zbin = vec_splat(zbin, 1);
  zero_mask1 = vec_cmpge(coeff1_abs, zbin);

  (void)scan_ptr;
  (void)skip_block;
  assert(!skip_block);

  qcoeff0 =
      quantize_coeff(coeff0, coeff0_abs, round, quant, quant_shift, zero_mask0);
  vec_vsx_st(qcoeff0, 0, qcoeff_ptr);
  round = vec_splat(round, 1);
  quant = vec_splat(quant, 1);
  quant_shift = vec_splat(quant_shift, 1);
  qcoeff1 =
      quantize_coeff(coeff1, coeff1_abs, round, quant, quant_shift, zero_mask1);
  vec_vsx_st(qcoeff1, 16, qcoeff_ptr);

  dqcoeff0 = vec_mladd(qcoeff0, dequant, vec_zeros_s16);
  vec_vsx_st(dqcoeff0, 0, dqcoeff_ptr);
  dequant = vec_splat(dequant, 1);
  dqcoeff1 = vec_mladd(qcoeff1, dequant, vec_zeros_s16);
  vec_vsx_st(dqcoeff1, 16, dqcoeff_ptr);

  eob = nonzero_scanindex(qcoeff0, zero_mask0, qcoeff1, zero_mask1, iscan_ptr);

  n_coeffs -= 16;
  while (n_coeffs > 0) {
    coeff_ptr += 16;
    qcoeff_ptr += 16;
    dqcoeff_ptr += 16;
    iscan_ptr += 16;

    coeff0 = vec_vsx_ld(0, coeff_ptr);
    coeff1 = vec_vsx_ld(16, coeff_ptr);
    coeff0_abs = vec_abs(coeff0);
    coeff1_abs = vec_abs(coeff1);
    zero_mask0 = vec_cmpge(coeff0_abs, zbin);
    zero_mask1 = vec_cmpge(coeff1_abs, zbin);
    qcoeff0 = quantize_coeff(coeff0, coeff0_abs, round, quant, quant_shift,
                             zero_mask0);
    qcoeff1 = quantize_coeff(coeff1, coeff1_abs, round, quant, quant_shift,
                             zero_mask1);
    vec_vsx_st(qcoeff0, 0, qcoeff_ptr);
    vec_vsx_st(qcoeff1, 16, qcoeff_ptr);

    dqcoeff0 = vec_mladd(qcoeff0, dequant, vec_zeros_s16);
    dqcoeff1 = vec_mladd(qcoeff1, dequant, vec_zeros_s16);
    vec_vsx_st(dqcoeff0, 0, dqcoeff_ptr);
    vec_vsx_st(dqcoeff1, 16, dqcoeff_ptr);

    eob = vec_max(eob, nonzero_scanindex(qcoeff0, zero_mask0, qcoeff1,
                                         zero_mask1, iscan_ptr));

    n_coeffs -= 16;
  }

  eob = vec_max_across(eob);
  *eob_ptr = eob[0];
}
