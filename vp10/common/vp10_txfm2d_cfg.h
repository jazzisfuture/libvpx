#ifndef TXFM2D_CFG_H_
#define TXFM2D_CFG_H_
#include "vp10_txfm_c.h"
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
    .txfm_func_col = fdct4,
    .txfm_func_row = fdct4};

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
    .txfm_func_col = idct4,
    .txfm_func_row = idct4};

//  ---------------- config_dct_dct_8 ----------------
static int8_t fwd_shift_dct_dct_8[3] = {6, -5, 0};
static int8_t fwd_stage_range_col_dct_dct_8[6] = {17, 18, 19, 20, 20, 20};
static int8_t fwd_stage_range_row_dct_dct_8[6] = {15, 16, 17, 17, 17, 17};
static int8_t fwd_cos_bit_col_dct_dct_8[6] = {15, 14, 13, 12, 12, 12};
static int8_t fwd_cos_bit_row_dct_dct_8[6] = {16, 16, 15, 15, 15, 15};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_dct_8 = {
    .txfm_size = 8,
    .stage_num_col = 6,
    .stage_num_row = 6,

    .shift = fwd_shift_dct_dct_8,
    .stage_range_col = fwd_stage_range_col_dct_dct_8,
    .stage_range_row = fwd_stage_range_row_dct_dct_8,
    .cos_bit_col = fwd_cos_bit_col_dct_dct_8,
    .cos_bit_row = fwd_cos_bit_row_dct_dct_8,
    .txfm_func_col = fdct8,
    .txfm_func_row = fdct8};

static int8_t inv_shift_dct_dct_8[2] = {-1, -4};
static int8_t inv_stage_range_col_dct_dct_8[6] = {16, 16, 16, 16, 15, 15};
static int8_t inv_stage_range_row_dct_dct_8[6] = {17, 17, 17, 17, 17, 17};
static int8_t inv_cos_bit_col_dct_dct_8[6] = {16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_row_dct_dct_8[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_8 = {
    .txfm_size = 8,
    .stage_num_col = 6,
    .stage_num_row = 6,

    .shift = inv_shift_dct_dct_8,
    .stage_range_col = inv_stage_range_col_dct_dct_8,
    .stage_range_row = inv_stage_range_row_dct_dct_8,
    .cos_bit_col = inv_cos_bit_col_dct_dct_8,
    .cos_bit_row = inv_cos_bit_row_dct_dct_8,
    .txfm_func_col = idct8,
    .txfm_func_row = idct8};

//  ---------------- config_dct_dct_16 ----------------
static int8_t fwd_shift_dct_dct_16[3] = {5, -3, -2};
static int8_t fwd_stage_range_col_dct_dct_16[8] = {16, 17, 18, 19,
                                                   20, 20, 20, 20};
static int8_t fwd_stage_range_row_dct_dct_16[8] = {17, 18, 19, 20,
                                                   20, 20, 20, 20};
static int8_t fwd_cos_bit_col_dct_dct_16[8] = {16, 15, 14, 13, 12, 12, 12, 12};
static int8_t fwd_cos_bit_row_dct_dct_16[8] = {15, 14, 13, 12, 12, 12, 12, 12};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_dct_16 = {
    .txfm_size = 16,
    .stage_num_col = 8,
    .stage_num_row = 8,

    .shift = fwd_shift_dct_dct_16,
    .stage_range_col = fwd_stage_range_col_dct_dct_16,
    .stage_range_row = fwd_stage_range_row_dct_dct_16,
    .cos_bit_col = fwd_cos_bit_col_dct_dct_16,
    .cos_bit_row = fwd_cos_bit_row_dct_dct_16,
    .txfm_func_col = fdct16,
    .txfm_func_row = fdct16};

static int8_t inv_shift_dct_dct_16[2] = {-1, -5};
static int8_t inv_stage_range_col_dct_dct_16[8] = {17, 17, 17, 17,
                                                   17, 17, 16, 16};
static int8_t inv_stage_range_row_dct_dct_16[8] = {18, 18, 18, 18,
                                                   18, 18, 18, 18};
static int8_t inv_cos_bit_col_dct_dct_16[8] = {15, 15, 15, 15, 15, 15, 15, 16};
static int8_t inv_cos_bit_row_dct_dct_16[8] = {14, 14, 14, 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_16 = {
    .txfm_size = 16,
    .stage_num_col = 8,
    .stage_num_row = 8,

    .shift = inv_shift_dct_dct_16,
    .stage_range_col = inv_stage_range_col_dct_dct_16,
    .stage_range_row = inv_stage_range_row_dct_dct_16,
    .cos_bit_col = inv_cos_bit_col_dct_dct_16,
    .cos_bit_row = inv_cos_bit_row_dct_dct_16,
    .txfm_func_col = idct16,
    .txfm_func_row = idct16};

//  ---------------- config_dct_dct_32 ----------------
static int8_t fwd_shift_dct_dct_32[3] = {3, -3, -2};
static int8_t fwd_stage_range_col_dct_dct_32[10] = {14, 15, 16, 17, 18,
                                                    19, 19, 19, 19, 19};
static int8_t fwd_stage_range_row_dct_dct_32[10] = {16, 17, 18, 19, 20,
                                                    20, 20, 20, 20, 20};
static int8_t fwd_cos_bit_col_dct_dct_32[10] = {16, 16, 16, 15, 14,
                                                13, 13, 13, 13, 13};
static int8_t fwd_cos_bit_row_dct_dct_32[10] = {16, 15, 14, 13, 12,
                                                12, 12, 12, 12, 12};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_dct_32 = {
    .txfm_size = 32,
    .stage_num_col = 10,
    .stage_num_row = 10,

    .shift = fwd_shift_dct_dct_32,
    .stage_range_col = fwd_stage_range_col_dct_dct_32,
    .stage_range_row = fwd_stage_range_row_dct_dct_32,
    .cos_bit_col = fwd_cos_bit_col_dct_dct_32,
    .cos_bit_row = fwd_cos_bit_row_dct_dct_32,
    .txfm_func_col = fdct32,
    .txfm_func_row = fdct32};

static int8_t inv_shift_dct_dct_32[2] = {0, -6};
static int8_t inv_stage_range_col_dct_dct_32[10] = {18, 18, 18, 18, 18,
                                                    18, 18, 18, 17, 17};
static int8_t inv_stage_range_row_dct_dct_32[10] = {18, 18, 18, 18, 18,
                                                    18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_dct_dct_32[10] = {14, 14, 14, 14, 14,
                                                14, 14, 14, 14, 15};
static int8_t inv_cos_bit_row_dct_dct_32[10] = {14, 14, 14, 14, 14,
                                                14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_32 = {
    .txfm_size = 32,
    .stage_num_col = 10,
    .stage_num_row = 10,

    .shift = inv_shift_dct_dct_32,
    .stage_range_col = inv_stage_range_col_dct_dct_32,
    .stage_range_row = inv_stage_range_row_dct_dct_32,
    .cos_bit_col = inv_cos_bit_col_dct_dct_32,
    .cos_bit_row = inv_cos_bit_row_dct_dct_32,
    .txfm_func_col = idct32,
    .txfm_func_row = idct32};

//  ---------------- config_dct_adst_4 ----------------
static int8_t fwd_shift_dct_adst_4[3] = {5, -2, -1};
static int8_t fwd_stage_range_col_dct_adst_4[4] = {16, 17, 18, 18};
static int8_t fwd_stage_range_row_dct_adst_4[6] = {16, 16, 16, 17, 17, 17};
static int8_t fwd_cos_bit_col_dct_adst_4[4] = {16, 15, 14, 14};
static int8_t fwd_cos_bit_row_dct_adst_4[6] = {16, 16, 16, 15, 15, 15};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_adst_4 = {
    .txfm_size = 4,
    .stage_num_col = 4,
    .stage_num_row = 6,

    .shift = fwd_shift_dct_adst_4,
    .stage_range_col = fwd_stage_range_col_dct_adst_4,
    .stage_range_row = fwd_stage_range_row_dct_adst_4,
    .cos_bit_col = fwd_cos_bit_col_dct_adst_4,
    .cos_bit_row = fwd_cos_bit_row_dct_adst_4,
    .txfm_func_col = fdct4,
    .txfm_func_row = fadst4};

static int8_t inv_shift_dct_adst_4[2] = {1, -5};
static int8_t inv_stage_range_col_dct_adst_4[4] = {17, 17, 16, 16};
static int8_t inv_stage_range_row_dct_adst_4[6] = {16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_col_dct_adst_4[4] = {15, 15, 15, 16};
static int8_t inv_cos_bit_row_dct_adst_4[6] = {16, 16, 16, 16, 16, 16};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_4 = {
    .txfm_size = 4,
    .stage_num_col = 4,
    .stage_num_row = 6,

    .shift = inv_shift_dct_adst_4,
    .stage_range_col = inv_stage_range_col_dct_adst_4,
    .stage_range_row = inv_stage_range_row_dct_adst_4,
    .cos_bit_col = inv_cos_bit_col_dct_adst_4,
    .cos_bit_row = inv_cos_bit_row_dct_adst_4,
    .txfm_func_col = idct4,
    .txfm_func_row = iadst4};

//  ---------------- config_dct_adst_8 ----------------
static int8_t fwd_shift_dct_adst_8[3] = {7, -3, -3};
static int8_t fwd_stage_range_col_dct_adst_8[6] = {18, 19, 20, 21, 21, 21};
static int8_t fwd_stage_range_row_dct_adst_8[8] = {18, 18, 18, 19,
                                                   19, 20, 20, 20};
static int8_t fwd_cos_bit_col_dct_adst_8[6] = {14, 13, 12, 11, 11, 11};
static int8_t fwd_cos_bit_row_dct_adst_8[8] = {14, 14, 14, 13, 13, 12, 12, 12};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_adst_8 = {
    .txfm_size = 8,
    .stage_num_col = 6,
    .stage_num_row = 8,

    .shift = fwd_shift_dct_adst_8,
    .stage_range_col = fwd_stage_range_col_dct_adst_8,
    .stage_range_row = fwd_stage_range_row_dct_adst_8,
    .cos_bit_col = fwd_cos_bit_col_dct_adst_8,
    .cos_bit_row = fwd_cos_bit_row_dct_adst_8,
    .txfm_func_col = fdct8,
    .txfm_func_row = fadst8};

static int8_t inv_shift_dct_adst_8[2] = {-1, -4};
static int8_t inv_stage_range_col_dct_adst_8[6] = {16, 16, 16, 16, 15, 15};
static int8_t inv_stage_range_row_dct_adst_8[8] = {17, 17, 17, 17,
                                                   17, 17, 17, 17};
static int8_t inv_cos_bit_col_dct_adst_8[6] = {16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_row_dct_adst_8[8] = {15, 15, 15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_8 = {
    .txfm_size = 8,
    .stage_num_col = 6,
    .stage_num_row = 8,

    .shift = inv_shift_dct_adst_8,
    .stage_range_col = inv_stage_range_col_dct_adst_8,
    .stage_range_row = inv_stage_range_row_dct_adst_8,
    .cos_bit_col = inv_cos_bit_col_dct_adst_8,
    .cos_bit_row = inv_cos_bit_row_dct_adst_8,
    .txfm_func_col = idct8,
    .txfm_func_row = iadst8};

//  ---------------- config_dct_adst_16 ----------------
static int8_t fwd_shift_dct_adst_16[3] = {4, -1, -3};
static int8_t fwd_stage_range_col_dct_adst_16[8] = {15, 16, 17, 18,
                                                    19, 19, 19, 19};
static int8_t fwd_stage_range_row_dct_adst_16[10] = {18, 18, 18, 19, 19,
                                                     20, 20, 21, 21, 21};
static int8_t fwd_cos_bit_col_dct_adst_16[8] = {16, 16, 15, 14, 13, 13, 13, 13};
static int8_t fwd_cos_bit_row_dct_adst_16[10] = {14, 14, 14, 13, 13,
                                                 12, 12, 11, 11, 11};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_adst_16 = {
    .txfm_size = 16,
    .stage_num_col = 8,
    .stage_num_row = 10,

    .shift = fwd_shift_dct_adst_16,
    .stage_range_col = fwd_stage_range_col_dct_adst_16,
    .stage_range_row = fwd_stage_range_row_dct_adst_16,
    .cos_bit_col = fwd_cos_bit_col_dct_adst_16,
    .cos_bit_row = fwd_cos_bit_row_dct_adst_16,
    .txfm_func_col = fdct16,
    .txfm_func_row = fadst16};

static int8_t inv_shift_dct_adst_16[2] = {1, -7};
static int8_t inv_stage_range_col_dct_adst_16[8] = {19, 19, 19, 19,
                                                    19, 19, 18, 18};
static int8_t inv_stage_range_row_dct_adst_16[10] = {18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_dct_adst_16[8] = {13, 13, 13, 13, 13, 13, 13, 14};
static int8_t inv_cos_bit_row_dct_adst_16[10] = {14, 14, 14, 14, 14,
                                                 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_16 = {
    .txfm_size = 16,
    .stage_num_col = 8,
    .stage_num_row = 10,

    .shift = inv_shift_dct_adst_16,
    .stage_range_col = inv_stage_range_col_dct_adst_16,
    .stage_range_row = inv_stage_range_row_dct_adst_16,
    .cos_bit_col = inv_cos_bit_col_dct_adst_16,
    .cos_bit_row = inv_cos_bit_row_dct_adst_16,
    .txfm_func_col = idct16,
    .txfm_func_row = iadst16};

//  ---------------- config_dct_adst_32 ----------------
static int8_t fwd_shift_dct_adst_32[3] = {3, -1, -4};
static int8_t fwd_stage_range_col_dct_adst_32[10] = {14, 15, 16, 17, 18,
                                                     19, 19, 19, 19, 19};
static int8_t fwd_stage_range_row_dct_adst_32[12] = {18, 18, 18, 19, 19, 20,
                                                     20, 21, 21, 22, 22, 22};
static int8_t fwd_cos_bit_col_dct_adst_32[10] = {16, 16, 16, 15, 14,
                                                 13, 13, 13, 13, 13};
static int8_t fwd_cos_bit_row_dct_adst_32[12] = {14, 14, 14, 13, 13, 12,
                                                 12, 11, 11, 10, 10, 10};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_dct_adst_32 = {
    .txfm_size = 32,
    .stage_num_col = 10,
    .stage_num_row = 12,

    .shift = fwd_shift_dct_adst_32,
    .stage_range_col = fwd_stage_range_col_dct_adst_32,
    .stage_range_row = fwd_stage_range_row_dct_adst_32,
    .cos_bit_col = fwd_cos_bit_col_dct_adst_32,
    .cos_bit_row = fwd_cos_bit_row_dct_adst_32,
    .txfm_func_col = fdct32,
    .txfm_func_row = fadst32};

static int8_t inv_shift_dct_adst_32[2] = {0, -6};
static int8_t inv_stage_range_col_dct_adst_32[10] = {18, 18, 18, 18, 18,
                                                     18, 18, 18, 17, 17};
static int8_t inv_stage_range_row_dct_adst_32[12] = {18, 18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_dct_adst_32[10] = {14, 14, 14, 14, 14,
                                                 14, 14, 14, 14, 15};
static int8_t inv_cos_bit_row_dct_adst_32[12] = {14, 14, 14, 14, 14, 14,
                                                 14, 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_32 = {
    .txfm_size = 32,
    .stage_num_col = 10,
    .stage_num_row = 12,

    .shift = inv_shift_dct_adst_32,
    .stage_range_col = inv_stage_range_col_dct_adst_32,
    .stage_range_row = inv_stage_range_row_dct_adst_32,
    .cos_bit_col = inv_cos_bit_col_dct_adst_32,
    .cos_bit_row = inv_cos_bit_row_dct_adst_32,
    .txfm_func_col = idct32,
    .txfm_func_row = iadst32};

//  ---------------- config_adst_adst_4 ----------------
static int8_t fwd_shift_adst_adst_4[3] = {6, 1, -5};
static int8_t fwd_stage_range_col_adst_adst_4[6] = {17, 17, 18, 19, 19, 19};
static int8_t fwd_stage_range_row_adst_adst_4[6] = {20, 20, 20, 21, 21, 21};
static int8_t fwd_cos_bit_col_adst_adst_4[6] = {15, 15, 14, 13, 13, 13};
static int8_t fwd_cos_bit_row_adst_adst_4[6] = {12, 12, 12, 11, 11, 11};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_adst_4 = {
    .txfm_size = 4,
    .stage_num_col = 6,
    .stage_num_row = 6,

    .shift = fwd_shift_adst_adst_4,
    .stage_range_col = fwd_stage_range_col_adst_adst_4,
    .stage_range_row = fwd_stage_range_row_adst_adst_4,
    .cos_bit_col = fwd_cos_bit_col_adst_adst_4,
    .cos_bit_row = fwd_cos_bit_row_adst_adst_4,
    .txfm_func_col = fadst4,
    .txfm_func_row = fadst4};

static int8_t inv_shift_adst_adst_4[2] = {0, -4};
static int8_t inv_stage_range_col_adst_adst_4[6] = {16, 16, 16, 16, 15, 15};
static int8_t inv_stage_range_row_adst_adst_4[6] = {16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_col_adst_adst_4[6] = {16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_row_adst_adst_4[6] = {16, 16, 16, 16, 16, 16};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_4 = {
    .txfm_size = 4,
    .stage_num_col = 6,
    .stage_num_row = 6,

    .shift = inv_shift_adst_adst_4,
    .stage_range_col = inv_stage_range_col_adst_adst_4,
    .stage_range_row = inv_stage_range_row_adst_adst_4,
    .cos_bit_col = inv_cos_bit_col_adst_adst_4,
    .cos_bit_row = inv_cos_bit_row_adst_adst_4,
    .txfm_func_col = iadst4,
    .txfm_func_row = iadst4};

//  ---------------- config_adst_adst_8 ----------------
static int8_t fwd_shift_adst_adst_8[3] = {3, -1, -1};
static int8_t fwd_stage_range_col_adst_adst_8[8] = {14, 14, 15, 16,
                                                    16, 17, 17, 17};
static int8_t fwd_stage_range_row_adst_adst_8[8] = {16, 16, 16, 17,
                                                    17, 18, 18, 18};
static int8_t fwd_cos_bit_col_adst_adst_8[8] = {16, 16, 16, 16, 16, 15, 15, 15};
static int8_t fwd_cos_bit_row_adst_adst_8[8] = {16, 16, 16, 15, 15, 14, 14, 14};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_adst_8 = {
    .txfm_size = 8,
    .stage_num_col = 8,
    .stage_num_row = 8,

    .shift = fwd_shift_adst_adst_8,
    .stage_range_col = fwd_stage_range_col_adst_adst_8,
    .stage_range_row = fwd_stage_range_row_adst_adst_8,
    .cos_bit_col = fwd_cos_bit_col_adst_adst_8,
    .cos_bit_row = fwd_cos_bit_row_adst_adst_8,
    .txfm_func_col = fadst8,
    .txfm_func_row = fadst8};

static int8_t inv_shift_adst_adst_8[2] = {-1, -4};
static int8_t inv_stage_range_col_adst_adst_8[8] = {16, 16, 16, 16,
                                                    16, 16, 15, 15};
static int8_t inv_stage_range_row_adst_adst_8[8] = {17, 17, 17, 17,
                                                    17, 17, 17, 17};
static int8_t inv_cos_bit_col_adst_adst_8[8] = {16, 16, 16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_row_adst_adst_8[8] = {15, 15, 15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_8 = {
    .txfm_size = 8,
    .stage_num_col = 8,
    .stage_num_row = 8,

    .shift = inv_shift_adst_adst_8,
    .stage_range_col = inv_stage_range_col_adst_adst_8,
    .stage_range_row = inv_stage_range_row_adst_adst_8,
    .cos_bit_col = inv_cos_bit_col_adst_adst_8,
    .cos_bit_row = inv_cos_bit_row_adst_adst_8,
    .txfm_func_col = iadst8,
    .txfm_func_row = iadst8};

//  ---------------- config_adst_adst_16 ----------------
static int8_t fwd_shift_adst_adst_16[3] = {2, 0, -2};
static int8_t fwd_stage_range_col_adst_adst_16[10] = {13, 13, 14, 15, 15,
                                                      16, 16, 17, 17, 17};
static int8_t fwd_stage_range_row_adst_adst_16[10] = {17, 17, 17, 18, 18,
                                                      19, 19, 20, 20, 20};
static int8_t fwd_cos_bit_col_adst_adst_16[10] = {16, 16, 16, 16, 16,
                                                  16, 16, 15, 15, 15};
static int8_t fwd_cos_bit_row_adst_adst_16[10] = {15, 15, 15, 14, 14,
                                                  13, 13, 12, 12, 12};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_adst_16 = {
    .txfm_size = 16,
    .stage_num_col = 10,
    .stage_num_row = 10,

    .shift = fwd_shift_adst_adst_16,
    .stage_range_col = fwd_stage_range_col_adst_adst_16,
    .stage_range_row = fwd_stage_range_row_adst_adst_16,
    .cos_bit_col = fwd_cos_bit_col_adst_adst_16,
    .cos_bit_row = fwd_cos_bit_row_adst_adst_16,
    .txfm_func_col = fadst16,
    .txfm_func_row = fadst16};

static int8_t inv_shift_adst_adst_16[2] = {0, -6};
static int8_t inv_stage_range_col_adst_adst_16[10] = {18, 18, 18, 18, 18,
                                                      18, 18, 18, 17, 17};
static int8_t inv_stage_range_row_adst_adst_16[10] = {18, 18, 18, 18, 18,
                                                      18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_adst_adst_16[10] = {14, 14, 14, 14, 14,
                                                  14, 14, 14, 14, 15};
static int8_t inv_cos_bit_row_adst_adst_16[10] = {14, 14, 14, 14, 14,
                                                  14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_16 = {
    .txfm_size = 16,
    .stage_num_col = 10,
    .stage_num_row = 10,

    .shift = inv_shift_adst_adst_16,
    .stage_range_col = inv_stage_range_col_adst_adst_16,
    .stage_range_row = inv_stage_range_row_adst_adst_16,
    .cos_bit_col = inv_cos_bit_col_adst_adst_16,
    .cos_bit_row = inv_cos_bit_row_adst_adst_16,
    .txfm_func_col = iadst16,
    .txfm_func_row = iadst16};

//  ---------------- config_adst_adst_32 ----------------
static int8_t fwd_shift_adst_adst_32[3] = {4, -2, -4};
static int8_t fwd_stage_range_col_adst_adst_32[12] = {15, 15, 16, 17, 17, 18,
                                                      18, 19, 19, 20, 20, 20};
static int8_t fwd_stage_range_row_adst_adst_32[12] = {18, 18, 18, 19, 19, 20,
                                                      20, 21, 21, 22, 22, 22};
static int8_t fwd_cos_bit_col_adst_adst_32[12] = {16, 16, 16, 15, 15, 14,
                                                  14, 13, 13, 12, 12, 12};
static int8_t fwd_cos_bit_row_adst_adst_32[12] = {14, 14, 14, 13, 13, 12,
                                                  12, 11, 11, 10, 10, 10};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_adst_32 = {
    .txfm_size = 32,
    .stage_num_col = 12,
    .stage_num_row = 12,

    .shift = fwd_shift_adst_adst_32,
    .stage_range_col = fwd_stage_range_col_adst_adst_32,
    .stage_range_row = fwd_stage_range_row_adst_adst_32,
    .cos_bit_col = fwd_cos_bit_col_adst_adst_32,
    .cos_bit_row = fwd_cos_bit_row_adst_adst_32,
    .txfm_func_col = fadst32,
    .txfm_func_row = fadst32};

static int8_t inv_shift_adst_adst_32[2] = {0, -6};
static int8_t inv_stage_range_col_adst_adst_32[12] = {18, 18, 18, 18, 18, 18,
                                                      18, 18, 18, 18, 17, 17};
static int8_t inv_stage_range_row_adst_adst_32[12] = {18, 18, 18, 18, 18, 18,
                                                      18, 18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_adst_adst_32[12] = {14, 14, 14, 14, 14, 14,
                                                  14, 14, 14, 14, 14, 15};
static int8_t inv_cos_bit_row_adst_adst_32[12] = {14, 14, 14, 14, 14, 14,
                                                  14, 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_32 = {
    .txfm_size = 32,
    .stage_num_col = 12,
    .stage_num_row = 12,

    .shift = inv_shift_adst_adst_32,
    .stage_range_col = inv_stage_range_col_adst_adst_32,
    .stage_range_row = inv_stage_range_row_adst_adst_32,
    .cos_bit_col = inv_cos_bit_col_adst_adst_32,
    .cos_bit_row = inv_cos_bit_row_adst_adst_32,
    .txfm_func_col = iadst32,
    .txfm_func_row = iadst32};

//  ---------------- config_adst_dct_4 ----------------
static int8_t fwd_shift_adst_dct_4[3] = {5, -4, 1};
static int8_t fwd_stage_range_col_adst_dct_4[6] = {16, 16, 17, 18, 18, 18};
static int8_t fwd_stage_range_row_adst_dct_4[4] = {14, 15, 15, 15};
static int8_t fwd_cos_bit_col_adst_dct_4[6] = {16, 16, 15, 14, 14, 14};
static int8_t fwd_cos_bit_row_adst_dct_4[4] = {16, 16, 16, 16};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_dct_4 = {
    .txfm_size = 4,
    .stage_num_col = 6,
    .stage_num_row = 4,

    .shift = fwd_shift_adst_dct_4,
    .stage_range_col = fwd_stage_range_col_adst_dct_4,
    .stage_range_row = fwd_stage_range_row_adst_dct_4,
    .cos_bit_col = fwd_cos_bit_col_adst_dct_4,
    .cos_bit_row = fwd_cos_bit_row_adst_dct_4,
    .txfm_func_col = fadst4,
    .txfm_func_row = fdct4};

static int8_t inv_shift_adst_dct_4[2] = {1, -5};
static int8_t inv_stage_range_col_adst_dct_4[6] = {17, 17, 17, 17, 16, 16};
static int8_t inv_stage_range_row_adst_dct_4[4] = {16, 16, 16, 16};
static int8_t inv_cos_bit_col_adst_dct_4[6] = {15, 15, 15, 15, 15, 16};
static int8_t inv_cos_bit_row_adst_dct_4[4] = {16, 16, 16, 16};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_4 = {
    .txfm_size = 4,
    .stage_num_col = 6,
    .stage_num_row = 4,

    .shift = inv_shift_adst_dct_4,
    .stage_range_col = inv_stage_range_col_adst_dct_4,
    .stage_range_row = inv_stage_range_row_adst_dct_4,
    .cos_bit_col = inv_cos_bit_col_adst_dct_4,
    .cos_bit_row = inv_cos_bit_row_adst_dct_4,
    .txfm_func_col = iadst4,
    .txfm_func_row = idct4};

//  ---------------- config_adst_dct_8 ----------------
static int8_t fwd_shift_adst_dct_8[3] = {5, 1, -5};
static int8_t fwd_stage_range_col_adst_dct_8[8] = {16, 16, 17, 18,
                                                   18, 19, 19, 19};
static int8_t fwd_stage_range_row_adst_dct_8[6] = {20, 21, 22, 22, 22, 22};
static int8_t fwd_cos_bit_col_adst_dct_8[8] = {16, 16, 15, 14, 14, 13, 13, 13};
static int8_t fwd_cos_bit_row_adst_dct_8[6] = {12, 11, 10, 10, 10, 10};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_dct_8 = {
    .txfm_size = 8,
    .stage_num_col = 8,
    .stage_num_row = 6,

    .shift = fwd_shift_adst_dct_8,
    .stage_range_col = fwd_stage_range_col_adst_dct_8,
    .stage_range_row = fwd_stage_range_row_adst_dct_8,
    .cos_bit_col = fwd_cos_bit_col_adst_dct_8,
    .cos_bit_row = fwd_cos_bit_row_adst_dct_8,
    .txfm_func_col = fadst8,
    .txfm_func_row = fdct8};

static int8_t inv_shift_adst_dct_8[2] = {-1, -4};
static int8_t inv_stage_range_col_adst_dct_8[8] = {16, 16, 16, 16,
                                                   16, 16, 15, 15};
static int8_t inv_stage_range_row_adst_dct_8[6] = {17, 17, 17, 17, 17, 17};
static int8_t inv_cos_bit_col_adst_dct_8[8] = {16, 16, 16, 16, 16, 16, 16, 16};
static int8_t inv_cos_bit_row_adst_dct_8[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_8 = {
    .txfm_size = 8,
    .stage_num_col = 8,
    .stage_num_row = 6,

    .shift = inv_shift_adst_dct_8,
    .stage_range_col = inv_stage_range_col_adst_dct_8,
    .stage_range_row = inv_stage_range_row_adst_dct_8,
    .cos_bit_col = inv_cos_bit_col_adst_dct_8,
    .cos_bit_row = inv_cos_bit_row_adst_dct_8,
    .txfm_func_col = iadst8,
    .txfm_func_row = idct8};

//  ---------------- config_adst_dct_16 ----------------
static int8_t fwd_shift_adst_dct_16[3] = {4, -3, -1};
static int8_t fwd_stage_range_col_adst_dct_16[10] = {15, 15, 16, 17, 17,
                                                     18, 18, 19, 19, 19};
static int8_t fwd_stage_range_row_adst_dct_16[8] = {16, 17, 18, 19,
                                                    19, 19, 19, 19};
static int8_t fwd_cos_bit_col_adst_dct_16[10] = {16, 16, 16, 15, 15,
                                                 14, 14, 13, 13, 13};
static int8_t fwd_cos_bit_row_adst_dct_16[8] = {16, 15, 14, 13, 13, 13, 13, 13};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_dct_16 = {
    .txfm_size = 16,
    .stage_num_col = 10,
    .stage_num_row = 8,

    .shift = fwd_shift_adst_dct_16,
    .stage_range_col = fwd_stage_range_col_adst_dct_16,
    .stage_range_row = fwd_stage_range_row_adst_dct_16,
    .cos_bit_col = fwd_cos_bit_col_adst_dct_16,
    .cos_bit_row = fwd_cos_bit_row_adst_dct_16,
    .txfm_func_col = fadst16,
    .txfm_func_row = fdct16};

static int8_t inv_shift_adst_dct_16[2] = {-1, -5};
static int8_t inv_stage_range_col_adst_dct_16[10] = {17, 17, 17, 17, 17,
                                                     17, 17, 17, 16, 16};
static int8_t inv_stage_range_row_adst_dct_16[8] = {18, 18, 18, 18,
                                                    18, 18, 18, 18};
static int8_t inv_cos_bit_col_adst_dct_16[10] = {15, 15, 15, 15, 15,
                                                 15, 15, 15, 15, 16};
static int8_t inv_cos_bit_row_adst_dct_16[8] = {14, 14, 14, 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_16 = {
    .txfm_size = 16,
    .stage_num_col = 10,
    .stage_num_row = 8,

    .shift = inv_shift_adst_dct_16,
    .stage_range_col = inv_stage_range_col_adst_dct_16,
    .stage_range_row = inv_stage_range_row_adst_dct_16,
    .cos_bit_col = inv_cos_bit_col_adst_dct_16,
    .cos_bit_row = inv_cos_bit_row_adst_dct_16,
    .txfm_func_col = iadst16,
    .txfm_func_row = idct16};

//  ---------------- config_adst_dct_32 ----------------
static int8_t fwd_shift_adst_dct_32[3] = {5, -4, -3};
static int8_t fwd_stage_range_col_adst_dct_32[12] = {16, 16, 17, 18, 18, 19,
                                                     19, 20, 20, 21, 21, 21};
static int8_t fwd_stage_range_row_adst_dct_32[10] = {17, 18, 19, 20, 21,
                                                     21, 21, 21, 21, 21};
static int8_t fwd_cos_bit_col_adst_dct_32[12] = {16, 16, 15, 14, 14, 13,
                                                 13, 12, 12, 11, 11, 11};
static int8_t fwd_cos_bit_row_adst_dct_32[10] = {15, 14, 13, 12, 11,
                                                 11, 11, 11, 11, 11};

static const TXFM_2D_CFG fwd_txfm_2d_cfg_adst_dct_32 = {
    .txfm_size = 32,
    .stage_num_col = 12,
    .stage_num_row = 10,

    .shift = fwd_shift_adst_dct_32,
    .stage_range_col = fwd_stage_range_col_adst_dct_32,
    .stage_range_row = fwd_stage_range_row_adst_dct_32,
    .cos_bit_col = fwd_cos_bit_col_adst_dct_32,
    .cos_bit_row = fwd_cos_bit_row_adst_dct_32,
    .txfm_func_col = fadst32,
    .txfm_func_row = fdct32};

static int8_t inv_shift_adst_dct_32[2] = {0, -6};
static int8_t inv_stage_range_col_adst_dct_32[12] = {18, 18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 17, 17};
static int8_t inv_stage_range_row_adst_dct_32[10] = {18, 18, 18, 18, 18,
                                                     18, 18, 18, 18, 18};
static int8_t inv_cos_bit_col_adst_dct_32[12] = {14, 14, 14, 14, 14, 14,
                                                 14, 14, 14, 14, 14, 15};
static int8_t inv_cos_bit_row_adst_dct_32[10] = {14, 14, 14, 14, 14,
                                                 14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_32 = {
    .txfm_size = 32,
    .stage_num_col = 12,
    .stage_num_row = 10,

    .shift = inv_shift_adst_dct_32,
    .stage_range_col = inv_stage_range_col_adst_dct_32,
    .stage_range_row = inv_stage_range_row_adst_dct_32,
    .cos_bit_col = inv_cos_bit_col_adst_dct_32,
    .cos_bit_row = inv_cos_bit_row_adst_dct_32,
    .txfm_func_col = iadst32,
    .txfm_func_row = idct32};

#endif  // TXFM2D_CFG_H_
