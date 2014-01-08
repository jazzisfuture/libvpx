/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CUDA_LOOPFILTERS_HU
#define CUDA_LOOPFILTERS_HU

__forceinline__ __device__
int clamp(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}

__forceinline__ __device__
int8_t signed_char_clamp(int t) {
  return (int8_t)clamp(t, -128, 127);
}

__forceinline__ __device__
uint8_t ROUND_POWER_OF_TWO(uint32_t value, uint32_t n)  {
  return (((value) + (1 << ((n) - 1))) >> (n));
}

// should we apply any filter at all: 11111111 yes, 00000000 no
__forceinline__ __device__
int8_t filter_mask(int32_t limit, int32_t blimit,
                                 int32_t p3, int32_t p2,
                                 int32_t p1, int32_t p0,
                                 int32_t q0, int32_t q1,
                                 int32_t q2, int32_t q3) {
  int8_t mask = 0;
  mask |= (abs(p3 - p2) > limit) * -1;
  mask |= (abs(p2 - p1) > limit) * -1;
  mask |= (abs(p1 - p0) > limit) * -1;
  mask |= (abs(q1 - q0) > limit) * -1;
  mask |= (abs(q2 - q1) > limit) * -1;
  mask |= (abs(q3 - q2) > limit) * -1;
  mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > blimit) * -1;
  return ~mask;
}

__forceinline__ __device__
int8_t flat_mask4(int32_t thresh,
                                int32_t p3, int32_t p2,
                                int32_t p1, int32_t p0,
                                int32_t q0, int32_t q1,
                                int32_t q2, int32_t q3) {
  int8_t mask = 0;
  mask |= (abs(p1 - p0) > thresh) * -1;
  mask |= (abs(q1 - q0) > thresh) * -1;
  mask |= (abs(p2 - p0) > thresh) * -1;
  mask |= (abs(q2 - q0) > thresh) * -1;
  mask |= (abs(p3 - p0) > thresh) * -1;
  mask |= (abs(q3 - q0) > thresh) * -1;
  return ~mask;
}

__forceinline__ __device__
int8_t flat_mask5(int32_t thresh,
                                int32_t p4, int32_t p3,
                                int32_t p2, int32_t p1,
                                int32_t p0, int32_t q0,
                                int32_t q1, int32_t q2,
                                int32_t q3, int32_t q4) {
  int8_t mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
  mask |= (abs(p4 - p0) > thresh) * -1;
  mask |= (abs(q4 - q0) > thresh) * -1;
  return ~mask;
}

// is there high edge variance internal edge: 11111111 yes, 00000000 no
__forceinline__ __device__
int8_t hev_mask(int32_t thresh, int32_t p1, int32_t p0,
                              int32_t q0, int32_t q1) {
  int8_t hev = 0;
  hev  |= (abs(p1 - p0) > thresh) * -1;
  hev  |= (abs(q1 - q0) > thresh) * -1;
  return hev;
}

__forceinline__ __device__
void filter4(int8_t mask, uint8_t hev, uint8_t *op1,
             uint8_t *op0, uint8_t *oq0, uint8_t *oq1) {
  int8_t filter1, filter2;

  const int8_t ps1 = (int8_t) *op1 ^ 0x80;
  const int8_t ps0 = (int8_t) *op0 ^ 0x80;
  const int8_t qs0 = (int8_t) *oq0 ^ 0x80;
  const int8_t qs1 = (int8_t) *oq1 ^ 0x80;

  // add outer taps if we have high edge variance
  int8_t filter = signed_char_clamp(ps1 - qs1) & hev;

  // inner taps
  filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;

  // save bottom 3 bits so that we round one side +4 and the other +3
  // if it equals 4 we'll set to adjust by -1 to account for the fact
  // we'd round 3 the other way
  filter1 = signed_char_clamp(filter + 4) >> 3;
  filter2 = signed_char_clamp(filter + 3) >> 3;

  *oq0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
  *op0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

  // outer tap adjustments
  filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

  *oq1 = signed_char_clamp(qs1 - filter) ^ 0x80;
  *op1 = signed_char_clamp(ps1 + filter) ^ 0x80;
}

__forceinline__ __device__
void vp9_loop_filter_horizontal_edge_cuda(uint8_t *s, int p /* pitch */,
                                       const uint8_t blimit,
                                       const uint8_t limit,
                                       const uint8_t thresh) {
  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uint8_t q0 = s[0 * p],  q1 = s[1 * p],  q2 = s[2 * p],  q3 = s[3 * p];
  const int8_t mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const int8_t hev = hev_mask(thresh, p1, p0, q0, q1);
  filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
}


__forceinline__ __device__
void filter8(int8_t mask, uint8_t hev, uint8_t flat,
                           uint8_t *op3, uint8_t *op2,
                           uint8_t *op1, uint8_t *op0,
                           uint8_t *oq0, uint8_t *oq1,
                           uint8_t *oq2, uint8_t *oq3) {
  if (flat && mask) {
    const uint32_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint32_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;

    // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
    *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
    *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
    *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
    *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
    *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
    *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
  } else {
    filter4(mask, hev, op1,  op0, oq0, oq1);
  }
}

__forceinline__ __device__
void vp9_mbloop_filter_horizontal_edge_cuda(uint8_t *s, int p,
                                         const uint8_t blimit,
                                         const uint8_t limit,
                                         const uint8_t thresh) {
  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
  const int8_t mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const int8_t hev = hev_mask(thresh, p1, p0, q0, q1);
  const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
  filter8(mask, hev, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
                           s,         s + 1 * p, s + 2 * p, s + 3 * p);
}


__forceinline__ __device__
void filter16(int8_t mask, uint8_t hev,
                            uint8_t flat, uint8_t flat2,
                            uint8_t *op7, uint8_t *op6,
                            uint8_t *op5, uint8_t *op4,
                            uint8_t *op3, uint8_t *op2,
                            uint8_t *op1, uint8_t *op0,
                            uint8_t *oq0, uint8_t *oq1,
                            uint8_t *oq2, uint8_t *oq3,
                            uint8_t *oq4, uint8_t *oq5,
                            uint8_t *oq6, uint8_t *oq7) {
  if (flat2 && flat && mask) {
    const uint32_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4,
                  p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;

    const uint32_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3,
                  q4 = *oq4, q5 = *oq5, q6 = *oq6, q7 = *oq7;

    // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
    *op6 = ROUND_POWER_OF_TWO(p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 +
                              q0, 4);
    *op5 = ROUND_POWER_OF_TWO(p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 +
                              q0 + q1, 4);
    *op4 = ROUND_POWER_OF_TWO(p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 +
                              q0 + q1 + q2, 4);
    *op3 = ROUND_POWER_OF_TWO(p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 +
                              q0 + q1 + q2 + q3, 4);
    *op2 = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 +
                              q0 + q1 + q2 + q3 + q4, 4);
    *op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                              q0 + q1 + q2 + q3 + q4 + q5, 4);
    *op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                              q0 + q1 + q2 + q3 + q4 + q5 + q6, 4);
    *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 +
                              q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 + q7, 4);
    *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 +
                              q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2, 4);
    *oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 +
                              q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3, 4);
    *oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 +
                              q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
    *oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 +
                              q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
    *oq5 = ROUND_POWER_OF_TWO(p1 + p0 +
                              q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
    *oq6 = ROUND_POWER_OF_TWO(p0 +
                              q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
  } else {
    filter8(mask, hev, flat, op3, op2, op1, op0, oq0, oq1, oq2, oq3);
  }
}

__forceinline__ __device__
void vp9_mb_lpf_horizontal_edge_w_cuda(uint8_t *s, int p,
                                    const uint8_t blimit,
                                    const uint8_t limit,
                                    const uint8_t thresh) {
  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
  const int8_t mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const int8_t hev = hev_mask(thresh, p1, p0, q0, q1);
  const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
  const int8_t flat2 = flat_mask5(1,
                           s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0,
                           q0, s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

  filter16(mask, hev, flat, flat2,
           s - 8 * p, s - 7 * p, s - 6 * p, s - 5 * p,
           s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
           s,         s + 1 * p, s + 2 * p, s + 3 * p,
           s + 4 * p, s + 5 * p, s + 6 * p, s + 7 * p);
}

__forceinline__ __device__
void write_four(const uint8_t& a,
                const uint8_t& b,
                const uint8_t& c,
                const uint8_t& d,
                uint32_t* out) {
  *out = (d << 24) | (c << 16) | (b << 8) | a;
}

__forceinline__ __device__
void filter_vertical_edge(uint8_t * s,
                          const uint8_t blimit,
                          const uint8_t limit,
                          const uint8_t thresh,
                          const uint8_t apply_16x16,
                          const uint8_t apply_8x8) {
  // first we generate all potential offsets.  Any values which
  // are not part of the filter will end up just pointing to *s
  uint32_t* c = (uint32_t*)s;
  uint32_t p = *(c - 1);
  uint32_t q = *c;
  const uint8_t p3 = p & 0xff;
  const uint8_t p2 = (p >> 8) & 0xff;
  uint8_t p1 = (p >> 16) & 0xff;
  uint8_t p0 = (p >> 24) & 0xff;
  uint8_t q0 = q & 0xff;
  uint8_t q1 = (q >> 8) & 0xff;
  const uint8_t q2 = (q >> 16) & 0xff;
  const uint8_t q3 = (q >> 24) & 0xff;

  //calculate masks
  // calculate mask

  const uint8_t abs_p1_p0 = abs(p1 - p0);
  const uint8_t abs_q1_q0 = abs(q1 - q0);
  int8_t mask = 1;
  mask &= abs(p3 - p2) <= limit;
  mask &= abs(p2 - p1) <= limit;
  mask &= abs_p1_p0 <= limit;
  mask &= abs_q1_q0 <= limit;
  mask &= abs(q2 - q1) <= limit;
  mask &= abs(q3 - q2) <= limit;
  mask &= abs(p0 - q0) * 2 + abs(p1 - q1) / 2  <= blimit;
  mask *= -1;

  // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
  if (mask & (apply_16x16 | apply_8x8)) {
    // calculate flat mask
    int8_t flat = 1;
    flat &= abs_p1_p0 <= 1;
    flat &= abs_q1_q0 <= 1;
    flat &= abs(p2 - p0) <= 1;
    flat &= abs(q2 - q0) <= 1;
    flat &= abs(p3 - p0) <= 1;
    flat &= abs(q3 - q0) <= 1;

    if (apply_16x16 & flat) {
      p = *(c - 2);
      q = *(c + 1);
      const uint8_t p7 = p & 0xff;
      const uint8_t p6 = (p >> 8) & 0xff;
      const uint8_t p5 = (p >> 16) & 0xff;
      const uint8_t p4 = (p >> 24) & 0xff;

      const uint8_t q4 = q & 0xff;
      const uint8_t q5 = (q >> 8) & 0xff;
      const uint8_t q6 = (q >> 16) & 0xff;
      const uint8_t q7 = (q >> 24) & 0xff;

      int8_t flat2 = 1;
      flat2 &= abs(p4 - p0) <= 1;
      flat2 &= abs(q4 - q0) <= 1;
      flat2 &= abs(p5 - p0) <= 1;
      flat2 &= abs(q5 - q0) <= 1;
      flat2 &= abs(p6 - p0) <= 1;
      flat2 &= abs(q6 - q0) <= 1;
      flat2 &= abs(p7 - p0) <= 1;
      flat2 &= abs(q7 - q0) <= 1;

      if (flat2) {
        const uint8_t op6 = ROUND_POWER_OF_TWO(p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 +
                                               q0, 4);
        const uint8_t op5 = ROUND_POWER_OF_TWO(p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1, 4);
        const uint8_t op4 = ROUND_POWER_OF_TWO(p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 +
                                q0 + q1 + q2, 4);
        const uint8_t op3 = ROUND_POWER_OF_TWO(p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3, 4);
        const uint8_t op2 = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4, 4);
        const uint8_t op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5, 4);
        const uint8_t op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                                  q0 + q1 + q2 + q3 + q4 + q5 + q6, 4);
        const uint8_t oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 + q7, 4);
        const uint8_t oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2, 4);
        const uint8_t oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3, 4);
        const uint8_t oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
        const uint8_t oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
        const uint8_t oq5 = ROUND_POWER_OF_TWO(p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
        const uint8_t oq6 = ROUND_POWER_OF_TWO(p0 +
                                q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
        write_four(p7, op6, op5, op4, c - 2);
        write_four(op3, op2, op1, op0, c - 1);
        write_four(oq0, oq1, oq2, oq3, c);
        write_four(oq4, oq5, oq6, q7, c + 1);
        return;
      }
    }
    if (flat & mask) {
      // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
      const uint8_t op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
      const uint8_t op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
      const uint8_t op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
      const uint8_t oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
      const uint8_t oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
      const uint8_t oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
      write_four(p3, op2, op1, op0, c - 1);
      write_four(oq0, oq1, oq2, q3, c);
      return;
    }
  }
  int8_t filter1, filter2;
  int8_t hev = 0;
  hev  |= abs_p1_p0 > thresh;
  hev  |= abs_q1_q0 > thresh;
  hev *= -1;

  const int8_t ps1 = (int8_t) p1 ^ 0x80;
  const int8_t ps0 = (int8_t) p0 ^ 0x80;
  const int8_t qs0 = (int8_t) q0 ^ 0x80;
  const int8_t qs1 = (int8_t) q1 ^ 0x80;

  // add outer taps if we have high edge variance
  int8_t filter = signed_char_clamp(ps1 - qs1) & hev;

  // inner taps
  filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;

  // save bottom 3 bits so that we round one side +4 and the other +3
  // if it equals 4 we'll set to adjust by -1 to account for the fact
  // we'd round 3 the other way
  filter1 = signed_char_clamp(filter + 4) >> 3;
  filter2 = signed_char_clamp(filter + 3) >> 3;

  q0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
  p0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

  // outer tap adjustments
  filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

  q1 = signed_char_clamp(qs1 - filter) ^ 0x80;
  p1 = signed_char_clamp(ps1 + filter) ^ 0x80;

  write_four(p3, p2, p1, p0, c - 1);
  write_four(q0, q1, q2, q3, c);
}

#endif
