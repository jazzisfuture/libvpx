/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_ARM_TRANSPOSE_NEON_H_
#define VPX_DSP_ARM_TRANSPOSE_NEON_H_

#include <arm_neon.h>

#include "./vpx_config.h"

// Transpose 64 bit elements as follows:
// a0: 00 01 02 03 04 05 06 07
// a1: 16 17 18 19 20 21 22 23
//
// b0.val[0]: 00 01 02 03 16 17 18 19
// b0.val[1]: 04 05 06 07 20 21 22 23
static INLINE int16x8x2_t vpx_vtrnq_s64(int32x4_t a0, int32x4_t a1) {
  int16x8x2_t b0;
  b0.val[0] = vcombine_s16(vreinterpret_s16_s32(vget_low_s32(a0)),
                           vreinterpret_s16_s32(vget_low_s32(a1)));
  b0.val[1] = vcombine_s16(vreinterpret_s16_s32(vget_high_s32(a0)),
                           vreinterpret_s16_s32(vget_high_s32(a1)));
  return b0;
}

static INLINE void transpose_s16_8x8(int16x8_t *a0, int16x8_t *a1,
                                     int16x8_t *a2, int16x8_t *a3,
                                     int16x8_t *a4, int16x8_t *a5,
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

  // Swap 64 bit elements resulting in:
  // d0.val[0]: 00 08 16 24 32 40 48 56
  // d0.val[1]: 04 12 20 28 36 44 52 60
  // d1.val[0]: 01 09 17 25 33 41 49 57
  // d1.val[1]: 05 13 21 29 37 45 53 61
  // d2.val[0]: 02 10 18 26 34 42 50 58
  // d2.val[1]: 06 14 22 30 38 46 54 62
  // d3.val[0]: 03 11 19 27 35 43 51 59
  // d3.val[1]: 07 15 23 31 39 47 55 63
  const int16x8x2_t d0 = vpx_vtrnq_s64(c0.val[0], c2.val[0]);
  const int16x8x2_t d1 = vpx_vtrnq_s64(c1.val[0], c3.val[0]);
  const int16x8x2_t d2 = vpx_vtrnq_s64(c0.val[1], c2.val[1]);
  const int16x8x2_t d3 = vpx_vtrnq_s64(c1.val[1], c3.val[1]);

  *a0 = d0.val[0];
  *a1 = d1.val[0];
  *a2 = d2.val[0];
  *a3 = d3.val[0];
  *a4 = d0.val[1];
  *a5 = d1.val[1];
  *a6 = d2.val[1];
  *a7 = d3.val[1];
}

static INLINE void transpose_u8_16x8(
    uint8x16_t i0, uint8x16_t i1, uint8x16_t i2, uint8x16_t i3, uint8x16_t i4,
    uint8x16_t i5, uint8x16_t i6, uint8x16_t i7, uint8x8_t *o0, uint8x8_t *o1,
    uint8x8_t *o2, uint8x8_t *o3, uint8x8_t *o4, uint8x8_t *o5, uint8x8_t *o6,
    uint8x8_t *o7, uint8x8_t *o8, uint8x8_t *o9, uint8x8_t *o10, uint8x8_t *o11,
    uint8x8_t *o12, uint8x8_t *o13, uint8x8_t *o14, uint8x8_t *o15) {
  // Input:
  // i0: 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F
  // i1: 10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F
  // i2: 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F
  // i3: 30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F
  // i4: 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F
  // i5: 50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F
  // i6: 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F
  // i7: 70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F
  uint8x16x2_t t0_u8x16x2, t1_u8x16x2, t2_u8x16x2, t3_u8x16x2;
  uint16x8x2_t t0_u16x8x2, t1_u16x8x2, t2_u16x8x2, t3_u16x8x2;
  uint32x4x2_t t0_u32x4x2, t1_u32x4x2, t2_u32x4x2, t3_u32x4x2;

  // t0_u8x16x2: 00 10 02 12 04 14 06 16  08 18 0A 1A 0C 1C 0E 1E
  //             01 11 03 13 05 15 07 17  09 19 0B 1B 0D 1D 0F 1F
  // t1_u8x16x2: 20 30 22 32 24 34 26 36  28 38 2A 3A 2C 3C 2E 3E
  //             21 31 23 33 25 35 27 37  29 39 2B 3B 2D 3D 2F 3F
  // t2_u8x16x2: 40 50 42 52 44 54 46 56  48 58 4A 5A 4C 5C 4E 5E
  //             41 51 43 53 45 55 47 57  49 59 4B 5B 4D 5D 4F 5F
  // t3_u8x16x2: 60 70 62 72 64 74 66 76  68 78 6A 7A 6C 7C 6E 7E
  //             61 71 63 73 65 75 67 77  69 79 6B 7B 6D 7D 6F 7F
  t0_u8x16x2 = vtrnq_u8(i0, i1);
  t1_u8x16x2 = vtrnq_u8(i2, i3);
  t2_u8x16x2 = vtrnq_u8(i4, i5);
  t3_u8x16x2 = vtrnq_u8(i6, i7);

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

  // t0_u32x4x2: 00 10 20 30 40 50 60 70  08 18 28 38 48 58 68 78
  //             04 14 24 34 44 54 64 74  0C 1C 2C 3C 4C 5C 6C 7C
  // t1_u32x4x2: 02 12 22 32 42 52 62 72  0A 1A 2A 3A 4A 5A 6A 7A
  //             06 16 26 36 46 56 66 76  0E 1E 2E 3E 4E 5E 6E 7E
  // t2_u32x4x2: 01 11 21 31 41 51 61 71  09 19 29 39 49 59 69 79
  //             05 15 25 35 45 55 65 75  0D 1D 2D 3D 4D 5D 6D 7D
  // t3_u32x4x2: 03 13 23 33 43 53 63 73  0B 1B 2B 3B 4B 5B 6B 7B
  //             07 17 27 37 47 57 67 77  0F 1F 2F 3F 4F 5F 6F 7F
  t0_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t0_u16x8x2.val[0]),
                         vreinterpretq_u32_u16(t2_u16x8x2.val[0]));
  t1_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t0_u16x8x2.val[1]),
                         vreinterpretq_u32_u16(t2_u16x8x2.val[1]));
  t2_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t1_u16x8x2.val[0]),
                         vreinterpretq_u32_u16(t3_u16x8x2.val[0]));
  t3_u32x4x2 = vtrnq_u32(vreinterpretq_u32_u16(t1_u16x8x2.val[1]),
                         vreinterpretq_u32_u16(t3_u16x8x2.val[1]));

  // Output:
  // o0 : 00 10 20 30 40 50 60 70
  // o1 : 01 11 21 31 41 51 61 71
  // o2 : 02 12 22 32 42 52 62 72
  // o3 : 03 13 23 33 43 53 63 73
  // o4 : 04 14 24 34 44 54 64 74
  // o5 : 05 15 25 35 45 55 65 75
  // o6 : 06 16 26 36 46 56 66 76
  // o7 : 07 17 27 37 47 57 67 77
  // o8 : 08 18 28 38 48 58 68 78
  // o9 : 09 19 29 39 49 59 69 79
  // o10: 0A 1A 2A 3A 4A 5A 6A 7A
  // o11: 0B 1B 2B 3B 4B 5B 6B 7B
  // o12: 0C 1C 2C 3C 4C 5C 6C 7C
  // o13: 0D 1D 2D 3D 4D 5D 6D 7D
  // o14: 0E 1E 2E 3E 4E 5E 6E 7E
  // o15: 0F 1F 2F 3F 4F 5F 6F 7F
  *o0 = vget_low_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[0]));
  *o1 = vget_low_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[0]));
  *o2 = vget_low_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[0]));
  *o3 = vget_low_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[0]));
  *o4 = vget_low_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[1]));
  *o5 = vget_low_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[1]));
  *o6 = vget_low_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[1]));
  *o7 = vget_low_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[1]));
  *o8 = vget_high_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[0]));
  *o9 = vget_high_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[0]));
  *o10 = vget_high_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[0]));
  *o11 = vget_high_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[0]));
  *o12 = vget_high_u8(vreinterpretq_u8_u32(t0_u32x4x2.val[1]));
  *o13 = vget_high_u8(vreinterpretq_u8_u32(t2_u32x4x2.val[1]));
  *o14 = vget_high_u8(vreinterpretq_u8_u32(t1_u32x4x2.val[1]));
  *o15 = vget_high_u8(vreinterpretq_u8_u32(t3_u32x4x2.val[1]));
}

static INLINE void transpose_u8_8x16(
    uint8x8_t i0, uint8x8_t i1, uint8x8_t i2, uint8x8_t i3, uint8x8_t i4,
    uint8x8_t i5, uint8x8_t i6, uint8x8_t i7, uint8x8_t i8, uint8x8_t i9,
    uint8x8_t i10, uint8x8_t i11, uint8x8_t i12, uint8x8_t i13, uint8x8_t i14,
    uint8x8_t i15, uint8x16_t *o0, uint8x16_t *o1, uint8x16_t *o2,
    uint8x16_t *o3, uint8x16_t *o4, uint8x16_t *o5, uint8x16_t *o6,
    uint8x16_t *o7) {
  // Input:
  // i0 : 00 10 20 30 40 50 60 70
  // i1 : 01 11 21 31 41 51 61 71
  // i2 : 02 12 22 32 42 52 62 72
  // i3 : 03 13 23 33 43 53 63 73
  // i4 : 04 14 24 34 44 54 64 74
  // i5 : 05 15 25 35 45 55 65 75
  // i6 : 06 16 26 36 46 56 66 76
  // i7 : 07 17 27 37 47 57 67 77
  // i8 : 08 18 28 38 48 58 68 78
  // i9 : 09 19 29 39 49 59 69 79
  // i10: 0A 1A 2A 3A 4A 5A 6A 7A
  // i11: 0B 1B 2B 3B 4B 5B 6B 7B
  // i12: 0C 1C 2C 3C 4C 5C 6C 7C
  // i13: 0D 1D 2D 3D 4D 5D 6D 7D
  // i14: 0E 1E 2E 3E 4E 5E 6E 7E
  // i15: 0F 1F 2F 3F 4F 5F 6F 7F
  uint8x16x2_t t0_u8x16x2, t1_u8x16x2, t2_u8x16x2, t3_u8x16x2;
  uint16x8x2_t t0_u16x8x2, t1_u16x8x2, t2_u16x8x2, t3_u16x8x2;
  uint32x4x2_t t0_u32x4x2, t1_u32x4x2, t2_u32x4x2, t3_u32x4x2;

  // t0_u32x4x2: 00 10 20 30 40 50 60 70  08 18 28 38 48 58 68 78  P7 Q0
  //             04 14 24 34 44 54 64 74  0C 1C 2C 3C 4C 5C 6C 7C  P3 Q4
  // t1_u32x4x2: 02 12 22 32 42 52 62 72  0A 1A 2A 3A 4A 5A 6A 7A  P5 Q2
  //             06 16 26 36 46 56 66 76  0E 1E 2E 3E 4E 5E 6E 7E  P1 Q6
  // t2_u32x4x2: 01 11 21 31 41 51 61 71  09 19 29 39 49 59 69 79  P6 Q1
  //             05 15 25 35 45 55 65 75  0D 1D 2D 3D 4D 5D 6D 7D  P2 Q5
  // t3_u32x4x2: 03 13 23 33 43 53 63 73  0B 1B 2B 3B 4B 5B 6B 7B  P4 Q3
  //             07 17 27 37 47 57 67 77  0F 1F 2F 3F 4F 5F 6F 7F  P0 Q7
  t0_u32x4x2.val[0] = vreinterpretq_u32_u8(vcombine_u8(i0, i8));
  t0_u32x4x2.val[1] = vreinterpretq_u32_u8(vcombine_u8(i4, i12));
  t1_u32x4x2.val[0] = vreinterpretq_u32_u8(vcombine_u8(i2, i10));
  t1_u32x4x2.val[1] = vreinterpretq_u32_u8(vcombine_u8(i6, i14));
  t2_u32x4x2.val[0] = vreinterpretq_u32_u8(vcombine_u8(i1, i9));
  t2_u32x4x2.val[1] = vreinterpretq_u32_u8(vcombine_u8(i5, i13));
  t3_u32x4x2.val[0] = vreinterpretq_u32_u8(vcombine_u8(i3, i11));
  t3_u32x4x2.val[1] = vreinterpretq_u32_u8(vcombine_u8(i7, i15));

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

  // Output:
  // o0 : 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F
  // o1 : 10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F
  // o2 : 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F
  // o3 : 30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F
  // o4 : 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F
  // o5 : 50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F
  // o6 : 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F
  // o7 : 70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F
  *o0 = t0_u8x16x2.val[0];
  *o1 = t0_u8x16x2.val[1];
  *o2 = t1_u8x16x2.val[0];
  *o3 = t1_u8x16x2.val[1];
  *o4 = t2_u8x16x2.val[0];
  *o5 = t2_u8x16x2.val[1];
  *o6 = t3_u8x16x2.val[0];
  *o7 = t3_u8x16x2.val[1];
}

#endif  // VPX_DSP_ARM_TRANSPOSE_NEON_H_
