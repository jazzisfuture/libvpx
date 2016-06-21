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

#include "vp9/common/vp9_matx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MATX;

#define assert_gray_image(img) \
  assert((img)->cn == 1 && (img)->typeid == TYPE_8U)

/*!\brief Copy one matx to another (reallocate if needed)
 *
 * \param    src    Source matrix
 * \param    dst    Destination matrix
 */
void vp9_matx_copy_to(const struct MATX* const src, struct MATX* const dst);

/*!\brief Fill in the matrix with zeros
 *
 * \param    image    Matrix to fill in
 */
void vp9_matx_zerofill(struct MATX* const image);

//
/*!\brief Dump the matrix in the PPM format (assuming matrix is interleaved)
 *
 * \param    image    Matrix to dump
 * \param    filename Destination filename
 * \param    maxval   Maximum value of the matrix elements (you can use -1)
 */
void vp9_matx_imwrite(const struct MATX* const image,
                      const char* const filename, int maxval);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_FUNCTIONS_H_
