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

// maximum number of frames
// to be blurred into superframe
#define MAX_WEIGHTS VPXMAX(MAX_SEGMENTS, 255)


#ifdef __cplusplus

extern "C" {
#endif

struct ALT_REF_AQ {
  // max number of non-zero weights
  // used while blurring lookahead buffers
  int nweights;

  // zero segment id, i.e. 1
  int zero_level;

  // number of delta-quantizer segments,
  // it can be different from nweights
  int nsegments;

  // number of segments to assign zero
  // and nonzero delta quantizer
  // (I choose between them
  //  greater number of nonzero segments)
  int num_zero_segments;
  int min_nonzero_segments;

  // single qdelta step between segments
  float single_delta;

  // qdelta[i] = (int) single_delta*segment_deltas[i];
  uint8_t segment_deltas[MAX_WEIGHTS];

  // this one is just for debugging purposes
  int alt_ref_number;

  // basic aq mode (I keep original
  // aq mode when encoding altref frame)
  AQ_MODE aq_mode;

  // wrapper around basic segmentation_map
  struct MATX cpi_segmentation_map;

  // altref segmentation
  struct MATX segmentation_map;

  // distribution of the segment ids
  uint8_t segment_hist[MAX_WEIGHTS];
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_ALT_REF_AQ_PRIVATE_H_
