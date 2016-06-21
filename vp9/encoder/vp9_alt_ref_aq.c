/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

#include <assert.h>
#include <string.h>

#include "vpx_ports/system_state.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_seg_common.h"
#include "vp9/common/vp9_blockd.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_segmentation.h"

#include "vp9/common/vp9_matx_enums.h"
#include "vp9/common/vp9_matx.h"
#include "vp9/common/vp9_matx_functions.h"

#include "vp9/encoder/vp9_alt_ref_aq_private.h"
#include "vp9/encoder/vp9_alt_ref_aq.h"


#ifndef NDEBUG
#  define ALT_REF_DEBUG_DIR "files/maps"
#endif

// in hundredth parts of percent
#define SEGMENT_AREA_EPS 0

static void set_single_delta(struct ALT_REF_AQ* const self,
                             const struct VP9_COMP* const cpi) {
  self->single_delta = (float) cpi->oxcf.alt_ref_aq;

  if (cpi->oxcf.alt_ref_aq == -1) {
    int ndeltas = VPXMIN(self->nweights - self->zero_level, MAX_SEGMENTS);
    int total_delta  = cpi->rc.avg_frame_qindex[1] - cpi->common.base_qindex;
    self->single_delta = total_delta/(float) VPXMAX(ndeltas - 1, 1);
  }
}

struct ALT_REF_AQ* vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ* self = vpx_malloc(1*sizeof(struct ALT_REF_AQ));

  self->aq_mode = LOOKAHEAD_AQ;

  self->zero_level = -1;
  self->nweights = -1;
  self->nsegments = -1;
  self->single_delta = -1;

  memset(self->segment_hist, 0, sizeof(self->segment_hist));

  // this parameter may have noticeable influence on the quality
  // because of the overhead for storing segmentation map
  self->num_zero_segments = INT_MAX;
  self->min_nonzero_segments = 1;

  // This is just initiallizatoin, allocation
  // is going to happen on the first request
  vp9_matx_init(&self->segmentation_map);
  vp9_matx_init(&self->cpi_segmentation_map);

  return self;
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ* const self) {
  vp9_matx_destroy(&self->segmentation_map);
  vp9_matx_destroy(&self->cpi_segmentation_map);

  vpx_free(self);
}

// prepare altref segmentation map
void vp9_alt_ref_aq_make_map(struct ALT_REF_AQ* const self,
                             struct VP9_COMP* const cpi) {
  struct MATX* const segm_map = &self->segmentation_map;
  uint8_t* const data = ((uint8_t *) segm_map->data);

  const int segm_map_area = segm_map->rows*segm_map->cols;
  const int kElements = SEGMENT_AREA_EPS*segm_map_area;

  uint8_t segment_id_remap[sizeof(self->segment_hist)];

  int nbytes;
  int max_segment_id;
  int last_segment;
  int new_nsegments;
  int i, j;

#ifdef ALT_REF_DEBUG_DIR
  char map_name_template[] = ALT_REF_DEBUG_DIR"/alt_ref_quality_map%d.ppm";
  char map_name[sizeof(map_name_template) - 2 + 10];
  const int kMapNameSize = sizeof(map_name)/sizeof(char);
#endif

  assert(self->zero_level >= 0);
  assert_gray_image(segm_map);

  assert(self->min_nonzero_segments > 0);
  assert(self->num_zero_segments > 0);

  nbytes = self->nsegments*sizeof(segment_id_remap[0]);
  memset(segment_id_remap, 0, nbytes);

  // number of segments after collapsing last self->num_zero_segments
  new_nsegments = self->nsegments - self->zero_level;
  new_nsegments -= (self->num_zero_segments - 1);

  new_nsegments = VPXMAX(self->min_nonzero_segments + 1, new_nsegments);
  new_nsegments = VPXMIN(new_nsegments, MAX_SEGMENTS);

  // compress segmentation map's range
  for (i = self->zero_level, j = 0; i < self->nsegments; ++i) {
    if (i - self->zero_level >= new_nsegments && j > 1)
      break;

    if (10000LL*self->segment_hist[i] > kElements)
      ++j;
  }

  last_segment = i - 1;

  for (i = last_segment; i >= self->zero_level; --i)
    if (10000LL*self->segment_hist[i] > kElements)
      segment_id_remap[i] = (uint8_t) --j;
    else
      segment_id_remap[i] = (uint8_t) j;

  // join last self->num_zero_segments segments
  for (i = last_segment + 1; i < self->nsegments; ++i)
    segment_id_remap[i] = segment_id_remap[last_segment];

  // invert segmentation (we need it inverted because
  // of the way macroblocks choose segment id)
  max_segment_id = segment_id_remap[self->nsegments - 1];
  for (j = self->zero_level; j < self->nsegments; ++j)
    segment_id_remap[j] = max_segment_id - segment_id_remap[j];

  // set-up deltas
  nbytes = (max_segment_id + 1)*sizeof(self->segment_deltas[0]);
  memset(self->segment_deltas, 0, nbytes);

  for (i = self->zero_level; i < self->nsegments; ++i)
    ++self->segment_deltas[segment_id_remap[i] + 1];

  // update number of segments
  self->nsegments = max_segment_id + 1;

  // integrate deltas
  for (i = 1; i < self->nsegments; ++i)
    self->segment_deltas[i] += self->segment_deltas[i - 1];

  ++self->alt_ref_number;
  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);

  // if altref is just a copy of the single future frame
  if (self->nsegments == 1)
    return;

  // update segmentation map
  nbytes = self->nsegments*sizeof(self->segment_hist[0]);
  memset(self->segment_hist, 0, nbytes);

  self->zero_level = 0;

  for (i = 0; i < segm_map->rows; ++i) {
    uint8_t* row_data = data + i*segm_map->stride;

    for (j = 0; j < segm_map->cols; ++j) {
      row_data[j] = segment_id_remap[row_data[j]];
      ++self->segment_hist[row_data[j]];
    }
  }

#ifdef ALT_REF_DEBUG_DIR
  snprintf(map_name, kMapNameSize, map_name_template, self->alt_ref_number);
  vp9_matx_imwrite(segm_map, map_name, self->nsegments - 1);
#endif
}

// if current frame is altref one than set basic segmentation
// to the altref one, otherwise does nothing
void vp9_alt_ref_aq_setup(struct ALT_REF_AQ* const self,
                          struct VP9_COMP* const cpi) {
  int guard = 0, i;

  assert(self->segmentation_map.rows == cpi->common.mi_rows);
  assert(self->segmentation_map.cols == cpi->common.mi_cols);

  assert_gray_image(&self->segmentation_map);

  // TODO(yuryg): this is probably not nice for rate control
  if (self->nsegments == 1) {
    cpi->common.base_qindex = cpi->rc.avg_frame_qindex[1];
    return;
  }

  if (self->single_delta > 0)
    guard = 1;

  // clear down mmx registers (only need for x86 arch)
  vpx_clear_system_state();

  // set single quantizer step between segments
  set_single_delta(self, cpi);

  // set cpi segmentation to altref one
  cpi->common.seg.abs_delta = SEGMENT_DELTADATA;

  vp9_enable_segmentation(&cpi->common.seg);
  vp9_clearall_segfeatures(&cpi->common.seg);

  for (i = 1; i < self->nsegments; ++i) {
    int qdelta = (int) (self->single_delta*self->segment_deltas[i] + 1e-2f);

    vp9_enable_segfeature(&cpi->common.seg, i, SEG_LVL_ALT_Q);
    vp9_set_segdata(&cpi->common.seg, i, SEG_LVL_ALT_Q, qdelta);
  }

  if (guard == 1)
    return;

  vp9_matx_wrap(&self->cpi_segmentation_map,
                cpi->common.mi_rows,
                cpi->common.mi_cols,
                // logically, it should be mi_stride,
                // but it is how it was before me
                cpi->common.mi_cols,
                1,
                cpi->segmentation_map,
                TYPE_8U);

  // we can avoid this copy, but this way it seems more clear
  vp9_matx_copy_to(&self->segmentation_map, &self->cpi_segmentation_map);
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset(struct ALT_REF_AQ* const self,
                          struct VP9_COMP* const cpi) {
  cpi->force_update_segmentation = 1;

  self->nsegments = -1;
  self->nweights = -1;
  self->single_delta = -1;

  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);

  if (cpi->oxcf.aq_mode == NO_AQ && cpi->common.seg.enabled)
    vp9_disable_segmentation(&cpi->common.seg);
}
