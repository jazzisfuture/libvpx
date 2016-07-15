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

typedef struct MATX_8U  MATX_8U;
typedef struct MATX_8S  MATX_8S;
typedef struct MATX_16U MATX_16U;
typedef struct MATX_16S MATX_16S;
typedef struct MATX_32U MATX_32U;
typedef struct MATX_32F MATX_32F;
typedef struct MATX_64F MATX_64F;

typedef struct MATX MATX;


#define vpx_runtime_assert(expression) assert(expression)

#define VP9_MATX_NBYTES(type, self) \
    (self)->rows*(self)->stride*sizeof(type)

#define VP9_MATX_ZEROFILL(type, image)                                    \
  memset((image)->data, 0, VP9_MATX_NBYTES(type, image));

#define VP9_MATX_SET_TO(type, image, value)                               \
{                                                                         \
  int i, j;                                                               \
                                                                          \
  type *data = (type *) (image)->data;                                    \
  int stride = (image)->stride;                                           \
                                                                          \
  for (i = 0; i < (image)->rows; ++i)                                     \
    for (j = 0; j < (image)->cols*(image)->cn; ++j)                       \
      data[i*stride + j] = (type) value;                                  \
}

#define VP9_MATX_COPYTO(type, src, dst)                                   \
{                                                                         \
  int nbytes = sizeof(type)*(src)->cols, i;                               \
                                                                          \
  if ((src)->stride == (dst)->stride)                                     \
    memcpy((dst)->data, (src)->data, VP9_MATX_NBYTES(type, src));         \
                                                                          \
  for (i = 0; i < src->rows; ++i)                                         \
    memcpy((type *) (dst)->data + i*(dst)->stride,                        \
           (const type *) (src)->data + i*(src)->stride, nbytes);         \
}

#define VP9_MATX_IMWRITE(type, image, image_file, format_string)          \
{                                                                         \
  int i, j;                                                               \
                                                                          \
  type *data = (type *) (image)->data;                                    \
  int stride = (image)->stride;                                           \
                                                                          \
  for (i = 0; i < (image)->rows; ++i) {                                   \
    for (j = 0; j < (image)->cols*(image)->cn; ++j) {                     \
      fprintf(image_file, format_string, data[i*stride + j]);             \
      fprintf(image_file, " ");                                           \
  }                                                                       \
    fprintf(image_file, "\n");                                            \
  }                                                                       \
}

// reflect index beyond edges back inside [0;length]
static INLINE int reflect_idx(int idx, int length) {
  while ((unsigned) idx >= (unsigned) length) {
    if (idx < 0)
      idx = (~idx) + 1;
    else
      idx = (length << 1) - idx - 1;
  }

  return idx;
}

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(CONST_MATX_PTR _src, MATX_PTR _dst) {
  const MATX* const src = (const MATX*) _src;
  MATX* const dst = (MATX*) _dst;

  if (dst->data == src->data)
    return;

  vp9_matx_create(dst, src->rows, src->cols,
                  src->stride,
                  src->cn, src->typeid);

  switch (src->typeid) {
    case TYPE_8U:
      VP9_MATX_COPYTO(uint8_t, src, dst);
      break;
    case TYPE_8S:
      VP9_MATX_COPYTO(int8_t, src, dst);
      break;
    case TYPE_16U:
      VP9_MATX_COPYTO(uint16_t, src, dst);
      break;
    case TYPE_16S:
      VP9_MATX_COPYTO(int16_t, src, dst);
      break;
    case TYPE_32U:
      VP9_MATX_COPYTO(uint32_t, src, dst);
      break;
    case TYPE_32S:
      VP9_MATX_COPYTO(int32_t, src, dst);
      break;
    case TYPE_32F:
      VP9_MATX_COPYTO(float, src, dst);
      break;
    case TYPE_64F:
      VP9_MATX_COPYTO(double, src, dst);
      break;
    default:
      vpx_runtime_assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_zerofill(MATX_PTR _image) {
  MATX* const image = (MATX* const) _image;

  switch (image->typeid) {
    case TYPE_8U:
      VP9_MATX_ZEROFILL(uint8_t, image);
      break;
    case TYPE_8S:
      VP9_MATX_ZEROFILL(int8_t, image);
      break;
    case TYPE_16U:
      VP9_MATX_ZEROFILL(uint16_t, image);
      break;
    case TYPE_16S:
      VP9_MATX_ZEROFILL(int16_t, image);
      break;
    case TYPE_32U:
      VP9_MATX_ZEROFILL(uint32_t, image);
      break;
    case TYPE_32S:
      VP9_MATX_ZEROFILL(int32_t, image);
      break;
    case TYPE_32F:
      VP9_MATX_ZEROFILL(float, image);
      break;
    case TYPE_64F:
      VP9_MATX_ZEROFILL(double, image);
      break;
    default:
      vpx_runtime_assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_set_to(MATX_PTR _image, int value) {
  MATX* const image = (MATX* const) _image;

  switch (image->typeid) {
    case TYPE_8U:
      VP9_MATX_SET_TO(uint8_t, image, value);
      break;
    case TYPE_8S:
      VP9_MATX_SET_TO(int8_t, image, value);
      break;
    case TYPE_16U:
      VP9_MATX_SET_TO(uint16_t, image, value);
      break;
    case TYPE_16S:
      VP9_MATX_SET_TO(int16_t, image, value);
      break;
    case TYPE_32U:
      VP9_MATX_SET_TO(uint32_t, image, value);
      break;
    case TYPE_32S:
      VP9_MATX_SET_TO(int32_t, image, value);
      break;
    default:
      vpx_runtime_assert(0 /* matx: inapprorpiate type */);
  }
}

// TODO(yuryg): I think there are bugs with large radii
// -----------| Also, for the large max_values it make
// -----------| sense to use pyramidal histograms
static void vp9_mat8u_nth_element(const MATX_8U* const src,
                                  MATX_8U* const dst,
                                  int radius, int nth, int max_value) {
  // vertical (2*radius + 1)-elements histograms
  // - I reserve last chunk for the moving histogram
  int nbins = (max_value + 1)*src->cols;
  int nbytes = (nbins + (max_value + 1))*sizeof(int);

  int hist_stride = max_value + 1;
  int *hist_row = (int *) vpx_malloc(nbytes);

  int offset, i, j, k;

  assert_same_kind_of_image(src, dst);
  assert(src->cn == 1);

  assert(max_value <= 255);
  assert(hist_row != NULL);

  memset(hist_row, 0, nbins*sizeof(int));

  for (i = -(radius + 1); i < radius; ++i) {
    int offset = reflect_idx(i, src->rows)*src->stride;

    for (j = 0; j < src->cols; ++j) {
      uint8_t value = src->data[offset + j];
      ++hist_row[j*hist_stride + value];
    }
  }

  for (i = 0; i < src->rows; ++i) {
    int *hist = &hist_row[nbins];
    uint8_t *dst_row = &dst->data[i*dst->stride];

    // add row above to the histograms
    offset = reflect_idx(i + radius, src->rows)*src->stride;
    for (j = 0; j < src->cols; ++j)
      ++hist_row[j*hist_stride + src->data[offset + j]];

    // subtract row below from the histograms
    offset = reflect_idx(i - radius - 1, src->rows)*src->stride;
    for (j = 0; j < src->cols; ++j)
      --hist_row[j*hist_stride + src->data[offset + j]];

    // initialize moving histogram
    memset(hist, 0, hist_stride*sizeof(int));

    for (j = -(radius + 1); j < radius; ++j) {
      offset = reflect_idx(j, src->cols)*hist_stride;

      for (k = 0; k <= max_value; ++k)
        hist[k] += hist_row[offset + k];
    }

    // process histogram row horizontally
    for (j = 0; j < src->cols; ++j) {
      int limit = max_value, p;

      // update moving histogram on the left
      offset = reflect_idx(j - radius - 1, src->cols)*hist_stride;
      for (k = 0; k <= max_value; ++k)
        hist[k] -= hist_row[offset + k];

      // update moving histogram on the right
      offset = reflect_idx(j + radius, src->cols)*hist_stride;
      for (k = 0; k <= max_value; ++k)
        hist[k] += hist_row[offset + k];

      // prune histogram at the top
      for (; limit > 0 && hist[limit] == 0; --limit) {}

      // find target histogram bin
      for (k = -1, p = 0; k < limit && p <= nth; p += hist[++k]) {}

      dst_row[j] = k;
    }
  }

  vpx_free(hist_row);
}

// TODO(yuryg): Someone may want to implement other algorithms
// -----------| and dynamic choice of the appropriate one
void vp9_matx_nth_element(MATX_PTR _src, MATX_PTR _dst,
                          int radius, int nth, int max_value) {
  MATX* const src = (MATX* const) _src;
  MATX* dst = (MATX *) _dst;

  if (radius <= 0)
    return;

  if (dst == NULL) {
    dst = vpx_malloc(sizeof(MATX));
    vp9_matx_init(dst);
  }

  assert(src->data != dst->data);
  assert(radius < VPXMIN(src->cols, src->rows)/2);

  vp9_matx_create(dst, src->rows, src->cols,
                  src->stride,
                  src->cn, src->typeid);

  switch (src->typeid) {
    case TYPE_8U:
    {
      const MATX_8U *src8u = (const MATX_8U*) src;
      MATX_8U *dst8u = (MATX_8U*) dst;

      vp9_mat8u_nth_element(src8u, dst8u, radius, nth, max_value);
      break; /* ----------------------------------------------- */
    }
    default:
      vpx_runtime_assert(0 /* matx: inapprorpiate type */);
  }

  if (_dst == NULL) {
    vp9_matx_copy_to(dst, src);
    vp9_matx_destroy(dst);
  }
}

void vp9_matx_imwrite(CONST_MATX_PTR _image, const char* filename, int maxval) {
  const MATX* const image = (const MATX*) _image;

  FILE *const image_file = fopen(filename, "wt");
  const char *format_string = matx_type_format_strings[image->typeid];

  vpx_runtime_assert(image->data != NULL);
  vpx_runtime_assert(image->cn == 1 || image->cn == 3);
  vpx_runtime_assert(image->rows > 0);
  vpx_runtime_assert(image->cols > 0);

  if (!maxval)
    maxval = 255;

  fprintf(image_file, "P%d\n", image->cn/2 + 2);
  fprintf(image_file, "%d %d %d\n", image->cols, image->rows, maxval);

  switch (image->typeid) {
    case TYPE_8U:
      VP9_MATX_IMWRITE(uint8_t,  image, image_file, format_string);
      break;
    case TYPE_8S:
      VP9_MATX_IMWRITE(int8_t,   image, image_file, format_string);
      break;
    case TYPE_16U:
      VP9_MATX_IMWRITE(uint16_t, image, image_file, format_string);
      break;
    case TYPE_16S:
      VP9_MATX_IMWRITE(int16_t,  image, image_file, format_string);
      break;
    case TYPE_32U:
      VP9_MATX_IMWRITE(uint32_t, image, image_file, format_string);
      break;
    case TYPE_32S:
      VP9_MATX_IMWRITE(int32_t,  image, image_file, format_string);
      break;
    default:
      vpx_runtime_assert(0 /* matx: inapprorpiate type */);
  }

  fclose(image_file);
}
