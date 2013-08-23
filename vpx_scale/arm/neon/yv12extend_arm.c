/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_scale_rtcd.h"

extern void vpx_yv12_copy_frame_func_neon(struct yv12_buffer_config *src_ybc,
                                          struct yv12_buffer_config *dst_ybc);

void vpx_yv12_copy_frame_neon(struct yv12_buffer_config *src_ybc,
                              struct yv12_buffer_config *dst_ybc) {
  vpx_yv12_copy_frame_func_neon(src_ybc, dst_ybc);

  vp8_yv12_extend_frame_borders_neon(dst_ybc);
}
