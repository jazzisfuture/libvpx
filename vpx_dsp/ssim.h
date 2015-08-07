/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_SSIM_H_
#define VPX_DSP_SSIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "./vpx_config.h"
#include "vpx_scale/yv12config.h"

// TODO(aconverse): Unify vp8/vp9_clear_system_state
#if ARCH_X86 || ARCH_X86_64
void vpx_reset_mmx_state(void);
#define vpx_clear_system_state() vpx_reset_mmx_state()
#else
#define vpx_clear_system_state()
#endif

double vpx_calc_ssim(const YV12_BUFFER_CONFIG *source,
                     const YV12_BUFFER_CONFIG *dest,
                     double *weight);

double vpx_calc_ssimg(const YV12_BUFFER_CONFIG *source,
                      const YV12_BUFFER_CONFIG *dest,
                      double *ssim_y, double *ssim_u, double *ssim_v);

#if CONFIG_VP9_HIGHBITDEPTH
double vpx_highbd_calc_ssim(const YV12_BUFFER_CONFIG *source,
                            const YV12_BUFFER_CONFIG *dest,
                            double *weight,
                            unsigned int bd);

double vpx_highbd_calc_ssimg(const YV12_BUFFER_CONFIG *source,
                             const YV12_BUFFER_CONFIG *dest,
                             double *ssim_y,
                             double *ssim_u,
                             double *ssim_v,
                             unsigned int bd);
#endif  // CONFIG_VP9_HIGHBITDEPTH

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_DSP_SSIM_H_
