/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma version(1)
#pragma rs java_package_name(com.example.vp9)
#pragma rs_fp_relaxed
#include "loopfilters.rsh"

// types for loop filtering
typedef enum {
  TX_4X4 = 0,                      // 4x4 transform
  TX_8X8 = 1,                      // 8x8 transform
  TX_16X16 = 2,                    // 16x16 transform
  TX_32X32 = 3,                    // 32x32 transform
  TX_SIZES
} TX_SIZE;

#define MAX_LOOP_FILTER 63
#define MI_SIZE 8
#define SUPER_BLOCK_DIM 64


typedef struct cuda_loop_filter_thresh{
  uint8_t mblim;
  uint8_t lim;
  uint8_t hev_thr;
}cuda_loop_filter_thresh;

typedef struct LOOP_FILTER_MASK{
  uint64_t left_y[TX_SIZES];
  uint64_t above_y[TX_SIZES];
  uint64_t int_4x4_y;
  uint16_t left_uv[TX_SIZES];
  uint16_t above_uv[TX_SIZES];
  uint16_t int_4x4_uv;
  uint8_t lfl_y[64];
  uint8_t lfl_uv[16];
}LOOP_FILTER_MASK;


#define Y_LVL_SHIFT 3
#define Y_STEP_MAX 8
#define Y_ROW_MULT 1
#define Y_BLOCK_DIM 64

#define UV_LVL_SHIFT 3
#define UV_STEP_MAX 8
#define UV_ROW_MULT 1
#define UV_BLOCK_DIM 64

uchar* raw_lfm;
uchar* raw_lft;

uchar* buf;
// ONLY y for now
bool is_y;
bool do_filter_cols;
uint32_t sb_cols;
uint32_t rows;
uint32_t cols;
uint32_t step_max;
uint32_t lvl_shift;
uint32_t row_mult;
uint32_t blockDim;
uint32_t start_sx;
uint32_t start_sy;


static
void filter_cols(uint32_t sx, 
    uint32_t sy,
    LOOP_FILTER_MASK* const lfm,
    cuda_loop_filter_thresh* const lft,
    uint32_t x) {
  const uint32_t x_idx = sx * blockDim;
  const uint32_t y = sy * blockDim + x;
  uint32_t step = 0;
  
  if (y >= rows) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint64_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  if (is_y) {
    mask_16x16 = mask->left_y[TX_16X16];
    mask_8x8 = mask->left_y[TX_8X8];
    mask_4x4 = mask->left_y[TX_4X4];
    mask_int_4x4 = mask->int_4x4_y;
  } else {
    mask_16x16 = mask->left_uv[TX_16X16];
    mask_8x8 = mask->left_uv[TX_8X8];
    mask_4x4 = mask->left_uv[TX_4X4];
    mask_int_4x4 = mask->int_4x4_uv;
  }

  const uint32_t row = x / MI_SIZE;
  const uint32_t mi_offset = row * step_max;
  uint32_t lfl_offset = row * row_mult << lvl_shift;

  for (; x_idx + step * MI_SIZE < cols && step < step_max; step++) {
    const uint32_t shift = mi_offset + step;
    const uint32_t x_off = x_idx + step * MI_SIZE;
    uint8_t* s = buf + y * cols + x_off;
    uint32_t lfl;
    if (is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;
    const uint32_t apply_16x16 = ((mask_16x16 >> shift) & 1);
    const uint32_t apply_8x8 = ((mask_8x8 >> shift) & 1);
    const uint32_t apply_4x4 = ((mask_4x4 >> shift) & 1);
    const uint32_t apply_int_4x4 = ((mask_int_4x4 >> shift) & 1);

    if (apply_16x16 | apply_8x8 | apply_4x4) {
      filter_vertical_edge(s, mblim, lim, hev_thr, apply_16x16, apply_8x8);
    }
    if(apply_int_4x4) {
      filter_vertical_edge(s + 4, mblim, lim, hev_thr, 0, 0);
    }

    lfl_offset += 1;
  }
}

static
void filter_rows(uint32_t sx, 
    uint32_t sy, 
    LOOP_FILTER_MASK* const lfm,
    cuda_loop_filter_thresh* const lft,
    uint32_t x) {
  const uint32_t x_idx = sx * blockDim + x;
  const uint32_t y = sy * blockDim;
  uint32_t step = 0;

  if (x_idx >= cols) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint64_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  if (is_y) {
    mask_16x16 = mask->above_y[TX_16X16];
    mask_8x8 = mask->above_y[TX_8X8];
    mask_4x4 = mask->above_y[TX_4X4];
    mask_int_4x4 = mask->int_4x4_y;
  } else {
    mask_16x16 = mask->above_uv[TX_16X16];
    mask_8x8 = mask->above_uv[TX_8X8];
    mask_4x4 = mask->above_uv[TX_4X4];
    mask_int_4x4 = mask->int_4x4_uv;
  }

  const uint32_t mi_offset = x / MI_SIZE;
  for (; y + step * MI_SIZE < rows && step < step_max; step++) {
    const uint32_t shift = mi_offset + step * step_max;
    const uint32_t y_off = y + step * MI_SIZE;
    uint8_t * s = buf + y_off * cols + x_idx;

    // Calculate loop filter threshold
    const uint32_t lfl_offset = (step * row_mult << lvl_shift) + mi_offset;
    uint32_t lfl;
    if(is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;

    if ((mask_16x16 >> shift) & 1) {
      vp9_mb_lpf_horizontal_edge_w_cuda(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_8x8 >> shift) & 1) {
      vp9_mbloop_filter_horizontal_edge_cuda(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_4x4 >> shift) & 1) {
      vp9_loop_filter_horizontal_edge_cuda(s, cols, mblim, lim, hev_thr);
    }
    if ((mask_int_4x4 >> shift) & 1 && (is_y || y_off + 4 < rows)) {
      vp9_loop_filter_horizontal_edge_cuda(s + 4 * cols, cols, mblim, lim, hev_thr);
    }
  }
}


void root(const uchar* input, uint32_t x) {
  uint32_t sb_offset = x / SUPER_BLOCK_DIM;
  uint32_t b_offset = x % SUPER_BLOCK_DIM;
  uint32_t sx = start_sx - sb_offset;
  uint32_t sy = start_sy + sb_offset;
  
  struct LOOP_FILTER_MASK* const lfm = (LOOP_FILTER_MASK*)raw_lfm;
  struct cuda_loop_filter_thresh* const lft = (cuda_loop_filter_thresh*)raw_lft;
  
  if(do_filter_cols) {
    filter_cols(sx, sy, lfm, lft, b_offset);
  } else {
    filter_rows(sx, sy, lfm, lft, b_offset);
  }
}
