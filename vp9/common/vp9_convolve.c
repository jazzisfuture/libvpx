/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <assert.h>

#include "./vpx_config.h"
#include "./vp9_rtcd.h"
#include "vp9/common/vp9_common.h"
#include "vpx/vpx_integer.h"

#define VP9_FILTER_WEIGHT 128
#define VP9_FILTER_SHIFT  7

static void convolve_horiz_c(const uint8_t *src, int src_stride,
                             uint8_t *dst, int dst_stride,
                             const int16_t *filter_x, int filter_x_step,
                             const int16_t *filter_y, int filter_y_step,
                             int w, int h, int taps) {
  int x, y, k, sum;

  src -= taps / 2 - 1;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      for (sum = 0, k = 0; k < taps; k++) {
        sum += src[x + k] * filter_x[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[x] = clip_pixel(sum >> VP9_FILTER_SHIFT);
      filter_x += filter_x_step;
    }
    src += src_stride;
    dst += dst_stride;
    filter_x -= filter_x_step * w;
  }
}

static void convolve_avg_horiz_c(const uint8_t *src, int src_stride,
                                 uint8_t *dst, int dst_stride,
                                 const int16_t *filter_x, int filter_x_step,
                                 const int16_t *filter_y, int filter_y_step,
                                 int w, int h, int taps) {
  int x, y, k, sum;

  src -= taps / 2 - 1;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      for (sum = 0, k = 0; k < taps; k++) {
        sum += src[x + k] * filter_x[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[x] = (dst[x] + clip_pixel(sum >> VP9_FILTER_SHIFT) + 1) >> 1;
      filter_x += filter_x_step;
    }
    src += src_stride;
    dst += dst_stride;
    filter_x -= filter_x_step * w;
  }
}

static void convolve_vert_c(const uint8_t *src, int src_stride,
                            uint8_t *dst, int dst_stride,
                            const int16_t *filter_x, int filter_x_step,
                            const int16_t *filter_y, int filter_y_step,
                            int w, int h, int taps) {
  int x, y, k, sum;

  src -= src_stride * (taps / 2 - 1);
  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      for (sum = 0, k = 0; k < taps; k++) {
        sum += src[(y + k) * src_stride] * filter_y[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[y * dst_stride] = clip_pixel(sum >> VP9_FILTER_SHIFT);
      filter_y += filter_y_step;
    }
    src++;
    dst++;
    filter_y -= filter_y_step * h;
  }
}

static void convolve_avg_vert_c(const uint8_t *src, int src_stride,
                                uint8_t *dst, int dst_stride,
                                const int16_t *filter_x, int filter_x_step,
                                const int16_t *filter_y, int filter_y_step,
                                int w, int h, int taps) {
  int x, y, k, sum;

  src -= src_stride * (taps / 2 - 1);
  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      for (sum = 0, k = 0; k < taps; k++) {
        sum += src[(y + k) * src_stride] * filter_y[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[y * dst_stride] =
          (dst[y * dst_stride] + clip_pixel(sum >> VP9_FILTER_SHIFT) + 1) >> 1;
      filter_y += filter_y_step;
    }
    src++;
    dst++;
    filter_y -= filter_y_step * h;
  }
}

static void convolve_c(const uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride,
                       const int16_t *filter_x, int filter_x_step,
                       const int16_t *filter_y, int filter_y_step,
                       int w, int h, int taps) {
  /* Fixed size intermediate buffer places limits on parameters. */
  uint8_t temp[16 * 23];
  assert(w <= 16);
  assert(h <= 16);
  assert(taps <= 8);

  convolve_horiz_c(src - src_stride * (taps / 2 - 1), src_stride,
                   temp, 16,
                   filter_x, filter_x_step, filter_y, filter_y_step,
                   w, h + taps - 1, taps);
  convolve_vert_c(temp + 16 * (taps / 2 - 1), 16, dst, dst_stride,
                  filter_x, filter_x_step, filter_y, filter_y_step,
                  w, h, taps);
}

static void convolve_avg_c(const uint8_t *src, int src_stride,
                           uint8_t *dst, int dst_stride,
                           const int16_t *filter_x, int filter_x_step,
                           const int16_t *filter_y, int filter_y_step,
                           int w, int h, int taps) {
  /* Fixed size intermediate buffer places limits on parameters. */
  uint8_t temp[16 * 23];
  assert(w <= 16);
  assert(h <= 16);
  assert(taps <= 8);

  convolve_horiz_c(src - src_stride * (taps / 2 - 1), src_stride,
                   temp, 16,
                   filter_x, filter_x_step, filter_y, filter_y_step,
                   w, h + taps - 1, taps);
  convolve_avg_vert_c(temp + 16 * (taps / 2 - 1), 16, dst, dst_stride,
                      filter_x, filter_x_step, filter_y, filter_y_step,
                      w, h, taps);
}

void vp9_convolve8_horiz_c(const uint8_t *src, int src_stride,
                           uint8_t *dst, int dst_stride,
                           const int16_t *filter_x, int filter_x_stride,
                           const int16_t *filter_y, int filter_y_stride,
                           int w, int h) {
  convolve_horiz_c(src, src_stride, dst, dst_stride,
                   filter_x, filter_x_stride, filter_y, filter_y_stride,
                   w, h, 8);
}

void vp9_convolve8_avg_horiz_c(const uint8_t *src, int src_stride,
                               uint8_t *dst, int dst_stride,
                               const int16_t *filter_x, int filter_x_stride,
                               const int16_t *filter_y, int filter_y_stride,
                               int w, int h) {
  convolve_avg_horiz_c(src, src_stride, dst, dst_stride,
                       filter_x, filter_x_stride, filter_y, filter_y_stride,
                       w, h, 8);
}

void vp9_convolve8_vert_c(const uint8_t *src, int src_stride,
                          uint8_t *dst, int dst_stride,
                          const int16_t *filter_x, int filter_x_stride,
                          const int16_t *filter_y, int filter_y_stride,
                          int w, int h) {
  convolve_vert_c(src, src_stride, dst, dst_stride,
                  filter_x, filter_x_stride, filter_y, filter_y_stride,
                  w, h, 8);
}

void vp9_convolve8_avg_vert_c(const uint8_t *src, int src_stride,
                              uint8_t *dst, int dst_stride,
                              const int16_t *filter_x, int filter_x_stride,
                              const int16_t *filter_y, int filter_y_stride,
                              int w, int h) {
  convolve_avg_vert_c(src, src_stride, dst, dst_stride,
                      filter_x, filter_x_stride, filter_y, filter_y_stride,
                      w, h, 8);
}

void vp9_convolve8_c(const uint8_t *src, int src_stride,
                     uint8_t *dst, int dst_stride,
                     const int16_t *filter_x, int filter_x_stride,
                     const int16_t *filter_y, int filter_y_stride,
                     int w, int h) {
  convolve_c(src, src_stride, dst, dst_stride,
             filter_x, filter_x_stride, filter_y, filter_y_stride,
             w, h, 8);
}

void vp9_convolve8_avg_c(const uint8_t *src, int src_stride,
                         uint8_t *dst, int dst_stride,
                         const int16_t *filter_x, int filter_x_stride,
                         const int16_t *filter_y, int filter_y_stride,
                         int w, int h) {
  convolve_avg_c(src, src_stride, dst, dst_stride,
                 filter_x, filter_x_stride, filter_y, filter_y_stride,
                 w, h, 8);
}

void vp9_convolve_copy(const uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride,
                       const int16_t *filter_x, int filter_x_stride,
                       const int16_t *filter_y, int filter_y_stride,
                       int w, int h) {
  if (h == 16) {
    vp9_copy_mem16x16(src, src_stride, dst, dst_stride);
  } else if (h == 8) {
    vp9_copy_mem8x8(src, src_stride, dst, dst_stride);
  } else if (w == 8) {
    vp9_copy_mem8x4(src, src_stride, dst, dst_stride);
  } else {
    // 4x4
    int r;

    for (r = 0; r < 4; r++) {
#if !(CONFIG_FAST_UNALIGNED)
      dst[0]  = src[0];
      dst[1]  = src[1];
      dst[2]  = src[2];
      dst[3]  = src[3];
#else
      *(uint32_t *)dst = *(const uint32_t *)src;
#endif
      src += src_stride;
      dst += dst_stride;
    }
  }
}

void vp9_convolve_avg(const uint8_t *src, int src_stride,
                      uint8_t *dst, int dst_stride,
                      const int16_t *filter_x, int filter_x_stride,
                      const int16_t *filter_y, int filter_y_stride,
                      int w, int h) {
  int x, y;

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      dst[x] = (dst[x] + src[x] + 1) >> 1;
    }
    src += src_stride;
    dst += dst_stride;
  }
}
