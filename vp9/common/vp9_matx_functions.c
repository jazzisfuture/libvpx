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
#include "vp9/common/vp9_matx_functions.h"

#define MATX_SUFFIX 8u
#include "vp9/common/vp9_matx_functions.def"

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(CONST_MATX_PTR const src, MATX_PTR const dst) {
  switch (((const struct MATX *)src)->typeid) {
    case TYPE_8U: vp9_mat8u_copy_to(src, dst); break;
    default: assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_zerofill(MATX_PTR const image) {
  switch (((struct MATX *)image)->typeid) {
    case TYPE_8U: vp9_mat8u_zerofill(image); break;
    default: assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_set_to(MATX_PTR const image, int value) {
  switch (((struct MATX *)image)->typeid) {
    case TYPE_8U: vp9_mat8u_set_to(image, value); break;
    default: assert(0 /* matx: inapprorpiate type */);
  }
}
