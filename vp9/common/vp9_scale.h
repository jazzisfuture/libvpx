/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_SCALE_H_
#define VP9_COMMON_VP9_SCALE_H_

#include "vp9/common/vp9_mv.h"
#include "vp9/common/vp9_convolve.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REF_SCALE_SHIFT 14
#define REF_NO_SCALE (1 << REF_SCALE_SHIFT)
#define REF_INVALID_SCALE -1

struct scale_factors {
  int x_scale_fp;   // horizontal fixed point scale factor
  int y_scale_fp;   // vertical fixed point scale factor
  int x_step_q4;
  int y_step_q4;

  int (*scale_value_x)(int val, const struct scale_factors *sf);
  int (*scale_value_y)(int val, const struct scale_factors *sf);

  convolve_fn_t predict[2][2][2];  // horiz, vert, avg
#if CONFIG_VP9_HIGH
  high_convolve_fn_t high_predict[2][2][2];  // horiz, vert, avg
#endif
};

MV32 vp9_scale_mv(const MV *mv, int x, int y, const struct scale_factors *sf);

#if CONFIG_VP9_HIGH
void vp9_setup_scale_factors_for_frame(
    struct scale_factors *sf, int other_w, int other_h, int this_w, int this_h,
    int use_high);
#else
void vp9_setup_scale_factors_for_frame(
    struct scale_factors *sf, int other_w, int other_h, int this_w, int this_h);
#endif

static INLINE int vp9_is_valid_scale(const struct scale_factors *sf) {
  return sf->x_scale_fp != REF_INVALID_SCALE &&
         sf->y_scale_fp != REF_INVALID_SCALE;
}

static INLINE int vp9_is_scaled(const struct scale_factors *sf) {
  return sf->x_scale_fp != REF_NO_SCALE ||
         sf->y_scale_fp != REF_NO_SCALE;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_SCALE_H_
