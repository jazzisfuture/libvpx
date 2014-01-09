#include "./vp9/common/cuda_loopfilter_def.h"
#include "./vp9/common/cuda_loopfilter.h"
#include "./vp9/common/cuda_loopfilters.h"

void filter_cols(__global const LOOP_FILTER_MASK* const lfm,
                 __global const cuda_loop_filter_thresh* const lft,
                 const uint sx,
                 const uint sy,
                 const uint sb_cols,
                 const uint rows,
                 const uint cols,
                 __global uchar * buf,
                 const bool is_y,
                 const uint step_max,
                 const uint lvl_shift,
                 const uint row_mult) {
  const uint x = sx * get_local_size(0);
  const uint y = sy * get_local_size(0) + get_local_id(0);
  uint step = 0;

  if (y >= rows) {
    return;
  }

  __global const LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  ulong mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
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

  const uint row = get_local_id(0) / MI_SIZE;
  const uint mi_offset = row * step_max;
  uint lfl_offset = row * row_mult << lvl_shift;

  for (; x + step * MI_SIZE < cols && step < step_max; step++) {
    const uint shift = mi_offset + step;
    const uint x_off = x + step * MI_SIZE;
    __global uchar* s = buf + y * cols + x_off;
    uint lfl;
    if (is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const cuda_loop_filter_thresh llft = lft[lfl];
    const uchar mblim = llft.mblim;
    const uchar lim = llft.lim;
    const uchar hev_thr = llft.hev_thr;
    const uint apply_16x16 = ((mask_16x16 >> shift) & 1);
    const uint apply_8x8 = ((mask_8x8 >> shift) & 1);
    const uint apply_4x4 = ((mask_4x4 >> shift) & 1);
    const uint apply_int_4x4 = ((mask_int_4x4 >> shift) & 1);

    if (apply_16x16 | apply_8x8 | apply_4x4) {
      filter_vertical_edge(s, mblim, lim, hev_thr, apply_16x16, apply_8x8);
    }
    if(apply_int_4x4) {
      filter_vertical_edge(s + 4, mblim, lim, hev_thr, 0, 0);
    }

    lfl_offset += 1;
  }
}

void filter_rows(__global const LOOP_FILTER_MASK* const lfm,
                 __global const cuda_loop_filter_thresh* const lft,
                 const uint sx,
                 const uint sy,
                 const uint sb_cols,
                 const uint rows,
                 const uint cols,
                 __global uchar * buf,
                 const bool is_y,
                 const uint step_max,
                 const uint lvl_shift,
                 const uint row_mult) {
  const uint x = sx * get_local_size(0) + get_local_id(0);
  const uint y = sy * get_local_size(0);
  uint step = 0;

  if (x >= cols) {
    return;
  }

  __global const LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
  ulong mask_16x16, mask_8x8, mask_4x4, mask_int_4x4;
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

  const uint mi_offset = get_local_id(0) / MI_SIZE;
  for (; y + step * MI_SIZE < rows && step < step_max; step++) {
    const uint shift = mi_offset + step * step_max;
    const uint y_off = y + step * MI_SIZE;
    __global uchar * s = buf + y_off * cols + x;

    // Calculate loop filter threshold
    const uint lfl_offset = (step * row_mult << lvl_shift) + mi_offset;
    uint lfl;
    if(is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const cuda_loop_filter_thresh llft = lft[lfl];
    const uchar mblim = llft.mblim;
    const uchar lim = llft.lim;
    const uchar hev_thr = llft.hev_thr;

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

__kernel
void filter_all(__global const LOOP_FILTER_MASK* const lfm,
                __global const cuda_loop_filter_thresh* const lft,
                __global volatile int *col_row_filtered,
                __global volatile int *col_col_filtered,
                const int sb_rows,
                const int sb_cols,
                const uint rows,
                const uint cols,
                __global uchar * buf,
                const uint is_y,
                const uint step_max,
                const uint lvl_shift,
                const uint row_mult) {
  const uint b_idx = get_group_id(0);
  if (b_idx == 0) {
    for (int c = 0; c < sb_cols; c++) {
      filter_cols(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_col_filtered[b_idx] = c;

      filter_rows(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_row_filtered[b_idx] = c;
    }
    col_row_filtered[b_idx] = sb_cols;
    col_col_filtered[b_idx] = sb_cols;
  }
  else {
    for (int c = 0; c < sb_cols; c++) {
      while(col_row_filtered[b_idx - 1] < c) {}
      filter_cols(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      while(col_col_filtered[b_idx - 1] < c + 1) {}
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_col_filtered[b_idx] = c;
      filter_rows(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_row_filtered[b_idx] = c;
    }
    col_row_filtered[b_idx] = sb_cols;
    col_col_filtered[b_idx] = sb_cols;
  }
}

__kernel
void fill_pattern(__global uint* p, uint pattern) {
  p[get_local_id(0)] = pattern;
}
