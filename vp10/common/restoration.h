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

#define BILATERAL_LEVEL_BITS_KF 4
#define BILATERAL_LEVELS_KF     (1 << BILATERAL_LEVEL_BITS_KF)
#define BILATERAL_LEVEL_BITS    3
#define BILATERAL_LEVELS        (1 << BILATERAL_LEVEL_BITS)
// #define DEF_BILATERAL_LEVEL     2

#define RESTORATION_TILETYPE_BITS    1
#define RESTORATION_TILETYPES        (1 << RESTORATION_TILETYPE_BITS)

#define BILATERAL_TILETYPE        0
#define WIENER_TILETYPE           1

#define RESTORATION_HALFWIN       3
#define RESTORATION_HALFWIN1      (RESTORATION_HALFWIN + 1)
#define RESTORATION_WIN           (2 * RESTORATION_HALFWIN + 1)
#define RESTORATION_WIN2          ((RESTORATION_WIN) * (RESTORATION_WIN))

#define RESTORATION_FILT_BITS 7
#define RESTORATION_FILT_STEP (1 << RESTORATION_FILT_BITS)

#define WIENER_FILT_TAP0_MINV     -5
#define WIENER_FILT_TAP1_MINV     (-23)
#define WIENER_FILT_TAP2_MINV     -20

#define WIENER_FILT_TAP0_BITS     4
#define WIENER_FILT_TAP1_BITS     5
#define WIENER_FILT_TAP2_BITS     6

#define WIENER_FILT_BITS \
  ((WIENER_FILT_TAP0_BITS + WIENER_FILT_TAP1_BITS + WIENER_FILT_TAP2_BITS) * 2)

#define WIENER_FILT_TAP0_MAXV \
  (WIENER_FILT_TAP0_MINV -1 + (1 << WIENER_FILT_TAP0_BITS))
#define WIENER_FILT_TAP1_MAXV \
  (WIENER_FILT_TAP1_MINV -1 + (1 << WIENER_FILT_TAP1_BITS))
#define WIENER_FILT_TAP2_MAXV \
  (WIENER_FILT_TAP2_MINV -1 + (1 << WIENER_FILT_TAP2_BITS))

typedef enum {
  RESTORE_NONE,
  RESTORE_BILATERAL,
  RESTORE_WIENER,
} RestorationType;

typedef struct {
  RestorationType restoration_type;
  // Bilateral filter
  int bilateral_tiletype;
  int bilateral_ntiles;
  int *bilateral_level;
  // Wiener filter
  int wiener_tiletype;
  int wiener_ntiles;
  int *wiener_process_tile;
  int (*vfilter)[RESTORATION_HALFWIN], (*hfilter)[RESTORATION_HALFWIN];
} RestorationInfo;

typedef struct {
  RestorationType restoration_type;
  int subsampling;
  // Bilateral filter
  int bilateral_tiletype;
  int bilateral_ntiles;
  int *bilateral_level;
  uint8_t (**wx_lut)[RESTORATION_WIN];
  uint8_t **wr_lut;
  // Wiener filter
  int wiener_tiletype;
  int wiener_ntiles;
  int *wiener_process_tile;
  int (*vfilter)[RESTORATION_WIN], (*hfilter)[RESTORATION_WIN];
} RestorationInternal;

int  vp10_bilateral_level_bits(const struct VP10Common *const cm);
int  vp10_restoration_ntiles(const struct VP10Common *const cm, int tile_type);
void vp10_restoration_tile_size(int tile_type,
                                int *tile_width, int *tile_height);
void vp10_loop_restoration_init(RestorationInternal *rst,
                                RestorationInfo *rsi, int kf);
void vp10_loop_restoration_frame(YV12_BUFFER_CONFIG *frame,
                                 struct VP10Common *cm,
                                 RestorationInfo *rsi,
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
