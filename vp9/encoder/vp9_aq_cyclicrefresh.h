/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_
#define VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_

#include "vp9/common/vp9_blockd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  // Target percentage of blocks per frame that are cyclicly refreshed.
  int max_mbs_perframe;
  // Maximum q-delta as percentage of base q.
  int max_qdelta_perc;
  // Block size below which we don't apply cyclic refresh.
  BLOCK_SIZE min_block_size;
  // Macroblock starting index (unit of 8x8) for cycling through the frame.
  int mb_index;
  // Controls how long a block will need to wait to be refreshed again.
  int time_for_refresh;
  // Actual number of blocks that were applied delta-q (segment 1).
  int num_seg_blocks;
  // Actual encoding bits for segment 1.
  int actual_seg_bits;
  // RD mult. parameters for segment 1.
  int rdmult;
  // Cyclic refresh map.
  signed char *map;
  // Projected rate and distortion for the current superblock.
  int64_t projected_rate_sb;
  int64_t projected_dist_sb;
  // Thresholds applied to projected rate/distortion of the superblock.
  int64_t thresh_rate_sb;
  int64_t thresh_dist_sb;
  // Rate target ratio to set q delta.
  float rate_ratio_qdelta;
} CYCLIC_REFRESH;

struct VP9_COMP;

// For cyclic refresh, update rate correction factor in the rate control
// structure, taking into account the segment delta-q.
int vp9_estimate_bits_at_q_cyclicrefresh(const struct VP9_COMP *cpi,
                                         int frame_kind,
                                         int q,
                                         int mbs,
                                         double correction_factor);

// For cyclic refresh, estimates q and delta-q (for segment 1) to achieve a
// target bits per frame.
int vp9_regulate_q_cyclic_refresh(const struct VP9_COMP *cpi,
                                  int target_bits_per_mb,
                                  int active_best_quality,
                                  int active_worst_quality,
                                  double correction_factor);

// Prior to coding a given prediction block, of size bsize at (mi_row, mi_col),
// check if we should reset the segment_id, and update the cyclic_refresh map
// and segmentation map.
void vp9_update_segment_aq(struct VP9_COMP *const cpi,
                           MODE_INFO *const mi,
                           int mi_row,
                           int mi_col,
                           int bsize,
                           int use_rd);

// Setup cyclic background refresh: set delta q and segmentation map.
void vp9_setup_cyclic_refresh_aq(struct VP9_COMP *const cpi);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_ENCODER_VP9_AQ_CYCLICREFRESH_H_
