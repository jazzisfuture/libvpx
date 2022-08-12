/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>
#include <assert.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/mem_neon.h"

static INLINE void highbd_calculate_dqcoeff_and_store(const int32x4_t dqcoeff_0,
                                                      const int32x4_t dqcoeff_1,
                                                      tran_low_t *dqcoeff_ptr) {
  vst1q_s32(dqcoeff_ptr, dqcoeff_0);
  vst1q_s32(dqcoeff_ptr + 4, dqcoeff_1);
}

static INLINE int16x8_t
highbd_quantize_b_neon(const tran_low_t *coeff_ptr, tran_low_t *qcoeff_ptr,
                       tran_low_t *dqcoeff_ptr, const int16x8_t zbin,
                       const int16x8_t round, const int16x8_t quant,
                       const int16x8_t quant_shift, const int16x8_t dequant) {
  int16x8_t qcoeff;
  int32x4_t qcoeff_0, qcoeff_1, dqcoeff_0, dqcoeff_1;

  // Load coeffs as 2 vectors of 4 x 32-bit ints each, take sign and abs values
  const int32x4_t coeff_0 = vld1q_s32(coeff_ptr);
  const int32x4_t coeff_1 = vld1q_s32(coeff_ptr + 4);
  const int32x4_t coeff_0_sign = vshrq_n_s32(coeff_0, 31);
  const int32x4_t coeff_1_sign = vshrq_n_s32(coeff_1, 31);
  const int32x4_t coeff_0_abs = vabsq_s32(coeff_0);
  const int32x4_t coeff_1_abs = vabsq_s32(coeff_1);

  // Calculate 2 masks of elements outside the bin
  const int32x4_t zbin_mask_0 = vreinterpretq_s32_u32(
      vcgeq_s32(coeff_0_abs, vmovl_s16(vget_low_s16(zbin))));
  const int32x4_t zbin_mask_1 = vreinterpretq_s32_u32(
      vcgeq_s32(coeff_1_abs, vmovl_s16(vget_high_s16(zbin))));

  // Get the rounded values
  const int32x4_t rounded_0 =
      vaddq_s32(coeff_0_abs, vmovl_s16(vget_low_s16(round)));
  const int32x4_t rounded_1 =
      vaddq_s32(coeff_1_abs, vmovl_s16(vget_high_s16(round)));

  // Extend the quant, quant_shift vectors to ones of 32-bit elements
  int32x4_t quant_0 = vmovl_s16(vget_low_s16(quant));
  int32x4_t quant_1 = vmovl_s16(vget_high_s16(quant));
  int32x4_t quant_shift_0 = vmovl_s16(vget_low_s16(quant_shift));
  int32x4_t quant_shift_1 = vmovl_s16(vget_high_s16(quant_shift));

  // Need the product: (round * quant)
  // Multiply and extend to 4 vectors of 2 x 64-bit
  int64x2_t qcoeff_00 =
      vmull_s32(vget_low_s32(rounded_0), vget_low_s32(quant_0));
  int64x2_t qcoeff_01 =
      vmull_s32(vget_high_s32(rounded_0), vget_high_s32(quant_0));
  int64x2_t qcoeff_10 =
      vmull_s32(vget_low_s32(rounded_1), vget_low_s32(quant_1));
  int64x2_t qcoeff_11 =
      vmull_s32(vget_high_s32(rounded_1), vget_high_s32(quant_1));

  // Shift right by 16
  qcoeff_00 = vshrq_n_s64(qcoeff_00, 16);
  qcoeff_01 = vshrq_n_s64(qcoeff_01, 16);
  qcoeff_10 = vshrq_n_s64(qcoeff_10, 16);
  qcoeff_11 = vshrq_n_s64(qcoeff_11, 16);

  // Add the rounded values, vaddw adds a 64-bit value (qcoeff) to a 32-bit
  // value (rounded)
  qcoeff_00 = vaddw_s32(qcoeff_00, vget_low_s32(rounded_0));
  qcoeff_01 = vaddw_s32(qcoeff_01, vget_high_s32(rounded_0));
  qcoeff_10 = vaddw_s32(qcoeff_10, vget_low_s32(rounded_1));
  qcoeff_11 = vaddw_s32(qcoeff_11, vget_high_s32(rounded_1));

  // Similarly get the (qcoeff * quant_shift) product
  qcoeff_00 = vmull_s32(vmovn_s64(qcoeff_00), vget_low_s32(quant_shift_0));
  qcoeff_01 = vmull_s32(vmovn_s64(qcoeff_01), vget_high_s32(quant_shift_0));
  qcoeff_10 = vmull_s32(vmovn_s64(qcoeff_10), vget_low_s32(quant_shift_1));
  qcoeff_11 = vmull_s32(vmovn_s64(qcoeff_11), vget_high_s32(quant_shift_1));

  // And again shift right 16 bits
  qcoeff_00 = vshrq_n_s64(qcoeff_00, 16);
  qcoeff_01 = vshrq_n_s64(qcoeff_01, 16);
  qcoeff_10 = vshrq_n_s64(qcoeff_10, 16);
  qcoeff_11 = vshrq_n_s64(qcoeff_11, 16);

  // Narrow down to 2 vectors of 4 x 32-bit
  qcoeff_0 = vcombine_s32(vmovn_s64(qcoeff_00), vmovn_s64(qcoeff_01));
  qcoeff_1 = vcombine_s32(vmovn_s64(qcoeff_10), vmovn_s64(qcoeff_11));

  // Restore the sign bit.
  qcoeff_0 = veorq_s32(qcoeff_0, coeff_0_sign);
  qcoeff_1 = veorq_s32(qcoeff_1, coeff_1_sign);
  qcoeff_0 = vsubq_s32(qcoeff_0, coeff_0_sign);
  qcoeff_1 = vsubq_s32(qcoeff_1, coeff_1_sign);

  // Only keep the relevant coeffs
  qcoeff_0 = vandq_s32(qcoeff_0, zbin_mask_0);
  qcoeff_1 = vandq_s32(qcoeff_1, zbin_mask_1);

  // Store the 32-bit qcoeffs
  vst1q_s32(qcoeff_ptr, qcoeff_0);
  vst1q_s32(qcoeff_ptr + 4, qcoeff_1);

  // Calculate and store the dqcoeffs
  qcoeff = vcombine_s16(vmovn_s32(qcoeff_0), vmovn_s32(qcoeff_1));
  dqcoeff_0 = vmulq_s32(qcoeff_0, vmovl_s16(vget_low_s16(dequant)));
  dqcoeff_1 = vmulq_s32(qcoeff_1, vmovl_s16(vget_high_s16(dequant)));

  highbd_calculate_dqcoeff_and_store(dqcoeff_0, dqcoeff_1, dqcoeff_ptr);

  return qcoeff;
}

void vpx_highbd_quantize_b_neon(const tran_low_t *coeff_ptr, intptr_t n_coeffs,
                                const int16_t *zbin_ptr,
                                const int16_t *round_ptr,
                                const int16_t *quant_ptr,
                                const int16_t *quant_shift_ptr,
                                tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr,
                                const int16_t *dequant_ptr, uint16_t *eob_ptr,
                                const int16_t *scan, const int16_t *iscan) {
  const int16x8_t one = vdupq_n_s16(1);
  const int16x8_t neg_one = vdupq_n_s16(-1);
  uint16x8_t eob_max;
  (void)scan;

  // Process first 8 values which include a dc component.
  {
    // Only the first element of each vector is DC.
    const int16x8_t zbin = vld1q_s16(zbin_ptr);
    const int16x8_t round = vld1q_s16(round_ptr);
    const int16x8_t quant = vld1q_s16(quant_ptr);
    const int16x8_t quant_shift = vld1q_s16(quant_shift_ptr);
    const int16x8_t dequant = vld1q_s16(dequant_ptr);
    // Add one because the eob does not index from 0.
    const uint16x8_t v_iscan =
        vreinterpretq_u16_s16(vaddq_s16(vld1q_s16(iscan), one));

    const int16x8_t qcoeff =
        highbd_quantize_b_neon(coeff_ptr, qcoeff_ptr, dqcoeff_ptr, zbin, round,
                               quant, quant_shift, dequant);

    // Set non-zero elements to -1 and use that to extract values for eob.
    eob_max = vandq_u16(vtstq_s16(qcoeff, neg_one), v_iscan);

    __builtin_prefetch(coeff_ptr + 64);

    coeff_ptr += 8;
    iscan += 8;
    qcoeff_ptr += 8;
    dqcoeff_ptr += 8;
  }

  n_coeffs -= 8;

  {
    const int16x8_t zbin = vdupq_n_s16(zbin_ptr[1]);
    const int16x8_t round = vdupq_n_s16(round_ptr[1]);
    const int16x8_t quant = vdupq_n_s16(quant_ptr[1]);
    const int16x8_t quant_shift = vdupq_n_s16(quant_shift_ptr[1]);
    const int16x8_t dequant = vdupq_n_s16(dequant_ptr[1]);

    do {
      // Add one because the eob is not its index.
      const uint16x8_t v_iscan =
          vreinterpretq_u16_s16(vaddq_s16(vld1q_s16(iscan), one));

      const int16x8_t qcoeff =
          highbd_quantize_b_neon(coeff_ptr, qcoeff_ptr, dqcoeff_ptr, zbin,
                                 round, quant, quant_shift, dequant);

      // Set non-zero elements to -1 and use that to extract values for eob.
      eob_max =
          vmaxq_u16(eob_max, vandq_u16(vtstq_s16(qcoeff, neg_one), v_iscan));

      __builtin_prefetch(coeff_ptr + 64);
      coeff_ptr += 8;
      iscan += 8;
      qcoeff_ptr += 8;
      dqcoeff_ptr += 8;
      n_coeffs -= 8;
    } while (n_coeffs > 0);
  }

#ifdef __aarch64__
  *eob_ptr = vmaxvq_u16(eob_max);
#else
  {
    const uint16x4_t eob_max_0 =
        vmax_u16(vget_low_u16(eob_max), vget_high_u16(eob_max));
    const uint16x4_t eob_max_1 = vpmax_u16(eob_max_0, eob_max_0);
    const uint16x4_t eob_max_2 = vpmax_u16(eob_max_1, eob_max_1);
    vst1_lane_u16(eob_ptr, eob_max_2, 0);
  }
#endif  // __aarch64__
}

static INLINE int32x4_t extract_sign_bit(int32x4_t a) {
  return vreinterpretq_s32_u32(vshrq_n_u32(vreinterpretq_u32_s32(a), 31));
}

static INLINE void highbd_calculate_dqcoeff_and_store_32x32(
    int32x4_t dqcoeff_0, int32x4_t dqcoeff_1, tran_low_t *dqcoeff_ptr) {
  // Add 1 if negative to round towards zero because the C uses division.
  dqcoeff_0 = vaddq_s32(dqcoeff_0, extract_sign_bit(dqcoeff_0));
  dqcoeff_1 = vaddq_s32(dqcoeff_1, extract_sign_bit(dqcoeff_1));

  dqcoeff_0 = vshrq_n_s32(dqcoeff_0, 1);
  dqcoeff_1 = vshrq_n_s32(dqcoeff_1, 1);
  vst1q_s32(dqcoeff_ptr, dqcoeff_0);
  vst1q_s32(dqcoeff_ptr + 4, dqcoeff_1);
}

static INLINE int16x8_t highbd_quantize_b_32x32_neon(
    const tran_low_t *coeff_ptr, tran_low_t *qcoeff_ptr,
    tran_low_t *dqcoeff_ptr, const int16x8_t zbin, const int16x8_t round,
    const int16x8_t quant, const int16x8_t quant_shift,
    const int16x8_t dequant) {
  int16x8_t qcoeff;
  int32x4_t qcoeff_0, qcoeff_1, dqcoeff_0, dqcoeff_1;

  // Load coeffs as 2 vectors of 4 x 32-bit ints each, take sign and abs values
  const int32x4_t coeff_0 = vld1q_s32(coeff_ptr);
  const int32x4_t coeff_1 = vld1q_s32(coeff_ptr + 4);
  const int32x4_t coeff_0_sign = vshrq_n_s32(coeff_0, 31);
  const int32x4_t coeff_1_sign = vshrq_n_s32(coeff_1, 31);
  const int32x4_t coeff_0_abs = vabsq_s32(coeff_0);
  const int32x4_t coeff_1_abs = vabsq_s32(coeff_1);

  // Calculate 2 masks of elements outside the bin
  const int32x4_t zbin_mask_0 = vreinterpretq_s32_u32(
      vcgeq_s32(coeff_0_abs, vmovl_s16(vget_low_s16(zbin))));
  const int32x4_t zbin_mask_1 = vreinterpretq_s32_u32(
      vcgeq_s32(coeff_1_abs, vmovl_s16(vget_high_s16(zbin))));

  // Get the rounded values
  const int32x4_t rounded_0 =
      vaddq_s32(coeff_0_abs, vmovl_s16(vget_low_s16(round)));
  const int32x4_t rounded_1 =
      vaddq_s32(coeff_1_abs, vmovl_s16(vget_high_s16(round)));

  // Extend the quant, quant_shift vectors to ones of 32-bit elements
  int32x4_t quant_0 = vmovl_s16(vget_low_s16(quant));
  int32x4_t quant_1 = vmovl_s16(vget_high_s16(quant));
  int32x4_t quant_shift_0 = vmovl_s16(vget_low_s16(quant_shift));
  int32x4_t quant_shift_1 = vmovl_s16(vget_high_s16(quant_shift));

  // Need the product: (round * quant)
  // Multiply and extend to 4 vectors of 2 x 64-bit
  int64x2_t qcoeff_00 =
      vmull_s32(vget_low_s32(rounded_0), vget_low_s32(quant_0));
  int64x2_t qcoeff_01 =
      vmull_s32(vget_high_s32(rounded_0), vget_high_s32(quant_0));
  int64x2_t qcoeff_10 =
      vmull_s32(vget_low_s32(rounded_1), vget_low_s32(quant_1));
  int64x2_t qcoeff_11 =
      vmull_s32(vget_high_s32(rounded_1), vget_high_s32(quant_1));

  // Shift right by 16
  qcoeff_00 = vshrq_n_s64(qcoeff_00, 16);
  qcoeff_01 = vshrq_n_s64(qcoeff_01, 16);
  qcoeff_10 = vshrq_n_s64(qcoeff_10, 16);
  qcoeff_11 = vshrq_n_s64(qcoeff_11, 16);

  // Add the rounded values, vaddw adds a 64-bit value (qcoeff) to a 32-bit
  // value (rounded)
  qcoeff_00 = vaddw_s32(qcoeff_00, vget_low_s32(rounded_0));
  qcoeff_01 = vaddw_s32(qcoeff_01, vget_high_s32(rounded_0));
  qcoeff_10 = vaddw_s32(qcoeff_10, vget_low_s32(rounded_1));
  qcoeff_11 = vaddw_s32(qcoeff_11, vget_high_s32(rounded_1));

  // Similarly get the (qcoeff * quant_shift) product
  qcoeff_00 = vmull_s32(vmovn_s64(qcoeff_00), vget_low_s32(quant_shift_0));
  qcoeff_01 = vmull_s32(vmovn_s64(qcoeff_01), vget_high_s32(quant_shift_0));
  qcoeff_10 = vmull_s32(vmovn_s64(qcoeff_10), vget_low_s32(quant_shift_1));
  qcoeff_11 = vmull_s32(vmovn_s64(qcoeff_11), vget_high_s32(quant_shift_1));

  // And again shift right 15 bits
  qcoeff_00 = vshrq_n_s64(qcoeff_00, 15);
  qcoeff_01 = vshrq_n_s64(qcoeff_01, 15);
  qcoeff_10 = vshrq_n_s64(qcoeff_10, 15);
  qcoeff_11 = vshrq_n_s64(qcoeff_11, 15);

  // Narrow down to 2 vectors of 4 x 32-bit
  qcoeff_0 = vcombine_s32(vmovn_s64(qcoeff_00), vmovn_s64(qcoeff_01));
  qcoeff_1 = vcombine_s32(vmovn_s64(qcoeff_10), vmovn_s64(qcoeff_11));

  // Restore the sign bit.
  qcoeff_0 = veorq_s32(qcoeff_0, coeff_0_sign);
  qcoeff_1 = veorq_s32(qcoeff_1, coeff_1_sign);
  qcoeff_0 = vsubq_s32(qcoeff_0, coeff_0_sign);
  qcoeff_1 = vsubq_s32(qcoeff_1, coeff_1_sign);

  // Only keep the relevant coeffs
  qcoeff_0 = vandq_s32(qcoeff_0, zbin_mask_0);
  qcoeff_1 = vandq_s32(qcoeff_1, zbin_mask_1);

  // Store the 32-bit qcoeffs
  vst1q_s32(qcoeff_ptr, qcoeff_0);
  vst1q_s32(qcoeff_ptr + 4, qcoeff_1);

  // Calculate and store the dqcoeffs
  qcoeff = vcombine_s16(vmovn_s32(qcoeff_0), vmovn_s32(qcoeff_1));
  dqcoeff_0 = vmulq_s32(qcoeff_0, vmovl_s16(vget_low_s16(dequant)));
  dqcoeff_1 = vmulq_s32(qcoeff_1, vmovl_s16(vget_high_s16(dequant)));

  highbd_calculate_dqcoeff_and_store_32x32(dqcoeff_0, dqcoeff_1, dqcoeff_ptr);

  return qcoeff;
}

void vpx_highbd_quantize_b_32x32_neon(
    const tran_low_t *coeff_ptr, intptr_t n_coeffs, const int16_t *zbin_ptr,
    const int16_t *round_ptr, const int16_t *quant_ptr,
    const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr,
    tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr,
    const int16_t *scan, const int16_t *iscan) {
  const int16x8_t one = vdupq_n_s16(1);
  const int16x8_t neg_one = vdupq_n_s16(-1);
  uint16x8_t eob_max;
  int i;
  (void)scan;
  (void)n_coeffs;  // Because we will always calculate 32*32.

  // Process first 8 values which include a dc component.
  {
    // Only the first element of each vector is DC.
    const int16x8_t zbin = vrshrq_n_s16(vld1q_s16(zbin_ptr), 1);
    const int16x8_t round = vrshrq_n_s16(vld1q_s16(round_ptr), 1);
    const int16x8_t quant = vld1q_s16(quant_ptr);
    const int16x8_t quant_shift = vld1q_s16(quant_shift_ptr);
    const int16x8_t dequant = vld1q_s16(dequant_ptr);
    // Add one because the eob does not index from 0.
    const uint16x8_t v_iscan =
        vreinterpretq_u16_s16(vaddq_s16(vld1q_s16(iscan), one));

    const int16x8_t qcoeff =
        highbd_quantize_b_32x32_neon(coeff_ptr, qcoeff_ptr, dqcoeff_ptr, zbin,
                                     round, quant, quant_shift, dequant);

    // Set non-zero elements to -1 and use that to extract values for eob.
    eob_max = vandq_u16(vtstq_s16(qcoeff, neg_one), v_iscan);

    __builtin_prefetch(coeff_ptr + 64);
    coeff_ptr += 8;
    iscan += 8;
    qcoeff_ptr += 8;
    dqcoeff_ptr += 8;
  }

  {
    const int16x8_t zbin = vrshrq_n_s16(vdupq_n_s16(zbin_ptr[1]), 1);
    const int16x8_t round = vrshrq_n_s16(vdupq_n_s16(round_ptr[1]), 1);
    const int16x8_t quant = vdupq_n_s16(quant_ptr[1]);
    const int16x8_t quant_shift = vdupq_n_s16(quant_shift_ptr[1]);
    const int16x8_t dequant = vdupq_n_s16(dequant_ptr[1]);

    for (i = 1; i < 32 * 32 / 8; ++i) {
      // Add one because the eob is not its index.
      const uint16x8_t v_iscan =
          vreinterpretq_u16_s16(vaddq_s16(vld1q_s16(iscan), one));

      const int16x8_t qcoeff =
          highbd_quantize_b_32x32_neon(coeff_ptr, qcoeff_ptr, dqcoeff_ptr, zbin,
                                       round, quant, quant_shift, dequant);

      // Set non-zero elements to -1 and use that to extract values for eob.
      eob_max =
          vmaxq_u16(eob_max, vandq_u16(vtstq_s16(qcoeff, neg_one), v_iscan));

      __builtin_prefetch(coeff_ptr + 64);
      coeff_ptr += 8;
      iscan += 8;
      qcoeff_ptr += 8;
      dqcoeff_ptr += 8;
    }
  }

#ifdef __aarch64__
  *eob_ptr = vmaxvq_u16(eob_max);
#else
  {
    const uint16x4_t eob_max_0 =
        vmax_u16(vget_low_u16(eob_max), vget_high_u16(eob_max));
    const uint16x4_t eob_max_1 = vpmax_u16(eob_max_0, eob_max_0);
    const uint16x4_t eob_max_2 = vpmax_u16(eob_max_1, eob_max_1);
    vst1_lane_u16(eob_ptr, eob_max_2, 0);
  }
#endif  // __aarch64__
}
