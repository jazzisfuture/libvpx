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

#ifndef VP9_COMMON_VP9_MATX_H_
#define VP9_COMMON_VP9_MATX_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "./vp9_matx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  int rows;           // number of rows
  int cols;           // number of columns
  int stride;         // row size in typeid's
  int cn;             // number of channels
  MATX_TYPE typeid;   // matrix type

  // true if matrix is a wrapper
  // around already existing memory
  bool is_wrapper;

  void *data;
} MATX;


void vp9_matx_init(MATX* const this);

// reallocate memory if needed
void vp9_matx_create(MATX* const this,
                     int rows,
                     int cols,
                     int stride,
                     int cn,
                     MATX_TYPE typeid);

void vp9_matx_wrap(MATX* const this,
                   int rows,
                   int cols,
                   int stride,
                   int cn,
                   void *data,
                   MATX_TYPE typeid);

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(const MATX* const this, MATX* const dst);

void vp9_matx_zerofill(MATX* const this);

// dump image in *.ppm format (assuming image is interleaved)
void vp9_matx_imwrite(const MATX* const this,
                      const char* const filename,
                      int maxval);

void vp9_matx_destroy(MATX* const this);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_H_
