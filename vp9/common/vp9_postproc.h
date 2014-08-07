/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_COMMON_VP9_POSTPROC_H_
#define VP9_COMMON_VP9_POSTPROC_H_

#include "vpx_ports/mem.h"
#include "vpx_scale/yv12config.h"
#include "vp9/common/vp9_ppflags.h"

#ifdef __cplusplus
extern "C" {
#endif

struct postproc_state {
  int last_q;
  int last_noise;
  char noise[3072];
  DECLARE_ALIGNED(16, char, blackclamp[16]);
  DECLARE_ALIGNED(16, char, whiteclamp[16]);
  DECLARE_ALIGNED(16, char, bothclamp[16]);
};

struct VP9Common;

int vp9_post_proc_frame(struct VP9Common *cm,
                        YV12_BUFFER_CONFIG *dest, vp9_ppflags_t *flags);

void vp9_denoise(const YV12_BUFFER_CONFIG *src, YV12_BUFFER_CONFIG *dst, int q);

void vp9_deblock(const YV12_BUFFER_CONFIG *src, YV12_BUFFER_CONFIG *dst, int q);

void vp9_deband(const YV12_BUFFER_CONFIG *src, YV12_BUFFER_CONFIG *dst, int q);

void vp9_unround_then_pattern_round(const uint8_t *src_ptr,
				    uint8_t *dst_ptr,
				    int src_stride,
				    int dst_stride,
				    int height,
				    int width,
				    int thresh);

void vp9_reround(const uint8_t *src_ptr,
		 uint8_t *dst_ptr,
		 int src_stride,
		 int dst_stride,
		 int height,
		 int width,
		 int thresh);

void vp9_unround(const uint8_t* src_in, uint16_t* dst_out,
		 int src_stride, int dst_stride,
		 int height, int width,
		 int thresh);

int vp9_find_BitGen_down_size(int width, int height, int depth);

void vp9_pattern_round(const uint16_t* src_in, uint8_t* dst_out,
		      int src_stride, int dst_stride,
		       int height, int width);

void vp9_normal_round(const uint16_t* src_in, uint8_t* dst_out,
		      int src_stride, int dst_stride,
		      int height, int width);


void vp9_BitGen_recur(const uint8_t* src_max,
		      const uint8_t* src_min,
		      uint16_t* src_dst_mean, 
		      uint8_t* downMax,
		      uint8_t* downMin,
		      uint16_t* downMean,
		      int max_stride,
		      int min_stride,
		      int mean_stride,
		      int depth, int height, int width,
		      int thresh, int hshift);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_POSTPROC_H_
