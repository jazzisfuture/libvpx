/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp8/common/mvref_common.h"

#ifdef CONFIG_NEW_MVREF

#define MVREF_NEIGHBOURS 8
static int mv_ref_search[MVREF_NEIGHBOURS][2] =
{ {0,-1},{-1,0},{-1,-1},{0,-2},{-2,0},{-1,-2},{-2,-1},{-2,-2} };
//#define MVREF_NEIGHBOURS 15
//static int mv_ref_search[MVREF_NEIGHBOURS][2] =
//{ {0,-1},{-1,0},{-1,-1},{0,-2},{-2,0},{-1,-2},{-2,-1},{-2,-2},
//  {0,-3},{-3,0},{-1,-3},{-3,-1},{-2,-3},{-3,-2},{-3,-3}};

// clamp_mv
#define MV_BORDER (16 << 3) // Allow 16 pels in 1/8th pel units
static void clamp_mv(int_mv *mv, const MACROBLOCKD *xd) {

  if (mv->as_mv.col < (xd->mb_to_left_edge - MV_BORDER))
    mv->as_mv.col = xd->mb_to_left_edge - MV_BORDER;
  else if (mv->as_mv.col > xd->mb_to_right_edge + MV_BORDER)
    mv->as_mv.col = xd->mb_to_right_edge + MV_BORDER;

  if (mv->as_mv.row < (xd->mb_to_top_edge - MV_BORDER))
    mv->as_mv.row = xd->mb_to_top_edge - MV_BORDER;
  else if (mv->as_mv.row > xd->mb_to_bottom_edge + MV_BORDER)
    mv->as_mv.row = xd->mb_to_bottom_edge + MV_BORDER;
}

// Code for selecting / building and entropy coding a motion vector reference
// Returns a seperation value for two vectors.
// This is taken as the sum of the abs x and y difference.
unsigned int mv_distance(int_mv * mv1, int_mv * mv2) {
  return (abs(mv1->as_mv.row - mv2->as_mv.row) +
          abs(mv1->as_mv.col - mv2->as_mv.col));
}

int valid_mvref(
  const MODE_INFO *here,
  const MODE_INFO *candidate_mi,
  MV_REF_TYPE ref_type
) {

  if ( ref_type == FIRST_REF) {
  //return (candidate_mi->mbmi.ref_frame == here->mbmi.ref_frame) ||
  //       ((candidate_mi->mbmi.ref_frame != INTRA_FRAME) &&
  //       (candidate_mi->mbmi.mv[ref_type].as_int == 0));
    return ((candidate_mi->mbmi.ref_frame != INTRA_FRAME));

  } else {
  //return (candidate_mi->mbmi.second_ref_frame ==
  //        here->mbmi.second_ref_frame) ||
  //       ((candidate_mi->mbmi.second_ref_frame != INTRA_FRAME) &&
  //       (candidate_mi->mbmi.mv[ref_type].as_int == 0));
    return ((candidate_mi->mbmi.second_ref_frame != INTRA_FRAME));
  }
}

int ref_matches(
  MODE_INFO *here,
  MODE_INFO *candidate_mi,
  MV_REF_TYPE ref_type
) {

  if ( ref_type == FIRST_REF) {
    return candidate_mi->mbmi.ref_frame == here->mbmi.ref_frame;
  } else {
    return candidate_mi->mbmi.second_ref_frame == here->mbmi.second_ref_frame;
  }
}

// Performs mv adjsutment based on reference frame and clamps the MV
// if it goes off the edge of the buffer.
void scale_mv(
  MACROBLOCKD *xd,
  MODE_INFO *here,
  MODE_INFO *candidate_mi,
  MV_REF_TYPE ref_type,
  int *ref_sign_bias,
  int_mv *new_mv
) {

  int last_distance = 1;
  int gf_distance = xd->frames_since_golden;
  int arf_distance = xd->frames_till_alt_ref_frame;
  MV_REFERENCE_FRAME this_ref_frame;
  MV_REFERENCE_FRAME candidate_ref_frame;

  if ( ref_type == FIRST_REF) {
    this_ref_frame = here->mbmi.ref_frame;
    candidate_ref_frame = candidate_mi->mbmi.ref_frame;
  } else {
    this_ref_frame = here->mbmi.second_ref_frame;
    candidate_ref_frame = candidate_mi->mbmi.second_ref_frame;
  }

  if (candidate_ref_frame != this_ref_frame) {

    // Sign inversion where appropriate.
    if (ref_sign_bias[candidate_ref_frame] != ref_sign_bias[this_ref_frame]) {
      new_mv->as_mv.row = -(candidate_mi->mbmi.mv[ref_type].as_mv.row);
      new_mv->as_mv.col = -(candidate_mi->mbmi.mv[ref_type].as_mv.col);
    } else {
      new_mv->as_int = candidate_mi->mbmi.mv[ref_type].as_int;
    }

    // Scale based on frame distance if the reference frames not the same.
    /*frame_distances[LAST_FRAME] = 1;
    frame_distances[GOLDEN_FRAME] =
      (xd->frames_since_golden) ? xd->frames_since_golden : 1;
    frame_distances[ALTREF_FRAME] =
      (xd->frames_till_alt_ref_frame) ? xd->frames_till_alt_ref_frame : 1;

    if (frame_distances[this_ref_frame] &&
        frame_distances[candidate_ref_frame]) {
      new_mv->as_mv.row =
        (short)(((int)(new_mv->as_mv.row) *
                 frame_distances[this_ref_frame]) /
                frame_distances[candidate_ref_frame]);

      new_mv->as_mv.col =
        (short)(((int)(new_mv->as_mv.col) *
                 frame_distances[this_ref_frame]) /
                frame_distances[candidate_ref_frame]);
      }*/

  } else {
    new_mv->as_int = candidate_mi->mbmi.mv[ref_type].as_int;
  }

  // Clamp the MV so it does not point out of the buffer
  clamp_mv(new_mv, xd);
}

// Adds a new candidate reference vector to the list if indeed it is new.
// If it is not new then the score of the existing candidate that it matches
// is incremented and the list is resorted to reflect the current score.
void addmv_and_shuffle(
  int_mv *mv_list,
  int *mv_scores,
  int *index,
  int_mv new_mv,
  int ref_match
) {

  int i = *index;
  int duplicate_found = FALSE;

  // Check for duplicates. If there is one increment its score.
  while (i > 0) {
    i--;
    if (new_mv.as_int == mv_list[i].as_int) {
      duplicate_found = TRUE;
      mv_scores[i]++;
      break;
    }
  }

  // If no duplicate was found add the new vector
  if (!duplicate_found) {
    mv_list[*index].as_int = new_mv.as_int;
    mv_scores[*index] = (ref_match << 3) + 1;
    i = *index;
    (*index)++;
  }

  // Reshuffle the list so that highest scoring mvs at the top.
  while (i > 0) {
    if (mv_scores[i] > mv_scores[i-1]) {
      int tmp_score = mv_scores[i-1];
      int_mv tmp_mv = mv_list[i-1];

      mv_scores[i-1] = mv_scores[i];
      mv_list[i-1] = mv_list[i];
      mv_scores[i] = tmp_score;
      mv_list[i] = tmp_mv;
      i--;
    } else
      break;
  }
}

// This function searches the neighbourhood of a given MB/SB and populates a
// list of candidate reference vectors.
//
void find_mv_refs(
  MACROBLOCKD *xd,
  MODE_INFO *here,
  MODE_INFO *lf_here,
  MV_REF_TYPE ref_type,
  int_mv *mv_ref_list,
  int *ref_sign_bias
) {

  int i;
  MODE_INFO *candidate_mi;
  int_mv candidate_mvs[MAX_MV_REFS];
  int_mv new_mv;
  int candidate_scores[MAX_MV_REFS];
  int index = 0;
  int ref_match = 0;

  // Blank the reference vector lists and other local structures.
  vpx_memset(mv_ref_list, 0, sizeof(MV) * MAX_MV_REFS);
  vpx_memset(candidate_mvs, 0, sizeof(MV) * MAX_MV_REFS);
  vpx_memset(candidate_scores, 0, sizeof(candidate_scores));

  // Populate a list with candidate reference vectors from the
  // spatial neighbours.
  for (i = 0; i < 2; ++i) {
    if (((mv_ref_search[i][0] << 7) >= xd->mb_to_left_edge) &&
        ((mv_ref_search[i][1] << 7) >= xd->mb_to_top_edge)) {

      candidate_mi = here + mv_ref_search[i][0] +
                     (mv_ref_search[i][1] * xd->mode_info_stride);

      if (valid_mvref(here, candidate_mi, ref_type)) {
          ref_match = ref_matches(here, candidate_mi, ref_type);
          scale_mv(xd, here, candidate_mi, ref_type, ref_sign_bias, &new_mv );
          addmv_and_shuffle(candidate_mvs, candidate_scores,
                            &index, new_mv, ref_match);
      }
    }
  }

  // Look at the corresponding vector in the last frame
  candidate_mi = lf_here;
  if (valid_mvref(here, candidate_mi, ref_type)) {
    ref_match = ref_matches(here, candidate_mi, ref_type);
    scale_mv(xd, here, candidate_mi, ref_type, ref_sign_bias, &new_mv);
    addmv_and_shuffle(candidate_mvs, candidate_scores,
                      &index, new_mv, ref_match);
  }

  // Populate a list with candidate reference vectors from the
  // spatial neighbours.
  for (i = 2; i < MVREF_NEIGHBOURS; ++i) {
    if (((mv_ref_search[i][0] << 7) >= xd->mb_to_left_edge) &&
        ((mv_ref_search[i][1] << 7) >= xd->mb_to_top_edge)) {

      candidate_mi = here + mv_ref_search[i][0] +
                     (mv_ref_search[i][1] * xd->mode_info_stride);

      if (valid_mvref(here, candidate_mi, ref_type)) {
          ref_match = ref_matches(here, candidate_mi, ref_type);
          scale_mv(xd, here, candidate_mi, ref_type, ref_sign_bias, &new_mv );
          addmv_and_shuffle(candidate_mvs, candidate_scores,
                            &index, new_mv, ref_match);
      }
    }
  }

  // 0,0 is always a valid reference.
  for (i = 0; i < index; ++i)
    if (candidate_mvs[i].as_int == 0)
      break;
  if (i == index) {
    new_mv.as_int = 0;
    addmv_and_shuffle(candidate_mvs, candidate_scores,
                      &index, new_mv, 1);
  }

  // Copy over the candidate list.
  vpx_memcpy(mv_ref_list, candidate_mvs, sizeof(candidate_mvs));
}

#endif