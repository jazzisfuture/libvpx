/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP8_ENCODER_DENOISING_H_
#define VP8_ENCODER_DENOISING_H_

#include "block.h"
#include "vp8/common/loopfilter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUM_DIFF_THRESHOLD (16 * 16 * 2)
#define SUM_DIFF_THRESHOLD_HIGH (16 * 16 * 3)
#define MOTION_MAGNITUDE_THRESHOLD (8*3)

#define SUM_DIFF_THRESHOLD_UV (96)   // (8 * 8 * 1.5)
#define SUM_DIFF_THRESHOLD_HIGH_UV (8 * 8 * 2)
#define SUM_DIFF_FROM_AVG_THRESH_UV (8 * 8 * 4)
#define MOTION_MAGNITUDE_THRESHOLD_UV (8*3)

enum vp8_denoiser_decision
{
  COPY_BLOCK,
  FILTER_BLOCK
};

enum vp8_denoiser_filter_state {
  kNoFilter,
  kFilterZeroMV,
  kFilterNonZeroMV
};

typedef struct {
  // Scale factor on sse threshold above which no denoising is done.
  unsigned int scale_sse_thresh;
  // Scale factor on motion magnitude threshold above which no 
  // denoising is done.
  unsigned int scale_motion_thresh;
  // Scale factor to bias to ZEROMV for denoising.
  unsigned int denoise_mv_bias;
  // Scale factor to bias to ZEROMV for coding mode selection.
  unsigned int pickmode_mv_bias;
  // Quantizer threshold below which we use the segmentation map to switch off
  // loop filter for blocks that have been coded as ZEROMV-LAST a certain number
  // (consec_zerolast) of consecutive frames. Not that the delta-QP is set to 0
  // when segmentation map is used for shutting off loop filter.
  unsigned int qp_thresh;
  // Threshold for number of consecutive frames for blocks coded as ZEROMV-LAST.
  unsigned int consec_zerolast;
  // Threshold for spatial blur, done on whole frame prior to encoding.
  // 0 means no spatial blurring is done.
  unsigned int spatial_blur;
} DENOISE_PARAMETERS;

typedef struct vp8_denoiser
{
    YV12_BUFFER_CONFIG yv12_running_avg[MAX_REF_FRAMES];
    YV12_BUFFER_CONFIG yv12_mc_running_avg;
    unsigned char* denoise_state;
    int num_mb_cols;
    int aggressive_mode;
    DENOISE_PARAMETERS denoise_pars;
} VP8_DENOISER;

int vp8_denoiser_allocate(VP8_DENOISER *denoiser, int width, int height,
                          int num_mb_rows, int num_mb_cols, int mode);

void vp8_denoiser_free(VP8_DENOISER *denoiser);

void vp8_denoiser_denoise_mb(VP8_DENOISER *denoiser,
                             MACROBLOCK *x,
                             unsigned int best_sse,
                             unsigned int zero_mv_sse,
                             int recon_yoffset,
                             int recon_uvoffset,
                             loop_filter_info_n *lfi_n,
                             int mb_row,
                             int mb_col,
                             int block_index,
                             int uv_denoise);

void vp8_denoiser_set_parameters(VP8_DENOISER *denoiser);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP8_ENCODER_DENOISING_H_
