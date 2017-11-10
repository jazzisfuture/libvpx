/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "./vpx_config.h"

#include "vpx_ports/mem.h"

typedef uint32_t (*high_variance_fn_t)(const uint16_t *src, int src_stride,
                                       const uint16_t *ref, int ref_stride,
                                       uint32_t *sse, int *sum);

uint32_t vpx_highbd_calc8x8var_sse2(const uint16_t *src, int src_stride,
                                    const uint16_t *ref, int ref_stride,
                                    uint32_t *sse, int *sum);

uint32_t vpx_highbd_calc16x16var_sse2(const uint16_t *src, int src_stride,
                                      const uint16_t *ref, int ref_stride,
                                      uint32_t *sse, int *sum);

static void highbd_8_variance_sse2(const uint16_t *src, int src_stride,
                                   const uint16_t *ref, int ref_stride, int w,
                                   int h, uint32_t *sse, int *sum,
                                   high_variance_fn_t var_fn, int block_size) {
  int i, j;

  *sse = 0;
  *sum = 0;

  for (i = 0; i < h; i += block_size) {
    for (j = 0; j < w; j += block_size) {
      unsigned int sse0;
      int sum0;
      var_fn(src + src_stride * i + j, src_stride, ref + ref_stride * i + j,
             ref_stride, &sse0, &sum0);
      *sse += sse0;
      *sum += sum0;
    }
  }
}

static void highbd_10_variance_sse2(const uint16_t *src, int src_stride,
                                    const uint16_t *ref, int ref_stride, int w,
                                    int h, uint32_t *sse, int *sum,
                                    high_variance_fn_t var_fn, int block_size) {
  int i, j;
  uint64_t sse_long = 0;
  int32_t sum_long = 0;

  for (i = 0; i < h; i += block_size) {
    for (j = 0; j < w; j += block_size) {
      unsigned int sse0;
      int sum0;
      var_fn(src + src_stride * i + j, src_stride, ref + ref_stride * i + j,
             ref_stride, &sse0, &sum0);
      sse_long += sse0;
      sum_long += sum0;
    }
  }
  *sum = ROUND_POWER_OF_TWO(sum_long, 2);
  *sse = (uint32_t)ROUND_POWER_OF_TWO(sse_long, 4);
}

static void highbd_12_variance_sse2(const uint16_t *src, int src_stride,
                                    const uint16_t *ref, int ref_stride, int w,
                                    int h, uint32_t *sse, int *sum,
                                    high_variance_fn_t var_fn, int block_size) {
  int i, j;
  uint64_t sse_long = 0;
  int32_t sum_long = 0;

  for (i = 0; i < h; i += block_size) {
    for (j = 0; j < w; j += block_size) {
      unsigned int sse0;
      int sum0;
      var_fn(src + src_stride * i + j, src_stride, ref + ref_stride * i + j,
             ref_stride, &sse0, &sum0);
      sse_long += sse0;
      sum_long += sum0;
    }
  }
  *sum = ROUND_POWER_OF_TWO(sum_long, 4);
  *sse = (uint32_t)ROUND_POWER_OF_TWO(sse_long, 8);
}

#define HIGH_GET_VAR(S)                                                       \
  void vpx_highbd_get##S##x##S##var_sse2(const uint8_t *src8, int src_stride, \
                                         const uint8_t *ref8, int ref_stride, \
                                         uint32_t *sse, int *sum) {           \
    uint16_t *src = CONVERT_TO_SHORTPTR(src8);                                \
    uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);                                \
    vpx_highbd_calc##S##x##S##var_sse2(src, src_stride, ref, ref_stride, sse, \
                                       sum);                                  \
  }                                                                           \
                                                                              \
  void vpx_highbd_10_get##S##x##S##var_sse2(                                  \
      const uint8_t *src8, int src_stride, const uint8_t *ref8,               \
      int ref_stride, uint32_t *sse, int *sum) {                              \
    uint16_t *src = CONVERT_TO_SHORTPTR(src8);                                \
    uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);                                \
    vpx_highbd_calc##S##x##S##var_sse2(src, src_stride, ref, ref_stride, sse, \
                                       sum);                                  \
    *sum = ROUND_POWER_OF_TWO(*sum, 2);                                       \
    *sse = ROUND_POWER_OF_TWO(*sse, 4);                                       \
  }                                                                           \
                                                                              \
  void vpx_highbd_12_get##S##x##S##var_sse2(                                  \
      const uint8_t *src8, int src_stride, const uint8_t *ref8,               \
      int ref_stride, uint32_t *sse, int *sum) {                              \
    uint16_t *src = CONVERT_TO_SHORTPTR(src8);                                \
    uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);                                \
    vpx_highbd_calc##S##x##S##var_sse2(src, src_stride, ref, ref_stride, sse, \
                                       sum);                                  \
    *sum = ROUND_POWER_OF_TWO(*sum, 4);                                       \
    *sse = ROUND_POWER_OF_TWO(*sse, 8);                                       \
  }

HIGH_GET_VAR(16);
HIGH_GET_VAR(8);

#undef HIGH_GET_VAR

unsigned int vpx_highbd_8_mse16x16_sse2(const uint8_t *src8, int src_stride,
                                        const uint8_t *ref8, int ref_stride,
                                        unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_8_variance_sse2(src, src_stride, ref, ref_stride, 16, 16, sse, &sum,
                         vpx_highbd_calc16x16var_sse2, 16);
  return *sse;
}

unsigned int vpx_highbd_10_mse16x16_sse2(const uint8_t *src8, int src_stride,
                                         const uint8_t *ref8, int ref_stride,
                                         unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_10_variance_sse2(src, src_stride, ref, ref_stride, 16, 16, sse, &sum,
                          vpx_highbd_calc16x16var_sse2, 16);
  return *sse;
}

unsigned int vpx_highbd_12_mse16x16_sse2(const uint8_t *src8, int src_stride,
                                         const uint8_t *ref8, int ref_stride,
                                         unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_12_variance_sse2(src, src_stride, ref, ref_stride, 16, 16, sse, &sum,
                          vpx_highbd_calc16x16var_sse2, 16);
  return *sse;
}

unsigned int vpx_highbd_8_mse8x8_sse2(const uint8_t *src8, int src_stride,
                                      const uint8_t *ref8, int ref_stride,
                                      unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_8_variance_sse2(src, src_stride, ref, ref_stride, 8, 8, sse, &sum,
                         vpx_highbd_calc8x8var_sse2, 8);
  return *sse;
}

unsigned int vpx_highbd_10_mse8x8_sse2(const uint8_t *src8, int src_stride,
                                       const uint8_t *ref8, int ref_stride,
                                       unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_10_variance_sse2(src, src_stride, ref, ref_stride, 8, 8, sse, &sum,
                          vpx_highbd_calc8x8var_sse2, 8);
  return *sse;
}

unsigned int vpx_highbd_12_mse8x8_sse2(const uint8_t *src8, int src_stride,
                                       const uint8_t *ref8, int ref_stride,
                                       unsigned int *sse) {
  int sum;
  uint16_t *src = CONVERT_TO_SHORTPTR(src8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  highbd_12_variance_sse2(src, src_stride, ref, ref_stride, 8, 8, sse, &sum,
                          vpx_highbd_calc8x8var_sse2, 8);
  return *sse;
}
