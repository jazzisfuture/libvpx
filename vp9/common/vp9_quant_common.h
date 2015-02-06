/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_QUANT_COMMON_H_
#define VP9_COMMON_VP9_QUANT_COMMON_H_

#include <math.h>
#include <stdio.h>

#include "vpx/vpx_codec.h"
#include "vp9/common/vp9_seg_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MINQ 0
#define MAXQ 255
#define QINDEX_RANGE (MAXQ - MINQ + 1)
#define QINDEX_BITS 8
#if CONFIG_TX_SKIP
#define TX_SKIP_Q_THRESH_INTER 64
#define TX_SKIP_Q_THRESH_INTRA 255
#define TX_SKIP_SHIFT_THRESH 0
#endif  // CONFIG_TX_SKIP

int16_t vp9_dc_quant(int qindex, int delta, vpx_bit_depth_t bit_depth);
int16_t vp9_ac_quant(int qindex, int delta, vpx_bit_depth_t bit_depth);

int vp9_get_qindex(const struct segmentation *seg, int segment_id,
                   int base_qindex);

static INLINE int16_t vp9_round_factor_to_round(int16_t quant,
                                                int16_t round_factor) {
  return (round_factor * quant) >> 7;
}

#if CONFIG_NEW_QUANT
// The 3 values below are in the 0 - 64 range
// where 64 = uniform quantization,
// and 0 = deadzone where the zero bin width is
// twice that of the other bins

#define Q_ROUNDING_FACTOR_Q0 64
#define Q_ROUNDING_FACTOR_DC 50
#define Q_ROUNDING_FACTOR_AC 46

// The functions below determine the adjustmemt factor for the dequantized
// coefficient lower than the mid-point ofthe quantization bin.
// The normative reconstruction adjustment is returned by the function
// dequant_off_adj_factor() as y, where the actual adjustment is y/128
// of the quantization step size lower than the mid-point of the bin.

static INLINE double quant_to_z(int16_t quant, int isac) {
  static const double qmax = 1024;
  static const double qmin = 24;
  static const double zmax_dc = 2.2;
  static const double zmin_dc = 1.2;
  static const double zmax_ac = 2.5;
  static const double zmin_ac = 1.5;
  double z;
  if (isac) {
    if (quant < qmin) z = zmin_ac;
    else if (quant > qmax) z = zmax_ac;
    else z = zmin_ac + (zmax_ac - zmin_ac) * (quant - qmin) / (qmax - qmin);
  } else {
    if (quant < qmin) z = zmin_dc;
    else if (quant > qmax) z = zmax_dc;
    else z = zmin_dc + (zmax_dc - zmin_dc) * (quant - qmin) / (qmax - qmin);
  }
  return z;
}

static INLINE int16_t z_to_doff_int(double z) {
  double doff = 0.5 / tanh(0.5 * z) - 1.0 / z;
  int16_t doff_int = (int16_t)(doff * 128 + 0.5);
  if (doff_int > 64) doff_int = 64;
  else if (doff_int < 0) doff_int = 0;
  // printf("z = %f, doff_int = %d\n", z, doff_int);
  return doff_int;
}

static INLINE int16_t dequant_off_adj_factor(int16_t quant, int isac) {
  static const int16_t qmax = 1024;
  static const int16_t qmin = 24;
  static const int16_t dmax_dc = 26;
  static const int16_t dmin_dc = 12;
  static const int16_t dmax_ac = 32;
  static const int16_t dmin_ac = 15;
  static const int16_t qdiff = qmax - qmin;
  int16_t d;
  if (isac) {
    if (quant < qmin) d = dmin_ac;
    else if (quant > qmax) d = dmax_ac;
    else d = dmin_ac +
      ((dmax_ac - dmin_ac) * (quant - qmin) + qdiff / 2) / qdiff;
  } else {
    if (quant < qmin) d = dmin_dc;
    else if (quant > qmax) d = dmax_dc;
    else d = dmin_dc +
      ((dmax_dc - dmin_dc) * (quant - qmin) + qdiff / 2) / qdiff;
  }
  if (d > 64) d = 64;
  else if (d < 0) d = 0;
  return d;
}

static INLINE int16_t dequant_off_adj_factor2(int16_t quant, int isac) {
  int16_t d;
  d = z_to_doff_int(quant_to_z(quant, isac));
  return d;
}

static INLINE int16_t dequant_off_adj_factor3(int16_t quant, int isac) {
  int16_t d;
  static const int16_t DQ_OFF_ADJ_FACTOR_DC = 14;
  static const int16_t DQ_OFF_ADJ_FACTOR_AC = 18;
  (void) quant;
  if (!isac)
    d = DQ_OFF_ADJ_FACTOR_DC;
  else
    d = DQ_OFF_ADJ_FACTOR_AC;
  return d;
}

static INLINE int16_t vp9_get_rounding_factor(int isac, int islossy) {
  if (!islossy)
    return Q_ROUNDING_FACTOR_Q0;
  else if (!isac)
    return Q_ROUNDING_FACTOR_DC;
  else
    return Q_ROUNDING_FACTOR_AC;
}

static INLINE int16_t vp9_get_dequant_off(int16_t quant,
                                          int isac, int islossy) {
  const int16_t round_factor = vp9_get_rounding_factor(isac, islossy);
  const int16_t round = vp9_round_factor_to_round(quant, round_factor);
  const int16_t dequant_off_adj =
      islossy ? dequant_off_adj_factor(quant, isac) : 0;
  return (((64 - dequant_off_adj) * quant - 128 * round) >> 7);
}

#endif  // CONFIG_NEW_QUANT

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_QUANT_COMMON_H_
