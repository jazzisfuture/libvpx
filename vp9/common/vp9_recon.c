/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "./vpx_config.h"
#include "vp9_rtcd.h"
#include "vp9/common/vp9_blockd.h"

static INLINE void recon(int rows, int cols,
                         const uint8_t *pred_ptr, int pred_stride,
                         const int16_t *diff_ptr, int diff_stride,
                         uint8_t *dst_ptr, int dst_stride) {
  int r, c;

  for (r = 0; r < rows; r++) {
    for (c = 0; c < cols; c++)
      dst_ptr[c] = clip_pixel(diff_ptr[c] + pred_ptr[c]);

    dst_ptr += dst_stride;
    diff_ptr += diff_stride;
    pred_ptr += pred_stride;
  }
}


void vp9_recon_b_c(uint8_t *pred_ptr, int16_t *diff_ptr, uint8_t *dst_ptr,
                   int stride) {
  recon(4, 4, pred_ptr, 16, diff_ptr, 16, dst_ptr, stride);
}

void vp9_recon_uv_b_c(uint8_t *pred_ptr, int16_t *diff_ptr, uint8_t *dst_ptr,
                      int stride) {
  recon(4, 4, pred_ptr, 8, diff_ptr, 8, dst_ptr, stride);
}

void vp9_recon4b_c(uint8_t *pred_ptr, int16_t *diff_ptr, uint8_t *dst_ptr,
                   int stride) {
  recon(4, 16, pred_ptr, 16, diff_ptr, 16, dst_ptr, stride);
}

void vp9_recon2b_c(uint8_t *pred_ptr, int16_t *diff_ptr, uint8_t *dst_ptr,
                   int stride) {
  recon(4, 8, pred_ptr, 8, diff_ptr, 8, dst_ptr, stride);
}

void vp9_recon_mby_s_c(MACROBLOCKD *xd, uint8_t *dst) {
  int x, y;
  BLOCKD *const b = &xd->block[0];
  const int stride = b->dst_stride;
  const int16_t *diff = b->diff;

  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++)
      dst[x] = clip_pixel(dst[x] + diff[x]);

    dst += stride;
    diff += 16;
  }
}

void vp9_recon_mbuv_s_c(MACROBLOCKD *xd, uint8_t *udst, uint8_t *vdst) {
  int x, y, i;
  uint8_t *dst = udst;

  for (i = 0; i < 2; i++, dst = vdst) {
    BLOCKD *const b = &xd->block[16 + 4 * i];
    const int stride = b->dst_stride;
    const int16_t *diff = b->diff;

    for (y = 0; y < 8; y++) {
      for (x = 0; x < 8; x++)
        dst[x] = clip_pixel(dst[x] + diff[x]);

      dst += stride;
      diff += 8;
    }
  }
}

#if CONFIG_SBSEGMENT
void vp9_recon_segy_c(MACROBLOCKD *xd, uint8_t *dst) {
  MB_MODE_INFO *mbmi = &xd->mode_info_context->mbmi;
  int i, j;
  int rows, cols, stride;
  int16_t *diff = xd->diff;
  int dst_stride = xd->block[0].dst_stride;

  get_seg_parameters(mbmi, &rows, &cols, &stride);

  for (j = 0; j < rows; j++) {
    for (i = 0; i < cols; i++) {
      dst[i] = clip_pixel(dst[i] + diff[i]);
    }
    dst  += dst_stride;
    diff += stride;
  }
}

void vp9_recon_seguv_c(MACROBLOCKD *xd, uint8_t *udst, uint8_t *vdst) {
  MB_MODE_INFO *mbmi = &xd->mode_info_context->mbmi;
  int i, j;
  int rows, cols, stride;
  int16_t *udiff, *vdiff;
  int dst_stride = (xd->block[0].dst_stride >> 1);
  int y_offset, uv_offset;

  get_seg_parameters(mbmi, &rows, &cols, &stride);

  rows = (rows >> 1);
  cols = (cols >> 1);
  stride = (stride >> 1);
  y_offset  = 4 * stride * stride;
  uv_offset = stride * stride;

  udiff = xd->diff + y_offset;
  vdiff = xd->diff + y_offset + uv_offset;

  for (j = 0; j < rows; j++) {
    for (i = 0; i < cols; i++) {
      udst[i] = clip_pixel(udst[i] + udiff[i]);
      vdst[i] = clip_pixel(vdst[i] + vdiff[i]);
    }
    udst += dst_stride;
    vdst += dst_stride;
    udiff += stride;
    vdiff += stride;
  }
}
#endif

static INLINE void recon_sby(MACROBLOCKD *mb, uint8_t *dst, int size) {
  int x, y;
  const int stride = mb->block[0].dst_stride;
  const int16_t *diff = mb->diff;

  for (y = 0; y < size; y++) {
    for (x = 0; x < size; x++)
      dst[x] = clip_pixel(dst[x] + diff[x]);

    dst += stride;
    diff += size;
  }
}

static INLINE void recon_sbuv(MACROBLOCKD *mb, uint8_t *u_dst, uint8_t *v_dst,
                              int y_offset, int size) {
  int x, y;
  const int stride = mb->block[16].dst_stride;
  const int16_t *u_diff = mb->diff + y_offset;
  const int16_t *v_diff = mb->diff + y_offset + size*size;

  for (y = 0; y < size; y++) {
    for (x = 0; x < size; x++) {
      u_dst[x] = clip_pixel(u_dst[x] + u_diff[x]);
      v_dst[x] = clip_pixel(v_dst[x] + v_diff[x]);
    }

    u_dst += stride;
    v_dst += stride;
    u_diff += size;
    v_diff += size;
  }
}

void vp9_recon_sby_s_c(MACROBLOCKD *mb, uint8_t *dst) {
  recon_sby(mb, dst, 32);
}

void vp9_recon_sbuv_s_c(MACROBLOCKD *mb, uint8_t *u_dst, uint8_t *v_dst) {
  recon_sbuv(mb, u_dst, v_dst, 1024, 16);
}

void vp9_recon_sb64y_s_c(MACROBLOCKD *mb, uint8_t *dst) {
  recon_sby(mb, dst, 64);
}

void vp9_recon_sb64uv_s_c(MACROBLOCKD *mb, uint8_t *u_dst, uint8_t *v_dst) {
  recon_sbuv(mb, u_dst, v_dst, 4096, 32);
}

void vp9_recon_mby_c(MACROBLOCKD *xd) {
  int i;

  for (i = 0; i < 16; i += 4) {
    BLOCKD *b = &xd->block[i];

    vp9_recon4b(b->predictor, b->diff, *(b->base_dst) + b->dst, b->dst_stride);
  }
}

void vp9_recon_mb_c(MACROBLOCKD *xd) {
  int i;

  for (i = 0; i < 16; i += 4) {
    BLOCKD *b = &xd->block[i];

    vp9_recon4b(b->predictor, b->diff, *(b->base_dst) + b->dst, b->dst_stride);
  }

  for (i = 16; i < 24; i += 2) {
    BLOCKD *b = &xd->block[i];

    vp9_recon2b(b->predictor, b->diff, *(b->base_dst) + b->dst, b->dst_stride);
  }
}
