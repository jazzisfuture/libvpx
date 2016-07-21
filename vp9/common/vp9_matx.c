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
#include <stdio.h>

#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_matx_enums.h"
#include "vp9/common/vp9_matx.h"


void vp9_matx_init(MATX_PTR const _self) {
  struct MATX* const self = (struct MATX*) _self;

  self->data = NULL;

  self->rows = 0;
  self->cols = 0;
  self->stride = 0;
  self->cn = -1;
  self->typeid = MATX_NO_TYPE;
}

void vp9_matx_destroy(MATX_PTR _self) {
  vpx_free(((struct MATX*) _self)->data);
}
