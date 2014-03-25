#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "vp9_loopfilter.h"
#include "hexagon_protos.h"         // part of Q6 tools, contains intrinsics definitions
#include "dspCV_worker.h"
#include "filters_asm.h"
#include "dsp_system_utils.h"

#define LOOPFILTER_HW
#define LOOPFILTER_HW_MT
#define LOOPFILTER_L1_PREFETCH
#define LOOPFILTER_L2_PREFETCH

#define LOOPFILTER_OPT_INTRINSICS

#ifdef LOOPFILTER_HW
#include "loopfilter.h"
#endif //LOOPFILTER_HW
#ifdef LOOPFILTER_L2_PREFETCH
#include "dspcache.h"               // contains assembly for cache pre-fetching.
#define L2_PREFETCH_WIDTH (128+8)
#define L2_PREFETCH_HEIGHT (64+8)
#endif // LOOPFILTER_L2_PREFETCH
#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))

#ifndef LOOPFILTER_OPT_INTRINSICS
static INLINE int clamp(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}
#endif

static INLINE int8_t signed_char_clamp(int t) {
#ifndef LOOPFILTER_OPT_INTRINSICS
  return (int8_t) clamp(t, -128, 127);
#else // LOOPFILTER_OPT_INTRINSICS
  int tmp;
  tmp = Q6_R_max_RR(-128, t);
  tmp = Q6_R_min_RR(127, t);
  return tmp;
#endif // LOOPFILTER_OPT_INTRINSICS
}

// should we apply any filter at all: 11111111 yes, 00000000 no
static INLINE int8_t filter_mask(uint8_t limit, uint8_t blimit, uint8_t p3,
    uint8_t p2, uint8_t p1, uint8_t p0, uint8_t q0, uint8_t q1, uint8_t q2,
    uint8_t q3) {
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

// is there high edge variance internal edge: 11111111 yes, 00000000 no
static INLINE int8_t hev_mask(uint8_t thresh, uint8_t p1, uint8_t p0,
    uint8_t q0, uint8_t q1) {
  int8_t hev = 0;
  hev |= (abs(p1 - p0) > thresh) * -1;
  hev |= (abs(q1 - q0) > thresh) * -1;
  return hev;
}

static INLINE int8_t flat_mask4(uint8_t thresh, uint8_t p3, uint8_t p2,
    uint8_t p1, uint8_t p0, uint8_t q0, uint8_t q1, uint8_t q2, uint8_t q3) {
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
    uint8_t p2, uint8_t p1, uint8_t p0, uint8_t q0, uint8_t q1, uint8_t q2,
    uint8_t q3, uint8_t q4) {
  int8_t mask = ~flat_mask4(thresh, p3, p2, p1, p0, q0, q1, q2, q3);
  mask |= (abs(p4 - p0) > thresh) * -1;
  mask |= (abs(q4 - q0) > thresh) * -1;
  return ~mask;
}

static INLINE void filter4(int8_t mask, uint8_t hev, uint8_t *op1, uint8_t *op0,
    uint8_t *oq0, uint8_t *oq1) {
  int8_t filter1, filter2;

  int8_t ps1 = (int8_t) *op1 ^ 0x80;
  int8_t ps0 = (int8_t) *op0 ^ 0x80;
  int8_t qs0 = (int8_t) *oq0 ^ 0x80;
  int8_t qs1 = (int8_t) *oq1 ^ 0x80;

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

static INLINE void filter8_t(uint8_t *op3, uint8_t *op2, uint8_t *op1,
    uint8_t *op0, uint8_t *oq0, uint8_t *oq1, uint8_t *oq2, uint8_t *oq3) {
  uint8_t p3 = *op3, p2 = *op2, p1 = *op1, p0 = *op0;
  uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3;

  uint32_t tmp1, tmp2;

  tmp1 = p3 + p2 + p1 + p0;
  tmp2 = q0 + q1 + q2 + q3;

  // 7-tap filter [1, 1, 1, 2, 1, 1, 1]
  *op2 = ROUND_POWER_OF_TWO(2*p3 + tmp1 + q0 + p2, 3);
  *op1 = ROUND_POWER_OF_TWO(p3 + tmp1 + q0 + q1 + p1, 3);
  *op0 = ROUND_POWER_OF_TWO(tmp1 + q0 + q1 + q2 + p0, 3);
  *oq0 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + tmp2 + q0, 3);
  *oq1 = ROUND_POWER_OF_TWO(p1 + p0 + tmp2 + q3 + q1, 3);
  *oq2 = ROUND_POWER_OF_TWO(p0 + tmp2 + 2*q3 + q2, 3);
}

static INLINE void filter16_t(uint8_t op7, uint8_t *op6, uint8_t *op5, uint8_t *op4,
    uint8_t *op3, uint8_t *op2, uint8_t *op1, uint8_t *op0, uint8_t *oq0,
    uint8_t *oq1, uint8_t *oq2, uint8_t *oq3, uint8_t *oq4, uint8_t *oq5,
    uint8_t *oq6, uint8_t oq7) {
  uint8_t p7 = op7, p6 = *op6, p5 = *op5, p4 = *op4, p3 = *op3, p2 = *op2,
      p1 = *op1, p0 = *op0;

  uint8_t q0 = *oq0, q1 = *oq1, q2 = *oq2, q3 = *oq3, q4 = *oq4, q5 = *oq5,
      q6 = *oq6, q7 = oq7;

  uint16_t tmp1, tmp2;
  /*
   unsigned long long top1=0, bot1=1;
   top1 = Q6_P_vraddub_PP(top1, bot1);
   */
  tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0;
  tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7;

  // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
  *op6 = ROUND_POWER_OF_TWO(6*p7 + tmp1 + q0 + p6, 4);
  *op5 = ROUND_POWER_OF_TWO(5*p7 + tmp1 + q0 + q1 + p5, 4);
  *op4 = ROUND_POWER_OF_TWO(4*p7 + tmp1 + q0 + q1 + q2 + p4, 4);
  *op3 = ROUND_POWER_OF_TWO(3*p7 + tmp1 + q0 + q1 + q2 + q3 + p3, 4);
  *op2 = ROUND_POWER_OF_TWO(2*p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2, 4);
  *op1 = ROUND_POWER_OF_TWO(p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1, 4);
  *op0 = ROUND_POWER_OF_TWO(tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0, 4);
  *oq0 = ROUND_POWER_OF_TWO(p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0, 4);
  *oq1 = ROUND_POWER_OF_TWO(p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1, 4);
  *oq2 = ROUND_POWER_OF_TWO(p4 + p3 + p2 + p1 + p0 + tmp2 + 2*q7 + q2, 4);
  *oq3 = ROUND_POWER_OF_TWO(p3 + p2 + p1 + p0 + tmp2 + 3*q7 + q3, 4);
  *oq4 = ROUND_POWER_OF_TWO(p2 + p1 + p0 + tmp2 + 4*q7 + q4, 4);
  *oq5 = ROUND_POWER_OF_TWO(p1 + p0 + tmp2 + 5*q7 + q5, 4);
  *oq6 = ROUND_POWER_OF_TWO(p0 + tmp2 + 6*q7 + q6, 4);
}

void filter16_v(uint8_t *data /*op7, uint8_t *op6, uint8_t *op5, uint8_t *op4,
 uint8_t *op3, uint8_t *op2, uint8_t *op1, uint8_t *op0, uint8_t *oq0,
 uint8_t *oq1, uint8_t *oq2, uint8_t *oq3, uint8_t *oq4, uint8_t *oq5,
 uint8_t *oq6, uint8_t oq7*/) {
  uint8_t p7 = data[0], p6 = data[1], p5 = data[2], p4 = data[3],
      p3 = data[4], p2 = data[5], p1 = data[6], p0 = data[7];

  uint8_t q0 = data[8], q1 = data[9], q2 = data[10], q3 = data[11], q4 =
      data[12], q5 = data[13], q6 = data[14], q7 = data[15];

  uint16_t tmp1, tmp2;
  uint16_t d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14;
  /*
   unsigned long long top1=0, bot1=1;
   top1 = Q6_P_vraddub_PP(top1, bot1);
   */
  tmp1 = p7 + p6 + p5 + p4 + p3 + p2 + p1 + p0;
  tmp2 = q0 + q1 + q2 + q3 + q4 + q5 + q6 + q7;

  d1 = 6 * p7 + tmp1 + q0 + p6;
  d2 = 5 * p7 + tmp1 + q0 + q1 + p5;
  d3 = 4 * p7 + tmp1 + q0 + q1 + q2 + p4;
  d4 = 3 * p7 + tmp1 + q0 + q1 + q2 + q3 + p3;
  d5 = 2 * p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + p2;
  d6 = p7 + tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + p1;
  d7 = tmp1 + q0 + q1 + q2 + q3 + q4 + q5 + q6 + p0;
  d8 = p6 + p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q0;
  d9 = p5 + p4 + p3 + p2 + p1 + p0 + tmp2 + q7 + q1;
  d10 = p4 + p3 + p2 + p1 + p0 + tmp2 + 2 * q7 + q2;
  d11 = p3 + p2 + p1 + p0 + tmp2 + 3 * q7 + q3;
  d12 = p2 + p1 + p0 + tmp2 + 4 * q7 + q4;
  d13 = p1 + p0 + tmp2 + 5 * q7 + q5;
  d14 = p0 + tmp2 + 6 * q7 + q6;

  // 15-tap filter [1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1]
  data[1] = (d1 + 8) >> 4;
  data[2] = (d2 + 8) >> 4;
  data[3] = (d3 + 8) >> 4;
  data[4] = (d4 + 8) >> 4;
  data[5] = (d5 + 8) >> 4;
  data[6] = (d6 + 8) >> 4;
  data[7] = (d7 + 8) >> 4;
  data[8] = (d8 + 8) >> 4;
  data[9] = (d9 + 8) >> 4;
  data[10] = (d10 + 8) >> 4;
  data[11] = (d11 + 8) >> 4;
  data[12] = (d12 + 8) >> 4;
  data[13] = (d13 + 8) >> 4;
  data[14] = (d14 + 8) >> 4;
}


static void vp9_mb_lpf_vertical_edge_w_16(uint8_t *s, int p, uint8_t *blimit,
    uint8_t *limit, uint8_t *thresh) {
  int i;

  for (i = 0; i < 16; ++i) {
    uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
    int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    int8_t flat2 = flat_mask5(1, s[-8], s[-7], s[-6], s[-5], p0, q0, s[4],
        s[5], s[6], s[7]);

#ifdef LOOPFILTER_L1_PREFETCH
    Q6_dcfetch_A(s + p); // L1 pre fetch
#endif

#if 0
    filter16(mask, hev, flat, flat2, *(s - 8), s - 7, s - 6, s - 5, s - 4,
        s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3, s + 4, s + 5,
        s + 6, *(s + 7));
#else
    if (flat2 && flat && mask) {
      filter16_v_dsp(&s[-8]);
    } else if (flat && mask) {
      filter8_t(s - 4, s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3);
    } else {
      filter4(mask, hev, s - 2, s - 1, s, s + 1);
    }
#endif
    s += p;
  }
}

static void vp9_mb_lpf_vertical_edge_w(uint8_t *s, int p, uint8_t *blimit,
    uint8_t *limit, uint8_t *thresh) {
  int i;

  for (i = 0; i < 8; ++i) {
    uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
    int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    int8_t flat2 = flat_mask5(1, s[-8], s[-7], s[-6], s[-5], p0, q0, s[4],
        s[5], s[6], s[7]);

    // Compiler may have added pre-fetch.
#ifdef LOOPFILTER_L1_PREFETCH
    //Q6_dcfetch_A(s + p); // L1 pre fetch
#endif

#if 0
    filter16(mask, hev, flat, flat2, *(s - 8), s - 7, s - 6, s - 5, s - 4,
        s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3, s + 4, s + 5,
        s + 6, *(s + 7));
#else
    if (flat2 && flat && mask) {
      filter16_v_dsp(&s[-8]);
    } else if (flat && mask) {
      filter8_t(s - 4, s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3);
    } else {
      filter4(mask, hev, s - 2, s - 1, s, s + 1);
    }
#endif

    s += p;
  }
}

static void vp9_mbloop_filter_vertical_edge_16(uint8_t *s, int pitch,
    uint8_t *blimit0, uint8_t *limit0, uint8_t *thresh0, uint8_t *blimit1,
    uint8_t *limit1, uint8_t *thresh1) {
  int i, j;
  uint8_t *blimit = blimit0;
  uint8_t *limit = limit0;
  uint8_t *thresh = thresh0;

  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 8; ++j) {
      uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
      uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
      int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1,
          q2, q3);
      int8_t hev = hev_mask(thresh[0], p1, p0, q0, q1);
      int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);

      // Compiler may have added pre-fetch.
#ifdef LOOPFILTER_L1_PREFETCH
      //Q6_dcfetch_A(s + pitch); // L1 pre fetch
#endif

#if 0
      filter8(mask, hev, flat, s - 4, s - 3, s - 2, s - 1, s, s + 1,
          s + 2, s + 3);
#else
      if (flat && mask) {
        filter8_t(s - 4, s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3);
      } else {
        filter4(mask, hev, s - 2, s - 1, s, s + 1);
      }
#endif
      s += pitch;
    }
    blimit = blimit1;
    limit = limit1;
    thresh = thresh1;
  }
}

static void vp9_loop_filter_vertical_edge(uint8_t *s, int pitch,
    uint8_t *blimit, uint8_t *limit, uint8_t *thresh, int count) {
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
    uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);

#ifdef LOOPFILTER_L1_PREFETCH
    Q6_dcfetch_A(s + pitch); // L1 pre fetch
#endif

    filter4(mask, hev, s - 2, s - 1, s, s + 1);
    s += pitch;
  }
}

static void vp9_loop_filter_vertical_edge_16(uint8_t *s, int pitch,
    uint8_t *blimit0, uint8_t *limit0, uint8_t *thresh0, uint8_t *blimit1,
    uint8_t *limit1, uint8_t *thresh1) {
  int i, j;
  uint8_t *blimit = blimit0;
  uint8_t *limit = limit0;
  uint8_t *thresh = thresh0;

  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 8; ++j) {
      uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
      uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
      int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1,
          q2, q3);
      int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);

#ifdef LOOPFILTER_L1_PREFETCH
      Q6_dcfetch_A(s + pitch); // L1 pre fetch
#endif

      filter4(mask, hev, s - 2, s - 1, s, s + 1);
      s += pitch;
    }
    blimit = blimit1;
    limit = limit1;
    thresh = thresh1;
  }
}

static void vp9_mbloop_filter_vertical_edge(uint8_t *s, int pitch,
    uint8_t *blimit, uint8_t *limit, uint8_t *thresh, int count) {
  int i;

  for (i = 0; i < 8 * count; ++i) {
    uint8_t p3 = s[-4], p2 = s[-3], p1 = s[-2], p0 = s[-1];
    uint8_t q0 = s[0], q1 = s[1], q2 = s[2], q3 = s[3];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(thresh[0], p1, p0, q0, q1);
    int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);

    // Compiler may have added pre-fetch.
#ifdef LOOPFILTER_L1_PREFETCH
    //Q6_dcfetch_A(s + pitch); // L1 pre fetch
#endif

#if 0
    filter8(mask, hev, flat, s - 4, s - 3, s - 2, s - 1, s, s + 1,
        s + 2, s + 3);
#else
    if (flat && mask) {
      filter8_t(s - 4, s - 3, s - 2, s - 1, s, s + 1, s + 2, s + 3);
    } else {
      filter4(mask, hev, s - 2, s - 1, s, s + 1);
    }
#endif
    s += pitch;
  }
}

static void filter_selectively_vert_row2(PLANE_TYPE plane_type, uint8_t *s,
    int pitch, unsigned int mask_16x16_l, unsigned int mask_8x8_l,
    unsigned int mask_4x4_l, unsigned int mask_4x4_int_l,
    loop_filter_info_n *lfi_n, uint8_t *lfl) {
  int mask_shift = plane_type ? 4 : 8;
  int mask_cutoff = plane_type ? 0xf : 0xff;
  int lfl_forward = plane_type ? 4 : 8;

  unsigned int mask_16x16_0 = mask_16x16_l & mask_cutoff;
  unsigned int mask_8x8_0 = mask_8x8_l & mask_cutoff;
  unsigned int mask_4x4_0 = mask_4x4_l & mask_cutoff;
  unsigned int mask_4x4_int_0 = mask_4x4_int_l & mask_cutoff;
  unsigned int mask_16x16_1 = (mask_16x16_l >> mask_shift) & mask_cutoff;
  unsigned int mask_8x8_1 = (mask_8x8_l >> mask_shift) & mask_cutoff;
  unsigned int mask_4x4_1 = (mask_4x4_l >> mask_shift) & mask_cutoff;
  unsigned int mask_4x4_int_1 = (mask_4x4_int_l >> mask_shift) & mask_cutoff;
  unsigned int mask;

  for (mask = mask_16x16_0 | mask_8x8_0 | mask_4x4_0 | mask_4x4_int_0
      | mask_16x16_1 | mask_8x8_1 | mask_4x4_1 | mask_4x4_int_1; mask;
      mask >>= 1) {
    loop_filter_thresh *lfi0 = lfi_n->lfthr + *lfl;
    loop_filter_thresh *lfi1 = lfi_n->lfthr + *(lfl + lfl_forward);

    // TODO(yunqingwang): count in loopfilter functions should be removed.
    if (mask & 1) {
      if ((mask_16x16_0 | mask_16x16_1) & 1) {
        // TODO(yunqingwang): if (mask_16x16_0 & 1), then (mask_16x16_0 & 1)
        // is always 1. Same is true for horizontal lf.
        if ((mask_16x16_0 & mask_16x16_1) & 1) {
          vp9_mb_lpf_vertical_edge_w_16(s, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr);
        } else if (mask_16x16_0 & 1) {
          vp9_mb_lpf_vertical_edge_w(s, pitch, lfi0->mblim, lfi0->lim,
              lfi0->hev_thr);
        } else {
          vp9_mb_lpf_vertical_edge_w(s + 8 * pitch, pitch,
              lfi1->mblim, lfi1->lim, lfi1->hev_thr);
        }
      }

      if ((mask_8x8_0 | mask_8x8_1) & 1) {
        if ((mask_8x8_0 & mask_8x8_1) & 1) {
          vp9_mbloop_filter_vertical_edge_16(s, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, lfi1->mblim, lfi1->lim,
              lfi1->hev_thr);
        } else if (mask_8x8_0 & 1) {
          vp9_mbloop_filter_vertical_edge(s, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, 1);
        } else {
          vp9_mbloop_filter_vertical_edge(s + 8 * pitch, pitch,
              lfi1->mblim, lfi1->lim, lfi1->hev_thr, 1);
        }
      }

      if ((mask_4x4_0 | mask_4x4_1) & 1) {
        if ((mask_4x4_0 & mask_4x4_1) & 1) {
          vp9_loop_filter_vertical_edge_16(s, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, lfi1->mblim, lfi1->lim,
              lfi1->hev_thr);
        } else if (mask_4x4_0 & 1) {
          vp9_loop_filter_vertical_edge(s, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, 1);
        } else {
          vp9_loop_filter_vertical_edge(s + 8 * pitch, pitch,
              lfi1->mblim, lfi1->lim, lfi1->hev_thr, 1);
        }
      }

      if ((mask_4x4_int_0 | mask_4x4_int_1) & 1) {
        if ((mask_4x4_int_0 & mask_4x4_int_1) & 1) {
          vp9_loop_filter_vertical_edge_16(s + 4, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, lfi1->mblim, lfi1->lim,
              lfi1->hev_thr);
        } else if (mask_4x4_int_0 & 1) {
          vp9_loop_filter_vertical_edge(s + 4, pitch, lfi0->mblim,
              lfi0->lim, lfi0->hev_thr, 1);
        } else {
          vp9_loop_filter_vertical_edge(s + 8 * pitch + 4, pitch,
              lfi1->mblim, lfi1->lim, lfi1->hev_thr, 1);
        }
      }
    }

    s += 8;
    lfl += 1;
    mask_16x16_0 >>= 1;
    mask_8x8_0 >>= 1;
    mask_4x4_0 >>= 1;
    mask_4x4_int_0 >>= 1;
    mask_16x16_1 >>= 1;
    mask_8x8_1 >>= 1;
    mask_4x4_1 >>= 1;
    mask_4x4_int_1 >>= 1;
  }
}

static void vp9_mb_lpf_horizontal_edge_w(uint8_t *s, int p, uint8_t *blimit,
    uint8_t *limit, uint8_t *thresh, int count) {
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
    uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
    int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);
    int8_t flat2 = flat_mask5(1, s[-8 * p], s[-7 * p], s[-6 * p], s[-5 * p],
        p0, q0, s[4 * p], s[5 * p], s[6 * p], s[7 * p]);

#if 0
    filter16(mask, hev, flat, flat2, *(s - 8 * p), s - 7 * p, s - 6 * p,
        s - 5 * p, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p, s,
        s + 1 * p, s + 2 * p, s + 3 * p, s + 4 * p, s + 5 * p,
        s + 6 * p, *(s + 7 * p));
#else
    if (flat2 && flat && mask) {
      filter16_t_dsp(*(s - 8 * p), s - 7 * p, s - 6 * p, s - 5 * p, s - 4 * p,
          s - 3 * p, s - 2 * p, s - 1 * p, s, s + 1 * p, s + 2 * p,
          s + 3 * p, s + 4 * p, s + 5 * p, s + 6 * p, *(s + 7 * p));
    } else if (flat && mask) {
      filter8_t(s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p, s, s + 1 * p,
          s + 2 * p, s + 3 * p);
    } else {
      filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
    }
#endif
    ++s;
  }

}

static void vp9_mbloop_filter_horizontal_edge_16(uint8_t *s, int p /* pitch */,
    uint8_t *blimit0, uint8_t *limit0, uint8_t *thresh0, uint8_t *blimit1,
    uint8_t *limit1, uint8_t *thresh1) {
  int i, j;
  uint8_t *blimit = blimit0;
  uint8_t *limit = limit0;
  uint8_t *thresh = thresh0;

  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 8; ++j) {
      uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
      uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];

      int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1,
          q2, q3);
      int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
      int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);

#if 0
      filter8(mask, hev, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
          s, s + 1 * p, s + 2 * p, s + 3 * p);
#else
      if (flat && mask) {
        filter8_t(s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p, s,
            s + 1 * p, s + 2 * p, s + 3 * p);
      } else {
        filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
      }
#endif

      ++s;
    }
    blimit = blimit1;
    limit = limit1;
    thresh = thresh1;
  }
}

static void vp9_loop_filter_horizontal_edge(uint8_t *s, int p /* pitch */,
    uint8_t *blimit, uint8_t *limit, uint8_t *thresh, int count) {
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
    uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
    filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
    ++s;
  }
}

static void vp9_loop_filter_horizontal_edge_16(uint8_t *s, int p /* pitch */,
    uint8_t *blimit0, uint8_t *limit0, uint8_t *thresh0, uint8_t *blimit1,
    uint8_t *limit1, uint8_t *thresh1) {
  int i, j;
  uint8_t *blimit = blimit0;
  uint8_t *limit = limit0;
  uint8_t *thresh = thresh0;

  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 8; ++j) {
      uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
      uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];
      int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1,
          q2, q3);
      int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
      filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
      ++s;
    }
    blimit = blimit1;
    limit = limit1;
    thresh = thresh1;
  }
}

static void vp9_mbloop_filter_horizontal_edge(uint8_t *s, int p,
    uint8_t *blimit, uint8_t *limit, uint8_t *thresh, int count) {
  int i;

  // loop filter designed to work using chars so that we can make maximum use
  // of 8 bit simd instructions.
  for (i = 0; i < 8 * count; ++i) {
    uint8_t p3 = s[-4 * p], p2 = s[-3 * p], p1 = s[-2 * p], p0 = s[-p];
    uint8_t q0 = s[0 * p], q1 = s[1 * p], q2 = s[2 * p], q3 = s[3 * p];

    int8_t mask = filter_mask(*limit, *blimit, p3, p2, p1, p0, q0, q1, q2,
        q3);
    int8_t hev = hev_mask(*thresh, p1, p0, q0, q1);
    int8_t flat = flat_mask4(1, p3, p2, p1, p0, q0, q1, q2, q3);

#if 0
    filter8(mask, hev, flat, s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p,
        s, s + 1 * p, s + 2 * p, s + 3 * p);
#else
    if (flat && mask) {
      filter8_t(s - 4 * p, s - 3 * p, s - 2 * p, s - 1 * p, s, s + 1 * p,
          s + 2 * p, s + 3 * p);
    } else {
      filter4(mask, hev, s - 2 * p, s - 1 * p, s, s + 1 * p);
    }
#endif
    ++s;
  }
}

static void filter_selectively_horiz(uint8_t *s, int pitch,
    unsigned int mask_16x16, unsigned int mask_8x8, unsigned int mask_4x4,
    unsigned int mask_4x4_int, loop_filter_info_n *lfi_n, uint8_t *lfl) {
  unsigned int mask;
  int count;

  for (mask = mask_16x16 | mask_8x8 | mask_4x4 | mask_4x4_int; mask; mask >>=
      count) {
    loop_filter_thresh *lfi = lfi_n->lfthr + *lfl;

    count = 1;
    if (mask & 1) {
      if (mask_16x16 & 1) {
        if ((mask_16x16 & 3) == 3) {
          vp9_mb_lpf_horizontal_edge_w(s, pitch, lfi->mblim, lfi->lim,
              lfi->hev_thr, 2);
          count = 2;
        } else {
          vp9_mb_lpf_horizontal_edge_w(s, pitch, lfi->mblim, lfi->lim,
              lfi->hev_thr, 1);
        }
      } else if (mask_8x8 & 1) {
        if ((mask_8x8 & 3) == 3) {
          // Next block's thresholds
          loop_filter_thresh *lfin = lfi_n->lfthr + *(lfl + 1);

          vp9_mbloop_filter_horizontal_edge_16(s, pitch, lfi->mblim,
              lfi->lim, lfi->hev_thr, lfin->mblim, lfin->lim,
              lfin->hev_thr);

          if ((mask_4x4_int & 3) == 3) {
            vp9_loop_filter_horizontal_edge_16(s + 4 * pitch, pitch,
                lfi->mblim, lfi->lim, lfi->hev_thr, lfin->mblim,
                lfin->lim, lfin->hev_thr);
          } else {
            if (mask_4x4_int & 1)
              vp9_loop_filter_horizontal_edge(s + 4 * pitch,
                  pitch, lfi->mblim, lfi->lim, lfi->hev_thr,
                  1);
            else if (mask_4x4_int & 2)
              vp9_loop_filter_horizontal_edge(s + 8 + 4 * pitch,
                  pitch, lfin->mblim, lfin->lim,
                  lfin->hev_thr, 1);
          }
          count = 2;
        } else {
          vp9_mbloop_filter_horizontal_edge(s, pitch, lfi->mblim,
              lfi->lim, lfi->hev_thr, 1);

          if (mask_4x4_int & 1)
            vp9_loop_filter_horizontal_edge(s + 4 * pitch, pitch,
                lfi->mblim, lfi->lim, lfi->hev_thr, 1);
        }
      } else if (mask_4x4 & 1) {
        if ((mask_4x4 & 3) == 3) {
          // Next block's thresholds
          loop_filter_thresh *lfin = lfi_n->lfthr + *(lfl + 1);

          vp9_loop_filter_horizontal_edge_16(s, pitch, lfi->mblim,
              lfi->lim, lfi->hev_thr, lfin->mblim, lfin->lim,
              lfin->hev_thr);
          if ((mask_4x4_int & 3) == 3) {
            vp9_loop_filter_horizontal_edge_16(s + 4 * pitch, pitch,
                lfi->mblim, lfi->lim, lfi->hev_thr, lfin->mblim,
                lfin->lim, lfin->hev_thr);
          } else {
            if (mask_4x4_int & 1)
              vp9_loop_filter_horizontal_edge(s + 4 * pitch,
                  pitch, lfi->mblim, lfi->lim, lfi->hev_thr,
                  1);
            else if (mask_4x4_int & 2)
              vp9_loop_filter_horizontal_edge(s + 8 + 4 * pitch,
                  pitch, lfin->mblim, lfin->lim,
                  lfin->hev_thr, 1);
          }
          count = 2;
        } else {
          vp9_loop_filter_horizontal_edge(s, pitch, lfi->mblim,
              lfi->lim, lfi->hev_thr, 1);

          if (mask_4x4_int & 1)
            vp9_loop_filter_horizontal_edge(s + 4 * pitch, pitch,
                lfi->mblim, lfi->lim, lfi->hev_thr, 1);
        }
      } else if (mask_4x4_int & 1) {
        vp9_loop_filter_horizontal_edge(s + 4 * pitch, pitch,
            lfi->mblim, lfi->lim, lfi->hev_thr, 1);
      }
    }
    s += 8 * count;
    lfl += count;
    mask_16x16 >>= count;
    mask_8x8 >>= count;
    mask_4x4 >>= count;
    mask_4x4_int >>= count;
  }
}

static void filter_block_plane_y(loop_filter_info_n *lf_info,
    LOOP_FILTER_MASK *lfm, int stride, uint8_t *buf, int mi_rows,
    int mi_row) {
  uint8_t* dst0 = buf;
  int r;  //, c;

  uint64_t mask_16x16 = lfm->left_y[TX_16X16];
  uint64_t mask_8x8 = lfm->left_y[TX_8X8];
  uint64_t mask_4x4 = lfm->left_y[TX_4X4];
  uint64_t mask_4x4_int = lfm->int_4x4_y;

  // Vertical pass: do 2 rows at one time
  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 2) {
    unsigned int mask_16x16_l = 0;
    unsigned int mask_8x8_l = 0;
    unsigned int mask_4x4_l = 0;
    unsigned int mask_4x4_int_l = 0;

#ifndef  LOOPFILTER_OPT_INTRINSICS
    mask_16x16_l = mask_16x16 & 0xffff;
    mask_8x8_l = mask_8x8 & 0xffff;
    mask_4x4_l = mask_4x4 & 0xffff;
    mask_4x4_int_l = mask_4x4_int & 0xffff;
#else //LOOPFILTER_OPT_INTRINSICS
    switch (r) {
    case 0: {
      mask_16x16_l = (unsigned int) Q6_P_extractu_PII(mask_16x16, 16, 0);
      mask_8x8_l = (unsigned int) Q6_P_extractu_PII(mask_8x8, 16, 0);
      mask_4x4_l = (unsigned int) Q6_P_extractu_PII(mask_4x4, 16, 0);
      mask_4x4_int_l = (unsigned int) Q6_P_extractu_PII(mask_4x4_int, 16,
          0);
      break;
    }
    case 2: {
      mask_16x16_l = (unsigned int) Q6_P_extractu_PII(mask_16x16, 16, 16);
      mask_8x8_l = (unsigned int) Q6_P_extractu_PII(mask_8x8, 16, 16);
      mask_4x4_l = (unsigned int) Q6_P_extractu_PII(mask_4x4, 16, 16);
      mask_4x4_int_l = (unsigned int) Q6_P_extractu_PII(mask_4x4_int, 16,
          16);
      break;
    }
    case 4: {
      mask_16x16_l = (unsigned int) Q6_P_extractu_PII(mask_16x16, 16, 32);
      mask_8x8_l = (unsigned int) Q6_P_extractu_PII(mask_8x8, 16, 32);
      mask_4x4_l = (unsigned int) Q6_P_extractu_PII(mask_4x4, 16, 32);
      mask_4x4_int_l = (unsigned int) Q6_P_extractu_PII(mask_4x4_int, 16,
          32);
      break;
    }
    case 6: {
      mask_16x16_l = (unsigned int) Q6_P_extractu_PII(mask_16x16, 16, 48);
      mask_8x8_l = (unsigned int) Q6_P_extractu_PII(mask_8x8, 16, 48);
      mask_4x4_l = (unsigned int) Q6_P_extractu_PII(mask_4x4, 16, 48);
      mask_4x4_int_l = (unsigned int) Q6_P_extractu_PII(mask_4x4_int, 16,
          48);
      break;
    }
    default: {
      /* NA */
      mask_16x16_l = 0;
      break;
    }
    }

#endif //LOOPFILTER_OPT_INTRINSICS
    // Disable filtering on the leftmost column
    filter_selectively_vert_row2(PLANE_TYPE_Y_WITH_DC, buf, stride,
        mask_16x16_l, mask_8x8_l, mask_4x4_l, mask_4x4_int_l, lf_info,
        &lfm->lfl_y[r << 3]);

    buf += 16 * stride;

#ifndef  LOOPFILTER_OPT_INTRINSICS
    mask_16x16 >>= 16;
    mask_8x8 >>= 16;
    mask_4x4 >>= 16;
    mask_4x4_int >>= 16;
#else //LOOPFILTER_OPT_INTRINSICS
#endif //LOOPFILTER_OPT_INTRINSICS
  }

  // Horizontal pass
  buf = dst0;
  mask_16x16 = lfm->above_y[TX_16X16];
  mask_8x8 = lfm->above_y[TX_8X8];
  mask_4x4 = lfm->above_y[TX_4X4];
  mask_4x4_int = lfm->int_4x4_y;

  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r++) {
    unsigned int mask_16x16_r;
    unsigned int mask_8x8_r;
    unsigned int mask_4x4_r;

    if (mi_row + r == 0) {
      mask_16x16_r = 0;
      mask_8x8_r = 0;
      mask_4x4_r = 0;
    } else {
      mask_16x16_r = mask_16x16 & 0xff;
      mask_8x8_r = mask_8x8 & 0xff;
      mask_4x4_r = mask_4x4 & 0xff;
    }

    filter_selectively_horiz(buf, stride, mask_16x16_r, mask_8x8_r,
        mask_4x4_r, mask_4x4_int & 0xff, lf_info, &lfm->lfl_y[r << 3]);

    buf += 8 * stride;
    mask_16x16 >>= 8;
    mask_8x8 >>= 8;
    mask_4x4 >>= 8;
    mask_4x4_int >>= 8;
  }

}

static void filter_block_plane_uv(loop_filter_info_n *lf_info,
    LOOP_FILTER_MASK *lfm, int stride, uint8_t *buf, int mi_rows,
    int mi_row) {
  uint8_t* dst0 = buf;
  int r, c;

  uint16_t mask_16x16 = lfm->left_uv[TX_16X16];
  uint16_t mask_8x8 = lfm->left_uv[TX_8X8];
  uint16_t mask_4x4 = lfm->left_uv[TX_4X4];
  uint16_t mask_4x4_int = lfm->int_4x4_uv;

  // Vertical pass: do 2 rows at one time
  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 4) {

    for (c = 0; c < (MI_BLOCK_SIZE >> 1); c++) {
      lfm->lfl_uv[(r << 1) + c] = lfm->lfl_y[(r << 3) + (c << 1)];
      lfm->lfl_uv[((r + 2) << 1) + c] = lfm->lfl_y[((r + 2) << 3)
          + (c << 1)];
    }

    {
      unsigned int mask_16x16_l = mask_16x16 & 0xff;
      unsigned int mask_8x8_l = mask_8x8 & 0xff;
      unsigned int mask_4x4_l = mask_4x4 & 0xff;
      unsigned int mask_4x4_int_l = mask_4x4_int & 0xff;

      // Disable filtering on the leftmost column
      filter_selectively_vert_row2(PLANE_TYPE_UV, buf, stride,
          mask_16x16_l, mask_8x8_l, mask_4x4_l, mask_4x4_int_l,
          lf_info, &lfm->lfl_uv[r << 1]);

      buf += 16 * stride;
      mask_16x16 >>= 8;
      mask_8x8 >>= 8;
      mask_4x4 >>= 8;
      mask_4x4_int >>= 8;
    }
  }

  // Horizontal pass
  buf = dst0;
  mask_16x16 = lfm->above_uv[TX_16X16];
  mask_8x8 = lfm->above_uv[TX_8X8];
  mask_4x4 = lfm->above_uv[TX_4X4];
  mask_4x4_int = lfm->int_4x4_uv;

  for (r = 0; r < MI_BLOCK_SIZE && mi_row + r < mi_rows; r += 2) {
    int skip_border_4x4_r = mi_row + r == mi_rows - 1;
    unsigned int mask_4x4_int_r =
        skip_border_4x4_r ? 0 : (mask_4x4_int & 0xf);
    unsigned int mask_16x16_r;
    unsigned int mask_8x8_r;
    unsigned int mask_4x4_r;

    if (mi_row + r == 0) {
      mask_16x16_r = 0;
      mask_8x8_r = 0;
      mask_4x4_r = 0;
    } else {
      mask_16x16_r = mask_16x16 & 0xf;
      mask_8x8_r = mask_8x8 & 0xf;
      mask_4x4_r = mask_4x4 & 0xf;
    }

    filter_selectively_horiz(buf, stride, mask_16x16_r, mask_8x8_r,
        mask_4x4_r, mask_4x4_int_r, lf_info, &lfm->lfl_uv[r << 1]);

    buf += 8 * stride;
    mask_16x16 >>= 4;
    mask_8x8 >>= 4;
    mask_4x4 >>= 4;
    mask_4x4_int >>= 4;
  }

}

typedef struct {
  dspCV_synctoken_t *token;            // worker pool token
  loop_filter_info_n *lf_info;
  LOOP_FILTER_MASK *lfms;
  int start;
  int stop;
  int mi_cols;
  int stride;
  uint8_t *bufStart;
  int mi_rows;
  int chroma;
} lfMultiThread;

typedef struct {
  dspCV_synctoken_t  *token;         // worker pool token
  unsigned int        odd;           // the odd worker
  unsigned int        even_job_count;
  unsigned int        odd_job_count;
  loop_filter_info_n *lf_info;
  LOOP_FILTER_MASK   *lfms;
  int start;
  int stop;
  int mi_cols;
  int stride;
  uint8_t *buf_start;
  int mi_rows;
} LF_CALLBACK_DATA_Y;

typedef struct {
  dspCV_synctoken_t *token;         // worker pool token
  loop_filter_info_n *lf_info;
  LOOP_FILTER_MASK *lfms;
  int start;
  int stop;
  int mi_cols;
  int stride;
  uint8_t *buf_start_u;
  uint8_t *buf_start_v;
  int mi_rows;
} LF_CALLBACK_DATA_UV;

static void filter_block_plane_loop_callback_uv(void* data) {
  int mi_row, mi_col;
  uint8_t *buf = NULL;
  int lfm_idx = 0;
  LF_CALLBACK_DATA_UV *dptr = (LF_CALLBACK_DATA_UV *) data;

  loop_filter_info_n *lf_info = dptr->lf_info;
  LOOP_FILTER_MASK *lfms = dptr->lfms;
  const int start = dptr->start;
  const int stop = dptr->stop;
  const int mi_cols = dptr->mi_cols;
  const int stride = dptr->stride;
  uint8_t *buf_start = dptr->buf_start_u;
  int mi_rows = dptr->mi_rows;

  int times = 2;
  while(times) {
    for (mi_row = start; mi_row < stop; mi_row += MI_BLOCK_SIZE) {
      buf = buf_start + ((mi_row * MI_BLOCK_SIZE * stride) >> 1);

      for (mi_col = 0; mi_col < (mi_cols + 1) >> 1; mi_col += MI_BLOCK_SIZE) {
#ifdef LOOPFILTER_L2_PREFETCH
        // Issue L2 prefetch
        dspcache_box_l2fetch((buf - (8 * stride) - 8), (L2_PREFETCH_WIDTH >> 1) + 8,
            L2_PREFETCH_HEIGHT >> 1, stride);
        //dspcache_linear_l2fetch((uint8_t*)(lfms + lfm_idx), sizeof(LOOP_FILTER_MASK));
#endif // LOOPFILTER_L2_PREFETCH
        filter_block_plane_uv(lf_info, lfms + lfm_idx, stride, buf,
                             mi_rows, mi_row);
        lfm_idx++;
        buf += (MI_BLOCK_SIZE * MI_BLOCK_SIZE) >> 1;
        filter_block_plane_uv(lf_info, lfms + lfm_idx, stride, buf,
                             mi_rows, mi_row);
        lfm_idx++;
        buf += (MI_BLOCK_SIZE * MI_BLOCK_SIZE) >> 1;

      }
    }
    times--;
    buf_start = dptr->buf_start_v;
    lfm_idx = 0;
  }
#ifdef LOOPFILTER_HW_MT
  dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
#endif //LOOPFILTER_HW_MT
  return;
}

#ifdef LOOPFILTER_HW_MT
void filter_block_plane_loop_callback_y_mt(void* data) {
  LF_CALLBACK_DATA_Y *dptr = (LF_CALLBACK_DATA_Y *) data;
  int mi_row, mi_col;
  uint8_t *buf = NULL;
  int lfm_idx = 0;

  loop_filter_info_n *lf_info = dptr->lf_info;
  LOOP_FILTER_MASK *lfms = dptr->lfms;
  const int start = dptr->start;
  const int stop = dptr->stop;
  const int mi_cols = dptr->mi_cols;
  const int stride = dptr->stride;
  uint8_t *buf_start = dptr->buf_start;
  const int mi_rows = dptr->mi_rows;

  const unsigned int odd = dspCV_atomic_inc_return(&dptr->odd) - 1;
  if (odd) {
    lfm_idx = (mi_cols + 1) / MI_BLOCK_SIZE;
    mi_row = start + MI_BLOCK_SIZE;
  } else {
    lfm_idx = 0;
    mi_row = start;
  }

  for (;
       mi_row < stop;
       mi_row += MI_BLOCK_SIZE * 2) {
    buf = buf_start + mi_row * MI_BLOCK_SIZE * stride;

    for (mi_col = 0; mi_col < (mi_cols + 1) >> 1; mi_col += MI_BLOCK_SIZE) {
      if (odd) {
        while (!atomic_read(&dptr->odd_job_count)) {
          sleep_10_cycles();
        }
        (void)dspCV_atomic_dec_return(&dptr->odd_job_count);
      } else {
        while (!atomic_read(&dptr->even_job_count)) {
          sleep_5_cycles();
        }
        (void)dspCV_atomic_dec_return(&dptr->even_job_count);
      }

#ifdef LOOPFILTER_L2_PREFETCH
      // Issue L2 prefetch
      dspcache_box_l2fetch((buf - (8 * stride) - 8), L2_PREFETCH_WIDTH + 8,
          L2_PREFETCH_HEIGHT, stride);
      //dspcache_linear_l2fetch((uint8_t*)(lfms + lfm_idx), sizeof(LOOP_FILTER_MASK));
#endif // LOOPFILTER_L2_PREFETCH
      filter_block_plane_y(lf_info, lfms + lfm_idx, stride, buf,
            mi_rows, mi_row);

      lfm_idx++;
      buf += MI_BLOCK_SIZE * MI_BLOCK_SIZE;

      filter_block_plane_y(lf_info, lfms + lfm_idx, stride, buf,
            mi_rows, mi_row);

      lfm_idx++;
      buf += MI_BLOCK_SIZE * MI_BLOCK_SIZE;

      if (odd) {
        (void)dspCV_atomic_inc_return(&dptr->even_job_count);
      } else {
        (void)dspCV_atomic_inc_return(&dptr->odd_job_count);
      }
    }
    lfm_idx += (mi_cols + 1) / MI_BLOCK_SIZE;
  }
  dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
  return;
}
#endif // LOOPFILTER_HW_MT

void filter_block_plane_loop_callback_y(void* data) {

  int mi_row, mi_col;
  uint8_t *buf = NULL;
  int lfm_idx = 0;
  LF_CALLBACK_DATA_Y *dptr = (LF_CALLBACK_DATA_Y *) data;
  loop_filter_info_n *lf_info = dptr->lf_info;
  LOOP_FILTER_MASK *lfms = dptr->lfms;
  const int start = dptr->start;
  const int stop = dptr->stop;
  const int mi_cols = dptr->mi_cols;
  const int stride = dptr->stride;
  uint8_t *buf_start = dptr->buf_start;
  const int mi_rows = dptr->mi_rows;

  for (mi_row = start; mi_row < stop; mi_row += MI_BLOCK_SIZE) {
    buf = buf_start + (mi_row * MI_BLOCK_SIZE * stride);
    for (mi_col = 0; mi_col < (mi_cols + 1) >> 1; mi_col += MI_BLOCK_SIZE) {

#ifdef LOOPFILTER_L2_PREFETCH
      // Issue L2 prefetch
      dspcache_box_l2fetch((buf - (8 * stride) - 8), L2_PREFETCH_WIDTH + 8,
          L2_PREFETCH_HEIGHT, stride);
      //dspcache_linear_l2fetch((uint8_t*)(lfms + lfm_idx), sizeof(LOOP_FILTER_MASK));
#endif // LOOPFILTER_L2_PREFETCH
      filter_block_plane_y(lf_info, lfms + lfm_idx, stride, buf,
            mi_rows, mi_row);

      lfm_idx++;
      buf += MI_BLOCK_SIZE * MI_BLOCK_SIZE;

      filter_block_plane_y(lf_info, lfms + lfm_idx, stride, buf,
            mi_rows, mi_row);

      lfm_idx++;
      buf += MI_BLOCK_SIZE * MI_BLOCK_SIZE;
    }
  }
  return;
}


#define DEFAULT_PLANE_NUM   3

int loopfilter_rows_work_dsp(int start, int stop, int num_planes, int mi_rows,
    int mi_cols, uint8_t *buffer_alloc, int len_buffer_alloc,
    BUF_INFO *buf_infos, int len_buf_infos, const uint8_t *lf_info_s,
    int len_lf_info, LOOP_FILTER_MASK *lfms, int len_lfms) {
  static int loopfilterEntryCounter = 0;

  loopfilterEntryCounter++;
  if (loopfilterEntryCounter < 3) {
    return 0;
  }
  assert(DEFAULT_PLANE_NUM <= DSPCV_NUM_WORKERS);

  BUF_INFO *buf_info = NULL;
  uint8_t *buf_start = NULL;
  int stride = 0;

  LF_CALLBACK_DATA_Y cb_data_y = {0};
#ifdef LOOPFILTER_HW_MT
  dspCV_worker_job_t job;
  dspCV_synctoken_t token;
  dspCV_worker_pool_synctoken_init(&token, DEFAULT_PLANE_NUM);
  cb_data_y.token = &token;
#endif //LOOPFILTER_HW_MT
  loop_filter_info_n *lf_info = (loop_filter_info_n *) lf_info_s;

  buf_info = buf_infos;
  buf_start = buffer_alloc + buf_info->offset;
  stride = buf_info->stride;

  cb_data_y.odd = 0;
  cb_data_y.even_job_count = (mi_cols + 1) / (MI_BLOCK_SIZE * 2);
  cb_data_y.odd_job_count = 0;
  cb_data_y.lf_info = lf_info;
  cb_data_y.lfms = lfms;
  cb_data_y.start = start;
  cb_data_y.stop = stop;
  cb_data_y.mi_cols = mi_cols;
  cb_data_y.stride = stride;
  cb_data_y.buf_start = buf_start;
  cb_data_y.mi_rows = mi_rows;

#ifdef LOOPFILTER_HW_MT
  job.fptr = filter_block_plane_loop_callback_y_mt;
  job.dptr = (void *) &cb_data_y;
  (void) dspCV_worker_pool_submit(job);
  (void) dspCV_worker_pool_submit(job);
#else // LOOPFILTER_HW_MT
  filter_block_plane_loop_callback_y((void*) &cb_data_y);
#endif // LOOPFILTER_HW_MT


  LF_CALLBACK_DATA_UV cb_data_uv;
  buf_info = buf_infos + 1;
  stride = buf_info->stride;
  cb_data_uv.buf_start_u = buffer_alloc + buf_info->offset;
  buf_info = buf_infos + 2;
  cb_data_uv.buf_start_v = buffer_alloc + buf_info->offset;

  cb_data_uv.lf_info = lf_info;
  cb_data_uv.lfms = lfms;
  cb_data_uv.start = start;
  cb_data_uv.stop = stop;
  cb_data_uv.mi_cols = mi_cols;
  cb_data_uv.stride = stride;
  cb_data_uv.mi_rows = mi_rows;

#ifdef LOOPFILTER_HW_MT
  cb_data_uv.token = &token;
  job.fptr = filter_block_plane_loop_callback_uv;
  job.dptr = (void *) &cb_data_uv;
  (void) dspCV_worker_pool_submit(job);
#else // LOOPFILTER_HW_MT
  filter_block_plane_loop_callback_uv((void*) &cb_data_uv);
#endif //LOOPFILTER_HW_MT

#ifdef LOOPFILTER_HW_MT
  dspCV_worker_pool_synctoken_wait(&token);
#endif //#LOOPFILTER_HW_MT

  return 0;
}
