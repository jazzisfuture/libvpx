/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 *  This file contains functions for setting up
 *  adaptive segmentation for altref frames
 *
 *  TODO(yuryg): ... detailed description, may be ...
 */

#ifndef VP9_ENCODER_VP9_ALT_REF_AQ_H_
#define VP9_ENCODER_VP9_ALT_REF_AQ_H_

#include "vpx/vpx_integer.h"

#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_matx.h"

#include "vp9/encoder/vp9_alt_ref_aq_common.h"


#ifdef __cplusplus

extern "C" {
#endif

struct VP9_COMP;
typedef struct VP9_COMP VP9_COMP;

typedef struct {
  ALT_REF_SEG_SWAP_STATE state;

  // number of delta-quantizer segments
  int32_t nsegments;

  // this one is just for debugging purposes
  int32_t alt_ref_number;

  // basic aq mode (I keep original
  // aq mode when encoding altref frame)
  AQ_MODE aq_mode;

  // wrapper around basic segmentation_map
  MATX cpi_segmentation_map;

  // altref segmentation
  MATX segmentation_map;

  // distribution of the segment ids
  uint8_t segment_hist[255];
} ALT_REF_AQ;


void vp9_alt_ref_aq_init(ALT_REF_AQ* const self);
void vp9_alt_ref_aq_deinit(ALT_REF_AQ* const self);

// change this->state: BEFORE -> AT -> AFTER -> BEFORE -> ...
void vp9_alt_ref_aq_go_to_next_state(ALT_REF_AQ* const self);

// set basic segmentation to altref one
void vp9_alt_ref_aq_setup(ALT_REF_AQ* const self, VP9_COMP* const cpi);

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset(ALT_REF_AQ* const self, VP9_COMP* const cpi);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_H_
