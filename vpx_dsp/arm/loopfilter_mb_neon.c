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
#include "./vpx_dsp_rtcd.h"

// should we apply any filter at all: 11111111 yes, 00000000 no
static INLINE uint8x8_t filter_mask_neon(
    uint8x8_t limit_u8x8, uint8x8_t blimit_u8x8, uint8x8_t thresh_u8x8,
    uint8x8_t p3_u8x8, uint8x8_t p2_u8x8, uint8x8_t p1_u8x8, uint8x8_t p0_u8x8,
    uint8x8_t q0_u8x8, uint8x8_t q1_u8x8, uint8x8_t q2_u8x8, uint8x8_t q3_u8x8,
    uint8x8_t *flat_u8x8, uint8x8_t *hev_u8x8) {
  uint8x8_t t0_u8x8, t1_u8x8;
  uint8x8_t max_u8x8 = vabd_u8(p1_u8x8, p0_u8x8);
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q1_u8x8, q0_u8x8));

  // is there high edge variance internal edge: 11111111 yes, 00000000 no
  *hev_u8x8 = vcgt_u8(max_u8x8, thresh_u8x8);
  *flat_u8x8 = vmax_u8(max_u8x8, vabd_u8(p2_u8x8, p0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(p3_u8x8, p2_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(p2_u8x8, p1_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q2_u8x8, q1_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q3_u8x8, q2_u8x8));
  t0_u8x8 = vabd_u8(p0_u8x8, q0_u8x8);
  t1_u8x8 = vabd_u8(p1_u8x8, q1_u8x8);
  t0_u8x8 = vqshl_n_u8(t0_u8x8, 1);
  t1_u8x8 = vshr_n_u8(t1_u8x8, 1);
  t0_u8x8 = vqadd_u8(t0_u8x8, t1_u8x8);
  max_u8x8 = vcle_u8(max_u8x8, limit_u8x8);
  t0_u8x8 = vcle_u8(t0_u8x8, blimit_u8x8);
  max_u8x8 = vand_u8(max_u8x8, t0_u8x8);

  *flat_u8x8 = vmax_u8(*flat_u8x8, vabd_u8(q2_u8x8, q0_u8x8));
  *flat_u8x8 = vmax_u8(*flat_u8x8, vabd_u8(p3_u8x8, p0_u8x8));
  *flat_u8x8 = vmax_u8(*flat_u8x8, vabd_u8(q3_u8x8, q0_u8x8));
  *flat_u8x8 = vcle_u8(*flat_u8x8, vdup_n_u8(1));

  return max_u8x8;
}

static INLINE uint8x8_t flat_mask5_neon(uint8x8_t p4_u8x8, uint8x8_t p3_u8x8,
                                        uint8x8_t p2_u8x8, uint8x8_t p1_u8x8,
                                        uint8x8_t p0_u8x8, uint8x8_t q0_u8x8,
                                        uint8x8_t q1_u8x8, uint8x8_t q2_u8x8,
                                        uint8x8_t q3_u8x8, uint8x8_t q4_u8x8) {
  uint8x8_t max_u8x8 = vabd_u8(p4_u8x8, p0_u8x8);
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(p3_u8x8, p0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(p2_u8x8, p0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(p1_u8x8, p0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q1_u8x8, q0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q2_u8x8, q0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q3_u8x8, q0_u8x8));
  max_u8x8 = vmax_u8(max_u8x8, vabd_u8(q4_u8x8, q0_u8x8));
  max_u8x8 = vcle_u8(max_u8x8, vdup_n_u8(1));

  return max_u8x8;
}

static INLINE uint8x8_t filter_neon(uint16x8_t *o_u16x8, uint8x8_t flat_u8x8,
                                    uint8x8_t sub0_u8x8, uint8x8_t sub1_u8x8,
                                    uint8x8_t add0_u8x8, uint8x8_t add1_u8x8,
                                    uint8x8_t i_u8x8, const int shift) {
  *o_u16x8 = vsubw_u8(*o_u16x8, sub0_u8x8);
  *o_u16x8 = vsubw_u8(*o_u16x8, sub1_u8x8);
  *o_u16x8 = vaddw_u8(*o_u16x8, add0_u8x8);
  *o_u16x8 = vaddw_u8(*o_u16x8, add1_u8x8);
  return vbsl_u8(flat_u8x8, vrshrn_n_u16(*o_u16x8, shift), i_u8x8);
}

static INLINE void filter16_neon(
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
    const uint8x8_t c_u8x8 = vdup_n_u8(0x80);
    int8x8_t filter_s8x8, filter1_s8x8, filter2_s8x8, t_s8x8;
    const int8x8_t ps1_s8x8 = vreinterpret_s8_u8(veor_u8(p1_u8x8, c_u8x8));
    const int8x8_t ps0_s8x8 = vreinterpret_s8_u8(veor_u8(p0_u8x8, c_u8x8));
    const int8x8_t qs0_s8x8 = vreinterpret_s8_u8(veor_u8(q0_u8x8, c_u8x8));
    const int8x8_t qs1_s8x8 = vreinterpret_s8_u8(veor_u8(q1_u8x8, c_u8x8));

    filter_s8x8 = vqsub_s8(ps1_s8x8, qs1_s8x8);
    filter_s8x8 = vand_s8(filter_s8x8, vreinterpret_s8_u8(hev_u8x8));
    t_s8x8 = vqsub_s8(qs0_s8x8, ps0_s8x8);

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

    *oq0_u8x8 = vreinterpret_u8_s8(vqsub_s8(qs0_s8x8, filter1_s8x8));
    *op0_u8x8 = vreinterpret_u8_s8(vqadd_s8(ps0_s8x8, filter2_s8x8));
    *oq0_u8x8 = veor_u8(*oq0_u8x8, c_u8x8);
    *op0_u8x8 = veor_u8(*op0_u8x8, c_u8x8);

    // outer tap adjustments
    filter_s8x8 = vrshr_n_s8(filter1_s8x8, 1);
    filter_s8x8 = vbic_s8(filter_s8x8, vreinterpret_s8_u8(hev_u8x8));

    *oq1_u8x8 = vreinterpret_u8_s8(vqsub_s8(qs1_s8x8, filter_s8x8));
    *op1_u8x8 = vreinterpret_u8_s8(vqadd_s8(ps1_s8x8, filter_s8x8));
    *oq1_u8x8 = veor_u8(*oq1_u8x8, c_u8x8);
    *op1_u8x8 = veor_u8(*op1_u8x8, c_u8x8);
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
      // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
      uint16x8_t o_u16x8;
      o_u16x8 = vaddl_u8(p3_u8x8, p3_u8x8);  // 2*p3
      o_u16x8 = vaddw_u8(o_u16x8, p3_u8x8);  // 3*p3
      o_u16x8 = vaddw_u8(o_u16x8, p2_u8x8);  // 3*p3+p2
      o_u16x8 = vaddw_u8(o_u16x8, p2_u8x8);  // 3*p3+2*p2
      o_u16x8 = vaddw_u8(o_u16x8, p1_u8x8);  // 3*p3+2*p2+p1
      o_u16x8 = vaddw_u8(o_u16x8, p0_u8x8);  // 3*p3+2*p2+p1+p0
      o_u16x8 = vaddw_u8(o_u16x8, q0_u8x8);  // 3*p3+2*p2+p1+p0+q0
      *op2_u8x8 = vbsl_u8(flat_u8x8, vrshrn_n_u16(o_u16x8, 3), p2_u8x8);
      *op1_u8x8 = filter_neon(&o_u16x8, flat_u8x8, p3_u8x8, p2_u8x8, p1_u8x8,
                              q1_u8x8, *op1_u8x8, 3);
      *op0_u8x8 = filter_neon(&o_u16x8, flat_u8x8, p3_u8x8, p1_u8x8, p0_u8x8,
                              q2_u8x8, *op0_u8x8, 3);
      *oq0_u8x8 = filter_neon(&o_u16x8, flat_u8x8, p3_u8x8, p0_u8x8, q0_u8x8,
                              q3_u8x8, *oq0_u8x8, 3);
      *oq1_u8x8 = filter_neon(&o_u16x8, flat_u8x8, p2_u8x8, q0_u8x8, q1_u8x8,
                              q3_u8x8, *oq1_u8x8, 3);
      *oq2_u8x8 = filter_neon(&o_u16x8, flat_u8x8, p1_u8x8, q1_u8x8, q2_u8x8,
                              q3_u8x8, q2_u8x8, 3);
    }
    if (flat2_u64) {
      // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
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
      *op5_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p6_u8x8, p5_u8x8,
                              q1_u8x8, p5_u8x8, 4);
      *op4_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p5_u8x8, p4_u8x8,
                              q2_u8x8, p4_u8x8, 4);
      *op3_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p4_u8x8, p3_u8x8,
                              q3_u8x8, p3_u8x8, 4);
      *op2_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p3_u8x8, p2_u8x8,
                              q4_u8x8, *op2_u8x8, 4);
      *op1_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p2_u8x8, p1_u8x8,
                              q5_u8x8, *op1_u8x8, 4);
      *op0_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p1_u8x8, p0_u8x8,
                              q6_u8x8, *op0_u8x8, 4);
      *oq0_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p7_u8x8, p0_u8x8, q0_u8x8,
                              q7_u8x8, *oq0_u8x8, 4);
      *oq1_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p6_u8x8, q0_u8x8, q1_u8x8,
                              q7_u8x8, *oq1_u8x8, 4);
      *oq2_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p5_u8x8, q1_u8x8, q2_u8x8,
                              q7_u8x8, *oq2_u8x8, 4);
      *oq3_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p4_u8x8, q2_u8x8, q3_u8x8,
                              q7_u8x8, q3_u8x8, 4);
      *oq4_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p3_u8x8, q3_u8x8, q4_u8x8,
                              q7_u8x8, q4_u8x8, 4);
      *oq5_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p2_u8x8, q4_u8x8, q5_u8x8,
                              q7_u8x8, q5_u8x8, 4);
      *oq6_u8x8 = filter_neon(&o_u16x8, flat2_u8x8, p1_u8x8, q5_u8x8, q6_u8x8,
                              q7_u8x8, q6_u8x8, 4);
    }
  }
}

static void mb_lpf_horizontal_edge_w_neon(uint8_t *s, int p,
                                          const uint8_t *blimit,
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
    uint8x8_t mask_u8x8 = filter_mask_neon(
        limit_u8x8, blimit_u8x8, thresh_u8x8, p3_u8x8, p2_u8x8, p1_u8x8,
        p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, &flat_u8x8, &hev_u8x8);
    uint8x8_t flat2_u8x8 =
        flat_mask5_neon(p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p0_u8x8, q0_u8x8,
                        q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8);
    uint64_t flat_u64, flat2_u64;

    flat_u8x8 = vand_u8(flat_u8x8, mask_u8x8);
    flat2_u8x8 = vand_u8(flat2_u8x8, flat_u8x8);
    flat_u64 = vget_lane_u64(vreinterpret_u64_u8(flat_u8x8), 0);
    flat2_u64 = vget_lane_u64(vreinterpret_u64_u8(flat2_u8x8), 0);

    filter16_neon(mask_u8x8, *thresh, flat_u8x8, flat_u64, flat2_u8x8,
                  flat2_u64, hev_u8x8, p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8,
                  p3_u8x8, p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8,
                  q3_u8x8, q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8, &op6_u8x8,
                  &op5_u8x8, &op4_u8x8, &op3_u8x8, &op2_u8x8, &op1_u8x8,
                  &op0_u8x8, &oq0_u8x8, &oq1_u8x8, &oq2_u8x8, &oq3_u8x8,
                  &oq4_u8x8, &oq5_u8x8, &oq6_u8x8);

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
  mb_lpf_horizontal_edge_w_neon(s, p, blimit, limit, thresh, 1);
}

void vpx_lpf_horizontal_edge_16_neon(uint8_t *s, int p, const uint8_t *blimit,
                                     const uint8_t *limit,
                                     const uint8_t *thresh) {
  mb_lpf_horizontal_edge_w_neon(s, p, blimit, limit, thresh, 2);
}

static void mb_lpf_vertical_edge_w_neon(uint8_t *s, int p,
                                        const uint8_t *blimit,
                                        const uint8_t *limit,
                                        const uint8_t *thresh, int count) {
  const uint8x8_t blimit_u8x8 = vld1_dup_u8(blimit);
  const uint8x8_t limit_u8x8 = vld1_dup_u8(limit);
  const uint8x8_t thresh_u8x8 = vld1_dup_u8(thresh);
  uint8_t *d;

  s -= 8;
  d = s;
  do {
    uint8x16_t t0_u8x16, t1_u8x16, t2_u8x16, t3_u8x16, t4_u8x16, t5_u8x16,
        t6_u8x16, t7_u8x16;
    uint8x16x2_t t0_u8x16x2, t1_u8x16x2, t2_u8x16x2, t3_u8x16x2;
    uint16x8x2_t t0_u16x8x2, t1_u16x8x2, t2_u16x8x2, t3_u16x8x2;
    uint32x4x2_t t0_u32x4x2, t1_u32x4x2, t2_u32x4x2, t3_u32x4x2;
    uint8x8_t p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p3_u8x8, p2_u8x8, p1_u8x8,
        p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8, q3_u8x8, q4_u8x8, q5_u8x8, q6_u8x8,
        q7_u8x8, op6_u8x8, op5_u8x8, op4_u8x8, op3_u8x8, op2_u8x8, op1_u8x8,
        op0_u8x8, oq0_u8x8, oq1_u8x8, oq2_u8x8, oq3_u8x8, oq4_u8x8, oq5_u8x8,
        oq6_u8x8, flat_u8x8, hev_u8x8, mask_u8x8, flat2_u8x8;
    uint64_t flat_u64, flat2_u64;

    // 16x8 8-bit transpose.
    // t0_u8x16: 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F
    // t1_u8x16: 10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F
    // t2_u8x16: 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F
    // t3_u8x16: 30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F
    // t4_u8x16: 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F
    // t5_u8x16: 50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F
    // t6_u8x16: 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F
    // t7_u8x16: 70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F
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

    // t0_u8x16x2: 00 10 02 12 04 14 06 16  08 18 0A 1A 0C 1C 0E 1E
    //             01 11 03 13 05 15 07 17  09 19 0B 1B 0D 1D 0F 1F
    // t1_u8x16x2: 20 30 22 32 24 34 26 36  28 38 2A 3A 2C 3C 2E 3E
    //             21 31 23 33 25 35 27 37  29 39 2B 3B 2D 3D 2F 3F
    // t2_u8x16x2: 40 50 42 52 44 54 46 56  48 58 4A 5A 4C 5C 4E 5E
    //             41 51 43 53 45 55 47 57  49 59 4B 5B 4D 5D 4F 5F
    // t3_u8x16x2: 60 70 62 72 64 74 66 76  68 78 6A 7A 6C 7C 6E 7E
    //             61 71 63 73 65 75 67 77  69 79 6B 7B 6D 7D 6F 7F
    t0_u8x16x2 = vtrnq_u8(t0_u8x16, t1_u8x16);
    t1_u8x16x2 = vtrnq_u8(t2_u8x16, t3_u8x16);
    t2_u8x16x2 = vtrnq_u8(t4_u8x16, t5_u8x16);
    t3_u8x16x2 = vtrnq_u8(t6_u8x16, t7_u8x16);

    // t0_u16x8x2: 00 10 20 30 04 14 24 34  08 18 28 38 0C 1C 2C 3C
    //             02 12 22 32 06 16 26 36  0A 1A 2A 3A 0E 1E 2E 3E
    // t1_u16x8x2: 01 11 21 31 05 15 25 35  09 19 29 39 0D 1D 2D 3D
    //             03 13 23 33 07 17 27 37  0B 1B 2B 3B 0F 1F 2F 3F
    // t2_u16x8x2: 40 50 60 70 44 54 64 74  48 58 68 78 4C 5C 6C 7C
    //             42 52 62 72 46 56 66 76  4A 5A 6A 7A 4E 5E 6E 7E
    // t3_u16x8x2: 41 51 61 71 45 55 65 75  49 59 69 79 4D 5D 6D 7D
    //             43 53 63 73 47 57 67 77  4B 5B 6B 7B 4F 5F 6F 7F
    t0_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u8(t0_u8x16x2.val[0]),
                           vreinterpretq_u16_u8(t1_u8x16x2.val[0]));
    t1_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u8(t0_u8x16x2.val[1]),
                           vreinterpretq_u16_u8(t1_u8x16x2.val[1]));
    t2_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u8(t2_u8x16x2.val[0]),
                           vreinterpretq_u16_u8(t3_u8x16x2.val[0]));
    t3_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u8(t2_u8x16x2.val[1]),
                           vreinterpretq_u16_u8(t3_u8x16x2.val[1]));

    // t0_u32x4x2: 00 10 20 30 40 50 60 70  08 18 28 38 48 58 68 78  P7 Q0
    //             04 14 24 34 44 54 64 74  0C 1C 2C 3C 4C 5C 6C 7C  P3 Q4
    // t1_u32x4x2: 02 12 22 32 42 52 62 72  0A 1A 2A 3A 4A 5A 6A 7A  P5 Q2
    //             06 16 26 36 46 56 66 76  0E 1E 2E 3E 4E 5E 6E 7E  P1 Q6
    // t2_u32x4x2: 01 11 21 31 41 51 61 71  09 19 29 39 49 59 69 79  P6 Q1
    //             05 15 25 35 45 55 65 75  0D 1D 2D 3D 4D 5D 6D 7D  P2 Q5
    // t3_u32x4x2: 03 13 23 33 43 53 63 73  0B 1B 2B 3B 4B 5B 6B 7B  P4 Q3
    //             07 17 27 37 47 57 67 77  0F 1F 2F 3F 4F 5F 6F 7F  P0 Q7
    t0_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t0_u16x8x2.val[0]),
                           vreinterpretq_u32_u16(t2_u16x8x2.val[0]));
    t1_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t0_u16x8x2.val[1]),
                           vreinterpretq_u32_u16(t2_u16x8x2.val[1]));
    t2_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t1_u16x8x2.val[0]),
                           vreinterpretq_u32_u16(t3_u16x8x2.val[0]));
    t3_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t1_u16x8x2.val[1]),
                           vreinterpretq_u32_u16(t3_u16x8x2.val[1]));

    p7_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[0]));
    p6_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[0]));
    p5_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[0]));
    p4_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[0]));
    p3_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[1]));
    p2_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[1]));
    p1_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[1]));
    p0_u8x8 = vget_low_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[1]));
    q0_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[0]));
    q1_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[0]));
    q2_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[0]));
    q3_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[0]));
    q4_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[1]));
    q5_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[1]));
    q6_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[1]));
    q7_u8x8 = vget_high_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[1]));
    mask_u8x8 = filter_mask_neon(limit_u8x8, blimit_u8x8, thresh_u8x8, p3_u8x8,
                                 p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8,
                                 q2_u8x8, q3_u8x8, &flat_u8x8, &hev_u8x8);
    flat2_u8x8 = flat_mask5_neon(p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8, p0_u8x8,
                                 q0_u8x8, q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8);
    flat_u8x8 = vand_u8(flat_u8x8, mask_u8x8);
    flat2_u8x8 = vand_u8(flat2_u8x8, flat_u8x8);
    flat_u64 = vget_lane_u64(vreinterpret_u64_u8(flat_u8x8), 0);
    flat2_u64 = vget_lane_u64(vreinterpret_u64_u8(flat2_u8x8), 0);

    filter16_neon(mask_u8x8, *thresh, flat_u8x8, flat_u64, flat2_u8x8,
                  flat2_u64, hev_u8x8, p7_u8x8, p6_u8x8, p5_u8x8, p4_u8x8,
                  p3_u8x8, p2_u8x8, p1_u8x8, p0_u8x8, q0_u8x8, q1_u8x8, q2_u8x8,
                  q3_u8x8, q4_u8x8, q5_u8x8, q6_u8x8, q7_u8x8, &op6_u8x8,
                  &op5_u8x8, &op4_u8x8, &op3_u8x8, &op2_u8x8, &op1_u8x8,
                  &op0_u8x8, &oq0_u8x8, &oq1_u8x8, &oq2_u8x8, &oq3_u8x8,
                  &oq4_u8x8, &oq5_u8x8, &oq6_u8x8);

    if (flat_u64) {
      if (flat2_u64) {
        // t0_u32x4x2: 00 10 20 30 40 50 60 70  08 18 28 38 48 58 68 78  P7 Q0
        //             04 14 24 34 44 54 64 74  0C 1C 2C 3C 4C 5C 6C 7C  P3 Q4
        // t1_u32x4x2: 02 12 22 32 42 52 62 72  0A 1A 2A 3A 4A 5A 6A 7A  P5 Q2
        //             06 16 26 36 46 56 66 76  0E 1E 2E 3E 4E 5E 6E 7E  P1 Q6
        // t2_u32x4x2: 01 11 21 31 41 51 61 71  09 19 29 39 49 59 69 79  P6 Q1
        //             05 15 25 35 45 55 65 75  0D 1D 2D 3D 4D 5D 6D 7D  P2 Q5
        // t3_u32x4x2: 03 13 23 33 43 53 63 73  0B 1B 2B 3B 4B 5B 6B 7B  P4 Q3
        //             07 17 27 37 47 57 67 77  0F 1F 2F 3F 4F 5F 6F 7F  P0 Q7
        t0_u32x4x2.val[0] =
            vreinterpretq_u32_u8(vcombine_u8(p7_u8x8, oq0_u8x8));
        t0_u32x4x2.val[1] =
            vreinterpretq_u32_u8(vcombine_u8(op3_u8x8, oq4_u8x8));
        t1_u32x4x2.val[0] =
            vreinterpretq_u32_u8(vcombine_u8(op5_u8x8, oq2_u8x8));
        t1_u32x4x2.val[1] =
            vreinterpretq_u32_u8(vcombine_u8(op1_u8x8, oq6_u8x8));
        t2_u32x4x2.val[0] =
            vreinterpretq_u32_u8(vcombine_u8(op6_u8x8, oq1_u8x8));
        t2_u32x4x2.val[1] =
            vreinterpretq_u32_u8(vcombine_u8(op2_u8x8, oq5_u8x8));
        t3_u32x4x2.val[0] =
            vreinterpretq_u32_u8(vcombine_u8(op4_u8x8, oq3_u8x8));
        t3_u32x4x2.val[1] =
            vreinterpretq_u32_u8(vcombine_u8(op0_u8x8, q7_u8x8));

        // t0_u32x4x2: 00 10 20 30 04 14 24 34  08 18 28 38 0C 1C 2C 3C
        //             40 50 60 70 44 54 64 74  48 58 68 78 4C 5C 6C 7C
        // t1_u32x4x2: 02 12 22 32 06 16 26 36  0A 1A 2A 3A 0E 1E 2E 3E
        //             42 52 62 72 46 56 66 76  4A 5A 6A 7A 4E 5E 6E 7E
        // t2_u32x4x2: 01 11 21 31 05 15 25 35  09 19 29 39 0D 1D 2D 3D
        //             41 51 61 71 45 55 65 75  49 59 69 79 4D 5D 6D 7D
        // t3_u32x4x2: 03 13 23 33 07 17 27 37  0B 1B 2B 3B 0F 1F 2F 3F
        //             43 53 63 73 47 57 67 77  4B 5B 6B 7B 4F 5F 6F 7F
        t0_u32x4x2 = vtrnq_u32(t0_u32x4x2.val[0], t0_u32x4x2.val[1]);
        t1_u32x4x2 = vtrnq_u32(t1_u32x4x2.val[0], t1_u32x4x2.val[1]);
        t2_u32x4x2 = vtrnq_u32(t2_u32x4x2.val[0], t2_u32x4x2.val[1]);
        t3_u32x4x2 = vtrnq_u32(t3_u32x4x2.val[0], t3_u32x4x2.val[1]);

        // t0_u16x8x2: 00 10 02 12 04 14 06 16  08 18 0A 1A 0C 1C 0E 1E
        //             20 30 22 32 24 34 26 36  28 38 2A 3A 2C 3C 2E 3E
        // t1_u16x8x2: 01 11 03 13 05 15 07 17  09 19 0B 1B 0D 1D 0F 1F
        //             21 31 23 33 25 35 27 37  29 39 2B 3B 2D 3D 2F 3F
        // t2_u16x8x2: 40 50 42 52 44 54 46 56  48 58 4A 5A 4C 5C 4E 5E
        //             60 70 62 72 64 74 66 76  68 78 6A 7A 6C 7C 6E 7E
        // t3_u16x8x2: 41 51 43 53 45 55 47 57  49 59 4B 5B 4D 5D 4F 5F
        //             61 71 63 73 65 75 67 77  69 79 6B 7B 6D 7D 6F 7F
        t0_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u32(t0_u32x4x2.val[0]),
                               vreinterpretq_u16_u32(t1_u32x4x2.val[0]));
        t1_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u32(t2_u32x4x2.val[0]),
                               vreinterpretq_u16_u32(t3_u32x4x2.val[0]));
        t2_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u32(t0_u32x4x2.val[1]),
                               vreinterpretq_u16_u32(t1_u32x4x2.val[1]));
        t3_u16x8x2 = vtrnq_u16(vreinterpretq_u16_u32(t2_u32x4x2.val[1]),
                               vreinterpretq_u16_u32(t3_u32x4x2.val[1]));

        // t0_u8x16: 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F
        //           10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F
        // t1_u8x16: 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F
        //           30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F
        // t2_u8x16: 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F
        //           50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F
        // t3_u8x16: 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F
        //           70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F
        t0_u8x16x2 = vtrnq_u8(vreinterpretq_u8_u16(t0_u16x8x2.val[0]),
                              vreinterpretq_u8_u16(t1_u16x8x2.val[0]));
        t1_u8x16x2 = vtrnq_u8(vreinterpretq_u8_u16(t0_u16x8x2.val[1]),
                              vreinterpretq_u8_u16(t1_u16x8x2.val[1]));
        t2_u8x16x2 = vtrnq_u8(vreinterpretq_u8_u16(t2_u16x8x2.val[0]),
                              vreinterpretq_u8_u16(t3_u16x8x2.val[0]));
        t3_u8x16x2 = vtrnq_u8(vreinterpretq_u8_u16(t2_u16x8x2.val[1]),
                              vreinterpretq_u8_u16(t3_u16x8x2.val[1]));

        vst1q_u8(d, t0_u8x16x2.val[0]);
        d += p;
        vst1q_u8(d, t0_u8x16x2.val[1]);
        d += p;
        vst1q_u8(d, t1_u8x16x2.val[0]);
        d += p;
        vst1q_u8(d, t1_u8x16x2.val[1]);
        d += p;
        vst1q_u8(d, t2_u8x16x2.val[0]);
        d += p;
        vst1q_u8(d, t2_u8x16x2.val[1]);
        d += p;
        vst1q_u8(d, t3_u8x16x2.val[0]);
        d += p;
        vst1q_u8(d, t3_u8x16x2.val[1]);
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
  mb_lpf_vertical_edge_w_neon(s, p, blimit, limit, thresh, 1);
}

// #if !HAVE_NEON_ASM
void vpx_lpf_vertical_16_dual_neon(uint8_t *s, int p, const uint8_t *blimit,
                                   const uint8_t *limit,
                                   const uint8_t *thresh) {
  mb_lpf_vertical_edge_w_neon(s, p, blimit, limit, thresh, 2);
}
// #endif  // !HAVE_NEON_ASM
