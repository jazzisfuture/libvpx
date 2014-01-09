#ifndef CUDA_LOOPFILTER_H
#define CUDA_LOOPFILTER_H

#ifdef __cplusplus
extern "C" {
#endif

// This structure holds bit masks for all 8x8 blocks in a 64x64 region.
// Each 1 bit represents a position in which we want to apply the loop filter.
// Left_ entries refer to whether we apply a filter on the border to the
// left of the block.   Above_ entries refer to whether or not to apply a
// filter on the above border.   Int_ entries refer to whether or not to
// apply borders on the 4x4 edges within the 8x8 block that each bit
// represents.
// Since each transform is accompanied by a potentially different type of
// loop filter there is a different entry in the array for each transform size.

// Need to align this structure so when it is declared and
// passed it can be loaded into vector registers.

typedef struct {
  uchar mblim;
  uchar lim;
  uchar hev_thr;
} cuda_loop_filter_thresh;

typedef struct {
  ulong left_y[TX_SIZES];
  ulong above_y[TX_SIZES];
  ulong int_4x4_y;
  ushort left_uv[TX_SIZES];
  ushort above_uv[TX_SIZES];
  ushort int_4x4_uv;
} lf_mask;

typedef struct {
  lf_mask masks;
  uchar lfl_y[64];
  uchar lfl_uv[16];
}  cuda_lf_mask;

typedef struct {
  ulong left_y[TX_SIZES];
  ulong above_y[TX_SIZES];
  ulong int_4x4_y;
  ushort left_uv[TX_SIZES];
  ushort above_uv[TX_SIZES];
  ushort int_4x4_uv;
  uchar lfl_y[64];
  uchar lfl_uv[16];
} LOOP_FILTER_MASK;

#ifdef __cplusplus
}
#endif
#endif
