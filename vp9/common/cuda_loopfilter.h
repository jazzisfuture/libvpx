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

#define THRESH_WIDTH 16
typedef struct {
  uint8_t mblim;
  uint8_t lim;
  uint8_t hev_thr;
} cuda_loop_filter_thresh;

typedef struct {
  uint64_t left_y[TX_SIZES];
  uint64_t above_y[TX_SIZES];
  uint64_t int_4x4_y;
  uint16_t left_uv[TX_SIZES];
  uint16_t above_uv[TX_SIZES];
  uint16_t int_4x4_uv;
} lf_mask;

typedef struct {
  lf_mask masks;
  uint8_t lfl_y[64];
  uint8_t lfl_uv[16];
}  cuda_lf_mask;

typedef struct {
  uint64_t left_y[TX_SIZES];
  uint64_t above_y[TX_SIZES];
  uint64_t int_4x4_y;
  uint16_t left_uv[TX_SIZES];
  uint16_t above_uv[TX_SIZES];
  uint16_t int_4x4_uv;
  uint8_t lfl_y[64];
  uint8_t lfl_uv[16];
} LOOP_FILTER_MASK;

void cuda_loopfilter(const LOOP_FILTER_MASK* lfm,
                     const cuda_loop_filter_thresh* lft,
                     uint32_t y_rows,
                     uint32_t y_cols,
                     uint8_t * y_buf,
                     uint32_t uv_rows,
                     uint32_t uv_cols,
                     uint8_t * u_buf,
                     uint8_t * v_buf);
#ifdef __cplusplus
}
#endif
#endif
