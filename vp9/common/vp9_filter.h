/*
 *  Copyright (c) 2011 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_FILTER_H_
#define VP9_COMMON_VP9_FILTER_H_

#include "vpx_config.h"
#include "vpx/vpx_integer.h"

// VP9 sub-pixel filters produce output at 1/16th pixel resolution.
#define SUBPEL_BITS 4
#define SUBPEL_MASK ((1 << SUBPEL_BITS) - 1)
#define SUBPEL_SHIFTS (1 << SUBPEL_BITS)
#define SUBPEL_TAPS 8

// Intermediate positional calculations are maintained to 8-bit precision.
#define PRECISION_BITS 8
#define PRECISION_MASK ((1 << PRECISION_BITS) - 1)

extern const int16_t vp9_bilinear_filters[SUBPEL_SHIFTS][SUBPEL_TAPS];
extern const int16_t vp9_sub_pel_filters_6[SUBPEL_SHIFTS][SUBPEL_TAPS];
extern const int16_t vp9_sub_pel_filters_8[SUBPEL_SHIFTS][SUBPEL_TAPS];
extern const int16_t vp9_sub_pel_filters_8s[SUBPEL_SHIFTS][SUBPEL_TAPS];
extern const int16_t vp9_sub_pel_filters_8lp[SUBPEL_SHIFTS][SUBPEL_TAPS];

// The VP9_BILINEAR_FILTERS_2TAP macro returns a pointer to the bilinear
// filter kernel as a 2 tap filter.
#define BILINEAR_FILTERS_2TAP(x) \
  (vp9_bilinear_filters[(x)] + SUBPEL_TAPS/2 - 1)

#endif  // VP9_COMMON_VP9_FILTER_H_
