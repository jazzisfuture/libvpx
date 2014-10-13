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
#include "./vp9_rtcd.h"

#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_reconintra.h"
#include "vp9/common/vp9_onyxc_int.h"

const TX_TYPE intra_mode_to_tx_type_lookup[INTRA_MODES] = {
  DCT_DCT,    // DC
  ADST_DCT,   // V
  DCT_ADST,   // H
  DCT_DCT,    // D45
  ADST_ADST,  // D135
  ADST_DCT,   // D117
  DCT_ADST,   // D153
  DCT_ADST,   // D207
  ADST_DCT,   // D63
  ADST_ADST,  // TM
};

// This serves as a wrapper function, so that all the prediction functions
// can be unified and accessed as a pointer array. Note that the boundary
// above and left are not necessarily used all the time.
#define intra_pred_sized(type, size) \
  void vp9_##type##_predictor_##size##x##size##_c(uint8_t *dst, \
                                                  ptrdiff_t stride, \
                                                  const uint8_t *above, \
                                                  const uint8_t *left) { \
    type##_predictor(dst, stride, size, above, left); \
  }

#if CONFIG_VP9_HIGHBITDEPTH
#define intra_pred_highbd_sized(type, size) \
  void vp9_highbd_##type##_predictor_##size##x##size##_c( \
      uint16_t *dst, ptrdiff_t stride, const uint16_t *above, \
      const uint16_t *left, int bd) { \
    highbd_##type##_predictor(dst, stride, size, above, left, bd); \
  }

#define intra_pred_allsizes(type) \
  intra_pred_sized(type, 4) \
  intra_pred_sized(type, 8) \
  intra_pred_sized(type, 16) \
  intra_pred_sized(type, 32) \
  intra_pred_highbd_sized(type, 4) \
  intra_pred_highbd_sized(type, 8) \
  intra_pred_highbd_sized(type, 16) \
  intra_pred_highbd_sized(type, 32)

#else

#define intra_pred_allsizes(type) \
  intra_pred_sized(type, 4) \
  intra_pred_sized(type, 8) \
  intra_pred_sized(type, 16) \
  intra_pred_sized(type, 32)
#endif  // CONFIG_VP9_HIGHBITDEPTH

#if CONFIG_VP9_HIGHBITDEPTH
static INLINE void highbd_d207_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r, c;
  (void) above;
  (void) bd;

  // First column.
  for (r = 0; r < bs - 1; ++r) {
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1], 1);
  }
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // Second column.
  for (r = 0; r < bs - 2; ++r) {
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1] * 2 +
                                         left[r + 2], 2);
  }
  dst[(bs - 2) * stride] = ROUND_POWER_OF_TWO(left[bs - 2] +
                                              left[bs - 1] * 3, 2);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // Rest of last row.
  for (c = 0; c < bs - 2; ++c)
    dst[(bs - 1) * stride + c] = left[bs - 1];

  for (r = bs - 2; r >= 0; --r) {
    for (c = 0; c < bs - 2; ++c)
      dst[r * stride + c] = dst[(r + 1) * stride + c - 2];
  }
}

static INLINE void highbd_d63_predictor(uint16_t *dst, ptrdiff_t stride,
                                        int bs, const uint16_t *above,
                                        const uint16_t *left, int bd) {
  int r, c;
  (void) left;
  (void) bd;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c) {
      dst[c] = r & 1 ? ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1] * 2 +
                                          above[r/2 + c + 2], 2)
                     : ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1], 1);
    }
    dst += stride;
  }
}

static INLINE void highbd_d45_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                        const uint16_t *above,
                                        const uint16_t *left, int bd) {
  int r, c;
  (void) left;
  (void) bd;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c) {
      dst[c] = r + c + 2 < bs * 2 ?  ROUND_POWER_OF_TWO(above[r + c] +
                                                        above[r + c + 1] * 2 +
                                                        above[r + c + 2], 2)
                                  : above[bs * 2 - 1];
    }
    dst += stride;
  }
}

static INLINE void highbd_d117_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r, c;
  (void) bd;

  // first row
  for (c = 0; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c], 1);
  dst += stride;

  // second row
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);
  dst += stride;

  // the rest of first col
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 3; r < bs; ++r)
    dst[(r - 2) * stride] = ROUND_POWER_OF_TWO(left[r - 3] + left[r - 2] * 2 +
                                               left[r - 1], 2);

  // the rest of the block
  for (r = 2; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-2 * stride + c - 1];
    dst += stride;
  }
}

static INLINE void highbd_d135_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r, c;
  (void) bd;
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);

  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);

  dst += stride;
  for (r = 1; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-stride + c - 1];
    dst += stride;
  }
}

static INLINE void highbd_d153_predictor(uint16_t *dst, ptrdiff_t stride,
                                         int bs, const uint16_t *above,
                                         const uint16_t *left, int bd) {
  int r, c;
  (void) bd;
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0], 1);
  for (r = 1; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 1] + left[r], 1);
  dst++;

  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);
  dst++;

  for (c = 0; c < bs - 2; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c] * 2 + above[c + 1], 2);
  dst += stride;

  for (r = 1; r < bs; ++r) {
    for (c = 0; c < bs - 2; c++)
      dst[c] = dst[-stride + c - 2];
    dst += stride;
  }
}

static INLINE void highbd_v_predictor(uint16_t *dst, ptrdiff_t stride,
                                      int bs, const uint16_t *above,
                                      const uint16_t *left, int bd) {
  int r;
  (void) left;
  (void) bd;
  for (r = 0; r < bs; r++) {
    vpx_memcpy(dst, above, bs * sizeof(uint16_t));
    dst += stride;
  }
}

static INLINE void highbd_h_predictor(uint16_t *dst, ptrdiff_t stride,
                                      int bs, const uint16_t *above,
                                      const uint16_t *left, int bd) {
  int r;
  (void) above;
  (void) bd;
  for (r = 0; r < bs; r++) {
    vpx_memset16(dst, left[r], bs);
    dst += stride;
  }
}

static INLINE void highbd_tm_predictor(uint16_t *dst, ptrdiff_t stride,
                                       int bs, const uint16_t *above,
                                       const uint16_t *left, int bd) {
  int r, c;
  int ytop_left = above[-1];
  (void) bd;

  for (r = 0; r < bs; r++) {
    for (c = 0; c < bs; c++)
      dst[c] = clip_pixel_highbd(left[r] + above[c] - ytop_left, bd);
    dst += stride;
  }
}

static INLINE void highbd_dc_128_predictor(uint16_t *dst, ptrdiff_t stride,
                                           int bs, const uint16_t *above,
                                           const uint16_t *left, int bd) {
  int r;
  (void) above;
  (void) left;

  for (r = 0; r < bs; r++) {
    vpx_memset16(dst, 128 << (bd - 8), bs);
    dst += stride;
  }
}

static INLINE void highbd_dc_left_predictor(uint16_t *dst, ptrdiff_t stride,
                                            int bs, const uint16_t *above,
                                            const uint16_t *left, int bd) {
  int i, r, expected_dc, sum = 0;
  (void) above;
  (void) bd;

  for (i = 0; i < bs; i++)
    sum += left[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    vpx_memset16(dst, expected_dc, bs);
    dst += stride;
  }
}

static INLINE void highbd_dc_top_predictor(uint16_t *dst, ptrdiff_t stride,
                                           int bs, const uint16_t *above,
                                           const uint16_t *left, int bd) {
  int i, r, expected_dc, sum = 0;
  (void) left;
  (void) bd;

  for (i = 0; i < bs; i++)
    sum += above[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    vpx_memset16(dst, expected_dc, bs);
    dst += stride;
  }
}

static INLINE void highbd_dc_predictor(uint16_t *dst, ptrdiff_t stride,
                                       int bs, const uint16_t *above,
                                       const uint16_t *left, int bd) {
  int i, r, expected_dc, sum = 0;
  const int count = 2 * bs;
  (void) bd;

  for (i = 0; i < bs; i++) {
    sum += above[i];
    sum += left[i];
  }

  expected_dc = (sum + (count >> 1)) / count;

  for (r = 0; r < bs; r++) {
    vpx_memset16(dst, expected_dc, bs);
    dst += stride;
  }
}
#endif  // CONFIG_VP9_HIGHBITDEPTH

static INLINE void d207_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r, c;
  (void) above;
  // first column
  for (r = 0; r < bs - 1; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1], 1);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // second column
  for (r = 0; r < bs - 2; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1] * 2 +
                                         left[r + 2], 2);
  dst[(bs - 2) * stride] = ROUND_POWER_OF_TWO(left[bs - 2] +
                                              left[bs - 1] * 3, 2);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // rest of last row
  for (c = 0; c < bs - 2; ++c)
    dst[(bs - 1) * stride + c] = left[bs - 1];

  for (r = bs - 2; r >= 0; --r)
    for (c = 0; c < bs - 2; ++c)
      dst[r * stride + c] = dst[(r + 1) * stride + c - 2];
}
intra_pred_allsizes(d207)

static INLINE void d63_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
  int r, c;
  (void) left;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c)
      dst[c] = r & 1 ? ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1] * 2 +
                                          above[r/2 + c + 2], 2)
                     : ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1], 1);
    dst += stride;
  }
}
intra_pred_allsizes(d63)

static INLINE void d45_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
  int r, c;
  (void) left;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c)
      dst[c] = r + c + 2 < bs * 2 ?  ROUND_POWER_OF_TWO(above[r + c] +
                                                        above[r + c + 1] * 2 +
                                                        above[r + c + 2], 2)
                                  : above[bs * 2 - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d45)

static INLINE void d117_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r, c;

  // first row
  for (c = 0; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c], 1);
  dst += stride;

  // second row
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);
  dst += stride;

  // the rest of first col
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 3; r < bs; ++r)
    dst[(r - 2) * stride] = ROUND_POWER_OF_TWO(left[r - 3] + left[r - 2] * 2 +
                                               left[r - 1], 2);

  // the rest of the block
  for (r = 2; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-2 * stride + c - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d117)

static INLINE void d135_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r, c;
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);

  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);

  dst += stride;
  for (r = 1; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-stride + c - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d135)

static INLINE void d153_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
  int r, c;
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0], 1);
  for (r = 1; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 1] + left[r], 1);
  dst++;

  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);
  dst++;

  for (c = 0; c < bs - 2; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c] * 2 + above[c + 1], 2);
  dst += stride;

  for (r = 1; r < bs; ++r) {
    for (c = 0; c < bs - 2; c++)
      dst[c] = dst[-stride + c - 2];
    dst += stride;
  }
}
intra_pred_allsizes(d153)

static INLINE void v_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  int r;
  (void) left;

  for (r = 0; r < bs; r++) {
    vpx_memcpy(dst, above, bs);
    dst += stride;
  }
}
intra_pred_allsizes(v)

static INLINE void h_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  int r;
  (void) above;

  for (r = 0; r < bs; r++) {
    vpx_memset(dst, left[r], bs);
    dst += stride;
  }
}
intra_pred_allsizes(h)

static INLINE void tm_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                const uint8_t *above, const uint8_t *left) {
  int r, c;
  int ytop_left = above[-1];

  for (r = 0; r < bs; r++) {
    for (c = 0; c < bs; c++)
      dst[c] = clip_pixel(left[r] + above[c] - ytop_left);
    dst += stride;
  }
}
intra_pred_allsizes(tm)

static INLINE void dc_128_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                    const uint8_t *above, const uint8_t *left) {
  int r;
  (void) above;
  (void) left;

  for (r = 0; r < bs; r++) {
    vpx_memset(dst, 128, bs);
    dst += stride;
  }
}
intra_pred_allsizes(dc_128)

static INLINE void dc_left_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                     const uint8_t *above,
                                     const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  (void) above;

  for (i = 0; i < bs; i++)
    sum += left[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    vpx_memset(dst, expected_dc, bs);
    dst += stride;
  }
}
intra_pred_allsizes(dc_left)

static INLINE void dc_top_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                    const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  (void) left;

  for (i = 0; i < bs; i++)
    sum += above[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
    vpx_memset(dst, expected_dc, bs);
    dst += stride;
  }
}
intra_pred_allsizes(dc_top)

static INLINE void dc_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
  const int count = 2 * bs;

  for (i = 0; i < bs; i++) {
    sum += above[i];
    sum += left[i];
  }

  expected_dc = (sum + (count >> 1)) / count;

  for (r = 0; r < bs; r++) {
    vpx_memset(dst, expected_dc, bs);
    dst += stride;
  }
}
intra_pred_allsizes(dc)
#undef intra_pred_allsizes

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

void vp9_init_intra_predictors() {
#define INIT_ALL_SIZES(p, type) \
  p[TX_4X4] = vp9_##type##_predictor_4x4; \
  p[TX_8X8] = vp9_##type##_predictor_8x8; \
  p[TX_16X16] = vp9_##type##_predictor_16x16; \
  p[TX_32X32] = vp9_##type##_predictor_32x32

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
  DECLARE_ALIGNED_ARRAY(16, uint16_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint16_t, above_data, 128 + 16);
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
          vpx_memcpy(above_row, above_ref, 2 * bs * sizeof(uint16_t));
        } else {
          vpx_memcpy(above_row, above_ref, bs * sizeof(uint16_t));
          vpx_memset16(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 + bs <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
          vpx_memcpy(above_row, above_ref, r * sizeof(uint16_t));
          vpx_memset16(above_row + r, above_row[r - 1],
                       x0 + 2 * bs - frame_width);
        } else {
          vpx_memcpy(above_row, above_ref, bs * sizeof(uint16_t));
          vpx_memset16(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
          vpx_memcpy(above_row, above_ref, r * sizeof(uint16_t));
          vpx_memset16(above_row + r, above_row[r - 1],
                       x0 + 2 * bs - frame_width);
        } else {
          vpx_memcpy(above_row, above_ref, r * sizeof(uint16_t));
          vpx_memset16(above_row + r, above_row[r - 1],
                       x0 + 2 * bs - frame_width);
        }
      }
      // TODO(Peter) this value should probably change for high bitdepth
      above_row[-1] = left_available ? above_ref[-1] : (base+1);
    } else {
      /* faster path if the block does not need extension */
      if (bs == 4 && right_available && left_available) {
        const_above_row = above_ref;
      } else {
        vpx_memcpy(above_row, above_ref, bs * sizeof(uint16_t));
        if (bs == 4 && right_available)
          vpx_memcpy(above_row + bs, above_ref + bs, bs * sizeof(uint16_t));
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
  DECLARE_ALIGNED_ARRAY(16, uint8_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, above_data, 128 + 16);
  uint8_t *above_row = above_data + 16;
  const uint8_t *const_above_row = above_row;
  const int bs = 4 << tx_size;
  int frame_width, frame_height;
  int x0, y0;
  const struct macroblockd_plane *const pd = &xd->plane[plane];

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

  vpx_memset(left_col, 129, 64);

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
  }

  // TODO(hkuang) do not extend 2*bs pixels for all modes.
  // above
  if (up_available) {
    const uint8_t *above_ref = ref - ref_stride;
    if (xd->mb_to_right_edge < 0) {
      /* slower path if the block needs border extension */
      if (x0 + 2 * bs <= frame_width) {
        if (right_available && bs == 4) {
          vpx_memcpy(above_row, above_ref, 2 * bs);
        } else {
          vpx_memcpy(above_row, above_ref, bs);
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 + bs <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
        } else {
          vpx_memcpy(above_row, above_ref, bs);
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
        }
      } else if (x0 <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
        } else {
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
        }
      }
      above_row[-1] = left_available ? above_ref[-1] : 129;
    } else {
      /* faster path if the block does not need extension */
      if (bs == 4 && right_available && left_available) {
        const_above_row = above_ref;
      } else {
        vpx_memcpy(above_row, above_ref, bs);
        if (bs == 4 && right_available)
          vpx_memcpy(above_row + bs, above_ref + bs, bs);
        else
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
        above_row[-1] = left_available ? above_ref[-1] : 129;
      }
    }
  } else {
    vpx_memset(above_row, 127, bs * 2);
    above_row[-1] = 127;
  }

  // predict
  if (mode == DC_PRED) {
    dc_pred[left_available][up_available][tx_size](dst, dst_stride,
                                                   const_above_row, left_col);
  } else {
    pred[mode][tx_size](dst, dst_stride, const_above_row, left_col);
  }
}

<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_FILTERINTRA
static void filter_intra_predictors(uint8_t *ypred_ptr, int y_stride, int bs,
                                    uint8_t *yabove_row, uint8_t *yleft_col,
                                    int mode) {
  static const int prec_bits = 10;
  static const int round_val = 511;
  static const int taps[10][3] = {
      {   0,   0,    0},  // DC
      { 972, 563, -534},  // V
      { 441, 975, -417},  // H
      {   0,   0,    0},  // D45
      { 502, 546,  -48},  // D135
      { 744, 523, -259},  // D117
      { 379, 760,  -73},  // D153
      {   0,   0,    0},  // D27
      {   0,   0,    0},  // D63
      { 783, 839, -687},  // TM
  };
  static const int taps8x8[10][3] = {
      {  0,    0,    0},  // DC
      {991,  655, -637},  // V
      {522,  987, -493},  // H
      {  0,    0,    0},  // d45
      {551,  608, -193},  // d135
      {762,  612, -392},  // d117
      {492,  781, -260},  // d153
      {  0,    0,    0},  // d27
      {  0,    0,    0},  // d63
      {823,  873, -715},  // TM
  };

  int k, r, c;
  int pred[17][17];
  int mean, ipred;
  const int c1 = (bs == 4) ? taps[mode][0]: taps8x8[mode][0];
  const int c2 = (bs == 4) ? taps[mode][1]: taps8x8[mode][1];
  const int c3 = (bs == 4) ? taps[mode][2]: taps8x8[mode][2];

  k = 0;
  mean = 0;
  while (k < bs) {
    mean = mean + (int)yleft_col[k];
    mean = mean + (int)yabove_row[k];
    ++k;
  }
  mean = (mean + bs) / (2 * bs);

  for (r = 0; r < bs; r++)
    pred[r + 1][0] = (int)yleft_col[r] - mean;

  for (c = 0; c < bs + 1; c++)
    pred[0][c] = (int)yabove_row[c - 1] - mean;

  for (r = 1; r < bs + 1; r++)
    for (c = 1; c < bs + 1; c++) {
      ipred = c1 * pred[r - 1][c] + c2 * pred[r][c - 1]
              + c3 * pred[r - 1][c - 1];
      pred[r][c] = ipred < 0 ? -((-ipred + round_val) >> prec_bits) :
                               ((ipred + round_val) >> prec_bits);
    }

  for (r = 0; r < bs; r++) {
    for (c = 0; c < bs; c++) {
      ipred = pred[r + 1][c + 1] + mean;
      ypred_ptr[c] = clip_pixel(ipred);
    }
    ypred_ptr += y_stride;
  }
}

static void build_filter_intra_predictors(uint8_t *src, int src_stride,
                                          uint8_t *pred_ptr, int stride,
                                          MB_PREDICTION_MODE mode, TX_SIZE txsz,
                                          int up_available, int left_available,
                                          int right_available) {
  int i;
  DECLARE_ALIGNED_ARRAY(16, uint8_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, yabove_data, 128 + 16);
  uint8_t *above_row = yabove_data + 16;
  const int bs = 4 << txsz;

  if (left_available) {
    for (i = 0; i < bs; i++)
      left_col[i] = src[i * src_stride - 1];
  } else {
    vpx_memset(left_col, 129, bs);
  }

  if (up_available) {
    uint8_t *above_ptr = src - src_stride;
    if (bs == 4 && right_available && left_available) {
      above_row = above_ptr;
    } else {
      vpx_memcpy(above_row, above_ptr, bs);
      if (bs == 4 && right_available)
        vpx_memcpy(above_row + bs, above_ptr + bs, bs);
      else
        vpx_memset(above_row + bs, above_row[bs - 1], bs);
      above_row[-1] = left_available ? above_ptr[-1] : 129;
    }
  } else {
    vpx_memset(above_row, 127, bs * 2);
    above_row[-1] = 127;
  }

  filter_intra_predictors(pred_ptr, stride, bs, above_row, left_col, mode);
}
#endif

void vp9_predict_intra_block(MACROBLOCKD *xd,
                            int block_idx,
                            int bwl_in,
                            TX_SIZE tx_size,
                            int mode,
#if CONFIG_FILTERINTRA
                            int filterbit,
#endif
                            uint8_t *reference, int ref_stride,
                            uint8_t *predictor, int pre_stride) {
=======
void vp9_predict_intra_block(const MACROBLOCKD *xd, int block_idx, int bwl_in,
                             TX_SIZE tx_size, PREDICTION_MODE mode,
                             const uint8_t *ref, int ref_stride,
                             uint8_t *dst, int dst_stride,
                             int aoff, int loff, int plane) {
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
  const int bwl = bwl_in - tx_size;
  const int wmask = (1 << bwl) - 1;
  const int have_top = (block_idx >> bwl) || xd->up_available;
  const int have_left = (block_idx & wmask) || xd->left_available;
  const int have_right = ((block_idx & wmask) != wmask);
<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_FILTERINTRA
  int filterflag = is_filter_allowed(mode) && (tx_size <= TX_8X8) && filterbit;
#endif
=======
  const int x = aoff * 4;
  const int y = loff * 4;
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")

  assert(bwl >= 0);
<<<<<<< HEAD   (a5a742 Redesigned recursive filters adapted to block-sizes)
#if CONFIG_FILTERINTRA
  if (!filterflag) {
#endif
  build_intra_predictors(reference, ref_stride,
                         predictor, pre_stride,
                         mode,
                         tx_size,
                         have_top, have_left,
                         have_right);
#if CONFIG_FILTERINTRA
  } else {
    build_filter_intra_predictors(reference, ref_stride,
                                  predictor, pre_stride,
                                  mode,
                                  tx_size,
                                  have_top, have_left,
                                  have_right);
  }
#endif
=======
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
>>>>>>> BRANCH (297717 Merge "Add adaptation option for VBR.")
}

#if CONFIG_INTERINTRA
// Intra predictor for the second square block in interintra prediction.
// Prediction of the first block (in pred_ptr) will be used to generate half of
// the boundary values.
static void build_intra_predictors_for_2nd_block_interintra
                                   (uint8_t *src, int src_stride,
                                   uint8_t *pred_ptr, int stride,
                                   MB_PREDICTION_MODE mode, TX_SIZE txsz,
                                   int up_available, int left_available,
                                   int right_available, int bwltbh) {
  int i;
  DECLARE_ALIGNED_ARRAY(16, uint8_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, yabove_data, 128 + 16);
  uint8_t *above_row = yabove_data + 16;
  const int bs = 4 << txsz;

  // 127 127 127 .. 127 127 127 127 127 127
  // 129  A   B  ..  Y   Z
  // 129  C   D  ..  W   X
  // 129  E   F  ..  U   V
  // 129  G   H  ..  S   T   T   T   T   T
  // ..

  once(init_intra_pred_fn_ptrs);
  if (left_available) {
    for (i = 0; i < bs; i++) {
      if (bwltbh)
        left_col[i] = src[i * src_stride - 1];
      else
        left_col[i] = pred_ptr[i * stride - 1];
    }
  } else {
    vpx_memset(left_col, 129, bs);
  }

  if (up_available) {
    uint8_t *above_ptr;
    if (bwltbh)
      above_ptr = pred_ptr - stride;
    else
      above_ptr = src - src_stride;
    if (bs == 4 && right_available && left_available) {
      above_row = above_ptr;
    } else {
      vpx_memcpy(above_row, above_ptr, bs);
      if (bs == 4 && right_available)
        vpx_memcpy(above_row + bs, above_ptr + bs, bs);
      else
        vpx_memset(above_row + bs, above_row[bs - 1], bs);
      above_row[-1] = left_available ? above_ptr[-1] : 129;
    }
  } else {
    vpx_memset(above_row, 127, bs * 2);
    above_row[-1] = 127;
  }

  if (mode == DC_PRED) {
    dc_pred[left_available][up_available][txsz](pred_ptr, stride,
                                                above_row, left_col);
  } else {
    pred[mode][txsz](pred_ptr, stride, above_row, left_col);
  }
}

#if CONFIG_MASKED_INTERINTRA
#define MASK_WEIGHT_BITS_INTERINTRA 6

static int get_masked_weight_interintra(int m) {
  #define SMOOTHER_LEN_INTERINTRA  32
  static const uint8_t smoothfn[2 * SMOOTHER_LEN_INTERINTRA + 1] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  1,  1,  1,
      1,  1,  2,  2,  3,  4,  5,  6,
      8,  9, 12, 14, 17, 21, 24, 28,
      32,
      36, 40, 43, 47, 50, 52, 55, 56,
      58, 59, 60, 61, 62, 62, 63, 63,
      63, 63, 63, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64,
  };
  if (m < -SMOOTHER_LEN_INTERINTRA)
    return 0;
  else if (m > SMOOTHER_LEN_INTERINTRA)
    return (1 << MASK_WEIGHT_BITS_INTERINTRA);
  else
    return smoothfn[m + SMOOTHER_LEN_INTERINTRA];
}

static int get_hard_mask_interintra(int m) {
  return m > 0;
}

// Equation of line: f(x, y) = a[0]*(x - a[2]*w/4) + a[1]*(y - a[3]*h/4) = 0
// The soft mask is obtained by computing f(x, y) and then calling
// get_masked_weight(f(x, y)).
static const int mask_params_sml_interintra[1 << MASK_BITS_SML_INTERINTRA]
                                            [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},
};

static const int mask_params_med_hgtw_interintra[1 << MASK_BITS_MED_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  {-1,  2, 2, 1},
  { 1, -2, 2, 1},
  {-1,  2, 2, 3},
  { 1, -2, 2, 3},
  { 1,  2, 2, 1},
  {-1, -2, 2, 1},
  { 1,  2, 2, 3},
  {-1, -2, 2, 3},
};

static const int mask_params_med_hltw_interintra[1 << MASK_BITS_MED_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  {-2,  1, 1, 2},
  { 2, -1, 1, 2},
  {-2,  1, 3, 2},
  { 2, -1, 3, 2},
  { 2,  1, 1, 2},
  {-2, -1, 1, 2},
  { 2,  1, 3, 2},
  {-2, -1, 3, 2},
};

static const int mask_params_med_heqw_interintra[1 << MASK_BITS_MED_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  { 0,  2, 0, 1},
  { 0, -2, 0, 1},
  { 0,  2, 0, 3},
  { 0, -2, 0, 3},
  { 2,  0, 1, 0},
  {-2,  0, 1, 0},
  { 2,  0, 3, 0},
  {-2,  0, 3, 0},
};

static const int mask_params_big_hgtw_interintra[1 << MASK_BITS_BIG_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  {-1,  2, 2, 1},
  { 1, -2, 2, 1},
  {-1,  2, 2, 3},
  { 1, -2, 2, 3},
  { 1,  2, 2, 1},
  {-1, -2, 2, 1},
  { 1,  2, 2, 3},
  {-1, -2, 2, 3},

  {-2,  1, 1, 2},
  { 2, -1, 1, 2},
  {-2,  1, 3, 2},
  { 2, -1, 3, 2},
  { 2,  1, 1, 2},
  {-2, -1, 1, 2},
  { 2,  1, 3, 2},
  {-2, -1, 3, 2},

  { 0,  2, 0, 1},
  { 0, -2, 0, 1},
  { 0,  2, 0, 2},
  { 0, -2, 0, 2},
  { 0,  2, 0, 3},
  { 0, -2, 0, 3},
  { 2,  0, 2, 0},
  {-2,  0, 2, 0},
};

static const int mask_params_big_hltw_interintra[1 << MASK_BITS_BIG_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  {-1,  2, 2, 1},
  { 1, -2, 2, 1},
  {-1,  2, 2, 3},
  { 1, -2, 2, 3},
  { 1,  2, 2, 1},
  {-1, -2, 2, 1},
  { 1,  2, 2, 3},
  {-1, -2, 2, 3},

  {-2,  1, 1, 2},
  { 2, -1, 1, 2},
  {-2,  1, 3, 2},
  { 2, -1, 3, 2},
  { 2,  1, 1, 2},
  {-2, -1, 1, 2},
  { 2,  1, 3, 2},
  {-2, -1, 3, 2},

  { 0,  2, 0, 2},
  { 0, -2, 0, 2},
  { 2,  0, 1, 0},
  {-2,  0, 1, 0},
  { 2,  0, 2, 0},
  {-2,  0, 2, 0},
  { 2,  0, 3, 0},
  {-2,  0, 3, 0},
};

static const int mask_params_big_heqw_interintra[1 << MASK_BITS_BIG_INTERINTRA]
                                                 [4] = {
  {-1,  2, 2, 2},
  { 1, -2, 2, 2},
  {-2,  1, 2, 2},
  { 2, -1, 2, 2},
  { 2,  1, 2, 2},
  {-2, -1, 2, 2},
  { 1,  2, 2, 2},
  {-1, -2, 2, 2},

  {-1,  2, 2, 1},
  { 1, -2, 2, 1},
  {-1,  2, 2, 3},
  { 1, -2, 2, 3},
  { 1,  2, 2, 1},
  {-1, -2, 2, 1},
  { 1,  2, 2, 3},
  {-1, -2, 2, 3},

  {-2,  1, 1, 2},
  { 2, -1, 1, 2},
  {-2,  1, 3, 2},
  { 2, -1, 3, 2},
  { 2,  1, 1, 2},
  {-2, -1, 1, 2},
  { 2,  1, 3, 2},
  {-2, -1, 3, 2},

  { 0,  2, 0, 1},
  { 0, -2, 0, 1},
  { 0,  2, 0, 3},
  { 0, -2, 0, 3},
  { 2,  0, 1, 0},
  {-2,  0, 1, 0},
  { 2,  0, 3, 0},
  {-2,  0, 3, 0},
};

static const int *get_mask_params_interintra(int mask_index,
                                             BLOCK_SIZE_TYPE sb_type,
                                             int h, int w) {
  const int *a;
  const int mask_bits = get_mask_bits_interintra(sb_type);

  if (mask_index == MASK_NONE_INTERINTRA)
    return NULL;

  if (mask_bits == MASK_BITS_SML_INTERINTRA) {
    a = mask_params_sml_interintra[mask_index];
  } else if (mask_bits == MASK_BITS_MED_INTERINTRA) {
    if (h > w)
      a = mask_params_med_hgtw_interintra[mask_index];
    else if (h < w)
      a = mask_params_med_hltw_interintra[mask_index];
    else
      a = mask_params_med_heqw_interintra[mask_index];
  } else if (mask_bits == MASK_BITS_BIG_INTERINTRA) {
    if (h > w)
      a = mask_params_big_hgtw_interintra[mask_index];
    else if (h < w)
      a = mask_params_big_hltw_interintra[mask_index];
    else
      a = mask_params_big_heqw_interintra[mask_index];
  } else {
    assert(0);
  }
  return a;
}

void vp9_generate_masked_weight_interintra(int mask_index,
                                           BLOCK_SIZE_TYPE sb_type,
                                           int h, int w,
                                           uint8_t *mask, int stride) {
  int i, j;
  const int *a = get_mask_params_interintra(mask_index, sb_type, h, w);
  if (!a) return;
  for (i = 0; i < h; ++i)
    for (j = 0; j < w; ++j) {
      int x = (j - (a[2] * w) / 4);
      int y = (i - (a[3] * h) / 4);
      int m = a[0] * x + a[1] * y;
      mask[i * stride + j] = get_masked_weight_interintra(m);
    }
}

void vp9_generate_hard_mask_interintra(int mask_index, BLOCK_SIZE_TYPE sb_type,
                            int h, int w, uint8_t *mask, int stride) {
  int i, j;
  const int *a = get_mask_params_interintra(mask_index, sb_type, h, w);
  if (!a) return;
  for (i = 0; i < h; ++i)
    for (j = 0; j < w; ++j) {
      int x = (j - (a[2] * w) / 4);
      int y = (i - (a[3] * h) / 4);
      int m = a[0] * x + a[1] * y;
      mask[i * stride + j] = get_hard_mask_interintra(m);
    }
}
#endif

static void combine_interintra(MB_PREDICTION_MODE mode,
#if CONFIG_MASKED_INTERINTRA
                               int use_masked_interintra,
                               int mask_index,
                               BLOCK_SIZE_TYPE bsize,
#endif
                               uint8_t *interpred,
                               int interstride,
                               uint8_t *intrapred,
                               int intrastride,
                               int bw, int bh) {
  static const int scale_bits = 8;
  static const int scale_max = 256;
  static const int scale_round = 127;
  static const int weights1d[64] = {
      128, 125, 122, 119, 116, 114, 111, 109,
      107, 105, 103, 101,  99,  97,  96,  94,
       93,  91,  90,  89,  88,  86,  85,  84,
       83,  82,  81,  81,  80,  79,  78,  78,
       77,  76,  76,  75,  75,  74,  74,  73,
       73,  72,  72,  71,  71,  71,  70,  70,
       70,  70,  69,  69,  69,  69,  68,  68,
       68,  68,  68,  67,  67,  67,  67,  67,
  };

  int size = MAX(bw, bh);
  int size_scale = (size >= 64 ? 1 :
                    size == 32 ? 2 :
                    size == 16 ? 4 :
                    size == 8  ? 8 : 16);
  int i, j;

#if CONFIG_MASKED_INTERINTRA
  uint8_t mask[4096];
  if (use_masked_interintra && get_mask_bits_interintra(bsize))
    vp9_generate_masked_weight_interintra(mask_index, bsize, bh, bw, mask, bw);
#endif

  switch (mode) {
    case V_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = weights1d[i * size_scale];
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case H_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = weights1d[j * size_scale];
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case D63_PRED:
    case D117_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = (weights1d[i * size_scale] * 3 +
                       weights1d[j * size_scale]) >> 2;
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case D27_PRED:
    case D153_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = (weights1d[j * size_scale] * 3 +
                       weights1d[i * size_scale]) >> 2;
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case D135_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = weights1d[(i < j ? i : j) * size_scale];
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case D45_PRED:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
          int scale = (weights1d[i * size_scale] +
                       weights1d[j * size_scale]) >> 1;
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] =
              ((scale_max - scale) * interpred[k] +
                  scale * intrapred[i * intrastride + j] + scale_round)
                  >> scale_bits;
        }
      }
     break;

    case TM_PRED:
    case DC_PRED:
    default:
      for (i = 0; i < bh; ++i) {
        for (j = 0; j < bw; ++j) {
          int k = i * interstride + j;
#if CONFIG_MASKED_INTERINTRA
          int m = mask[i * bw + j];
          if (use_masked_interintra && get_mask_bits_interintra(bsize))
              interpred[k] = (intrapred[i * intrastride + j] * m +
                              interpred[k] *
                              ((1 << MASK_WEIGHT_BITS_INTERINTRA) - m) +
                              (1 << (MASK_WEIGHT_BITS_INTERINTRA - 1))) >>
                              MASK_WEIGHT_BITS_INTERINTRA;
          else
#endif
          interpred[k] = (interpred[k] + intrapred[i * intrastride + j]) >> 1;
        }
      }
      break;
  }
}

// Break down rectangular intra prediction for joint spatio-temporal prediction
// into two square intra predictions.
static void build_intra_predictors_for_interintra(uint8_t *src, int src_stride,
                                           uint8_t *pred_ptr, int stride,
                                           MB_PREDICTION_MODE mode,
                                           int bw, int bh,
                                           int up_available, int left_available,
                                           int right_available) {
  if (bw == bh) {
    build_intra_predictors(src, src_stride, pred_ptr, stride,
                           mode, intra_size_log2_for_interintra(bw),
                           up_available, left_available, right_available);
  } else if (bw < bh) {
    uint8_t *src_bottom = src + bw * src_stride;
    uint8_t *pred_ptr_bottom = pred_ptr + bw * stride;
    build_intra_predictors(src, src_stride, pred_ptr, stride,
                           mode, intra_size_log2_for_interintra(bw),
                           up_available, left_available, right_available);
    build_intra_predictors_for_2nd_block_interintra(src_bottom, src_stride,
                                                    pred_ptr_bottom, stride,
                                       mode, intra_size_log2_for_interintra(bw),
                                       1, left_available, right_available, 1);
  } else {
    uint8_t *src_right = src + bh;
    uint8_t *pred_ptr_right = pred_ptr + bh;
    build_intra_predictors(src, src_stride, pred_ptr, stride,
                           mode, intra_size_log2_for_interintra(bh),
                           up_available, left_available, right_available);
    build_intra_predictors_for_2nd_block_interintra(src_right, src_stride,
                                                    pred_ptr_right, stride,
                                       mode, intra_size_log2_for_interintra(bh),
                                       up_available, 1, right_available, 0);
  }
}

void vp9_build_interintra_predictors_sby(MACROBLOCKD *xd,
                                         uint8_t *ypred,
                                         int ystride,
                                         BLOCK_SIZE_TYPE bsize) {
  const struct macroblockd_plane* const pd = &xd->plane[0];
  const int bw = plane_block_width(bsize, pd);
  const int bh = plane_block_height(bsize, pd);
  uint8_t intrapredictor[4096];
  build_intra_predictors_for_interintra(
      xd->plane[0].dst.buf, xd->plane[0].dst.stride,
      intrapredictor, bw,
      xd->mode_info_context->mbmi.interintra_mode, bw, bh,
      xd->up_available, xd->left_available, xd->right_available);
  combine_interintra(xd->mode_info_context->mbmi.interintra_mode,
#if CONFIG_MASKED_INTERINTRA
                     xd->mode_info_context->mbmi.use_masked_interintra,
                     xd->mode_info_context->mbmi.interintra_mask_index,
                     bsize,
#endif
                     ypred, ystride, intrapredictor, bw, bw, bh);
}

void vp9_build_interintra_predictors_sbuv(MACROBLOCKD *xd,
                                          uint8_t *upred,
                                          uint8_t *vpred,
                                          int uvstride,
                                          BLOCK_SIZE_TYPE bsize) {
  int bwl = b_width_log2(bsize), bw = 2 << bwl;
  int bhl = b_height_log2(bsize), bh = 2 << bhl;
  uint8_t uintrapredictor[1024];
  uint8_t vintrapredictor[1024];
  build_intra_predictors_for_interintra(
      xd->plane[1].dst.buf, xd->plane[1].dst.stride,
      uintrapredictor, bw,
      xd->mode_info_context->mbmi.interintra_uv_mode, bw, bh,
      xd->up_available, xd->left_available, xd->right_available);
  build_intra_predictors_for_interintra(
      xd->plane[2].dst.buf, xd->plane[1].dst.stride,
      vintrapredictor, bw,
      xd->mode_info_context->mbmi.interintra_uv_mode, bw, bh,
      xd->up_available, xd->left_available, xd->right_available);
  combine_interintra(xd->mode_info_context->mbmi.interintra_uv_mode,
#if CONFIG_MASKED_INTERINTRA
                     xd->mode_info_context->mbmi.use_masked_interintra,
                     xd->mode_info_context->mbmi.interintra_uv_mask_index,
                     bsize,
#endif
                     upred, uvstride, uintrapredictor, bw, bw, bh);
  combine_interintra(xd->mode_info_context->mbmi.interintra_uv_mode,
#if CONFIG_MASKED_INTERINTRA
                     xd->mode_info_context->mbmi.use_masked_interintra,
                     xd->mode_info_context->mbmi.interintra_uv_mask_index,
                     bsize,
#endif
                     vpred, uvstride, vintrapredictor, bw, bw, bh);
}

void vp9_build_interintra_predictors(MACROBLOCKD *xd,
                                     uint8_t *ypred,
                                     uint8_t *upred,
                                     uint8_t *vpred,
                                     int ystride, int uvstride,
                                     BLOCK_SIZE_TYPE bsize) {
  vp9_build_interintra_predictors_sby(xd, ypred, ystride, bsize);
  vp9_build_interintra_predictors_sbuv(xd, upred, vpred, uvstride, bsize);
}
#endif
