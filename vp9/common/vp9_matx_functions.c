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

#define vpx_runtime_assert(expression) assert(expression)

#define VP9_MATX_ZEROFILL(type, image)                                    \
  memset((image)->data, 0, VP9_MATX_NBYTES(type, image));

#define VP9_MATX_COPYTO(type, src, dst)                                   \
  memcpy((dst)->data, (src)->data, VP9_MATX_NBYTES(type, src));

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

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(const struct MATX* const src,
                      struct MATX* const dst) {
  vp9_matx_create(dst, src->rows, src->cols,
                  src->stride, src->cn, src->typeid);

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

void vp9_matx_zerofill(struct MATX* const image) {
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

void vp9_matx_imwrite(const struct MATX* const image,
                      const char* const filename, int maxval) {
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
