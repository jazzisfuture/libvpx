/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// For abs
#include <stdlib.h>
#include <assert.h>

#include "./vpx_dsp_rtcd.h"

#include "vpx_ports/mem.h"
// For clamp
#include "vpx_dsp/vpx_dsp_common.h"

#include "vpx_dsp/ppc/types_vsx.h"

#define DELETEME_USE_C 0

static INLINE int8_t signed_char_clamp(int t) {
  return (int8_t)clamp(t, -128, 127);
}

// Is there high edge variance internal edge: 11111111 yes, 00000000 no
static INLINE int8_t hev_mask(uint8_t thresh, uint8_t p1, uint8_t p0,
                              uint8_t q0, uint8_t q1) {
  int8_t hev = 0;
  hev |= (abs(p1 - p0) > thresh) * -1;
  hev |= (abs(q1 - q0) > thresh) * -1;
  return hev;
}

static INLINE void filter4(int8_t mask, uint8_t thresh, uint8_t *op1,
                           uint8_t *op0, uint8_t *oq0, uint8_t *oq1) {
  int8_t filter1, filter2;

  const int8_t ps1 = (int8_t)*op1 ^ 0x80;
  const int8_t ps0 = (int8_t)*op0 ^ 0x80;
  const int8_t qs0 = (int8_t)*oq0 ^ 0x80;
  const int8_t qs1 = (int8_t)*oq1 ^ 0x80;
  const uint8_t hev = hev_mask(thresh, *op1, *op0, *oq0, *oq1);

  // add outer taps if we have high edge variance
  int8_t filter = signed_char_clamp(ps1 - qs1) & hev;

  // inner taps
  filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;

  // save bottom 3 bits so that we round one side +4 and the other +3
  // if it equals 4 we'll set it to adjust by -1 to account for the fact
  // we'd round it by 3 the other way
  filter1 = signed_char_clamp(filter + 4) >> 3;
  filter2 = signed_char_clamp(filter + 3) >> 3;

  *oq0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
  *op0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

  // outer tap adjustments
  filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

  *oq1 = signed_char_clamp(qs1 - filter) ^ 0x80;
  *op1 = signed_char_clamp(ps1 + filter) ^ 0x80;
}

static INLINE void filter8(int8_t mask, uint8_t thresh, uint8_t flat,
                           uint8_t *op3, uint8_t *op2, uint8_t *op1,
                           uint8_t *op0, uint8_t *oq0, uint8_t *oq1,
                           uint8_t *oq2, uint8_t *oq3) {
  if (flat && mask) {
    const uint8_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;

    // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
    *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
    *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
    *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
    *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
    *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
    *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
  } else {
    filter4(mask, thresh, op1, op0, oq0, oq1);
  }
}

#if DELETEME_USE_C

// Should we apply any filter at all: 11111111 yes, 00000000 no
static INLINE int8_t filter_mask(uint8_t limit, uint8_t blimit, uint8_t p3,
                                 uint8_t p2, uint8_t p1, uint8_t p0, uint8_t q0,
                                 uint8_t q1, uint8_t q2, uint8_t q3) {
  int8_t mask = 0;
  mask |= (abs(p3 - p2) > limit) * -1;
  mask |= (abs(p2 - p1) > limit) * -1;
  mask |= (abs(p1 - p0) > limit) * -1;
  mask |= (abs(q1 - q0) > limit) * -1;
  mask |= (abs(q2 - q1) > limit) * -1;
  mask |= (abs(q3 - q2) > limit) * -1;
  mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2 > blimit) * -1;
  return ~mask;
}

static INLINE int8_t flat_mask4(uint8_t thresh, uint8_t p3, uint8_t p2,
                                uint8_t p1, uint8_t p0, uint8_t q0, uint8_t q1,
                                uint8_t q2, uint8_t q3) {
  int8_t mask = 0;
  mask |= (abs(p1 - p0) > thresh) * -1;
  mask |= (abs(q1 - q0) > thresh) * -1;
  mask |= (abs(p2 - p0) > thresh) * -1;
  mask |= (abs(q2 - q0) > thresh) * -1;
  mask |= (abs(p3 - p0) > thresh) * -1;
  mask |= (abs(q3 - q0) > thresh) * -1;
  return ~mask;
}

static INLINE int8_t flat_mask5(uint8_t thresh, uint8_t p4, uint8_t p3,
                                uint8_t p2, uint8_t p1, uint8_t p0, uint8_t q0,
                                uint8_t q1, uint8_t q2, uint8_t q3,
                                uint8_t q4) {
  int8_t mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
  mask |= (abs(p4 - p0) > thresh) * -1;
  mask |= (abs(q4 - q0) > thresh) * -1;
  return ~mask;
}

static INLINE void filter16(int8_t mask, uint8_t thresh, uint8_t flat,
                            uint8_t flat2, uint8_t *op7, uint8_t *op6,
                            uint8_t *op5, uint8_t *op4, uint8_t *op3,
                            uint8_t *op2, uint8_t *op1, uint8_t *op0,
                            uint8_t *oq0, uint8_t *oq1, uint8_t *oq2,
                            uint8_t *oq3, uint8_t *oq4, uint8_t *oq5,
                            uint8_t *oq6, uint8_t *oq7) {
  if (flat2 && flat && mask) {
    const uint8_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4, p3 = *op3,
                  p2 = *op2, p1 = *op1, p0 = *op0;

    const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3, q4 = *oq4,
                  q5 = *oq5, q6 = *oq6, q7 = *oq7;

    // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
    *op6 = ROUND_POWER_OF_TWO(
        p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 + q0, 4);
    *op5 = ROUND_POWER_OF_TWO(
        p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 + q0 + q1, 4);
    *op4 = ROUND_POWER_OF_TWO(
        p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 + q0 + q1 + q2, 4);
    *op3 = ROUND_POWER_OF_TWO(
        p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 + q0 + q1 + q2 + q3, 4);
    *op2 = ROUND_POWER_OF_TWO(
        p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 + q0 + q1 + q2 + q3 + q4,
        4);
    *op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5,
                              4);
    *op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 + q0 +
                                  q1 + q2 + q3 + q4 + q5 + q6,
                              4);
    *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 + q0 * 2 + q1 +
                                  q2 + q3 + q4 + q5 + q6 + q7,
                              4);
    *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 + q0 + q1 * 2 + q2 +
                                  q3 + q4 + q5 + q6 + q7 * 2,
                              4);
    *oq2 = ROUND_POWER_OF_TWO(
        p4 + p3 + p2 + p1 + p0 + q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3,
        4);
    *oq3 = ROUND_POWER_OF_TWO(
        p3 + p2 + p1 + p0 + q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
    *oq4 = ROUND_POWER_OF_TWO(
        p2 + p1 + p0 + q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
    *oq5 = ROUND_POWER_OF_TWO(
        p1 + p0 + q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
    *oq6 = ROUND_POWER_OF_TWO(
        p0 + q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
  } else {
    filter8(mask, thresh, flat, op3, op2, op1, op0, oq0, oq1, oq2, oq3);
  }
}

static void mb_lpf_horizontal_edge_w(uint8_t *s, int p, const uint8_t *blimit,
                                     const uint8_t *limit,
                                     const uint8_t *thresh) {
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 16; ++i) {
    const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
    const int8_t mask =
        filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat2 =
        flat_mask5(1, s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0, q0,
                   s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

    filter16(mask, *thresh, flat, flat2, s - 8 * p, s - 7 * p, s - 6 * p,
             s - 5 * p, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p, s,
             s + 1 * p, s + 2 * p, s + 3 * p, s + 4 * p, s + 5 * p, s + 6 * p,
             s + 7 * p);
    ++s;
  }
}
#else

static INLINE bool8x16_t flat_mask4(uint8x16_t p3, uint8x16_t p2, uint8x16_t p1,
                                    uint8x16_t p0, uint8x16_t q0, uint8x16_t q1,
                                    uint8x16_t q2, uint8x16_t q3) {
  const uint8x16_t dp1 = vec_sub(vec_max(p1, p0), vec_min(p1, p0));
  const uint8x16_t dq1 = vec_sub(vec_max(q1, q0), vec_min(q1, q0));
  const uint8x16_t dp2 = vec_sub(vec_max(p2, p0), vec_min(p2, p0));
  const uint8x16_t dq2 = vec_sub(vec_max(q2, q0), vec_min(q2, q0));
  const uint8x16_t dp3 = vec_sub(vec_max(p3, p0), vec_min(p3, p0));
  const uint8x16_t dq3 = vec_sub(vec_max(q3, q0), vec_min(q3, q0));

  const uint8x16_t max_dpq1 = vec_max(dp1, dq1);
  const uint8x16_t max_dpq2 = vec_max(dp2, dq2);
  const uint8x16_t max_dpq3 = vec_max(dp3, dq3);
  const uint8x16_t max = vec_max(vec_max(max_dpq1, max_dpq2), max_dpq3);

  return vec_cmple(max, vec_ones_u8);
}

static INLINE bool8x16_t flat_mask5(uint8x16_t p4, uint8x16_t p3, uint8x16_t p2,
                                    uint8x16_t p1, uint8x16_t p0, uint8x16_t q0,
                                    uint8x16_t q1, uint8x16_t q2, uint8x16_t q3,
                                    uint8x16_t q4) {
  const uint8x16_t dp1 = vec_sub(vec_max(p1, p0), vec_min(p1, p0));
  const uint8x16_t dq1 = vec_sub(vec_max(q1, q0), vec_min(q1, q0));
  const uint8x16_t dp2 = vec_sub(vec_max(p2, p0), vec_min(p2, p0));
  const uint8x16_t dq2 = vec_sub(vec_max(q2, q0), vec_min(q2, q0));
  const uint8x16_t dp3 = vec_sub(vec_max(p3, p0), vec_min(p3, p0));
  const uint8x16_t dq3 = vec_sub(vec_max(q3, q0), vec_min(q3, q0));
  const uint8x16_t dp4 = vec_sub(vec_max(p4, p0), vec_min(p4, p0));
  const uint8x16_t dq4 = vec_sub(vec_max(q4, q0), vec_min(q4, q0));

  const uint8x16_t max_dpq1 = vec_max(dp1, dq1);
  const uint8x16_t max_dpq2 = vec_max(dp2, dq2);
  const uint8x16_t max_dpq3 = vec_max(dp3, dq3);
  const uint8x16_t max_dpq4 = vec_max(dp4, dq4);
  const uint8x16_t max =
      vec_max(vec_max(max_dpq1, max_dpq2), vec_max(max_dpq3, max_dpq4));

  return vec_cmple(max, vec_ones_u8);
}

static INLINE bool8x16_t filter_mask(const uint8_t *limit,
                                     const uint8_t *blimit, uint8x16_t p3,
                                     uint8x16_t p2, uint8x16_t p1,
                                     uint8x16_t p0, uint8x16_t q0,
                                     uint8x16_t q1, uint8x16_t q2,
                                     uint8x16_t q3) {
  const uint8x16_t vec_limit = vec_splats(*limit);
  const uint8x16_t vec_blimit = vec_splats(*blimit);

  // TODO(ltrudeau) POWER9 has a new abs diff operator
  const uint8x16_t dp3 = vec_sub(vec_max(p3, p2), vec_min(p3, p2));
  const uint8x16_t dp2 = vec_sub(vec_max(p2, p1), vec_min(p2, p1));
  const uint8x16_t dp1 = vec_sub(vec_max(p1, p0), vec_min(p1, p0));
  const uint8x16_t dq1 = vec_sub(vec_max(q1, q0), vec_min(q1, q0));
  const uint8x16_t dq2 = vec_sub(vec_max(q2, q1), vec_min(q2, q1));
  const uint8x16_t dq3 = vec_sub(vec_max(q3, q2), vec_min(q3, q2));
  const uint8x16_t dpq0 = vec_sub(vec_max(p0, q0), vec_min(p0, q0));
  const uint8x16_t dpq1 = vec_sub(vec_max(p1, q1), vec_min(p1, q1));

  const uint8x16_t max_dp32 = vec_max(dp3, dp2);
  const uint8x16_t max_dq32 = vec_max(dq3, dq2);
  const uint8x16_t max =
      vec_max(vec_max(dp1, dq1), vec_max(max_dp32, max_dq32));

  // Since blimit is uint8 we use saturated adds to avoid having to use 16-bit
  // intermidiaries.
  const uint8x16_t sum =
      vec_adds(vec_adds(dpq0, dpq0), vec_sr(dpq1, vec_splats((uint8_t)1)));
  // However, this only works when blimit < 255. When sum == 255, we don't know
  // if it's really 255 or if satured.
  assert(*blimit < 255);

  // We reduce the number of operations from the C code
  // ~(mask |= (abs(X - Y) > limit))
  // is equivalent to
  // mask &= (abs(X - Y) <= limit)
  return vec_and(vec_cmple(sum, vec_blimit), vec_cmple(max, vec_limit));
}
/*
static INLINE void filter16(bool8x16_t mask, uint8x16_t thresh, bool8x16_t flat,
                            bool8x16_t flat2, uint8x16_t *op7, uint8x16_t *op6,
                            uint8x16_t *op5, uint8x16_t *op4, uint8x16_t *op3,
                            uint8x16_t *op2, uint8x16_t *op1, uint8x16_t *op0,
                            uint8x16_t *oq0, uint8x16_t *oq1, uint8x16_t *oq2,
                            uint8x16_t *oq3, uint8x16_t *oq4, uint8x16_t *oq5,
                            uint8x16_t *oq6, uint8x16_t *oq7) {
  for (int i = 0; i < 16; i++) {
    const uint8_t p7 = (*op7)[i], p6 = (*op6)[i], p5 = (*op5)[i],
    p4 = (*op4)[i], p3 = (*op3)[i], p2 = (*op2)[i],
    p1 = (*op1)[i], p0 = (*op0)[i];

    const uint8_t q0 = (*oq0)[i], q1 = (*oq1)[i], q2 = (*oq2)[i],
    q3 = (*oq3)[i], q4 = (*oq4)[i], q5 = (*oq5)[i],
    q6 = (*oq6)[i], q7 = (*oq7)[i];
    if (flat2[i] && flat[i] && mask[i]) {

      // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
      (*op6)[i] = ROUND_POWER_OF_TWO(
          p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 + q0, 4);
      (*op5)[i] = ROUND_POWER_OF_TWO(
          p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 + q0 + q1, 4);
      (*op4)[i] = ROUND_POWER_OF_TWO(
          p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 + q0 + q1 + q2, 4);
      (*op3)[i] = ROUND_POWER_OF_TWO(
          p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 + q0 + q1 + q2 + q3, 4);
      (*op2)[i] = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 +
                                         p0 + q0 + q1 + q2 + q3 + q4,
                                     4);
      (*op1)[i] = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 +
                                         p0 + q0 + q1 + q2 + q3 + q4 + q5,
                                     4);
      (*op0)[i] = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                                         q0 + q1 + q2 + q3 + q4 + q5 + q6,
                                     4);
      (*oq0)[i] = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 + q0 * 2 +
                                         q1 + q2 + q3 + q4 + q5 + q6 + q7,
                                     4);
      (*oq1)[i] = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 + q0 + q1 * 2 +
                                         q2 + q3 + q4 + q5 + q6 + q7 * 2,
                                     4);
      (*oq2)[i] = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 + q0 + q1 + q2 * 2 +
                                         q3 + q4 + q5 + q6 + q7 * 3,
                                     4);
      (*oq3)[i] = ROUND_POWER_OF_TWO(
          p3 + p2 + p1 + p0 + q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
      (*oq4)[i] = ROUND_POWER_OF_TWO(
          p2 + p1 + p0 + q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
      (*oq5)[i] = ROUND_POWER_OF_TWO(
          p1 + p0 + q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
      (*oq6)[i] = ROUND_POWER_OF_TWO(
          p0 + q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
    } else if (flat[i] && mask[i]) {
      // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
      *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
      *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
      *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
      *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
      *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
      *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
    } else {
      int8_t filter1, filter2;

      const int8_t ps1 = (int8_t)*op1 ^ 0x80;
      const int8_t ps0 = (int8_t)*op0 ^ 0x80;
      const int8_t qs0 = (int8_t)*oq0 ^ 0x80;
      const int8_t qs1 = (int8_t)*oq1 ^ 0x80;
      const uint8_t hev = hev_mask(thresh, *op1, *op0, *oq0, *oq1);

      // add outer taps if we have high edge variance
      int8_t filter = signed_char_clamp(ps1 - qs1) & hev;

      // inner taps
      filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;

      // save bottom 3 bits so that we round one side +4 and the other +3
      // if it equals 4 we'll set it to adjust by -1 to account for the fact
      // we'd round it by 3 the other way
      filter1 = signed_char_clamp(filter + 4) >> 3;
      filter2 = signed_char_clamp(filter + 3) >> 3;

      *oq0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
      *op0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

      // outer tap adjustments
      filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

      *oq1 = signed_char_clamp(qs1 - filter) ^ 0x80;
      *op1 = signed_char_clamp(ps1 + filter) ^ 0x80;
    }
    filter8(mask[i], thresh[i], flat[i], &(op3[i]), &(op2[i]), &(op1[i]),
            &(op0[i]), &(oq0[i]), &(oq1[i]), &(oq2[i]), &(oq3[i]));
  }
}
*/
#endif  // DELETEME_USE_C

static INLINE uint16x8_t vec_round_power_of_two_u16(uint16x8_t v) {
  static const uint16x8_t offset = vec_splats((uint16_t)8);
  static const uint16x8_t sr = vec_splats((uint16_t)4);
  return vec_sr(vec_add(v, offset), sr);
}

static INLINE uint16x8_t vec_round_power_of_two_8(uint16x8_t v) {
  static const uint16x8_t offset = vec_splats((uint16_t)4);
  static const uint16x8_t sr = vec_splats((uint16_t)3);
  return vec_sr(vec_add(v, offset), sr);
}

static INLINE uint16x8_t vec_unpacke(uint8x16_t v) {
  static const uint16x8_t mask = vec_splats((uint16_t)0x00FF);
  return vec_and((uint16x8_t)v, mask);
}

static INLINE uint16x8_t vec_unpacko(uint8x16_t v) {
  static const uint16x8_t sr8 = vec_splats((uint16_t)8);
  return vec_sr((uint16x8_t)v, sr8);
}

static INLINE uint16x8_t move_window(uint16x8_t sum, uint16x8_t prev1,
                                     uint16x8_t prev2, uint16x8_t next1,
                                     uint16x8_t next2) {
  const uint16x8_t n = vec_add(next1, next2);
  const uint16x8_t p = vec_add(prev1, prev2);
  return vec_sub(vec_add(sum, n), p);
}

static INLINE uint8x16_t combine_window(uint16x8_t win_e, uint16x8_t win_o,
                                        uint8x16_t orig, bool8x16_t mask) {
  static const uint8x16_t perm_pack = {
    0x00, 0x10, 0x02, 0x12, 0x04, 0x14, 0x06, 0x16,
    0x08, 0x18, 0x0A, 0x1A, 0x0C, 0x1C, 0x0E, 0x1E,
  };

  const uint8x16_t win =
      (uint8x16_t)vec_perm(vec_round_power_of_two_u16(win_e),
                           vec_round_power_of_two_u16(win_o), perm_pack);

  return vec_sel(orig, win, mask);
}

static INLINE uint8x16_t combine_window_f8(uint16x8_t win_e, uint16x8_t win_o,
                                           uint8x16_t orig, bool8x16_t mask) {
  static const uint8x16_t perm_pack = {
    0x00, 0x10, 0x02, 0x12, 0x04, 0x14, 0x06, 0x16,
    0x08, 0x18, 0x0A, 0x1A, 0x0C, 0x1C, 0x0E, 0x1E,
  };

  const uint8x16_t win =
      (uint8x16_t)vec_perm(vec_round_power_of_two_8(win_e),
                           vec_round_power_of_two_8(win_o), perm_pack);

  return vec_sel(orig, win, mask);
}

void vpx_lpf_horizontal_16_dual_vsx(uint8_t *s, int p, const uint8_t *blimit,
                                    const uint8_t *limit,
                                    const uint8_t *thresh) {
#if DELETEME_USE_C
  mb_lpf_horizontal_edge_w(s, p, blimit, limit, thresh);
#else

  const uint8x16_t p7 = vec_vsx_ld(0, s + -8 * p);
  const uint8x16_t p6 = vec_vsx_ld(0, s + -7 * p);
  const uint8x16_t p5 = vec_vsx_ld(0, s + -6 * p);
  const uint8x16_t p4 = vec_vsx_ld(0, s + -5 * p);
  const uint8x16_t p3 = vec_vsx_ld(0, s + -4 * p);
  const uint8x16_t p2 = vec_vsx_ld(0, s + -3 * p);
  const uint8x16_t p1 = vec_vsx_ld(0, s + -2 * p);
  const uint8x16_t p0 = vec_vsx_ld(0, s + -p);

  const uint8x16_t q0 = vec_vsx_ld(0, s + 0 * p);
  const uint8x16_t q1 = vec_vsx_ld(0, s + 1 * p);
  const uint8x16_t q2 = vec_vsx_ld(0, s + 2 * p);
  const uint8x16_t q3 = vec_vsx_ld(0, s + 3 * p);
  const uint8x16_t q4 = vec_vsx_ld(0, s + 4 * p);
  const uint8x16_t q5 = vec_vsx_ld(0, s + 5 * p);
  const uint8x16_t q6 = vec_vsx_ld(0, s + 6 * p);
  const uint8x16_t q7 = vec_vsx_ld(0, s + 7 * p);

  const bool8x16_t mask =
      filter_mask(limit, blimit, p3, p2, p1, p0, q0, q1, q2, q3);
  const bool8x16_t flat = flat_mask4(p3, p2, p1, p0, q0, q1, q2, q3);
  const bool8x16_t filter8_mask = vec_and(flat, mask);
  const bool8x16_t flat2 = flat_mask5(p7, p6, p5, p4, p0, q0, q4, q5, q6, q7);
  const bool8x16_t filter16_mask = vec_and(filter8_mask, flat2);

  // filter16(mask, *thresh, flat, flat2, p7, p6, p5, p4, p3, p2, p1, p0, q0,
  // q1,
  //         q2, q3, q4, q5, q6, q7);

  /*  for (int i = 0; i < 16; ++i) {
      // const int8_t flat =
      //    flat_mask4(1, p3[i], p2[i], p1[i], p0[i], q0[i], q1[i], q2[i],
    q3[i]);
      // const int8_t flat2 =
      //    flat_mask5(1, s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0[i],
      //    q0[i],
      //               s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

      filter16(mask[i], *thresh, flat[i], flat2[i], s - 8 * p, s - 7 * p,
               s - 6 * p, s - 5 * p, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
               s, s + 1 * p, s + 2 * p, s + 3 * p, s + 4 * p, s + 5 * p,
               s + 6 * p, s + 7 * p);
      ++s;
    }
  */

  // OD6
  // p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 + q0
  uint16x8_t m7p7_e = vec_mule(p7, vec_splats((uint8_t)7));
  uint16x8_t m7p7_o = vec_mulo(p7, vec_splats((uint8_t)7));

  uint16x8_t m2p6_e = vec_mule(p6, vec_splats((uint8_t)2));
  uint16x8_t m2p6_o = vec_mulo(p6, vec_splats((uint8_t)2));

  uint16x8_t p5_e = vec_unpacke(p5);
  uint16x8_t p5_o = vec_unpacko(p5);
  uint16x8_t p4_e = vec_unpacke(p4);
  uint16x8_t p4_o = vec_unpacko(p4);
  uint16x8_t p3_e = vec_unpacke(p3);
  uint16x8_t p3_o = vec_unpacko(p3);
  uint16x8_t p2_e = vec_unpacke(p2);
  uint16x8_t p2_o = vec_unpacko(p2);
  uint16x8_t p1_e = vec_unpacke(p1);
  uint16x8_t p1_o = vec_unpacko(p1);
  uint16x8_t p0_e = vec_unpacke(p0);
  uint16x8_t p0_o = vec_unpacko(p0);

  uint16x8_t p54_e = vec_add(p5_e, p4_e);
  uint16x8_t p54_o = vec_add(p5_o, p4_o);

  uint16x8_t p32_e = vec_add(p3_e, p2_e);
  uint16x8_t p32_o = vec_add(p3_o, p2_o);

  uint16x8_t p10_e = vec_add(p1_e, p0_e);
  uint16x8_t p10_o = vec_add(p1_o, p0_o);

  uint16x8_t q0_e = vec_unpacke(q0);
  uint16x8_t q0_o = vec_unpacko(q0);

  uint16x8_t p76_e = vec_add(m7p7_e, m2p6_e);
  uint16x8_t p76_o = vec_add(m7p7_o, m2p6_o);
  uint16x8_t p5432_e = vec_add(p54_e, p32_e);
  uint16x8_t p5432_o = vec_add(p54_o, p32_o);
  uint16x8_t p10q0_e = vec_add(p10_e, q0_e);
  uint16x8_t p10q0_o = vec_add(p10_o, q0_o);

  uint16x8_t p765432_e = vec_add(p76_e, p5432_e);
  uint16x8_t p765432_o = vec_add(p76_o, p5432_o);
  uint16x8_t win_e = vec_add(p765432_e, p10q0_e);
  uint16x8_t win_o = vec_add(p765432_o, p10q0_o);

  uint8x16_t op6 = combine_window(win_e, win_o, p6, filter16_mask);

  // OP5
  // p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 + q0 + q1
  uint16x8_t p7_e = vec_unpacke(p7);
  uint16x8_t p7_o = vec_unpacko(p7);

  uint16x8_t p6_e = vec_unpacke(p6);
  uint16x8_t p6_o = vec_unpacko(p6);

  uint16x8_t q1_e = vec_unpacke(q1);
  uint16x8_t q1_o = vec_unpacko(q1);

  win_e = move_window(win_e, p7_e, p6_e, q1_e, p5_e);
  win_o = move_window(win_o, p7_o, p6_o, q1_o, p5_o);
  uint8x16_t op5 = combine_window(win_e, win_o, p5, filter16_mask);

  // OP4
  // p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 + q0 + q1 + q2
  uint16x8_t q2_e = vec_unpacke(q2);
  uint16x8_t q2_o = vec_unpacko(q2);

  win_e = move_window(win_e, p7_e, p5_e, q2_e, p4_e);
  win_o = move_window(win_o, p7_o, p5_o, q2_o, p4_o);

  uint8x16_t op4 = combine_window(win_e, win_o, p4, filter16_mask);

  // OP3
  // p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 + q0 + q1 + q2 + q3
  uint16x8_t q3_e = vec_unpacke(q3);
  uint16x8_t q3_o = vec_unpacko(q3);

  win_e = move_window(win_e, p7_e, p4_e, q3_e, p3_e);
  win_o = move_window(win_o, p7_o, p4_o, q3_o, p3_o);

  uint8x16_t op3 = combine_window(win_e, win_o, p3, filter16_mask);

  // OP2 (FILTER8)
  // p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0
  uint16x8_t m3p3_e = vec_add(vec_add(p3_e, p3_e), p3_e);
  uint16x8_t m3p3_o = vec_add(vec_add(p3_o, p3_o), p3_o);

  uint16x8_t m2p2_e = vec_add(p2_e, p2_e);
  uint16x8_t m2p2_o = vec_add(p2_o, p2_o);

  uint16x8_t win8_e = vec_add(vec_add(m3p3_e, m2p2_e), p10q0_e);
  uint16x8_t win8_o = vec_add(vec_add(m3p3_o, m2p2_o), p10q0_o);

  uint8x16_t op2 = combine_window_f8(win8_e, win8_o, p2, filter8_mask);

  // OP2 (FILTER 16)
  // p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 + q0 + q1 + q2 + q3 + q4
  uint16x8_t q4_e = vec_unpacke(q4);
  uint16x8_t q4_o = vec_unpacko(q4);

  win_e = move_window(win_e, p7_e, p3_e, q4_e, p2_e);
  win_o = move_window(win_o, p7_o, p3_o, q4_o, p2_o);

  op2 = combine_window(win_e, win_o, op2, filter16_mask);

  // OP1 (FILTER 8)
  // p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1
  win8_e = move_window(win8_e, p3_e, p2_e, p1_e, q1_e);
  win8_o = move_window(win8_o, p3_o, p2_o, p1_o, q1_o);

  uint8x16_t op1 = combine_window_f8(win8_e, win8_o, p1, filter8_mask);

  // OP1 (FILTER 16)
  // p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 + q0 + q1 + q2 + q3 + q4 + q5
  uint16x8_t q5_e = vec_unpacke(q5);
  uint16x8_t q5_o = vec_unpacko(q5);

  win_e = move_window(win_e, p7_e, p2_e, q5_e, p1_e);
  win_o = move_window(win_o, p7_o, p2_o, q5_o, p1_o);

  op1 = combine_window(win_e, win_o, op1, filter16_mask);

  // OP0 (FILTER 8)
  // p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2
  win8_e = move_window(win8_e, p3_e, p1_e, p0_e, q2_e);
  win8_o = move_window(win8_o, p3_o, p1_o, p0_o, q2_o);

  uint8x16_t op0 = combine_window_f8(win8_e, win8_o, p0, filter8_mask);

  // OP0 (FILTER 16)
  // p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 + q0 + q1 + q2 + q3 + q4 + q5 +
  // q6
  uint16x8_t q6_e = vec_unpacke(q6);
  uint16x8_t q6_o = vec_unpacko(q6);

  win_e = move_window(win_e, p7_e, p1_e, q6_e, p0_e);
  win_o = move_window(win_o, p7_o, p1_o, q6_o, p0_o);

  op0 = combine_window(win_e, win_o, op0, filter16_mask);

  // OQ0 (FILTER 8)
  // p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3
  win8_e = move_window(win8_e, p3_e, p0_e, q0_e, q3_e);
  win8_o = move_window(win8_o, p3_o, p0_o, q0_o, q3_o);

  uint8x16_t oq0 = combine_window_f8(win8_e, win8_o, q0, filter8_mask);

  // OQ0 (FILTER 16)
  // p6 + p5 + p4 + p3 + p2 + p1 + p0 + q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 +
  // q7
  uint16x8_t q7_e = vec_unpacke(q7);
  uint16x8_t q7_o = vec_unpacko(q7);

  win_e = move_window(win_e, p7_e, p0_e, q7_e, q0_e);
  win_o = move_window(win_o, p7_o, p0_o, q7_o, q0_o);

  oq0 = combine_window(win_e, win_o, oq0, filter16_mask);

  // OQ1 (FILTER 8)
  // p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3
  win8_e = move_window(win8_e, p2_e, q0_e, q1_e, q3_e);
  win8_o = move_window(win8_o, p2_o, q0_o, q1_o, q3_o);

  uint8x16_t oq1 = combine_window_f8(win8_e, win8_o, q1, filter8_mask);

  // OQ1 (FILTER 16)
  // p5 + p4 + p3 + p2 + p1 + p0 + q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2
  win_e = move_window(win_e, p6_e, q0_e, q7_e, q1_e);
  win_o = move_window(win_o, p6_o, q0_o, q7_o, q1_o);

  oq1 = combine_window(win_e, win_o, oq1, filter16_mask);

  // OQ2 (FILTER 8)
  // p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3
  win8_e = move_window(win8_e, p1_e, q1_e, q2_e, q3_e);
  win8_o = move_window(win8_o, p1_o, q1_o, q2_o, q3_o);

  uint8x16_t oq2 = combine_window_f8(win8_e, win8_o, q2, filter8_mask);

  // OQ2 (FILTER16)
  // p4 + p3 + p2 + p1 + p0 + q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3
  win_e = move_window(win_e, p5_e, q1_e, q7_e, q2_e);
  win_o = move_window(win_o, p5_o, q1_o, q7_o, q2_o);

  oq2 = combine_window(win_e, win_o, oq2, filter16_mask);

  // OQ3
  // p3 + p2 + p1 + p0 + q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4
  win_e = move_window(win_e, p4_e, q2_e, q7_e, q3_e);
  win_o = move_window(win_o, p4_o, q2_o, q7_o, q3_o);

  uint8x16_t oq3 = combine_window(win_e, win_o, q3, filter16_mask);

  // OQ4
  // p2 + p1 + p0 + q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5
  win_e = move_window(win_e, p3_e, q3_e, q7_e, q4_e);
  win_o = move_window(win_o, p3_o, q3_o, q7_o, q4_o);

  uint8x16_t oq4 = combine_window(win_e, win_o, q4, filter16_mask);

  // OQ5
  // p1 + p0 + q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6
  win_e = move_window(win_e, p2_e, q4_e, q7_e, q5_e);
  win_o = move_window(win_o, p2_o, q4_o, q7_o, q5_o);

  uint8x16_t oq5 = combine_window(win_e, win_o, q5, filter16_mask);

  // OQ6
  // p0 + q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7
  win_e = move_window(win_e, p1_e, q5_e, q7_e, q6_e);
  win_o = move_window(win_o, p1_o, q5_o, q7_o, q6_o);

  uint8x16_t oq6 = combine_window(win_e, win_o, q6, filter16_mask);

  const uint8x16_t dp1 = vec_sub(vec_max(p1, p0), vec_min(p1, p0));
  const uint8x16_t dq1 = vec_sub(vec_max(q1, q0), vec_min(q1, q0));
  const uint8x16_t max_dpq1 = vec_max(dp1, dq1);
  bool8x16_t hev = vec_cmpgt(max_dpq1, vec_splats(*thresh));

  const int8x16_t ps1 = vec_xor((int8x16_t)p1, vec_splats((int8_t)0x80));
  const int8x16_t ps0 = vec_xor((int8x16_t)p0, vec_splats((int8_t)0x80));
  const int8x16_t qs0 = vec_xor((int8x16_t)q0, vec_splats((int8_t)0x80));
  const int8x16_t qs1 = vec_xor((int8x16_t)q1, vec_splats((int8_t)0x80));

  // Causes qemu-ppc64le to core dump
  const int8x16_t psqs1 = vec_subs(ps1, qs1);

  for (int i = 0; i < 16; i++) {
    if (flat2[i] && flat[i] && mask[i]) {
    } else if (flat[i] && mask[i]) {
    } else {
      int8_t filter, filter1, filter2;

      // const int8_t ps1 = (int8_t)p1[i] ^ 0x80;
      // const int8_t ps0 = (int8_t)p0[i] ^ 0x80;
      // const int8_t qs0 = (int8_t)q0[i] ^ 0x80;
      // const int8_t qs1 = (int8_t)q1[i] ^ 0x80;

      // add outer taps if we have high edge variance
      printf("(%d %d => %d %d)%d -> %d | %d\n", p1[i], q1[i], ps1[i], qs1[i],
             ps1[i] - qs1[i], signed_char_clamp(ps1[i] - qs1[i]), psqs1[i]);
      filter = signed_char_clamp(ps1[i] - qs1[i]) & hev[i];

      // inner taps
      filter = signed_char_clamp(filter + 3 * (qs0[i] - ps0[i])) & mask[i];

      // save bottom 3 bits so that we round one side +4 and the
      // other +3 if it equals 4 we'll set it to adjust by -1 to
      // account for the fact we'd round it by 3 the other way
      filter1 = signed_char_clamp(filter + 4) >> 3;
      filter2 = signed_char_clamp(filter + 3) >> 3;

      oq0[i] = signed_char_clamp(qs0[i] - filter1) ^ 0x80;
      op0[i] = signed_char_clamp(ps0[i] + filter2) ^ 0x80;

      // outer tap adjustments
      filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev[i];

      oq1[i] = signed_char_clamp(qs1[i] - filter) ^ 0x80;
      op1[i] = signed_char_clamp(ps1[i] + filter) ^ 0x80;
    }
  }

  vec_vsx_st(op6, 0, s + -7 * p);
  vec_vsx_st(op5, 0, s + -6 * p);
  vec_vsx_st(op4, 0, s + -5 * p);
  vec_vsx_st(op3, 0, s + -4 * p);
  vec_vsx_st(op2, 0, s + -3 * p);
  vec_vsx_st(op1, 0, s + -2 * p);
  vec_vsx_st(op0, 0, s + -p);

  vec_vsx_st(oq0, 0, s + 0 * p);
  vec_vsx_st(oq1, 0, s + 1 * p);
  vec_vsx_st(oq2, 0, s + 2 * p);
  vec_vsx_st(oq3, 0, s + 3 * p);
  vec_vsx_st(oq4, 0, s + 4 * p);
  vec_vsx_st(oq5, 0, s + 5 * p);
  vec_vsx_st(oq6, 0, s + 6 * p);

#endif
}
