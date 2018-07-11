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

#include "vpx_ports/mem.h"

#include "vpx_dsp/ppc/types_vsx.h"

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

static INLINE uint8x16_t s8_to_u8(int8x16_t a) {
  return (uint8x16_t)vec_xor(a, vec_splats((int8_t)0x80));
}

static INLINE int8x16_t u8_to_s8(uint8x16_t a) {
  return vec_xor((int8x16_t)a, vec_splats((int8_t)0x80));
}

void vpx_lpf_horizontal_16_dual_vsx(uint8_t *s, int p, const uint8_t *blimit,
                                    const uint8_t *limit,
                                    const uint8_t *thresh) {
  const uint8x16_t p7 = vec_vsx_ld(0, s - 8 * p);
  const uint8x16_t p6 = vec_vsx_ld(0, s - 7 * p);
  const uint8x16_t p5 = vec_vsx_ld(0, s - 6 * p);
  const uint8x16_t p4 = vec_vsx_ld(0, s - 5 * p);
  const uint8x16_t p3 = vec_vsx_ld(0, s - 4 * p);
  const uint8x16_t p2 = vec_vsx_ld(0, s - 3 * p);
  const uint8x16_t p1 = vec_vsx_ld(0, s - 2 * p);
  const uint8x16_t p0 = vec_vsx_ld(0, s - p);
  const uint8x16_t q0 = vec_vsx_ld(0, s);
  const uint8x16_t q1 = vec_vsx_ld(0, s + p);
  const uint8x16_t q2 = vec_vsx_ld(0, s + 2 * p);
  const uint8x16_t q3 = vec_vsx_ld(0, s + 3 * p);
  const uint8x16_t q4 = vec_vsx_ld(0, s + 4 * p);
  const uint8x16_t q5 = vec_vsx_ld(0, s + 5 * p);
  const uint8x16_t q6 = vec_vsx_ld(0, s + 6 * p);
  const uint8x16_t q7 = vec_vsx_ld(0, s + 7 * p);

  const bool8x16_t mask =
      filter_mask(limit, blimit, p3, p2, p1, p0, q0, q1, q2, q3);
  const bool8x16_t filter8_mask =
      vec_and(flat_mask4(p3, p2, p1, p0, q0, q1, q2, q3), mask);
  const bool8x16_t filter16_mask =
      vec_and(filter8_mask, flat_mask5(p7, p6, p5, p4, p0, q0, q4, q5, q6, q7));

  // Unpack ps and qs to 16 bits
  const uint16x8_t p7_e = vec_unpacke(p7);
  const uint16x8_t p7_o = vec_unpacko(p7);
  const uint16x8_t p6_e = vec_unpacke(p6);
  const uint16x8_t p6_o = vec_unpacko(p6);
  const uint16x8_t p5_e = vec_unpacke(p5);
  const uint16x8_t p5_o = vec_unpacko(p5);
  const uint16x8_t p4_e = vec_unpacke(p4);
  const uint16x8_t p4_o = vec_unpacko(p4);
  const uint16x8_t p3_e = vec_unpacke(p3);
  const uint16x8_t p3_o = vec_unpacko(p3);
  const uint16x8_t p2_e = vec_unpacke(p2);
  const uint16x8_t p2_o = vec_unpacko(p2);
  const uint16x8_t p1_e = vec_unpacke(p1);
  const uint16x8_t p1_o = vec_unpacko(p1);
  const uint16x8_t p0_e = vec_unpacke(p0);
  const uint16x8_t p0_o = vec_unpacko(p0);
  const uint16x8_t q0_e = vec_unpacke(q0);
  const uint16x8_t q0_o = vec_unpacko(q0);
  const uint16x8_t q1_e = vec_unpacke(q1);
  const uint16x8_t q1_o = vec_unpacko(q1);
  const uint16x8_t q2_e = vec_unpacke(q2);
  const uint16x8_t q2_o = vec_unpacko(q2);
  const uint16x8_t q3_e = vec_unpacke(q3);
  const uint16x8_t q3_o = vec_unpacko(q3);
  const uint16x8_t q4_e = vec_unpacke(q4);
  const uint16x8_t q4_o = vec_unpacko(q4);
  const uint16x8_t q5_e = vec_unpacke(q5);
  const uint16x8_t q5_o = vec_unpacko(q5);
  const uint16x8_t q6_e = vec_unpacke(q6);
  const uint16x8_t q6_o = vec_unpacko(q6);
  const uint16x8_t q7_e = vec_unpacke(q7);
  const uint16x8_t q7_o = vec_unpacko(q7);

  // p1 + p0 + q0
  const uint16x8_t p10q0_e = vec_add(vec_add(p1_e, p0_e), q0_e);
  const uint16x8_t p10q0_o = vec_add(vec_add(p1_o, p0_o), q0_o);

  // Sliding windows
  uint16x8_t win_e, win_o;
  uint16x8_t win8_e, win8_o;

  uint8x16_t comb;

  // Filter 4 filters
  int8x16_t p0_filter, q0_filter, filter4;
  const int8x16_t ps1 = u8_to_s8(p1);
  const int8x16_t ps0 = u8_to_s8(p0);
  const int8x16_t qs0 = u8_to_s8(q0);
  const int8x16_t qs1 = u8_to_s8(q1);

  // OP6
  // p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 + q0
  {  // Fill Filter16 Sliding Window
    const uint16x8_t m7p7_e = vec_mule(p7, vec_splats((uint8_t)7));
    const uint16x8_t m7p7_o = vec_mulo(p7, vec_splats((uint8_t)7));
    const uint16x8_t m2p6_e = vec_add(p6_e, p6_e);
    const uint16x8_t m2p6_o = vec_add(p6_o, p6_o);

    const uint16x8_t p76_e = vec_add(m7p7_e, m2p6_e);
    const uint16x8_t p76_o = vec_add(m7p7_o, m2p6_o);
    const uint16x8_t p5432_e =
        vec_add(vec_add(p5_e, p4_e), vec_add(p3_e, p2_e));
    const uint16x8_t p5432_o =
        vec_add(vec_add(p5_o, p4_o), vec_add(p3_o, p2_o));

    const uint16x8_t p765432_e = vec_add(p76_e, p5432_e);
    const uint16x8_t p765432_o = vec_add(p76_o, p5432_o);

    win_e = vec_add(p765432_e, p10q0_e);
    win_o = vec_add(p765432_o, p10q0_o);
  }
  vec_vsx_st(combine_window(win_e, win_o, p6, filter16_mask), 0, s - 7 * p);

  // OP5
  // p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 + q0 + q1
  win_e = move_window(win_e, p7_e, p6_e, q1_e, p5_e);
  win_o = move_window(win_o, p7_o, p6_o, q1_o, p5_o);
  vec_vsx_st(combine_window(win_e, win_o, p5, filter16_mask), 0, s - 6 * p);

  // OP4
  // p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 + q0 + q1 + q2
  win_e = move_window(win_e, p7_e, p5_e, q2_e, p4_e);
  win_o = move_window(win_o, p7_o, p5_o, q2_o, p4_o);
  vec_vsx_st(combine_window(win_e, win_o, p4, filter16_mask), 0, s - 5 * p);

  // OP3
  // p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 + q0 + q1 + q2 + q3
  win_e = move_window(win_e, p7_e, p4_e, q3_e, p3_e);
  win_o = move_window(win_o, p7_o, p4_o, q3_o, p3_o);
  vec_vsx_st(combine_window(win_e, win_o, p3, filter16_mask), 0, s - 4 * p);

  // OP2 (FILTER8)
  // p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0
  {  // Fill Filter8 Sliding Window
    const uint16x8_t m3p3_e = vec_add(vec_add(p3_e, p3_e), p3_e);
    const uint16x8_t m3p3_o = vec_add(vec_add(p3_o, p3_o), p3_o);

    win8_e = vec_add(vec_add(m3p3_e, vec_add(p2_e, p2_e)), p10q0_e);
    win8_o = vec_add(vec_add(m3p3_o, vec_add(p2_o, p2_o)), p10q0_o);
  }
  comb = combine_window_f8(win8_e, win8_o, p2, filter8_mask);

  // OP2 (FILTER 16)
  // p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 + q0 + q1 + q2 + q3 + q4
  win_e = move_window(win_e, p7_e, p3_e, q4_e, p2_e);
  win_o = move_window(win_o, p7_o, p3_o, q4_o, p2_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s - 3 * p);

  // OP1 (FILTER 4)
  {  // Compute Filter 4 filters
    const uint8x16_t dp1 = vec_sub(vec_max(p1, p0), vec_min(p1, p0));
    const uint8x16_t dq1 = vec_sub(vec_max(q1, q0), vec_min(q1, q0));
    const bool8x16_t hev = vec_cmpgt(vec_max(dp1, dq1), vec_splats(*thresh));

    const int8x16_t qsps0 = vec_subs(qs0, ps0);
    const int8x16_t psqs1 = vec_subs(ps1, qs1);

    // equivalent to
    // filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask
    filter4 = vec_and(psqs1, hev);
    filter4 = vec_adds(filter4, qsps0);
    filter4 = vec_adds(filter4, qsps0);
    filter4 = vec_adds(filter4, qsps0);
    filter4 = vec_and(filter4, mask);

    q0_filter = vec_sra(vec_adds(filter4, vec_splats((int8_t)4)),
                        vec_splats((uint8_t)3));
    p0_filter = vec_sra(vec_adds(filter4, vec_splats((int8_t)3)),
                        vec_splats((uint8_t)3));
    filter4 =
        vec_andc(vec_sra(vec_add(q0_filter, vec_ones_s8), vec_ones_u8), hev);
  }
  comb = s8_to_u8(vec_adds(ps1, filter4));

  // OP1 (FILTER 8)
  // p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1
  win8_e = move_window(win8_e, p3_e, p2_e, p1_e, q1_e);
  win8_o = move_window(win8_o, p3_o, p2_o, p1_o, q1_o);
  comb = combine_window_f8(win8_e, win8_o, comb, filter8_mask);

  // OP1 (FILTER 16)
  // p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 + q0 + q1 + q2 + q3 + q4 + q5
  win_e = move_window(win_e, p7_e, p2_e, q5_e, p1_e);
  win_o = move_window(win_o, p7_o, p2_o, q5_o, p1_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s - 2 * p);

  // OP0 (FILTER 4)
  comb =
      (uint8x16_t)vec_xor(vec_adds(ps0, p0_filter), vec_splats((int8_t)0x80));

  // OP0 (FILTER 8)
  // p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2
  win8_e = move_window(win8_e, p3_e, p1_e, p0_e, q2_e);
  win8_o = move_window(win8_o, p3_o, p1_o, p0_o, q2_o);
  comb = combine_window_f8(win8_e, win8_o, comb, filter8_mask);

  // OP0 (FILTER 16)
  // p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 + q0 + q1 + q2 + q3 + q4 + q5 +
  // q6
  win_e = move_window(win_e, p7_e, p1_e, q6_e, p0_e);
  win_o = move_window(win_o, p7_o, p1_o, q6_o, p0_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s - p);

  // OQ0 (FILTER 4)
  comb =
      (uint8x16_t)vec_xor(vec_subs(qs0, q0_filter), vec_splats((int8_t)0x80));

  // OQ0 (FILTER 8)
  // p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3
  win8_e = move_window(win8_e, p3_e, p0_e, q0_e, q3_e);
  win8_o = move_window(win8_o, p3_o, p0_o, q0_o, q3_o);
  comb = combine_window_f8(win8_e, win8_o, comb, filter8_mask);

  // OQ0 (FILTER 16)
  // p6 + p5 + p4 + p3 + p2 + p1 + p0 + q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 +
  // q7
  win_e = move_window(win_e, p7_e, p0_e, q7_e, q0_e);
  win_o = move_window(win_o, p7_o, p0_o, q7_o, q0_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s);

  // OQ1 (FILTER 4)
  comb = (uint8x16_t)vec_xor(vec_subs(qs1, filter4), vec_splats((int8_t)0x80));

  // OQ1 (FILTER 8)
  // p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3
  win8_e = move_window(win8_e, p2_e, q0_e, q1_e, q3_e);
  win8_o = move_window(win8_o, p2_o, q0_o, q1_o, q3_o);
  comb = combine_window_f8(win8_e, win8_o, comb, filter8_mask);

  // OQ1 (FILTER 16)
  // p5 + p4 + p3 + p2 + p1 + p0 + q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2
  win_e = move_window(win_e, p6_e, q0_e, q7_e, q1_e);
  win_o = move_window(win_o, p6_o, q0_o, q7_o, q1_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s + p);

  // OQ2 (FILTER 8)
  // p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3
  win8_e = move_window(win8_e, p1_e, q1_e, q2_e, q3_e);
  win8_o = move_window(win8_o, p1_o, q1_o, q2_o, q3_o);
  comb = combine_window_f8(win8_e, win8_o, q2, filter8_mask);

  // OQ2 (FILTER16)
  // p4 + p3 + p2 + p1 + p0 + q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3
  win_e = move_window(win_e, p5_e, q1_e, q7_e, q2_e);
  win_o = move_window(win_o, p5_o, q1_o, q7_o, q2_o);
  vec_vsx_st(combine_window(win_e, win_o, comb, filter16_mask), 0, s + 2 * p);

  // OQ3
  // p3 + p2 + p1 + p0 + q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4
  win_e = move_window(win_e, p4_e, q2_e, q7_e, q3_e);
  win_o = move_window(win_o, p4_o, q2_o, q7_o, q3_o);
  vec_vsx_st(combine_window(win_e, win_o, q3, filter16_mask), 0, s + 3 * p);

  // OQ4
  // p2 + p1 + p0 + q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5
  win_e = move_window(win_e, p3_e, q3_e, q7_e, q4_e);
  win_o = move_window(win_o, p3_o, q3_o, q7_o, q4_o);
  vec_vsx_st(combine_window(win_e, win_o, q4, filter16_mask), 0, s + 4 * p);

  // OQ5
  // p1 + p0 + q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6
  win_e = move_window(win_e, p2_e, q4_e, q7_e, q5_e);
  win_o = move_window(win_o, p2_o, q4_o, q7_o, q5_o);
  vec_vsx_st(combine_window(win_e, win_o, q5, filter16_mask), 0, s + 5 * p);

  // OQ6
  // p0 + q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7
  win_e = move_window(win_e, p1_e, q5_e, q7_e, q6_e);
  win_o = move_window(win_o, p1_o, q5_o, q7_o, q6_o);
  vec_vsx_st(combine_window(win_e, win_o, q6, filter16_mask), 0, s + 6 * p);
}
