/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits.h>
#include <math.h>

#include "vp9/encoder/vp9_aq_arf.h"
#include "vp9/encoder/vp9_encodeframe.h"
#include "vp9/common/vp9_seg_common.h"
#include "vp9/encoder/vp9_segmentation.h"

#define AQ_ARF_LOW_RATE_CUTTOF 256
#define DEFAULT_AQ_ARF_SEG     0         // Neutral Q segment index
#define HIGHQ_AQ_ARF_SEG       1         // Raised Q segment
#define AQ_ARF_SEGMENTS        2
#define LOW_ARF_WEIGHT_THRESH  8

static const double aq_arf_q_adj_factor[AQ_ARF_SEGMENTS] =
  {1.00, 0.75};

// Set up the segmentation structures and delta Q values.
void vp9_setup_arf_aq(VP9_COMP *cpi) {
  VP9_COMMON *const cm = &cpi->common;
  struct segmentation *const seg = &cm->seg;

  // Make SURE use of floating point in this function is safe.
  vp9_clear_system_state();


  // Segmentation is just for the ARF itself
  if (cpi->refresh_alt_ref_frame && !cpi->rc.is_src_frame_alt_ref) {
    int segment;

    // const int aq_strength =
    //   get_aq_c_strength(cm->base_qindex, cm->bit_depth);

    // Clear down the segment map.
    //vpx_memset(cpi->segmentation_map, DEFAULT_AQ_ARF_SEG,
    //           cm->mi_rows * cm->mi_cols);

    //vp9_clearall_segfeatures(seg);

    // Segmentation only makes sense if the target bits per SB is above a
    // threshold. Below this the overheads will usually outweigh any benefit.
    /* if (cpi->rc.sb64_target_rate < AQ_ARF_LOW_RATE_CUTTOF) {
      vp9_disable_segmentation(seg);
      return;
    } */

    vp9_clearall_segfeatures(seg);
    vp9_enable_segmentation(seg);

    // Select delta coding method.
    seg->abs_delta = SEGMENT_DELTADATA;

    // Use some of the segments for in frame Q adjustment.
    for (segment = 0; segment < AQ_ARF_SEGMENTS; ++segment) {
      int qindex_delta;

      // For the default segment disable the Q deltas.
      if (segment == DEFAULT_AQ_ARF_SEG) {
        vp9_disable_segfeature(seg, segment, SEG_LVL_ALT_Q);
      } else {
        qindex_delta =
          vp9_compute_qdelta_by_rate(&cpi->rc, cm->frame_type, cm->base_qindex,
                                     aq_arf_q_adj_factor[segment],
                                     cm->bit_depth);

        // Do not allow Q0 in a segment if the base Q is not 0.
        /*if ((cm->base_qindex != 0) &&
              ((cm->base_qindex + qindex_delta) == 0)) {
          qindex_delta = -cm->base_qindex + 1;
        }*/
        if ((cm->base_qindex + qindex_delta) > 0) {
          vp9_enable_segfeature(seg, segment, SEG_LVL_ALT_Q);
          vp9_set_segdata(seg, segment, SEG_LVL_ALT_Q, qindex_delta);
        } else {
          vp9_disable_segfeature(seg, segment, SEG_LVL_ALT_Q);
        }
      }
    }
  } else {
    // Clear down the segment map and disable segmentation.
    vpx_memset(cpi->segmentation_map, DEFAULT_AQ_ARF_SEG,
               cm->mi_rows * cm->mi_cols);
    vp9_clearall_segfeatures(seg);
    vp9_disable_segmentation(seg);
  }
}

// stub for now
// Select and set a segment for the current block
void vp9_arf_aq_select_segment(VP9_COMP *cpi, int mb_row, int mb_col,
                               unsigned int weight_value) {
  VP9_COMMON *const cm = &cpi->common;
  const int mi_row = mb_row * 2;
  const int mi_col = mb_col * 2;
  const int mi_offset = mi_row * cm->mi_cols + mi_col;
  const BLOCK_SIZE bs = BLOCK_16X16;
  const int xmis = MIN(cm->mi_cols - mi_col, num_8x8_blocks_wide_lookup[bs]);
  const int ymis = MIN(cm->mi_rows - mi_row, num_8x8_blocks_high_lookup[bs]);
  unsigned char segment;
  int x, y;

  // Note that there is always a minimum wieght of 2 coming from the centre
  // frame of the arf filter group.
  if (weight_value > LOW_ARF_WEIGHT_THRESH) {
    segment = DEFAULT_AQ_ARF_SEG;
  } else {
    segment = HIGHQ_AQ_ARF_SEG;
  }

  // Fill in the entires in the segment map corresponding to this macroblock.
  for (y = 0; y < ymis; y++) {
    for (x = 0; x < xmis; x++) {
      cpi->segmentation_map[mi_offset + y * cm->mi_cols + x] = segment;
    }
  }
}