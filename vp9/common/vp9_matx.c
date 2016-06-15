/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 *  This file contains generic templated matrix class
 *  designed like OpenCV Mat class, but much simpler
 *
 *  TODO(yuryg): ... detailed description, may be ...
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

#include "./vp9_matx_enums.h"
#include "vp9/common/vp9_matx.h"


#define VP9_MATX_NBYTES(type, self)                                       \
  (self)->rows*(self)->stride*sizeof(type)

#define VP9_MATX_REALLOC(type, self)                                      \
  (self)->data = vpx_realloc((self)->data, VP9_MATX_NBYTES(type, self));

#define VP9_MATX_ZEROFILL(type, self)                                     \
  memset((self)->data, 0, VP9_MATX_NBYTES(type, self));

#define VP9_MATX_COPYTO(type, self, dst)                                  \
  memcpy((dst)->data, (self)->data, VP9_MATX_NBYTES(type, self));

#define VP9_MATX_IMWRITE(type, self, image_file, format_string)           \
{                                                                         \
  int i, j;                                                               \
                                                                          \
  type *data = (type *) self->data;                                       \
                                                                          \
  for (i = 0; i < self->rows; ++i) {                                      \
    for (j = 0; j < self->cols*self->cn; ++j)                             \
      fprintf(image_file, format_string, data[i*self->stride + j]);       \
                                                                          \
    fprintf(image_file, "\n");                                            \
  }                                                                       \
}

void vp9_matx_init(MATX* const self) {
  self->data = NULL;
  self->is_wrapper = 0;

  self->rows = 0;
  self->cols = 0;
  self->stride = 0;
  self->cn = -1;
  self->typeid = MATX_NO_TYPE;
}

void vp9_matx_create(MATX* const self,
                     int rows,
                     int cols,
                     int stride,
                     int cn,
                     MATX_TYPE typeid) {
  uint8_t reallocation_not_needed = 1;

  if (self->rows != rows) {
    self->rows = rows;
    reallocation_not_needed = 0;
  }

  if (self->cols != cols) {
    self->cols = cols;
    reallocation_not_needed = 0;
  }

  if (self->stride != stride && stride != 0) {
    self->stride = stride;
    reallocation_not_needed = 0;
  }

  if (!stride)
    self->stride = cols*cn;

  if (self->cn != cn || self->typeid != typeid) {
    self->typeid = typeid;
    self->cn = cn;
    reallocation_not_needed = 0;
  }

  if (reallocation_not_needed)
    return;

  assert(!self->is_wrapper);
  assert(self->stride >= self->cols*self->cn);

  switch (self->typeid) {
    case TYPE_8U:
      VP9_MATX_REALLOC(uint8_t, self);
      break;
    case TYPE_8S:
      VP9_MATX_REALLOC(int8_t, self);
      break;
    case TYPE_16U:
      VP9_MATX_REALLOC(uint16_t, self);
      break;
    case TYPE_16S:
      VP9_MATX_REALLOC(int16_t, self);
      break;
    case TYPE_32U:
      VP9_MATX_REALLOC(uint32_t, self);
      break;
    case TYPE_32S:
      VP9_MATX_REALLOC(int32_t, self);
      break;
    case TYPE_32F:
      VP9_MATX_REALLOC(float, self);
      break;
    case TYPE_64F:
      VP9_MATX_REALLOC(double, self);
      break;
    default:
      assert(0 /* matx: inapprorpiate type */);
  }

  assert(self->data != NULL);
}

void vp9_matx_wrap(MATX* const self,
                   int rows,
                   int cols,
                   int stride,
                   int cn,
                   void *data,
                   MATX_TYPE typeid) {
  self->rows = rows;
  self->cols = cols;
  self->cn = cn;
  self->typeid = typeid;

  self->stride = stride;
  if (!self->stride)
    self->stride = cols*cn;

  assert(self->stride >= self->cols*self->cn);

  // you can't wrap matrix around itself
  assert(self->is_wrapper || self->data != data);

  vp9_matx_destroy(self);
  self->is_wrapper = 1;
  self->data = data;

  assert(self->data != NULL);
}

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(const MATX* const self, MATX* const dst) {
  vp9_matx_create(dst, self->rows, self->cols,
                  self->stride, self->cn, self->typeid);

  switch (self->typeid) {
    case TYPE_8U:
      VP9_MATX_COPYTO(uint8_t, self, dst);
      break;
    case TYPE_8S:
      VP9_MATX_COPYTO(int8_t, self, dst);
      break;
    case TYPE_16U:
      VP9_MATX_COPYTO(uint16_t, self, dst);
      break;
    case TYPE_16S:
      VP9_MATX_COPYTO(int16_t, self, dst);
      break;
    case TYPE_32U:
      VP9_MATX_COPYTO(uint32_t, self, dst);
      break;
    case TYPE_32S:
      VP9_MATX_COPYTO(int32_t, self, dst);
      break;
    case TYPE_32F:
      VP9_MATX_COPYTO(float, self, dst);
      break;
    case TYPE_64F:
      VP9_MATX_COPYTO(double, self, dst);
      break;
    default:
      assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_zerofill(MATX* const self) {
  switch (self->typeid) {
    case TYPE_8U:
      VP9_MATX_ZEROFILL(uint8_t, self);
      break;
    case TYPE_8S:
      VP9_MATX_ZEROFILL(int8_t, self);
      break;
    case TYPE_16U:
      VP9_MATX_ZEROFILL(uint16_t, self);
      break;
    case TYPE_16S:
      VP9_MATX_ZEROFILL(int16_t, self);
      break;
    case TYPE_32U:
      VP9_MATX_ZEROFILL(uint32_t, self);
      break;
    case TYPE_32S:
      VP9_MATX_ZEROFILL(int32_t, self);
      break;
    case TYPE_32F:
      VP9_MATX_ZEROFILL(float, self);
      break;
    case TYPE_64F:
      VP9_MATX_ZEROFILL(double, self);
      break;
    default:
      assert(0 /* matx: inapprorpiate type */);
  }
}

void vp9_matx_imwrite(const MATX* const self,
                      const char* const filename,
                      int maxval) {
  FILE *const image_file = fopen(filename, "wt");

  assert(self->data != NULL);
  assert(self->cn == 1 || self->cn == 3);
  assert(self->rows > 0);
  assert(self->cols > 0);

  if (!maxval)
    maxval = 255;

  fprintf(image_file, "P%d\n", self->cn/2 + 2);
  fprintf(image_file, "%d %d %d\n", self->cols, self->rows, maxval);

  switch (self->typeid) {
    case TYPE_8U:
      VP9_MATX_IMWRITE(uint8_t,  self, image_file, "%hhu ");
      break;
    case TYPE_8S:
      VP9_MATX_IMWRITE(int8_t,   self, image_file, "%hhd ");
      break;
    case TYPE_16U:
      VP9_MATX_IMWRITE(uint16_t, self, image_file,  "%hu ");
      break;
    case TYPE_16S:
      VP9_MATX_IMWRITE(int16_t,  self, image_file,  "%hd ");
      break;
    case TYPE_32U:
      VP9_MATX_IMWRITE(uint32_t, self, image_file,   "%u ");
      break;
    case TYPE_32S:
      VP9_MATX_IMWRITE(int32_t,  self, image_file,   "%d ");
      break;
    default:
      assert(0 /* matx: inapprorpiate type */);
  }

  fclose(image_file);
}

void vp9_matx_destroy(MATX* const self) {
  if (!self->is_wrapper)
    vpx_free(self->data);
}
