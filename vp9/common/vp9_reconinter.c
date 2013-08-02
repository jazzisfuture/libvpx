/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_filter.h"
#include "vp9/common/vp9_reconinter.h"
#include "vp9/common/vp9_reconintra.h"
#include "./vpx_scale_rtcd.h"

#if CONFIG_AFFINE_MP
#define SP(x) (((x) & 7) << 4)
int sse_block(uint8_t *src, int src_stride, uint8_t *pred_ptr,
              int pred_stride, int rows, int cols) {
  int sse = 0, r_idx, c_idx, temp;
  for (r_idx = 0; r_idx < rows; r_idx++) {
    for (c_idx = 0; c_idx < cols; c_idx++) {
      temp = (src[c_idx] - pred_ptr[c_idx]) * (src[c_idx] - pred_ptr[c_idx]);
      sse = sse + temp;
    }
    src += src_stride;
    pred_ptr += pred_stride;
  }

  return sse;
}

void template_matching(uint8_t *src, int src_stride,
                       uint8_t *dst, int dst_stride,
                       int bh, int bw,
                       int rows, int cols,
                       int *best_r, int *best_c) {
  int r_idx, c_idx;
  int best_error = bh * bw * 256 * 256, this_error;

  *best_r = *best_c = 0;

  for (r_idx = 0; r_idx < rows; r_idx++) {
    for (c_idx = 0; c_idx < cols; c_idx++) {
      this_error = sse_block(src, src_stride, dst + c_idx, dst_stride,
                           bh, bw);
      if (this_error < best_error) {
        best_error = this_error;
        *best_r = r_idx;
        *best_c = c_idx;
      }
    }
    dst += dst_stride;
  }
}

void affine_estimation(int *mv_x, int *mv_y, int n, float *coeff) {
  // coeff 0 1 2 3 4 5
  //       a b c d e f
  // point 1: (0, 0);
  // point 2: (0, n);
  // point 3: (n, 0);

  coeff[2] = ((float) mv_x[0]) / 8;
  coeff[5] = ((float) mv_y[0]) / 8;

  coeff[1] = (((float) mv_x[1]) / 8 - coeff[2]) / n;
  coeff[4] = (((float) mv_y[1]) / 8 - coeff[5]) / n;

  coeff[0] = (((float) mv_x[2]) / 8 - coeff[2]) / n;
  coeff[3] = (((float) mv_y[2]) / 8 - coeff[5]) / n;
}

uint8_t bil_interp(float *xp, float *yp, uint8_t *src, int src_stride) {
  int x_int, y_int;
  float value, x = *xp, y = *yp;

  x_int = floor(x);
  y_int = floor(y);
  value = (y_int + 1 - y) * (x_int + 1 - x) * src[src_stride * y_int + x_int] +
      (y_int + 1 - y) * (x - x_int) * src[src_stride * y_int + x_int + 1] +
      (y - y_int) * (x_int + 1 - x) * src[src_stride * (y_int + 1) + x_int] +
      (y - y_int) * (x - x_int) * src[src_stride * (y_int + 1) + x_int + 1];

  return (uint8_t) value;
}

void sub_pixel_filtering_h(uint8_t *src, int src_stride,
                           uint8_t *dst, int dst_stride,
                           int rows, int cols, int *filter) {
  int i, j;

  for (i = 0; i < rows + 1; i++) {
    for (j = 0; j < cols; j++) {
      // Apply bilinear filter
      dst[j] = (((int) src[j] * filter[0]) +  ((int) src[j + 1] * filter[1]) +
          (128 / 2)) >> 7;
    }
    src += src_stride;
    dst += dst_stride;
  }
}

void sub_pixel_filtering_v(uint8_t *src, int src_stride,
                           uint8_t *dst, int dst_stride,
                           int rows, int cols, int *filter) {
  int i, j;

  for (j = 0; j < rows; j++) {
    for (i = 0; i < cols; i++) {
      // Apply bilinear filter
      dst[i* dst_stride] = ((((int) src[i * src_stride]) * filter[0]) +
          (((int) src[(i + 1) * src_stride]) * filter[1]) + (128 / 2)) >> 7;
    }
    src += 1;
    dst += 1;
  }
}

void sub_pixel_adjust(MACROBLOCKD *xd, uint8_t *z, uint8_t *y,
                         int bh, int bw, int_mv startmv, int_mv *bestmv) {
  int left, right, up, down, diag;
  int best_sad;
  int whichdir;
  int y_stride = xd->plane[0].pre[0].stride;
  int z_stride = xd->plane[0].dst.stride;
  int filter[2];
  uint8_t temp2[256], temp[256];
  int_mv this_mv;

  startmv.as_mv.row = startmv.as_mv.row << 3;
  startmv.as_mv.col = startmv.as_mv.col << 3;
  *bestmv = startmv;
  best_sad = sse_block(z, z_stride, y, y_stride, bh, bw);

  filter[0] = 64, filter[1] = 64;

  // go left
  this_mv.as_mv.row = startmv.as_mv.row;
  this_mv.as_mv.col = ((startmv.as_mv.col - 8) | 4);
  sub_pixel_filtering_h(y-1, y_stride, temp, bw, bh, bw, filter);
  left = sse_block(z, z_stride, temp, bw, bh, bw);
  if (left < best_sad) {
    *bestmv = this_mv;
    best_sad = left;
  }

  // go right
  this_mv.as_mv.row = startmv.as_mv.row;
  this_mv.as_mv.col += 8;
  sub_pixel_filtering_h(y, y_stride, temp, bw, bh, bw, filter);
  right = sse_block(z, z_stride, temp, bw, bh, bw);
  if (right < best_sad) {
    *bestmv = this_mv;
    best_sad = right;
  }

  // go up
  this_mv.as_mv.col = startmv.as_mv.col;
  this_mv.as_mv.row = ((startmv.as_mv.row - 8) | 4);
  sub_pixel_filtering_v(y - y_stride, y_stride, temp, bw, bh, bw, filter);
  up = sse_block(z, z_stride, temp, bw, bh, bw);
  if (up < best_sad) {
    *bestmv = this_mv;
    best_sad = up;
  }

  // go down
  this_mv.as_mv.row += 8;
  sub_pixel_filtering_v(y, y_stride, temp, bw, bh, bw, filter);
  down = sse_block(z, z_stride, temp, bw, bh, bw);
  if (down < best_sad) {
    *bestmv = this_mv;
    best_sad = down;
  }

  whichdir = (left < right ? 0 : 1) + (up < down ? 0 : 2);
  this_mv = startmv;

  switch (whichdir) {
    case 0:
      this_mv.as_mv.col = (this_mv.as_mv.col - 8) | 4;
      this_mv.as_mv.row = (this_mv.as_mv.row - 8) | 4;
      sub_pixel_filtering_h(y - y_stride - 1, y_stride, temp2,
                            bw, bh, bw, filter);
      sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      diag = sse_block(z, z_stride, temp, bw, bh, bw);
      break;
    case 1:
      this_mv.as_mv.col += 4;
      this_mv.as_mv.row = (this_mv.as_mv.row - 8) | 4;
      sub_pixel_filtering_h(y - y_stride, y_stride, temp2, bw, bh, bw, filter);
      sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      diag = sse_block(z, z_stride, temp, bw, bh, bw);
      break;
    case 2:
      this_mv.as_mv.col = (this_mv.as_mv.col - 8) | 4;
      this_mv.as_mv.row += 4;
      sub_pixel_filtering_h(y - 1, y_stride, temp2, bw, bh, bw, filter);
      sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      diag = sse_block(z, z_stride, temp, bw, bh, bw);
      break;
    case 3:
    default:
      this_mv.as_mv.col += 4;
      this_mv.as_mv.row += 4;
      sub_pixel_filtering_h(y, y_stride, temp2, bw, bh, bw, filter);
      sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      diag = sse_block(z, z_stride, temp, bw, bh, bw);
      break;
  }
  if (diag < best_sad) {
    *bestmv = this_mv;
    best_sad = diag;
  }

  if (bestmv->as_mv.row < startmv.as_mv.row) {
    y -= y_stride;
  }
  if (bestmv->as_mv.col < startmv.as_mv.col) {
    y--;
  }

  startmv = *bestmv;

  this_mv.as_mv.row = startmv.as_mv.row;
  // left
  if (startmv.as_mv.col & 7) {
    this_mv.as_mv.col = startmv.as_mv.col - 2;
    filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
    sub_pixel_filtering_h(y, y_stride, temp, bw, bh, bw, filter);
  } else {
    this_mv.as_mv.col = (startmv.as_mv.col - 8) | 6;
    filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
    sub_pixel_filtering_h(y - 1, y_stride, temp, bw, bh, bw, filter);
  }
  left = sse_block(z, z_stride, temp, bw, bh, bw);
  if (left < best_sad) {
    *bestmv = this_mv;
    best_sad = left;
  }

  this_mv.as_mv.col += 4;
  filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
  sub_pixel_filtering_h(y, y_stride, temp, bw, bh, bw, filter);
  right = sse_block(z, z_stride, temp, bw, bh, bw);
  if (right < best_sad) {
    *bestmv = this_mv;
    best_sad = right;
  }

  this_mv.as_mv.col = startmv.as_mv.col;
  if (startmv.as_mv.row & 7) {
    this_mv.as_mv.row = startmv.as_mv.row - 2;
    filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
    sub_pixel_filtering_v(y, y_stride, temp, bw, bh, bw, filter);
  } else {
    this_mv.as_mv.row = (startmv.as_mv.row - 8) | 6;
    filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
    sub_pixel_filtering_v(y - y_stride, y_stride, temp, bw, bh, bw, filter);
  }
  up = sse_block(z, z_stride, temp, bw, bh, bw);
  if (up < best_sad) {
    *bestmv = this_mv;
    best_sad = up;
  }

  this_mv.as_mv.row += 4;
  filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
  sub_pixel_filtering_v(y, y_stride, temp, bw, bh, bw, filter);
  down = sse_block(z, z_stride, temp, bw, bh, bw);
  if (down < best_sad) {
    *bestmv = this_mv;
    best_sad = down;
  }

  whichdir = (left < right ? 0 : 1) + (up < down ? 0 : 2);
  this_mv = startmv;
  switch (whichdir) {
    case 0:

      if (startmv.as_mv.row & 7) {
        this_mv.as_mv.row -= 2;
        if (startmv.as_mv.col & 7) {
          this_mv.as_mv.col -= 2;
          filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
          sub_pixel_filtering_h(y, y_stride, temp2, bw, bh, bw, filter);
          filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
          sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
        } else {
          this_mv.as_mv.col = (startmv.as_mv.col - 8) | 6;
          filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
          sub_pixel_filtering_h(y - 1, y_stride, temp2, bw, bh, bw, filter);
          filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
          sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
        }
      } else {
        this_mv.as_mv.row = (startmv.as_mv.row - 8) | 6;
        if (startmv.as_mv.col & 7) {
          this_mv.as_mv.col -= 2;
          filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
          sub_pixel_filtering_h(y - y_stride, y_stride, temp2,
                                bw, bh, bw, filter);
          filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
          sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
        } else {
          this_mv.as_mv.col = (startmv.as_mv.col - 8) | 6;
          filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
          sub_pixel_filtering_h(y - y_stride - 1, y_stride, temp2,
                                bw, bh, bw, filter);
          sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
        }
      }
      break;
    case 1:
      this_mv.as_mv.col += 2;
      if (startmv.as_mv.row & 7) {
        this_mv.as_mv.row -= 2;
        filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
        sub_pixel_filtering_h(y, y_stride, temp2, bw, bh, bw, filter);
        filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
        sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      } else {
        this_mv.as_mv.row = (startmv.as_mv.row - 8) | 6;
        filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
        sub_pixel_filtering_h(y - y_stride, y_stride,
                              temp2, bw, bh, bw, filter);
        filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
        sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      }
      break;
    case 2:
      this_mv.as_mv.row += 2;

      if (startmv.as_mv.col & 7) {
        this_mv.as_mv.col -= 2;
        filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
        sub_pixel_filtering_h(y, y_stride, temp2, bw, bh, bw, filter);
        filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
        sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      } else {
        this_mv.as_mv.col = (startmv.as_mv.col - 8) | 6;
        filter[0] = 128 - SP(6), filter[1] = 128 - filter[0];
        sub_pixel_filtering_h(y - 1, y_stride, temp2, bw, bh, bw, filter);
        filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
        sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      }
      break;
    case 3:
      this_mv.as_mv.col += 2;
      this_mv.as_mv.row += 2;
      filter[0] = 128 - SP(this_mv.as_mv.col), filter[1] = 128 - filter[0];
      sub_pixel_filtering_h(y, y_stride, temp2, bw, bh, bw, filter);
      filter[0] = 128 - SP(this_mv.as_mv.row), filter[1] = 128 - filter[0];
      sub_pixel_filtering_v(temp2, bw, temp, bw, bh, bw, filter);
      break;
  }
  diag = sse_block(z, z_stride, temp, bw, bh, bw);
  if (diag < best_sad) {
    *bestmv = this_mv;
    best_sad = diag;
  }
}

void affine_motion_prediciton(MACROBLOCKD *xd, int n, int m, int t,
                                 int mi_row, int mi_col,
                                 int mi_rows, int mi_cols,
                                 int shift_mi_u, int shift_mi_l) {
  int i, j;
  int mv_x[3], mv_y[3], plane;
  int l = 2 * m + 1, h = 2 * t + 1;
  int ur, uc, lr, lc;
  float aff_coeff[6];
  float x_f, y_f;
  uint8_t *temp_dst, *temp_pre;
  int_mv mv1, mv2, mv3;
  int_mv temp_mv;

  assert(n > l);

  if (mi_row >= shift_mi_u)
    ur = shift_mi_u << 3;
  else
    ur = mi_row << 3;

  if (mi_col >= shift_mi_u)
    uc = shift_mi_u << 3;
  else
    uc = mi_col << 3;

  if (mi_row + shift_mi_l < mi_rows)
    lr = (shift_mi_l << 3) + 8 - l;
  else
    lr = ((mi_rows - 1 - mi_row) << 3) + 8 - l;

  if (mi_col + shift_mi_l < mi_cols)
    lc = (shift_mi_l << 3) + 8 - l;
  else
    lc = ((mi_cols - 1 - mi_col) << 3) + 8 - l;

  temp_pre = xd->plane[0].pre[0].buf - ur * xd->plane[0].pre[0].stride - uc;

  temp_dst = xd->plane[0].dst.buf - l * xd->plane[0].dst.stride - l;
  template_matching(temp_dst, xd->plane[0].dst.stride,
                       temp_pre, xd->plane[0].pre[0].stride,
                       l, l, ur + lr, uc + lc, &mv_y[0], &mv_x[0]);
  mv1.as_mv.row = mv_y[0];
  mv1.as_mv.col = mv_x[0];
  sub_pixel_adjust(xd, temp_dst,
                      temp_pre + mv_y[0] * xd->plane[0].pre[0].stride + mv_x[0],
                      l, l, mv1, &temp_mv);
  mv1 = temp_mv;

  temp_dst = xd->plane[0].dst.buf + (n - h) * xd->plane[0].dst.stride - l;
  template_matching(temp_dst, xd->plane[0].dst.stride,
                       temp_pre, xd->plane[0].pre[0].stride,
                       h, l, ur + lr, uc + lc, &mv_y[1], &mv_x[1]);
  mv2.as_mv.row = mv_y[1];
  mv2.as_mv.col = mv_x[1];
  sub_pixel_adjust(xd, temp_dst,
                      temp_pre + mv_y[1] * xd->plane[0].pre[0].stride + mv_x[1],
                      h, l, mv2, &temp_mv);
  mv2 = temp_mv;

  temp_dst = xd->plane[0].dst.buf - l * xd->plane[0].dst.stride + (n - h);
  template_matching(temp_dst, xd->plane[0].dst.stride,
                       temp_pre, xd->plane[0].pre[0].stride,
                       l, h, ur + lr, uc + lc, &mv_y[2], &mv_x[2]);
  mv3.as_mv.row = mv_y[2];
  mv3.as_mv.col = mv_x[2];
  sub_pixel_adjust(xd, temp_dst,
                      temp_pre + mv_y[2] * xd->plane[0].pre[0].stride + mv_x[2],
                      l, h, mv3, &temp_mv);
  mv3 = temp_mv;

  mv_x[0] = mv1.as_mv.col + (m << 3);
  mv_y[0] = mv1.as_mv.row + (m << 3);
  mv_x[1] = mv2.as_mv.col + (m << 3);
  mv_y[1] = mv2.as_mv.row + (t << 3);
  mv_x[2] = mv3.as_mv.col + (t << 3);
  mv_y[2] = mv3.as_mv.row + (m << 3);

  affine_estimation(mv_x, mv_y, n + m - t, aff_coeff);


  for (i = 0; i < n; i ++) {
    for (j = 0; j < n; j++) {
      x_f = (j + m + 1) * aff_coeff[0] + (i + m + 1) * aff_coeff[1] +
          aff_coeff[2];
      y_f = (j + m + 1) * aff_coeff[3] + (i + m + 1) * aff_coeff[4] +
          aff_coeff[5];
      xd->plane[0].dst.buf[i * xd->plane[0].dst.stride + j] =
          bil_interp(&x_f, &y_f, temp_pre, xd->plane[0].pre[0].stride);
    }
  }

  for (plane = 1; plane < 3; plane++) {
    temp_pre = xd->plane[plane].pre[0].buf -
        (ur >> 1) * xd->plane[plane].pre[0].stride - (uc >> 1);
    for (i = 0; i < (n >> 1); i++) {
      for (j = 0; j < (n >> 1); j++) {
        x_f = (2 * j + m + 1) * aff_coeff[0] + (2 * i + m + 1) * aff_coeff[1] +
            aff_coeff[2];
        y_f = (2 * j + m +1) * aff_coeff[3] + (2 * i + m + 1) * aff_coeff[4] +
            aff_coeff[5];
        x_f = x_f /2;
        y_f = y_f /2;
        xd->plane[plane].dst.buf[i * xd->plane[plane].dst.stride + j] =
            bil_interp(&x_f, &y_f, temp_pre, xd->plane[plane].pre[0].stride);
      }
    }
  }

  xd->mode_info_context->mbmi.mv[0].as_mv.col = mv1.as_mv.col -
      ((uc - l) << 3);
  xd->mode_info_context->mbmi.mv[0].as_mv.row = mv1.as_mv.row -
      ((ur - l) << 3);

  mv_y[0] = mv_y[0] >> 1;
  mv_x[0] = mv_x[0] >> 1;
  for (plane = 1; plane < 3; plane++) {
    temp_pre = xd->plane[plane].pre[0].buf -
        (ur >> 1) * xd->plane[plane].pre[0].stride - (uc >> 1);
    for (i = 0; i < (n >> 1); i ++) {
      for (j = 0; j < (n >> 1); j++) {
        xd->plane[plane].dst.buf[i * xd->plane[plane].dst.stride + j] =
            temp_pre[(i + mv_y[0]) * xd->plane[plane].pre[0].stride +
                     j + mv_x[0]];
      }
    }
  }
  xd->mode_info_context->mbmi.mv[0] = mv1;
}

void amp_inter(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize,
               int mi_row, int mi_col, int mi_rows, int mi_cols) {
  if (bsize == BLOCK_SIZE_MB16X16) {
    affine_motion_prediciton(xd, 16, 4, 4, mi_row, mi_col,
                             mi_rows, mi_cols, 4, 3);
    (xd->mode_info_context + 1) -> mbmi = xd->mode_info_context->mbmi;
    (xd->mode_info_context + xd->mode_info_stride) -> mbmi =
        xd->mode_info_context->mbmi;
    (xd->mode_info_context + xd->mode_info_stride + 1) -> mbmi =
        xd->mode_info_context->mbmi;
  } else if (bsize == BLOCK_SIZE_SB8X8) {
    affine_motion_prediciton(xd, 8, 3, 3, mi_row, mi_col,
                             mi_rows, mi_cols, 4, 3);
  } else {
    assert(0);
  }
}
#endif
static int scale_value_x_with_scaling(int val,
                                      const struct scale_factors *scale) {
  return (val * scale->x_scale_fp >> VP9_REF_SCALE_SHIFT);
}

static int scale_value_y_with_scaling(int val,
                                      const struct scale_factors *scale) {
  return (val * scale->y_scale_fp >> VP9_REF_SCALE_SHIFT);
}

static int unscaled_value(int val, const struct scale_factors *scale) {
  (void) scale;
  return val;
}

static MV32 mv_q3_to_q4_with_scaling(const MV *mv,
                                     const struct scale_factors *scale) {
  const MV32 res = {
    ((mv->row << 1) * scale->y_scale_fp >> VP9_REF_SCALE_SHIFT)
        + scale->y_offset_q4,
    ((mv->col << 1) * scale->x_scale_fp >> VP9_REF_SCALE_SHIFT)
        + scale->x_offset_q4
  };
  return res;
}

static MV32 mv_q3_to_q4_without_scaling(const MV *mv,
                                        const struct scale_factors *scale) {
  const MV32 res = {
     mv->row << 1,
     mv->col << 1
  };
  return res;
}

static MV32 mv_q4_with_scaling(const MV *mv,
                               const struct scale_factors *scale) {
  const MV32 res = {
    (mv->row * scale->y_scale_fp >> VP9_REF_SCALE_SHIFT) + scale->y_offset_q4,
    (mv->col * scale->x_scale_fp >> VP9_REF_SCALE_SHIFT) + scale->x_offset_q4
  };
  return res;
}

static MV32 mv_q4_without_scaling(const MV *mv,
                                  const struct scale_factors *scale) {
  const MV32 res = {
    mv->row,
    mv->col
  };
  return res;
}

static void set_offsets_with_scaling(struct scale_factors *scale,
                                     int row, int col) {
  const int x_q4 = 16 * col;
  const int y_q4 = 16 * row;

  scale->x_offset_q4 = (x_q4 * scale->x_scale_fp >> VP9_REF_SCALE_SHIFT) & 0xf;
  scale->y_offset_q4 = (y_q4 * scale->y_scale_fp >> VP9_REF_SCALE_SHIFT) & 0xf;
}

static void set_offsets_without_scaling(struct scale_factors *scale,
                                        int row, int col) {
  scale->x_offset_q4 = 0;
  scale->y_offset_q4 = 0;
}

static int get_fixed_point_scale_factor(int other_size, int this_size) {
  // Calculate scaling factor once for each reference frame
  // and use fixed point scaling factors in decoding and encoding routines.
  // Hardware implementations can calculate scale factor in device driver
  // and use multiplication and shifting on hardware instead of division.
  return (other_size << VP9_REF_SCALE_SHIFT) / this_size;
}

void vp9_setup_scale_factors_for_frame(struct scale_factors *scale,
                                       int other_w, int other_h,
                                       int this_w, int this_h) {
  scale->x_scale_fp = get_fixed_point_scale_factor(other_w, this_w);
  scale->x_offset_q4 = 0;  // calculated per-mb
  scale->x_step_q4 = (16 * scale->x_scale_fp >> VP9_REF_SCALE_SHIFT);

  scale->y_scale_fp = get_fixed_point_scale_factor(other_h, this_h);
  scale->y_offset_q4 = 0;  // calculated per-mb
  scale->y_step_q4 = (16 * scale->y_scale_fp >> VP9_REF_SCALE_SHIFT);

  if ((other_w == this_w) && (other_h == this_h)) {
    scale->scale_value_x = unscaled_value;
    scale->scale_value_y = unscaled_value;
    scale->set_scaled_offsets = set_offsets_without_scaling;
    scale->scale_mv_q3_to_q4 = mv_q3_to_q4_without_scaling;
    scale->scale_mv_q4 = mv_q4_without_scaling;
  } else {
    scale->scale_value_x = scale_value_x_with_scaling;
    scale->scale_value_y = scale_value_y_with_scaling;
    scale->set_scaled_offsets = set_offsets_with_scaling;
    scale->scale_mv_q3_to_q4 = mv_q3_to_q4_with_scaling;
    scale->scale_mv_q4 = mv_q4_with_scaling;
  }

  // TODO(agrange): Investigate the best choice of functions to use here
  // for EIGHTTAP_SMOOTH. Since it is not interpolating, need to choose what
  // to do at full-pel offsets. The current selection, where the filter is
  // applied in one direction only, and not at all for 0,0, seems to give the
  // best quality, but it may be worth trying an additional mode that does
  // do the filtering on full-pel.
  if (scale->x_step_q4 == 16) {
    if (scale->y_step_q4 == 16) {
      // No scaling in either direction.
      scale->predict[0][0][0] = vp9_convolve_copy;
      scale->predict[0][0][1] = vp9_convolve_avg;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_avg_vert;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // No scaling in x direction. Must always scale in the y direction.
      scale->predict[0][0][0] = vp9_convolve8_vert;
      scale->predict[0][0][1] = vp9_convolve8_avg_vert;
      scale->predict[0][1][0] = vp9_convolve8_vert;
      scale->predict[0][1][1] = vp9_convolve8_avg_vert;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_avg;
    }
  } else {
    if (scale->y_step_q4 == 16) {
      // No scaling in the y direction. Must always scale in the x direction.
      scale->predict[0][0][0] = vp9_convolve8_horiz;
      scale->predict[0][0][1] = vp9_convolve8_avg_horiz;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_avg;
      scale->predict[1][0][0] = vp9_convolve8_horiz;
      scale->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // Must always scale in both directions.
      scale->predict[0][0][0] = vp9_convolve8;
      scale->predict[0][0][1] = vp9_convolve8_avg;
      scale->predict[0][1][0] = vp9_convolve8;
      scale->predict[0][1][1] = vp9_convolve8_avg;
      scale->predict[1][0][0] = vp9_convolve8;
      scale->predict[1][0][1] = vp9_convolve8_avg;
    }
  }
  // 2D subpel motion always gets filtered in both directions
  scale->predict[1][1][0] = vp9_convolve8;
  scale->predict[1][1][1] = vp9_convolve8_avg;
}

void vp9_setup_interp_filters(MACROBLOCKD *xd,
                              INTERPOLATIONFILTERTYPE mcomp_filter_type,
                              VP9_COMMON *cm) {
  if (xd->mode_info_context) {
    MB_MODE_INFO *mbmi = &xd->mode_info_context->mbmi;

    set_scale_factors(xd, mbmi->ref_frame[0] - 1, mbmi->ref_frame[1] - 1,
                      cm->active_ref_scale);
  }

  switch (mcomp_filter_type) {
    case EIGHTTAP:
    case SWITCHABLE:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8;
      break;
    case EIGHTTAP_SMOOTH:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8lp;
      break;
    case EIGHTTAP_SHARP:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_sub_pel_filters_8s;
      break;
    case BILINEAR:
      xd->subpix.filter_x = xd->subpix.filter_y = vp9_bilinear_filters;
      break;
  }
  assert(((intptr_t)xd->subpix.filter_x & 0xff) == 0);
}

void vp9_build_inter_predictor(const uint8_t *src, int src_stride,
                               uint8_t *dst, int dst_stride,
                               const int_mv *src_mv,
                               const struct scale_factors *scale,
                               int w, int h, int weight,
                               const struct subpix_fn_table *subpix,
                               enum mv_precision precision) {
  const MV32 mv = precision == MV_PRECISION_Q4
                     ? scale->scale_mv_q4(&src_mv->as_mv, scale)
                     : scale->scale_mv_q3_to_q4(&src_mv->as_mv, scale);
  const int subpel_x = mv.col & 15;
  const int subpel_y = mv.row & 15;

  src += (mv.row >> 4) * src_stride + (mv.col >> 4);
  scale->predict[!!subpel_x][!!subpel_y][weight](
      src, src_stride, dst, dst_stride,
      subpix->filter_x[subpel_x], scale->x_step_q4,
      subpix->filter_y[subpel_y], scale->y_step_q4,
      w, h);
}

static INLINE int round_mv_comp_q4(int value) {
  return (value < 0 ? value - 2 : value + 2) / 4;
}

static int mi_mv_pred_row_q4(MACROBLOCKD *mb, int idx) {
  const int temp = mb->mode_info_context->bmi[0].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[1].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[2].as_mv[idx].as_mv.row +
                   mb->mode_info_context->bmi[3].as_mv[idx].as_mv.row;
  return round_mv_comp_q4(temp);
}

static int mi_mv_pred_col_q4(MACROBLOCKD *mb, int idx) {
  const int temp = mb->mode_info_context->bmi[0].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[1].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[2].as_mv[idx].as_mv.col +
                   mb->mode_info_context->bmi[3].as_mv[idx].as_mv.col;
  return round_mv_comp_q4(temp);
}

// TODO(jkoleszar): yet another mv clamping function :-(
MV clamp_mv_to_umv_border_sb(const MV *src_mv,
    int bwl, int bhl, int ss_x, int ss_y,
    int mb_to_left_edge, int mb_to_top_edge,
    int mb_to_right_edge, int mb_to_bottom_edge) {
  /* If the MV points so far into the UMV border that no visible pixels
   * are used for reconstruction, the subpel part of the MV can be
   * discarded and the MV limited to 16 pixels with equivalent results.
   */
  const int spel_left = (VP9_INTERP_EXTEND + (4 << bwl)) << 4;
  const int spel_right = spel_left - (1 << 4);
  const int spel_top = (VP9_INTERP_EXTEND + (4 << bhl)) << 4;
  const int spel_bottom = spel_top - (1 << 4);
  MV clamped_mv;

  assert(ss_x <= 1);
  assert(ss_y <= 1);
  clamped_mv.col = clamp(src_mv->col << (1 - ss_x),
                         (mb_to_left_edge << (1 - ss_x)) - spel_left,
                         (mb_to_right_edge << (1 - ss_x)) + spel_right);
  clamped_mv.row = clamp(src_mv->row << (1 - ss_y),
                         (mb_to_top_edge << (1 - ss_y)) - spel_top,
                         (mb_to_bottom_edge << (1 - ss_y)) + spel_bottom);
  return clamped_mv;
}

struct build_inter_predictors_args {
  MACROBLOCKD *xd;
  int x;
  int y;
  uint8_t* dst[MAX_MB_PLANE];
  int dst_stride[MAX_MB_PLANE];
  uint8_t* pre[2][MAX_MB_PLANE];
  int pre_stride[2][MAX_MB_PLANE];
};
static void build_inter_predictors(int plane, int block,
                                   BLOCK_SIZE_TYPE bsize,
                                   int pred_w, int pred_h,
                                   void *argv) {
  const struct build_inter_predictors_args* const arg = argv;
  MACROBLOCKD * const xd = arg->xd;
  const int bwl = b_width_log2(bsize) - xd->plane[plane].subsampling_x;
  const int bhl = b_height_log2(bsize) - xd->plane[plane].subsampling_y;
  const int x = 4 * (block & ((1 << bwl) - 1)), y = 4 * (block >> bwl);
  const int use_second_ref = xd->mode_info_context->mbmi.ref_frame[1] > 0;
  int which_mv;

  assert(x < (4 << bwl));
  assert(y < (4 << bhl));
  assert(xd->mode_info_context->mbmi.sb_type < BLOCK_SIZE_SB8X8 ||
         4 << pred_w == (4 << bwl));
  assert(xd->mode_info_context->mbmi.sb_type < BLOCK_SIZE_SB8X8 ||
         4 << pred_h == (4 << bhl));

  for (which_mv = 0; which_mv < 1 + use_second_ref; ++which_mv) {
    // source
    const uint8_t * const base_pre = arg->pre[which_mv][plane];
    const int pre_stride = arg->pre_stride[which_mv][plane];
    const uint8_t *const pre = base_pre +
        scaled_buffer_offset(x, y, pre_stride, &xd->scale_factor[which_mv]);
    struct scale_factors * const scale = &xd->scale_factor[which_mv];

    // dest
    uint8_t *const dst = arg->dst[plane] + arg->dst_stride[plane] * y + x;

    // motion vector
    const MV *mv;
    MV split_chroma_mv;
    int_mv clamped_mv;

    if (xd->mode_info_context->mbmi.sb_type < BLOCK_SIZE_SB8X8) {
      if (plane == 0) {
        mv = &xd->mode_info_context->bmi[block].as_mv[which_mv].as_mv;
      } else {
        // TODO(jkoleszar): All chroma MVs in SPLITMV mode are taken as the
        // same MV (the average of the 4 luma MVs) but we could do something
        // smarter for non-4:2:0. Just punt for now, pending the changes to get
        // rid of SPLITMV mode entirely.
        split_chroma_mv.row = mi_mv_pred_row_q4(xd, which_mv);
        split_chroma_mv.col = mi_mv_pred_col_q4(xd, which_mv);
        mv = &split_chroma_mv;
      }
    } else {
      mv = &xd->mode_info_context->mbmi.mv[which_mv].as_mv;
    }

    /* TODO(jkoleszar): This clamping is done in the incorrect place for the
     * scaling case. It needs to be done on the scaled MV, not the pre-scaling
     * MV. Note however that it performs the subsampling aware scaling so
     * that the result is always q4.
     */
    clamped_mv.as_mv = clamp_mv_to_umv_border_sb(mv, bwl, bhl,
                                                 xd->plane[plane].subsampling_x,
                                                 xd->plane[plane].subsampling_y,
                                                 xd->mb_to_left_edge,
                                                 xd->mb_to_top_edge,
                                                 xd->mb_to_right_edge,
                                                 xd->mb_to_bottom_edge);
    scale->set_scaled_offsets(scale, arg->y + y, arg->x + x);

    vp9_build_inter_predictor(pre, pre_stride,
                              dst, arg->dst_stride[plane],
                              &clamped_mv, &xd->scale_factor[which_mv],
                              4 << pred_w, 4 << pred_h, which_mv,
                              &xd->subpix, MV_PRECISION_Q4);
  }
}
void vp9_build_inter_predictors_sby(MACROBLOCKD *xd,
                                    int mi_row,
                                    int mi_col,
                                    BLOCK_SIZE_TYPE bsize) {
  struct build_inter_predictors_args args = {
    xd, mi_col * MI_SIZE, mi_row * MI_SIZE,
    {xd->plane[0].dst.buf, NULL, NULL}, {xd->plane[0].dst.stride, 0, 0},
    {{xd->plane[0].pre[0].buf, NULL, NULL},
     {xd->plane[0].pre[1].buf, NULL, NULL}},
    {{xd->plane[0].pre[0].stride, 0, 0}, {xd->plane[0].pre[1].stride, 0, 0}},
  };

  foreach_predicted_block_in_plane(xd, bsize, 0, build_inter_predictors, &args);
}
void vp9_build_inter_predictors_sbuv(MACROBLOCKD *xd,
                                     int mi_row,
                                     int mi_col,
                                     BLOCK_SIZE_TYPE bsize) {
  struct build_inter_predictors_args args = {
    xd, mi_col * MI_SIZE, mi_row * MI_SIZE,
#if CONFIG_ALPHA
    {NULL, xd->plane[1].dst.buf, xd->plane[2].dst.buf,
     xd->plane[3].dst.buf},
    {0, xd->plane[1].dst.stride, xd->plane[1].dst.stride,
     xd->plane[3].dst.stride},
    {{NULL, xd->plane[1].pre[0].buf, xd->plane[2].pre[0].buf,
      xd->plane[3].pre[0].buf},
     {NULL, xd->plane[1].pre[1].buf, xd->plane[2].pre[1].buf,
      xd->plane[3].pre[1].buf}},
    {{0, xd->plane[1].pre[0].stride, xd->plane[1].pre[0].stride,
      xd->plane[3].pre[0].stride},
     {0, xd->plane[1].pre[1].stride, xd->plane[1].pre[1].stride,
      xd->plane[3].pre[1].stride}},
#else
    {NULL, xd->plane[1].dst.buf, xd->plane[2].dst.buf},
    {0, xd->plane[1].dst.stride, xd->plane[1].dst.stride},
    {{NULL, xd->plane[1].pre[0].buf, xd->plane[2].pre[0].buf},
     {NULL, xd->plane[1].pre[1].buf, xd->plane[2].pre[1].buf}},
    {{0, xd->plane[1].pre[0].stride, xd->plane[1].pre[0].stride},
     {0, xd->plane[1].pre[1].stride, xd->plane[1].pre[1].stride}},
#endif
  };
  foreach_predicted_block_uv(xd, bsize, build_inter_predictors, &args);
}
#if CONFIG_AFFINE_MP
void vp9_build_inter_predictors_sb(MACROBLOCKD *xd,
                                   int mi_row, int mi_col,
                                   BLOCK_SIZE_TYPE bsize,
                                   int mi_rows, int mi_cols) {
  if (xd->mode_info_context->mbmi.mode == AFFINEMV) {
    assert(xd->mode_info_context->mbmi.ref_frame[0] == LAST_FRAME);
    assert(xd->mode_info_context->mbmi.ref_frame[1] == NONE);
    amp_inter(xd, bsize, mi_row, mi_col, mi_rows, mi_cols);
  } else {
    vp9_build_inter_predictors_sby(xd, mi_row, mi_col, bsize);
    vp9_build_inter_predictors_sbuv(xd, mi_row, mi_col, bsize);
  }
}
#else
void vp9_build_inter_predictors_sb(MACROBLOCKD *xd,
                                   int mi_row, int mi_col,
                                   BLOCK_SIZE_TYPE bsize) {
  vp9_build_inter_predictors_sby(xd, mi_row, mi_col, bsize);
  vp9_build_inter_predictors_sbuv(xd, mi_row, mi_col, bsize);
}
#endif

// TODO(dkovalev: find better place for this function)
void vp9_setup_scale_factors(VP9_COMMON *cm, int i) {
  const int ref = cm->active_ref_idx[i];
  struct scale_factors *const sf = &cm->active_ref_scale[i];
  if (ref >= NUM_YV12_BUFFERS) {
    memset(sf, 0, sizeof(*sf));
  } else {
    YV12_BUFFER_CONFIG *const fb = &cm->yv12_fb[ref];
    vp9_setup_scale_factors_for_frame(sf,
                                      fb->y_crop_width, fb->y_crop_height,
                                      cm->width, cm->height);

    if (sf->x_scale_fp != VP9_REF_NO_SCALE ||
        sf->y_scale_fp != VP9_REF_NO_SCALE)
      vp9_extend_frame_borders(fb, cm->subsampling_x, cm->subsampling_y);
  }
}

