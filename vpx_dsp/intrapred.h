/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_INTRAPRED_H_
#define VPX_DSP_INTRAPRED_H_

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"

#include "vpx_ports/mem.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

INLINE void d207_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d63_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left);
INLINE void d45_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left);
INLINE void d117_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d135_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d153_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void v_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left);
INLINE void h_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left);
INLINE void tm_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left);
INLINE void dc_128_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left);
INLINE void dc_left_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                              const uint8_t *above,
                              const uint8_t *left);
INLINE void dc_top_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left);
INLINE void dc_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left);

void vp9_d207_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d63_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                             const uint8_t *above, const uint8_t *left);
void vp9_d45_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                             const uint8_t *above, const uint8_t *left);
void vp9_d117_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d135_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d153_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);

#if CONFIG_VP9_HIGHBITDEPTH
INLINE void highbd_d207_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d63_predictor(uint16_t *dst, ptrdiff_t stride,
                                 int bs, const uint16_t *above,
                                 const uint16_t *left, int bd);
INLINE void highbd_d45_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                 const uint16_t *above,
                                 const uint16_t *left, int bd);
INLINE void highbd_d117_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d135_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d153_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_v_predictor(uint16_t *dst, ptrdiff_t stride,
                               int bs, const uint16_t *above,
                               const uint16_t *left, int bd);
INLINE void highbd_h_predictor(uint16_t *dst, ptrdiff_t stride,
                               int bs, const uint16_t *above,
                               const uint16_t *left, int bd);
INLINE void highbd_tm_predictor(uint16_t *dst, ptrdiff_t stride,
                                int bs, const uint16_t *above,
                                const uint16_t *left, int bd);
INLINE void highbd_dc_128_predictor(uint16_t *dst, ptrdiff_t stride,
                                    int bs, const uint16_t *above,
                                    const uint16_t *left, int bd);
INLINE void highbd_dc_left_predictor(uint16_t *dst, ptrdiff_t stride,
                                     int bs, const uint16_t *above,
                                     const uint16_t *left, int bd);
INLINE void highbd_dc_top_predictor(uint16_t *dst, ptrdiff_t stride,
                                    int bs, const uint16_t *above,
                                    const uint16_t *left, int bd);
INLINE void highbd_dc_predictor(uint16_t *dst, ptrdiff_t stride,
                                int bs, const uint16_t *above,
                                const uint16_t *left, int bd);
#endif  // CONFIG_VP9_HIGHBITDEPTH
#endif  // VPX_DSP_INTRAPRED_H_
