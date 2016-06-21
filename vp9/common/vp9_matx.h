/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

/*
 *  \file vp9_matx.h
 *
 *  This file contains generic templated matrix class
 *  designed like OpenCV Mat class,  but much simpler
 */

#ifndef VP9_COMMON_VP9_MATX_H_
#define VP9_COMMON_VP9_MATX_H_

#include "vpx/vpx_integer.h"
#include "./vp9_matx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Templated image/matrix class
 *  (inspired by OpenCV Matx class) */

struct MATX {
  int rows;           // number of rows
  int cols;           // number of columns
  int stride;         // row size in typeid's
  int cn;             // number of channels
  MATX_TYPE typeid;   // matrix type

  // true if matrix is a wrapper
  // around already existing memory
  uint8_t is_wrapper;

  void *data;
};

#define VP9_MATX_NBYTES(type, self) \
    (self)->rows*(self)->stride*sizeof(type)

/*!\brief Constructor
 *
 * \param    self    Instance of the class
 */
void vp9_matx_init(struct MATX* const self);

/*!\brief Create new matrix or update existing
 *
 * \param    self    Instance of the class
 * \param    rows    Number of rows
 * \param    cols    Number of cols
 * \param    stride  Stride in elements
 * \param    cn      Number of channels
 */
void vp9_matx_create(struct MATX* const self,
                     int rows,
                     int cols,
                     int stride,
                     int cn,
                     MATX_TYPE typeid);

/*!\brief Create a wrapper matrix around existing memory
 *
 * \param    self    Instance of the class
 * \param    rows    Number of rows
 * \param    cols    Number of cols
 * \param    stride  Stride in elements
 * \param    cn      Number of channels
 * \param    data    Pointer to the pixel data
 */
void vp9_matx_wrap(struct MATX* const self,
                   int rows,
                   int cols,
                   int stride,
                   int cn,
                   void *data,
                   MATX_TYPE typeid);

/*!\brief Destructor
 *
 * \param    self    Instance of the class
 */
void vp9_matx_destroy(struct MATX* const self);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_H_
