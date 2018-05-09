/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include <string.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/ppc/types_vsx.h"

static const int16x8_t vsx_zeros = { 0, 0, 0, 0, 0, 0, 0, 0 };
static const uint16x8_t vsx_ones = { 1, 1, 1, 1, 1, 1, 1, 1 };
static const uint16x8_t vsx_shift_sign = { 15, 15, 15, 15, 15, 15, 15, 15 };
static const uint8x16_t mask_64 = { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
                                    0x0E, 0x0F, 0x00, 0x01, 0x02, 0x03,
                                    0x04, 0x05, 0x06, 0x07 };
static const uint8x16_t mask_32 = { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                                    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                    0x00, 0x01, 0x02, 0x03 };
static const uint8x16_t mask_16 = { 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                    0x08, 0x09, 0x0A, 0x0B, 0x0E, 0x0D,
                                    0x0E, 0x0F, 0x00, 0x01 };

typedef vector bool short bool16x8_t;

// Negate 16-bit integers in a when the corresponding signed 16-bit
// integer in b is negative.
static INLINE int16x8_t vec_sign(int16x8_t a, int16x8_t b) {
  const int16x8_t mask = vec_sra(b, vsx_shift_sign);
  return vec_xor(vec_add(a, mask), mask);
}

// Multiply the packed 16-bit integers in a and b, producing intermediate 32-bit
// integers, and return the high 16 bits of the intermediate integers.
static INLINE int16x8_t vec_mulhi(int16x8_t a, int16x8_t b) {
  // madds does ((A * B) >>15) + C, we need >> 16, so we perform an extra right
  // shift.
  return vec_sra(vec_madds(a, b, vsx_zeros), vsx_ones);
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

static INLINE int16x8_t nonzero_scanindex(int16x8_t qcoeff, bool16x8_t mask,
                                          const int16_t *iscan_ptr) {
  bool16x8_t zero_coeff;
  int16x8_t scan = vec_vsx_ld(0, iscan_ptr);
  zero_coeff = vec_cmpeq(qcoeff, vsx_zeros);
  scan = vec_sub(scan, mask);
  return vec_andc(scan, zero_coeff);
}

// Compare packed 16-bit integers across a, and return the maximum value in
// every element. Returns a vector containing the biggest value across vector a.
static INLINE int16x8_t vec_max_across(int16x8_t a) {
  a = vec_max(a, vec_perm(a, a, mask_64));
  a = vec_max(a, vec_perm(a, a, mask_32));
  return vec_max(a, vec_perm(a, a, mask_16));
}

void vpx_quantize_b_vsx(const tran_low_t *coeff_ptr, intptr_t n_coeffs,
                        int skip_block, const int16_t *zbin_ptr,
                        const int16_t *round_ptr, const int16_t *quant_ptr,
                        const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr,
                        tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr,
                        uint16_t *eob_ptr, const int16_t *scan_ptr,
                        const int16_t *iscan_ptr) {
  int16x8_t qcoeff, dqcoeff, eob;

  // First set of 8 coeff starts with DC + 7 AC
  int16x8_t zbin = vec_vsx_ld(0, zbin_ptr);
  int16x8_t round = vec_vsx_ld(0, round_ptr);
  int16x8_t quant = vec_vsx_ld(0, quant_ptr);
  int16x8_t dequant = vec_vsx_ld(0, dequant_ptr);
  int16x8_t quant_shift = vec_vsx_ld(0, quant_shift_ptr);

  int16x8_t coeff = vec_vsx_ld(0, coeff_ptr);
  int16x8_t coeff_abs = vec_abs(coeff);
  bool16x8_t mask = vec_cmpge(coeff_abs, zbin);

  (void)scan_ptr;
  (void)skip_block;
  assert(!skip_block);

  qcoeff = quantize_coeff(coeff, coeff_abs, round, quant, quant_shift, mask);
  vec_vsx_st(qcoeff, 0, qcoeff_ptr);

  dqcoeff = vec_mladd(qcoeff, dequant, vsx_zeros);
  vec_vsx_st(dqcoeff, 0, dqcoeff_ptr);

  eob = nonzero_scanindex(qcoeff, mask, iscan_ptr);

  // All other sets of 8 coeffs will only contain AC
  zbin = vec_splat(zbin, 1);
  round = vec_splat(round, 1);
  quant = vec_splat(quant, 1);
  dequant = vec_splat(dequant, 1);
  quant_shift = vec_splat(quant_shift, 1);

  n_coeffs -= 8;
  do {
    coeff_ptr += 8;
    qcoeff_ptr += 8;
    dqcoeff_ptr += 8;
    iscan_ptr += 8;

    coeff = vec_vsx_ld(0, coeff_ptr);
    coeff_abs = vec_abs(coeff);
    mask = vec_cmpge(coeff_abs, zbin);
    qcoeff = quantize_coeff(coeff, coeff_abs, round, quant, quant_shift, mask);
    vec_vsx_st(qcoeff, 0, qcoeff_ptr);

    dqcoeff = vec_mladd(qcoeff, dequant, vsx_zeros);
    vec_vsx_st(dqcoeff, 0, dqcoeff_ptr);

    eob = vec_max(eob, nonzero_scanindex(qcoeff, mask, iscan_ptr));

    n_coeffs -= 8;
  } while (n_coeffs > 0);

  eob = vec_max_across(eob);
  *eob_ptr = eob[0];
}
