/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_ALT_REF_AQ_COMMON_H_
#define VP9_COMMON_VP9_ALT_REF_AQ_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  // encoder uses basic segmentation
  ALTREF_AQ_BEFORE,

  // encoder uses altref segmentation
  ALTREF_AQ_AT,

  // encoder uses basic segmentation,
  // but it should forcibly update it
  ALTREF_AQ_AFTER,

  // number of states
  ALTREF_AQ_NSTATES
} ALT_REF_SEG_SWAP_STATE;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_ALT_REF_AQ_COMMON_H_
