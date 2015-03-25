/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/encoder/vp9_block.h"

#ifndef VP9_ENCODER_VP9_AQ_ARF_H_
#define VP9_ENCODER_VP9_AQ_ARf_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VP9_COMP;
struct macroblock;

#define DEFAULT_AQ_ARF_SEG     0         // Neutral Q segment index
#define HIGHQ_AQ_ARF_SEG       1         // Raised Q segment

// This function sets up a set of segments with delta Q values around
// the baseline frame quantizer for ARF AQ.
void vp9_setup_arf_aq(struct VP9_COMP *cpi);
void vp9_arf_aq_select_segment(struct VP9_COMP *cpi, int mb_row, int mb_col,
                               unsigned int weight_value);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_AQ_ARf_H_
