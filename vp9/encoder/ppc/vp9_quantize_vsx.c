/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"

#include "./vp9_rtcd.h"
#include "vpx_dsp/ppc/types_vsx.h"

// Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit
// integers, and return the high 16 bits of the intermediate integers.
// (a * b) >> 16
// Note: Because this is done in 2 operations, a and b cannot both be UINT16_MIN
static INLINE int16x8_t vec_mulhi(int16x8_t a, int16x8_t b) {
  // madds does ((A * B) >> 15) + C, we need >> 16, so we perform an extra right
  // shift.
  return vec_sra(vec_madds(a, b, vec_zeros_s16), vec_ones_u16);
}

// Negate 16-bit integers in a when the corresponding signed 16-bit
// integer in b is negative.
static INLINE int16x8_t vec_sign(int16x8_t a, int16x8_t b) {
  const int16x8_t mask = vec_sra(b, vec_shift_sign_s16);
  return vec_xor(vec_add(a, mask), mask);
}

// Compare packed 16-bit integers across a, and return the maximum value in
// every element. Returns a vector containing the biggest value across vector a.
static INLINE int16x8_t vec_max_across(int16x8_t a) {
  a = vec_max(a, vec_perm(a, a, vec_perm64));
  a = vec_max(a, vec_perm(a, a, vec_perm32));
  return vec_max(a, vec_perm(a, a, vec_perm16));
}

void vp9_quantize_fp_vsx(const tran_low_t *coeff_ptr, intptr_t n_coeffs,
                         int skip_block, const int16_t *round_ptr,
                         const int16_t *quant_ptr, tran_low_t *qcoeff_ptr,
                         tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr,
                         uint16_t *eob_ptr, const int16_t *scan_ptr,
                         const int16_t *iscan_ptr) {
  int index = 8;
  int16x8_t qcoeff, dqcoeff, scan, eob;
  bool16x8_t zero_coeff;
  int16x8_t round = vec_vsx_ld(0, round_ptr);
  int16x8_t quant = vec_vsx_ld(0, quant_ptr);
  int16x8_t dequant = vec_vsx_ld(0, dequant_ptr);
  int16x8_t coeff = vec_vsx_ld(0, coeff_ptr);

  (void)scan_ptr;
  (void)skip_block;
  assert(!skip_block);

  qcoeff = vec_mulhi(vec_vaddshs(vec_abs(coeff), round), quant);
  qcoeff = vec_sign(qcoeff, coeff);

  vec_vsx_st(qcoeff, 0, qcoeff_ptr);
  dqcoeff = vec_mladd(qcoeff, dequant, vec_zeros_s16);
  vec_vsx_st(dqcoeff, 0, dqcoeff_ptr);

  round = vec_splat(round, 1);
  quant = vec_splat(quant, 1);
  dequant = vec_splat(dequant, 1);

  scan = vec_vsx_ld(0, iscan_ptr);
  zero_coeff = vec_cmpeq(qcoeff, vec_zeros_s16);
  eob = vec_andc(vec_add(scan, vec_ones_s16), zero_coeff);

  do {
    coeff = vec_vsx_ld(0, coeff_ptr + index);
    qcoeff = vec_mulhi(vec_vaddshs(vec_abs(coeff), round), quant);
    qcoeff = vec_sign(qcoeff, coeff);
    vec_vsx_st(qcoeff, 0, qcoeff_ptr + index);

    dqcoeff = vec_mladd(qcoeff, dequant, vec_zeros_s16);
    vec_vsx_st(dqcoeff, 0, dqcoeff_ptr + index);

    scan = vec_vsx_ld(0, iscan_ptr + index);
    zero_coeff = vec_cmpeq(qcoeff, vec_zeros_s16);
    eob = vec_max(eob, vec_andc(vec_add(scan, vec_ones_s16), zero_coeff));
    index += 8;
  } while (index < n_coeffs);

  eob = vec_max_across(eob);
  *eob_ptr = eob[0];
}
