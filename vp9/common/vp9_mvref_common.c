
/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/common/vp9_mvref_common.h"

#define MVREF_NEIGHBOURS 8

typedef enum {
  BOTH_ZERO = 0,
  ZERO_PLUS_PREDICTED = 1,
  BOTH_PREDICTED = 2,
  NEW_PLUS_NON_INTRA = 3,
  BOTH_NEW = 4,
  INTRA_PLUS_NON_INTRA = 5,
  BOTH_INTRA = 6,
  INVALID_CASE = 9
} motion_vector_context;

// This is used to figure out a context for the ref blocks. The code flattens
// an array that would have 3 possible counts (0, 1 & 2) for 3 choices by
// adding 9 for each intra block, 3 for each zero mv and 1 for each new
// motion vector. This single number is then converted into a context
// with a single lookup ( counter_to_context ).
static const int mode_2_counter[MB_MODE_COUNT] = {
  9,  // DC_PRED
  9,  // V_PRED
  9,  // H_PRED
  9,  // D45_PRED
  9,  // D135_PRED
  9,  // D117_PRED
  9,  // D153_PRED
  9,  // D207_PRED
  9,  // D63_PRED
  9,  // TM_PRED
  0,  // NEARESTMV
  0,  // NEARMV
  3,  // ZEROMV
  1,  // NEWMV
};

// There are 3^3 different combinations of 3 counts that can be either 0,1 or
// 2. However the actual count can never be greater than 2 so the highest
// counter we need is 18. 9 is an invalid counter that's never used.
static const int counter_to_context[19] = {
  BOTH_PREDICTED,  // 0
  NEW_PLUS_NON_INTRA,  // 1
  BOTH_NEW,  // 2
  ZERO_PLUS_PREDICTED,  // 3
  NEW_PLUS_NON_INTRA,  // 4
  INVALID_CASE,  // 5
  BOTH_ZERO,  // 6
  INVALID_CASE,  // 7
  INVALID_CASE,  // 8
  INTRA_PLUS_NON_INTRA,  // 9
  INTRA_PLUS_NON_INTRA,  // 10
  INVALID_CASE,  // 11
  INTRA_PLUS_NON_INTRA,  // 12
  INVALID_CASE,  // 13
  INVALID_CASE,  // 14
  INVALID_CASE,  // 15
  INVALID_CASE,  // 16
  INVALID_CASE,  // 17
  BOTH_INTRA  // 18
};

static const MV mv_ref_blocks[BLOCK_SIZES][MVREF_NEIGHBOURS] = {
  // 4X4
  {{-1, 0}, {0, -1}, {-1, -1}, {-2, 0}, {0, -2}, {-2, -1}, {-1, -2}, {-2, -2}},
  // 4X8
  {{-1, 0}, {0, -1}, {-1, -1}, {-2, 0}, {0, -2}, {-2, -1}, {-1, -2}, {-2, -2}},
  // 8X4
  {{-1, 0}, {0, -1}, {-1, -1}, {-2, 0}, {0, -2}, {-2, -1}, {-1, -2}, {-2, -2}},
  // 8X8
  {{-1, 0}, {0, -1}, {-1, -1}, {-2, 0}, {0, -2}, {-2, -1}, {-1, -2}, {-2, -2}},
  // 8X16
  {{0, -1}, {-1, 0}, {1, -1}, {-1, -1}, {0, -2}, {-2, 0}, {-2, -1}, {-1, -2}},
  // 16X8
  {{-1, 0}, {0, -1}, {-1, 1}, {-1, -1}, {-2, 0}, {0, -2}, {-1, -2}, {-2, -1}},
  // 16X16
  {{-1, 0}, {0, -1}, {-1, 1}, {1, -1}, {-1, -1}, {-3, 0}, {0, -3}, {-3, -3}},
  // 16X32
  {{0, -1}, {-1, 0}, {2, -1}, {-1, -1}, {-1, 1}, {0, -3}, {-3, 0}, {-3, -3}},
  // 32X16
  {{-1, 0}, {0, -1}, {-1, 2}, {-1, -1}, {1, -1}, {-3, 0}, {0, -3}, {-3, -3}},
  // 32X32
  {{-1, 1}, {1, -1}, {-1, 2}, {2, -1}, {-1, -1}, {-3, 0}, {0, -3}, {-3, -3}},
  // 32X64
  {{0, -1}, {-1, 0}, {4, -1}, {-1, 2}, {-1, -1}, {0, -3}, {-3, 0}, {2, -1}},
  // 64X32
  {{-1, 0}, {0, -1}, {-1, 4}, {2, -1}, {-1, -1}, {-3, 0}, {0, -3}, {-1, 2}},
  // 64X64
  {{-1, 3}, {3, -1}, {-1, 4}, {4, -1}, {-1, -1}, {-1, 0}, {0, -1}, {-1, 6}}
};

static const int idx_n_column_to_subblock[4][2] = {
  {1, 2},
  {1, 3},
  {3, 2},
  {3, 3}
};

// clamp_mv_ref
#define MV_BORDER (16 << 3) // Allow 16 pels in 1/8th pel units

static void clamp_mv_ref(MV *mv, const MACROBLOCKD *xd) {
  clamp_mv(mv, xd->mb_to_left_edge - MV_BORDER,
               xd->mb_to_right_edge + MV_BORDER,
               xd->mb_to_top_edge - MV_BORDER,
               xd->mb_to_bottom_edge + MV_BORDER);
}

// This function returns either the appropriate sub block or block's mv
// on whether the block_size < 8x8 and we have check_sub_blocks set.
static INLINE int_mv get_sub_block_mv(const MODE_INFO *candidate,
                                      int check_sub_blocks, int which_mv,
                                      int search_col, int block_idx) {
  return check_sub_blocks && candidate->mbmi.sb_type < BLOCK_8X8
          ? candidate->bmi[idx_n_column_to_subblock[block_idx][search_col == 0]]
              .as_mv[which_mv]
          : candidate->mbmi.mv[which_mv];
}


// Performs mv sign inversion if indicated by the reference frame combination.
static INLINE int_mv scale_mv(const MB_MODE_INFO *mbmi, int ref,
                              const MV_REFERENCE_FRAME this_ref_frame,
                              const int *ref_sign_bias) {
  int_mv mv = mbmi->mv[ref];
  if (ref_sign_bias[mbmi->ref_frame[ref]] != ref_sign_bias[this_ref_frame]) {
    mv.as_mv.row *= -1;
    mv.as_mv.col *= -1;
  }
  return mv;
}

// Add motion vector if it is not in the list. The return value indicates
// if the list is full
static INLINE int add_mv_ref_list(int_mv mv, int_mv *mv_ref_list, int *count) {
  if (*count) {
    if (mv.as_int != mv_ref_list[0].as_int) {
      mv_ref_list[1] = mv;
      return 1;
    }
  } else {
    mv_ref_list[0] = mv;
    ++(*count);
  }
  return 0;
}

static INLINE int add_mv_diff_ref(const MB_MODE_INFO *mbmi, int rf,
                                  const int *sign_bias, int_mv *mv_ref_list,
                                  int *count) {
  if (mbmi->ref_frame[0] != rf) {
    if (add_mv_ref_list(scale_mv(mbmi, 0, rf, sign_bias), mv_ref_list, count))
      return 1;
  }
  if (mbmi->ref_frame[1] != rf &&
      has_second_ref(mbmi) && mbmi->mv[1].as_int != mbmi->mv[0].as_int) {
    return add_mv_ref_list(scale_mv(mbmi, 1, rf, sign_bias),
                           mv_ref_list, count);
  }
  return 0;
}

// Checks that the given mi_row, mi_col and search point
// are inside the borders of the tile.
static INLINE int is_inside(const VP9_COMMON *cm, int mi_col, int mi_row,
                            const MV *mv) {
  return !(mi_row + mv->row < 0 ||
           mi_col + mv->col < cm->cur_tile_mi_col_start ||
           mi_row + mv->row >= cm->mi_rows ||
           mi_col + mv->col >= cm->cur_tile_mi_col_end);
}

// This function searches the neighbourhood of a given MB/SB
// to try and find candidate reference vectors.
void vp9_find_mv_refs_idx(const VP9_COMMON *cm, const MACROBLOCKD *xd,
                          MODE_INFO *mi, const MODE_INFO *prev_mi,
                          MV_REFERENCE_FRAME ref_frame,
                          int_mv *mv_ref_list,
                          int block_idx,
                          int mi_row, int mi_col) {
  const int *ref_sign_bias = cm->ref_frame_sign_bias;
  int i, refmv_count = 0;
  const MV *const mv_ref_search = mv_ref_blocks[mi->mbmi.sb_type];
  const MB_MODE_INFO *const prev_mbmi = prev_mi ? &prev_mi->mbmi : NULL;
  int different_ref_found = 0;
  int context_counter = 0;

  // Blank the reference vector list
  vpx_memset(mv_ref_list, 0, sizeof(*mv_ref_list) * MAX_MV_REF_CANDIDATES);

  // The nearest 2 blocks are treated differently
  // if the size < 8x8 we get the mv from the bmi substructure,
  // and we also need to keep a mode count.
  for (i = 0; i < 2; ++i) {
    const MV *const mv_ref = &mv_ref_search[i];
    if (is_inside(cm, mi_col, mi_row, mv_ref)) {
      const int check_sub_blocks = block_idx >= 0;
      const MODE_INFO *const candidate_mi = xd->mi_8x8[mv_ref->col + mv_ref->row
                                                   * xd->mode_info_stride];
      const MB_MODE_INFO *const candidate = &candidate_mi->mbmi;

      // Keep counts for entropy encoding.
      context_counter += mode_2_counter[candidate->mode];

      // Check if the candidate comes from the same reference frame.
      if (candidate->ref_frame[0] == ref_frame) {
        if (add_mv_ref_list(get_sub_block_mv(candidate_mi, check_sub_blocks,
                                             0, mv_ref->col, block_idx),
                            mv_ref_list, &refmv_count))
          goto Done;
        different_ref_found = candidate->ref_frame[1] != ref_frame;
      } else {
        if (candidate->ref_frame[1] == ref_frame)
          if (add_mv_ref_list(get_sub_block_mv(candidate_mi, check_sub_blocks,
                                               1, mv_ref->col, block_idx),
                              mv_ref_list, &refmv_count))
            goto Done;
        different_ref_found = 1;
      }
    }
  }

  // Check the rest of the neighbors in much the same way
  // as before except we don't need to keep track of sub blocks or
  // mode counts.
  for (; i < MVREF_NEIGHBOURS; ++i) {
    const MV *const mv_ref = &mv_ref_search[i];
    if (is_inside(cm, mi_col, mi_row, mv_ref)) {
      const MB_MODE_INFO *const candidate = &xd->mi_8x8[mv_ref->col +
                                            mv_ref->row
                                            * xd->mode_info_stride]->mbmi;

      if (candidate->ref_frame[0] == ref_frame) {
        if (add_mv_ref_list(candidate->mv[0], mv_ref_list, &refmv_count))
          goto Done;
        different_ref_found = candidate->ref_frame[1] != ref_frame;
      } else {
        if (candidate->ref_frame[1] == ref_frame)
          if (add_mv_ref_list(candidate->mv[1], mv_ref_list, &refmv_count))
            goto Done;
        different_ref_found = 1;
      }
    }
  }

  // Check the last frame's mode and mv info.
  if (prev_mbmi) {
    if (prev_mbmi->ref_frame[0] == ref_frame) {
      if (add_mv_ref_list(prev_mbmi->mv[0], mv_ref_list, &refmv_count))
        goto Done;
    } else if (prev_mbmi->ref_frame[1] == ref_frame) {
      if (add_mv_ref_list(prev_mbmi->mv[1], mv_ref_list, &refmv_count))
        goto Done;
    }
  }

  // Since we couldn't find 2 mvs from the same reference frame
  // go back through the neighbors and find motion vectors from
  // different reference frames.
  if (different_ref_found) {
    for (i = 0; i < MVREF_NEIGHBOURS; ++i) {
      const MV *mv_ref = &mv_ref_search[i];
      if (is_inside(cm, mi_col, mi_row, mv_ref)) {
        const MB_MODE_INFO *const candidate = &xd->mi_8x8[mv_ref->col +
                                                          mv_ref->row
                                              * xd->mode_info_stride]->mbmi;

        // If the candidate is INTRA we don't want to consider its mv.
        if (is_inter_block(candidate)) {
          if (add_mv_diff_ref(candidate, ref_frame, ref_sign_bias,
                              mv_ref_list, &refmv_count))
            goto Done;
        }
      }
    }
  }

  // Since we still don't have a candidate we'll try the last frame.
  if (prev_mbmi && is_inter_block(prev_mbmi)) {
    if (add_mv_diff_ref(prev_mbmi, ref_frame, ref_sign_bias,
                        mv_ref_list, &refmv_count))
      goto Done;
  }

 Done:

  mi->mbmi.mode_context[ref_frame] = counter_to_context[context_counter];

  // Clamp vectors
  for (i = 0; i < MAX_MV_REF_CANDIDATES; ++i)
    clamp_mv_ref(&mv_ref_list[i].as_mv, xd);
}
