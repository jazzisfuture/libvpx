
/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp10/common/mvref_common.h"

// This function searches the neighbourhood of a given MB/SB
// to try and find candidate reference vectors.
#if CONFIG_REF_MV && CONFIG_VAR_TX
static void scan_row_mbmi(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                          const int mi_row, const int mi_col, int block,
                          const MV_REFERENCE_FRAME ref_frame,
                          int row_offset,
                          CANDIDATE_MV *ref_mv_stack,
                          int *refmv_count) {
  const TileInfo *const tile = &xd->tile;
  int i;

  for (i = 0; i < xd->n8_w && *refmv_count < MAX_REF_MV_STACK_SIZE;) {
    POSITION mi_pos;
    mi_pos.row = row_offset;
    mi_pos.col = i;

    if (is_inside(tile, mi_col, mi_row, cm->mi_rows, &mi_pos)) {
      const MODE_INFO *const candidate_mi =
          xd->mi[mi_pos.row * xd->mi_stride + mi_pos.col];
      const MB_MODE_INFO *const candidate = &candidate_mi->mbmi;
      int len = VPXMIN(xd->n8_w,
                       num_8x8_blocks_wide_lookup[candidate->sb_type]);
      int weight = len << 3;
      int index = 0;

      if (candidate->ref_frame[0] == ref_frame) {
        for (index = 0; index < *refmv_count; ++index)
          if (ref_mv_stack[index].this_mv.as_int ==
              get_sub_block_mv(candidate_mi, 0, mi_pos.col, block).as_int)
            break;

        if (index < *refmv_count)
          ref_mv_stack[index].weight += weight;

        // Add a new item to the list.
        if (index == *refmv_count) {
          ref_mv_stack[index].this_mv =
              get_sub_block_mv(candidate_mi, 0, mi_pos.col, block);
          ref_mv_stack[index].weight = weight;
          ++(*refmv_count);
        }
      }

      if (candidate->ref_frame[1] == ref_frame) {
        for (index = 0; index < *refmv_count; ++index)
          if (ref_mv_stack[index].this_mv.as_int ==
              get_sub_block_mv(candidate_mi, 1, mi_pos.col, block).as_int)
            break;

        if (index < *refmv_count)
          ref_mv_stack[index].weight += weight;

        // Add a new item to the list.
        if (index == *refmv_count) {
          ref_mv_stack[index].this_mv =
              get_sub_block_mv(candidate_mi, 1, mi_pos.col, block);
          ref_mv_stack[index].weight = weight;
          ++(*refmv_count);
        }
      }

      i += len;
    } else {
      ++i;
    }
  }
}

static void scan_col_mbmi(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                          const int mi_row, const int mi_col, int block,
                          const MV_REFERENCE_FRAME ref_frame,
                          int col_offset,
                          CANDIDATE_MV *ref_mv_stack,
                          int *refmv_count) {
  const TileInfo *const tile = &xd->tile;
  int i;

  for (i = 0; i < xd->n8_h && *refmv_count < MAX_REF_MV_STACK_SIZE;) {
    POSITION mi_pos;
    mi_pos.row = i;
    mi_pos.col = col_offset;

    if (is_inside(tile, mi_col, mi_row, cm->mi_rows, &mi_pos)) {
      const MODE_INFO *const candidate_mi =
          xd->mi[mi_pos.row * xd->mi_stride + mi_pos.col];
      const MB_MODE_INFO *const candidate = &candidate_mi->mbmi;
      int len = VPXMIN(xd->n8_h,
                       num_8x8_blocks_high_lookup[candidate->sb_type]);
      int weight = len << 3;
      int index = 0;

      if (candidate->ref_frame[0] == ref_frame) {
        for (index = 0; index < *refmv_count; ++index)
          if (ref_mv_stack[index].this_mv.as_int ==
              get_sub_block_mv(candidate_mi, 0, mi_pos.col, block).as_int)
            break;

        if (index < *refmv_count)
          ref_mv_stack[index].weight += weight;

        if (index == *refmv_count) {
          ref_mv_stack[index].this_mv =
              get_sub_block_mv(candidate_mi, 0, mi_pos.col, block);
          ref_mv_stack[index].weight = weight;
          ++(*refmv_count);
        }
      }

      if (candidate->ref_frame[1] == ref_frame) {
        for (index = 0; index < *refmv_count; ++index)
          if (ref_mv_stack[index].this_mv.as_int ==
              get_sub_block_mv(candidate_mi, 1, mi_pos.col, block).as_int)
            break;

        if (index < *refmv_count)
          ref_mv_stack[index].weight += weight;

        // Add a new item to the list.
        if (index == *refmv_count) {
          ref_mv_stack[index].this_mv =
                  get_sub_block_mv(candidate_mi, 1, mi_pos.col, block);
          ref_mv_stack[index].weight = weight;
          ++(*refmv_count);
        }
      }

      i += len;
    } else {
      ++i;
    }
  }
}

static void scan_blk_mbmi(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                          const int mi_row, const int mi_col, int block,
                          const MV_REFERENCE_FRAME ref_frame,
                          int row_offset, int col_offset,
                          CANDIDATE_MV *ref_mv_stack,
                          int *refmv_count) {
  const TileInfo *const tile = &xd->tile;
  POSITION mi_pos;

  mi_pos.row = row_offset;
  mi_pos.col = col_offset;

  if (is_inside(tile, mi_col, mi_row, cm->mi_rows, &mi_pos) &&
      *refmv_count < MAX_REF_MV_STACK_SIZE) {
    const MODE_INFO *const candidate_mi =
        xd->mi[mi_pos.row * xd->mi_stride + mi_pos.col];
    const MB_MODE_INFO *const candidate = &candidate_mi->mbmi;
    int len = 1;
    int weight = len << 3;
    int index = 0;

    if (candidate->ref_frame[0] == ref_frame) {
      for (index = 0; index < *refmv_count; ++index)
        if (ref_mv_stack[index].this_mv.as_int ==
            get_sub_block_mv(candidate_mi, 0, mi_pos.col, block).as_int)
          break;

      if (index < *refmv_count)
        ref_mv_stack[index].weight += weight;

      if (index == *refmv_count) {
        ref_mv_stack[index].this_mv =
            get_sub_block_mv(candidate_mi, 0, mi_pos.col, block);
        ref_mv_stack[index].weight = weight;
        ++(*refmv_count);
      }
    }

    if (candidate->ref_frame[1] == ref_frame) {
      for (index = 0; index < *refmv_count; ++index)
        if (ref_mv_stack[index].this_mv.as_int ==
            get_sub_block_mv(candidate_mi, 1, mi_pos.col, block).as_int)
          break;

      if (index < *refmv_count)
        ref_mv_stack[index].weight += weight;

      // Add a new item to the list.
      if (index == *refmv_count) {
        ref_mv_stack[index].this_mv =
                get_sub_block_mv(candidate_mi, 1, mi_pos.col, block);
        ref_mv_stack[index].weight = weight;
        ++(*refmv_count);
      }
    }
  }  // Analyze a single 8x8 block motion information.
}

static void setup_ref_mv_list(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                              MODE_INFO *mi, MV_REFERENCE_FRAME ref_frame,
                              int_mv *mv_ref_list,
                              int block, int mi_row, int mi_col,
                              uint8_t *mode_context) {
  int idx, refmv_count = 0, nearest_refmv_count = 0;
  const int bw = num_8x8_blocks_wide_lookup[mi->mbmi.sb_type] << 3;
  const int bh = num_8x8_blocks_high_lookup[mi->mbmi.sb_type] << 3;

  CANDIDATE_MV ref_mv_stack[MAX_REF_MV_STACK_SIZE];
  CANDIDATE_MV tmp_mv;
  int len, nr_len;

  (void) mode_context;

  memset(ref_mv_stack, 0, sizeof(ref_mv_stack));

  // Scan the first above row mode info.
  scan_row_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -1, ref_mv_stack, &refmv_count);
  // Scan the first left column mode info.
  scan_col_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -1, ref_mv_stack, &refmv_count);

  nearest_refmv_count = refmv_count;

  // Analyze the top-left corner block mode info.
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -1, -1, ref_mv_stack, &refmv_count);

  // Scan the second outer area.
  scan_row_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -2, ref_mv_stack, &refmv_count);
  scan_col_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -2, ref_mv_stack, &refmv_count);
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -1, -2, ref_mv_stack, &refmv_count);
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -2, -1, ref_mv_stack, &refmv_count);

  // Scan the third outer area.
  scan_row_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -3, ref_mv_stack, &refmv_count);
  scan_col_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -3, ref_mv_stack, &refmv_count);
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -1, -3, ref_mv_stack, &refmv_count);
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -2, -2, ref_mv_stack, &refmv_count);
//  scan_blk_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
//                -3, -1, ref_mv_stack, &refmv_count);

  // Scan the fourth outer area.
  scan_row_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -4, ref_mv_stack, &refmv_count);
  // Scan the third left row mode info.
  scan_col_mbmi(cm, xd, mi_row, mi_col, block, ref_frame,
                -4, ref_mv_stack, &refmv_count);

  // Rank the likelihood and assign nearest and near mvs.
  len = refmv_count;
  while (len > nearest_refmv_count) {
    nr_len = nearest_refmv_count;
    for (idx = nearest_refmv_count + 1; idx < len; ++idx) {
      if (ref_mv_stack[idx - 1].weight < ref_mv_stack[idx].weight) {
        tmp_mv = ref_mv_stack[idx - 1];
        ref_mv_stack[idx - 1] = ref_mv_stack[idx];
        ref_mv_stack[idx] = tmp_mv;
        nr_len = idx;
      }
    }
    len = nr_len;
  }

  for (idx = 0; idx < VPXMIN(MAX_MV_REF_CANDIDATES, refmv_count); ++idx) {
    mv_ref_list[idx].as_int = ref_mv_stack[idx].this_mv.as_int;
    clamp_mv_ref(&mv_ref_list[idx].as_mv, bw, bh, xd);
  }
}
#endif

static void find_mv_refs_idx(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                             MODE_INFO *mi, MV_REFERENCE_FRAME ref_frame,
                             int_mv *mv_ref_list,
                             int block, int mi_row, int mi_col,
                             find_mv_refs_sync sync, void *const data,
                             uint8_t *mode_context) {
  const int *ref_sign_bias = cm->ref_frame_sign_bias;
  int i, refmv_count = 0;
  const POSITION *const mv_ref_search = mv_ref_blocks[mi->mbmi.sb_type];
  int different_ref_found = 0;
  int context_counter = 0;
  const MV_REF *const  prev_frame_mvs = cm->use_prev_frame_mvs ?
      cm->prev_frame->mvs + mi_row * cm->mi_cols + mi_col : NULL;
  const TileInfo *const tile = &xd->tile;
  const int bw = num_8x8_blocks_wide_lookup[mi->mbmi.sb_type] << 3;
  const int bh = num_8x8_blocks_high_lookup[mi->mbmi.sb_type] << 3;

#if !CONFIG_MISC_FIXES
  // Blank the reference vector list
  memset(mv_ref_list, 0, sizeof(*mv_ref_list) * MAX_MV_REF_CANDIDATES);
#endif

  // The nearest 2 blocks are treated differently
  // if the size < 8x8 we get the mv from the bmi substructure,
  // and we also need to keep a mode count.
  for (i = 0; i < 2; ++i) {
    const POSITION *const mv_ref = &mv_ref_search[i];
    if (is_inside(tile, mi_col, mi_row, cm->mi_rows, mv_ref)) {
      const MODE_INFO *const candidate_mi = xd->mi[mv_ref->col + mv_ref->row *
                                                   xd->mi_stride];
      const MB_MODE_INFO *const candidate = &candidate_mi->mbmi;
      // Keep counts for entropy encoding.
      context_counter += mode_2_counter[candidate->mode];
      different_ref_found = 1;

      if (candidate->ref_frame[0] == ref_frame)
        ADD_MV_REF_LIST(get_sub_block_mv(candidate_mi, 0, mv_ref->col, block),
                        refmv_count, mv_ref_list, bw, bh, xd, Done);
      else if (candidate->ref_frame[1] == ref_frame)
        ADD_MV_REF_LIST(get_sub_block_mv(candidate_mi, 1, mv_ref->col, block),
                        refmv_count, mv_ref_list, bw, bh, xd, Done);
    }
  }

  // Check the rest of the neighbors in much the same way
  // as before except we don't need to keep track of sub blocks or
  // mode counts.
  for (; i < MVREF_NEIGHBOURS; ++i) {
    const POSITION *const mv_ref = &mv_ref_search[i];
    if (is_inside(tile, mi_col, mi_row, cm->mi_rows, mv_ref)) {
      const MB_MODE_INFO *const candidate = &xd->mi[mv_ref->col + mv_ref->row *
                                                    xd->mi_stride]->mbmi;
      different_ref_found = 1;

      if (candidate->ref_frame[0] == ref_frame)
        ADD_MV_REF_LIST(candidate->mv[0], refmv_count, mv_ref_list,
                        bw, bh, xd, Done);
      else if (candidate->ref_frame[1] == ref_frame)
        ADD_MV_REF_LIST(candidate->mv[1], refmv_count, mv_ref_list,
                        bw, bh, xd, Done);
    }
  }

  // TODO(hkuang): Remove this sync after fixing pthread_cond_broadcast
  // on windows platform. The sync here is unncessary if use_perv_frame_mvs
  // is 0. But after removing it, there will be hang in the unit test on windows
  // due to several threads waiting for a thread's signal.
#if defined(_WIN32) && !HAVE_PTHREAD_H
    if (cm->frame_parallel_decode && sync != NULL) {
      sync(data, mi_row);
    }
#endif

  // Check the last frame's mode and mv info.
  if (cm->use_prev_frame_mvs) {
    // Synchronize here for frame parallel decode if sync function is provided.
    if (cm->frame_parallel_decode && sync != NULL) {
      sync(data, mi_row);
    }

    if (prev_frame_mvs->ref_frame[0] == ref_frame) {
      ADD_MV_REF_LIST(prev_frame_mvs->mv[0], refmv_count, mv_ref_list,
                      bw, bh, xd, Done);
    } else if (prev_frame_mvs->ref_frame[1] == ref_frame) {
      ADD_MV_REF_LIST(prev_frame_mvs->mv[1], refmv_count, mv_ref_list,
                      bw, bh, xd, Done);
    }
  }

  // Since we couldn't find 2 mvs from the same reference frame
  // go back through the neighbors and find motion vectors from
  // different reference frames.
  if (different_ref_found) {
    for (i = 0; i < MVREF_NEIGHBOURS; ++i) {
      const POSITION *mv_ref = &mv_ref_search[i];
      if (is_inside(tile, mi_col, mi_row, cm->mi_rows, mv_ref)) {
        const MB_MODE_INFO *const candidate = &xd->mi[mv_ref->col + mv_ref->row
                                              * xd->mi_stride]->mbmi;

        // If the candidate is INTRA we don't want to consider its mv.
        IF_DIFF_REF_FRAME_ADD_MV(candidate, ref_frame, ref_sign_bias,
                                 refmv_count, mv_ref_list, bw, bh, xd, Done);
      }
    }
  }

  // Since we still don't have a candidate we'll try the last frame.
  if (cm->use_prev_frame_mvs) {
    if (prev_frame_mvs->ref_frame[0] != ref_frame &&
        prev_frame_mvs->ref_frame[0] > INTRA_FRAME) {
      int_mv mv = prev_frame_mvs->mv[0];
      if (ref_sign_bias[prev_frame_mvs->ref_frame[0]] !=
          ref_sign_bias[ref_frame]) {
        mv.as_mv.row *= -1;
        mv.as_mv.col *= -1;
      }
      ADD_MV_REF_LIST(mv, refmv_count, mv_ref_list, bw, bh, xd, Done);
    }

    if (prev_frame_mvs->ref_frame[1] > INTRA_FRAME &&
#if !CONFIG_MISC_FIXES
        prev_frame_mvs->mv[1].as_int != prev_frame_mvs->mv[0].as_int &&
#endif
        prev_frame_mvs->ref_frame[1] != ref_frame) {
      int_mv mv = prev_frame_mvs->mv[1];
      if (ref_sign_bias[prev_frame_mvs->ref_frame[1]] !=
          ref_sign_bias[ref_frame]) {
        mv.as_mv.row *= -1;
        mv.as_mv.col *= -1;
      }
      ADD_MV_REF_LIST(mv, refmv_count, mv_ref_list, bw, bh, xd, Done);
    }
  }

 Done:

  mode_context[ref_frame] = counter_to_context[context_counter];

#if CONFIG_MISC_FIXES
  for (i = refmv_count; i < MAX_MV_REF_CANDIDATES; ++i)
      mv_ref_list[i].as_int = 0;
#else
  // Clamp vectors
  for (i = 0; i < MAX_MV_REF_CANDIDATES; ++i)
    clamp_mv_ref(&mv_ref_list[i].as_mv, bw, bh, xd);
#endif
}

void vp10_find_mv_refs(const VP10_COMMON *cm, const MACROBLOCKD *xd,
                      MODE_INFO *mi, MV_REFERENCE_FRAME ref_frame,
                      int_mv *mv_ref_list,
                      int mi_row, int mi_col,
                      find_mv_refs_sync sync, void *const data,
                      uint8_t *mode_context) {
  find_mv_refs_idx(cm, xd, mi, ref_frame, mv_ref_list, -1,
                   mi_row, mi_col, sync, data, mode_context);

#if CONFIG_REF_MV
  setup_ref_mv_list(cm, xd, mi, ref_frame, mv_ref_list, -1,
                    mi_row, mi_col, mode_context);
#endif
}

static void lower_mv_precision(MV *mv, int allow_hp) {
  const int use_hp = allow_hp && vp10_use_mv_hp(mv);
  if (!use_hp) {
    if (mv->row & 1)
      mv->row += (mv->row > 0 ? -1 : 1);
    if (mv->col & 1)
      mv->col += (mv->col > 0 ? -1 : 1);
  }
}

void vp10_find_best_ref_mvs(int allow_hp,
                           int_mv *mvlist, int_mv *nearest_mv,
                           int_mv *near_mv) {
  int i;
  // Make sure all the candidates are properly clamped etc
  for (i = 0; i < MAX_MV_REF_CANDIDATES; ++i) {
    lower_mv_precision(&mvlist[i].as_mv, allow_hp);
  }
  *nearest_mv = mvlist[0];
  *near_mv = mvlist[1];
}

void vp10_append_sub8x8_mvs_for_idx(VP10_COMMON *cm, MACROBLOCKD *xd,
                                   int block, int ref, int mi_row, int mi_col,
                                   int_mv *nearest_mv, int_mv *near_mv,
                                   uint8_t *mode_context) {
  int_mv mv_list[MAX_MV_REF_CANDIDATES];
  MODE_INFO *const mi = xd->mi[0];
  b_mode_info *bmi = mi->bmi;
  int n;

  assert(MAX_MV_REF_CANDIDATES == 2);

  find_mv_refs_idx(cm, xd, mi, mi->mbmi.ref_frame[ref], mv_list, block,
                   mi_row, mi_col, NULL, NULL, mode_context);

  near_mv->as_int = 0;
  switch (block) {
    case 0:
      nearest_mv->as_int = mv_list[0].as_int;
      near_mv->as_int = mv_list[1].as_int;
      break;
    case 1:
    case 2:
      nearest_mv->as_int = bmi[0].as_mv[ref].as_int;
      for (n = 0; n < MAX_MV_REF_CANDIDATES; ++n)
        if (nearest_mv->as_int != mv_list[n].as_int) {
          near_mv->as_int = mv_list[n].as_int;
          break;
        }
      break;
    case 3: {
      int_mv candidates[2 + MAX_MV_REF_CANDIDATES];
      candidates[0] = bmi[1].as_mv[ref];
      candidates[1] = bmi[0].as_mv[ref];
      candidates[2] = mv_list[0];
      candidates[3] = mv_list[1];

      nearest_mv->as_int = bmi[2].as_mv[ref].as_int;
      for (n = 0; n < 2 + MAX_MV_REF_CANDIDATES; ++n)
        if (nearest_mv->as_int != candidates[n].as_int) {
          near_mv->as_int = candidates[n].as_int;
          break;
        }
      break;
    }
    default:
      assert(0 && "Invalid block index.");
  }
}
