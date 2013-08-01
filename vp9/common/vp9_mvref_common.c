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
static const int mv_ref_blocks[BLOCK_SIZE_TYPES][MVREF_NEIGHBOURS][2] = {
  // SB4X4
  {{0, -1}, {-1, 0}, {-1, -1}, {0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, -2}},
  // SB4X8
  {{0, -1}, {-1, 0}, {-1, -1}, {0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, -2}},
  // SB8X4
  {{0, -1}, {-1, 0}, {-1, -1}, {0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, -2}},
  // SB8X8
  {{0, -1}, {-1, 0}, {-1, -1}, {0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, -2}},
  // SB8X16
  {{-1, 0}, {0, -1}, {-1, 1}, {-1, -1}, {-2, 0}, {0, -2}, {-1, -2}, {-2, -1}},
  // SB16X8
  {{0, -1}, {-1, 0}, {1, -1}, {-1, -1}, {0, -2}, {-2, 0}, {-2, -1}, {-1, -2}},
  // SB16X16
  {{0, -1}, {-1, 0}, {1, -1}, {-1, 1}, {-1, -1}, {0, -3}, {-3, 0}, {-3, -3}},
  // SB16X32
  {{-1, 0}, {0, -1}, {-1, 2}, {-1, -1}, {1, -1}, {-3, 0}, {0, -3}, {-3, -3}},
  // SB32X16
  {{0, -1}, {-1, 0}, {2, -1}, {-1, -1}, {-1, 1}, {0, -3}, {-3, 0}, {-3, -3}},
  // SB32X32
  {{1, -1}, {-1, 1}, {2, -1}, {-1, 2}, {-1, -1}, {0, -3}, {-3, 0}, {-3, -3}},
  // SB32X64
  {{-1, 0}, {0, -1}, {-1, 4}, {2, -1}, {-1, -1}, {-3, 0}, {0, -3}, {-1, 2}},
  // SB64X32
  {{0, -1}, {-1, 0}, {4, -1}, {-1, 2}, {-1, -1}, {0, -3}, {-3, 0}, {2, -1}},
  // SB64X64
  {{3, -1}, {-1, 3}, {4, -1}, {-1, 4}, {-1, -1}, {0, -1}, {-1, 0}, {6, -1}}
};


// clamp_mv_ref
#define MV_BORDER (16 << 3) // Allow 16 pels in 1/8th pel units

static void clamp_mv_ref(const MACROBLOCKD *xd, int_mv *mv) {
  mv->as_mv.col = clamp(mv->as_mv.col, xd->mb_to_left_edge - MV_BORDER,
                                       xd->mb_to_right_edge + MV_BORDER);
  mv->as_mv.row = clamp(mv->as_mv.row, xd->mb_to_top_edge - MV_BORDER,
                                       xd->mb_to_bottom_edge + MV_BORDER);
}

// This is used to figure out a context for the ref blocks. The code flattens
// an array that would have 3 possible counts (0, 1 & 2) for 3 choices by
// adding 9 for each intra block, 3 for each zero mv and 1 for each new
// motion vector. This single number is then converted into a context
// with a single lookup ( counter to context ).
static const int mode_2_counter[MB_MODE_COUNT] = {
  9,  // DC_PRED
  9,  // V_PRED
  9,  // H_PRED
  9,  // D45_PRED
  9,  // D135_PRED
  9,  // D117_PRED
  9,  // D153_PRED
  9,  // D27_PRED
  9,  // D63_PRED
  9,  // TM_PRED
  0,  // NEARESTMV
  0,  // NEARMV
  3,  // ZEROMV
  1,  // NEWMV
};

// There are 3^3 different combinations of 3 counts that can be either 0,1 or
// 2.   However the actual count can never be greater than 2 so the highest
// counter we need is 18.    9 is an invalid counter that's never used.
static int const counter_to_context[19] = {
    2,  // 0
    3,  // 1
    4,  // 2
    1,  // 3
    3,  // 4
    9,  // 5
    0,  // 6
    9,  // 7
    9,  // 8
    5,  // 9
    5,  // 10
    9,  // 11
    5,  // 12
    9,  // 13
    9,  // 14
    9,  // 15
    9,  // 16
    9,  // 17
    6   // 18
};

// This macro is used to add a motion vector mv_ref list if it isn't
// already in the list.  If its the second motion vector it will also
// skip all addition processing and jump to done!
#define ADD_MV_REF_LIST(MV) \
  if (refmv_count) { \
    if (MV.as_int != mv_ref_list[0].as_int) { \
      mv_ref_list[refmv_count] = MV; \
      goto done; \
    } \
  } else { \
    mv_ref_list[refmv_count++] = MV; \
  }

static const int idx_n_column_to_subblock[4][2] = {
    {1, 2},
    {1, 3},
    {3, 2},
    {3, 3}
};

// This function returns either the appropriate sub block or block's mv
// on whether the block_size < 8x8 and we have check_sub_blocks set.
static INLINE int_mv get_sub_block_mv(const MODE_INFO *candidate,
                                      int check_sub_blocks, int which_mv,
                                      int search_col, int block_idx) {
  return (check_sub_blocks && candidate->mbmi.sb_type < BLOCK_SIZE_SB8X8 ?
      candidate->bmi[idx_n_column_to_subblock[block_idx][search_col == 0]]
          .as_mv[which_mv] :
      candidate->mbmi.mv[which_mv]);
}


// Performs mv sign inversion if indicated by the reference frame combination.
static INLINE int_mv scale_mv(const MODE_INFO *candidate, int which_mv,
                              MV_REFERENCE_FRAME this_ref_frame,
                              int *ref_sign_bias) {
  int_mv return_mv = candidate->mbmi.mv[which_mv];

  // Sign inversion where appropriate.
  if (ref_sign_bias[candidate->mbmi.ref_frame[which_mv]] !=
      ref_sign_bias[this_ref_frame]) {
    return_mv.as_mv.row *= -1;
    return_mv.as_mv.col *= -1;
  }
  return return_mv;
}

// If either reference frame is different, and not INTRA, and they
// are different from each other scale and add the mv to our list.
#define IF_DIFF_REF_FRAME_ADD_MV(CANDIDATE) \
  if (CANDIDATE->mbmi.ref_frame[0] != ref_frame) { \
    ADD_MV_REF_LIST(scale_mv(CANDIDATE, 0, ref_frame, ref_sign_bias)); \
  } \
  if (CANDIDATE->mbmi.ref_frame[1] != ref_frame && \
      CANDIDATE->mbmi.ref_frame[1] > INTRA_FRAME && \
      CANDIDATE->mbmi.mv[1].as_int != CANDIDATE->mbmi.mv[0].as_int) { \
    ADD_MV_REF_LIST(scale_mv(CANDIDATE, 1, ref_frame, ref_sign_bias)); \
  }

// Checks that the given mi_row, mi_col and search point
// are inside the borders of the tile.
static INLINE int is_inside(const int mi_col, const int mi_row,
                            const int cur_tile_mi_col_start,
                            const int cur_tile_mi_col_end, const int mi_rows,
                            const int (*mv_ref_search)[2], int i) {
  int mi_search_col, mi_search_row;

  // Check that the candidate is within the border.  We only need to check
  // the left side because all the positive right side ones are for blocks that
  // are large enough to support the + value they have within their border.
  mi_search_row = mi_row + mv_ref_search[i][1];
  if (mi_search_row < 0 )
    return 0;

  mi_search_col = mi_col + mv_ref_search[i][0];
  if (mi_search_col < cur_tile_mi_col_start)
    return 0;

  return 1;
}

// This function searches the neighbourhood of a given MB/SB
// to try and find candidate reference vectors.
void vp9_find_mv_refs_idx(VP9_COMMON *cm, MACROBLOCKD *xd, MODE_INFO *here,
                          MODE_INFO *lf_here, MV_REFERENCE_FRAME ref_frame,
                          int_mv *mv_ref_list, int *ref_sign_bias,
                          int block_idx) {
  int i;
  MB_MODE_INFO * mbmi = &xd->mode_info_context->mbmi;
  int refmv_count = 0;
  const int (*mv_ref_search)[2] = mv_ref_blocks[mbmi->sb_type];
  const int mi_col = get_mi_col(xd);
  const int mi_row = get_mi_row(xd);
  const MODE_INFO *candidate;
  int check_sub_blocks = block_idx >= 0;
  int different_ref_found = 0;
  int context_counter = 0;

  // Blank the reference vector list
  vpx_memset(mv_ref_list, 0, sizeof(int_mv) * MAX_MV_REF_CANDIDATES);

  // The nearest 2 blocks are treated differently
  // if the size < 8x8 we get the mv from the bmi substructure,
  // and we also need to keep a mode count.
  for (i = 0; i < 2 ; i++) {
    if (!is_inside(mi_col, mi_row, cm->cur_tile_mi_col_start,
                   cm->cur_tile_mi_col_end, cm->mi_rows, mv_ref_search, i))
      continue;

    candidate = here + mv_ref_search[i][0]
                + (mv_ref_search[i][1] * xd->mode_info_stride);

    // Keep counts for entropy encoding.
    context_counter += mode_2_counter[candidate->mbmi.mode];

    // Check if the candidate comes from the same reference frame.
    if (candidate->mbmi.ref_frame[0] == ref_frame) {
      ADD_MV_REF_LIST(get_sub_block_mv(candidate, check_sub_blocks, 0,
                                       mv_ref_search[i][0], block_idx));
      different_ref_found = candidate->mbmi.ref_frame[1] != ref_frame;
    } else {
      different_ref_found = 1;
      if (candidate->mbmi.ref_frame[1] == ref_frame) {
        // Add second motion vector if it has the same ref_frame.
        ADD_MV_REF_LIST(get_sub_block_mv(candidate, check_sub_blocks, 1,
                                         mv_ref_search[i][0], block_idx));
      }
    }
  }
  // Check the rest of the neighbors in much the same way
  // as before except we don't need to keep track of sub blocks or
  // mode counts;
  for (; i < MVREF_NEIGHBOURS; ++i) {
    if (!is_inside(mi_col, mi_row, cm->cur_tile_mi_col_start,
                   cm->cur_tile_mi_col_end, cm->mi_rows, mv_ref_search, i))
      continue;

    candidate = here + mv_ref_search[i][0]
                + (mv_ref_search[i][1] * xd->mode_info_stride);

    if (candidate->mbmi.ref_frame[0] == ref_frame) {
      ADD_MV_REF_LIST(candidate->mbmi.mv[0]);
      different_ref_found = candidate->mbmi.ref_frame[1] != ref_frame;
    } else {
      different_ref_found = 1;
      if (candidate->mbmi.ref_frame[1] == ref_frame) {
        ADD_MV_REF_LIST(candidate->mbmi.mv[1]);
      }
    }
  }

  // Check the last frames mode and mv info
  if (lf_here) {
    if (lf_here->mbmi.ref_frame[0] == ref_frame) {
      ADD_MV_REF_LIST(lf_here->mbmi.mv[0]);
    } else if (lf_here->mbmi.ref_frame[1] == ref_frame) {
        ADD_MV_REF_LIST(lf_here->mbmi.mv[1]);
    }
  };

  // Since we couldn't find 2 mvs from the same reference frame
  // go back through the neighbors and find motion vectors from
  // different reference frames.
  if (different_ref_found) {
    for (i = 0; i < MVREF_NEIGHBOURS; ++i) {
      if (!is_inside(mi_col, mi_row, cm->cur_tile_mi_col_start,
                     cm->cur_tile_mi_col_end, cm->mi_rows, mv_ref_search, i))
        continue;

      candidate = here + mv_ref_search[i][0]
          + (mv_ref_search[i][1] * xd->mode_info_stride);

      // If the candidate is INTRA we don't want to consider its mv.
      if (candidate->mbmi.ref_frame[0] == INTRA_FRAME)
        continue;

      IF_DIFF_REF_FRAME_ADD_MV(candidate);
    }
  }

  // Since we still don't have a candidate we'll try the last frame.
  if (lf_here && lf_here->mbmi.ref_frame[0] != INTRA_FRAME) {
    IF_DIFF_REF_FRAME_ADD_MV(lf_here);
  }

 done:

  mbmi->mb_mode_context[ref_frame] = counter_to_context[context_counter];

  // Clamp vectors
  for (i = 0; i < MAX_MV_REF_CANDIDATES; ++i) {
    clamp_mv_ref(xd, &mv_ref_list[i]);
  }
}
