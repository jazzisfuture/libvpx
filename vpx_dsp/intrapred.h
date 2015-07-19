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
#include "vpx_mem/vpx_mem.h"

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

// This serves as a wrapper function, so that all the prediction functions
// can be unified and accessed as a pointer array. Note that the boundary
// above and left are not necessarily used all the time.
#define declare_intra_pred_sized(type, size) \
  void vp9_##type##_predictor_##size##x##size##_c(uint8_t *dst, \
                                                  ptrdiff_t stride, \
                                                  const uint8_t *above, \
                                                  const uint8_t *left);

#if CONFIG_VP9_HIGHBITDEPTH
#define declare_intra_pred_highbd_sized(type, size) \
  void vp9_highbd_##type##_predictor_##size##x##size##_c( \
      uint16_t *dst, ptrdiff_t stride, const uint16_t *above, \
      const uint16_t *left, int bd);

#define declare_intra_pred_allsizes(type) \
  declare_intra_pred_sized(type, 4) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32) \
  declare_intra_pred_highbd_sized(type, 4) \
  declare_intra_pred_highbd_sized(type, 8) \
  declare_intra_pred_highbd_sized(type, 16) \
  declare_intra_pred_highbd_sized(type, 32)

#define declare_intra_pred_no_4x4(type) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32) \
  declare_intra_pred_highbd_sized(type, 4) \
  declare_intra_pred_highbd_sized(type, 8) \
  declare_intra_pred_highbd_sized(type, 16) \
  declare_intra_pred_highbd_sized(type, 32)

#else
#define declare_intra_pred_allsizes(type) \
  declare_intra_pred_sized(type, 4) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32)

#define declare_intra_pred_no_4x4(type) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32)
#endif  // CONFIG_VP9_HIGHBITDEPTH

declare_intra_pred_no_4x4(d207)
declare_intra_pred_no_4x4(d63)
declare_intra_pred_no_4x4(d45)
declare_intra_pred_no_4x4(d117)
declare_intra_pred_no_4x4(d135)
declare_intra_pred_no_4x4(d153)
declare_intra_pred_allsizes(v)
declare_intra_pred_allsizes(h)
declare_intra_pred_allsizes(tm)
declare_intra_pred_allsizes(dc_128)
declare_intra_pred_allsizes(dc_left)
declare_intra_pred_allsizes(dc_top)
declare_intra_pred_allsizes(dc)
#undef declare_intra_pred_allsizes

#endif  // VPX_DSP_INTRAPRED_H_
