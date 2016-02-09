/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP10_ENCODER_PICKRST_H_
#define VP10_ENCODER_PICKRST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vp10/encoder/encoder.h"

#define JOINT_FILTER_RESTORATION_SEARCH 1

struct yv12_buffer_config;
struct VP10_COMP;

int vp10_search_restoration_level(const YV12_BUFFER_CONFIG *sd,
                                  VP10_COMP *cpi,
                                  int filter_level, int partial_frame,
                                  double *best_cost_ret);
#if JOINT_FILTER_RESTORATION_SEARCH
int vp10_search_filter_restoration_level(const YV12_BUFFER_CONFIG *sd,
                                         VP10_COMP *cpi,
                                         int partial_frame,
                                         int *restoration_level);
#endif  // JOINT_FILTER_RESTORATION_SEARCH

void vp10_pick_filter_restoration_level(
    const YV12_BUFFER_CONFIG *sd, VP10_COMP *cpi, LPF_PICK_METHOD method);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP10_ENCODER_PICKRST_H_
