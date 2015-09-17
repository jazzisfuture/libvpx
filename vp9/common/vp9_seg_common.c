/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_loopfilter.h"
#include "vp9/common/vp9_seg_common.h"
#include "vp9/common/vp9_quant_common.h"

const uint8_t vp9_seg_feature_data_signed[SEG_LVL_MAX] = { 1, 1, 0, 0 };

const uint8_t vp9_seg_feature_data_max[SEG_LVL_MAX] = {
  MAXQ, MAX_LOOP_FILTER, 3, 0 };

const vpx_tree_index vp9_segment_tree[TREE_SIZE(MAX_SEGMENTS)] = {
  2,  4,  6,  8, 10, 12,
  0, -1, -2, -3, -4, -5, -6, -7
};

// TBD? Functions to read and write segment data with range / validity checking
