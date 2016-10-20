/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_PPC_BITDEPTH_CONVERSION_VSX_H_
#define VPX_DSP_PPC_BITDEPTH_CONVERSION_VSX_H_

#include <altivec.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/vpx_dsp_common.h"

// Load 8 16 bit values. If the source is 32 bits then pack down with
// saturation.
static INLINE vector signed short load_tran_low(int c, const tran_low_t *s) {
#if CONFIG_VP9_HIGHBITDEPTH
  vector signed int u = vec_vsx_ld(c, s);
  vector signed int v = vec_vsx_ld(c, s + 4);
  return vec_packs(u, v);
#else
  return vec_vsx_ld(c, s);
#endif
}

// Store 8 16 bit values. If the destination is 32 bits then sign extend the
// values by multiplying by 1.
static INLINE void store_tran_low(vector signed short v, int c, tran_low_t *s) {
#if CONFIG_VP9_HIGHBITDEPTH
  const vector signed short one = vec_splat_s16(1);
  const vector signed int even = vec_mule(v, one);
  const vector signed int odd = vec_mulo(v, one);
  const vector signed int high = vec_mergeh(even, odd);
  const vector signed int low = vec_mergel(even, odd);
  vec_vsx_st(high, c, s);
  vec_vsx_st(low, c, s + 4);
#else
  vec_vsx_st(v, c, s);
#endif
}

#endif  // VPX_DSP_PPC_BITDEPTH_CONVERSION_VSX_H_
