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

#define UV_LVL_SHIFT 1
#define UV_STEP_MAX 4
#define UV_ROW_MULT 2
#define UV_BLOCK_DIM 32
#define UV_ROW_MULT 2

uchar* raw_lfm;
uchar* raw_lft;
uchar* buf;
uint32_t sb_cols;
uint32_t rows;
uint32_t cols;
uint32_t start_sx;
uint32_t start_sy;

// Y plane filters
void __attribute__((kernel)) filter_cols_y(uchar input, uint32_t x) {
  uint32_t sb_offset = x / Y_BLOCK_DIM;
  uint32_t b_offset = x % Y_BLOCK_DIM;
  uint32_t sx = start_sx - sb_offset;
  uint32_t sy = start_sy + sb_offset;
  
  struct LOOP_FILTER_MASK* const lfm = (LOOP_FILTER_MASK*)raw_lfm;
  struct cuda_loop_filter_thresh* const lft = (cuda_loop_filter_thresh*)raw_lft;
  
  const uint32_t x_idx = sx * Y_BLOCK_DIM;
  const uint32_t y_idx = sy * Y_BLOCK_DIM + b_offset;
  uint32_t step = 0;
  
  if (y_idx >= rows) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint64_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  
  // apply y filters
  mask_16x16 = mask->left_y[TX_16X16];
  mask_8x8 = mask->left_y[TX_8X8];
  mask_4x4 = mask->left_y[TX_4X4];
  mask_int_4x4 = mask->int_4x4_y;

  const uint32_t row = b_offset / MI_SIZE;
  const uint32_t mi_offset = row * Y_STEP_MAX;
  uint32_t lfl_offset = row << Y_LVL_SHIFT;

  for (; x_idx + step * MI_SIZE < cols && step < Y_STEP_MAX; step++) {
    const uint32_t shift = mi_offset + step;
    const uint32_t x_off = x_idx + step * MI_SIZE;
    uint8_t* s = buf + y_idx * cols + x_off;
    uint32_t lfl;
    
    // calculate loop filter threshold
    lfl = mask->lfl_y[lfl_offset];
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;
    const uint32_t apply_16x16 = ((mask_16x16 >> shift) & 1);
    const uint32_t apply_8x8 = ((mask_8x8 >> shift) & 1);
    const uint32_t apply_4x4 = ((mask_4x4 >> shift) & 1);
    const uint32_t apply_int_4x4 = ((mask_int_4x4 >> shift) & 1);

    if ((mask_16x16 >> shift) & 1) {
      filter_16_col(s, mblim, lim, hev_thr);
    }
    else if ((mask_8x8 >> shift) & 1) {
      filter_8_col(s, mblim, lim, hev_thr);
    }
    else if ((mask_4x4 >> shift) & 1) {
      filter_4_col(s, mblim, lim, hev_thr);
    }
    if ((mask_int_4x4 >> shift) & 1) {
      filter_4_col(s + 4, mblim, lim, hev_thr);
    }
    lfl_offset += 1;
  }
}

void __attribute__((kernel)) filter_rows_y(uchar input, uint32_t x) {
  uint32_t sb_offset = x / Y_BLOCK_DIM;
  uint32_t b_offset = x % Y_BLOCK_DIM;
  uint32_t sx = start_sx - sb_offset;
  uint32_t sy = start_sy + sb_offset;
  
  struct LOOP_FILTER_MASK* const lfm = (LOOP_FILTER_MASK*)raw_lfm;
  struct cuda_loop_filter_thresh* const lft = (cuda_loop_filter_thresh*)raw_lft;
  
  const uint32_t x_idx = sx * Y_BLOCK_DIM + b_offset;
  const uint32_t y = sy * Y_BLOCK_DIM;
  uint32_t step = 0;

  if (x_idx >= cols) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint64_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  mask_16x16 = mask->above_y[TX_16X16];
  mask_8x8 = mask->above_y[TX_8X8];
  mask_4x4 = mask->above_y[TX_4X4];
  mask_int_4x4 = mask->int_4x4_y;

  const uint32_t mi_offset = b_offset / MI_SIZE;
  for (; y + step * MI_SIZE < rows && step < Y_STEP_MAX; step++) {
    const uint32_t shift = mi_offset + step * Y_STEP_MAX;
    const uint32_t y_off = y + step * MI_SIZE;
    uint8_t * s = buf + y_off * cols + x_idx;

    // Calculate loop filter threshold
    const uint32_t lfl_offset = (step << Y_LVL_SHIFT) + mi_offset;
    uint32_t lfl;
    lfl = mask->lfl_y[lfl_offset];
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;

    if ((mask_16x16 >> shift) & 1) {
      filter_16_row(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_8x8 >> shift) & 1) {
      filter_8_row(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_4x4 >> shift) & 1) {
      filter_4_row(s, cols, mblim, lim, hev_thr);
    }
    if ((mask_int_4x4 >> shift) & 1) {
      filter_4_row(s + 4 * cols, cols, mblim, lim, hev_thr);
    }
  }
}

// UV plane filters
void __attribute__((kernel)) filter_cols_uv(uchar input, uint32_t x) {
  uint32_t sb_offset = x / UV_BLOCK_DIM;
  uint32_t b_offset = x % UV_BLOCK_DIM;
  uint32_t sx = start_sx - sb_offset;
  uint32_t sy = start_sy + sb_offset;
  
  struct LOOP_FILTER_MASK* const lfm = (LOOP_FILTER_MASK*)raw_lfm;
  struct cuda_loop_filter_thresh* const lft = (cuda_loop_filter_thresh*)raw_lft;
  
  const uint32_t x_idx = sx * UV_BLOCK_DIM;
  const uint32_t y_idx = sy * UV_BLOCK_DIM + b_offset;
  uint32_t step = 0;
  
  if (y_idx >= rows) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint16_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  
  // apply uv filters
  mask_16x16 = mask->left_uv[TX_16X16];
  mask_8x8 = mask->left_uv[TX_8X8];
  mask_4x4 = mask->left_uv[TX_4X4];
  mask_int_4x4 = mask->int_4x4_uv;

  const uint32_t row = b_offset / MI_SIZE;
  const uint32_t mi_offset = row * UV_STEP_MAX;
  uint32_t lfl_offset = (row * UV_ROW_MULT) << UV_LVL_SHIFT;

  for (; x_idx + step * MI_SIZE < cols && step < UV_STEP_MAX; step++) {
    const uint32_t shift = mi_offset + step;
    const uint32_t x_off = x_idx + step * MI_SIZE;
    uint8_t* s = buf + y_idx * cols + x_off;
    uint32_t lfl;
    
    // calculate loop filter threshold
    lfl = mask->lfl_uv[lfl_offset];
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;

    if ((mask_16x16 >> shift) & 1) {
      filter_16_col(s, mblim, lim, hev_thr);
    }
    else if ((mask_8x8 >> shift) & 1) {
      filter_8_col(s, mblim, lim, hev_thr);
    }
    else if ((mask_4x4 >> shift) & 1) {
      filter_4_col(s, mblim, lim, hev_thr);
    }
    if ((mask_int_4x4 >> shift) & 1) {
      filter_4_col(s + 4, mblim, lim, hev_thr);
    }
    lfl_offset += 1;
  }
}


void __attribute__((kernel)) filter_rows_uv(uchar input, uint32_t x) {
  uint32_t sb_offset = x / UV_BLOCK_DIM;
  uint32_t b_offset = x % UV_BLOCK_DIM;
  uint32_t sx = start_sx - sb_offset;
  uint32_t sy = start_sy + sb_offset;
  
  struct LOOP_FILTER_MASK* const lfm = (LOOP_FILTER_MASK*)raw_lfm;
  struct cuda_loop_filter_thresh* const lft = (cuda_loop_filter_thresh*)raw_lft;
  
  const uint32_t x_idx = sx * UV_BLOCK_DIM + b_offset;
  const uint32_t y = sy * UV_BLOCK_DIM;
  uint32_t step = 0;

  if (x_idx >= cols) {
    return;
  }

  const struct LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  uint16_t mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
  mask_16x16 = mask->above_uv[TX_16X16];
  mask_8x8 = mask->above_uv[TX_8X8];
  mask_4x4 = mask->above_uv[TX_4X4];
  mask_int_4x4 = mask->int_4x4_uv;

  const uint32_t mi_offset = b_offset / MI_SIZE;
  for (; y + step * MI_SIZE < rows && step < UV_STEP_MAX; step++) {
    const uint32_t shift = mi_offset + step * UV_STEP_MAX;
    const uint32_t y_off = y + step * MI_SIZE;
    uint8_t * s = buf + y_off * cols + x_idx;

    // Calculate loop filter threshold
    const uint32_t lfl_offset = (step * UV_ROW_MULT << UV_LVL_SHIFT) + mi_offset;
    uint32_t lfl = mask->lfl_uv[lfl_offset];
    const struct cuda_loop_filter_thresh llft = lft[lfl];
    const uint8_t mblim = llft.mblim;
    const uint8_t lim = llft.lim;
    const uint8_t hev_thr = llft.hev_thr;

    if ((mask_16x16 >> shift) & 1) {
      filter_16_row(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_8x8 >> shift) & 1) {
      filter_8_row(s, cols, mblim, lim, hev_thr);
    }
    else if ((mask_4x4 >> shift) & 1) {
      filter_4_row(s, cols, mblim, lim, hev_thr);
    }
    if ((mask_int_4x4 >> shift) & 1 && y_off + 4 < rows) {
      filter_4_row(s + 4 * cols, cols, mblim, lim, hev_thr);
    }
  }
}


