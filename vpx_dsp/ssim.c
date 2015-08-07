/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include "./vpx_dsp_rtcd.h"
#include "vpx_dsp/ssim.h"
#include "vpx_ports/mem.h"

void vpx_ssim_parms_16x16_c(const uint8_t *s, int sp, const uint8_t *r,
                            int rp, uint32_t *sum_s, uint32_t *sum_r,
                            uint32_t *sum_sq_s, uint32_t *sum_sq_r,
                            uint32_t *sum_sxr) {
  int i, j;
  for (i = 0; i < 16; i++, s += sp, r += rp) {
    for (j = 0; j < 16; j++) {
      *sum_s += s[j];
      *sum_r += r[j];
      *sum_sq_s += s[j] * s[j];
      *sum_sq_r += r[j] * r[j];
      *sum_sxr += s[j] * r[j];
    }
  }
}
void vpx_ssim_parms_8x8_c(const uint8_t *s, int sp, const uint8_t *r, int rp,
                          uint32_t *sum_s, uint32_t *sum_r,
                          uint32_t *sum_sq_s, uint32_t *sum_sq_r,
                          uint32_t *sum_sxr) {
  int i, j;
  for (i = 0; i < 8; i++, s += sp, r += rp) {
    for (j = 0; j < 8; j++) {
      *sum_s += s[j];
      *sum_r += r[j];
      *sum_sq_s += s[j] * s[j];
      *sum_sq_r += r[j] * r[j];
      *sum_sxr += s[j] * r[j];
    }
  }
}

#if CONFIG_VP9_HIGHBITDEPTH
void vpx_highbd_ssim_parms_8x8_c(const uint16_t *s, int sp,
                                 const uint16_t *r, int rp,
                                 uint32_t *sum_s, uint32_t *sum_r,
                                 uint32_t *sum_sq_s, uint32_t *sum_sq_r,
                                 uint32_t *sum_sxr) {
  int i, j;
  for (i = 0; i < 8; i++, s += sp, r += rp) {
    for (j = 0; j < 8; j++) {
      *sum_s += s[j];
      *sum_r += r[j];
      *sum_sq_s += s[j] * s[j];
      *sum_sq_r += r[j] * r[j];
      *sum_sxr += s[j] * r[j];
    }
  }
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

static const int64_t cc1 =  26634;  // (64^2*(.01*255)^2
static const int64_t cc2 = 239708;  // (64^2*(.03*255)^2

static double similarity(uint32_t sum_s, uint32_t sum_r,
                         uint32_t sum_sq_s, uint32_t sum_sq_r,
                         uint32_t sum_sxr, int count) {
  int64_t ssim_n, ssim_d;
  int64_t c1, c2;

  // scale the constants by number of pixels
  c1 = (cc1 * count * count) >> 12;
  c2 = (cc2 * count * count) >> 12;

  ssim_n = (2 * sum_s * sum_r + c1) * ((int64_t) 2 * count * sum_sxr -
                                       (int64_t) 2 * sum_s * sum_r + c2);

  ssim_d = (sum_s * sum_s + sum_r * sum_r + c1) *
           ((int64_t)count * sum_sq_s - (int64_t)sum_s * sum_s +
            (int64_t)count * sum_sq_r - (int64_t) sum_r * sum_r + c2);

  return ssim_n * 1.0 / ssim_d;
}

static double ssim_8x8(const uint8_t *s, int sp, const uint8_t *r, int rp) {
  uint32_t sum_s = 0, sum_r = 0, sum_sq_s = 0, sum_sq_r = 0, sum_sxr = 0;
  vpx_ssim_parms_8x8(s, sp, r, rp, &sum_s, &sum_r, &sum_sq_s, &sum_sq_r,
                     &sum_sxr);
  return similarity(sum_s, sum_r, sum_sq_s, sum_sq_r, sum_sxr, 64);
}

#if CONFIG_VP9_HIGHBITDEPTH
static double highbd_ssim_8x8(const uint16_t *s, int sp, const uint16_t *r, int rp,
                              unsigned int bd) {
  uint32_t sum_s = 0, sum_r = 0, sum_sq_s = 0, sum_sq_r = 0, sum_sxr = 0;
  const int oshift = bd - 8;
  vpx_highbd_ssim_parms_8x8(s, sp, r, rp, &sum_s, &sum_r, &sum_sq_s, &sum_sq_r,
                            &sum_sxr);
  return similarity(sum_s >> oshift,
                    sum_r >> oshift,
                    sum_sq_s >> (2 * oshift),
                    sum_sq_r >> (2 * oshift),
                    sum_sxr >> (2 * oshift),
                    64);
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

// We are using a 8x8 moving window with starting location of each 8x8 window
// on the 4x4 pixel grid. Such arrangement allows the windows to overlap
// block boundaries to penalize blocking artifacts.
static double vpx_ssim2(const uint8_t *img1, const uint8_t *img2,
                        int stride_img1, int stride_img2, int width,
                        int height) {
  int i, j;
  int samples = 0;
  double ssim_total = 0;

  // sample point start with each 4x4 location
  for (i = 0; i <= height - 8;
       i += 4, img1 += stride_img1 * 4, img2 += stride_img2 * 4) {
    for (j = 0; j <= width - 8; j += 4) {
      double v = ssim_8x8(img1 + j, stride_img1, img2 + j, stride_img2);
      ssim_total += v;
      samples++;
    }
  }
  ssim_total /= samples;
  return ssim_total;
}

#if CONFIG_VP9_HIGHBITDEPTH
static double vpx_highbd_ssim2(const uint8_t *img1, const uint8_t *img2,
                               int stride_img1, int stride_img2, int width,
                               int height, unsigned int bd) {
  int i, j;
  int samples = 0;
  double ssim_total = 0;

  // sample point start with each 4x4 location
  for (i = 0; i <= height - 8;
       i += 4, img1 += stride_img1 * 4, img2 += stride_img2 * 4) {
    for (j = 0; j <= width - 8; j += 4) {
      double v = highbd_ssim_8x8(CONVERT_TO_SHORTPTR(img1 + j), stride_img1,
                                 CONVERT_TO_SHORTPTR(img2 + j), stride_img2,
                                 bd);
      ssim_total += v;
      samples++;
    }
  }
  ssim_total /= samples;
  return ssim_total;
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

double vpx_calc_ssim(const YV12_BUFFER_CONFIG *source,
                     const YV12_BUFFER_CONFIG *dest,
                     double *weight) {
  double a, b, c;
  double ssimv;

  a = vpx_ssim2(source->y_buffer, dest->y_buffer,
                source->y_stride, dest->y_stride,
                source->y_crop_width, source->y_crop_height);

  b = vpx_ssim2(source->u_buffer, dest->u_buffer,
                source->uv_stride, dest->uv_stride,
                source->uv_crop_width, source->uv_crop_height);

  c = vpx_ssim2(source->v_buffer, dest->v_buffer,
                source->uv_stride, dest->uv_stride,
                source->uv_crop_width, source->uv_crop_height);

  ssimv = a * .8 + .1 * (b + c);

  *weight = 1;

  return ssimv;
}

double vpx_calc_ssimg(const YV12_BUFFER_CONFIG *source,
                      const YV12_BUFFER_CONFIG *dest,
                      double *ssim_y, double *ssim_u, double *ssim_v) {
  double ssim_all = 0;
  double a, b, c;

  a = vpx_ssim2(source->y_buffer, dest->y_buffer,
                source->y_stride, dest->y_stride,
                source->y_crop_width, source->y_crop_height);

  b = vpx_ssim2(source->u_buffer, dest->u_buffer,
                source->uv_stride, dest->uv_stride,
                source->uv_crop_width, source->uv_crop_height);

  c = vpx_ssim2(source->v_buffer, dest->v_buffer,
                source->uv_stride, dest->uv_stride,
                source->uv_crop_width, source->uv_crop_height);
  *ssim_y = a;
  *ssim_u = b;
  *ssim_v = c;
  ssim_all = (a * 4 + b + c) / 6;

  return ssim_all;
}


#if CONFIG_VP9_HIGHBITDEPTH
double vpx_highbd_calc_ssim(const YV12_BUFFER_CONFIG *source,
                            const YV12_BUFFER_CONFIG *dest,
                            double *weight, unsigned int bd) {
  double a, b, c;
  double ssimv;

  a = vpx_highbd_ssim2(source->y_buffer, dest->y_buffer,
                       source->y_stride, dest->y_stride,
                       source->y_crop_width, source->y_crop_height, bd);

  b = vpx_highbd_ssim2(source->u_buffer, dest->u_buffer,
                       source->uv_stride, dest->uv_stride,
                       source->uv_crop_width, source->uv_crop_height, bd);

  c = vpx_highbd_ssim2(source->v_buffer, dest->v_buffer,
                       source->uv_stride, dest->uv_stride,
                       source->uv_crop_width, source->uv_crop_height, bd);

  ssimv = a * .8 + .1 * (b + c);

  *weight = 1;

  return ssimv;
}

double vpx_highbd_calc_ssimg(const YV12_BUFFER_CONFIG *source,
                             const YV12_BUFFER_CONFIG *dest, double *ssim_y,
                             double *ssim_u, double *ssim_v, unsigned int bd) {
  double ssim_all = 0;
  double a, b, c;

  a = vpx_highbd_ssim2(source->y_buffer, dest->y_buffer,
                       source->y_stride, dest->y_stride,
                       source->y_crop_width, source->y_crop_height, bd);

  b = vpx_highbd_ssim2(source->u_buffer, dest->u_buffer,
                       source->uv_stride, dest->uv_stride,
                       source->uv_crop_width, source->uv_crop_height, bd);

  c = vpx_highbd_ssim2(source->v_buffer, dest->v_buffer,
                       source->uv_stride, dest->uv_stride,
                       source->uv_crop_width, source->uv_crop_height, bd);
  *ssim_y = a;
  *ssim_u = b;
  *ssim_v = c;
  ssim_all = (a * 4 + b + c) / 6;

  return ssim_all;
}
#endif  // CONFIG_VP9_HIGHBITDEPTH
