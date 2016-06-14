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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "./vp9_matx_enums.h"
#include "vp9/common/vp9_matx.h"


#define VP9_MATX_NBYTES(type, this)                                 \
  (this)->rows*(this)->stride*sizeof(type)

#define VP9_MATX_REALLOC(type, this)                                \
  (this)->data = realloc((this)->data, VP9_MATX_NBYTES(type, this));

#define VP9_MATX_ZEROFILL(type, this)                               \
  memset((this)->data, 0, VP9_MATX_NBYTES(type, this));

#define VP9_MATX_COPYTO(type, this, dst)                            \
  memcpy((dst)->data, (this)->data, VP9_MATX_NBYTES(type, this));

#define VP9_MATX_IMWRITE(type, this, image_file, format_string)     \
{                                                                   \
  int i, j;                                                         \
                                                                    \
  type *data = (type *) this->data;                                 \
                                                                    \
  for (i = 0; i < this->rows; ++i) {                                \
    for (j = 0; j < this->cols*this->cn; ++j)                       \
      fprintf(image_file, format_string, data[i*this->stride + j]); \
                                                                    \
    fprintf(image_file, "\n");                                      \
  }                                                                 \
}

void vp9_matx_init(MATX* const this) {
  this->data = NULL;
  this->is_wrapper = false;

  this->rows = 0;
  this->cols = 0;
  this->stride = 0;
  this->cn = -1;
  this->typeid = MATX_NO_TYPE;
}

void vp9_matx_create(MATX* const this,
                     int rows,
                     int cols,
                     int stride,
                     int cn,
                     MATX_TYPE typeid) {
  bool reallocation_not_needed = true;

  if (this->rows != rows) {
    this->rows = rows;
    reallocation_not_needed = false;
  }

  if (this->cols != cols) {
    this->cols = cols;
    reallocation_not_needed = false;
  }

  if (this->stride != stride && stride != 0) {
    this->stride = stride;
    reallocation_not_needed = false;
  }

  if (!stride)
    this->stride = cols*cn;

  if (this->cn != cn || this->typeid != typeid) {
    this->typeid = typeid;
    this->cn = cn;
    reallocation_not_needed = false;
  }

  if (reallocation_not_needed)
    return;

  assert(!this->is_wrapper);
  assert(this->stride >= this->cols*this->cn);

  switch (this->typeid) {
    case TYPE_8U:
      VP9_MATX_REALLOC(uint8_t, this);
      break;
    case TYPE_8S:
      VP9_MATX_REALLOC(int8_t, this);
      break;
    case TYPE_16U:
      VP9_MATX_REALLOC(uint16_t, this);
      break;
    case TYPE_16S:
      VP9_MATX_REALLOC(int16_t, this);
      break;
    case TYPE_32U:
      VP9_MATX_REALLOC(uint32_t, this);
      break;
    case TYPE_32S:
      VP9_MATX_REALLOC(int32_t, this);
      break;
    case TYPE_32F:
      VP9_MATX_REALLOC(float, this);
      break;
    case TYPE_64F:
      VP9_MATX_REALLOC(double, this);
      break;
    default:
      assert(false /* matx: inapprorpiate type */);
  }

  assert(this->data != NULL);
}

void vp9_matx_wrap(MATX* const this,
                   int rows,
                   int cols,
                   int stride,
                   int cn,
                   void *data,
                   MATX_TYPE typeid) {
  this->rows = rows;
  this->cols = cols;
  this->cn = cn;
  this->typeid = typeid;

  this->stride = stride;
  if (!this->stride)
    this->stride = cols*cn;

  assert(this->stride >= this->cols*this->cn);

  // you can't wrap matrix around itself
  assert(this->is_wrapper || this->data != data);

  vp9_matx_destroy(this);
  this->is_wrapper = true;
  this->data = data;

  assert(this->data != NULL);
}

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(const MATX* const this, MATX* const dst) {
  vp9_matx_create(dst, this->rows, this->cols,
                  this->stride, this->cn, this->typeid);

  switch (this->typeid) {
    case TYPE_8U:
      VP9_MATX_COPYTO(uint8_t, this, dst);
      break;
    case TYPE_8S:
      VP9_MATX_COPYTO(int8_t, this, dst);
      break;
    case TYPE_16U:
      VP9_MATX_COPYTO(uint16_t, this, dst);
      break;
    case TYPE_16S:
      VP9_MATX_COPYTO(int16_t, this, dst);
      break;
    case TYPE_32U:
      VP9_MATX_COPYTO(uint32_t, this, dst);
      break;
    case TYPE_32S:
      VP9_MATX_COPYTO(int32_t, this, dst);
      break;
    case TYPE_32F:
      VP9_MATX_COPYTO(float, this, dst);
      break;
    case TYPE_64F:
      VP9_MATX_COPYTO(double, this, dst);
      break;
    default:
      assert(false /* matx: inapprorpiate type */);
  }
}

void vp9_matx_zerofill(MATX* const this) {
  switch (this->typeid) {
    case TYPE_8U:
      VP9_MATX_ZEROFILL(uint8_t, this);
      break;
    case TYPE_8S:
      VP9_MATX_ZEROFILL(int8_t, this);
      break;
    case TYPE_16U:
      VP9_MATX_ZEROFILL(uint16_t, this);
      break;
    case TYPE_16S:
      VP9_MATX_ZEROFILL(int16_t, this);
      break;
    case TYPE_32U:
      VP9_MATX_ZEROFILL(uint32_t, this);
      break;
    case TYPE_32S:
      VP9_MATX_ZEROFILL(int32_t, this);
      break;
    case TYPE_32F:
      VP9_MATX_ZEROFILL(float, this);
      break;
    case TYPE_64F:
      VP9_MATX_ZEROFILL(double, this);
      break;
    default:
      assert(false /* matx: inapprorpiate type */);
  }
}

void vp9_matx_imwrite(const MATX* const this,
                      const char* const filename,
                      int maxval) {
  FILE *const image_file = fopen(filename, "wt");

  assert(this->data != NULL);
  assert(this->cn == 1 || this->cn == 3);
  assert(this->rows > 0);
  assert(this->cols > 0);

  if (!maxval)
    maxval = 255;

  fprintf(image_file, "P%d\n", this->cn/2 + 2);
  fprintf(image_file, "%d %d %d\n", this->cols, this->rows, maxval);

  switch (this->typeid) {
    case TYPE_8U:
      VP9_MATX_IMWRITE(uint8_t,  this, image_file, "%hhu ");
      break;
    case TYPE_8S:
      VP9_MATX_IMWRITE(int8_t,   this, image_file, "%hhd ");
      break;
    case TYPE_16U:
      VP9_MATX_IMWRITE(uint16_t, this, image_file,  "%hu ");
      break;
    case TYPE_16S:
      VP9_MATX_IMWRITE(int16_t,  this, image_file,  "%hd ");
      break;
    case TYPE_32U:
      VP9_MATX_IMWRITE(uint32_t, this, image_file,   "%u ");
      break;
    case TYPE_32S:
      VP9_MATX_IMWRITE(int32_t,  this, image_file,   "%d ");
      break;
    default:
      assert(false /* matx: inapprorpiate type */);
  }

  fclose(image_file);
}

void vp9_matx_destroy(MATX* const this) {
  if (!this->data)
    return;
  if (this->is_wrapper)
    return;

  free(this->data);
}
