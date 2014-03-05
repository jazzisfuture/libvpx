/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>

#include "vp9/encoder/vp9_craq.h"

#include "vp9/common/vp9_seg_common.h"

#include "vp9/encoder/vp9_ratectrl.h"
#include "vp9/encoder/vp9_rdopt.h"
#include "vp9/encoder/vp9_segmentation.h"
#include "vp9/common/vp9_systemdependent.h"

// Computes a q delta (in "q index" terms) to get from a starting q value
// to a value that should equate to the given rate ratio.
static int compute_qdelta_by_rate(VP9_COMP *cpi, int base_q_index,
                                  double rate_target_ratio) {
  int i;
  int target_index = cpi->rc.worst_quality;

  // Look up the current projected bits per block for the base index
  const int base_bits_per_mb = vp9_rc_bits_per_mb(cpi->common.frame_type,
                                            base_q_index, 1.0);

  // Find the target bits per mb based on the base value and given ratio.
  const int target_bits_per_mb = (int)(rate_target_ratio * base_bits_per_mb);

  // Convert the q target to an index
  for (i = cpi->rc.best_quality; i < cpi->rc.worst_quality; ++i) {
    target_index = i;
    if (vp9_rc_bits_per_mb(cpi->common.frame_type, i, 1.0) <=
            target_bits_per_mb )
      break;
  }

  return target_index - base_q_index;
}

// Check if we should change the refresh map. For now, we consider refresh
// for zero-mv encoded blocks.
static void update_refresh_map(VP9_COMP *const cpi,
                               const MODE_INFO *mi,
                               int xmis,
                               int ymis,
                               int mi_row,
                               int mi_col,
                               int block_index) {
  VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = &cpi->cyclic_refresh;
  int x = 0;
  int y = 0;
  int new_map_value = 0;
  xmis = MIN(cm->mi_cols - mi_col, xmis);
  ymis = MIN(cm->mi_rows - mi_row, ymis);
  // Copy mbmi->segment_id from last encoding back into global segmentation
  // (since this segment_id may have been reset during the encoding).
  cpi->segmentation_map[block_index] = mi->mbmi.segment_id;
  // Update the cyclic refresh map, to be used for setting segmentation map
  // for the next frame. If the block was refreshed in this frame, mark it
  // as clean. The magnitude of the -ve influences how long before we
  // consider it for refresh again.
  if (cpi->segmentation_map[block_index] == 1) {
    new_map_value = -cr->time_for_refresh;
  } else if ((mi->mbmi.mode == ZEROMV ||
      (mi->mbmi.mode == NEARESTMV && mi->mbmi.mv[0].as_int == 0)) &&
      (mi->mbmi.ref_frame[0] == LAST_FRAME)) {
    // Else if it was coded as zero-mv last, and has not already been
    // refreshed (marked as 1), then mark it as a candidate for cleanup
    // for future time (marked as 0).
    if (cpi->cyclic_refresh.map[block_index] == 1)
      new_map_value = 0;
  } else {
    // Leave it marked as block that is not a candidate for refresh.
    new_map_value = 1;
  }
  // Update entries in the cyclic refresh map with new_map_value.
  for (x = 0; x < xmis; x++)
    for (y = 0; y < ymis; y++) {
      cpi->cyclic_refresh.map[block_index + y * cm->mi_cols + x] =
          new_map_value;
    }
  // Keep track of actual number of (8x8) blocks in segment from previous frame.
  if (mi->mbmi.segment_id)
    cr->num_seg_blocks += xmis * ymis;
}

// We have encoded the frame, so update the cyclic refresh map. Update is based
// on the encoding mode, and possibly the overall segment rate-over/undershoot.
void vp9_postencode_cyclic_refresh_map(VP9_COMP *const cpi) {
  VP9_COMMON *const cm = &cpi->common;
  const int mi_stride = cm->mode_info_stride;
  int mi_row, mi_col, bl2_index, bl_index_16, idx, idx2;
  // Offsets for the next mi in the 64x64 block, for splits to 32 and 16.
  // The last step brings us back to the starting position.
  const int mi_offset_32[] = {4, (mi_stride << 2) - 4, 4,
      -(mi_stride << 2) - 4};
  const int offset_16[] = {2, (mi_stride << 1) - 2, 2,
      -(mi_stride << 1) - 2};
  // Offsets for block index, for splits to 32 and 16.
  const int bl_offset_32[] = {0 , 4, (cm->mi_cols << 2) - 4, 4,
      -(mi_stride << 2) - 4};
  const int bl_offset_16[] = {0 , 2, (cm->mi_cols << 1) - 2, 2,
      -(mi_stride << 1) - 2};

  // Loop through all sb blocks, and update cyclic map. Only consider block
  // sizes above a min_block_size, e.g., ~16x16, as larger blocks should
  // be more likely to represent stable background.
  for (mi_row = 0; mi_row <  cm->mi_rows; mi_row += MI_BLOCK_SIZE)
    for (mi_col = 0; mi_col < cm->mi_cols; mi_col += MI_BLOCK_SIZE) {
      MODE_INFO *mip =
          cm->mi_grid_visible[mi_row * cm->mode_info_stride + mi_col];
      MODE_INFO *mip2 = mip;
      int bl_index = mi_row * cm->mi_cols + mi_col;
      const int max_rows = (mi_row + MI_BLOCK_SIZE > cm->mi_rows ?
          cm->mi_rows - mi_row : MI_BLOCK_SIZE);
      const int max_cols = (mi_col + MI_BLOCK_SIZE > cm->mi_cols ?
          cm->mi_cols - mi_col : MI_BLOCK_SIZE);

      switch (mip->mbmi.sb_type) {
        case BLOCK_64X64:
          update_refresh_map(cpi, mip, 8, 8, mi_row, mi_col, bl_index);
          break;
        case BLOCK_64X32:
          update_refresh_map(cpi, mip, 8, 4, mi_row, mi_col, bl_index);
          mip2 = mip + mi_stride * 4;
          bl2_index = bl_index + cm->mi_cols * 4;
          if (4 >= max_rows)
          break;
          update_refresh_map(cpi, mip2, 8, 4, mi_row, mi_col, bl2_index);
          break;
        case BLOCK_32X64:
          update_refresh_map(cpi, mip, 4, 8, mi_row, mi_col, bl_index);
          mip2 = mip + 4;
          bl2_index = bl_index + 4;
          if (4 >= max_cols)
            break;
          update_refresh_map(cpi, mip2, 4, 8, mi_row, mi_col, bl2_index);
          break;
        default:
          for (idx = 0; idx < 4; mip += mi_offset_32[idx], ++idx) {
            const int mi_32_col_offset = ((idx & 1) << 2);
            const int mi_32_row_offset = ((idx >> 1) << 2);
            bl_index += bl_offset_32[idx];
            if (mi_32_col_offset >= max_cols || mi_32_row_offset >= max_rows)
              continue;
            switch (mip->mbmi.sb_type) {
              case BLOCK_32X32:
                update_refresh_map(cpi, mip, 4, 4, mi_row, mi_col, bl_index);
                break;
              case BLOCK_32X16:
                update_refresh_map(cpi, mip, 4, 2, mi_row, mi_col, bl_index);
                mip2 = mip + mi_stride * 2;
                bl2_index = bl_index + cm->mi_cols * 2;
                if (mi_32_row_offset + 2 >= max_rows)
                  continue;
                update_refresh_map(cpi, mip2, 4, 2, mi_row, mi_col, bl2_index);
                break;
              case BLOCK_16X32:
                update_refresh_map(cpi, mip, 2, 4, mi_row, mi_col, bl_index);
                mip2 = mip + 2;
                bl2_index = bl_index + 2;
                if (mi_32_col_offset + 2 >= max_cols)
                  continue;
                update_refresh_map(cpi, mip2, 2, 4, mi_row, mi_col, bl2_index);
                break;
              default:
                bl_index_16 = bl_index;
                for (idx2 = 0; idx2 < 4; mip += offset_16[idx2], ++idx2) {
                  const int mi_16_col_offset = mi_32_col_offset +
                      ((idx2 & 1) << 1);
                  const int mi_16_row_offset = mi_32_row_offset +
                      ((idx2 >> 1) << 1);
                  bl_index_16 += bl_offset_16[idx2];
                  if (mi_16_col_offset >= max_cols ||
                      mi_16_row_offset >= max_rows)
                    continue;
                  switch (mip->mbmi.sb_type) {
                    case BLOCK_16X16:
                      update_refresh_map(cpi, mip, 2, 2, mi_row, mi_col,
                                         bl_index_16);
                      break;
                      // TODO: Make explicit the stopping we do here for
                      // block sizes < cyclic_refresh->min_block_size.
                    case BLOCK_16X8:
                      break;
                    case BLOCK_8X16:
                      break;
                    default:
                      break;
                  }
                }
                break;
            }
          }
          break;
      }
    }
}

// Setup cyclic background refresh: set q delta, and segmentation map.
void vp9_setup_cyclic_background_refresh(VP9_COMP *const cpi) {
  VP9_COMMON *const cm = &cpi->common;
  CYCLIC_REFRESH *const cr = &cpi->cyclic_refresh;
  struct segmentation *const seg = &cm->seg;
  unsigned char *seg_map = cpi->segmentation_map;
  if ((cpi->common.frame_type == KEY_FRAME) ||
      (cpi->svc.temporal_layer_id > 0)) {
    if (cpi->common.frame_type == KEY_FRAME)
      cr->mb_index = 0;
    // Don't apply refresh on key frame or enhancement layer frames.
    // Set segmentation map to 0 and disable.
    vpx_memset(seg_map, 0, cm->mi_rows * cm->mi_cols);
    vp9_disable_segmentation(&cm->seg);
    return;
  } else {
    // Rate target ratio to set q delta.
    float rate_ratio_qdelta = 2.0;
    int qindex_delta = 0;
    int mbs_in_frame = cm->mi_rows * cm->mi_cols;
    int i, block_count, qindex2;

    // These control parameters may be set via control function, for now keep
    // them here and fixed.
    cr->max_mbs_perframe = 20;
    cr->max_qdelta_perc = 50;
    cr->min_block_size = BLOCK_16X16;
    cr->time_for_refresh = 1;

    printf("%d \n", cr->num_seg_blocks);
    cr->num_seg_blocks = 0;
    vp9_clear_system_state();
    // Set up segmentation.
    // Clear down the segment map.
    vpx_memset(seg_map, 0, cm->mi_rows * cm->mi_cols);
    vp9_enable_segmentation(&cm->seg);
    vp9_clearall_segfeatures(seg);
    // Select delta coding method.
    seg->abs_delta = SEGMENT_DELTADATA;
    // Segment 0 "Q" feature is disabled so it defaults to the baseline Q.
    vp9_disable_segfeature(seg, 0, SEG_LVL_ALT_Q);
    // Use segment 1 for in-frame Q adjustment.
    vp9_enable_segfeature(seg, 1, SEG_LVL_ALT_Q);

    // Set the q delta for segment 1.
    qindex_delta = compute_qdelta_by_rate(cpi,
                                          cm->base_qindex,
                                          rate_ratio_qdelta);
    // TODO: Incorporate the actual-vs-target rate over/undershoot from
    // previous encoded frame.
    if ((-qindex_delta) < cr->max_qdelta_perc * cm->base_qindex / 100) {
      qindex_delta = -cr->max_qdelta_perc * cm->base_qindex / 100;
    }

    // Compute rd-mult for segment 1.
    qindex2 = clamp(cm->base_qindex + cm->y_dc_delta_q + qindex_delta, 0, MAXQ);
    cr->rdmult = vp9_compute_rd_mult(cpi, qindex2);

    vp9_set_segdata(seg, 1, SEG_LVL_ALT_Q, qindex_delta);
    // Number of target macroblocks to get the q delta (segment 1).
    block_count = cr->max_mbs_perframe * mbs_in_frame / 100;
    // Set the segmentation map: cycle through the macroblocks, starting at
    // cr->mb_index, and stopping when either block_count blocks have been found
    // to be refreshed, or we have passed through whole frame.
    i = cr->mb_index;
    assert(i < mbs_in_frame);
    do {
      // If the macroblock is as a candidate for clean up then mark it
      // for possible boost/refresh (segment 1) The segment id may get reset to
      // 0 later if the macroblock gets coded anything other than ZEROMV.
      if (cr->map[i] == 0) {
        seg_map[i] = 1;
        block_count--;
      } else if (cr->map[i] < 0) {
        cr->map[i]++;
      }
      i++;
      if (i == mbs_in_frame) {
        i = 0;
      }
    }
    while (block_count && i != cr->mb_index);
    cr->mb_index = i;
  }
}
