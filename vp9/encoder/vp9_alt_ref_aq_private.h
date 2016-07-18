/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

/*
 *  \file vp9_alt_ref_aq_private.h
 *
 *  This file  describes class  used for  setting up  adaptive segmentation
 *  for  altref frames.   It  is  private file  and  most  likely you  need
 *  alt_ref_aq.h instead.
 */

#ifndef VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_
#define VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_

#include "vpx/vpx_integer.h"

#include "vp9/common/vp9_seg_common.h"

#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_matx.h"

#include "vp9/encoder/vp9_encoder.h"

#ifdef __cplusplus

extern "C" {
#endif

struct ALT_REF_AQ {
  // private member used for segment ids convertion
  int __segment_changes[VPXMAX(ALT_REF_MAX_FRAMES, MAX_SEGMENTS)];

  // private member used to compute qdelta steps
  int __nsteps;

  // number of delta-quantizer segments,
  // it can be different from nsteps
  // because of the range compression
  int nsegments;

  // number of segments to assign zero
  // and nonzero delta quantizer
  // (I choose between them
  //  greater number of nonzero segments)
  int NUM_ZERO_SEGMENTS;
  int MIN_NONZERO_SEGMENTS;

  // single qdelta step between segments
  float single_delta;

  // qdelta[i] = (int) single_delta*segment_deltas[i];
  // ------------------------------------------------
  // qdelta[0] has single delta quantizer
  // for the frame in the case we need it
  int segment_deltas[VPXMAX(ALT_REF_MAX_FRAMES, MAX_SEGMENTS)];

  // this one is just for debugging purposes
  int alt_ref_number;

  // basic aq mode (I keep original
  // aq mode when encoding altref frame)
  AQ_MODE aq_mode;

  // wrapper around basic segmentation_map
  struct MATX_8U cpi_segmentation_map;

  // altref segmentation
  struct MATX_8U segmentation_map;

  // distribution of the segment ids
  int segment_hist[VPXMAX(ALT_REF_MAX_FRAMES, MAX_SEGMENTS)];
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_
