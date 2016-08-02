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
  memset((array), 0, (nelems) * sizeof((array)[0]));

/* #ifndef NDEBUG */
/* #  define ALT_REF_DEBUG_DIR "files/maps" */
/* #endif */

#ifdef ALT_REF_DEBUG_DIR

static void vp9_alt_ref_aq_dump_debug_data(struct ALT_REF_AQ *self) {
  char format[] = ALT_REF_DEBUG_DIR "/alt_ref_quality_map%d.ppm";
  char filename[sizeof(format) / sizeof(format[0]) + 10];

  ++self->alt_ref_number;

  snprintf(filename, sizeof(filename), format, self->alt_ref_number);
  vp9_matx_imwrite(&self->segmentation_map, filename, self->nsegments - 1);
}

#endif

static void vp9_alt_ref_aq_update_number_of_segments(struct ALT_REF_AQ *self) {
  int i, j;

  for (i = 0, j = 0; i < self->nsegments; ++i)
    j = VPXMAX(j, self->_segment_changes[i]);

  self->nsegments = j + 1;
}

static void vp9_alt_ref_aq_setup_segment_deltas(struct ALT_REF_AQ *self) {
  int i;

  array_zerofill(&self->segment_deltas[1], self->nsegments);

  for (i = 0; i < self->nsegments; ++i)
    ++self->segment_deltas[self->_segment_changes[i] + 1];

  for (i = 2; i < self->nsegments; ++i)
    self->segment_deltas[i] += self->segment_deltas[i - 1];
}

static void vp9_alt_ref_aq_compress_segment_hist(struct ALT_REF_AQ *self) {
  int i, j;

  // join few highest segments
  for (i = 0, j = 0; i < self->nsegments; ++i) {
    if (j >= self->NUM_NONZERO_SEGMENTS)  //
      break;

    if (self->_segment_hist[i] > self->SEGMENT_MIN_AREA)  //
      ++j;
  }

  for (; i < self->nsegments; ++i)  //
    self->_segment_hist[i] = 0;

  // this always makes sense
  self->_segment_hist[self->nsegments - 1] = 0;

  // invert segment ids and drop out segments with zero
  // areas, so we can save few segment numbers for other
  // staff  and  save bitrate  on segmentation overhead
  self->_segment_changes[self->nsegments] = 0;

  for (i = self->nsegments - 1, j = 0; i >= 0; --i)
    if (self->_segment_hist[i] > self->SEGMENT_MIN_AREA)
      self->_segment_changes[i] = ++j;
    else
      self->_segment_changes[i] = self->_segment_changes[i + 1];
}

static void vp9_alt_ref_aq_update_segmentation_map(struct ALT_REF_AQ *self) {
  int i, j;

  array_zerofill(self->_segment_hist, self->nsegments);

  for (i = 0; i < self->segmentation_map.rows; ++i) {
    uint8_t *data = self->segmentation_map.data;
    uint8_t *row_data = data + i * self->segmentation_map.stride;

    for (j = 0; j < self->segmentation_map.cols; ++j) {
      row_data[j] = self->_segment_changes[row_data[j]];
      ++self->_segment_hist[row_data[j]];
    }
  }
}

static void vp9_alt_ref_aq_set_single_delta(struct ALT_REF_AQ *const self,
                                            const struct VP9_COMP *cpi) {
  int total_delta = cpi->rc.avg_frame_qindex[1] - cpi->common.base_qindex;
  float ndeltas = VPXMIN(self->_nsteps, MAX_SEGMENTS - 1);

  // Don't ask me why it is (... + self->DELTA_SHRINK). It just works better.
  self->single_delta = total_delta / VPXMAX(ndeltas + self->delta_shrink, 1);
}

// set up histogram (I use it for filtering zero-area segments out)
static void vp9_alt_ref_aq_setup_histogram(struct ALT_REF_AQ *const self) {
  struct MATX_8U segm_map;
  int i, j;

  segm_map = self->segmentation_map;

  array_zerofill(self->_segment_hist, self->nsegments);

  for (i = 0; i < segm_map.rows; ++i) {
    const uint8_t *const data = segm_map.data;
    int offset = i * segm_map.stride;

    for (j = 0; j < segm_map.cols; ++j)  //
      ++self->_segment_hist[data[offset + j]];
  }
}

struct ALT_REF_AQ *vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ *self = vpx_malloc(sizeof(struct ALT_REF_AQ));

  self->aq_mode = LOOKAHEAD_AQ;

  self->nsegments = -1;
  self->_nsteps = -1;
  self->single_delta = -1;

  memset(self->_segment_hist, 0, sizeof(self->_segment_hist));

  // These parameters may have noticeable
  // influence on the quality because of the:
  //
  // 1) Possibly non-perfect representation of
  //    the all future frames with lookahead buffers
  //
  // 2) Block grid alignment during delta quantizer
  //    map estimation
  //
  // 3) Overhead for storing segmentation map
  self->NUM_NONZERO_SEGMENTS = 1;
  self->SEGMENT_MIN_AREA = 0;

  self->DELTA_SHRINK = 2.0;

  // TODO(yuryg): this may be an overfitting
  self->SINGLE_SEGMENT_DELTA_SHRINK = 1.5;

  // This is just initiallization, allocation
  // is going to happen on the first request
  vp9_matx_init(&self->segmentation_map);
  vp9_matx_init(&self->cpi_segmentation_map);

  return (struct ALT_REF_AQ *)self;
}

static void vp9_alt_ref_aq_process_map(struct ALT_REF_AQ *const self) {
  int i, j;

  self->_nsteps = self->nsegments - 1;

  vp9_alt_ref_aq_setup_histogram(self);

  assert(self->nsegments > 0);

  // super special case, e.g., riverbed sequence
  if (self->nsegments == 1) {
    self->segment_deltas[0] = 1;
    self->delta_shrink = self->SINGLE_SEGMENT_DELTA_SHRINK;
  }

  if (self->nsegments == 1)  //
    return;

  // special case: all blocks belong to the same segment,
  // and then we can just update frame's base_qindex
  for (i = 0, j = 0; i < self->nsegments; ++i)
    if (self->_segment_hist[i] >= 0) ++j;

  if (j == 1) {
    for (i = 0; self->_segment_hist[i] <= 0;) ++i;
    self->segment_deltas[0] = self->nsegments - i - 1;

    self->nsegments = 1;
    self->delta_shrink = self->SINGLE_SEGMENT_DELTA_SHRINK;
  } else {
    vp9_alt_ref_aq_compress_segment_hist(self);
    vp9_alt_ref_aq_setup_segment_deltas(self);

    vp9_alt_ref_aq_update_number_of_segments(self);
    self->delta_shrink = self->DELTA_SHRINK;
  }

  if (self->nsegments == 1)  //
    return;

#ifdef ALT_REF_DEBUG_DIR
  vp9_alt_ref_aq_dump_debug_data(self);
#endif

  vp9_alt_ref_aq_update_segmentation_map(self);
}

static void vp9_alt_ref_aq_self_destroy(struct ALT_REF_AQ *const self) {
  vpx_free(self);
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ *const self) {
  vp9_matx_destroy(&self->segmentation_map);
  vp9_matx_destroy(&self->cpi_segmentation_map);
  vp9_alt_ref_aq_self_destroy(self);
}

struct MATX_8U *vp9_alt_ref_aq_segm_map(struct ALT_REF_AQ *const self) {
  return &self->segmentation_map;
}

void vp9_alt_ref_aq_set_nsegments(struct ALT_REF_AQ *const self,
                                  int nsegments) {
  self->nsegments = nsegments;
}

void vp9_alt_ref_aq_setup_mode(struct ALT_REF_AQ *const self,
                               struct VP9_COMP *const cpi) {
  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);
  vp9_alt_ref_aq_process_map(self);
}

// set basic segmentation to the altref's one
void vp9_alt_ref_aq_setup_map(struct ALT_REF_AQ *const self,
                              struct VP9_COMP *const cpi) {
  int guard = 0, i, j;

  assert(self->segmentation_map.rows == cpi->common.mi_rows);
  assert(self->segmentation_map.cols == cpi->common.mi_cols);

  assert_gray8_image(&self->segmentation_map);

  vp9_mat8u_wrap(&self->cpi_segmentation_map,  //
                 cpi->common.mi_rows,          //
                 cpi->common.mi_cols,          //
                 cpi->common.mi_cols,          //
                 1,                            //
                 cpi->segmentation_map);

  if (self->single_delta >= 0) guard = 1;

  // clear down mmx registers (only need for x86 arch)
  vpx_clear_system_state();

  // set single quantizer step between segments
  vp9_alt_ref_aq_set_single_delta(self, cpi);

  // TODO(yuryg): this is probably not nice for rate control,
  if (self->nsegments == 1) {
    float qdelta = self->single_delta * self->segment_deltas[0];
    cpi->common.base_qindex += (int)(qdelta + 1e-2);
  }

  // TODO(yuryg): do I need this?
  if (self->nsegments == 1) vp9_mat8u_zerofill(&self->cpi_segmentation_map);

  if (self->nsegments == 1) return;

  // set cpi segmentation to altref one
  cpi->common.seg.abs_delta = SEGMENT_DELTADATA;

  vp9_enable_segmentation(&cpi->common.seg);
  vp9_clearall_segfeatures(&cpi->common.seg);

  for (i = 1, j = 1; i < self->nsegments; ++i) {
    int qdelta = (int)(self->single_delta * self->segment_deltas[i] + 1e-2f);

    vp9_enable_segfeature(&cpi->common.seg, j, SEG_LVL_ALT_Q);
    vp9_set_segdata(&cpi->common.seg, j++, SEG_LVL_ALT_Q, qdelta);
  }

  if (guard == 1) return;

  // TODO(yuryg): avoid this copy
  vp9_mat8u_copy_to(&self->segmentation_map, &self->cpi_segmentation_map);
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ *const self,
                              struct VP9_COMP *const cpi) {
  cpi->force_update_segmentation = 1;

  self->nsegments = -1;
  self->_nsteps = -1;
  self->single_delta = -1;

  VPX_SWAP(AQ_MODE, self->aq_mode, cpi->oxcf.aq_mode);

  // TODO(yuryg): may be it is better to move this to encoder.c
  if (cpi->oxcf.aq_mode == NO_AQ)  //
    vp9_disable_segmentation(&cpi->common.seg);
}
