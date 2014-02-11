#ifndef LF_TYPES_H
#define LF_TYPES_H

typedef enum {
  TX_4X4 = 0,                      // 4x4 transform
  TX_8X8 = 1,                      // 8x8 transform
  TX_16X16 = 2,                    // 16x16 transform
  TX_32X32 = 3,                    // 32x32 transform
  TX_SIZES
} TX_SIZE;

#define MAX_LOOP_FILTER 63
#define MI_SIZE 8
#define Y_LFL_SHIFT 3
#define UV_LFL_SHIFT 1
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

#endif
