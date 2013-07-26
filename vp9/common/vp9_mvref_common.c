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



// Performs mv sign inversion if indicated by the reference frame combination.
static void scale_mv(MACROBLOCKD *xd, MV_REFERENCE_FRAME this_ref_frame,
                     MV_REFERENCE_FRAME candidate_ref_frame,
                     int_mv *candidate_mv, int *ref_sign_bias) {
  // Sign inversion where appropriate.
  if (ref_sign_bias[candidate_ref_frame] != ref_sign_bias[this_ref_frame]) {
    candidate_mv->as_mv.row = -candidate_mv->as_mv.row;
    candidate_mv->as_mv.col = -candidate_mv->as_mv.col;
  }
}


// This function searches the neighbourhood of a given MB/SB
// to try and find candidate reference vectors.
//
typedef enum {
  MV_REF_INTRA,
  MV_REF_ZERO,
  MV_REF_NEW,
  MV_REF_NONE,
  MV_REF_NUM
} MV_REF;

static const int mode_2_mv_ref_count[MB_MODE_COUNT] = {
  MV_REF_INTRA,  // DC_PRED
  MV_REF_INTRA,  // V_PRED
  MV_REF_INTRA,  // H_PRED
  MV_REF_INTRA,  // D45_PRED
  MV_REF_INTRA,  // D135_PRED
  MV_REF_INTRA,  // D117_PRED
  MV_REF_INTRA,  // D153_PRED
  MV_REF_INTRA,  // D27_PRED
  MV_REF_INTRA,  // D63_PRED
  MV_REF_INTRA,  // TM_PRED
  MV_REF_NONE,  // NEARESTMV
  MV_REF_NONE,  // NEARMV
  MV_REF_ZERO,  // ZEROMV
  MV_REF_NEW,  // NEWMV
};

// this function converts xoffset to a block index
static INLINE int block_idx_to_mv_ref(int x_offset, int block_idx) {
  return (x_offset ? 1 + (block_idx & ~1) : 2 + (block_idx & 1));
}

// This macro is used to add a motion vector mv_ref list if it isn't
// already in the list.  If its the second motion vector it will also
// skip all addition processing and jump to done!
#define add_mv_ref_list(MV) \
  if (refmv_count) { \
    if (MV.as_int != mv_ref_list[0].as_int) { \
      mv_ref_list[refmv_count].as_int = MV.as_int; \
      goto done; \
    } \
  } else { \
    mv_ref_list[refmv_count++].as_int = MV.as_int; \
  }

// This macro scales the mv to match the sign bias and then adds
// it to the list of candidates.
#define scale_and_add(MVI) \
  which_mv = candidate->mbmi.mv[MVI]; \
  scale_mv(xd, ref_frame, candidate->mbmi.ref_frame[MVI], &which_mv, \
           ref_sign_bias); \
  add_mv_ref_list(which_mv);

// Checks that the given mi_row, mi_col and search point
// are inside the borders of the tile.
static INLINE int is_inside(const int mi_col, const int mi_row,
                            const int cur_tile_mi_col_start,
                            const int cur_tile_mi_col_end, const int mi_rows,
                            const int (*mv_ref_search)[2], int i) {
  int mi_search_col, mi_search_row;

  // Check that the candidate is within the border.
  mi_search_col = mi_col + mv_ref_search[i][0];
  if (mi_search_col < cur_tile_mi_col_start ||
      mi_search_col > cur_tile_mi_col_end)
    return 0;

  mi_search_row = mi_row + mv_ref_search[i][1];
  if (mi_search_row < 0 || mi_search_row > mi_rows)
    return 0;

  return 1;
}
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
  int mv_counts[MV_REF_NUM] = {0, 0, 0, 0};
  int block_num;
  int_mv which_mv;
  const MODE_INFO *candidate;
  int check_sub_blocks = block_idx >= 0;

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
    mv_counts[mode_2_mv_ref_count[candidate->mbmi.mode]]++;

    // Check if the candidate comes from the same reference frame.
    if (candidate->mbmi.ref_frame[0] == ref_frame) {
      which_mv.as_int = candidate->mbmi.mv[0].as_int;

      // If the candidate is < 8x8 we grab the mv from bmi.
      if (check_sub_blocks && candidate->mbmi.sb_type < BLOCK_SIZE_SB8X8) {
        block_num = block_idx_to_mv_ref(mv_ref_search[i][0], block_idx);
        which_mv.as_int = candidate->bmi[block_num].as_mv[0].as_int;
      }
      add_mv_ref_list(which_mv);

    // Else if we have compound 2 mv vector and the second ref frame matches
    // grab its mv.
    } else if (candidate->mbmi.ref_frame[1] == ref_frame) {
      which_mv.as_int = candidate->mbmi.mv[1].as_int;

      // If the candidate is < 8x8 we grab the mv from bmi.
      if (check_sub_blocks && candidate->mbmi.sb_type < BLOCK_SIZE_SB8X8) {
        block_num = block_idx_to_mv_ref(mv_ref_search[i][0], block_idx);
        which_mv.as_int = candidate->bmi[block_num].as_mv[1].as_int;
      }
      add_mv_ref_list(which_mv);
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

    // Check if the candidate comes from the same reference frame.
    if (candidate->mbmi.ref_frame[0] == ref_frame) {
      add_mv_ref_list(candidate->mbmi.mv[0]);
    } else if (candidate->mbmi.ref_frame[1] == ref_frame) {
      // If the second reference frame matches add it to the list.
      add_mv_ref_list(candidate->mbmi.mv[1]);
    }
  }

  // Check the last frames mode and mv info
  if (lf_here && lf_here->mbmi.ref_frame[0] != INTRA_FRAME) {
    candidate = lf_here;
    // Check if the candidate comes from the same reference frame.
    if (candidate->mbmi.ref_frame[0] == ref_frame) {
      add_mv_ref_list(candidate->mbmi.mv[0]);
    }  else if (candidate->mbmi.ref_frame[1] == ref_frame) {
      // If the second reference frame matches add it to the list.
      add_mv_ref_list(candidate->mbmi.mv[1]);
    }
  }

  // Since we couldn't find 2 mvs from the same reference frame
  // go back through the neighbors and find motion vectors from
  // different reference frames.
  for (i = 0; i < MVREF_NEIGHBOURS; ++i) {
    if (!is_inside(mi_col, mi_row, cm->cur_tile_mi_col_start,
                   cm->cur_tile_mi_col_end, cm->mi_rows, mv_ref_search, i))
      continue;

    candidate = here + mv_ref_search[i][0]
                + (mv_ref_search[i][1] * xd->mode_info_stride);

    // If the candidate is INTRA we don't want to consider its mv.
    if (candidate->mbmi.ref_frame[0] == INTRA_FRAME)
      continue;

    // If the reference frame is different.
    if (candidate->mbmi.ref_frame[0] != ref_frame) {
      scale_and_add(0);
    }

    // If its compound prediction and second reference frame is used
    // and not the same as first.
    if (candidate->mbmi.ref_frame[1] != ref_frame &&
        candidate->mbmi.ref_frame[1] > INTRA_FRAME &&
        candidate->mbmi.mv[1].as_int != candidate->mbmi.mv[0].as_int) {
      scale_and_add(1);
    }
  }

  // Since we still don't have a candidate we'll try the last frame.
  if (lf_here && lf_here->mbmi.ref_frame[0] != INTRA_FRAME) {
    candidate = lf_here;
    // If the reference frame is different.
    if (candidate->mbmi.ref_frame[0] != ref_frame) {
      scale_and_add(0);
    }

    // If its compound prediction and second reference frame is used..
    if (candidate->mbmi.ref_frame[1] != ref_frame &&
        candidate->mbmi.ref_frame[1] > INTRA_FRAME &&
        candidate->mbmi.mv[1].as_int != candidate->mbmi.mv[0].as_int) {
        scale_and_add(1);
    }
  }

 done:

  if (!mv_counts[MV_REF_INTRA]) {
    if (!mv_counts[MV_REF_NEW]) {
      // 0 = both zero mv
      // 1 = one zero mv + one a predicted mv
      // 2 = two predicted mvs
      mbmi->mb_mode_context[ref_frame] = 2 - mv_counts[MV_REF_ZERO];
    } else {
      // 3 = one predicted/zero and one new mv
      // 4 = two new mvs
      mbmi->mb_mode_context[ref_frame] = 2 + mv_counts[MV_REF_NEW];
    }
  } else {
    // 5 = one intra neighbour + x
    // 6 = two intra neighbours
    mbmi->mb_mode_context[ref_frame] = 4 + mv_counts[MV_REF_INTRA];
  }

  // Clamp vectors
  for (i = 0; i < MAX_MV_REF_CANDIDATES; ++i) {
    clamp_mv_ref(xd, &mv_ref_list[i]);
  }
}
