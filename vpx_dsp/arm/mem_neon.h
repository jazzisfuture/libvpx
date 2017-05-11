/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_ARM_MEM_NEON_H_
#define VPX_DSP_ARM_MEM_NEON_H_

#include <arm_neon.h>
#include <assert.h>
#include <string.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/vpx_dsp_common.h"

//------------------------------------------------------------------------------
// Helper functions used to load tran_low_t into int16, narrowing if necessary.

static INLINE int16x8x2_t load_tran_low_to_s16x2q(const tran_low_t *buf) {
#if CONFIG_VP9_HIGHBITDEPTH
  const int32x4x2_t v0 = vld2q_s32(buf);
  const int32x4x2_t v1 = vld2q_s32(buf + 8);
  const int16x4_t s0 = vmovn_s32(v0.val[0]);
  const int16x4_t s1 = vmovn_s32(v0.val[1]);
  const int16x4_t s2 = vmovn_s32(v1.val[0]);
  const int16x4_t s3 = vmovn_s32(v1.val[1]);
  int16x8x2_t res;
  res.val[0] = vcombine_s16(s0, s2);
  res.val[1] = vcombine_s16(s1, s3);
  return res;
#else
  return vld2q_s16(buf);
#endif
}

static INLINE int16x8_t load_tran_low_to_s16q(const tran_low_t *buf) {
#if CONFIG_VP9_HIGHBITDEPTH
  const int32x4_t v0 = vld1q_s32(buf);
  const int32x4_t v1 = vld1q_s32(buf + 4);
  const int16x4_t s0 = vmovn_s32(v0);
  const int16x4_t s1 = vmovn_s32(v1);
  return vcombine_s16(s0, s1);
#else
  return vld1q_s16(buf);
#endif
}

static INLINE int16x4_t load_tran_low_to_s16d(const tran_low_t *buf) {
#if CONFIG_VP9_HIGHBITDEPTH
  const int32x4_t v0 = vld1q_s32(buf);
  return vmovn_s32(v0);
#else
  return vld1_s16(buf);
#endif
}

static INLINE void store_s16q_to_tran_low(tran_low_t *buf, const int16x8_t a) {
#if CONFIG_VP9_HIGHBITDEPTH
  const int32x4_t v0 = vmovl_s16(vget_low_s16(a));
  const int32x4_t v1 = vmovl_s16(vget_high_s16(a));
  vst1q_s32(buf, v0);
  vst1q_s32(buf + 4, v1);
#else
  vst1q_s16(buf, a);
#endif
}

//------------------------------------------------------------------------------
// Load 4 sets of 4 bytes when alignment is not guaranteed.

// Prevent clang and gcc from adding alignment hints to load instructions:
// https://bugs.llvm.org//show_bug.cgi?id=24421
// The attribute has existed for gcc  for a long time. clang appears to have
// added it in 3.8:
// http://releases.llvm.org/3.8.0/tools/clang/docs/ReleaseNotes.html
#if !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)
typedef uint32_t __attribute__((aligned(1))) uint32_unaligned;
#endif  //  !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)

static INLINE uint8x16_t load_unaligned_u8_to_u8x16q(const uint8_t *a,
                                                     int a_stride) {
  uint32x4_t a_u32 = vdupq_n_u32(0);
#if !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 0);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 1);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 2);
  a += a_stride;
  a_u32 = vld1q_lane_u32((const uint32_unaligned *)a, a_u32, 3);
#else
  uint32_t b[4];
  uint32_t *b_ptr = b;
  memcpy(b_ptr, a, 4);
  b_ptr++;
  a += a_stride;
  memcpy(b_ptr, a, 4);
  b_ptr++;
  a += a_stride;
  memcpy(b_ptr, a, 4);
  b_ptr++;
  a += a_stride;
  memcpy(b_ptr, a, 4);
  b_ptr++;
  a += a_stride;
  a_u32 = vld1q_u32(b);
#endif  //  !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)
  return vreinterpretq_u8_u32(a_u32);
}

// Store 4 sets of 4 bytes when alignment is not guaranteed.
static INLINE void store_unaligned_u8x16_to_u8q(uint8_t *a, int a_stride,
                                                const uint8x16_t b) {
  const uint32x4_t b_u32 = vreinterpretq_u32_u8(b);
#if !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)
  vst1q_lane_u32((uint32_unaligned *)a, b_u32, 0);
  a += a_stride;
  vst1q_lane_u32((uint32_unaligned *)a, b_u32, 1);
  a += a_stride;
  vst1q_lane_u32((uint32_unaligned *)a, b_u32, 2);
  a += a_stride;
  vst1q_lane_u32((uint32_unaligned *)a, b_u32, 3);
#else
  uint32_t c[4];

  vst1q_lane_u32(c, b_u32, 0);
  vst1q_lane_u32(c + 1, b_u32, 1);
  vst1q_lane_u32(c + 2, b_u32, 2);
  vst1q_lane_u32(c + 3, b_u32, 3);

  memcpy(a, c, 4);
  a += a_stride;
  memcpy(a, c + 1, 4);
  a += a_stride;
  memcpy(a, c + 2, 4);
  a += a_stride;
  memcpy(a, c + 3, 4);
#endif  //  !defined(VPX_INCOMPATIBLE_GCC) && !defined(_WIN32)
}

// Load 2 sets of 4 bytes when alignment is guaranteed.
static INLINE uint8x8_t load_u8_to_u8x8(const uint8_t *a, int a_stride) {
  uint32x2_t a_u32 = vdup_n_u32(0);

  assert(!((intptr_t)a % sizeof(uint32_t)));
  assert(!(a_stride % sizeof(uint32_t)));

  a_u32 = vld1_lane_u32((const uint32_t *)a, a_u32, 0);
  a += a_stride;
  a_u32 = vld1_lane_u32((const uint32_t *)a, a_u32, 1);
  return vreinterpret_u8_u32(a_u32);
}

// Store 2 sets of 4 bytes when alignment is guaranteed.
static INLINE void store_u8x8_to_u8(uint8_t *a, int a_stride,
                                    const uint8x8_t b) {
  uint32x2_t b_u32 = vreinterpret_u32_u8(b);

  assert(!((intptr_t)a % sizeof(uint32_t)));
  assert(!(a_stride % sizeof(uint32_t)));

  vst1_lane_u32((uint32_t *)a, b_u32, 0);
  a += a_stride;
  vst1_lane_u32((uint32_t *)a, b_u32, 1);
}

#endif  // VPX_DSP_ARM_MEM_NEON_H_
