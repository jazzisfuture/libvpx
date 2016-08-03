/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>
#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/arm/transpose_neon.h"

// Should we apply any filter at all: 11111111 yes, 00000000 no
static INLINE uint8x8_t filter_mask(uint8x8_t limit, uint8x8_t blimit,
                                    uint8x8_t thresh, uint8x8_t p3,
                                    uint8x8_t p2, uint8x8_t p1, uint8x8_t p0,
                                    uint8x8_t q0, uint8x8_t q1, uint8x8_t q2,
                                    uint8x8_t q3, uint8x8_t *flat,
                                    uint8x8_t *hev) {
  uint8x8_t t0, t1;
  uint8x8_t max = vabd_u8(p1, p0);
  max = vmax_u8(max, vabd_u8(q1, q0));

  // Is there high edge variance internal edge: 11111111 yes, 00000000 no
  *hev = vcgt_u8(max, thresh);
  *flat = vmax_u8(max, vabd_u8(p2, p0));
  max = vmax_u8(max, vabd_u8(p3, p2));
  max = vmax_u8(max, vabd_u8(p2, p1));
  max = vmax_u8(max, vabd_u8(q2, q1));
  max = vmax_u8(max, vabd_u8(q3, q2));
  t0 = vabd_u8(p0, q0);
  t1 = vabd_u8(p1, q1);
  t0 = vqshl_n_u8(t0, 1);
  t1 = vshr_n_u8(t1, 1);
  t0 = vqadd_u8(t0, t1);
  max = vcle_u8(max, limit);
  t0 = vcle_u8(t0, blimit);
  max = vand_u8(max, t0);

  *flat = vmax_u8(*flat, vabd_u8(q2, q0));
  *flat = vmax_u8(*flat, vabd_u8(p3, p0));
  *flat = vmax_u8(*flat, vabd_u8(q3, q0));
  *flat = vcle_u8(*flat, vdup_n_u8(1));  // flat_mask4()

  return max;
}

static INLINE uint8x8_t flat_mask5(uint8x8_t p4, uint8x8_t p3, uint8x8_t p2,
                                   uint8x8_t p1, uint8x8_t p0, uint8x8_t q0,
                                   uint8x8_t q1, uint8x8_t q2, uint8x8_t q3,
                                   uint8x8_t q4) {
  uint8x8_t max = vabd_u8(p4, p0);
  max = vmax_u8(max, vabd_u8(p3, p0));
  max = vmax_u8(max, vabd_u8(p2, p0));
  max = vmax_u8(max, vabd_u8(p1, p0));
  max = vmax_u8(max, vabd_u8(q1, q0));
  max = vmax_u8(max, vabd_u8(q2, q0));
  max = vmax_u8(max, vabd_u8(q3, q0));
  max = vmax_u8(max, vabd_u8(q4, q0));
  max = vcle_u8(max, vdup_n_u8(1));

  return max;
}

static INLINE int8x8_t flip_sign(const uint8x8_t v) {
  const uint8x8_t sign_bit = vdup_n_u8(0x80);
  return vreinterpret_s8_u8(veor_u8(v, sign_bit));
}

static INLINE uint8x8_t flip_sign_back(const int8x8_t v) {
  const int8x8_t sign_bit = vdup_n_s8(0x80);
  return vreinterpret_u8_s8(veor_s8(v, sign_bit));
}

static INLINE uint8x8_t filter(uint16x8_t *o_u16x8, uint8x8_t flat_u8x8,
                               uint8x8_t sub0_u8x8, uint8x8_t sub1_u8x8,
                               uint8x8_t add0_u8x8, uint8x8_t add1_u8x8,
                               uint8x8_t i_u8x8, const int shift) {
  *o_u16x8 = vsubw_u8(*o_u16x8, sub0_u8x8);
  *o_u16x8 = vsubw_u8(*o_u16x8, sub1_u8x8);
  *o_u16x8 = vaddw_u8(*o_u16x8, add0_u8x8);
  *o_u16x8 = vaddw_u8(*o_u16x8, add1_u8x8);
  return vbsl_u8(flat_u8x8, vrshrn_n_u16(*o_u16x8, shift), i_u8x8);
}

// 7-tap filter [1, 1, 1, 2, 1, 1, 1]
static INLINE void apply_7_tap_filter(uint8x8_t flat_u8x8, uint8x8_t p3_u8x8,
                                      uint8x8_t p2_u8x8, uint8x8_t p1_u8x8,
                                      uint8x8_t p0_u8x8, uint8x8_t q0_u8x8,
                                      uint8x8_t q1_u8x8, uint8x8_t q2_u8x8,
                                      uint8x8_t q3_u8x8, uint8x8_t *op2_u8x8,
                                      uint8x8_t *op1_u8x8, uint8x8_t *op0_u8x8,
                                      uint8x8_t *oq0_u8x8, uint8x8_t *oq1_u8x8,
                                      uint8x8_t *oq2_u8x8) {
  uint16x8_t o_u16x8;
  o_u16x8 = vaddl_u8(p3_u8x8, p3_u8x8);  // 2*p3
  o_u16x8 = vaddw_u8(o_u16x8, p3_u8x8);  // 3*p3
  o_u16x8 = vaddw_u8(o_u16x8, p2_u8x8);  // 3*p3+p2
  o_u16x8 = vaddw_u8(o_u16x8, p2_u8x8);  // 3*p3+2*p2
  o_u16x8 = vaddw_u8(o_u16x8, p1_u8x8);  // 3*p3+2*p2+p1
  o_u16x8 = vaddw_u8(o_u16x8, p0_u8x8);  // 3*p3+2*p2+p1+p0
  o_u16x8 = vaddw_u8(o_u16x8, q0_u8x8);  // 3*p3+2*p2+p1+p0+q0
  *op2_u8x8 = vbsl_u8(flat_u8x8, vrshrn_n_u16(o_u16x8, 3), p2_u8x8);
  *op1_u8x8 = filter(&o_u16x8, flat_u8x8, p3_u8x8, p2_u8x8, p1_u8x8, q1_u8x8,
                     *op1_u8x8, 3);
  *op0_u8x8 = filter(&o_u16x8, flat_u8x8, p3_u8x8, p1_u8x8, p0_u8x8, q2_u8x8,
                     *op0_u8x8, 3);
  *oq0_u8x8 = filter(&o_u16x8, flat_u8x8, p3_u8x8, p0_u8x8, q0_u8x8, q3_u8x8,
                     *oq0_u8x8, 3);
  *oq1_u8x8 = filter(&o_u16x8, flat_u8x8, p2_u8x8, q0_u8x8, q1_u8x8, q3_u8x8,
                     *oq1_u8x8, 3);
  *oq2_u8x8 = filter(&o_u16x8, flat_u8x8, p1_u8x8, q1_u8x8, q2_u8x8, q3_u8x8,
                     q2_u8x8, 3);
}

// 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
static INLINE void apply_15_tap_filter(
    uint8x8_t flat_u8x8, uint8x8_t flat2_u8x8, uint8x8_t p7_u8x8,
    uint8x8_t p6_u8x8, uint8x8_t p5_u8x8, uint8x8_t p4_u8x8, uint8x8_t p3_u8x8,
    uint8x8_t p2_u8x8, uint8x8_t p1_u8x8, uint8x8_t p0_u8x8, uint8x8_t q0_u8x8,
    uint8x8_t q1_u8x8, uint8x8_t q2_u8x8, uint8x8_t q3_u8x8, uint8x8_t q4_u8x8,
    uint8x8_t q5_u8x8, uint8x8_t q6_u8x8, uint8x8_t q7_u8x8,
    uint8x8_t *op6_u8x8, uint8x8_t *op5_u8x8, uint8x8_t *op4_u8x8,
    uint8x8_t *op3_u8x8, uint8x8_t *op2_u8x8, uint8x8_t *op1_u8x8,
    uint8x8_t *op0_u8x8, uint8x8_t *oq0_u8x8, uint8x8_t *oq1_u8x8,
    uint8x8_t *oq2_u8x8, uint8x8_t *oq3_u8x8, uint8x8_t *oq4_u8x8,
    uint8x8_t *oq5_u8x8, uint8x8_t *oq6_u8x8) {
  uint16x8_t o_u16x8;
  o_u16x8 = vshll_n_u8(p7_u8x8, 3);      // 8*p7
  o_u16x8 = vsubw_u8(o_u16x8, p7_u8x8);  // 7*p7
  o_u16x8 = vaddw_u8(o_u16x8, p6_u8x8);  // 7*p7+p6
  o_u16x8 = vaddw_u8(o_u16x8, p6_u8x8);  // 7*p7+2*p6
  o_u16x8 = vaddw_u8(o_u16x8, p5_u8x8);  // 7*p7+2*p6+p5
  o_u16x8 = vaddw_u8(o_u16x8, p4_u8x8);  // 7*p7+2*p6+p5+p4
  o_u16x8 = vaddw_u8(o_u16x8, p3_u8x8);  // 7*p7+2*p6+p5+p4+p3
  o_u16x8 = vaddw_u8(o_u16x8, p2_u8x8);  // 7*p7+2*p6+p5+p4+p3+p2
  o_u16x8 = vaddw_u8(o_u16x8, p1_u8x8);  // 7*p7+2*p6+p5+p4+p3+p2+p1
  o_u16x8 = vaddw_u8(o_u16x8, p0_u8x8);  // 7*p7+2*p6+p5+p4+p3+p2+p1+p0
  o_u16x8 = vaddw_u8(o_u16x8, q0_u8x8);  // 7*p7+2*p6+p5+p4+p3+p2+p1+p0+q0
  *op6_u8x8 = vbsl_u8(flat2_u8x8, vrshrn_n_u16(o_u16x8, 4), p6_u8x8);
  *op5_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p6_u8x8, p5_u8x8, q1_u8x8,
                     p5_u8x8, 4);
  *op4_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p5_u8x8, p4_u8x8, q2_u8x8,
                     p4_u8x8, 4);
  *op3_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p4_u8x8, p3_u8x8, q3_u8x8,
                     p3_u8x8, 4);
  *op2_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p3_u8x8, p2_u8x8, q4_u8x8,
                     *op2_u8x8, 4);
  *op1_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p2_u8x8, p1_u8x8, q5_u8x8,
                     *op1_u8x8, 4);
  *op0_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p1_u8x8, p0_u8x8, q6_u8x8,
                     *op0_u8x8, 4);
  *oq0_u8x8 = filter(&o_u16x8, flat2_u8x8, p7_u8x8, p0_u8x8, q0_u8x8, q7_u8x8,
                     *oq0_u8x8, 4);
  *oq1_u8x8 = filter(&o_u16x8, flat2_u8x8, p6_u8x8, q0_u8x8, q1_u8x8, q7_u8x8,
                     *oq1_u8x8, 4);
  *oq2_u8x8 = filter(&o_u16x8, flat2_u8x8, p5_u8x8, q1_u8x8, q2_u8x8, q7_u8x8,
                     *oq2_u8x8, 4);
  *oq3_u8x8 = filter(&o_u16x8, flat2_u8x8, p4_u8x8, q2_u8x8, q3_u8x8, q7_u8x8,
                     q3_u8x8, 4);
  *oq4_u8x8 = filter(&o_u16x8, flat2_u8x8, p3_u8x8, q3_u8x8, q4_u8x8, q7_u8x8,
                     q4_u8x8, 4);
  *oq5_u8x8 = filter(&o_u16x8, flat2_u8x8, p2_u8x8, q4_u8x8, q5_u8x8, q7_u8x8,
                     q5_u8x8, 4);
  *oq6_u8x8 = filter(&o_u16x8, flat2_u8x8, p1_u8x8, q5_u8x8, q6_u8x8, q7_u8x8,
                     q6_u8x8, 4);
}

static INLINE void filter16(
    uint8x8_t mask_u8x8, uint8_t thresh, uint8x8_t flat_u8x8, uint64_t flat_u64,
    uint8x8_t flat2_u8x8, uint64_t flat2_u64, uint8x8_t hev_u8x8,
    uint8x8_t p7_u8x8, uint8x8_t p6_u8x8, uint8x8_t p5_u8x8, uint8x8_t p4_u8x8,
    uint8x8_t p3_u8x8, uint8x8_t p2_u8x8, uint8x8_t p1_u8x8, uint8x8_t p0_u8x8,
    uint8x8_t q0_u8x8, uint8x8_t q1_u8x8, uint8x8_t q2_u8x8, uint8x8_t q3_u8x8,
    uint8x8_t q4_u8x8, uint8x8_t q5_u8x8, uint8x8_t q6_u8x8, uint8x8_t q7_u8x8,
    uint8x8_t *op6_u8x8, uint8x8_t *op5_u8x8, uint8x8_t *op4_u8x8,
    uint8x8_t *op3_u8x8, uint8x8_t *op2_u8x8, uint8x8_t *op1_u8x8,
    uint8x8_t *op0_u8x8, uint8x8_t *oq0_u8x8, uint8x8_t *oq1_u8x8,
    uint8x8_t *oq2_u8x8, uint8x8_t *oq3_u8x8, uint8x8_t *oq4_u8x8,
    uint8x8_t *oq5_u8x8, uint8x8_t *oq6_u8x8) {
  // add outer taps if we have high edge variance
  if (flat_u64 != (uint64_t)-1) {
    int8x8_t filter_s8x8, filter1_s8x8, filter2_s8x8, t_s8x8;
    int8x8_t ps1 = flip_sign(p1_u8x8);
    int8x8_t ps0 = flip_sign(p0_u8x8);
    int8x8_t qs0 = flip_sign(q0_u8x8);
    int8x8_t qs1 = flip_sign(q1_u8x8);

    filter_s8x8 = vqsub_s8(ps1, qs1);
    filter_s8x8 = vand_s8(filter_s8x8, vreinterpret_s8_u8(hev_u8x8));
    t_s8x8 = vqsub_s8(qs0, ps0);

    // inner taps
    filter_s8x8 = vqadd_s8(filter_s8x8, t_s8x8);
    filter_s8x8 = vqadd_s8(filter_s8x8, t_s8x8);
    filter_s8x8 = vqadd_s8(filter_s8x8, t_s8x8);
    filter_s8x8 = vand_s8(filter_s8x8, vreinterpret_s8_u8(mask_u8x8));

    // save bottom 3 bits so that we round one side +4 and the other +3
    // if it equals 4 we'll set to adjust by -1 to account for the fact
    // we'd round 3 the other way
    filter1_s8x8 = vshr_n_s8(vqadd_s8(filter_s8x8, vdup_n_s8(4)), 3);
    filter2_s8x8 = vshr_n_s8(vqadd_s8(filter_s8x8, vdup_n_s8(3)), 3);

    qs0 = vqsub_s8(qs0, filter1_s8x8);
    ps0 = vqadd_s8(ps0, filter2_s8x8);
    *oq0_u8x8 = flip_sign_back(qs0);
    *op0_u8x8 = flip_sign_back(ps0);

    // outer tap adjustments
    filter_s8x8 = vrshr_n_s8(filter1_s8x8, 1);
    filter_s8x8 = vbic_s8(filter_s8x8, vreinterpret_s8_u8(hev_u8x8));

    qs1 = vqsub_s8(qs1, filter_s8x8);
    ps1 = vqadd_s8(ps1, filter_s8x8);
    *oq1_u8x8 = flip_sign_back(qs1);
    *op1_u8x8 = flip_sign_back(ps1);
  }

  if (flat_u64) {
    *op6_u8x8 = p6_u8x8;
    *op5_u8x8 = p5_u8x8;
    *op4_u8x8 = p4_u8x8;
    *op3_u8x8 = p3_u8x8;
    *op2_u8x8 = p2_u8x8;
    *oq2_u8x8 = q2_u8x8;
    *oq3_u8x8 = q3_u8x8;
    *oq4_u8x8 = q4_u8x8;
    *oq5_u8x8 = q5_u8x8;
    *oq6_u8x8 = q6_u8x8;
    if (flat2_u64 != (uint64_t)-1) {
      apply_7_tap_filter(flat_u8x8, p3_u8x8, p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8,
                         q1_u8x8, q2_u8x8, q3_u8x8, op2_u8x8, op1_u8x8,
                         op0_u8x8, oq0_u8x8, oq1_u8x8, oq2_u8x8);
    }
    if (flat2_u64) {
      apply_15_tap_filter(flat_u8x8, flat2_u8x8, p7_u8x8, p6_u8x8, p5_u8x8,
                          p4_u8x8, p3_u8x8, p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8,
                          q1_u8x8, q2_u8x8, q3_u8x8, q4_u8x8, q5_u8x8, q6_u8x8,
                          q7_u8x8, op6_u8x8, op5_u8x8, op4_u8x8, op3_u8x8,
                          op2_u8x8, op1_u8x8, op0_u8x8, oq0_u8x8, oq1_u8x8,
                          oq2_u8x8, oq3_u8x8, oq4_u8x8, oq5_u8x8, oq6_u8x8);
    }
  }
}

static void mb_lpf_horizontal_edge_w(uint8_t *s, int p, const uint8_t *blimit,
                                     const uint8_t *limit,
                                     const uint8_t *thresh, int count) {
  const uint8x8_t blimit_u8x8 = vld1_dup_u8(blimit);
  const uint8x8_t limit_u8x8 = vld1_dup_u8(limit);
  const uint8x8_t thresh_u8x8 = vld1_dup_u8(thresh);

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  do {
    const uint8x8_t p7_u8x8 = vld1_u8(s - 8 * p);
    const uint8x8_t p6_u8x8 = vld1_u8(s - 7 * p);
    const uint8x8_t p5_u8x8 = vld1_u8(s - 6 * p);
    const uint8x8_t p4_u8x8 = vld1_u8(s - 5 * p);
    const uint8x8_t p3_u8x8 = vld1_u8(s - 4 * p);
    const uint8x8_t p2_u8x8 = vld1_u8(s - 3 * p);
    const uint8x8_t p1_u8x8 = vld1_u8(s - 2 * p);
    const uint8x8_t p0_u8x8 = vld1_u8(s - 1 * p);
    const uint8x8_t q0_u8x8 = vld1_u8(s + 0 * p);
    const uint8x8_t q1_u8x8 = vld1_u8(s + 1 * p);
    const uint8x8_t q2_u8x8 = vld1_u8(s + 2 * p);
    const uint8x8_t q3_u8x8 = vld1_u8(s + 3 * p);
    const uint8x8_t q4_u8x8 = vld1_u8(s + 4 * p);
    const uint8x8_t q5_u8x8 = vld1_u8(s + 5 * p);
    const uint8x8_t q6_u8x8 = vld1_u8(s + 6 * p);
    const uint8x8_t q7_u8x8 = vld1_u8(s + 7 * p);
    uint8x8_t op6_u8x8, op5_u8x8, op4_u8x8, op3_u8x8, op2_u8x8, op1_u8x8,
        op0_u8x8, oq0_u8x8, oq1_u8x8, oq2_u8x8, oq3_u8x8, oq4_u8x8, oq5_u8x8,
        oq6_u8x8, flat_u8x8, hev_u8x8;
    uint8x8_t mask_u8x8 = filter_mask(
        limit_u8x8, blimit_u8x8, thresh_u8x8, p3_u8x8, p2_u8x8, p1_u8x8,
        p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, &flat_u8x8, &hev_u8x8);
    uint8x8_t flat2_u8x8 =
        flat_mask5(p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p0_u8x8, q0_u8x8,
                   q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8);
    uint64_t flat_u64, flat2_u64;

    flat_u8x8 = vand_u8(flat_u8x8, mask_u8x8);
    flat2_u8x8 = vand_u8(flat2_u8x8, flat_u8x8);
    flat_u64 = vget_lane_u64(vreinterpret_u64_u8(flat_u8x8), 0);
    flat2_u64 = vget_lane_u64(vreinterpret_u64_u8(flat2_u8x8), 0);

    filter16(mask_u8x8, *thresh, flat_u8x8, flat_u64, flat2_u8x8, flat2_u64,
             hev_u8x8, p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p3_u8x8, p2_u8x8,
             p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, q4_u8x8,
             q5_u8x8, q6_u8x8, q7_u8x8, &op6_u8x8, &op5_u8x8, &op4_u8x8,
             &op3_u8x8, &op2_u8x8, &op1_u8x8, &op0_u8x8, &oq0_u8x8, &oq1_u8x8,
             &oq2_u8x8, &oq3_u8x8, &oq4_u8x8, &oq5_u8x8, &oq6_u8x8);

    if (flat_u64) {
      if (flat2_u64) {
        vst1_u8(s - 7 * p, op6_u8x8);
        vst1_u8(s - 6 * p, op5_u8x8);
        vst1_u8(s - 5 * p, op4_u8x8);
        vst1_u8(s - 4 * p, op3_u8x8);
        vst1_u8(s + 3 * p, oq3_u8x8);
        vst1_u8(s + 4 * p, oq4_u8x8);
        vst1_u8(s + 5 * p, oq5_u8x8);
        vst1_u8(s + 6 * p, oq6_u8x8);
      }
      vst1_u8(s - 3 * p, op2_u8x8);
      vst1_u8(s + 2 * p, oq2_u8x8);
    }
    vst1_u8(s - 2 * p, op1_u8x8);
    vst1_u8(s - 1 * p, op0_u8x8);
    vst1_u8(s + 0 * p, oq0_u8x8);
    vst1_u8(s + 1 * p, oq1_u8x8);
    s += 8;
  } while (--count);
}

void vpx_lpf_horizontal_edge_8_neon(uint8_t *s, int p, const uint8_t *blimit,
                                    const uint8_t *limit,
                                    const uint8_t *thresh) {
  mb_lpf_horizontal_edge_w(s, p, blimit, limit, thresh, 1);
}

void vpx_lpf_horizontal_edge_16_neon(uint8_t *s, int p, const uint8_t *blimit,
                                     const uint8_t *limit,
                                     const uint8_t *thresh) {
  mb_lpf_horizontal_edge_w(s, p, blimit, limit, thresh, 2);
}

static void mb_lpf_vertical_edge_w(uint8_t *s, int p, const uint8_t *blimit,
                                   const uint8_t *limit, const uint8_t *thresh,
                                   int count) {
  const uint8x8_t blimit_u8x8 = vld1_dup_u8(blimit);
  const uint8x8_t limit_u8x8 = vld1_dup_u8(limit);
  const uint8x8_t thresh_u8x8 = vld1_dup_u8(thresh);
  uint8_t *d;

  s -= 8;
  d = s;
  do {
    uint8x16_t t0_u8x16, t1_u8x16, t2_u8x16, t3_u8x16, t4_u8x16, t5_u8x16,
        t6_u8x16, t7_u8x16;
    uint8x8_t p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p3_u8x8, p2_u8x8, p1_u8x8,
        p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, q4_u8x8, q5_u8x8, q6_u8x8,
        q7_u8x8, op6_u8x8, op5_u8x8, op4_u8x8, op3_u8x8, op2_u8x8, op1_u8x8,
        op0_u8x8, oq0_u8x8, oq1_u8x8, oq2_u8x8, oq3_u8x8, oq4_u8x8, oq5_u8x8,
        oq6_u8x8, flat_u8x8, hev_u8x8, mask_u8x8, flat2_u8x8;
    uint64_t flat_u64, flat2_u64;

    t0_u8x16 = vld1q_u8(s);
    s += p;
    t1_u8x16 = vld1q_u8(s);
    s += p;
    t2_u8x16 = vld1q_u8(s);
    s += p;
    t3_u8x16 = vld1q_u8(s);
    s += p;
    t4_u8x16 = vld1q_u8(s);
    s += p;
    t5_u8x16 = vld1q_u8(s);
    s += p;
    t6_u8x16 = vld1q_u8(s);
    s += p;
    t7_u8x16 = vld1q_u8(s);
    s += p;

    transpose_u8_16x8(t0_u8x16, t1_u8x16, t2_u8x16, t3_u8x16, t4_u8x16,
                      t5_u8x16, t6_u8x16, t7_u8x16, &p7_u8x8, &p6_u8x8,
                      &p5_u8x8, &p4_u8x8, &p3_u8x8, &p2_u8x8, &p1_u8x8,
                      &p0_u8x8, &q0_u8x8, &q1_u8x8, &q2_u8x8, &q3_u8x8,
                      &q4_u8x8, &q5_u8x8, &q6_u8x8, &q7_u8x8);

    mask_u8x8 = filter_mask(limit_u8x8, blimit_u8x8, thresh_u8x8, p3_u8x8,
                            p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8,
                            q2_u8x8, q3_u8x8, &flat_u8x8, &hev_u8x8);
    flat2_u8x8 = flat_mask5(p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p0_u8x8,
                            q0_u8x8, q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8);
    flat_u8x8 = vand_u8(flat_u8x8, mask_u8x8);
    flat2_u8x8 = vand_u8(flat2_u8x8, flat_u8x8);
    flat_u64 = vget_lane_u64(vreinterpret_u64_u8(flat_u8x8), 0);
    flat2_u64 = vget_lane_u64(vreinterpret_u64_u8(flat2_u8x8), 0);

    filter16(mask_u8x8, *thresh, flat_u8x8, flat_u64, flat2_u8x8, flat2_u64,
             hev_u8x8, p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p3_u8x8, p2_u8x8,
             p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, q4_u8x8,
             q5_u8x8, q6_u8x8, q7_u8x8, &op6_u8x8, &op5_u8x8, &op4_u8x8,
             &op3_u8x8, &op2_u8x8, &op1_u8x8, &op0_u8x8, &oq0_u8x8, &oq1_u8x8,
             &oq2_u8x8, &oq3_u8x8, &oq4_u8x8, &oq5_u8x8, &oq6_u8x8);

    if (flat_u64) {
      if (flat2_u64) {
        uint8x16_t o0, o1, o2, o3, o4, o5, o6, o7;
        transpose_u8_8x16(p7_u8x8, op6_u8x8, op5_u8x8, op4_u8x8, op3_u8x8,
                          op2_u8x8, op1_u8x8, op0_u8x8, oq0_u8x8, oq1_u8x8,
                          oq2_u8x8, oq3_u8x8, oq4_u8x8, oq5_u8x8, oq6_u8x8,
                          q7_u8x8, &o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7);

        vst1q_u8(d, o0);
        d += p;
        vst1q_u8(d, o1);
        d += p;
        vst1q_u8(d, o2);
        d += p;
        vst1q_u8(d, o3);
        d += p;
        vst1q_u8(d, o4);
        d += p;
        vst1q_u8(d, o5);
        d += p;
        vst1q_u8(d, o6);
        d += p;
        vst1q_u8(d, o7);
        d += p;
      } else {
        uint8x8x3_t o0_u8x8x3, o1_u8x8x3;
        d += 8;
        o0_u8x8x3.val[0] = op2_u8x8;
        o0_u8x8x3.val[1] = op1_u8x8;
        o0_u8x8x3.val[2] = op0_u8x8;
        o1_u8x8x3.val[0] = oq0_u8x8;
        o1_u8x8x3.val[1] = oq1_u8x8;
        o1_u8x8x3.val[2] = oq2_u8x8;
        vst3_lane_u8(d - 3, o0_u8x8x3, 0);
        vst3_lane_u8(d + 0, o1_u8x8x3, 0);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 1);
        vst3_lane_u8(d + 0, o1_u8x8x3, 1);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 2);
        vst3_lane_u8(d + 0, o1_u8x8x3, 2);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 3);
        vst3_lane_u8(d + 0, o1_u8x8x3, 3);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 4);
        vst3_lane_u8(d + 0, o1_u8x8x3, 4);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 5);
        vst3_lane_u8(d + 0, o1_u8x8x3, 5);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 6);
        vst3_lane_u8(d + 0, o1_u8x8x3, 6);
        d += p;
        vst3_lane_u8(d - 3, o0_u8x8x3, 7);
        vst3_lane_u8(d + 0, o1_u8x8x3, 7);
        d += p;
        d -= 8;
      }
    } else {
      uint8x8x4_t o_u8x8x4;
      d += 6;
      o_u8x8x4.val[0] = op1_u8x8;
      o_u8x8x4.val[1] = op0_u8x8;
      o_u8x8x4.val[2] = oq0_u8x8;
      o_u8x8x4.val[3] = oq1_u8x8;
      vst4_lane_u8(d, o_u8x8x4, 0);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 1);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 2);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 3);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 4);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 5);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 6);
      d += p;
      vst4_lane_u8(d, o_u8x8x4, 7);
      d += p;
      d -= 6;
    }
  } while (--count);
}

void vpx_lpf_vertical_16_neon(uint8_t *s, int p, const uint8_t *blimit,
                              const uint8_t *limit, const uint8_t *thresh) {
  mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 1);
}

void vpx_lpf_vertical_16_dual_neon(uint8_t *s, int p, const uint8_t *blimit,
                                   const uint8_t *limit,
                                   const uint8_t *thresh) {
  mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 2);
}
