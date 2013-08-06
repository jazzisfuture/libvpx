/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_blockd.h"

#ifndef VP9_COMMON_VP9_MVREF_COMMON_H_
#define VP9_COMMON_VP9_MVREF_COMMON_H_

void vp9_find_mv_refs_idx(VP9_COMMON *cm,
                          MACROBLOCKD *xd,
                          const MODE_INFO *lf_here,
                          const MV_REFERENCE_FRAME ref_frame,
                          int_mv *mv_ref_list,
                          const int *ref_sign_bias,
                          const int block_idx,
                          const int mi_row,
                          const int mi_col);

static INLINE void vp9_find_mv_refs(VP9_COMMON *cm,
                                    MACROBLOCKD *xd,
                                    MODE_INFO *lf_here,
                                    MV_REFERENCE_FRAME ref_frame,
                                    int_mv *mv_ref_list,
                                    int *ref_sign_bias,
                                    int mi_row, int mi_col) {
  vp9_find_mv_refs_idx(cm, xd, lf_here, ref_frame,
                       mv_ref_list, ref_sign_bias, -1, mi_row, mi_col);
}

#endif  // VP9_COMMON_VP9_MVREF_COMMON_H_
