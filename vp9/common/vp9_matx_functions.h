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
 *  This file contains image processing functions for MATX class
 *
 *  TODO(yuryg): ... detailed description, may be ...
 */

#ifndef VP9_COMMON_VP9_MATX_FUNCTIONS_H_
#define VP9_COMMON_VP9_MATX_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VP9_COMMON_VP9_MATX_H_
struct MATX;
typedef struct MATX MATX;
#endif

// copy from one matx to another (reallocate if needed)
void vp9_matx_copy_to(const MATX* const src, MATX* const dst);

// fill the matrix by zeros
void vp9_matx_zerofill(MATX* const image);

// dump image in *.ppm format (assuming image is interleaved)
void vp9_matx_imwrite(const MATX* const image,
                      const char* const filename,
                      int maxval);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_MATX_FUNCTIONS_H_
