#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <cuda.h>
#include <cuda_runtime.h>
#include <assert.h>
#include "cuda_loopfilter_def.h"
#include "cuda_loopfilter.h"
#include "cuda_loopfilters.cu"

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__forceinline__ __device__
void filter_cols(const LOOP_FILTER_MASK* const lfm,
                 const cuda_loop_filter_thresh* const lft,
                 const uint32_t sx,
                 const uint32_t sy,
                 const uint32_t sb_cols,
                 const uint32_t rows,
                 const uint32_t cols,
                 uint8_t * buf,
                 const bool is_y,
                 const uint32_t step_max,
                 const uint32_t lvl_shift,
                 const uint32_t row_mult) {
  const uint32_t x = sx * blockDim.x;
  const uint32_t y = sy * blockDim.x + threadIdx.x;
  uint32_t step = 0;

  if (y >= rows) {
    return;
  }

  const LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
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

  const uint32_t row = threadIdx.x / MI_SIZE;
  const uint32_t mi_offset = row * step_max;
  uint32_t lfl_offset = row * row_mult << lvl_shift;

  for (; x + step * MI_SIZE < cols && step < step_max; step++) {
    const uint32_t shift = mi_offset + step;
    const uint32_t x_off = x + step * MI_SIZE;
    uint8_t* s = buf + y * cols + x_off;
    uint32_t lfl;
    if (is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const cuda_loop_filter_thresh llft = lft[lfl];
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

__forceinline__ __device__
void filter_rows(const LOOP_FILTER_MASK* const lfm,
                 const cuda_loop_filter_thresh* const lft,
                 const uint32_t sx,
                 const uint32_t sy,
                 const uint32_t sb_cols,
                 const uint32_t rows,
                 const uint32_t cols,
                 uint8_t * buf,
                 const bool is_y,
                 const uint32_t step_max,
                 const uint32_t lvl_shift,
                 const uint32_t row_mult) {
  const uint32_t x = sx * blockDim.x + threadIdx.x;
  const uint32_t y = sy * blockDim.x;
  uint32_t step = 0;

  if (x >= cols) {
    return;
  }

  const LOOP_FILTER_MASK* const mask = &lfm[sy*sb_cols + sx];
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

  const uint32_t mi_offset = threadIdx.x / MI_SIZE;
  for (; y + step * MI_SIZE < rows && step < step_max; step++) {
    const uint32_t shift = mi_offset + step * step_max;
    const uint32_t y_off = y + step * MI_SIZE;
    uint8_t * s = buf + y_off * cols + x;

    // Calculate loop filter threshold
    const uint32_t lfl_offset = (step * row_mult << lvl_shift) + mi_offset;
    uint32_t lfl;
    if(is_y) {
      lfl = mask->lfl_y[lfl_offset];
    } else {
      lfl = mask->lfl_uv[lfl_offset];
    }
    const cuda_loop_filter_thresh llft = lft[lfl];
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

__global__
void filter_all(const LOOP_FILTER_MASK* const lfm,
                const cuda_loop_filter_thresh* const lft,
                volatile int32_t *col_row_filtered,
                volatile int32_t *col_col_filtered,
                const int32_t sb_rows,
                const int32_t sb_cols,
                const uint32_t rows,
                const uint32_t cols,
                uint8_t * buf,
                const bool is_y,
                const uint32_t step_max,
                const uint32_t lvl_shift,
                const uint32_t row_mult) {
  const uint32_t b_idx = blockIdx.x;
  if (b_idx == 0) {
    for (int32_t c = 0; c < sb_cols; c++) {
      filter_cols(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      __syncthreads();
      col_col_filtered[b_idx] = c;

      filter_rows(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      __syncthreads();
      col_row_filtered[b_idx] = c;
    }
    col_row_filtered[b_idx] = sb_cols;
    col_col_filtered[b_idx] = sb_cols;
  }
  else {
    for (int32_t c = 0; c < sb_cols; c++) {
      while(col_row_filtered[b_idx - 1] < c) {}
      filter_cols(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      while(col_col_filtered[b_idx - 1] < c + 1) {}
      __syncthreads();
      col_col_filtered[b_idx] = c;
      filter_rows(lfm, lft, c, b_idx, sb_cols, rows, cols, buf, is_y,
                  step_max, lvl_shift, row_mult);
      __syncthreads();
      col_row_filtered[b_idx] = c;
    }
    col_row_filtered[b_idx] = sb_cols;
    col_col_filtered[b_idx] = sb_cols;
  }
}

extern "C" {

/*
 * cuda_loopfilter takes a fully decode frame, buf, as well as an array of
 * masks, and the frame dimensions.  buf is assumed to be sizeof(rows * cols)
 */
void cuda_loopfilter(const LOOP_FILTER_MASK* lfm,
                     const cuda_loop_filter_thresh* lft,
                     uint32_t y_rows,
                     uint32_t y_cols,
                     uint8_t * y_buf,
                     uint32_t uv_rows,
                     uint32_t uv_cols,
                     uint8_t * u_buf,
                     uint8_t * v_buf) {
  unsigned char    *d_y_buf;
  unsigned char    *d_u_buf;
  unsigned char    *d_v_buf;
  int32_t * d_col_col_y_filtered,* d_col_col_u_filtered, *d_col_col_v_filtered;
  int32_t * d_col_row_y_filtered,* d_col_row_u_filtered, *d_col_row_v_filtered;

  static double total_time = 0;
  LOOP_FILTER_MASK* d_masks;
  cuda_loop_filter_thresh* d_lft;
  uint32_t y_buf_size = y_rows * y_cols * sizeof(uint8_t);
  uint32_t uv_buf_size = uv_rows * uv_cols * sizeof(uint8_t);
  cudaStream_t y_s, u_s, v_s, gen1, gen2;
  cudaStreamCreate(&y_s);
  cudaStreamCreate(&u_s);
  cudaStreamCreate(&v_s);
  cudaStreamCreate(&gen1);
  cudaStreamCreate(&gen2);
  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start);

  gpuErrchk(cudaMalloc( (void**)&d_y_buf, y_buf_size));
  gpuErrchk(cudaMemcpyAsync(d_y_buf, (void*)y_buf, y_buf_size,
    cudaMemcpyHostToDevice, y_s));

  gpuErrchk(cudaMalloc( (void**)&d_u_buf, uv_buf_size));
  gpuErrchk(cudaMemcpyAsync(d_u_buf, (void*)u_buf, uv_buf_size,
    cudaMemcpyHostToDevice, u_s));

  gpuErrchk(cudaMalloc( (void**)&d_v_buf, uv_buf_size));
  gpuErrchk(cudaMemcpyAsync(d_v_buf, (void*)v_buf, uv_buf_size,
    cudaMemcpyHostToDevice, v_s));

  gpuErrchk(cudaMalloc( (void**)&d_lft,
    (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh)));
  gpuErrchk(cudaMemcpyAsync(d_lft, (void*)lft,
    (MAX_LOOP_FILTER + 1) * sizeof(cuda_loop_filter_thresh),
    cudaMemcpyHostToDevice, gen1));

  uint32_t sb_rows = (y_rows + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_cols = (y_cols + SUPER_BLOCK_DIM - 1) / SUPER_BLOCK_DIM;
  uint32_t sb_count = sb_rows * sb_cols;
  uint32_t mask_bytes = sb_count * sizeof(LOOP_FILTER_MASK);
  gpuErrchk(cudaMalloc(&d_masks, mask_bytes));
  gpuErrchk(cudaMemcpyAsync(d_masks, (void*)lfm, mask_bytes,
    cudaMemcpyHostToDevice, gen2));

  gpuErrchk(cudaMalloc( (void**)&d_col_row_y_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMalloc( (void**)&d_col_col_y_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMemsetAsync(d_col_row_y_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), y_s))
  gpuErrchk(cudaMemsetAsync(d_col_col_y_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), y_s))

  gpuErrchk(cudaMalloc( (void**)&d_col_row_u_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMalloc( (void**)&d_col_col_u_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMemsetAsync(d_col_row_u_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), u_s))
  gpuErrchk(cudaMemsetAsync(d_col_col_u_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), u_s))

  gpuErrchk(cudaMalloc( (void**)&d_col_row_v_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMalloc( (void**)&d_col_col_v_filtered,
    sb_rows * sizeof(int32_t)));
  gpuErrchk(cudaMemsetAsync(d_col_row_v_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), v_s))
  gpuErrchk(cudaMemsetAsync(d_col_col_v_filtered, 0xFFFFFFFF,
    sb_rows * sizeof(int32_t), v_s))
  // iterate over all diagonals

  dim3 work_block(SUPER_BLOCK_DIM);
  dim3 uv_work_block(SUPER_BLOCK_DIM / 2);
  dim3 work_grid(sb_rows);
  // First filter all left most columns on the diagonal frontier
  // These are the left most columns for each super block
  filter_all<<<work_grid, work_block,0,y_s>>>(d_masks, d_lft,
    d_col_row_y_filtered, d_col_col_y_filtered, sb_rows, sb_cols, y_rows,
    y_cols, d_y_buf, true, MI_SIZE, Y_LFL_SHIFT, 1);
  filter_all<<<work_grid, uv_work_block,0,u_s>>>(d_masks, d_lft,
    d_col_row_u_filtered, d_col_col_u_filtered, sb_rows, sb_cols, uv_rows,
    uv_cols, d_u_buf, false, MI_SIZE / 2, UV_LFL_SHIFT, 2);
  filter_all<<<work_grid, uv_work_block,0,v_s>>>(d_masks, d_lft,
    d_col_row_v_filtered, d_col_col_v_filtered, sb_rows, sb_cols, uv_rows,
    uv_cols, d_v_buf, false, MI_SIZE / 2, UV_LFL_SHIFT, 2);

  gpuErrchk(cudaMemcpyAsync(y_buf, d_y_buf, y_buf_size,
    cudaMemcpyDeviceToHost, y_s));

  gpuErrchk(cudaMemcpyAsync(u_buf, d_u_buf, uv_buf_size,
    cudaMemcpyDeviceToHost, u_s));
  gpuErrchk(cudaMemcpyAsync(v_buf, d_v_buf, uv_buf_size,
    cudaMemcpyDeviceToHost, v_s) );
  cudaStreamDestroy(y_s);
  cudaStreamDestroy(u_s);
  cudaStreamDestroy(v_s);
  cudaStreamDestroy(gen1);
  cudaStreamDestroy(gen2);
  cudaFree(d_y_buf);
  cudaFree(d_u_buf);
  cudaFree(d_v_buf);
  cudaFree(d_masks);
  cudaFree(d_lft);
  cudaFree(d_col_col_y_filtered);
  cudaFree(d_col_row_y_filtered);
  cudaFree(d_col_col_u_filtered);
  cudaFree(d_col_row_u_filtered);
  cudaFree(d_col_col_v_filtered);
  cudaFree(d_col_row_v_filtered);

  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  float gpu_time;
  cudaEventElapsedTime(&gpu_time, start, stop);
  total_time += gpu_time;
  printf("Elapsed GPU timer: %dX%d %.5fms, total_time %.5fms\n", y_cols,
    y_rows, gpu_time, total_time);
  cudaEventDestroy(start);
  cudaEventDestroy(stop);
}
}

