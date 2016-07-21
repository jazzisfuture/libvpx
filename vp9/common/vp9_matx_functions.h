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

// clang-format off
#define MATX_SUFFIX 8u
#include "vp9/common/vp9_matx_functions_h.def"
// clang-format on

struct MATX;

/*!\brief Copy one matx to another (reallocate if needed)
 *
 * \param    src    Source matrix to copy
 * \param    dst    Destination matrix
 */
void vp9_matx_copy_to(CONST_MATX_PTR src, MATX_PTR dst);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_FUNCTIONS_H_
