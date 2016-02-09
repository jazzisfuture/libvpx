/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_COMMON_RESTORATION_H_
#define VP10_COMMON_RESTORATION_H_

#include "vpx_ports/mem.h"
#include "./vpx_config.h"

#include "vp10/common/blockd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RESTORATION_LEVEL_BITS_KF 4
#define RESTORATION_LEVELS_KF     (1 << RESTORATION_LEVEL_BITS_KF)
#define RESTORATION_LEVEL_BITS    3
#define RESTORATION_LEVELS        (1 << RESTORATION_LEVEL_BITS)
#define DEF_RESTORATION_LEVEL     2

#define RESTORATION_HALFWIN       3
#define RESTORATION_WIN           (2 * RESTORATION_HALFWIN + 1)

typedef struct {
  int restoration_level;
  int *wx_lut[RESTORATION_WIN];
  int *wr_lut;
} restoration_info_n;

int vp10_restoration_level_bits(const struct VP10Common *const cm);
int vp10_loop_restoration_used(int level, int kf);

void vp10_loop_restoration_init(restoration_info_n *rst, int T, int kf);
void vp10_loop_restoration_frame(YV12_BUFFER_CONFIG *frame,
                                 struct VP10Common *cm,
                                 int restoration_level,
                                 int y_only, int partial_frame);
void vp10_loop_restoration_rows(YV12_BUFFER_CONFIG *frame,
                                struct VP10Common *cm,
                                int start_mi_row, int end_mi_row,
                                int y_only);
void vp10_loop_restoration_precal();
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP10_COMMON_RESTORATION_H_
