/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_onyxc_int.h"

static INLINE int8_t signed_char_clamp(int t) {
  return (int8_t)clamp(t, -128, 127);
}

// should we apply any filter at all: 11111111 yes, 00000000 no
#if CONFIG_B10_EXT
static INLINE int8_t filter_mask(uint16_t limit, uint16_t blimit,
                                 uint16_t p3, uint16_t p2,
                                 uint16_t p1, uint16_t p0,
                                 uint16_t q0, uint16_t q1,
                                 uint16_t q2, uint16_t q3) {
#else
static INLINE int8_t filter_mask(uint8_t limit, uint8_t blimit,
                                 uint8_t p3, uint8_t p2,
                                 uint8_t p1, uint8_t p0,
                                 uint8_t q0, uint8_t q1,
                                 uint8_t q2, uint8_t q3) {
#endif
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

#if CONFIG_B10_EXT
static INLINE int8_t flat_mask4(uint16_t thresh,
#else
static INLINE int8_t flat_mask4(uint8_t thresh,
#endif
#if CONFIG_B10_EXT
                                uint16_t p3, uint16_t p2,
#else
                                uint8_t p3, uint8_t p2,
#endif
#if CONFIG_B10_EXT
                                uint16_t p1, uint16_t p0,
#else
                                uint8_t p1, uint8_t p0,
#endif
#if CONFIG_B10_EXT
                                uint16_t q0, uint16_t q1,
#else
                                uint8_t q0, uint8_t q1,
#endif
#if CONFIG_B10_EXT
                                uint16_t q2, uint16_t q3) {
#else
                                uint8_t q2, uint8_t q3) {
#endif
  int8_t mask = 0;
  mask |= (abs(p1 - p0) > thresh) * -1;
  mask |= (abs(q1 - q0) > thresh) * -1;
  mask |= (abs(p2 - p0) > thresh) * -1;
  mask |= (abs(q2 - q0) > thresh) * -1;
  mask |= (abs(p3 - p0) > thresh) * -1;
  mask |= (abs(q3 - q0) > thresh) * -1;
  return ~mask;
}

#if CONFIG_B10_EXT
static INLINE int8_t flat_mask5(uint16_t thresh,
#else
static INLINE int8_t flat_mask5(uint8_t thresh,
#endif
#if CONFIG_B10_EXT
                                uint16_t p4, uint16_t p3,
#else
                                uint8_t p4, uint8_t p3,
#endif
#if CONFIG_B10_EXT
                                uint16_t p2, uint16_t p1,
#else
                                uint8_t p2, uint8_t p1,
#endif
#if CONFIG_B10_EXT
                                uint16_t p0, uint16_t q0,
#else
                                uint8_t p0, uint8_t q0,
#endif
#if CONFIG_B10_EXT
                                uint16_t q1, uint16_t q2,
#else
                                uint8_t q1, uint8_t q2,
#endif
#if CONFIG_B10_EXT
                                uint16_t q3, uint16_t q4) {
#else
                                uint8_t q3, uint8_t q4) {
#endif
  int8_t mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
  mask |= (abs(p4 - p0) > thresh) * -1;
  mask |= (abs(q4 - q0) > thresh) * -1;
  return ~mask;
}

#if CONFIG_B10_EXT
static INLINE int16_t hev_mask(uint16_t thresh, uint16_t p1, uint16_t p0,
                              uint16_t q0, uint16_t q1) {
  int16_t hev = 0;
#else
// is there high edge variance internal edge: 11111111 yes, 00000000 no
static INLINE int8_t hev_mask(uint8_t thresh, uint8_t p1, uint8_t p0,
                              uint8_t q0, uint8_t q1) {
  int8_t hev = 0;
#endif
  hev  |= (abs(p1 - p0) > thresh) * -1;
  hev  |= (abs(q1 - q0) > thresh) * -1;
  return hev;
}

#if CONFIG_B10_EXT
static INLINE void filter4(int8_t mask, uint16_t thresh, uint16_t *op1,
                           uint16_t *op0, uint16_t *oq0, uint16_t *oq1) {
#else
static INLINE void filter4(int8_t mask, uint8_t thresh, uint8_t *op1,
                           uint8_t *op0, uint8_t *oq0, uint8_t *oq1) {
#endif
#if CONFIG_B10_EXT
  int16_t filter1, filter2;
  const int16_t ps1 = (int16_t)*op1 - 128;
  const int16_t ps0 = (int16_t)*op0 - 128;
  const int16_t qs0 = (int16_t)*oq0 - 128;
  const int16_t qs1 = (int16_t)*oq1 - 128;
#else
  int8_t filter1, filter2;
  const int8_t ps1 = (int8_t) *op1 ^ 0x80;
  const int8_t ps0 = (int8_t) *op0 ^ 0x80;
  const int8_t qs0 = (int8_t) *oq0 ^ 0x80;
  const int8_t qs1 = (int8_t) *oq1 ^ 0x80;
#endif
#if CONFIG_B10_EXT
  const uint16_t hev = hev_mask(thresh, *op1, *op0, *oq0, *oq1);
#else
  const uint8_t hev = hev_mask(thresh, *op1, *op0, *oq0, *oq1);
#endif

  // add outer taps if we have high edge variance
#if CONFIG_B10_EXT
  int16_t filter = (int16_t)clamp(ps1 - qs1,-128,127) & hev;
#else
  int8_t filter = signed_char_clamp(ps1 - qs1) & hev;
#endif

  // inner taps
#if CONFIG_B10_EXT
  filter= (int16_t)clamp(filter + 3 * (qs0-ps0), -128, 127) &((int16_t)mask); 
#else
  filter = signed_char_clamp(filter + 3 * (qs0 - ps0)) & mask;
#endif
  // save bottom 3 bits so that we round one side +4 and the other +3
  // if it equals 4 we'll set to adjust by -1 to account for the fact
  // we'd round 3 the other way
#if CONFIG_B10_EXT
  filter1 = (int16_t)clamp(filter + 4, -128, 127) >> 3;
  filter2 = (int16_t)clamp(filter + 3, -128, 127) >> 3;
  
  *oq0 = (int16_t)clamp(qs0 - filter1, -128, 127) + 128;
  *op0 = (int16_t)clamp(ps0 + filter2, -128, 127) + 128;

  // outer tap adjustments
  filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

  *oq1 = (int16_t)clamp(qs1 - filter, -128, 127) + 128;
  *op1 = (int16_t)clamp(ps1 + filter, -128, 127) + 128;
  //if(*op0>256){printf("error %d",*op0);}
  //if(*oq0>256){printf("error %d",*oq0);}
  //if(*op1>256){printf("error %d",*op1);}
  //if(*oq1>256){printf("error %d",*oq1);}
#else
  filter1 = signed_char_clamp(filter + 4) >> 3;
  filter2 = signed_char_clamp(filter + 3) >> 3;

  *oq0 = signed_char_clamp(qs0 - filter1) ^ 0x80;
  *op0 = signed_char_clamp(ps0 + filter2) ^ 0x80;

  // outer tap adjustments
  filter = ROUND_POWER_OF_TWO(filter1, 1) & ~hev;

  *oq1 = signed_char_clamp(qs1 - filter) ^ 0x80;
  *op1 = signed_char_clamp(ps1 + filter) ^ 0x80;
#endif
}

#if CONFIG_B10_EXT
void vp9_lpf_horizontal_4_c(uint16_t *s, int p /* pitch */,
                            const uint16_t *blimit, const uint16_t *limit,
                            const uint16_t *thresh, int count) {
#else
void vp9_lpf_horizontal_4_c(uint8_t *s, int p /* pitch */,
                            const uint8_t *blimit, const uint8_t *limit,
                            const uint8_t *thresh, int count) {
#endif
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint16_t q0 = s[0 * p],  q1 = s[1 * p],  q2 = s[2 * p],  q3 = s[3 * p];
#else
    const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint8_t q0 = s[0 * p],  q1 = s[1 * p],  q2 = s[2 * p],  q3 = s[3 * p];
#endif
    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    filter4(mask, *thresh, s - 2 * p, s - 1 * p, s, s + 1 * p);
    ++s;
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_horizontal_4_dual_c(uint16_t *s, int p, const uint16_t *blimit0,
                                 const uint16_t *limit0, const uint16_t *thresh0,
                                 const uint16_t *blimit1, const uint16_t *limit1,
                                 const uint16_t *thresh1) {
#else
void vp9_lpf_horizontal_4_dual_c(uint8_t *s, int p, const uint8_t *blimit0,
                                 const uint8_t *limit0, const uint8_t *thresh0,
                                 const uint8_t *blimit1, const uint8_t *limit1,
                                 const uint8_t *thresh1) {
#endif
  vp9_lpf_horizontal_4_c(s, p, blimit0, limit0, thresh0, 1);
  vp9_lpf_horizontal_4_c(s + 8, p, blimit1, limit1, thresh1, 1);
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_4_c(uint16_t *s, int pitch, const uint16_t *blimit,
                          const uint16_t *limit, const uint16_t *thresh,
                          int count) {
#else
void vp9_lpf_vertical_4_c(uint8_t *s, int pitch, const uint8_t *blimit,
                          const uint8_t *limit, const uint8_t *thresh,
                          int count) {
#endif
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint16_t q0 = s[0],  q1 = s[1],  q2 = s[2],  q3 = s[3];
#else
    const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint8_t q0 = s[0],  q1 = s[1],  q2 = s[2],  q3 = s[3];
#endif
    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    filter4(mask, *thresh, s - 2, s - 1, s, s + 1);
    s += pitch;
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_4_dual_c(uint16_t *s, int pitch, const uint16_t *blimit0,
                               const uint16_t *limit0, const uint16_t *thresh0,
                               const uint16_t *blimit1, const uint16_t *limit1,
                               const uint16_t *thresh1) {
#else
void vp9_lpf_vertical_4_dual_c(uint8_t *s, int pitch, const uint8_t *blimit0,
                               const uint8_t *limit0, const uint8_t *thresh0,
                               const uint8_t *blimit1, const uint8_t *limit1,
                               const uint8_t *thresh1) {
#endif
  vp9_lpf_vertical_4_c(s, pitch, blimit0, limit0, thresh0, 1);
  vp9_lpf_vertical_4_c(s + 8 * pitch, pitch, blimit1, limit1,
                                  thresh1, 1);
}

#if CONFIG_B10_EXT
static INLINE void filter8(int8_t mask, uint16_t thresh, uint16_t flat,
                           uint16_t *op3, uint16_t *op2,
                           uint16_t *op1, uint16_t *op0,
                           uint16_t *oq0, uint16_t *oq1,
                           uint16_t *oq2, uint16_t *oq3) {
#else
static INLINE void filter8(int8_t mask, uint8_t thresh, uint8_t flat,
                           uint8_t *op3, uint8_t *op2,
                           uint8_t *op1, uint8_t *op0,
                           uint8_t *oq0, uint8_t *oq1,
                           uint8_t *oq2, uint8_t *oq3) {
#endif
  if (flat && mask) {
#if CONFIG_B10_EXT
    const uint16_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint16_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;
#else
    const uint8_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;
#endif

    // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
    *op2 = ROUND_POWER_OF_TWO(p3 + p3 + p3 + 2 * p2 + p1 + p0 + q0, 3);
    *op1 = ROUND_POWER_OF_TWO(p3 + p3 + p2 + 2 * p1 + p0 + q0 + q1, 3);
    *op0 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + 2 * p0 + q0 + q1 + q2, 3);
    *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + 2 * q0 + q1 + q2 + q3, 3);
    *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + q0 + 2 * q1 + q2 + q3 + q3, 3);
    *oq2 = ROUND_POWER_OF_TWO(p0 + q0 + q1 + 2 * q2 + q3 + q3 + q3, 3);
  } else {
    filter4(mask, thresh, op1,  op0, oq0, oq1);
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_horizontal_8_c(uint16_t *s, int p, const uint16_t *blimit,
                            const uint16_t *limit, const uint16_t *thresh,
                            int count) {
#else
void vp9_lpf_horizontal_8_c(uint8_t *s, int p, const uint8_t *blimit,
                            const uint8_t *limit, const uint8_t *thresh,
                            int count) {
#endif
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint16_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
#else
    const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
#endif

    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    filter8(mask, *thresh, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
                                 s,         s + 1 * p, s + 2 * p, s + 3 * p);
    ++s;
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_horizontal_8_dual_c(uint16_t *s, int p, const uint16_t *blimit0,
                                 const uint16_t *limit0, const uint16_t *thresh0,
                                 const uint16_t *blimit1, const uint16_t *limit1,
                                 const uint16_t *thresh1) {
#else
void vp9_lpf_horizontal_8_dual_c(uint8_t *s, int p, const uint8_t *blimit0,
                                 const uint8_t *limit0, const uint8_t *thresh0,
                                 const uint8_t *blimit1, const uint8_t *limit1,
                                 const uint8_t *thresh1) {
#endif
  vp9_lpf_horizontal_8_c(s, p, blimit0, limit0, thresh0, 1);
  vp9_lpf_horizontal_8_c(s + 8, p, blimit1, limit1, thresh1, 1);
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_8_c(uint16_t *s, int pitch, const uint16_t *blimit,
                          const uint16_t *limit, const uint16_t *thresh,
                          int count) {
#else
void vp9_lpf_vertical_8_c(uint8_t *s, int pitch, const uint8_t *blimit,
                          const uint8_t *limit, const uint8_t *thresh,
                          int count) {
#endif
  int i;

  for (i = 0; i < 8 * count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint16_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
#else
    const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
#endif
    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    filter8(mask, *thresh, flat, s - 4, s - 3, s - 2, s - 1,
                                 s,     s + 1, s + 2, s + 3);
    s += pitch;
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_8_dual_c(uint16_t *s, int pitch, const uint16_t *blimit0,
                               const uint16_t *limit0, const uint16_t *thresh0,
                               const uint16_t *blimit1, const uint16_t *limit1,
                               const uint16_t *thresh1) {
#else
void vp9_lpf_vertical_8_dual_c(uint8_t *s, int pitch, const uint8_t *blimit0,
                               const uint8_t *limit0, const uint8_t *thresh0,
                               const uint8_t *blimit1, const uint8_t *limit1,
                               const uint8_t *thresh1) {
#endif
  vp9_lpf_vertical_8_c(s, pitch, blimit0, limit0, thresh0, 1);
  vp9_lpf_vertical_8_c(s + 8 * pitch, pitch, blimit1, limit1,
                                    thresh1, 1);
}

#if CONFIG_B10_EXT
static INLINE void filter16(int8_t mask, uint16_t thresh,
                            uint16_t flat, uint16_t flat2,
                            uint16_t *op7, uint16_t *op6,
                            uint16_t *op5, uint16_t *op4,
                            uint16_t *op3, uint16_t *op2,
                            uint16_t *op1, uint16_t *op0,
                            uint16_t *oq0, uint16_t *oq1,
                            uint16_t *oq2, uint16_t *oq3,
                            uint16_t *oq4, uint16_t *oq5,
                            uint16_t *oq6, uint16_t *oq7) {
#else
static INLINE void filter16(int8_t mask, uint8_t thresh,
                            uint8_t flat, uint8_t flat2,
                            uint8_t *op7, uint8_t *op6,
                            uint8_t *op5, uint8_t *op4,
                            uint8_t *op3, uint8_t *op2,
                            uint8_t *op1, uint8_t *op0,
                            uint8_t *oq0, uint8_t *oq1,
                            uint8_t *oq2, uint8_t *oq3,
                            uint8_t *oq4, uint8_t *oq5,
                            uint8_t *oq6, uint8_t *oq7) {
#endif
  if (flat2 && flat && mask) {
#if CONFIG_B10_EXT
    const uint16_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4,
                  p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint16_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3,
                  q4 = *oq4, q5 = *oq5, q6 = *oq6, q7 = *oq7;
#else
    const uint8_t p7 = *op7, p6 = *op6, p5 = *op5, p4 = *op4,
                  p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
    const uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3,
                  q4 = *oq4, q5 = *oq5, q6 = *oq6, q7 = *oq7;
#endif

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
    filter8(mask, thresh, flat, op3, op2, op1, op0, oq0, oq1, oq2, oq3);
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_horizontal_16_c(uint16_t *s, int p, const uint16_t *blimit,
                             const uint16_t *limit, const uint16_t *thresh,
                             int count) {
#else
void vp9_lpf_horizontal_16_c(uint8_t *s, int p, const uint8_t *blimit,
                             const uint8_t *limit, const uint8_t *thresh,
                             int count) {
#endif
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint16_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
#else
    const uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    const uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
#endif
    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat2 = flat_mask5(1,
                             s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p], p0,
                             q0, s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

    filter16(mask, *thresh, flat, flat2,
             s - 8 * p, s - 7 * p, s - 6 * p, s - 5 * p,
             s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
             s,         s + 1 * p, s + 2 * p, s + 3 * p,
             s + 4 * p, s + 5 * p, s + 6 * p, s + 7 * p);
    ++s;
  }
}

#if CONFIG_B10_EXT
static void mb_lpf_vertical_edge_w(uint16_t *s, int p,
                                   const uint16_t *blimit,
                                   const uint16_t *limit,
                                   const uint16_t *thresh,
                                   int count) {
#else
static void mb_lpf_vertical_edge_w(uint8_t *s, int p,
                                   const uint8_t *blimit,
                                   const uint8_t *limit,
                                   const uint8_t *thresh,
                                   int count) {
#endif
  int i;

  for (i = 0; i < count; ++i) {
#if CONFIG_B10_EXT
    const uint16_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint16_t q0 = s[0], q1 = s[1],  q2 = s[2], q3 = s[3];
#else
    const uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    const uint8_t q0 = s[0], q1 = s[1],  q2 = s[2], q3 = s[3];
#endif
    const int8_t mask = filter_mask(*limit, *blimit,
                                    p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    const int8_t flat2 = flat_mask5(1, s[-8], s[-7], s[-6], s[-5], p0,
                                    q0, s[4], s[5], s[6], s[7]);

    filter16(mask, *thresh, flat, flat2,
             s - 8, s - 7, s - 6, s - 5, s - 4, s - 3, s - 2, s - 1,
             s,     s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7);
    s += p;
  }
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_16_c(uint16_t *s, int p, const uint16_t *blimit,
                           const uint16_t *limit, const uint16_t *thresh) {
#else
void vp9_lpf_vertical_16_c(uint8_t *s, int p, const uint8_t *blimit,
                           const uint8_t *limit, const uint8_t *thresh) {
#endif
  mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 8);
}

#if CONFIG_B10_EXT
void vp9_lpf_vertical_16_dual_c(uint16_t *s, int p, const uint16_t *blimit,
                                const uint16_t *limit, const uint16_t *thresh) {
#else
void vp9_lpf_vertical_16_dual_c(uint8_t *s, int p, const uint8_t *blimit,
                                const uint8_t *limit, const uint8_t *thresh) {
#endif
  mb_lpf_vertical_edge_w(s, p, blimit, limit, thresh, 16);
}
