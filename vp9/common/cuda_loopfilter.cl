#include "./vp9/common/cuda_loopfilter_def.h"
#include "./vp9/common/cuda_loopfilter.h"
#include "./vp9/common/cuda_loopfilters.h"

/* This filter can be applied to both the Y and UV planes.  We have
   to handle some slight differences between the mask setup and such
   for these planes.

   The actual algorithm moves in chunks of 8 pixels.  For each chunk,
   referred to as mi, we check the corresponding bit in all of the masks
   to see if we are supposed to apply a filter, and if so which.

   This function currently depends on there being a single thread per
   row of the image.  The threads sweep across a 64x64(y plane)
   or 32x32 block(uv plane) 8 pixels at a time, applying filters
   directly to global memory.  I tried caching, but the absolute
   maximum we ever read / write a single pixel is 3 times.  This is an
   extreme case which probably almost never happens(16 wide filters
   applied on both sides of a pixel, and some filter applied to the pixel
   itself).  Many pixels will have no filter applied and thus never need
   to be read or written.  The benefits of caching from a memory coalescing
   standpoint did not seem to be worth the upfront cost of caching, and the
   extra logic to sync.  Filtering vertical edges is worse than the rows
   from a perf standpoint due to memory coalescing, but transposing would be
   costly enough to offset any gains, especially because we would have to
   reverse the transformation between filtering vertical edges and
   horizontal edges.

   note: even if we are asked by the mask to apply a wider filter, we may
   end up applying a more narrow filter in certain cases.  I suspect this
   divergence is the chief source of perf issues, and have not found a way
   to keep all of the threads busy doing useful work.
 */

inline
void filter_vert(__global const LOOP_FILTER_MASK* const lfm,
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

    // calculate loop filter threshold
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

/* See general comments above.  In addition, the work group sweeps down the
   image to filter horizontal edges, one thread per column.*/

inline
void filter_horiz(__global const LOOP_FILTER_MASK* const lfm,
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

/* This algorithm applies the loop filter to all of the vertical and horizontal
   edges of an image.  Currently it operates on the full frame at once.  The
   vp9 codec breaks an image into 64x64 blocks, each of which may be further
   decomposed into blocks of 32x32, 16x16, 8x8, or 4x4.  Each of these blocks
   optionally has filters applied to its edges.  These filters may be 16 wide
   8 wide, or 4 wide.

  To calculate the value pixel value of an edge, we need to have final
  filtered pixel values to the left and top of a given pixel.  Additionally,
  we work our way across, and then down the image, in blocks of 64x64.  In
  this file we refer to blocks of 64x64 as SUPER_BLOCKS, or sb.  To filter
  a given super block at (x,y), we must have already completely filtered
  the super block at (x-1,y), and the block at (x, y-1).  Additionally,
  we must have already filtered the vertical edges of the super block
  at(x + 1, y-1) to start filtering the horizontal edges of (x,y).

  My original implementation of this algorithm maintained these constraints
  by working on a diagonal of super blocks, and I would only advance this
  diagonal by returning control to the cpu.  I then tried the implementation
  below which experimentation has found to be faster.

  The below algorithm launches a single kernel, and the constraints are
  maintained using global memory and barriers.

  The work group which starts filtering at super block which starts at (0,0),
  has no dependencies other than itself.  It simply filters its own vertical
  edges, updates the global mutex, filters its own horizontal edges, updates
  its mutex, and then it advances to (1,0) and repeats.

  All of the other work groups also start filtering super blocks starting
  at (0, N), and poll the global mutex until the work group at (0, N-1) has
  completed filtering.  They then filter the vertical edges of (0, N), update
  the global mutex and begin polling the mutex to see if the work group
  filtering super block (1, N-1), has finished filtering its vertical edges.
  When it has, the work group at (0, N) filters its horizontal edges, updates
  the mutex and moves to the next super block in the row, (1, N).
*/

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
      filter_vert(lfm, lft, c, b_idx, sb_cols, rows, cols, buf,
        is_y, step_max, lvl_shift, row_mult);
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_col_filtered[b_idx] = c;

      filter_horiz(lfm, lft, c, b_idx, sb_cols, rows, cols, buf,
        is_y, step_max, lvl_shift, row_mult);
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_row_filtered[b_idx] = c;
    }
    col_row_filtered[b_idx] = sb_cols;
    col_col_filtered[b_idx] = sb_cols;
  }
  else {
    for (int c = 0; c < sb_cols; c++) {
      while(col_row_filtered[b_idx - 1] < c) {}
      filter_vert(lfm, lft, c, b_idx, sb_cols, rows, cols, buf,
        is_y, step_max, lvl_shift, row_mult);
      while(col_col_filtered[b_idx - 1] < c + 1) {}
      barrier(CLK_GLOBAL_MEM_FENCE);
      col_col_filtered[b_idx] = c;
      filter_horiz(lfm, lft, c, b_idx, sb_cols, rows, cols, buf,
        is_y, step_max, lvl_shift, row_mult);
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
