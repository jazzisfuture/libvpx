/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vp9/common/vp9_sr_txfm.h"
#include "vp9/common/vp9_idct.h"
#include "vp9/common/vp9_idwt.h"

#if CONFIG_SR_MODE
int is_enable_srmode(BLOCK_SIZE bsize) {
  TX_SIZE max_tx_size = max_txsize_lookup[bsize];
  return (max_tx_size == TX_16X16 || max_tx_size == TX_32X32
#if CONFIG_TX64X64
    || max_tx_size == TX_64X64
#endif  // CONFIG_TX64X64
    );
}

void sr_extend(int16_t *src, int src_stride, int w, int h,
               int16_t *src_ext, int src_ext_stride, int border) {
  int16_t *src_ext_ori = src_ext;
  int i, j;

  src_ext = src_ext_ori - border;
  for (i = 0; i < h; i ++) {
    for (j = 0; j < border; j ++)
      src_ext[j] = src[0];
    vpx_memcpy(src_ext + border, src, sizeof(int16_t) * w);
    for (j = 0; j < border; j ++)
      src_ext[border + w + j] = src[w - 1];
    src_ext += src_ext_stride;
    src += src_stride;
  }

  src_ext = src_ext_ori - border * src_ext_stride - border;
  for (i = 0; i < border; i ++) 
    vpx_memcpy(src_ext + i * src_ext_stride, src_ext_ori - border,
               sizeof(int16_t) * (w + 2 * border));

  src_ext = src_ext_ori + h * src_ext_stride - border;
  for (i = 0; i < border; i ++) 
    vpx_memcpy(src_ext + i * src_ext_stride,
               src_ext_ori + (h - 1) * src_ext_stride - border,
               sizeof(int16_t) * (w + 2 * border));
}

static void convolve_horiz(int16_t *src, int src_stride, int src_offset,
                           int16_t *dst, int dst_stride, int dst_offset,
                           int *x_filter, int filter_taps, int fil_offset,
                           int w, int h) {
  int x, y;
  int round_offset = 1 << (UPSCALE_FILTER_SHIFT - 1);

  // Shift the buffer to the first pixel of filter
  src -= fil_offset;

  for (y = 0; y < h; ++y) {
    int16_t *src_x = src;
    for (x = 0; x < w; ++x) {
      int k, sum = 0;
      // If the filter is symmetric, can fold it first, then multiply
      for (k = 0; k < filter_taps; ++k)
        sum += src_x[k * src_offset] * x_filter[k];
      src_x += src_offset;
      dst[x * dst_offset] = (sum + round_offset) >> UPSCALE_FILTER_SHIFT;
    }
    src += src_stride;
    dst += dst_stride;
  }
}

static void convolve_vert(int16_t *src, int src_stride, int src_offset,
                          int16_t *dst, int dst_stride, int dst_offset,
                          int *y_filter, int filter_taps, int fil_offset,
                          int w, int h) {
  int x, y;
  int round_offset = 1 << (UPSCALE_FILTER_SHIFT - 1);

  // Shift the buffer to the first pixel of filter
  src -= src_stride * fil_offset;

  for (x = 0; x < w; ++x) {
    int16_t *src_y = src;
    for (y = 0; y < h; ++y) {
      int k, sum = 0;
      for (k = 0; k < filter_taps; ++k)
        sum += src_y[k * src_offset * src_stride] * y_filter[k];
      src_y += src_stride;
      dst[y * dst_stride] = (sum + round_offset) >> UPSCALE_FILTER_SHIFT;
    }
    src += src_offset;
    dst += dst_offset;
  }
}

int post_filter[UPSCALE_FILTER_TAPS - 1] =
    {0, -16, -10, 47, 86, 47, -10, -16, 0};
int lp_filter[UPSCALE_FILTER_TAPS - 1] = {2, -14, -2, 43, 70, 43, -2, -14, 2};

#if SR_USE_MULTI_F
//  For multiple interplation filter options
int interpl_filter[SR_USFILTER_NUM_D][UPSCALE_FILTER_TAPS] =
    {
      {4, -9, -20, 16, 73, 73, 16, -20, -9, 4},
      {1, -4, 10, -23, 80, 80, -23, 10, -4, 1}
    };
#else
int interpl_filter[UPSCALE_FILTER_TAPS] =
    {1, -4, 10, -23, 80, 80, -23, 10, -4, 1};
#endif

#define DEBUG 0
#define buf_size (64 + UPSCALE_FILTER_TAPS*2)
static void sr_convolution(int16_t *src, int src_stride, int src_offset,
                    int16_t *dst, int dst_stride, int dst_offset,
                    int * fil_hor, int * fil_ver,
                    int fil_offset_h, int fil_offset_v,
                    int filter_taps,
                    int w, int h) {
  DECLARE_ALIGNED_ARRAY(16, int16_t, tmp_buf, buf_size * buf_size);
  int tmp_buf_stride = buf_size;
  int16_t *tmp_buf_ori = tmp_buf + fil_offset_v * tmp_buf_stride + fil_offset_h;
  
  convolve_horiz(
      src - fil_offset_v * src_stride, src_stride, src_offset,
      tmp_buf_ori - fil_offset_v * tmp_buf_stride, tmp_buf_stride, 1,
      fil_hor, filter_taps, fil_offset_h, w, h + filter_taps);

  convolve_vert(
      tmp_buf_ori, tmp_buf_stride, 1,
      dst, dst_stride, dst_offset,
      fil_ver, filter_taps, fil_offset_v, w, h);
}


void sr_lowpass(int16_t *src, int src_stride, int16_t *dst, int dst_stride,
                int w, int h) {
  /*** pay attention here when changing the filters  ***/
  // filter taps of designed lowpass filter
  int filter_taps = UPSCALE_FILTER_TAPS - 1;
  // maximum pixels needs to extended
  int border = (UPSCALE_FILTER_TAPS >> 1);
  // which location in the filter corresponds to the original pixel
  int fil_offset = (filter_taps & 0x01) ? (filter_taps >> 1) :
                   (filter_taps / 2 - 1);
  /******/

  DECLARE_ALIGNED_ARRAY(16, int16_t, src_ext, buf_size * buf_size);
  int src_ext_stride = buf_size;
  int16_t *src_ext_ori = src_ext + border * src_ext_stride + border;

  // extension
  sr_extend(src, src_stride, w, h, src_ext_ori, src_ext_stride, border);
  sr_convolution(src_ext_ori, src_ext_stride, 1, dst, dst_stride, 1,
                 lp_filter, lp_filter, fil_offset, fil_offset,
                 filter_taps, w, h);
}

#if SR_USE_MULTI_F
void sr_upsample(int16_t *src, int src_stride, int16_t *dst, int dst_stride,
                 int w, int h, int f_hor, int f_ver) {
#else
void sr_upsample(int16_t *src, int src_stride, int16_t *dst, int dst_stride,
                 int w, int h) {
#endif
  /*** pay attention here when changing the filters  ***/
  int filter_taps = UPSCALE_FILTER_TAPS;
  int border = (UPSCALE_FILTER_TAPS >> 1);
  int fil_offset = (filter_taps & 0x01) ? (filter_taps >> 1) :
                   (filter_taps / 2 - 1);
  /******/

  int i, j;
  DECLARE_ALIGNED_ARRAY(16, int16_t, src_ext, buf_size * buf_size);
  DECLARE_ALIGNED_ARRAY(16, int16_t, tmp_buf, buf_size * buf_size);
  int src_ext_stride = buf_size, tmp_buf_stride = buf_size;
  int16_t *src_ext_ori = src_ext + border * src_ext_stride + border;
  int16_t *dst_ori = dst;
  int16_t *tmp_buf_ori = tmp_buf + border * tmp_buf_stride + border;

  // extension
  sr_extend(src, src_stride, w, h, src_ext_ori, src_ext_stride, border);

  // copy src to dst
  for (i = 0; i < h; i++) {
    for (j = 0; j < w; j++) {
      dst[j << 1] = src[j];
    }
    dst += (dst_stride << 1);
    src += src_stride;
  }

  convolve_horiz(src_ext_ori - border * src_ext_stride, src_ext_stride, 1,
                 tmp_buf_ori - border * tmp_buf_stride, tmp_buf_stride, 1,
#if SR_USE_MULTI_F
                 interpl_filter[f_hor], filter_taps, fil_offset,
                 w, h + (2 * border));
#else
                 interpl_filter, filter_taps, fil_offset, w, h + (2 * border));
#endif
  // copy tmp_buf to dst
  dst = dst_ori;
  tmp_buf = tmp_buf_ori;
  for (i = 0; i < h; i++) {
    for (j = 0; j < w; j++) {
      dst[(j << 1) + 1] = tmp_buf[j];
    }
    dst += (dst_stride << 1);
    tmp_buf += tmp_buf_stride;
  }


  convolve_vert(src_ext_ori, src_ext_stride, 1,
                dst_ori + dst_stride, 2 * dst_stride, 2,
#if SR_USE_MULTI_F
                interpl_filter[f_ver], filter_taps, fil_offset, w, h);
#else
                interpl_filter, filter_taps, fil_offset, w, h);
#endif
  convolve_vert(tmp_buf_ori, tmp_buf_stride, 1,
                dst_ori + dst_stride + 1, 2 * dst_stride, 2,
#if SR_USE_MULTI_F
                interpl_filter[f_ver], filter_taps, fil_offset, w, h);
#else
                interpl_filter, filter_taps, fil_offset, w, h);
#endif
}

static void sr_post_filter(int16_t *src, int src_stride,
                           int16_t *dst, int dst_stride, int w, int h) {
  /*** pay attention here when changing the filters  ***/
  int filter_taps = UPSCALE_FILTER_TAPS - 1;
  int border = (UPSCALE_FILTER_TAPS >> 1);
  int fil_offset = (filter_taps & 0x01) ? (filter_taps >> 1) :
                   (filter_taps / 2 - 1);
  /******/

  DECLARE_ALIGNED_ARRAY(16, int16_t, src_ext, buf_size * buf_size);
  int src_ext_stride = buf_size;
  int16_t *src_ext_ori = src_ext + border * src_ext_stride + border;

  // extension
  sr_extend(src, src_stride, w, h, src_ext_ori, src_ext_stride, border);
  sr_convolution(src_ext_ori, src_ext_stride, 1, dst, dst_stride, 1,
                 post_filter, post_filter, fil_offset, fil_offset,
                 filter_taps, w, h);
}

#if SR_USE_MULTI_F
void sr_recon(int16_t *src, int src_stride, uint8_t *dst, int dst_stride,
              int w, int h, int f_hor, int f_ver) {
#else
void sr_recon(int16_t *src, int src_stride, uint8_t *dst, int dst_stride,
              int w, int h) {
#endif
  DECLARE_ALIGNED_ARRAY(16, int16_t, tmp_buf, 64 * 64);
  DECLARE_ALIGNED_ARRAY(16, int16_t, tmp2_buf, 64 * 64);
  int ii, jj, tmp_stride = 64, tmp2_stride = 64;

#if SR_USE_MULTI_F
  sr_upsample(src, src_stride, tmp2_buf, tmp2_stride, w/2, h/2, f_hor, f_ver);
#else
  sr_upsample(src, src_stride, tmp2_buf, tmp2_stride, w/2, h/2);
#endif
  sr_post_filter(tmp2_buf, tmp2_stride, tmp_buf, tmp_stride, w, h);

  for (ii = 0; ii < h; ii++) {
    for (jj = 0; jj < w; jj++) {
      dst[jj] = clip_pixel_add(dst[jj], tmp_buf[jj]);
    }
    dst += dst_stride;
    tmp_buf += tmp_stride;
  }
}
#endif  // CONFIG_SR_MODE
