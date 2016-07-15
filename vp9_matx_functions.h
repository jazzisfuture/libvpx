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
 *  \file vp9_matx_functions.h
 *
 *  This file contains image processing functions for MATX class
 */

#ifndef VP9_COMMON_VP9_MATX_FUNCTIONS_H_
#define VP9_COMMON_VP9_MATX_FUNCTIONS_H_

#include <assert.h>
#include "vp9/common/vp9_matx_enums.h"


#ifdef __cplusplus
extern "C" {
#endif

#define assert_gray8_image(img) \
  assert((img)->cn == 1 && (img)->typeid == TYPE_8U)

#define assert_same_kind_of_image(img_a, img_b) \
  assert((img_a)->rows == (img_b)->rows);       \
  assert((img_a)->cols == (img_b)->cols);       \
  assert((img_a)->cn == (img_b)->cn);           \
  assert((img_a)->typeid == (img_b)->typeid)


/*!\brief Copy one matx to another (reallocate if needed)
 *
 * \param    src    Source matrix to copy
 * \param    dst    Destination matrix
 */
void vp9_matx_copy_to(CONST_MATX_PTR src, MATX_PTR dst);

/*!\brief Fill in the matrix with zeros
 *
 * \param    image    Matrix to fill in
 */
void vp9_matx_zerofill(MATX_PTR image);

/*!\brief Assign all matrix elements to value
 *
 * \param    image    Source matrix
 * \param    value    Value to assign
 */
void vp9_matx_set_to(MATX_PTR image, int value);

/*!\brief Apply square morphological dilation
 *
 * \param    src       Source matrix to dilate
 * \param    dst       Destination matrix (pass NULL to process in-place)
 * \param    radius    Radius of the filtering square-element
 *
 * \param    nth       Position in the sorted neighborhood
 *                     to extract (pass INT_MAX to extract maximum)
 *
 * \param    max_value Maximum matrix value
 */
void vp9_matx_nth_element(MATX_PTR src, MATX_PTR dst,
                          int radius, int nth, int max_value);

/*!\brief Dump the matrix in the PPM format (assuming matrix is interleaved)
 *
 * \param    image    Matrix to dump
 * \param    filename Destination filename
 * \param    maxval   Maximum value of the matrix elements (you can use -1)
 */
void vp9_matx_imwrite(CONST_MATX_PTR image, const char *filename, int maxval);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_FUNCTIONS_H_
