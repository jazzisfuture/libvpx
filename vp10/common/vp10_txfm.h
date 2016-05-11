/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef VP10_TXFM_H_
#define VP10_TXFM_H_

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "vpx/vpx_integer.h"
#include "vpx_dsp/vpx_dsp_common.h"

static const int cos_bit_min = 10;
static const int cos_bit_max = 16;

// cospi_arr[i][j] = (int)round(cos(M_PI*j/128) * (1<<(cos_bit_min+i)));
static const int32_t cospi_arr[7][64] =
  {{ 1024,  1024,  1023,  1021,  1019,  1016,  1013,  1009,
     1004,   999,   993,   987,   980,   972,   964,   955,
      946,   936,   926,   915,   903,   891,   878,   865,
      851,   837,   822,   807,   792,   775,   759,   742,
      724,   706,   688,   669,   650,   630,   610,   590,
      569,   548,   526,   505,   483,   460,   438,   415,
      392,   369,   345,   321,   297,   273,   249,   224,
      200,   175,   150,   125,   100,    75,    50,    25},
  {  2048,  2047,  2046,  2042,  2038,  2033,  2026,  2018,
     2009,  1998,  1987,  1974,  1960,  1945,  1928,  1911,
     1892,  1872,  1851,  1829,  1806,  1782,  1757,  1730,
     1703,  1674,  1645,  1615,  1583,  1551,  1517,  1483,
     1448,  1412,  1375,  1338,  1299,  1260,  1220,  1179,
     1138,  1096,  1053,  1009,   965,   921,   876,   830,
      784,   737,   690,   642,   595,   546,   498,   449,
      400,   350,   301,   251,   201,   151,   100,    50},
  {  4096,  4095,  4091,  4085,  4076,  4065,  4052,  4036,
     4017,  3996,  3973,  3948,  3920,  3889,  3857,  3822,
     3784,  3745,  3703,  3659,  3612,  3564,  3513,  3461,
     3406,  3349,  3290,  3229,  3166,  3102,  3035,  2967,
     2896,  2824,  2751,  2675,  2598,  2520,  2440,  2359,
     2276,  2191,  2106,  2019,  1931,  1842,  1751,  1660,
     1567,  1474,  1380,  1285,  1189,  1092,   995,   897,
      799,   700,   601,   501,   401,   301,   201,   101},
  {  8192,  8190,  8182,  8170,  8153,  8130,  8103,  8071,
     8035,  7993,  7946,  7895,  7839,  7779,  7713,  7643,
     7568,  7489,  7405,  7317,  7225,  7128,  7027,  6921,
     6811,  6698,  6580,  6458,  6333,  6203,  6070,  5933,
     5793,  5649,  5501,  5351,  5197,  5040,  4880,  4717,
     4551,  4383,  4212,  4038,  3862,  3683,  3503,  3320,
     3135,  2948,  2760,  2570,  2378,  2185,  1990,  1795,
     1598,  1401,  1202,  1003,   803,   603,   402,   201},
  { 16384, 16379, 16364, 16340, 16305, 16261, 16207, 16143,
    16069, 15986, 15893, 15791, 15679, 15557, 15426, 15286,
    15137, 14978, 14811, 14635, 14449, 14256, 14053, 13842,
    13623, 13395, 13160, 12916, 12665, 12406, 12140, 11866,
    11585, 11297, 11003, 10702, 10394, 10080,  9760,  9434,
     9102,  8765,  8423,  8076,  7723,  7366,  7005,  6639,
     6270,  5897,  5520,  5139,  4756,  4370,  3981,  3590,
     3196,  2801,  2404,  2006,  1606,  1205,   804,   402},
  { 32768, 32758, 32729, 32679, 32610, 32522, 32413, 32286,
    32138, 31972, 31786, 31581, 31357, 31114, 30853, 30572,
    30274, 29957, 29622, 29269, 28899, 28511, 28106, 27684,
    27246, 26791, 26320, 25833, 25330, 24812, 24279, 23732,
    23170, 22595, 22006, 21403, 20788, 20160, 19520, 18868,
    18205, 17531, 16846, 16151, 15447, 14733, 14010, 13279,
    12540, 11793, 11039, 10279,  9512,  8740,  7962,  7180,
     6393,  5602,  4808,  4011,  3212,  2411,  1608,   804},
  { 65536, 65516, 65457, 65358, 65220, 65043, 64827, 64571,
    64277, 63944, 63572, 63162, 62714, 62228, 61705, 61145,
    60547, 59914, 59244, 58538, 57798, 57022, 56212, 55368,
    54491, 53581, 52639, 51665, 50660, 49624, 48559, 47464,
    46341, 45190, 44011, 42806, 41576, 40320, 39040, 37736,
    36410, 35062, 33692, 32303, 30893, 29466, 28020, 26558,
    25080, 23586, 22078, 20557, 19024, 17479, 15924, 14359,
    12785, 11204,  9616,  8022,  6424,  4821,  3216,  1608}};

static INLINE int32_t round_shift(int32_t value, int bit) {
  return (value + (1 << (bit - 1))) >> bit;
}

static INLINE void round_shift_array(int32_t *arr, int size, int bit) {
  int i;
  if (bit == 0) {
    return;
  } else {
    if (bit > 0) {
      for (i = 0; i < size; i++) {
        arr[i] = round_shift(arr[i], bit);
      }
    } else {
      for (i = 0; i < size; i++) {
        arr[i] = arr[i] << (-bit);
      }
    }
  }
}

static INLINE int32_t half_btf(int32_t w0, int32_t in0, int32_t w1, int32_t in1,
                               int bit) {
  int32_t result_32 = w0 * in0 + w1 * in1;
#if CONFIG_COEFFICIENT_RANGE_CHECKING
  int64_t result_64 = (int64_t)w0 * (int64_t)in0 + (int64_t)w1 * (int64_t)in1;
  if (result_32 != result_64) {
    printf(
        "%s overflow result_32: %d result_64: %lld w0: %d in0: %d w1: %d in1: "
        "%d\n",
        __func__, result_32, (long long int)result_64, w0, in0, w1, in1);
    assert(0 && "half_btf overflow");
  }
#endif
  return round_shift(result_32, bit);
}

static INLINE int get_max_bit(int x) {
  int max_bit = -1;
  while (x) {
    x = x >> 1;
    max_bit++;
  }
  return max_bit;
}

// TODO(angiebird): implement SSE
static INLINE void clamp_block(int16_t *block, int block_size, int stride,
                               int low, int high) {
  int i, j;
  for (i = 0; i < block_size; ++i) {
    for (j = 0; j < block_size; ++j) {
      block[i * stride + j] = clamp(block[i * stride + j], low, high);
    }
  }
}

typedef void (*TxfmFunc)(const int32_t *input, int32_t *output,
                         const int8_t *cos_bit, const int8_t *stage_range);

typedef enum TXFM_TYPE {
  TXFM_TYPE_DCT4,
  TXFM_TYPE_DCT8,
  TXFM_TYPE_DCT16,
  TXFM_TYPE_DCT32,
  TXFM_TYPE_DCT64,
  TXFM_TYPE_ADST4,
  TXFM_TYPE_ADST8,
  TXFM_TYPE_ADST16,
  TXFM_TYPE_ADST32,
} TXFM_TYPE;

typedef struct TXFM_2D_CFG {
  const int txfm_size;
  const int stage_num_col;
  const int stage_num_row;

  const int8_t *shift;
  const int8_t *stage_range_col;
  const int8_t *stage_range_row;
  const int8_t *cos_bit_col;
  const int8_t *cos_bit_row;
  const TXFM_TYPE txfm_type_col;
  const TXFM_TYPE txfm_type_row;
} TXFM_2D_CFG;

typedef struct TXFM_2D_FLIP_CFG {
  int ud_flip;  // flip upside down
  int lr_flip;  // flip left to right
  const TXFM_2D_CFG* cfg;
} TXFM_2D_FLIP_CFG;

#ifdef __cplusplus
extern "C" {
#endif
TXFM_2D_FLIP_CFG vp10_get_fwd_txfm_cfg(int tx_type, int tx_size);
TXFM_2D_FLIP_CFG vp10_get_fwd_txfm_64x64_cfg(int tx_type);
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // VP10_TXFM_H_
