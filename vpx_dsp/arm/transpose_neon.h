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

#define BIG_TO_SMALL
// In the case of the 8x8 hadamard, both styles of transpose generate the same
// number of instructions of the same type, in approximately reverse order.
#ifdef BIG_TO_SMALL
static void transpose_16_8x8(int16x8_t *a0, int16x8_t *a1, int16x8_t *a2,
                             int16x8_t *a3, int16x8_t *a4, int16x8_t *a5,
                             int16x8_t *a6, int16x8_t *a7) {
  // Swap 64 bit elements. Goes from:
  // a0: 00 01 02 03 04 05 06 07
  // a1: 08 09 10 11 12 13 14 15
  // a2: 16 17 18 19 20 21 22 23
  // a3: 24 25 26 27 28 29 30 31
  // a4: 32 33 34 35 36 37 38 39
  // a5: 40 41 42 43 44 45 46 47
  // a6: 48 49 50 51 52 53 54 55
  // a7: 56 57 58 59 60 61 62 63
  // to:
  // a04_lo: 00 01 02 03 32 33 34 35
  // a15_lo: 08 09 10 11 40 41 42 43
  // a26_lo: 16 17 18 19 48 49 50 51
  // a37_lo: 24 25 26 27 56 57 58 59
  // a04_hi: 04 05 06 07 36 37 38 39
  // a15_hi: 12 13 14 15 44 45 46 47
  // a26_hi: 20 21 22 23 52 53 54 55
  // a37_hi: 28 29 30 31 60 61 62 63
  const int16x8_t a04_lo = vcombine_s16(vget_low_s16(*a0), vget_low_s16(*a4));
  const int16x8_t a15_lo = vcombine_s16(vget_low_s16(*a1), vget_low_s16(*a5));
  const int16x8_t a26_lo = vcombine_s16(vget_low_s16(*a2), vget_low_s16(*a6));
  const int16x8_t a37_lo = vcombine_s16(vget_low_s16(*a3), vget_low_s16(*a7));
  const int16x8_t a04_hi = vcombine_s16(vget_high_s16(*a0), vget_high_s16(*a4));
  const int16x8_t a15_hi = vcombine_s16(vget_high_s16(*a1), vget_high_s16(*a5));
  const int16x8_t a26_hi = vcombine_s16(vget_high_s16(*a2), vget_high_s16(*a6));
  const int16x8_t a37_hi = vcombine_s16(vget_high_s16(*a3), vget_high_s16(*a7));

  // Swap 32 bit elements resulting in:
  // a0246_lo:
  // 00 01 16 17 32 33 48 49
  // 02 03 18 19 34 35 50 51
  // a1357_lo:
  // 08 09 24 25 40 41 56 57
  // 10 11 26 27 42 43 58 59
  // a0246_hi:
  // 04 05 20 21 36 37 52 53
  // 06 07 22 23 38 39 54 55
  // a1657_hi:
  // 12 13 28 29 44 45 60 61
  // 14 15 30 31 46 47 62 63
  const int32x4x2_t a0246_lo =
      vtrnq_s32(vreinterpretq_s32_s16(a04_lo), vreinterpretq_s32_s16(a26_lo));
  const int32x4x2_t a1357_lo =
      vtrnq_s32(vreinterpretq_s32_s16(a15_lo), vreinterpretq_s32_s16(a37_lo));
  const int32x4x2_t a0246_hi =
      vtrnq_s32(vreinterpretq_s32_s16(a04_hi), vreinterpretq_s32_s16(a26_hi));
  const int32x4x2_t a1357_hi =
      vtrnq_s32(vreinterpretq_s32_s16(a15_hi), vreinterpretq_s32_s16(a37_hi));

  // Swap 16 bit elements resulting in:
  // b0:
  // 00 08 16 24 32 40 48 56
  // 01 09 17 25 33 41 49 57
  // b1:
  // 02 10 18 26 34 42 50 58
  // 03 11 19 27 35 43 51 59
  // b2:
  // 04 12 20 28 36 44 52 60
  // 05 13 21 29 37 45 53 61
  // b3:
  // 06 14 22 30 38 46 54 62
  // 07 15 23 31 39 47 55 63
  const int16x8x2_t b0 = vtrnq_s16(vreinterpretq_s16_s32(a0246_lo.val[0]),
                                   vreinterpretq_s16_s32(a1357_lo.val[0]));
  const int16x8x2_t b1 = vtrnq_s16(vreinterpretq_s16_s32(a0246_lo.val[1]),
                                   vreinterpretq_s16_s32(a1357_lo.val[1]));
  const int16x8x2_t b2 = vtrnq_s16(vreinterpretq_s16_s32(a0246_hi.val[0]),
                                   vreinterpretq_s16_s32(a1357_hi.val[0]));
  const int16x8x2_t b3 = vtrnq_s16(vreinterpretq_s16_s32(a0246_hi.val[1]),
                                   vreinterpretq_s16_s32(a1357_hi.val[1]));

  *a0 = b0.val[0];
  *a1 = b0.val[1];
  *a2 = b1.val[0];
  *a3 = b1.val[1];
  *a4 = b2.val[0];
  *a5 = b2.val[1];
  *a6 = b3.val[0];
  *a7 = b3.val[1];
}
#else
static void transpose_16_8x8(int16x8_t *a0, int16x8_t *a1, int16x8_t *a2,
                             int16x8_t *a3, int16x8_t *a4, int16x8_t *a5,
                             int16x8_t *a6, int16x8_t *a7) {
  // Swap 16 bit elements. Goes from:
  // a0: 00 01 02 03 04 05 06 07
  // a1: 08 09 10 11 12 13 14 15
  // a2: 16 17 18 19 20 21 22 23
  // a3: 24 25 26 27 28 29 30 31
  // a4: 32 33 34 35 36 37 38 39
  // a5: 40 41 42 43 44 45 46 47
  // a6: 48 49 50 51 52 53 54 55
  // a7: 56 57 58 59 60 61 62 63
  // to:
  // b0.val[0]: 00 08 02 10 04 12 06 14
  // b0.val[1]: 01 09 03 11 05 13 07 15
  // b1.val[0]: 16 24 18 26 20 28 22 30
  // b1.val[1]: 17 25 19 27 21 29 23 31
  // b2.val[0]: 32 40 34 42 36 44 38 46
  // b2.val[1]: 33 41 35 43 37 45 39 47
  // b3.val[0]: 48 56 50 58 52 60 54 62
  // b3.val[1]: 49 57 51 59 53 61 55 63

  const int16x8x2_t b0 = vtrnq_s16(*a0, *a1);
  const int16x8x2_t b1 = vtrnq_s16(*a2, *a3);
  const int16x8x2_t b2 = vtrnq_s16(*a4, *a5);
  const int16x8x2_t b3 = vtrnq_s16(*a6, *a7);

  // Swap 32 bit elements resulting in:
  // c0.val[0]: 00 08 16 24 04 12 20 28
  // c0.val[1]: 02 10 18 26 06 14 22 30
  // c1.val[0]: 01 09 17 25 05 13 21 29
  // c1.val[1]: 03 11 19 27 07 15 23 31
  // c2.val[0]: 32 40 48 56 36 44 52 60
  // c2.val[1]: 34 42 50 58 38 46 54 62
  // c3.val[0]: 33 41 49 57 37 45 53 61
  // c3.val[1]: 35 43 51 59 39 47 55 63

  const int32x4x2_t c0 = vtrnq_s32(vreinterpretq_s32_s16(b0.val[0]),
                                   vreinterpretq_s32_s16(b1.val[0]));
  const int32x4x2_t c1 = vtrnq_s32(vreinterpretq_s32_s16(b0.val[1]),
                                   vreinterpretq_s32_s16(b1.val[1]));
  const int32x4x2_t c2 = vtrnq_s32(vreinterpretq_s32_s16(b2.val[0]),
                                   vreinterpretq_s32_s16(b3.val[0]));
  const int32x4x2_t c3 = vtrnq_s32(vreinterpretq_s32_s16(b2.val[1]),
                                   vreinterpretq_s32_s16(b3.val[1]));

  // Swap 64 bit elements again:
  // a0: 00 08 16 24 32 40 48 56
  // a1: 01 09 17 25 33 41 49 57
  // a2: 02 10 18 26 34 42 50 58
  // a3: 03 11 19 27 35 43 51 59
  // a4: 04 12 20 28 36 44 52 60
  // a5: 05 13 21 29 37 45 53 61
  // a6: 06 14 22 30 38 46 54 62
  // a7: 07 15 23 31 39 47 55 63
  *a0 = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(c0.val[0])),
                     vreinterpret_s16_s32(vget_low_s32(c2.val[0])));
  *a1 = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(c1.val[0])),
                     vreinterpret_s16_s32(vget_low_s32(c3.val[0])));
  *a2 = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(c0.val[1])),
                     vreinterpret_s16_s32(vget_low_s32(c2.val[1])));
  *a3 = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(c1.val[1])),
                     vreinterpret_s16_s32(vget_low_s32(c3.val[1])));
  *a4 = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(c0.val[0])),
                     vreinterpret_s16_s32(vget_high_s32(c2.val[0])));
  *a5 = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(c1.val[0])),
                     vreinterpret_s16_s32(vget_high_s32(c3.val[0])));
  *a6 = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(c0.val[1])),
                     vreinterpret_s16_s32(vget_high_s32(c2.val[1])));
  *a7 = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(c1.val[1])),
                     vreinterpret_s16_s32(vget_high_s32(c3.val[1])));
}
#endif
