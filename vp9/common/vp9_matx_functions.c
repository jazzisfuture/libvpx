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

typedef int (*MATX_BORDER_FUNC)(int idx, int size);

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

void vp9_matx_imwrite(CONST_MATX_PTR const image, const char *filename,
                      int max_value) {
  switch (((const struct MATX *)image)->typeid) {
    case TYPE_8U: vp9_mat8u_imwrite(image, filename, max_value); break;
    default: assert(0 /* matx: inapprorpiate type */);
  }
}

// Reflect indices beyond edges back inside [0;size]
static VPX_INLINE int border_reflect(int idx, int size) {
  while ((unsigned)idx >= (unsigned)size)
    idx = (idx < 0) ? -idx : ((size << 1) - idx - 1);

  return idx;
}

// Clamp indices beyond edges to the corresponding edges [0;size]
static VPX_INLINE int border_repeat(int idx, int size) {
  if ((unsigned)idx < (unsigned)size) return idx;

  return VPXMIN(VPXMAX(idx, 0), size - 1);
}

void vp9_mat8u_nth_element(struct MATX_8U *src, struct MATX_8U *dst, int radius,
                           int nth, MATX_BORDER_TYPE border_type,
                           int max_value) {
  int do_in_place = 0;
  MATX_BORDER_FUNC expand_border = NULL;

  // vertical (2*radius + 1)-elements histograms
  // - I reserve last chunk for the moving histogram
  int nbins = (max_value + 1) * src->cols;
  int nbytes = (nbins + (max_value + 1)) * sizeof(int);

  int hist_stride = max_value + 1;
  int *hist_row = (int *)vpx_malloc(nbytes);

  int offset, i, j, k;

  if (radius <= 0) return;

  if (dst == NULL) {
    do_in_place = 1;
    dst = vpx_malloc(sizeof(struct MATX_8U));
    vp9_matx_init(dst);
  }

  assert(src->data != dst->data);
  assert(radius < VPXMIN(src->cols, src->rows) / 2);

  vp9_mat8u_affirm(dst, src->rows, src->cols, src->stride, src->cn);

  switch (border_type) {
    case MATX_BORDER_REFLECT: expand_border = &border_reflect; break;
    case MATX_BORDER_REPEAT: expand_border = &border_repeat; break;
    default: assert(0 /* matx-functions: unsupported border expansion*/);
  }

  assert_same_kind_of_image(src, dst);
  assert(src->cn == 1);

  assert(max_value <= 255);
  assert(hist_row != NULL);

  memset(hist_row, 0, nbins * sizeof(int));

  for (i = -(radius + 1); i < radius; ++i) {
    int offset = expand_border(i, src->rows) * src->stride;

    for (j = 0; j < src->cols; ++j) {
      uint8_t value = src->data[offset + j];
      ++hist_row[j * hist_stride + value];
    }
  }

  for (i = 0; i < src->rows; ++i) {
    int *hist = &hist_row[nbins];
    uint8_t *dst_row = &dst->data[i * dst->stride];

    // add row above to the histograms
    offset = expand_border(i + radius, src->rows) * src->stride;
    for (j = 0; j < src->cols; ++j)
      ++hist_row[j * hist_stride + src->data[offset + j]];

    // subtract row below from the histograms
    offset = expand_border(i - radius - 1, src->rows) * src->stride;
    for (j = 0; j < src->cols; ++j)
      --hist_row[j * hist_stride + src->data[offset + j]];

    // initialize moving histogram
    memset(hist, 0, hist_stride * sizeof(int));

    for (j = -(radius + 1); j < radius; ++j) {
      offset = expand_border(j, src->cols) * hist_stride;

      for (k = 0; k <= max_value; ++k) hist[k] += hist_row[offset + k];
    }

    // TODO(yuryg): I can improve performance by splitting
    // -----------| the followint cycle to near-border and
    // -----------| far-inside-the-image.

    // process histogram row horizontally
    for (j = 0; j < src->cols; ++j) {
      int limit = max_value, p;

      // update moving histogram on the left
      offset = expand_border(j - radius - 1, src->cols) * hist_stride;
      for (k = 0; k <= max_value; ++k) hist[k] -= hist_row[offset + k];

      // update moving histogram on the right
      offset = expand_border(j + radius, src->cols) * hist_stride;
      for (k = 0; k <= max_value; ++k) hist[k] += hist_row[offset + k];

      // prune histogram at the top
      for (; limit > 0 && hist[limit] == 0; --limit) {
      }

      // find target histogram bin
      for (k = -1, p = 0; k < limit && p <= nth; p += hist[++k]) {
      }

      dst_row[j] = k;
    }
  }

  vpx_free(hist_row);

  if (do_in_place == 1) {
    vp9_mat8u_copy_to(dst, src);
    vp9_matx_destroy(dst);
  }
}
