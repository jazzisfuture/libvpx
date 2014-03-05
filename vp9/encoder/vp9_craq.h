/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_ENCODER_VP9_CRAQ_H_
#define VP9_ENCODER_VP9_CRAQ_H_

#include "vp9/encoder/vp9_onyx_int.h"

#ifdef __cplusplus
extern "C" {
#endif

static int compute_qdelta_by_rate(VP9_COMP *const cpi, int base_q_index,
                                  double rate_target_ratio);

int vp9_candidate_refresh_aq(VP9_COMP *const cpi,
                             MODE_INFO *const mi,
                             int bsize);

static void update_map(VP9_COMP *const cpi,
                       MODE_INFO *const mi,
                       int xmis,
                       int ymis,
                       int mi_row,
                       int mi_col,
                       int block_index,
                       int bsize);

void vp9_postencode_cyclic_refresh_map(VP9_COMP *const cpi);

void vp9_setup_cyclic_background_refresh(VP9_COMP *const cpi);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_CRAQ_H_
