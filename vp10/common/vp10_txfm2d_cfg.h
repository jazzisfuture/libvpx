#ifndef TXFM2D_CFG_H_
#define TXFM2D_CFG_H_
#include "vp10/common/vp10_txfm_c.h"
typedef struct TXFM_2D_CFG {
  int txfm_size;
  int stage_num_col;
  int stage_num_row;

  int8_t *shift;
  int8_t *stage_range_col;
  int8_t *stage_range_row;
  int8_t *cos_bit_col;
  int8_t *cos_bit_row;
  TxfmFunc txfm_func_col;
  TxfmFunc txfm_func_row;
} TXFM_2D_CFG;

//  ---------------- config_dct_dct_4 ----------------
static int8_t fwd_shift_dct_dct_4[3] = {6, -4, 0};
static int8_t fwd_stage_range_col_dct_dct_4[4] = {17, 18, 19, 19};
static int8_t fwd_stage_range_row_dct_dct_4[4] = {15, 16, 16, 16};
static int8_t fwd_cos_bit_col_dct_dct_4[4] = {15, 14, 13, 13};
static int8_t fwd_cos_bit_row_dct_dct_4[4] = {16, 16, 16, 16};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_dct_4 = {
    .txfm_size = 4,
    .stage_num_col = 4,
    .stage_num_row = 4,

    .shift = fwd_shift_dct_dct_4,
    .stage_range_col = fwd_stage_range_col_dct_dct_4,
    .stage_range_row = fwd_stage_range_row_dct_dct_4,
    .cos_bit_col = fwd_cos_bit_col_dct_dct_4,
    .cos_bit_row = fwd_cos_bit_row_dct_dct_4,
    .txfm_func_col = vp10_fdct4_new,
    .txfm_func_row = vp10_fdct4_new};

static int8_t inv_shift_dct_dct_4[2] = {1, -5};
static int8_t inv_stage_range_col_dct_dct_4[4] = {17, 17, 16, 16};
static int8_t inv_stage_range_row_dct_dct_4[4] = {16, 16, 16, 16};
static int8_t inv_cos_bit_col_dct_dct_4[4] = {15, 15, 15, 16};
static int8_t inv_cos_bit_row_dct_dct_4[4] = {16, 16, 16, 16};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_4 = {
    .txfm_size = 4,
    .stage_num_col = 4,
    .stage_num_row = 4,

    .shift = inv_shift_dct_dct_4,
    .stage_range_col = inv_stage_range_col_dct_dct_4,
    .stage_range_row = inv_stage_range_row_dct_dct_4,
    .cos_bit_col = inv_cos_bit_col_dct_dct_4,
    .cos_bit_row = inv_cos_bit_row_dct_dct_4,
    .txfm_func_col = vp10_idct4_new,
    .txfm_func_row = vp10_idct4_new};

#endif  // TXFM2D_CFG_H_
