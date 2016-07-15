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

#define _DEFINE_MATX(typesuffix)  \
struct MATX ## typesuffix {       \
  int rows;                       \
  int cols;                       \
  int stride;                     \
  int cn;                         \
  MATX_TYPE typeid;               \
                                  \
  uint8_t is_wrapper;             \
                                  \
  ELEMTYPE##typesuffix *data;     \
};

#define DEFINE_MATX(typesuffix) \
  _DEFINE_MATX(typesuffix)


/*! Templated image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX();

/*! 8bit unsigned image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_8U);

/*! 8bit signed image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_8S);

/*! 16bit unsigned image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_16U);

/*! 16bit signed image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_16S);

/*! 32bit unsigned image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_32U);

/*! 32bit signed image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_32S);

/*! 32bit floating point image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_32F);

/*! 64bit floating point image/matrix class
 *  (inspired by OpenCV Matx class) */

DEFINE_MATX(_64F);


/*!\brief Constructor
 *
 * \param    self    Instance of the class
 */
void vp9_matx_init(MATX_PTR const self);

/*!\brief Create new matrix or update existing
 *
 * \param    self    Instance of the class
 * \param    rows    Number of rows
 * \param    cols    Number of cols
 * \param    stride  Stride in elements
 * \param    cn      Number of channels
 * \param    typeid  Element type for the created matrix
 */
void vp9_matx_create(MATX_PTR const self,
                     int rows,
                     int cols,
                     int stride,
                     int cn,
                     MATX_TYPE typeid);

/*!\brief Destructor
 *
 * \param    self    Instance of the class
 */
void vp9_matx_destroy(MATX_PTR const self);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_H_
