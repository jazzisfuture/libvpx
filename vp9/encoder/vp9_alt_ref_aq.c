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


#define array_zerofill(array, nelems) \
  memset((array), 0, (nelems)*sizeof((array)[0]));

#ifndef NDEBUG
/* #  define ALT_REF_DEBUG_DIR "files/maps" */
#endif

#ifdef ALT_REF_DEBUG_DIR

static void vp9_alt_ref_aq_dump_debug_data(struct ALT_REF_AQ *self) {
  char format[] = ALT_REF_DEBUG_DIR"/alt_ref_quality_map%d.ppm";
  char filename[sizeof(format)/sizeof(char) + 10];

  ++self->alt_ref_number;

  snprintf(filename, sizeof(filename), format, self->alt_ref_number);
  vp9_matx_imwrite(&self->segmentation_map, filename, self->nsegments - 1);
}

#endif

static void vp9_alt_ref_aq_update_number_of_segments(struct ALT_REF_AQ *self) {
  int i, j;

  for (i = 0, j = 0; i < self->nsegments; ++i)
    j = VPXMAX(j, self->__segment_changes[i]);

  self->nsegments = j + 1;
}

static void vp9_alt_ref_aq_setup_segment_deltas(struct ALT_REF_AQ *self) {
  int i;

  array_zerofill(&self->segment_deltas[1], self->nsegments);

  for (i = 0; i < self->nsegments; ++i)
    ++self->segment_deltas[self->__segment_changes[i] + 1];

  for (i = 2; i < self->nsegments; ++i)
    self->segment_deltas[i] += self->segment_deltas[i - 1];
}

static void vp9_alt_ref_aq_compress_segment_hist(struct ALT_REF_AQ *self) {
  int i, j;

  // invert segment ids and drop out segments
  // with zero so we can save few segments
  // for other staff and save segmentation overhead
  for (i = self->nsegments - 1, j = 0; i >= 0; --i)
    if (self->segment_hist[i] > 0)
       self->__segment_changes[i] = j++;

  // in the case we drop out zero segment, update
  // segment changes from zero to the first non-dropped segment id
  for (i = 0; self->segment_hist[i] <= 0; ++i)
    self->__segment_changes[i] = self->__segment_changes[i] - 1;

  // I only keep it for the case of the single segment
  self->segment_deltas[0] = self->nsegments - i;

  // join several largest segments
  i = VPXMAX(self->MIN_NONZERO_SEGMENTS, i + 1);
  i = VPXMAX(self->nsegments - self->NUM_ZERO_SEGMENTS + 1, i);

  for (; i < self->nsegments; ++i) {
      int last = self->nsegments - 1;
      self->__segment_changes[i] = self->__segment_changes[last];
  }

  // compress segment id's range
  for (i = self->nsegments - 2; i >= 0; --i) {
    int previous_id = self->__segment_changes[i + 1];
    int diff = self->__segment_changes[i] - previous_id;

    self->__segment_changes[i] = previous_id + !!diff;
  }
}

static void vp9_alt_ref_aq_update_segmentation_map(struct ALT_REF_AQ *self) {
  int i, j;

  array_zerofill(self->segment_hist, self->nsegments);

  for (i = 0; i < self->segmentation_map.rows; ++i) {
    uint8_t* data = self->segmentation_map.data;
    uint8_t* row_data = data + i*self->segmentation_map.stride;

    for (j = 0; j < self->segmentation_map.cols; ++j) {
      row_data[j] = self->__segment_changes[row_data[j]];
      ++self->segment_hist[row_data[j]];
    }
  }
}

static void vp9_alt_ref_aq_set_single_delta(struct ALT_REF_AQ* const self,
                                            const struct VP9_COMP* cpi) {
  int ndeltas = VPXMIN(self->__nsteps, MAX_SEGMENTS);
  int total_delta  = cpi->rc.avg_frame_qindex[1] - cpi->common.base_qindex;

  if (cpi->oxcf.alt_ref_aq == -1)
    self->single_delta = total_delta/(float) VPXMAX(ndeltas, 1);
  else
    self->single_delta = (float) cpi->oxcf.alt_ref_aq;
}

struct ALT_REF_AQ* vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ* self = vpx_malloc(sizeof(struct ALT_REF_AQ));

  self->aq_mode = LOOKAHEAD_AQ;

  self->nsegments = -1;
  self->__nsteps = -1;
  self->single_delta = -1;

  memset(self->segment_hist, 0, sizeof(self->segment_hist));

  // these parameters may have noticeable
  // influence on the quality because
  // of the overhead for storing segmentation map
  self->NUM_ZERO_SEGMENTS = INT_MAX/2;
  self->MIN_NONZERO_SEGMENTS = 1;

  // This is just initiallization, allocation
  // is going to happen on the first request
  vp9_matx_init(&self->segmentation_map);
  vp9_matx_init(&self->cpi_segmentation_map);

  return self;
}

static void vp9_alt_ref_aq_process_map(struct ALT_REF_AQ* const self) {
  int i, j;

  assert(self->nsegments != -1);
  self->__nsteps = self->nsegments - 1;

#if 0
  vp9_matx_nth_element(&self->segmentation_map, NULL, 1, 4, self->nsegments);
  vp9_matx_nth_element(&self->segmentation_map, NULL, 1, 5, self->nsegments);
#endif

  // set up histogram (I use it for filtering zero-area segment out)
  array_zerofill(self->segment_hist, self->nsegments);

  for (i = 0; i < self->segmentation_map.rows; ++i) {
    const uint8_t* const data = self->segmentation_map.data;
    int offset = i*self->segmentation_map.stride;

    for (j = 0; j < self->segmentation_map.cols; ++j)
      ++self->segment_hist[data[offset + j]];
  }

#ifdef ALT_REF_DEBUG_DIR
  vp9_alt_ref_aq_dump_debug_data(self);
#endif

  vp9_alt_ref_aq_compress_segment_hist(self);
  vp9_alt_ref_aq_setup_segment_deltas(self);
  vp9_alt_ref_aq_update_number_of_segments(self);

  if (self->nsegments == 1)
    return;

  vp9_alt_ref_aq_update_segmentation_map(self);
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ* const self) {
  vp9_matx_destroy(&self->cpi_segmentation_map);
  vp9_matx_destroy(&self->segmentation_map);

  vpx_free(self); /* ------------------------ */
}

void vp9_alt_ref_aq_upload_map(struct ALT_REF_AQ* const self,
                               const struct MATX_8U* const segmentation_map,
                               int deep_copy) {
  assert(segmentation_map->typeid == TYPE_8U);

  if (deep_copy) {
    vp9_matx_copy_to(segmentation_map, &self->segmentation_map);
  } else {
    vp9_matx_destroy(&self->segmentation_map);
    vp9_mat8u_wrap(&self->segmentation_map,
                   segmentation_map->rows,
                   segmentation_map->cols,
                   segmentation_map->stride,
                   segmentation_map->cn,
                   segmentation_map->data);
  }

  vp9_alt_ref_aq_process_map(self);
}

void vp9_alt_ref_aq_set_nsegments(
  struct ALT_REF_AQ* const self, int nsegments) {
  self->nsegments = nsegments;
}

void vp9_alt_ref_aq_setup_mode(struct ALT_REF_AQ* const self,
                               struct VP9_COMP* const cpi) {
  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);
}

// set basic segmentation to the altref's one
void vp9_alt_ref_aq_setup_map(struct ALT_REF_AQ* const self,
                              struct VP9_COMP* const cpi) {
  int guard = 0, i, j;

  assert(self->segmentation_map.rows == cpi->common.mi_rows);
  assert(self->segmentation_map.cols == cpi->common.mi_cols);

  assert_gray8_image(&self->segmentation_map);

  if (self->single_delta >= 0)
    guard = 1;

  // clear down mmx registers (only need for x86 arch)
  vpx_clear_system_state();

  // set single quantizer step between segments
  vp9_alt_ref_aq_set_single_delta(self, cpi);

  // TODO(yuryg): this is probably not nice for rate control,
  if (self->nsegments == 1) {
    float qdelta = self->single_delta*self->segment_deltas[0];
    cpi->common.base_qindex += (int) (qdelta + 1e-2);
  }

  if (self->nsegments == 1)
    return;

  // set cpi segmentation to altref one
  cpi->common.seg.abs_delta = SEGMENT_DELTADATA;

  vp9_enable_segmentation(&cpi->common.seg);
  vp9_clearall_segfeatures(&cpi->common.seg);

  for (i = 1, j = 1; i < self->nsegments; ++i) {
    int qdelta = (int) (self->single_delta*self->segment_deltas[i] + 1e-2f);

    vp9_enable_segfeature(&cpi->common.seg, j, SEG_LVL_ALT_Q);
    vp9_set_segdata(&cpi->common.seg, j++, SEG_LVL_ALT_Q, qdelta);
  }

  if (guard == 1)
    return;

  vp9_mat8u_wrap(&self->cpi_segmentation_map,
                 cpi->common.mi_rows,
                 cpi->common.mi_cols,
                 // logically, it should be mi_stride,
                 // but it is how it was before me
                 cpi->common.mi_cols,
                 1,
                 cpi->segmentation_map);

  // TODO(yuryg): avoid this copy
  vp9_matx_copy_to(&self->segmentation_map, &self->cpi_segmentation_map);
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ* const self,
                              struct VP9_COMP* const cpi) {
  cpi->force_update_segmentation = 1;

  self->nsegments = -1;
  self->__nsteps = -1;
  self->single_delta = -1;

  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);

  // TODO(yuryg): may be it is better to move this to encoder.c
  if (cpi->oxcf.aq_mode == NO_AQ)
    vp9_disable_segmentation(&cpi->common.seg);
}
