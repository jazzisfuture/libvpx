/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"

#if CONFIG_VP9_HIGHBITDEPTH
#include "vpx_dsp/vpx_dsp_common.h"
#endif  // CONFIG_VP9_HIGHBITDEPTH
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "vpx_ports/vpx_once.h"

#include "vp10/common/reconintra.h"
#include "vp10/common/onyxc_int.h"

enum {
  NEED_LEFT = 1 << 1,
  NEED_ABOVE = 1 << 2,
  NEED_ABOVERIGHT = 1 << 3,
};

static const uint8_t extend_modes[INTRA_MODES] = {
  NEED_ABOVE | NEED_LEFT,       // DC
  NEED_ABOVE,                   // V
  NEED_LEFT,                    // H
  NEED_ABOVERIGHT,              // D45
  NEED_LEFT | NEED_ABOVE,       // D135
  NEED_LEFT | NEED_ABOVE,       // D117
  NEED_LEFT | NEED_ABOVE,       // D153
  NEED_LEFT,                    // D207
  NEED_ABOVERIGHT,              // D63
  NEED_LEFT | NEED_ABOVE,       // TM
};

#if CONFIG_EXT_INTRA
static const uint8_t ext_intra_extend_modes[EXT_INTRA_MODES] = {
  NEED_ABOVERIGHT,              // D76
  NEED_LEFT | NEED_ABOVE,       // D104
  NEED_LEFT | NEED_ABOVE,       // D166
  NEED_LEFT,                    // D194
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_DC
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_V
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_H
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D45
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D135
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D117
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D153
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D207
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_D63
  NEED_LEFT | NEED_ABOVERIGHT,  // FILTER_TM
};
#endif  // CONFIG_EXT_INTRA

typedef void (*intra_pred_fn)(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);

static intra_pred_fn pred[INTRA_MODES][TX_SIZES];
static intra_pred_fn dc_pred[2][2][TX_SIZES];

#if CONFIG_VP9_HIGHBITDEPTH
typedef void (*intra_high_pred_fn)(uint16_t *dst, ptrdiff_t stride,
                                   const uint16_t *above, const uint16_t *left,
                                   int bd);
static intra_high_pred_fn pred_high[INTRA_MODES][4];
static intra_high_pred_fn dc_pred_high[2][2][4];
#endif  // CONFIG_VP9_HIGHBITDEPTH

static void vp10_init_intra_predictors_internal(void) {
#define INIT_ALL_SIZES(p, type) \
  p[TX_4X4] = vpx_##type##_predictor_4x4; \
  p[TX_8X8] = vpx_##type##_predictor_8x8; \
  p[TX_16X16] = vpx_##type##_predictor_16x16; \
  p[TX_32X32] = vpx_##type##_predictor_32x32

  INIT_ALL_SIZES(pred[V_PRED], v);
  INIT_ALL_SIZES(pred[H_PRED], h);
  INIT_ALL_SIZES(pred[D207_PRED], d207);
  INIT_ALL_SIZES(pred[D45_PRED], d45);
  INIT_ALL_SIZES(pred[D63_PRED], d63);
  INIT_ALL_SIZES(pred[D117_PRED], d117);
  INIT_ALL_SIZES(pred[D135_PRED], d135);
  INIT_ALL_SIZES(pred[D153_PRED], d153);
  INIT_ALL_SIZES(pred[TM_PRED], tm);

  INIT_ALL_SIZES(dc_pred[0][0], dc_128);
  INIT_ALL_SIZES(dc_pred[0][1], dc_top);
  INIT_ALL_SIZES(dc_pred[1][0], dc_left);
  INIT_ALL_SIZES(dc_pred[1][1], dc);

#if CONFIG_VP9_HIGHBITDEPTH
  INIT_ALL_SIZES(pred_high[V_PRED], highbd_v);
  INIT_ALL_SIZES(pred_high[H_PRED], highbd_h);
  INIT_ALL_SIZES(pred_high[D207_PRED], highbd_d207);
  INIT_ALL_SIZES(pred_high[D45_PRED], highbd_d45);
  INIT_ALL_SIZES(pred_high[D63_PRED], highbd_d63);
  INIT_ALL_SIZES(pred_high[D117_PRED], highbd_d117);
  INIT_ALL_SIZES(pred_high[D135_PRED], highbd_d135);
  INIT_ALL_SIZES(pred_high[D153_PRED], highbd_d153);
  INIT_ALL_SIZES(pred_high[TM_PRED], highbd_tm);

  INIT_ALL_SIZES(dc_pred_high[0][0], highbd_dc_128);
  INIT_ALL_SIZES(dc_pred_high[0][1], highbd_dc_top);
  INIT_ALL_SIZES(dc_pred_high[1][0], highbd_dc_left);
  INIT_ALL_SIZES(dc_pred_high[1][1], highbd_dc);
#endif  // CONFIG_VP9_HIGHBITDEPTH

#undef intra_pred_allsizes
}

#if CONFIG_EXT_INTRA
#define AVG3(a, b, c) (((a) + 2 * (b) + (c) + 2) >> 2)
#define FILTER_INTRA_PREC_BITS 10
#define FILTER_INTRA_ROUND_VAL 511

static INLINE void d76_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
  int r, c, i;
  uint8_t buf[4][48];
  (void)left;

  for (c = 0; c < bs + bs / 4; ++c) {
    buf[0][c] = AVG3(above[c], above[c], above[c + 1]);
    buf[1][c] = AVG3(above[c + 1], above[c], above[c + 1]);
    buf[2][c] = AVG3(above[c], above[c + 1], above[c + 1]);
    buf[3][c] = above[c + 1];
  }

  for (r = 0; r < bs; r += 4) {
    for (i = 0; i < 4; ++i)
      memcpy(dst + i * stride, buf[i] + (r >> 2), bs * sizeof(*dst));
    dst += 4 * stride;
  }
}

static INLINE void d104_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r, c;

  // first 4 rows
  for (c = 0; c < bs; c++) {
    dst[c] = AVG3(above[c - 1], above[c], above[c]);
    dst[c + stride] = AVG3(above[c - 1], above[c], above[c - 1]);
    dst[c + 2 * stride] = AVG3(above[c - 1], above[c - 1], above[c]);
    dst[c + 3 * stride] = above[c - 1];
  }

  if (bs <= 4)
    return;
  dst += 4 * stride;

  // the rest of first column
  for (r = 4; r < bs; ++r)
    dst[(r - 4) * stride] = left[r - 4];

  // the rest of the block
  for (r = 4; r < bs; ++r) {
    memcpy(dst + 1, dst - 4 * stride, (bs - 1) * sizeof(*dst));
    dst += stride;
  }
}

static INLINE void d166_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r;

  // first 4 columns
  dst[0] = AVG3(above[-1], left[0], left[0]);
  dst[1] = AVG3(above[-1], left[0], above[-1]);
  dst[2] = AVG3(above[-1], above[-1], left[0]);
  dst[3] = above[-1];
  for (r = 1; r < bs; ++r) {
    dst[r * stride] = AVG3(left[r], left[r], left[r - 1]);
    dst[r * stride + 1] = AVG3(left[r - 1], left[r], left[r - 1]);
    dst[r * stride + 2] = AVG3(left[r], left[r - 1], left[r - 1]);
    dst[r * stride + 3] = left[r - 1];
  }

  if (bs <= 4)
    return;
  dst += 4;

  // the rest of first row
  memcpy(dst, above, (bs - 4) * sizeof(*dst));
  dst += stride;

  // the rest of the block
  for (r = 1; r < bs; ++r) {
    memcpy(dst, dst - stride - 4, (bs - 4) * sizeof(*dst));
    dst += stride;
  }
}

static INLINE void d194_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r;
  (void) above;

  // first 4 columns
  for (r = 0; r < bs - 1; ++r) {
    dst[r * stride] = AVG3(left[r], left[r], left[r + 1]);
    dst[r * stride + 1] = AVG3(left[r + 1], left[r], left[r + 1]);
    dst[r * stride + 2] = AVG3(left[r], left[r + 1], left[r + 1]);
    dst[r * stride + 3] = left[r + 1];
  }

  // last row
  memset(dst + (bs - 1) * stride, left[bs - 1], bs * sizeof(*dst));

  if (bs <= 4)
    return;
  dst += 4;

  // the rest of the block
  for (r = bs - 2; r >= 0; --r)
    memcpy(dst + r * stride, dst + (r + 1) * stride - 4,
           (bs - 4) * sizeof(*dst));
}

static int filter_intra_taps_4[TX_SIZES][INTRA_MODES][4] = {
    {
        {735, 881, -537, -54},
        {1005, 519, -488, -11},
        {383, 990, -343, -6},
        {442, 805, -542, 319},
        {658, 616, -133, -116},
        {875, 442, -141, -151},
        {386, 741, -23, -80},
        {390, 1027, -446, 51},
        {679, 606, -523, 262},
        {903, 922, -778, -23},
    },
    {
        {648, 803, -444, 16},
        {972, 620, -576, 7},
        {561, 967, -499, -5},
        {585, 762, -468, 144},
        {596, 619, -182, -9},
        {895, 459, -176, -153},
        {557, 722, -126, -129},
        {601, 839, -523, 105},
        {562, 709, -499, 251},
        {803, 872, -695, 43},
    },
    {
        {423, 728, -347, 111},
        {963, 685, -665, 23},
        {281, 1024, -480, 216},
        {640, 596, -437, 78},
        {429, 669, -259, 99},
        {740, 646, -415, 23},
        {568, 771, -346, 40},
        {404, 833, -486, 209},
        {398, 712, -423, 307},
        {939, 935, -887, 17},
    },
    {
        {477, 737, -393, 150},
        {881, 630, -546, 67},
        {506, 984, -443, -20},
        {114, 459, -270, 528},
        {433, 528, 14, 3},
        {837, 470, -301, -30},
        {181, 777, 89, -107},
        {-29, 716, -232, 259},
        {589, 646, -495, 255},
        {740, 884, -728, 77},
    },
};

static void filter_intra_predictors_4tap(uint8_t *dst, ptrdiff_t stride, int bs,
                                         const uint8_t *above,
                                         const uint8_t *left,
                                         int mode) {
  int k, r, c;
  int pred[33][65];
  int mean, ipred;
  const TX_SIZE tx_size = (bs == 32) ? TX_32X32 :
      ((bs == 16) ? TX_16X16 : ((bs == 8) ? TX_8X8 : (TX_4X4)));
  const int c0 = filter_intra_taps_4[tx_size][mode][0];
  const int c1 = filter_intra_taps_4[tx_size][mode][1];
  const int c2 = filter_intra_taps_4[tx_size][mode][2];
  const int c3 = filter_intra_taps_4[tx_size][mode][3];

  k = 0;
  mean = 0;
  while (k < bs) {
    mean = mean + (int)left[k];
    mean = mean + (int)above[k];
    k++;
  }
  mean = (mean + bs) / (2 * bs);

  for (r = 0; r < bs; ++r)
    pred[r + 1][0] = (int)left[r] - mean;

  for (c = 0; c < 2 * bs + 1; ++c)
    pred[0][c] = (int)above[c - 1] - mean;

  for (r = 1; r < bs + 1; ++r)
    for (c = 1; c < 2 * bs + 1 - r; ++c) {
      ipred = c0 * pred[r - 1][c] + c1 * pred[r][c - 1] +
          c2 * pred[r - 1][c - 1] + c3 * pred[r - 1][c + 1];
      pred[r][c] = ipred < 0 ?
          -((-ipred + FILTER_INTRA_ROUND_VAL) >> FILTER_INTRA_PREC_BITS) :
          ((ipred + FILTER_INTRA_ROUND_VAL) >> FILTER_INTRA_PREC_BITS);
    }

  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c) {
      ipred = pred[r + 1][c + 1] + mean;
      dst[c] = clip_pixel(ipred);
    }
    dst += stride;
  }
}

static void dc_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, DC_PRED);
}

static void v_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, V_PRED);
}

static void h_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, H_PRED);
}

static void d45_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D45_PRED);
}

static void d135_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D135_PRED);
}

static void d117_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D117_PRED);
}

static void d153_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D153_PRED);
}

static void d207_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D207_PRED);
}

static void d63_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, D63_PRED);
}

static void tm_filter_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                const uint8_t *above, const uint8_t *left) {
  filter_intra_predictors_4tap(dst, stride, bs, above, left, TM_PRED);
}

static void (*ext_intra_predictors[EXT_INTRA_MODES])(uint8_t *dst,
    ptrdiff_t stride, int bs, const uint8_t *above, const uint8_t *left) = {
        d76_predictor, d104_predictor, d166_predictor, d194_predictor,
        dc_filter_predictor, v_filter_predictor, h_filter_predictor,
        d45_filter_predictor, d135_filter_predictor, d117_filter_predictor,
        d153_filter_predictor, d207_filter_predictor, d63_filter_predictor,
        tm_filter_predictor,
};

#if CONFIG_VP9_HIGHBITDEPTH
static INLINE void highbd_d76_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                        const uint16_t *above,
                                        const uint16_t *left, int bd) {
  int r, c, i;
  uint16_t buf[4][48];
  (void)left;
  (void) bd;

  for (c = 0; c < bs + bs / 4; ++c) {
    buf[0][c] = AVG3(above[c], above[c], above[c + 1]);
    buf[1][c] = AVG3(above[c + 1], above[c], above[c + 1]);
    buf[2][c] = AVG3(above[c], above[c + 1], above[c + 1]);
    buf[3][c] = above[c + 1];
  }

  for (r = 0; r < bs; r += 4) {
    for (i = 0; i < 4; ++i)
      memcpy(dst + i * stride, buf[i] + (r >> 2), bs * sizeof(*dst));
    dst += 4 * stride;
  }
}

static INLINE void highbd_d104_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r, c;
  (void) bd;

  // first 4 rows
  for (c = 0; c < bs; c++) {
    dst[c] = AVG3(above[c - 1], above[c], above[c]);
    dst[c + stride] = AVG3(above[c - 1], above[c], above[c - 1]);
    dst[c + 2 * stride] = AVG3(above[c - 1], above[c - 1], above[c]);
    dst[c + 3 * stride] = above[c - 1];
  }

  if (bs <= 4)
    return;
  dst += 4 * stride;

  // the rest of first column
  for (r = 4; r < bs; ++r)
    dst[(r - 4) * stride] = left[r - 4];

  // the rest of the block
  for (r = 4; r < bs; ++r) {
    memcpy(dst + 1, dst - 4 * stride, (bs - 1) * sizeof(*dst));
    dst += stride;
  }
}

static INLINE void highbd_d166_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r;
  (void) bd;

  // first 4 columns
  dst[0] = AVG3(above[-1], left[0], left[0]);
  dst[1] = AVG3(above[-1], left[0], above[-1]);
  dst[2] = AVG3(above[-1], above[-1], left[0]);
  dst[3] = above[-1];
  for (r = 1; r < bs; ++r) {
    dst[r * stride] = AVG3(left[r], left[r], left[r - 1]);
    dst[r * stride + 1] = AVG3(left[r - 1], left[r], left[r - 1]);
    dst[r * stride + 2] = AVG3(left[r], left[r - 1], left[r - 1]);
    dst[r * stride + 3] = left[r - 1];
  }

  if (bs <= 4)
    return;
  dst += 4;

  // the rest of first row
  memcpy(dst, above, (bs - 4) * sizeof(*dst));
  dst += stride;

  // the rest of the block
  for (r = 1; r < bs; ++r) {
    memcpy(dst, dst - stride - 4, (bs - 4) * sizeof(*dst));
    dst += stride;
  }
}

static INLINE void highbd_d194_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r;
  (void) bd;
  (void) above;

  // first 4 columns
  for (r = 0; r < bs - 1; ++r) {
    dst[r * stride] = AVG3(left[r], left[r], left[r + 1]);
    dst[r * stride + 1] = AVG3(left[r + 1], left[r], left[r + 1]);
    dst[r * stride + 2] = AVG3(left[r], left[r + 1], left[r + 1]);
    dst[r * stride + 3] = left[r + 1];
  }

  // last row
  memset(dst + (bs - 1) * stride, left[bs - 1], bs * sizeof(*dst));

  if (bs <= 4)
    return;
  dst += 4;

  // the rest of the block
  for (r = bs - 2; r >= 0; --r)
    memcpy(dst + r * stride, dst + (r + 1) * stride - 4,
           (bs - 4) * sizeof(*dst));
}

static void highbd_filter_intra_predictors_4tap(uint16_t *dst, ptrdiff_t stride,
                                                int bs, const uint16_t *above,
                                                const uint16_t *left, int mode,
                                                int bd) {
  int k, r, c;
  int pred[33][65];
  int mean, ipred;
  const TX_SIZE tx_size = (bs == 32) ? TX_32X32 :
      ((bs == 16) ? TX_16X16 : ((bs == 8) ? TX_8X8 : (TX_4X4)));
  const int c0 = filter_intra_taps_4[tx_size][mode][0];
  const int c1 = filter_intra_taps_4[tx_size][mode][1];
  const int c2 = filter_intra_taps_4[tx_size][mode][2];
  const int c3 = filter_intra_taps_4[tx_size][mode][3];

  k = 0;
  mean = 0;
  while (k < bs) {
    mean = mean + (int)left[k];
    mean = mean + (int)above[k];
    k++;
  }
  mean = (mean + bs) / (2 * bs);

  for (r = 0; r < bs; ++r)
    pred[r + 1][0] = (int)left[r] - mean;

  for (c = 0; c < 2 * bs + 1; ++c)
    pred[0][c] = (int)above[c - 1] - mean;

  for (r = 1; r < bs + 1; ++r)
    for (c = 1; c < 2 * bs + 1 - r; ++c) {
      ipred = c0 * pred[r - 1][c] + c1 * pred[r][c - 1] +
          c2 * pred[r - 1][c - 1] + c3 * pred[r - 1][c + 1];
      pred[r][c] = ipred < 0 ?
          -((-ipred + FILTER_INTRA_ROUND_VAL) >> FILTER_INTRA_PREC_BITS) :
          ((ipred + FILTER_INTRA_ROUND_VAL) >> FILTER_INTRA_PREC_BITS);
    }

  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c) {
      ipred = pred[r + 1][c + 1] + mean;
      dst[c] = clip_pixel_highbd(ipred, bd);
    }
    dst += stride;
  }
}

static void highbd_dc_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                       int bs, const uint16_t *above,
                                       const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, DC_PRED,
                                      bd);
}

static void highbd_v_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                      int bs, const uint16_t *above,
                                      const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, V_PRED,
                                      bd);
}

static void highbd_h_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                      int bs, const uint16_t *above,
                                      const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, H_PRED,
                                      bd);
}

static void highbd_d45_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                        int bs, const uint16_t *above,
                                        const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D45_PRED,
                                      bd);
}

static void highbd_d135_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D135_PRED,
                                      bd);
}

static void highbd_d117_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D117_PRED,
                                      bd);
}

static void highbd_d153_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D153_PRED,
                                      bd);
}

static void highbd_d207_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D207_PRED,
                                      bd);
}

static void highbd_d63_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                        int bs, const uint16_t *above,
                                        const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, D63_PRED,
                                      bd);
}

static void highbd_tm_filter_predictor(uint16_t *dst, ptrdiff_t stride,
                                       int bs, const uint16_t *above,
                                       const uint16_t *left, int bd) {
  highbd_filter_intra_predictors_4tap(dst, stride, bs, above, left, TM_PRED,
                                      bd);
}

static void (*highbd_ext_intra_predictors[EXT_INTRA_MODES])(uint16_t *dst,
    ptrdiff_t stride, int bs, const uint16_t *above, const uint16_t *left,
    int bd) = {
        highbd_d76_predictor, highbd_d104_predictor, highbd_d166_predictor,
        highbd_d194_predictor, highbd_dc_filter_predictor,
        highbd_v_filter_predictor, highbd_h_filter_predictor,
        highbd_d45_filter_predictor, highbd_d135_filter_predictor,
        highbd_d117_filter_predictor, highbd_d153_filter_predictor,
        highbd_d207_filter_predictor, highbd_d63_filter_predictor,
        highbd_tm_filter_predictor,
};
#endif  // CONFIG_VP9_HIGHBITDEPTH
#endif  // CONFIG_EXT_INTRA

#if CONFIG_VP9_HIGHBITDEPTH
static void build_intra_predictors_high(const MACROBLOCKD *xd,
                                        const uint8_t *ref8,
                                        int ref_stride,
                                        uint8_t *dst8,
                                        int dst_stride,
                                        PREDICTION_MODE mode,
                                        TX_SIZE tx_size,
                                        int up_available,
                                        int left_available,
                                        int right_available,
                                        int x, int y,
                                        int plane, int bd) {
  int i;
  uint16_t *dst = CONVERT_TO_SHORTPTR(dst8);
  uint16_t *ref = CONVERT_TO_SHORTPTR(ref8);
  DECLARE_ALIGNED(16, uint16_t, left_col[32]);
  DECLARE_ALIGNED(16, uint16_t, above_data[64 + 16]);
  uint16_t *above_row = above_data + 16;
  const uint16_t *const_above_row = above_row;
  const int bs = 4 << tx_size;
  int frame_width, frame_height;
  int x0, y0;
  const struct macroblockd_plane *const pd = &xd->plane[plane];
  //  int base=128;
  int base = 128 << (bd - 8);
  // 127 127 127 .. 127 127 127 127 127 127
  // 129  A   B  ..  Y   Z
  // 129  C   D  ..  W   X
  // 129  E   F  ..  U   V
  // 129  G   H  ..  S   T   T   T   T   T

  // Get current frame pointer, width and height.
  if (plane == 0) {
    frame_width = xd->cur_buf->y_width;
    frame_height = xd->cur_buf->y_height;
  } else {
    frame_width = xd->cur_buf->uv_width;
    frame_height = xd->cur_buf->uv_height;
  }

  // Get block position in current frame.
  x0 = (-xd->mb_to_left_edge >> (3 + pd->subsampling_x)) + x;
  y0 = (-xd->mb_to_top_edge >> (3 + pd->subsampling_y)) + y;

  // left
  if (left_available) {
    if (xd->mb_to_bottom_edge < 0) {
      /* slower path if the block needs border extension */
      if (y0 + bs <= frame_height) {
        for (i = 0; i < bs; ++i)
          left_col[i] = ref[i * ref_stride - 1];
      } else {
        const int extend_bottom = frame_height - y0;
        for (i = 0; i < extend_bottom; ++i)
          left_col[i] = ref[i * ref_stride - 1];
        for (; i < bs; ++i)
          left_col[i] = ref[(extend_bottom - 1) * ref_stride - 1];
      }
    } else {
      /* faster path if the block does not need extension */
      for (i = 0; i < bs; ++i)
        left_col[i] = ref[i * ref_stride - 1];
    }
  } else {
    // TODO(Peter): this value should probably change for high bitdepth
    vpx_memset16(left_col, base + 1, bs);
  }

  // TODO(hkuang) do not extend 2*bs pixels for all modes.
  // above
  if (up_available) {
    const uint16_t *above_ref = ref - ref_stride;
    if (xd->mb_to_right_edge < 0) {
      /* slower path if the block needs border extension */
      if (x0 + 2 * bs <= frame_width) {
        if (right_available && bs == 4) {
          memcpy(above_row, above_ref, 2 * bs * sizeof(above_row[0]));
        } else {
          memcpy(above_row, above_ref, bs * sizeof(above_row[0]));
          vpx_memset16(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 + bs <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
          memcpy(above_row, above_ref, r * sizeof(above_row[0]));
          vpx_memset16(above_row + r, above_row[r - 1],
                       x0 + 2 * bs - frame_width);
        } else {
          memcpy(above_row, above_ref, bs * sizeof(above_row[0]));
          vpx_memset16(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 <= frame_width) {
        const int r = frame_width - x0;
        memcpy(above_row, above_ref, r * sizeof(above_row[0]));
        vpx_memset16(above_row + r, above_row[r - 1],
                       x0 + 2 * bs - frame_width);
      }
      // TODO(Peter) this value should probably change for high bitdepth
      above_row[-1] = left_available ? above_ref[-1] : (base+1);
    } else {
      /* faster path if the block does not need extension */
      if (bs == 4 && right_available && left_available) {
        const_above_row = above_ref;
      } else {
        memcpy(above_row, above_ref, bs * sizeof(above_row[0]));
        if (bs == 4 && right_available)
          memcpy(above_row + bs, above_ref + bs, bs * sizeof(above_row[0]));
        else
          vpx_memset16(above_row + bs, above_row[bs - 1], bs);
        // TODO(Peter): this value should probably change for high bitdepth
        above_row[-1] = left_available ? above_ref[-1] : (base+1);
      }
    }
  } else {
    vpx_memset16(above_row, base - 1, bs * 2);
    // TODO(Peter): this value should probably change for high bitdepth
    above_row[-1] = base - 1;
  }

#if CONFIG_EXT_INTRA
  if (xd->mi[0]->mbmi.ext_intra_mode_info.use_ext_intra_mode[plane != 0]) {
    const EXT_INTRA_MODE ext_intra_mode =
        xd->mi[0]->mbmi.ext_intra_mode_info.ext_intra_mode[plane != 0];
    highbd_ext_intra_predictors[ext_intra_mode](dst, dst_stride, bs,
                                                const_above_row, left_col, bd);
    return;
  }
#endif  // CONFIG_EXT_INTRA

  // predict
  if (mode == DC_PRED) {
    dc_pred_high[left_available][up_available][tx_size](dst, dst_stride,
                                                        const_above_row,
                                                        left_col, xd->bd);
  } else {
    pred_high[mode][tx_size](dst, dst_stride, const_above_row, left_col,
                             xd->bd);
  }
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

static void build_intra_predictors(const MACROBLOCKD *xd, const uint8_t *ref,
                                   int ref_stride, uint8_t *dst, int dst_stride,
                                   PREDICTION_MODE mode, TX_SIZE tx_size,
                                   int up_available, int left_available,
                                   int right_available, int x, int y,
                                   int plane) {
  int i;
  DECLARE_ALIGNED(16, uint8_t, left_col[32]);
  DECLARE_ALIGNED(16, uint8_t, above_data[64 + 16]);
  uint8_t *above_row = above_data + 16;
  const uint8_t *const_above_row = above_row;
  const int bs = 4 << tx_size;
  int need_left = extend_modes[mode] & NEED_LEFT;
  int need_above = extend_modes[mode] & NEED_ABOVE;
  int need_aboveright = extend_modes[mode] & NEED_ABOVERIGHT;
  int frame_width, frame_height;
  int x0, y0;
  const struct macroblockd_plane *const pd = &xd->plane[plane];

#if CONFIG_EXT_INTRA
  if (xd->mi[0]->mbmi.ext_intra_mode_info.use_ext_intra_mode[plane != 0]) {
    EXT_INTRA_MODE ext_intra_mode =
        xd->mi[0]->mbmi.ext_intra_mode_info.ext_intra_mode[plane != 0];
    need_left = ext_intra_extend_modes[ext_intra_mode] & NEED_LEFT;
    need_above = ext_intra_extend_modes[ext_intra_mode] & NEED_ABOVE;
    need_aboveright = ext_intra_extend_modes[ext_intra_mode] & NEED_ABOVERIGHT;
  }
#endif  // CONFIG_EXT_INTRA

  // 127 127 127 .. 127 127 127 127 127 127
  // 129  A   B  ..  Y   Z
  // 129  C   D  ..  W   X
  // 129  E   F  ..  U   V
  // 129  G   H  ..  S   T   T   T   T   T
  // ..

  // Get current frame pointer, width and height.
  if (plane == 0) {
    frame_width = xd->cur_buf->y_width;
    frame_height = xd->cur_buf->y_height;
  } else {
    frame_width = xd->cur_buf->uv_width;
    frame_height = xd->cur_buf->uv_height;
  }

  // Get block position in current frame.
  x0 = (-xd->mb_to_left_edge >> (3 + pd->subsampling_x)) + x;
  y0 = (-xd->mb_to_top_edge >> (3 + pd->subsampling_y)) + y;

  // NEED_LEFT
  if (need_left) {
    if (left_available) {
      if (xd->mb_to_bottom_edge < 0) {
        /* slower path if the block needs border extension */
        if (y0 + bs <= frame_height) {
          for (i = 0; i < bs; ++i)
            left_col[i] = ref[i * ref_stride - 1];
        } else {
          const int extend_bottom = frame_height - y0;
          for (i = 0; i < extend_bottom; ++i)
            left_col[i] = ref[i * ref_stride - 1];
          for (; i < bs; ++i)
            left_col[i] = ref[(extend_bottom - 1) * ref_stride - 1];
        }
      } else {
        /* faster path if the block does not need extension */
        for (i = 0; i < bs; ++i)
          left_col[i] = ref[i * ref_stride - 1];
      }
    } else {
      memset(left_col, 129, bs);
    }
  }

  // NEED_ABOVE
  if (need_above) {
    if (up_available) {
      const uint8_t *above_ref = ref - ref_stride;
      if (xd->mb_to_right_edge < 0) {
        /* slower path if the block needs border extension */
        if (x0 + bs <= frame_width) {
          memcpy(above_row, above_ref, bs);
        } else if (x0 <= frame_width) {
          const int r = frame_width - x0;
          memcpy(above_row, above_ref, r);
          memset(above_row + r, above_row[r - 1], x0 + bs - frame_width);
        }
      } else {
        /* faster path if the block does not need extension */
        if (bs == 4 && right_available && left_available) {
          const_above_row = above_ref;
        } else {
          memcpy(above_row, above_ref, bs);
        }
      }
      above_row[-1] = left_available ? above_ref[-1] : 129;
    } else {
      memset(above_row, 127, bs);
      above_row[-1] = 127;
    }
  }

  // NEED_ABOVERIGHT
  if (need_aboveright) {
    if (up_available) {
      const uint8_t *above_ref = ref - ref_stride;
      if (xd->mb_to_right_edge < 0) {
        /* slower path if the block needs border extension */
        if (x0 + 2 * bs <= frame_width) {
          if (right_available && bs == 4) {
            memcpy(above_row, above_ref, 2 * bs);
          } else {
            memcpy(above_row, above_ref, bs);
            memset(above_row + bs, above_row[bs - 1], bs);
          }
        } else if (x0 + bs <= frame_width) {
          const int r = frame_width - x0;
          if (right_available && bs == 4) {
            memcpy(above_row, above_ref, r);
            memset(above_row + r, above_row[r - 1], x0 + 2 * bs - frame_width);
          } else {
            memcpy(above_row, above_ref, bs);
            memset(above_row + bs, above_row[bs - 1], bs);
          }
        } else if (x0 <= frame_width) {
          const int r = frame_width - x0;
          memcpy(above_row, above_ref, r);
          memset(above_row + r, above_row[r - 1], x0 + 2 * bs - frame_width);
        }
      } else {
        /* faster path if the block does not need extension */
        if (bs == 4 && right_available && left_available) {
          const_above_row = above_ref;
        } else {
          memcpy(above_row, above_ref, bs);
          if (bs == 4 && right_available)
            memcpy(above_row + bs, above_ref + bs, bs);
          else
            memset(above_row + bs, above_row[bs - 1], bs);
        }
      }
      above_row[-1] = left_available ? above_ref[-1] : 129;
    } else {
      memset(above_row, 127, bs * 2);
      above_row[-1] = 127;
    }
  }

#if CONFIG_EXT_INTRA
  if (xd->mi[0]->mbmi.ext_intra_mode_info.use_ext_intra_mode[plane != 0]) {
    const EXT_INTRA_MODE ext_intra_mode =
        xd->mi[0]->mbmi.ext_intra_mode_info.ext_intra_mode[plane != 0];
    ext_intra_predictors[ext_intra_mode](dst, dst_stride, bs,
                                         const_above_row, left_col);
    return;
  }
#endif  // CONFIG_EXT_INTRA

  // predict
  if (mode == DC_PRED) {
    dc_pred[left_available][up_available][tx_size](dst, dst_stride,
                                                   const_above_row, left_col);
  } else {
    pred[mode][tx_size](dst, dst_stride, const_above_row, left_col);
  }
}

void vp10_predict_intra_block(const MACROBLOCKD *xd, int bwl_in,
                             TX_SIZE tx_size, PREDICTION_MODE mode,
                             const uint8_t *ref, int ref_stride,
                             uint8_t *dst, int dst_stride,
                             int aoff, int loff, int plane) {
  const int bw = (1 << bwl_in);
  const int txw = (1 << tx_size);
  const int have_top = loff || xd->up_available;
  const int have_left = aoff || xd->left_available;
  const int have_right = (aoff + txw) < bw;
  const int x = aoff * 4;
  const int y = loff * 4;

#if CONFIG_VP9_HIGHBITDEPTH
  if (xd->cur_buf->flags & YV12_FLAG_HIGHBITDEPTH) {
    build_intra_predictors_high(xd, ref, ref_stride, dst, dst_stride, mode,
                                tx_size, have_top, have_left, have_right,
                                x, y, plane, xd->bd);
    return;
  }
#endif
  build_intra_predictors(xd, ref, ref_stride, dst, dst_stride, mode, tx_size,
                         have_top, have_left, have_right, x, y, plane);
}

void vp10_init_intra_predictors(void) {
  once(vp10_init_intra_predictors_internal);
}
