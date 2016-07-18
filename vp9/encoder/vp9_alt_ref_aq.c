/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

#include "vp9/encoder/vp9_encoder.h"

#include "vp9/encoder/vp9_alt_ref_aq_private.h"
#include "vp9/encoder/vp9_alt_ref_aq.h"

static void vp9_mat8u_init(struct MATX_8U *self) {
  self->rows = 0;
  self->cols = 0;
  self->stride = 0;
  self->data = NULL;
}

static void vp9_mat8u_destroy(struct MATX_8U *self) { vpx_free(self->data); }

struct ALT_REF_AQ *vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ *self = vpx_malloc(sizeof(struct ALT_REF_AQ));
  vp9_mat8u_init(&self->segmentation_map);
  return (struct ALT_REF_AQ *)self;
}

static void vp9_alt_ref_aq_self_destroy(struct ALT_REF_AQ *const self) {
  vpx_free(self);
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ *const self) {
  vp9_mat8u_destroy(&self->segmentation_map);
  vp9_alt_ref_aq_self_destroy(self);
}

struct MATX_8U *vp9_alt_ref_aq_segm_map(struct ALT_REF_AQ *const self) {
  return &self->segmentation_map;
}

void vp9_alt_ref_aq_set_nsegments(struct ALT_REF_AQ *const self,
                                  int nsegments) {
  (void)self;
  (void)nsegments;
}

void vp9_alt_ref_aq_setup_mode(struct ALT_REF_AQ *const self,
                               struct VP9_COMP *const cpi) {
  (void)cpi;
  (void)self;
}

// set basic segmentation to the altref's one
void vp9_alt_ref_aq_setup_map(struct ALT_REF_AQ *const self,
                              struct VP9_COMP *const cpi) {
  (void)cpi;
  (void)self;
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ *const self,
                              struct VP9_COMP *const cpi) {
  (void)cpi;
  (void)self;
}
