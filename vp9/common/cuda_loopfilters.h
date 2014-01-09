/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/* helper functions */
inline
char signed_char_clamp(int t) {
  return (char)clamp(t, -128, 127);
}

inline
uchar ROUND_POWER_OF_TWO(uint value, uint n)  {
  return (((value) + (1 << ((n) - 1))) >> (n));
}

// should we apply any filter at all: 11111111 yes, 00000000 no
inline
char filter_mask(int limit, int blimit,
                                 int p3, int p2,
                                 int p1, int p0,
                                 int q0, int q1,
                                 int q2, int q3) {
  char mask = 0;
  mask |= (abs(p3 - p2) > limit) * -1;
  mask |= (abs(p2 - p1) > limit) * -1;
  mask |= (abs(p1 - p0) > limit) * -1;
  mask |= (abs(q1 - q0) > limit) * -1;
  mask |= (abs(q2 - q1) > limit) * -1;
  mask |= (abs(q3 - q2) > limit) * -1;
  mask |= (abs(p0 - q0) * 2 + abs(p1 - q1) / 2  > blimit) * -1;
  return ~mask;
}

inline
char flat_mask4(int thresh,
                                int p3, int p2,
                                int p1, int p0,
                                int q0, int q1,
                                int q2, int q3) {
  char mask = 0;
  mask |= (abs(p1 - p0) > thresh) * -1;
  mask |= (abs(q1 - q0) > thresh) * -1;
  mask |= (abs(p2 - p0) > thresh) * -1;
  mask |= (abs(q2 - q0) > thresh) * -1;
  mask |= (abs(p3 - p0) > thresh) * -1;
  mask |= (abs(q3 - q0) > thresh) * -1;
  return ~mask;
}

inline
char flat_mask5(int thresh,
                                int p4, int p3,
                                int p2, int p1,
                                int p0, int q0,
                                int q1, int q2,
                                int q3, int q4) {
  char mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
  mask |= (abs(p4 - p0) > thresh) * -1;
  mask |= (abs(q4 - q0) > thresh) * -1;
  return ~mask;
}

// is there high edge variance internal edge: 11111111 yes, 00000000 no
inline
char hev_mask(int thresh, int p1, int p0,
                              int q0, int q1) {
  char hev = 0;
  hev  |= (abs(p1 - p0) > thresh) * -1;
  hev  |= (abs(q1 - q0) > thresh) * -1;
  return hev;
}

inline
void filter4(char mask, uchar hev, __global uchar *op1,
             __global uchar *op0, __global uchar *oq0,
             __global uchar *oq1) {
  char filter1, filter2;

  const char ps1 = (char) *op1 ^ 0x80;
  const char ps0 = (char) *op0 ^ 0x80;
  const char qs0 = (char) *oq0 ^ 0x80;
  const char qs1 = (char) *oq1 ^ 0x80;

  // add outer taps if we have high edge variance
  char filter = signed_char_clamp(ps1 - qs1) & hev;

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

inline
void vp9_loop_filter_horizontal_edge_cuda(__global uchar *s, int p /* pitch */,
                                       const uchar blimit,
                                       const uchar limit,
                                       const uchar thresh) {
  const uchar p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uchar q0 = s[0 * p],  q1 = s[1 * p],  q2 = s[2 * p],  q3 = s[3 * p];
  const char mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const char hev = hev_mask(thresh, p1, p0, q0, q1);
  filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
}

inline
void filter8(char mask, uchar hev, uchar flat,
                           __global uchar *op3, __global uchar *op2,
                           __global uchar *op1, __global uchar *op0,
                           __global uchar *oq0, __global uchar *oq1,
                           __global uchar *oq2, __global uchar *oq3) {
  if (flat && mask) {
    const uint p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;

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

inline
void vp9_mbloop_filter_horizontal_edge_cuda(__global uchar *s, int p,
                                         const uchar blimit,
                                         const uchar limit,
                                         const uchar thresh) {
  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  const uchar p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uchar q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
  const char mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const char hev = hev_mask(thresh, p1, p0, q0, q1);
  const char flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
  filter8(mask, hev, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
                           s,         s + 1 * p, s + 2 * p, s + 3 * p);
}

inline
void filter16(char mask, uchar hev,
                            uchar flat, uchar flat2,
                            __global uchar *op7, __global uchar *op6,
                            __global uchar *op5, __global uchar *op4,
                            __global uchar *op3, __global uchar *op2,
                            __global uchar *op1, __global uchar *op0,
                            __global uchar *oq0, __global uchar *oq1,
                            __global uchar *oq2, __global uchar *oq3,
                            __global uchar *oq4, __global uchar *oq5,
                            __global uchar *oq6, __global uchar *oq7) {
  if (flat2 && flat && mask) {
    const uint p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4,
                  p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;

    const uint q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3,
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

inline
void vp9_mb_lpf_horizontal_edge_w_cuda(__global uchar *s, int p,
                                    const uchar blimit,
                                    const uchar limit,
                                    const uchar thresh) {
  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  const uchar p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
  const uchar q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
  const char mask = filter_mask(limit, blimit,
                                  p3, p2, p1, p0, q0, q1, q2, q3);
  const char hev = hev_mask(thresh, p1, p0, q0, q1);
  const char flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
  const char flat2 = flat_mask5(1,
                           s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0,
                           q0, s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

  filter16(mask, hev, flat, flat2,
           s - 8 * p, s - 7 * p, s - 6 * p, s - 5 * p,
           s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
           s,         s + 1 * p, s + 2 * p, s + 3 * p,
           s + 4 * p, s + 5 * p, s + 6 * p, s + 7 * p);
}

/* Due to poor memory access patterns on the vertical edges, we grab
 * an entire word at a time and decompose it into bytes.  We write back
 * a full word as well.  lots of manual inlining in this function as well,
 * but the logic is the same as the horizontal filter*/
inline
void write_four(const uchar* a,
                const uchar* b,
                const uchar* c,
                const uchar* d,
                __global uint* out) {
  *out = (*d << 24) | (*c << 16) | (*b << 8) | *a;
}
inline
void filter_vertical_edge(__global uchar * s,
                          const uchar blimit,
                          const uchar limit,
                          const uchar thresh,
                          const uchar apply_16x16,
                          const uchar apply_8x8) {
  // first we generate all potential offsets.  Any values which
  // are not part of the filter will end up just pointing to *s
  __global uint* c = (__global uint*)s;
  uint p = *(c - 1);
  uint q = *c;
  const uchar p3 = p & 0xff;
  const uchar p2 = (p >> 8) & 0xff;
  uchar p1 = (p >> 16) & 0xff;
  uchar p0 = (p >> 24) & 0xff;
  uchar q0 = q & 0xff;
  uchar q1 = (q >> 8) & 0xff;
  const uchar q2 = (q >> 16) & 0xff;
  const uchar q3 = (q >> 24) & 0xff;

  //calculate masks
  // calculate mask

  const uchar abs_p1_p0 = abs(p1 - p0);
  const uchar abs_q1_q0 = abs(q1 - q0);
  char mask = 1;
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
    char flat = 1;
    flat &= abs_p1_p0 <= 1;
    flat &= abs_q1_q0 <= 1;
    flat &= abs(p2 - p0) <= 1;
    flat &= abs(q2 - q0) <= 1;
    flat &= abs(p3 - p0) <= 1;
    flat &= abs(q3 - q0) <= 1;

    if (apply_16x16 & flat) {
      p = *(c - 2);
      q = *(c + 1);
      const uchar p7 = p & 0xff;
      const uchar p6 = (p >> 8) & 0xff;
      const uchar p5 = (p >> 16) & 0xff;
      const uchar p4 = (p >> 24) & 0xff;

      const uchar q4 = q & 0xff;
      const uchar q5 = (q >> 8) & 0xff;
      const uchar q6 = (q >> 16) & 0xff;
      const uchar q7 = (q >> 24) & 0xff;

      char flat2 = 1;
      flat2 &= abs(p4 - p0) <= 1;
      flat2 &= abs(q4 - q0) <= 1;
      flat2 &= abs(p5 - p0) <= 1;
      flat2 &= abs(q5 - q0) <= 1;
      flat2 &= abs(p6 - p0) <= 1;
      flat2 &= abs(q6 - q0) <= 1;
      flat2 &= abs(p7 - p0) <= 1;
      flat2 &= abs(q7 - q0) <= 1;

      if (flat2) {
        const uchar op6 = ROUND_POWER_OF_TWO(p7 * 7 + p6 * 2 + p5 + p4 + p3 + p2 + p1 + p0 +
                                               q0, 4);
        const uchar op5 = ROUND_POWER_OF_TWO(p7 * 6 + p6 + p5 * 2 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1, 4);
        const uchar op4 = ROUND_POWER_OF_TWO(p7 * 5 + p6 + p5 + p4 * 2 + p3 + p2 + p1 + p0 +
                                q0 + q1 + q2, 4);
        const uchar op3 = ROUND_POWER_OF_TWO(p7 * 4 + p6 + p5 + p4 + p3 * 2 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3, 4);
        const uchar op2 = ROUND_POWER_OF_TWO(p7 * 3 + p6 + p5 + p4 + p3 + p2 * 2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4, 4);
        const uchar op1 = ROUND_POWER_OF_TWO(p7 * 2 + p6 + p5 + p4 + p3 + p2 + p1 * 2 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5, 4);
        const uchar op0 = ROUND_POWER_OF_TWO(p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0 * 2 +
                                  q0 + q1 + q2 + q3 + q4 + q5 + q6, 4);
        const uchar oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 * 2 + q1 + q2 + q3 + q4 + q5 + q6 + q7, 4);
        const uchar oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 * 2 + q2 + q3 + q4 + q5 + q6 + q7 * 2, 4);
        const uchar oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 * 2 + q3 + q4 + q5 + q6 + q7 * 3, 4);
        const uchar oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 * 2 + q4 + q5 + q6 + q7 * 4, 4);
        const uchar oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 * 2 + q5 + q6 + q7 * 5, 4);
        const uchar oq5 = ROUND_POWER_OF_TWO(p1 + p0 +
                                  q0 + q1 + q2 + q3 + q4 + q5 * 2 + q6 + q7 * 6, 4);
        const uchar oq6 = ROUND_POWER_OF_TWO(p0 +
                                q0 + q1 + q2 + q3 + q4 + q5 + q6 * 2 + q7 * 7, 4);
        write_four(&p7, &op6, &op5, &op4, c - 2);
        write_four(&op3, &op2, &op1, &op0, c - 1);
        write_four(&oq0, &oq1, &oq2, &oq3, c);
        write_four(&oq4, &oq5, &oq6, &q7, c + 1);
        return;
      }
    }
    if (flat & mask) {
      // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
      const uchar op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
      const uchar op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
      const uchar op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
      const uchar oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
      const uchar oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
      const uchar oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
      write_four(&p3, &op2, &op1, &op0, c - 1);
      write_four(&oq0, &oq1, &oq2, &q3, c);
      return;
    }
  }
  char filter1, filter2;
  char hev = 0;
  hev  |= abs_p1_p0 > thresh;
  hev  |= abs_q1_q0 > thresh;
  hev *= -1;

  const char ps1 = (char) p1 ^ 0x80;
  const char ps0 = (char) p0 ^ 0x80;
  const char qs0 = (char) q0 ^ 0x80;
  const char qs1 = (char) q1 ^ 0x80;

  // add outer taps if we have high edge variance
  char filter = signed_char_clamp(ps1 - qs1) & hev;

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

  write_four(&p3, &p2, &p1, &p0, c - 1);
  write_four(&q0, &q1, &q2, &q3, c);
}
