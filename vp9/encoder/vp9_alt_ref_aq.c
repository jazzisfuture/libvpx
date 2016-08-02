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
#include <math.h>

#include "vpx_ports/system_state.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_seg_common.h"
#include "vp9/common/vp9_blockd.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_segmentation.h"

#include "vp9/encoder/vp9_alt_ref_aq_private.c"
#include "vp9/encoder/vp9_alt_ref_aq.h"

/* #ifndef NDEBUG */
/* #  define ALT_REF_DEBUG_DIR "files/maps" */
/* #endif */

static void vp9_mat8u_init(struct MATX_8U *self) {
  self->rows = 0;
  self->cols = 0;
  self->stride = 0;
  self->data = NULL;
}

static void vp9_mat8u_create(struct MATX_8U *self, int rows, int cols,
                             int stride) {
  assert(stride >= cols || stride <= 0);

  if (stride <= cols) stride = cols;

  if (rows * stride != self->rows * self->stride) {
    int nbytes = rows * stride * sizeof(uint8_t);
    self->data = vpx_realloc(self->data, nbytes);
  }

  self->rows = rows;
  self->cols = cols;
  self->stride = stride;

  assert(self->data != NULL);
}

static void vp9_mat8u_destroy(struct MATX_8U *self) { vpx_free(self->data); }

static void vp9_mat8u_copy_to(const struct MATX_8U *src, struct MATX_8U *dst) {
  int i;

  if (dst->data == src->data) return;

  vp9_mat8u_create(dst, src->rows, src->cols, src->stride);

  for (i = 0; i < src->rows; ++i) {
    const uint8_t *copy_from = src->data + i * src->stride;
    uint8_t *copy_to = dst->data + i * dst->stride;

    memcpy(copy_to, copy_from, src->cols * sizeof(uint8_t));
  }
}

#ifdef ALT_REF_DEBUG_DIR

static void vp9_mat8u_imwrite(const struct MATX_8U *image, const char *filename,
                              int max_value) {
  int i, j;

  FILE *const image_file = fopen(filename, "wt");

  assert(image->rows > 0 && image->cols > 0);
  assert(image->data != NULL);

  if (max_value <= 0) max_value = 255;

  fprintf(image_file, "P2\n");
  fprintf(image_file, "%d ", image->cols);
  fprintf(image_file, "%d ", image->rows);
  fprintf(image_file, "%d ", max_value);

  for (i = 0; i < image->rows; ++i) {
    const uint8_t *row_data = &image->data[i * image->stride];

    fprintf(image_file, "\n");
    for (j = 0; j < image->cols; ++j) fprintf(image_file, "%hhu ", row_data[j]);
  }

  fclose(image_file);
}

static void vp9_alt_ref_aq_dump_debug_data(struct ALT_REF_AQ *self) {
  char format[] = ALT_REF_DEBUG_DIR "/alt_ref_quality_map%d.ppm";
  char filename[sizeof(format) / sizeof(format[0]) + 10];

  ++self->alt_ref_number;

  snprintf(filename, sizeof(filename), format, self->alt_ref_number);
  vp9_mat8u_imwrite(&self->segmentation_map, filename, self->nsegments - 1);
}

#endif

static void vp9_alt_ref_aq_update_number_of_segments(struct ALT_REF_AQ *self) {
  int i, j;

  for (i = 0, j = 0; i < self->nsegments; ++i)
    j = VPXMAX(j, self->segment_changes[i]);

  self->nsegments = j + 1;
}

static void vp9_alt_ref_aq_setup_segment_deltas(struct ALT_REF_AQ *self) {
  int i;

  vp9_zero_array(&self->segment_deltas[1], self->nsegments);

  for (i = 0; i < self->nsegments; ++i)
    ++self->segment_deltas[self->segment_changes[i] + 1];

  for (i = 2; i < self->nsegments; ++i)
    self->segment_deltas[i] += self->segment_deltas[i - 1];
}

static void vp9_alt_ref_aq_compress_segment_hist(struct ALT_REF_AQ *self) {
  int i, j;

  // join few highest segments
  for (i = 0, j = 0; i < self->nsegments; ++i) {
    if (j >= ALT_REF_AQ_NUM_NONZERO_SEGMENTS) break;

    if (self->segment_hist[i] > ALT_REF_AQ_SEGMENT_MIN_AREA) ++j;
  }

  for (; i < self->nsegments; ++i) self->segment_hist[i] = 0;

  // this always at least doesn't harm, but sometimes helps
  self->segment_hist[self->nsegments - 1] = 0;

  // invert segment ids and drop out segments with zero
  // areas, so we can save few segment numbers for other
  // stuff  and  save bitrate  on segmentation overhead
  self->segment_changes[self->nsegments] = 0;

  for (i = self->nsegments - 1, j = 0; i >= 0; --i)
    if (self->segment_hist[i] > ALT_REF_AQ_SEGMENT_MIN_AREA)
      self->segment_changes[i] = ++j;
    else
      self->segment_changes[i] = self->segment_changes[i + 1];
}

static void vp9_alt_ref_aq_update_segmentation_map(struct ALT_REF_AQ *self) {
  int i, j;

  vp9_zero_array(self->segment_hist, self->nsegments);

  for (i = 0; i < self->segmentation_map.rows; ++i) {
    uint8_t *data = self->segmentation_map.data;
    uint8_t *row_data = data + i * self->segmentation_map.stride;

    for (j = 0; j < self->segmentation_map.cols; ++j) {
      row_data[j] = self->segment_changes[row_data[j]];
      ++self->segment_hist[row_data[j]];
    }
  }
}

// set up histogram (I use it for filtering zero-area segments out)
static void vp9_alt_ref_aq_setup_histogram(struct ALT_REF_AQ *const self) {
  struct MATX_8U segm_map;
  int i, j;

  segm_map = self->segmentation_map;

  vp9_zero_array(self->segment_hist, self->nsegments);

  for (i = 0; i < segm_map.rows; ++i) {
    const uint8_t *const data = segm_map.data;
    int offset = i * segm_map.stride;

    for (j = 0; j < segm_map.cols; ++j) ++self->segment_hist[data[offset + j]];
  }
}

struct ALT_REF_AQ *vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ *self = vpx_malloc(sizeof(struct ALT_REF_AQ));

  if (self == NULL) return self;

  self->aq_mode = LOOKAHEAD_AQ;
  self->nsegments = -1;

  memset(self->segment_hist, 0, sizeof(self->segment_hist));

  // This is just initiallization, allocation
  // is going to happen on the first request
  vp9_mat8u_init(&self->segmentation_map);

  return self;
}

static void vp9_alt_ref_aq_process_map(struct ALT_REF_AQ *const self) {
  int i, j;

  assert(self->nsegments > 0);

  vp9_alt_ref_aq_setup_histogram(self);

  // super special case, e.g., riverbed sequence
  if (self->nsegments == 1) return;

  // special case: all blocks belong to the same segment,
  // and then we can just update frame's base_qindex
  for (i = 0, j = 0; i < self->nsegments; ++i)
    if (self->segment_hist[i] >= 0) ++j;

  if (j == 1) {
    for (i = 0; self->segment_hist[i] <= 0;) ++i;
    self->segment_deltas[0] = self->nsegments - i - 1;

    self->nsegments = 1;
  } else {
    vp9_alt_ref_aq_compress_segment_hist(self);
    vp9_alt_ref_aq_setup_segment_deltas(self);

    vp9_alt_ref_aq_update_number_of_segments(self);
  }

  if (self->nsegments == 1) return;

#ifdef ALT_REF_DEBUG_DIR
  vp9_alt_ref_aq_dump_debug_data(self);
#endif

  vp9_alt_ref_aq_update_segmentation_map(self);
}

static void vp9_alt_ref_aq_self_destroy(struct ALT_REF_AQ *const self) {
  vpx_free(self);
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ *const self) {
  vp9_mat8u_destroy(&self->segmentation_map);
  vp9_alt_ref_aq_self_destroy(self);
}

void vp9_alt_ref_aq_upload_map(struct ALT_REF_AQ *const self,
                               const struct MATX_8U *segmentation_map) {
  // TODO(yuryg): I don't really need this
  vp9_mat8u_copy_to(segmentation_map, &self->segmentation_map);
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
  (void)self;
  vp9_disable_segmentation(&cpi->common.seg);
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ *const self,
                              struct VP9_COMP *const cpi) {
  (void)cpi;
  (void)self;
}

int vp9_alt_ref_aq_disable_if(const struct ALT_REF_AQ *self,
                              int segmentation_overhead, int bandwidth) {
  (void)bandwidth;
  (void)self;
  (void)segmentation_overhead;

  return 0;
}
