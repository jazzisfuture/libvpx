/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "vp9/common/mips/msa/vp9_types_msa.h"
#include "vp9/common/mips/msa/vp9_macros_msa.h"

#if HAVE_MSA
static void intra_predict_vert_4x4_msa(const uint8_t *src,
                                       uint8_t *dst,
                                       int32_t dst_stride) {
  uint32_t src_data;

  src_data = LOAD_WORD(src);

  STORE_WORD(dst, src_data);
  dst += dst_stride;
  STORE_WORD(dst, src_data);
  dst += dst_stride;
  STORE_WORD(dst, src_data);
  dst += dst_stride;
  STORE_WORD(dst, src_data);
}

static void intra_predict_vert_8x8_msa(const uint8_t *src,
                                       uint8_t *dst,
                                       int32_t dst_stride) {
  uint32_t row;
  uint32_t src_data1, src_data2;

  src_data1 = LOAD_WORD(src);
  src_data2 = LOAD_WORD(src + 4);

  for (row = 8; row--;) {
    STORE_WORD(dst, src_data1);
    STORE_WORD((dst + 4), src_data2);
    dst += dst_stride;
  }
}

static void intra_predict_vert_16x16_msa(const uint8_t *src,
                                         uint8_t *dst,
                                         int32_t dst_stride) {
  uint32_t row;
  VUINT8 src0;

  src0 = LOAD_B(src);

  for (row = 16; row--;) {
    STORE_B(src0, dst);
    dst += dst_stride;
  }
}

static void intra_predict_vert_32x32_msa(const uint8_t *src,
                                         uint8_t *dst,
                                         int32_t dst_stride) {
  uint32_t row;
  VUINT8 src1, src2;

  src1 = LOAD_B(src);
  src2 = LOAD_B(src + 16);

  for (row = 32; row--;) {
    STORE_B(src1, dst);
    STORE_B(src2, (dst + 16));
    dst += dst_stride;
  }
}

static void intra_predict_horiz_4x4_msa(const uint8_t *src,
                                        int32_t src_stride,
                                        uint8_t *dst,
                                        int32_t dst_stride) {
  uint8_t inp;
  uint32_t out0, out1, out2, out3;
  VUINT8 src0, src1, src2, src3;

  inp = src[0 * src_stride];
  src0 = __fill_b(inp);
  inp = src[1 * src_stride];
  src1 = __fill_b(inp);
  inp = src[2 * src_stride];
  src2 = __fill_b(inp);
  inp = src[3 * src_stride];
  src3 = __fill_b(inp);

  out0 = __copy_u_w(src0, 0);
  out1 = __copy_u_w(src1, 0);
  out2 = __copy_u_w(src2, 0);
  out3 = __copy_u_w(src3, 0);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void intra_predict_horiz_8x8_msa(const uint8_t *src,
                                        int32_t src_stride,
                                        uint8_t *dst,
                                        int32_t dst_stride) {
  uint8_t inp0, inp1;
  uint32_t row;
  uint32_t out0, out1, out2, out3;
  VUINT8 src0, src1, src2, src3;

  for (row = 0; row < 2; row++) {
    inp0 = src[0];
    src += src_stride;
    src0 = __fill_b(inp0);
    inp1 = src[0];
    src += src_stride;
    src1 = __fill_b(inp1);

    inp0 = src[0];
    src += src_stride;
    src2 = __fill_b(inp0);
    inp1 = src[0];
    src += src_stride;
    src3 = __fill_b(inp1);

    out0 = __copy_u_w(src0, 0);
    out1 = __copy_u_w(src1, 0);
    out2 = __copy_u_w(src2, 0);
    out3 = __copy_u_w(src3, 0);

    STORE_WORD(dst, out0);
    STORE_WORD((dst + 4), out0);
    dst += dst_stride;
    STORE_WORD(dst, out1);
    STORE_WORD((dst + 4), out1);
    dst += dst_stride;
    STORE_WORD(dst, out2);
    STORE_WORD((dst + 4), out2);
    dst += dst_stride;
    STORE_WORD(dst, out3);
    STORE_WORD((dst + 4), out3);
    dst += dst_stride;
  }
}

static void intra_predict_horiz_16x16_msa(const uint8_t *src,
                                          int32_t src_stride,
                                          uint8_t *dst,
                                          int32_t dst_stride) {
  uint32_t row;
  uint8_t inp0, inp1, inp2, inp3;
  VUINT8 src0, src1, src2, src3;

  for (row = 0; row < 4; row++) {
    inp0 = src[0];
    src += src_stride;
    inp1 = src[0];
    src += src_stride;
    inp2 = src[0];
    src += src_stride;
    inp3 = src[0];
    src += src_stride;

    src0 = __fill_b(inp0);
    src1 = __fill_b(inp1);
    src2 = __fill_b(inp2);
    src3 = __fill_b(inp3);

    STORE_B(src0, dst);
    dst += dst_stride;
    STORE_B(src1, dst);
    dst += dst_stride;
    STORE_B(src2, dst);
    dst += dst_stride;
    STORE_B(src3, dst);
    dst += dst_stride;
  }
}

static void intra_predict_horiz_32x32_msa(const uint8_t *src,
                                          int32_t src_stride,
                                          uint8_t *dst,
                                          int32_t dst_stride) {
  uint32_t row;
  uint8_t inp0, inp1;
  VUINT8 src0, src1;

  for (row = 0; row < 16; row++) {
    inp0 = src[0];
    src += src_stride;
    inp1 = src[0];
    src += src_stride;

    src0 = __fill_b(inp0);
    src1 = __fill_b(inp1);

    STORE_B(src0, dst);
    STORE_B(src0, (dst + 16));
    dst += dst_stride;
    STORE_B(src1, dst);
    STORE_B(src1, (dst + 16));
    dst += dst_stride;
  }
}

static void intra_predict_dc_4x4_msa(const uint8_t *src_top,
                                     const uint8_t *src_left,
                                     int32_t src_stride_left,
                                     uint8_t *dst,
                                     int32_t dst_stride,
                                     uint8_t is_above,
                                     uint8_t is_left) {
  uint32_t row;
  uint32_t addition = 0;
  uint32_t out;
  VUINT8 src_above, store;
  VUINT16 sum_above;
  VUINT32 sum;

  if (is_left && is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum = __hadd_u_w(sum_above, sum_above);
    addition = __copy_u_w(sum, 0);

    for (row = 0; row < 4; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 4) >> 3;
    store = __fill_b(addition);
  } else if (is_left) {
    for (row = 0; row < 4; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 2) >> 2;
    store = __fill_b(addition);
  } else if (is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum = __hadd_u_w(sum_above, sum_above);
    sum = __srari_w(sum, 2);
    store = __splati_b(sum, 0);
  } else {
    store = __ldi_b(128);
  }

  out = __copy_u_w(store, 0);

  for (row = 4; row--;) {
    STORE_WORD(dst, out);
    dst += dst_stride;
  }
}

static void intra_predict_dc_8x8_msa(const uint8_t *src_top,
                                     const uint8_t *src_left,
                                     int32_t src_stride_left,
                                     uint8_t *dst,
                                     int32_t dst_stride,
                                     uint8_t is_above,
                                     uint8_t is_left) {
  uint32_t row;
  uint32_t addition = 0;
  uint32_t out;
  VUINT8 src_above, store;
  VUINT16 sum_above;
  VUINT32 sum_top;
  VUINT64 sum;

  if (is_left && is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    addition = __copy_u_w(sum, 0);

    for (row = 0; row < 8; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 8) >> 4;
    store = __fill_b(addition);
  } else if (is_left) {
    for (row = 0; row < 8; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 4) >> 3;
    store = __fill_b(addition);
  } else if (is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    sum = __srari_d(sum, 3);
    store = __splati_b(sum, 0);
  } else {
    store = __ldi_b(128);
  }

  out = __copy_u_w(store, 0);

  for (row = 8; row--;) {
    STORE_WORD(dst, out);
    STORE_WORD((dst + 4), out);
    dst += dst_stride;
  }
}

static void intra_predict_dc_16x16_msa(const uint8_t *src_top,
                                       const uint8_t *src_left,
                                       int32_t src_stride_left,
                                       uint8_t *dst,
                                       int32_t dst_stride,
                                       uint8_t is_above,
                                       uint8_t is_left) {
  uint32_t row;
  uint32_t addition = 0;
  VUINT8 src_above, store;
  VUINT16 sum_above;
  VUINT32 sum_top;
  VUINT64 sum;

  if (is_left && is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    sum_top = __pckev_w(sum, sum);
    sum = __hadd_u_d(sum_top, sum_top);
    addition = __copy_u_w(sum, 0);

    for (row = 0; row < 16; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 16) >> 5;
    store = __fill_b(addition);
  } else if (is_left) {
    for (row = 0; row < 16; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 8) >> 4;
    store = __fill_b(addition);
  } else if (is_above) {
    src_above = LOAD_B(src_top);

    sum_above = __hadd_u_h(src_above, src_above);
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    sum_top = __pckev_w(sum, sum);
    sum = __hadd_u_d(sum_top, sum_top);
    sum = __srari_d(sum, 4);
    store = __splati_b(sum, 0);
  } else {
    store = __ldi_b(128);
  }

  for (row = 16; row--;) {
    STORE_B(store, dst);
    dst += dst_stride;
  }
}

static void intra_predict_dc_32x32_msa(const uint8_t *src_top,
                                       const uint8_t *src_left,
                                       int32_t src_stride_left,
                                       uint8_t *dst,
                                       int32_t dst_stride,
                                       uint8_t is_above,
                                       uint8_t is_left) {
  uint32_t row;
  uint32_t addition = 0;
  VUINT8 src_above1, src_above2, store;
  VUINT16 sum_above1, sum_above2, sum_above;
  VUINT32 sum_top;
  VUINT64 sum;

  if (is_left && is_above) {
    src_above1 = LOAD_B(src_top);
    src_above2 = LOAD_B(src_top + 16);

    sum_above1 = __hadd_u_h(src_above1, src_above1);
    sum_above2 = __hadd_u_h(src_above2, src_above2);

    sum_above = sum_above1 + sum_above2;
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    sum_top = __pckev_w(sum, sum);
    sum = __hadd_u_d(sum_top, sum_top);
    addition = __copy_u_w(sum, 0);

    for (row = 0; row < 32; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 32) >> 6;
    store = __fill_b(addition);
  } else if (is_left) {
    for (row = 0; row < 32; row++) {
      addition += src_left[row * src_stride_left];
    }

    addition = (addition + 16) >> 5;
    store = __fill_b(addition);
  } else if (is_above) {
    src_above1 = LOAD_B(src_top);
    src_above2 = LOAD_B(src_top + 16);

    sum_above1 = __hadd_u_h(src_above1, src_above1);
    sum_above2 = __hadd_u_h(src_above2, src_above2);

    sum_above = sum_above1 + sum_above2;
    sum_top = __hadd_u_w(sum_above, sum_above);
    sum = __hadd_u_d(sum_top, sum_top);
    sum_top = __pckev_w(sum, sum);
    sum = __hadd_u_d(sum_top, sum_top);
    sum = __srari_d(sum, 5);
    store = __splati_b(sum, 0);
  } else {
    store = __ldi_b(128);
  }

  for (row = 32; row--;) {
    STORE_B(store, dst);
    STORE_B(store, (dst + 16));
    dst += dst_stride;
  }
}

static void intra_predict_tm_4x4_msa(const uint8_t *src_top_ptr,
                                     const uint8_t *src_left,
                                     int32_t src_left_stride,
                                     uint8_t *dst,
                                     int32_t dst_stride) {
  uint8_t top_left = src_top_ptr[-1];
  uint32_t out0, out1, out2, out3;
  VUINT8 src_top, src_left0, src_left1, src_left2, src_left3;
  VUINT16 src_top_left;
  VUINT16 vec0, vec1, vec2, vec3;
  VUINT8 res0, res1, res2, res3;

  src_top_left = __fill_h(top_left);
  src_top = LOAD_B(src_top_ptr);

  src_left0 = __fill_b(src_left[0]);
  src_left += src_left_stride;
  src_left1 = __fill_b(src_left[0]);
  src_left += src_left_stride;
  src_left2 = __fill_b(src_left[0]);
  src_left += src_left_stride;
  src_left3 = __fill_b(src_left[0]);

  vec0 = __ilvr_b(src_left0, src_top);
  vec1 = __ilvr_b(src_left1, src_top);
  vec2 = __ilvr_b(src_left2, src_top);
  vec3 = __ilvr_b(src_left3, src_top);

  vec0 = __hadd_u_h(vec0, vec0);
  vec1 = __hadd_u_h(vec1, vec1);
  vec2 = __hadd_u_h(vec2, vec2);
  vec3 = __hadd_u_h(vec3, vec3);

  vec0 = __subs_u_h(vec0, src_top_left);
  vec1 = __subs_u_h(vec1, src_top_left);
  vec2 = __subs_u_h(vec2, src_top_left);
  vec3 = __subs_u_h(vec3, src_top_left);

  vec0 = __sat_u_h(vec0, 7);
  vec1 = __sat_u_h(vec1, 7);
  vec2 = __sat_u_h(vec2, 7);
  vec3 = __sat_u_h(vec3, 7);

  res0 = __pckev_b(vec0, vec0);
  res1 = __pckev_b(vec1, vec1);
  res2 = __pckev_b(vec2, vec2);
  res3 = __pckev_b(vec3, vec3);

  out0 = __copy_u_w(res0, 0);
  out1 = __copy_u_w(res1, 0);
  out2 = __copy_u_w(res2, 0);
  out3 = __copy_u_w(res3, 0);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void intra_predict_tm_8x8_msa(const uint8_t *src_top_ptr,
                                     const uint8_t *src_left,
                                     int32_t src_left_stride,
                                     uint8_t *dst,
                                     int32_t dst_stride) {
  uint8_t top_left = src_top_ptr[-1];
  uint32_t loop_cnt;
  VUINT8 src_top, src_left0, src_left1, src_left2, src_left3;
  VUINT16 src_top_left;
  VUINT16 vec0, vec1, vec2, vec3;

  src_top = LOAD_B(src_top_ptr);
  src_top_left = __fill_h(top_left);

  for (loop_cnt = 2; loop_cnt--;) {
    src_left0 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left1 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left2 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left3 = __fill_b(src_left[0]);
    src_left += src_left_stride;

    vec0 = __ilvr_b(src_left0, src_top);
    vec1 = __ilvr_b(src_left1, src_top);
    vec2 = __ilvr_b(src_left2, src_top);
    vec3 = __ilvr_b(src_left3, src_top);

    vec0 = __hadd_u_h(vec0, vec0);
    vec1 = __hadd_u_h(vec1, vec1);
    vec2 = __hadd_u_h(vec2, vec2);
    vec3 = __hadd_u_h(vec3, vec3);

    vec0 = __subs_u_h(vec0, src_top_left);
    vec1 = __subs_u_h(vec1, src_top_left);
    vec2 = __subs_u_h(vec2, src_top_left);
    vec3 = __subs_u_h(vec3, src_top_left);

    vec0 = __sat_u_h(vec0, 7);
    vec1 = __sat_u_h(vec1, 7);
    vec2 = __sat_u_h(vec2, 7);
    vec3 = __sat_u_h(vec3, 7);

    PCKEV_B_STORE_8_BYTES_4(vec0, vec1, vec2, vec3, dst, dst_stride);
    dst += (4 * dst_stride);
  }
}

static void intra_predict_tm_16x16_msa(const uint8_t *src_top_ptr,
                                       const uint8_t *src_left,
                                       int32_t src_left_stride,
                                       uint8_t *dst,
                                       int32_t dst_stride) {
  uint8_t top_left = src_top_ptr[-1];
  uint32_t loop_cnt;
  VUINT8 src_top, src_left0, src_left1, src_left2, src_left3;
  VUINT16 src_top_left;
  VUINT16 res_r, res_l;
  VUINT8 res;

  src_top = LOAD_B(src_top_ptr);
  src_top_left = __fill_h(top_left);

  for (loop_cnt = 4; loop_cnt--;) {
    src_left0 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left1 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left2 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left3 = __fill_b(src_left[0]);
    src_left += src_left_stride;

    res_r = __ilvr_b(src_left0, src_top);
    res_l = __ilvl_b(src_left0, src_top);
    res_r = __hadd_u_h(res_r, res_r);
    res_l = __hadd_u_h(res_l, res_l);
    res_r = __subs_u_h(res_r, src_top_left);
    res_l = __subs_u_h(res_l, src_top_left);
    res_r = __sat_u_h(res_r, 7);
    res_l = __sat_u_h(res_l, 7);
    res = __pckev_b(res_l, res_r);

    STORE_B(res, dst);
    dst += dst_stride;

    res_r = __ilvr_b(src_left1, src_top);
    res_l = __ilvl_b(src_left1, src_top);
    res_r = __hadd_u_h(res_r, res_r);
    res_l = __hadd_u_h(res_l, res_l);
    res_r = __subs_u_h(res_r, src_top_left);
    res_l = __subs_u_h(res_l, src_top_left);
    res_r = __sat_u_h(res_r, 7);
    res_l = __sat_u_h(res_l, 7);
    res = __pckev_b(res_l, res_r);

    STORE_B(res, dst);
    dst += dst_stride;

    res_r = __ilvr_b(src_left2, src_top);
    res_l = __ilvl_b(src_left2, src_top);
    res_r = __hadd_u_h(res_r, res_r);
    res_l = __hadd_u_h(res_l, res_l);
    res_r = __subs_u_h(res_r, src_top_left);
    res_l = __subs_u_h(res_l, src_top_left);
    res_r = __sat_u_h(res_r, 7);
    res_l = __sat_u_h(res_l, 7);
    res = __pckev_b(res_l, res_r);

    STORE_B(res, dst);
    dst += dst_stride;

    res_r = __ilvr_b(src_left3, src_top);
    res_l = __ilvl_b(src_left3, src_top);
    res_r = __hadd_u_h(res_r, res_r);
    res_l = __hadd_u_h(res_l, res_l);
    res_l = __subs_u_h(res_l, src_top_left);
    res_r = __subs_u_h(res_r, src_top_left);
    res_r = __sat_u_h(res_r, 7);
    res_l = __sat_u_h(res_l, 7);
    res = __pckev_b(res_l, res_r);

    STORE_B(res, dst);
    dst += dst_stride;
  }
}

static void intra_predict_tm_32x32_msa(const uint8_t *src_top,
                                       const uint8_t *src_left,
                                       int32_t src_left_stride,
                                       uint8_t *dst,
                                       int32_t dst_stride) {
  uint8_t top_left = src_top[-1];
  uint32_t loop_cnt;
  VUINT8 src_top0, src_top1;
  VUINT8 src_left0, src_left1, src_left2, src_left3;
  VUINT16 src_top_left;
  VUINT16 res_r0, res_r1, res_l0, res_l1;
  VUINT8 res0, res1;

  src_top0 = LOAD_B(src_top);
  src_top1 = LOAD_B(src_top + 16);
  src_top_left = __fill_h(top_left);

  for (loop_cnt = 8; loop_cnt--;) {
    src_left0 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left1 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left2 = __fill_b(src_left[0]);
    src_left += src_left_stride;
    src_left3 = __fill_b(src_left[0]);
    src_left += src_left_stride;

    res_r0 = __ilvr_b(src_left0, src_top0);
    res_l0 = __ilvl_b(src_left0, src_top0);
    res_r1 = __ilvr_b(src_left0, src_top1);
    res_l1 = __ilvl_b(src_left0, src_top1);

    res_r0 = __hadd_u_h(res_r0, res_r0);
    res_l0 = __hadd_u_h(res_l0, res_l0);
    res_r1 = __hadd_u_h(res_r1, res_r1);
    res_l1 = __hadd_u_h(res_l1, res_l1);

    res_r0 = __subs_u_h(res_r0, src_top_left);
    res_l0 = __subs_u_h(res_l0, src_top_left);
    res_r1 = __subs_u_h(res_r1, src_top_left);
    res_l1 = __subs_u_h(res_l1, src_top_left);

    res_r0 = __sat_u_h(res_r0, 7);
    res_l0 = __sat_u_h(res_l0, 7);
    res_r1 = __sat_u_h(res_r1, 7);
    res_l1 = __sat_u_h(res_l1, 7);

    res0 = __pckev_b(res_l0, res_r0);
    res1 = __pckev_b(res_l1, res_r1);

    STORE_B(res0, dst);
    STORE_B(res1, dst + 16);
    dst += dst_stride;

    res_r0 = __ilvr_b(src_left1, src_top0);
    res_l0 = __ilvl_b(src_left1, src_top0);
    res_r1 = __ilvr_b(src_left1, src_top1);
    res_l1 = __ilvl_b(src_left1, src_top1);

    res_r0 = __hadd_u_h(res_r0, res_r0);
    res_l0 = __hadd_u_h(res_l0, res_l0);
    res_r1 = __hadd_u_h(res_r1, res_r1);
    res_l1 = __hadd_u_h(res_l1, res_l1);

    res_r0 = __subs_u_h(res_r0, src_top_left);
    res_l0 = __subs_u_h(res_l0, src_top_left);
    res_r1 = __subs_u_h(res_r1, src_top_left);
    res_l1 = __subs_u_h(res_l1, src_top_left);

    res_r0 = __sat_u_h(res_r0, 7);
    res_l0 = __sat_u_h(res_l0, 7);
    res_r1 = __sat_u_h(res_r1, 7);
    res_l1 = __sat_u_h(res_l1, 7);

    res0 = __pckev_b(res_l0, res_r0);
    res1 = __pckev_b(res_l1, res_r1);

    STORE_B(res0, dst);
    STORE_B(res1, dst + 16);
    dst += dst_stride;

    res_r0 = __ilvr_b(src_left2, src_top0);
    res_l0 = __ilvl_b(src_left2, src_top0);
    res_r1 = __ilvr_b(src_left2, src_top1);
    res_l1 = __ilvl_b(src_left2, src_top1);

    res_r0 = __hadd_u_h(res_r0, res_r0);
    res_l0 = __hadd_u_h(res_l0, res_l0);
    res_r1 = __hadd_u_h(res_r1, res_r1);
    res_l1 = __hadd_u_h(res_l1, res_l1);

    res_r0 = __subs_u_h(res_r0, src_top_left);
    res_l0 = __subs_u_h(res_l0, src_top_left);
    res_r1 = __subs_u_h(res_r1, src_top_left);
    res_l1 = __subs_u_h(res_l1, src_top_left);

    res_r0 = __sat_u_h(res_r0, 7);
    res_l0 = __sat_u_h(res_l0, 7);
    res_r1 = __sat_u_h(res_r1, 7);
    res_l1 = __sat_u_h(res_l1, 7);

    res0 = __pckev_b(res_l0, res_r0);
    res1 = __pckev_b(res_l1, res_r1);

    STORE_B(res0, dst);
    STORE_B(res1, dst + 16);
    dst += dst_stride;

    res_r0 = __ilvr_b(src_left3, src_top0);
    res_l0 = __ilvl_b(src_left3, src_top0);
    res_r1 = __ilvr_b(src_left3, src_top1);
    res_l1 = __ilvl_b(src_left3, src_top1);

    res_r0 = __hadd_u_h(res_r0, res_r0);
    res_l0 = __hadd_u_h(res_l0, res_l0);
    res_r1 = __hadd_u_h(res_r1, res_r1);
    res_l1 = __hadd_u_h(res_l1, res_l1);

    res_r0 = __subs_u_h(res_r0, src_top_left);
    res_l0 = __subs_u_h(res_l0, src_top_left);
    res_r1 = __subs_u_h(res_r1, src_top_left);
    res_l1 = __subs_u_h(res_l1, src_top_left);

    res_r0 = __sat_u_h(res_r0, 7);
    res_l0 = __sat_u_h(res_l0, 7);
    res_r1 = __sat_u_h(res_r1, 7);
    res_l1 = __sat_u_h(res_l1, 7);

    res0 = __pckev_b(res_l0, res_r0);
    res1 = __pckev_b(res_l1, res_r1);

    STORE_B(res0, dst);
    STORE_B(res1, dst + 16);
    dst += dst_stride;
  }
}

void vp9_v_predictor_4x4_msa(uint8_t *dst,
                             ptrdiff_t y_stride,
                             const uint8_t *above,
                             const uint8_t *left) {
  (void)left;

  intra_predict_vert_4x4_msa(above, dst, y_stride);
}

void vp9_v_predictor_8x8_msa(uint8_t *dst,
                             ptrdiff_t y_stride,
                             const uint8_t *above,
                             const uint8_t *left) {
  (void)left;

  intra_predict_vert_8x8_msa(above, dst, y_stride);
}

void vp9_v_predictor_16x16_msa(uint8_t *dst,
                               ptrdiff_t y_stride,
                               const uint8_t *above,
                               const uint8_t *left) {
  (void)left;

  intra_predict_vert_16x16_msa(above, dst, y_stride);
}

void vp9_v_predictor_32x32_msa(uint8_t *dst,
                               ptrdiff_t y_stride,
                               const uint8_t *above,
                               const uint8_t *left) {
  (void)left;

  intra_predict_vert_32x32_msa(above, dst, y_stride);
}

void vp9_h_predictor_4x4_msa(uint8_t *dst,
                             ptrdiff_t y_stride,
                             const uint8_t *above,
                             const uint8_t *left) {
  (void)above;

  intra_predict_horiz_4x4_msa(left, 1, dst, y_stride);
}

void vp9_h_predictor_8x8_msa(uint8_t *dst,
                             ptrdiff_t y_stride,
                             const uint8_t *above,
                             const uint8_t *left) {
  (void)above;

  intra_predict_horiz_8x8_msa(left, 1, dst, y_stride);
}

void vp9_h_predictor_16x16_msa(uint8_t *dst,
                               ptrdiff_t y_stride,
                               const uint8_t *above,
                               const uint8_t *left) {
  (void)above;

  intra_predict_horiz_16x16_msa(left, 1, dst, y_stride);
}

void vp9_h_predictor_32x32_msa(uint8_t *dst,
                               ptrdiff_t y_stride,
                               const uint8_t *above,
                               const uint8_t *left) {
  (void)above;

  intra_predict_horiz_32x32_msa(left, 1, dst, y_stride);
}

void vp9_dc_predictor_4x4_msa(uint8_t *dst,
                              ptrdiff_t y_stride,
                              const uint8_t *above,
                              const uint8_t *left) {
  intra_predict_dc_4x4_msa(above, left, 1, dst, y_stride, 1, 1);
}

void vp9_dc_top_predictor_4x4_msa(uint8_t *dst,
                                  ptrdiff_t y_stride,
                                  const uint8_t *above,
                                  const uint8_t *left) {
  intra_predict_dc_4x4_msa(above, left, 1, dst, y_stride, 1, 0);
}

void vp9_dc_left_predictor_4x4_msa(uint8_t *dst,
                                   ptrdiff_t y_stride,
                                   const uint8_t *above,
                                   const uint8_t *left) {
  intra_predict_dc_4x4_msa(above, left, 1, dst, y_stride, 0, 1);
}

void vp9_dc_128_predictor_4x4_msa(uint8_t *dst,
                                  ptrdiff_t y_stride,
                                  const uint8_t *above,
                                  const uint8_t *left) {
  intra_predict_dc_4x4_msa(above, left, 1, dst, y_stride, 0, 0);
}

void vp9_dc_predictor_8x8_msa(uint8_t *dst,
                              ptrdiff_t y_stride,
                              const uint8_t *above,
                              const uint8_t *left) {
  intra_predict_dc_8x8_msa(above, left, 1, dst, y_stride, 1, 1);
}

void vp9_dc_top_predictor_8x8_msa(uint8_t *dst,
                                  ptrdiff_t y_stride,
                                  const uint8_t *above,
                                  const uint8_t *left) {
  intra_predict_dc_8x8_msa(above, left, 1, dst, y_stride, 1, 0);
}

void vp9_dc_left_predictor_8x8_msa(uint8_t *dst,
                                   ptrdiff_t y_stride,
                                   const uint8_t *above,
                                   const uint8_t *left) {
  intra_predict_dc_8x8_msa(above, left, 1, dst, y_stride, 0, 1);
}

void vp9_dc_128_predictor_8x8_msa(uint8_t *dst,
                                  ptrdiff_t y_stride,
                                  const uint8_t *above,
                                  const uint8_t *left) {
  intra_predict_dc_8x8_msa(above, left, 1, dst, y_stride, 0, 0);
}

void vp9_dc_predictor_16x16_msa(uint8_t *dst,
                                ptrdiff_t y_stride,
                                const uint8_t *above,
                                const uint8_t *left) {
  intra_predict_dc_16x16_msa(above, left, 1, dst, y_stride, 1, 1);
}

void vp9_dc_top_predictor_16x16_msa(uint8_t *dst,
                                    ptrdiff_t y_stride,
                                    const uint8_t *above,
                                    const uint8_t *left) {
  intra_predict_dc_16x16_msa(above, left, 1, dst, y_stride, 1, 0);
}

void vp9_dc_left_predictor_16x16_msa(uint8_t *dst,
                                     ptrdiff_t y_stride,
                                     const uint8_t *above,
                                     const uint8_t *left) {
  intra_predict_dc_16x16_msa(above, left, 1, dst, y_stride, 0, 1);
}

void vp9_dc_128_predictor_16x16_msa(uint8_t *dst,
                                    ptrdiff_t y_stride,
                                    const uint8_t *above,
                                    const uint8_t *left) {
  intra_predict_dc_16x16_msa(above, left, 1, dst, y_stride, 0, 0);
}

void vp9_dc_predictor_32x32_msa(uint8_t *dst,
                                ptrdiff_t y_stride,
                                const uint8_t *above,
                                const uint8_t *left) {
  intra_predict_dc_32x32_msa(above, left, 1, dst, y_stride, 1, 1);
}

void vp9_dc_top_predictor_32x32_msa(uint8_t *dst,
                                    ptrdiff_t y_stride,
                                    const uint8_t *above,
                                    const uint8_t *left) {
  intra_predict_dc_32x32_msa(above, left, 1, dst, y_stride, 1, 0);
}

void vp9_dc_left_predictor_32x32_msa(uint8_t *dst,
                                     ptrdiff_t y_stride,
                                     const uint8_t *above,
                                     const uint8_t *left) {
  intra_predict_dc_32x32_msa(above, left, 1, dst, y_stride, 0, 1);
}

void vp9_dc_128_predictor_32x32_msa(uint8_t *dst,
                                    ptrdiff_t y_stride,
                                    const uint8_t *above,
                                    const uint8_t *left) {
  intra_predict_dc_32x32_msa(above, left, 1, dst, y_stride, 0, 0);
}

void vp9_tm_predictor_4x4_msa(uint8_t *dst,
                              ptrdiff_t y_stride,
                              const uint8_t *above,
                              const uint8_t *left) {
  intra_predict_tm_4x4_msa(above, left, 1, dst, y_stride);
}

void vp9_tm_predictor_8x8_msa(uint8_t *dst,
                              ptrdiff_t y_stride,
                              const uint8_t *above,
                              const uint8_t *left) {
  intra_predict_tm_8x8_msa(above, left, 1, dst, y_stride);
}

void vp9_tm_predictor_16x16_msa(uint8_t *dst,
                                ptrdiff_t y_stride,
                                const uint8_t *above,
                                const uint8_t *left) {
  intra_predict_tm_16x16_msa(above, left, 1, dst, y_stride);
}

void vp9_tm_predictor_32x32_msa(uint8_t *dst,
                                ptrdiff_t y_stride,
                                const uint8_t *above,
                                const uint8_t *left) {
  intra_predict_tm_32x32_msa(above, left, 1, dst, y_stride);
}
#endif  /* #if HAVE_MSA */
