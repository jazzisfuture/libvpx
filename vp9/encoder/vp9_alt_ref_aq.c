/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 *  This file contains functions for setting up
 *  adaptive segmentation for altref frames
 *
 *  TODO(yuryg): ... detailed description, may be ...
 */

#include <string.h>

#include "vpx_dsp/vpx_dsp_common.h"

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_seg_common.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_segmentation.h"

#include "vp9/common/vp9_matx.h"
#include "vp9/encoder/vp9_alt_ref_aq.h"


/* #define ALT_REF_AQ_DEBUG */

// TODO(yuryg): may be, it make sense to move this to init
static void vp9_alt_ref_aq_wrap_cpi_segmentation(ALT_REF_AQ* const this,
                                                 const VP9_COMP *const cpi) {
  vp9_matx_wrap(&this->cpi_segmentation_map,
                cpi->common.mi_rows,
                cpi->common.mi_cols,
                // logically, it should be mi_stride,
                // but it is how it was before me
                cpi->common.mi_cols,
                1,
                cpi->segmentation_map,
                TYPE_8U);
}

static void vp9_alt_ref_aq_make_map(ALT_REF_AQ* const this) {
  MATX* const segm_map = &this->segmentation_map;

  int denominator = this->segmentation_map_denominator;
  int segment_length = (denominator + 1)/this->nsegments;
  int i, j;

#ifdef ALT_REF_AQ_DEBUG
  char map_name_template[] = "files/alt_ref_quality_map%d.ppm";
  char map_name[sizeof(map_name_template) - 2 + 10];
#endif

  if (denominator <= 1)
    return;

  dbg_assert(segm_map->typeid == TYPE_8U && segm_map->cn == 1);

  ++this->alt_ref_number;

  for (i = 0; i < segm_map->rows; ++i)
    for (j = 0; j < segm_map->cols; ++j) {
      int idx = i*segm_map->stride + j;
      uint8_t elem = ((uint8_t *) segm_map->data)[idx];

      // last segment has larger range, but
      // I think it is actually what we need
      elem = elem/segment_length;
      elem -= elem/this->nsegments;

      // this is important, we want smallest
      // segments be more conservative because
      // of the way macroblocks choose them
      elem = this->nsegments - elem - 1;

      ((uint8_t *) segm_map->data)[idx] = elem;
    }

#ifdef ALT_REF_AQ_DEBUG
  sprintf(map_name, map_name_template, this->alt_ref_number);
  vp9_matx_imwrite(segm_map, map_name, this->nsegments);
#endif
}

void vp9_alt_ref_aq_go_to_next_state(ALT_REF_AQ* const this) {
  this->state = (this->state + 1)%ALTREF_AQ_NSTATES;
}

void vp9_alt_ref_aq_init(ALT_REF_AQ* const this) {
  this->state = ALTREF_AQ_BEFORE;
  this->segmentation_map_denominator = 0;

  // Usually we have just 7 lookahead buffers to blur them into
  // altref frame, it is hardly enough for two/three segments
  this->nsegments = 3;
  this->aq_mode = LOOKAHEAD_AQ;

  // This is just initiallizatoin, allocation
  // is going to happen on the first request
  vp9_matx_init(&this->segmentation_map);
  vp9_matx_init(&this->cpi_segmentation_map);
}

void vp9_alt_ref_aq_deinit(ALT_REF_AQ* const this) {
  vp9_matx_destroy(&this->segmentation_map);
  vp9_matx_destroy(&this->cpi_segmentation_map);
}

// Set basic segmentation to altref one
void vp9_alt_ref_aq_setup(ALT_REF_AQ* const this, VP9_COMP* const cpi) {
  int i;

  dbg_assert(this->state == ALTREF_AQ_BEFORE);
  vp9_alt_ref_aq_go_to_next_state(this);

  dbg_assert(this->segmentation_map.typeid == TYPE_8U);
  dbg_assert(this->segmentation_map.cn == 1);
  dbg_assert(this->segmentation_map.rows == cpi->common.mi_rows);
  dbg_assert(this->segmentation_map.cols == cpi->common.mi_cols);

  vp9_alt_ref_aq_wrap_cpi_segmentation(this, cpi);
  vp9_alt_ref_aq_make_map(this);

  // swap aq_modes
  VPX_SWAP(AQ_MODE, this->aq_mode, cpi->oxcf.aq_mode);

  // set cpi segmentation to altref one
  memset(&cpi->common.seg, 0, sizeof(struct segmentation));
  vp9_enable_segmentation(&cpi->common.seg);

  cpi->common.seg.abs_delta = SEGMENT_DELTADATA;

  for (i = 0; i < this->nsegments; ++i) {
    int qdelta = i*cpi->oxcf.alt_ref_aq;
    vp9_set_segdata(&cpi->common.seg, i, SEG_LVL_ALT_Q, qdelta);
    vp9_enable_segfeature(&cpi->common.seg, i, SEG_LVL_ALT_Q);
  }

  // set cpi segmentation map to altref one
  // (we can avoid this copy, but this way it will be easier to
  //  apply any changes to the map before setting it up)
  vp9_matx_copy_to(&this->segmentation_map, &this->cpi_segmentation_map);
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset(ALT_REF_AQ* const this, VP9_COMP* const cpi) {
  dbg_assert(this->state == ALTREF_AQ_AT);
  vp9_alt_ref_aq_go_to_next_state(this);

  VPX_SWAP(AQ_MODE, this->aq_mode, cpi->oxcf.aq_mode);

  // TODO(yuryg): I am not sure if it is better to have this inside encoder.c
  if (cpi->oxcf.aq_mode == NO_AQ) {
    vp9_alt_ref_aq_wrap_cpi_segmentation(this, cpi);
    vp9_matx_zerofill(&this->cpi_segmentation_map);

    memset(&cpi->common.seg, 0, sizeof(struct segmentation));
  }
}
