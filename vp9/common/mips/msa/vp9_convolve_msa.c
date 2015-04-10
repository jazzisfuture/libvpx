/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "vp9/common/vp9_filter.h"
#include "vp9/common/mips/msa/vp9_macros_msa.h"
#include "vpx_mem/vpx_mem.h"

#if HAVE_MSA
uint8_t mc_filt_mask_arr[16 * 3] = {
  /* 8 width cases */
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,

  /* 4 width cases */
  0, 1, 1, 2, 2, 3, 3, 4, 16, 17, 17, 18, 18, 19, 19, 20,

  8, 9, 9, 10, 10, 11, 11, 12, 24, 25, 25, 26, 26, 27, 27, 28
};

/*******************************************************************************
                                Macros
*******************************************************************************/
#define HORIZ_8TAP_FILT(src, mask0, mask1, mask2, mask3,                   \
                        filt_h0, filt_h1, filt_h2, filt_h3) ({             \
  v8i16 vec0, vec1, vec2, vec3, horiz_out;                                 \
                                                                           \
  vec0 = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src), (v16i8)(src));  \
  vec0 = __msa_dotp_s_h((v16i8)vec0, (v16i8)(filt_h0));                    \
  vec1 = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src), (v16i8)(src));  \
  vec0 = __msa_dpadd_s_h(vec0, (v16i8)(filt_h1), (v16i8)vec1);             \
  vec2 = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src), (v16i8)(src));  \
  vec2 = __msa_dotp_s_h((v16i8)vec2, (v16i8)(filt_h2));                    \
  vec3 = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src), (v16i8)(src));  \
  vec2 = __msa_dpadd_s_h(vec2, (v16i8)(filt_h3), (v16i8)vec3);             \
  vec0 = __msa_adds_s_h(vec0, vec2);                                       \
  horiz_out = SRARI_SATURATE_SIGNED_H(vec0, FILTER_BITS, 7);               \
                                                                           \
  horiz_out;                                                               \
})

#define HORIZ_8TAP_FILT_2VECS(src0, src1, mask0, mask1, mask2, mask3,        \
                              filt_h0, filt_h1, filt_h2, filt_h3) ({         \
  v8i16 vec0, vec1, vec2, vec3, horiz_out;                                   \
                                                                             \
  vec0 = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src1), (v16i8)(src0));  \
  vec0 = __msa_dotp_s_h((v16i8)vec0, (v16i8)(filt_h0));                      \
  vec1 = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src1), (v16i8)(src0));  \
  vec0 = __msa_dpadd_s_h(vec0, (v16i8)(filt_h1), (v16i8)vec1);               \
  vec2 = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src1), (v16i8)(src0));  \
  vec2 = __msa_dotp_s_h((v16i8)vec2, (v16i8)(filt_h2));                      \
  vec3 = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src1), (v16i8)(src0));  \
  vec2 = __msa_dpadd_s_h(vec2, ((v16i8)filt_h3), (v16i8)vec3);               \
  vec0 = __msa_adds_s_h(vec0, vec2);                                         \
  horiz_out = (v8i16)SRARI_SATURATE_SIGNED_H(vec0, FILTER_BITS, 7);          \
                                                                             \
  horiz_out;                                                                 \
})

#define FILT_8TAP_DPADD_S_H(vec0, vec1, vec2, vec3,             \
                            filt0, filt1, filt2, filt3) ({      \
  v8i16 tmp0, tmp1;                                             \
                                                                \
  tmp0 = __msa_dotp_s_h((v16i8)(vec0), (v16i8)(filt0));         \
  tmp0 = __msa_dpadd_s_h(tmp0, (v16i8)(vec1), (v16i8)(filt1));  \
  tmp1 = __msa_dotp_s_h((v16i8)(vec2), (v16i8)(filt2));         \
  tmp1 = __msa_dpadd_s_h(tmp1, (v16i8)(vec3), ((v16i8)filt3));  \
  tmp0 = __msa_adds_s_h(tmp0, tmp1);                            \
                                                                \
  tmp0;                                                         \
})

#define FILT_4TAP_DPADD_S_H(vec0, vec1, filt0, filt1) ({        \
  v8i16 tmp0;                                                   \
                                                                \
  tmp0 = __msa_dotp_s_h((v16i8)(vec0), (v16i8)(filt0));         \
  tmp0 = __msa_dpadd_s_h(tmp0, (v16i8)(vec1), (v16i8)(filt1));  \
                                                                \
  tmp0;                                                         \
})

#define HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,                     \
                                   mask0, mask1, mask2, mask3,                 \
                                   filt0, filt1, filt2, filt3,                 \
                                   out0, out1) {                               \
  v8i16 vec0_m, vec1_m, vec2_m, vec3_m, vec4_m, vec5_m, vec6_m, vec7_m;        \
  v8i16 res0_m, res1_m, res2_m, res3_m;                                        \
                                                                               \
  vec0_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src1), (v16i8)(src0));  \
  vec1_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src3), (v16i8)(src2));  \
                                                                               \
  res0_m = __msa_dotp_s_h((v16i8)vec0_m, (v16i8)(filt0));                      \
  res1_m = __msa_dotp_s_h((v16i8)vec1_m, (v16i8)(filt0));                      \
                                                                               \
  vec2_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src1), (v16i8)(src0));  \
  vec3_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src3), (v16i8)(src2));  \
                                                                               \
  res0_m = __msa_dpadd_s_h(res0_m, (filt1), (v16i8)vec2_m);                    \
  res1_m = __msa_dpadd_s_h(res1_m, (filt1), (v16i8)vec3_m);                    \
                                                                               \
  vec4_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src1), (v16i8)(src0));  \
  vec5_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src3), (v16i8)(src2));  \
                                                                               \
  res2_m = __msa_dotp_s_h((v16i8)(filt2), (v16i8)vec4_m);                      \
  res3_m = __msa_dotp_s_h((v16i8)(filt2), (v16i8)vec5_m);                      \
                                                                               \
  vec6_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src1), (v16i8)(src0));  \
  vec7_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src3), (v16i8)(src2));  \
                                                                               \
  res2_m = __msa_dpadd_s_h(res2_m, (v16i8)(filt3), (v16i8)vec6_m);             \
  res3_m = __msa_dpadd_s_h(res3_m, (v16i8)(filt3), (v16i8)vec7_m);             \
                                                                               \
  out0 = __msa_adds_s_h(res0_m, res2_m);                                       \
  out1 = __msa_adds_s_h(res1_m, res3_m);                                       \
}

#define HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,                     \
                                   mask0, mask1, mask2, mask3,                 \
                                   filt0, filt1, filt2, filt3,                 \
                                   out0, out1, out2, out3) {                   \
  v8i16 vec0_m, vec1_m, vec2_m, vec3_m;                                        \
  v8i16 vec4_m, vec5_m, vec6_m, vec7_m;                                        \
  v8i16 res0_m, res1_m, res2_m, res3_m;                                        \
  v8i16 res4_m, res5_m, res6_m, res7_m;                                        \
                                                                               \
  vec0_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src0), (v16i8)(src0));  \
  vec1_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src1), (v16i8)(src1));  \
  vec2_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src2), (v16i8)(src2));  \
  vec3_m = (v8i16)__msa_vshf_b((v16i8)(mask0), (v16i8)(src3), (v16i8)(src3));  \
                                                                               \
  res0_m = __msa_dotp_s_h((v16i8)vec0_m, (v16i8)(filt0));                      \
  res1_m = __msa_dotp_s_h((v16i8)vec1_m, (v16i8)(filt0));                      \
  res2_m = __msa_dotp_s_h((v16i8)vec2_m, (v16i8)(filt0));                      \
  res3_m = __msa_dotp_s_h((v16i8)vec3_m, (v16i8)(filt0));                      \
                                                                               \
  vec0_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src0), (v16i8)(src0));  \
  vec1_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src1), (v16i8)(src1));  \
  vec2_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src2), (v16i8)(src2));  \
  vec3_m = (v8i16)__msa_vshf_b((v16i8)(mask2), (v16i8)(src3), (v16i8)(src3));  \
                                                                               \
  res4_m = __msa_dotp_s_h((v16i8)vec0_m, (v16i8)(filt2));                      \
  res5_m = __msa_dotp_s_h((v16i8)vec1_m, (v16i8)(filt2));                      \
  res6_m = __msa_dotp_s_h((v16i8)vec2_m, (v16i8)(filt2));                      \
  res7_m = __msa_dotp_s_h((v16i8)vec3_m, (v16i8)(filt2));                      \
                                                                               \
  vec4_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src0), (v16i8)(src0));  \
  vec5_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src1), (v16i8)(src1));  \
  vec6_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src2), (v16i8)(src2));  \
  vec7_m = (v8i16)__msa_vshf_b((v16i8)(mask1), (v16i8)(src3), (v16i8)(src3));  \
                                                                               \
  res0_m = __msa_dpadd_s_h(res0_m, (v16i8)(filt1), (v16i8)vec4_m);             \
  res1_m = __msa_dpadd_s_h(res1_m, (v16i8)(filt1), (v16i8)vec5_m);             \
  res2_m = __msa_dpadd_s_h(res2_m, (v16i8)(filt1), (v16i8)vec6_m);             \
  res3_m = __msa_dpadd_s_h(res3_m, (v16i8)(filt1), (v16i8)vec7_m);             \
                                                                               \
  vec4_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src0), (v16i8)(src0));  \
  vec5_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src1), (v16i8)(src1));  \
  vec6_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src2), (v16i8)(src2));  \
  vec7_m = (v8i16)__msa_vshf_b((v16i8)(mask3), (v16i8)(src3), (v16i8)(src3));  \
                                                                               \
  res4_m = __msa_dpadd_s_h(res4_m, (v16i8)(filt3), (v16i8)vec4_m);             \
  res5_m = __msa_dpadd_s_h(res5_m, (v16i8)(filt3), (v16i8)vec5_m);             \
  res6_m = __msa_dpadd_s_h(res6_m, (v16i8)(filt3), (v16i8)vec6_m);             \
  res7_m = __msa_dpadd_s_h(res7_m, (v16i8)(filt3), (v16i8)vec7_m);             \
                                                                               \
  out0 = __msa_adds_s_h(res0_m, res4_m);                                       \
  out1 = __msa_adds_s_h(res1_m, res5_m);                                       \
  out2 = __msa_adds_s_h(res2_m, res6_m);                                       \
  out3 = __msa_adds_s_h(res3_m, res7_m);                                       \
}

static void common_hz_8t_4x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h((v8i16)filt, 0);
  filt1 = (v16i8)__msa_splati_h((v8i16)filt, 1);
  filt2 = (v16i8)__msa_splati_h((v8i16)filt, 2);
  filt3 = (v16i8)__msa_splati_h((v8i16)filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             out0, out1);

  out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
  out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);

  PCKEV_2B_XORI128_STORE_4_BYTES_4(out0, out1, dst, dst_stride);
}

static void common_hz_8t_4x8_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             out0, out1);

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             out2, out3);

  out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
  out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
  out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
  out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

  PCKEV_2B_XORI128_STORE_4_BYTES_4(out0, out1, dst, dst_stride);
  dst += (4 * dst_stride);
  PCKEV_2B_XORI128_STORE_4_BYTES_4(out2, out3, dst, dst_stride);
}

static void common_hz_8t_4w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_hz_8t_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_hz_8t_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_hz_8t_8x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             out0, out1, out2, out3);

  out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
  out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
  out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
  out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

  PCKEV_B_4_XORI128_STORE_8_BYTES_4(out0, out1, out2, out3, dst, dst_stride);
}

static void common_hz_8t_8x8mult_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                               mask0, mask1, mask2, mask3,
                               filt0, filt1, filt2, filt3,
                               out0, out1, out2, out3);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_STORE_8_BYTES_4(out0, out1, out2, out3, dst, dst_stride);
    dst += (4 * dst_stride);
  }
}

static void common_hz_8t_8w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_hz_8t_8x4_msa(src, src_stride, dst, dst_stride, filter);
  } else {
    common_hz_8t_8x8mult_msa(src, src_stride, dst, dst_stride, filter, height);
  }
}

static void common_hz_8t_16w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = (height >> 1); loop_cnt--;) {
    src0 = LOAD_SB(src);
    src1 = LOAD_SB(src + 8);
    src += src_stride;
    src2 = LOAD_SB(src);
    src3 = LOAD_SB(src + 8);
    src += src_stride;

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                               mask0, mask1, mask2, mask3,
                               filt0, filt1, filt2, filt3,
                               out0, out1, out2, out3);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    PCKEV_B_XORI128_STORE_VEC(out1, out0, dst);
    dst += dst_stride;
    PCKEV_B_XORI128_STORE_VEC(out3, out2, dst);
    dst += dst_stride;
  }
}

static void common_hz_8t_32w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = (height >> 1); loop_cnt--;) {
    src0 = LOAD_SB(src);
    src2 = LOAD_SB(src + 16);
    src3 = LOAD_SB(src + 24);

    src1 = __msa_sld_b((v16i8)src2, (v16i8)src0, 8);

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                               mask0, mask1, mask2, mask3,
                               filt0, filt1, filt2, filt3,
                               out0, out1, out2, out3);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    src += src_stride;

    src0 = LOAD_SB(src);
    src2 = LOAD_SB(src + 16);
    src3 = LOAD_SB(src + 24);

    src1 = __msa_sld_b((v16i8)src2, (v16i8)src0, 8);

    PCKEV_B_XORI128_STORE_VEC(out1, out0, dst);
    PCKEV_B_XORI128_STORE_VEC(out3, out2, (dst + 16));
    dst += dst_stride;

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                               mask0, mask1, mask2, mask3,
                               filt0, filt1, filt2, filt3,
                               out0, out1, out2, out3);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    PCKEV_B_XORI128_STORE_VEC(out1, out0, dst);
    PCKEV_B_XORI128_STORE_VEC(out3, out2, (dst + 16));

    src += src_stride;
    dst += dst_stride;
  }
}

static void common_hz_8t_64w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt, cnt;
  v16i8 src0, src1, src2, src3;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = height; loop_cnt--;) {
    for (cnt = 0; cnt < 2; ++cnt) {
      src0 = LOAD_SB(&src[cnt << 5]);
      src2 = LOAD_SB(&src[16 + (cnt << 5)]);
      src3 = LOAD_SB(&src[24 + (cnt << 5)]);

      src1 = __msa_sld_b((v16i8)src2, (v16i8)src0, 8);

      XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

      HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                                 mask0, mask1, mask2, mask3,
                                 filt0, filt1, filt2, filt3,
                                 out0, out1, out2, out3);

      out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
      out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
      out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
      out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

      PCKEV_B_XORI128_STORE_VEC(out1, out0, &dst[cnt << 5]);
      PCKEV_B_XORI128_STORE_VEC(out3, out2, &dst[16 + (cnt << 5)]);
    }

    src += src_stride;
    dst += dst_stride;
  }
}

static void common_vt_8t_4w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v16i8 src2110, src4332, src6554, src8776, src10998;
  v8i16 filt, out10, out32;
  v16i8 filt0, filt1, filt2, filt3;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                  src1, src3, src5, src2, src4, src6,
                  src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

  ILVR_D_3VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r,
                  src6554, src65_r, src54_r);

  XORI_B_3VECS_SB(src2110, src4332, src6554, src2110, src4332, src6554, 128);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                    src76_r, src87_r, src98_r, src109_r);

    ILVR_D_2VECS_SB(src8776, src87_r, src76_r, src10998, src109_r, src98_r);

    XORI_B_2VECS_SB(src8776, src10998, src8776, src10998, 128);

    out10 = FILT_8TAP_DPADD_S_H(src2110, src4332, src6554, src8776,
                                filt0, filt1, filt2, filt3);
    out32 = FILT_8TAP_DPADD_S_H(src4332, src6554, src8776, src10998,
                                filt0, filt1, filt2, filt3);

    out10 = SRARI_SATURATE_SIGNED_H(out10, FILTER_BITS, 7);
    out32 = SRARI_SATURATE_SIGNED_H(out32, FILTER_BITS, 7);

    PCKEV_2B_XORI128_STORE_4_BYTES_4(out10, out32, dst, dst_stride);
    dst += (4 * dst_stride);

    src2110 = src6554;
    src4332 = src8776;
    src6554 = src10998;

    src6 = src10;
  }
}

static void common_vt_8t_8w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v16i8 filt0, filt1, filt2, filt3;
  v8i16 filt, out0_r, out1_r, out2_r, out3_r;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                  src1, src3, src5, src2, src4, src6,
                  src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                    src76_r, src87_r, src98_r, src109_r);

    out0_r = FILT_8TAP_DPADD_S_H(src10_r, src32_r, src54_r, src76_r,
                                 filt0, filt1, filt2, filt3);
    out1_r = FILT_8TAP_DPADD_S_H(src21_r, src43_r, src65_r, src87_r,
                                 filt0, filt1, filt2, filt3);
    out2_r = FILT_8TAP_DPADD_S_H(src32_r, src54_r, src76_r, src98_r,
                                 filt0, filt1, filt2, filt3);
    out3_r = FILT_8TAP_DPADD_S_H(src43_r, src65_r, src87_r, src109_r,
                                 filt0, filt1, filt2, filt3);

    out0_r = SRARI_SATURATE_SIGNED_H(out0_r, FILTER_BITS, 7);
    out1_r = SRARI_SATURATE_SIGNED_H(out1_r, FILTER_BITS, 7);
    out2_r = SRARI_SATURATE_SIGNED_H(out2_r, FILTER_BITS, 7);
    out3_r = SRARI_SATURATE_SIGNED_H(out3_r, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_STORE_8_BYTES_4(out0_r, out1_r, out2_r, out3_r,
                                      dst, dst_stride);
    dst += (4 * dst_stride);

    src10_r = src54_r;
    src32_r = src76_r;
    src54_r = src98_r;
    src21_r = src65_r;
    src43_r = src87_r;
    src65_r = src109_r;

    src6 = src10;
  }
}

static void common_vt_8t_16w_mult_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter, int32_t height,
                                      int32_t width) {
  const uint8_t *src_tmp;
  uint8_t *dst_tmp;
  uint32_t loop_cnt, cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v16i8 src10_l, src32_l, src54_l, src76_l, src98_l;
  v16i8 src21_l, src43_l, src65_l, src87_l, src109_l;
  v8i16 out0_r, out1_r, out2_r, out3_r, out0_l, out1_l, out2_l, out3_l;
  v8i16 filt;
  v16u8 tmp0, tmp1, tmp2, tmp3;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  for (cnt = (width >> 4); cnt--;) {
    src_tmp = src;
    dst_tmp = dst;

    LOAD_7VECS_SB(src_tmp, src_stride,
                  src0, src1, src2, src3, src4, src5, src6);
    src_tmp += (7 * src_stride);

    XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                    src0, src1, src2, src3, src4, src5, src6, 128);

    ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                    src1, src3, src5, src2, src4, src6,
                    src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

    ILVL_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                    src1, src3, src5, src2, src4, src6,
                    src10_l, src32_l, src54_l, src21_l, src43_l, src65_l);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
      LOAD_4VECS_SB(src_tmp, src_stride, src7, src8, src9, src10);
      src_tmp += (4 * src_stride);

      XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

      ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                      src76_r, src87_r, src98_r, src109_r);

      ILVL_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                      src76_l, src87_l, src98_l, src109_l);

      out0_r = FILT_8TAP_DPADD_S_H(src10_r, src32_r, src54_r, src76_r,
                                   filt0, filt1, filt2, filt3);
      out1_r = FILT_8TAP_DPADD_S_H(src21_r, src43_r, src65_r, src87_r,
                                   filt0, filt1, filt2, filt3);
      out2_r = FILT_8TAP_DPADD_S_H(src32_r, src54_r, src76_r, src98_r,
                                   filt0, filt1, filt2, filt3);
      out3_r = FILT_8TAP_DPADD_S_H(src43_r, src65_r, src87_r, src109_r,
                                   filt0, filt1, filt2, filt3);

      out0_l = FILT_8TAP_DPADD_S_H(src10_l, src32_l, src54_l, src76_l,
                                   filt0, filt1, filt2, filt3);
      out1_l = FILT_8TAP_DPADD_S_H(src21_l, src43_l, src65_l, src87_l,
                                   filt0, filt1, filt2, filt3);
      out2_l = FILT_8TAP_DPADD_S_H(src32_l, src54_l, src76_l, src98_l,
                                   filt0, filt1, filt2, filt3);
      out3_l = FILT_8TAP_DPADD_S_H(src43_l, src65_l, src87_l, src109_l,
                                   filt0, filt1, filt2, filt3);

      out0_r = SRARI_SATURATE_SIGNED_H(out0_r, FILTER_BITS, 7);
      out1_r = SRARI_SATURATE_SIGNED_H(out1_r, FILTER_BITS, 7);
      out2_r = SRARI_SATURATE_SIGNED_H(out2_r, FILTER_BITS, 7);
      out3_r = SRARI_SATURATE_SIGNED_H(out3_r, FILTER_BITS, 7);
      out0_l = SRARI_SATURATE_SIGNED_H(out0_l, FILTER_BITS, 7);
      out1_l = SRARI_SATURATE_SIGNED_H(out1_l, FILTER_BITS, 7);
      out2_l = SRARI_SATURATE_SIGNED_H(out2_l, FILTER_BITS, 7);
      out3_l = SRARI_SATURATE_SIGNED_H(out3_l, FILTER_BITS, 7);

      out0_r = (v8i16)__msa_pckev_b((v16i8)out0_l, (v16i8)out0_r);
      out1_r = (v8i16)__msa_pckev_b((v16i8)out1_l, (v16i8)out1_r);
      out2_r = (v8i16)__msa_pckev_b((v16i8)out2_l, (v16i8)out2_r);
      out3_r = (v8i16)__msa_pckev_b((v16i8)out3_l, (v16i8)out3_r);

      XORI_B_4VECS_UB(out0_r, out1_r, out2_r, out3_r,
                      tmp0, tmp1, tmp2, tmp3, 128);

      STORE_4VECS_UB(dst_tmp, dst_stride, tmp0, tmp1, tmp2, tmp3);
      dst_tmp += (4 * dst_stride);

      src10_r = src54_r;
      src32_r = src76_r;
      src54_r = src98_r;
      src21_r = src65_r;
      src43_r = src87_r;
      src65_r = src109_r;

      src10_l = src54_l;
      src32_l = src76_l;
      src54_l = src98_l;
      src21_l = src65_l;
      src43_l = src87_l;
      src65_l = src109_l;

      src6 = src10;
    }

    src += 16;
    dst += 16;
  }
}

static void common_vt_8t_16w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  common_vt_8t_16w_mult_msa(src, src_stride, dst, dst_stride,
                            filter, height, 16);
}

static void common_vt_8t_32w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  common_vt_8t_16w_mult_msa(src, src_stride, dst, dst_stride,
                            filter, height, 32);
}

static void common_vt_8t_64w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  common_vt_8t_16w_mult_msa(src, src_stride, dst, dst_stride,
                            filter, height, 64);
}

static void common_hv_8ht_8vt_4w_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter_horiz, int8_t *filter_vert,
                                     int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 filt_horiz0, filt_horiz1, filt_horiz2, filt_horiz3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt_horiz;
  v8i16 horiz_out0, horiz_out1, horiz_out2, horiz_out3, horiz_out4;
  v8i16 horiz_out5, horiz_out6, horiz_out7, horiz_out8, horiz_out9;
  v8i16 tmp0, tmp1, out0, out1, out2, out3, out4;
  v8i16 filt, filt_vert0, filt_vert1, filt_vert2, filt_vert3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= (3 + 3 * src_stride);

  /* rearranging filter */
  filt_horiz = LOAD_SH(filter_horiz);
  filt_horiz0 = (v16i8)__msa_splati_h(filt_horiz, 0);
  filt_horiz1 = (v16i8)__msa_splati_h(filt_horiz, 1);
  filt_horiz2 = (v16i8)__msa_splati_h(filt_horiz, 2);
  filt_horiz3 = (v16i8)__msa_splati_h(filt_horiz, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  horiz_out0 = HORIZ_8TAP_FILT_2VECS(src0, src1, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out2 = HORIZ_8TAP_FILT_2VECS(src2, src3, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out4 = HORIZ_8TAP_FILT_2VECS(src4, src5, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out5 = HORIZ_8TAP_FILT_2VECS(src5, src6, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out1 = (v8i16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8i16)__msa_sldi_b((v16i8)horiz_out4, (v16i8)horiz_out2, 8);

  filt = LOAD_SH(filter_vert);
  filt_vert0 = (v8i16)__msa_splati_h(filt, 0);
  filt_vert1 = (v8i16)__msa_splati_h(filt, 1);
  filt_vert2 = (v8i16)__msa_splati_h(filt, 2);
  filt_vert3 = (v8i16)__msa_splati_h(filt, 3);

  out0 = (v8i16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  out1 = (v8i16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  out2 = (v8i16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    horiz_out7 = HORIZ_8TAP_FILT_2VECS(src7, src8,
                                       mask0, mask1, mask2, mask3,
                                       filt_horiz0, filt_horiz1,
                                       filt_horiz2, filt_horiz3);

    horiz_out6 = (v8i16)__msa_sldi_b((v16i8)horiz_out7, (v16i8)horiz_out5, 8);

    out3 = (v8i16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);

    tmp0 = FILT_8TAP_DPADD_S_H(out0, out1, out2, out3,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    horiz_out9 = HORIZ_8TAP_FILT_2VECS(src9, src10,
                                       mask0, mask1, mask2, mask3,
                                       filt_horiz0, filt_horiz1,
                                       filt_horiz2, filt_horiz3);

    horiz_out8 = (v8i16)__msa_sldi_b((v16i8)horiz_out9, (v16i8)horiz_out7, 8);

    out4 = (v8i16)__msa_ilvev_b((v16i8)horiz_out9, (v16i8)horiz_out8);

    tmp1 = FILT_8TAP_DPADD_S_H(out1, out2, out3, out4,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp0 = SRARI_SATURATE_SIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_SIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_2B_XORI128_STORE_4_BYTES_4(tmp0, tmp1, dst, dst_stride);
    dst += (4 * dst_stride);

    horiz_out5 = horiz_out9;

    out0 = out2;
    out1 = out3;
    out2 = out4;
  }
}

static void common_hv_8ht_8vt_8w_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter_horiz, int8_t *filter_vert,
                                     int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 filt_horiz0, filt_horiz1, filt_horiz2, filt_horiz3;
  v8i16 filt_horiz, filt, filt_vert0, filt_vert1, filt_vert2, filt_vert3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8i16 horiz_out4, horiz_out5, horiz_out6, horiz_out7;
  v8i16 horiz_out8, horiz_out9, horiz_out10;
  v8i16 out0, out1, out2, out3, out4, out5, out6, out7, out8, out9;
  v8i16 tmp0, tmp1, tmp2, tmp3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= (3 + 3 * src_stride);

  /* rearranging filter */
  filt_horiz = LOAD_SH(filter_horiz);
  filt_horiz0 = (v16i8)__msa_splati_h(filt_horiz, 0);
  filt_horiz1 = (v16i8)__msa_splati_h(filt_horiz, 1);
  filt_horiz2 = (v16i8)__msa_splati_h(filt_horiz, 2);
  filt_horiz3 = (v16i8)__msa_splati_h(filt_horiz, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  horiz_out0 = HORIZ_8TAP_FILT(src0, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out1 = HORIZ_8TAP_FILT(src1, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out2 = HORIZ_8TAP_FILT(src2, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out3 = HORIZ_8TAP_FILT(src3, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out4 = HORIZ_8TAP_FILT(src4, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out5 = HORIZ_8TAP_FILT(src5, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out6 = HORIZ_8TAP_FILT(src6, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  filt = LOAD_SH(filter_vert);
  filt_vert0 = (v8i16)__msa_splati_h(filt, 0);
  filt_vert1 = (v8i16)__msa_splati_h(filt, 1);
  filt_vert2 = (v8i16)__msa_splati_h(filt, 2);
  filt_vert3 = (v8i16)__msa_splati_h(filt, 3);

  out0 = (v8i16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  out1 = (v8i16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  out2 = (v8i16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);

  out4 = (v8i16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out1);
  out5 = (v8i16)__msa_ilvev_b((v16i8)horiz_out4, (v16i8)horiz_out3);
  out6 = (v8i16)__msa_ilvev_b((v16i8)horiz_out6, (v16i8)horiz_out5);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    horiz_out7 = HORIZ_8TAP_FILT(src7, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out3 = (v8i16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);
    tmp0 = FILT_8TAP_DPADD_S_H(out0, out1, out2, out3,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp0 = SRARI_SATURATE_SIGNED_H(tmp0, FILTER_BITS, 7);

    horiz_out8 = HORIZ_8TAP_FILT(src8, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out7 = (v8i16)__msa_ilvev_b((v16i8)horiz_out8, (v16i8)horiz_out7);
    tmp1 = FILT_8TAP_DPADD_S_H(out4, out5, out6, out7,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp1 = SRARI_SATURATE_SIGNED_H(tmp1, FILTER_BITS, 7);

    horiz_out9 = HORIZ_8TAP_FILT(src9, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out8 = (v8i16)__msa_ilvev_b((v16i8)horiz_out9, (v16i8)horiz_out8);
    tmp2 = FILT_8TAP_DPADD_S_H(out1, out2, out3, out8,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp2 = SRARI_SATURATE_SIGNED_H(tmp2, FILTER_BITS, 7);

    horiz_out10 = HORIZ_8TAP_FILT(src10, mask0, mask1, mask2, mask3,
                                  filt_horiz0, filt_horiz1,
                                  filt_horiz2, filt_horiz3);

    out9 = (v8i16)__msa_ilvev_b((v16i8)horiz_out10, (v16i8)horiz_out9);
    tmp3 = FILT_8TAP_DPADD_S_H(out5, out6, out7, out9,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp3 = SRARI_SATURATE_SIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_STORE_8_BYTES_4(tmp0, tmp1, tmp2, tmp3, dst, dst_stride);
    dst += (4 * dst_stride);

    horiz_out6 = horiz_out10;

    out0 = out2;
    out1 = out3;
    out2 = out8;
    out4 = out6;
    out5 = out7;
    out6 = out9;
  }
}

static void common_hv_8ht_8vt_16w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 2; multiple8_cnt--;) {
    common_hv_8ht_8vt_8w_msa(src, src_stride, dst, dst_stride,
                             filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hv_8ht_8vt_32w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 4; multiple8_cnt--;) {
    common_hv_8ht_8vt_8w_msa(src, src_stride, dst, dst_stride,
                             filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hv_8ht_8vt_64w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 8; multiple8_cnt--;) {
    common_hv_8ht_8vt_8w_msa(src, src_stride, dst, dst_stride,
                             filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hz_8t_and_aver_dst_4x4_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, res0, res1;
  v16i8 res2, res3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             res0, res1);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  res0 = SRARI_SATURATE_SIGNED_H(res0, FILTER_BITS, 7);
  res1 = SRARI_SATURATE_SIGNED_H(res1, FILTER_BITS, 7);

  res2 = __msa_pckev_b((v16i8)res0, (v16i8)res0);
  res3 = __msa_pckev_b((v16i8)res1, (v16i8)res1);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);

  res2 = (v16i8)__msa_xori_b((v16u8)res2, 128);
  res3 = (v16i8)__msa_xori_b((v16u8)res3, 128);

  res2 = (v16i8)__msa_aver_u_b((v16u8)res2, (v16u8)dst0);
  res3 = (v16i8)__msa_aver_u_b((v16u8)res3, (v16u8)dst2);

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_8t_and_aver_dst_4x8_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v8i16 filt, vec0, vec1, vec2, vec3;
  v16i8 res0, res1, res2, res3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  LOAD_8VECS_UB(dst, dst_stride,
                dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             vec0, vec1);

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);

  XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

  HORIZ_8TAP_4WID_4VECS_FILT(src0, src1, src2, src3,
                             mask0, mask1, mask2, mask3,
                             filt0, filt1, filt2, filt3,
                             vec2, vec3);

  vec0 = SRARI_SATURATE_SIGNED_H(vec0, FILTER_BITS, 7);
  vec1 = SRARI_SATURATE_SIGNED_H(vec1, FILTER_BITS, 7);
  vec2 = SRARI_SATURATE_SIGNED_H(vec2, FILTER_BITS, 7);
  vec3 = SRARI_SATURATE_SIGNED_H(vec3, FILTER_BITS, 7);

  res0 = __msa_pckev_b((v16i8)vec0, (v16i8)vec0);
  res1 = __msa_pckev_b((v16i8)vec1, (v16i8)vec1);
  res2 = __msa_pckev_b((v16i8)vec2, (v16i8)vec2);
  res3 = __msa_pckev_b((v16i8)vec3, (v16i8)vec3);

  XORI_B_4VECS_SB(res0, res1, res2, res3, res0, res1, res2, res3, 128);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);
  dst4 = (v16u8)__msa_ilvr_w((v4i32)dst5, (v4i32)dst4);
  dst6 = (v16u8)__msa_ilvr_w((v4i32)dst7, (v4i32)dst6);

  res0 = (v16i8)__msa_aver_u_b((v16u8)res0, (v16u8)dst0);
  res1 = (v16i8)__msa_aver_u_b((v16u8)res1, (v16u8)dst2);
  res2 = (v16i8)__msa_aver_u_b((v16u8)res2, (v16u8)dst4);
  res3 = (v16i8)__msa_aver_u_b((v16u8)res3, (v16u8)dst6);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_8t_and_aver_dst_4w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  if (4 == height) {
    common_hz_8t_and_aver_dst_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_hz_8t_and_aver_dst_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_hz_8t_and_aver_dst_8w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  uint32_t loop_cnt;
  v16i8 filt0, filt1, filt2, filt3;
  v16i8 src0, src1, src2, src3;
  v16u8 mask0, mask1, mask2, mask3;
  v16u8 dst0, dst1, dst2, dst3;
  v8i16 filt, out0, out1, out2, out3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    HORIZ_8TAP_8WID_4VECS_FILT(src0, src1, src2, src3,
                               mask0, mask1, mask2, mask3,
                               filt0, filt1, filt2, filt3,
                               out0, out1, out2, out3);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_AVG_STORE_8_BYTES_4(out0, dst0, out1, dst1,
                                          out2, dst2, out3, dst3,
                                          dst, dst_stride);
    dst += (4 * dst_stride);
  }
}

static void common_hz_8t_and_aver_dst_16w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 dst0, dst1;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;
  v8i16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8i16 vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = height >> 1; loop_cnt--;) {
    src0 = LOAD_SB(src);
    src1 = LOAD_SB(src + 8);
    src += src_stride;
    src2 = LOAD_SB(src);
    src3 = LOAD_SB(src + 8);
    src += src_stride;

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    vec0 = (v8i16)__msa_vshf_b((v16i8)mask0, src0, src0);
    vec1 = (v8i16)__msa_vshf_b((v16i8)mask0, src1, src1);
    vec2 = (v8i16)__msa_vshf_b((v16i8)mask0, src2, src2);
    vec3 = (v8i16)__msa_vshf_b((v16i8)mask0, src3, src3);

    vec4 = (v8i16)__msa_vshf_b((v16i8)mask1, src0, src0);
    vec5 = (v8i16)__msa_vshf_b((v16i8)mask1, src1, src1);
    vec6 = (v8i16)__msa_vshf_b((v16i8)mask1, src2, src2);
    vec7 = (v8i16)__msa_vshf_b((v16i8)mask1, src3, src3);

    vec8 = (v8i16)__msa_vshf_b((v16i8)mask2, src0, src0);
    vec9 = (v8i16)__msa_vshf_b((v16i8)mask2, src1, src1);
    vec10 = (v8i16)__msa_vshf_b((v16i8)mask2, src2, src2);
    vec11 = (v8i16)__msa_vshf_b((v16i8)mask2, src3, src3);

    vec12 = (v8i16)__msa_vshf_b((v16i8)mask3, src0, src0);
    vec13 = (v8i16)__msa_vshf_b((v16i8)mask3, src1, src1);
    vec14 = (v8i16)__msa_vshf_b((v16i8)mask3, src2, src2);
    vec15 = (v8i16)__msa_vshf_b((v16i8)mask3, src3, src3);

    vec0 = __msa_dotp_s_h((v16i8)vec0, filt0);
    vec9 = __msa_dotp_s_h((v16i8)vec9, filt2);
    vec2 = __msa_dotp_s_h((v16i8)vec2, filt0);
    vec11 = __msa_dotp_s_h((v16i8)vec11, filt2);

    vec8 = __msa_dotp_s_h((v16i8)vec8, filt2);
    vec1 = __msa_dotp_s_h((v16i8)vec1, filt0);
    vec10 = __msa_dotp_s_h((v16i8)vec10, filt2);
    vec3 = __msa_dotp_s_h((v16i8)vec3, filt0);

    vec0 = __msa_dpadd_s_h(vec0, (v16i8)vec4, filt1);
    vec1 = __msa_dpadd_s_h(vec1, (v16i8)vec5, filt1);
    vec2 = __msa_dpadd_s_h(vec2, (v16i8)vec6, filt1);
    vec3 = __msa_dpadd_s_h(vec3, (v16i8)vec7, filt1);

    vec8 = __msa_dpadd_s_h(vec8, (v16i8)vec12, filt3);
    vec9 = __msa_dpadd_s_h(vec9, (v16i8)vec13, filt3);
    vec10 = __msa_dpadd_s_h(vec10, (v16i8)vec14, filt3);
    vec11 = __msa_dpadd_s_h(vec11, (v16i8)vec15, filt3);

    out0 = __msa_adds_s_h(vec0, vec8);
    out1 = __msa_adds_s_h(vec1, vec9);
    out2 = __msa_adds_s_h(vec2, vec10);
    out3 = __msa_adds_s_h(vec3, vec11);

    LOAD_2VECS_UB(dst, dst_stride, dst0, dst1);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    PCKEV_B_XORI128_AVG_STORE_VEC(out1, out0, dst0, dst);
    dst += dst_stride;
    PCKEV_B_XORI128_AVG_STORE_VEC(out3, out2, dst1, dst);
    dst += dst_stride;
  }
}

static void common_hz_8t_and_aver_dst_32w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3;
  v16u8 dst1, dst2;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;
  v8i16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8i16 vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = height; loop_cnt--;) {
    src0 = LOAD_SB(src);
    src2 = LOAD_SB(src + 16);
    src3 = LOAD_SB(src + 24);

    src1 = __msa_sld_b(src2, src0, 8);

    XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

    vec0 = (v8i16)__msa_vshf_b((v16i8)mask0, src0, src0);
    vec1 = (v8i16)__msa_vshf_b((v16i8)mask1, src0, src0);
    vec2 = (v8i16)__msa_vshf_b((v16i8)mask2, src0, src0);
    vec3 = (v8i16)__msa_vshf_b((v16i8)mask3, src0, src0);
    vec4 = (v8i16)__msa_vshf_b((v16i8)mask0, src1, src1);
    vec5 = (v8i16)__msa_vshf_b((v16i8)mask1, src1, src1);
    vec6 = (v8i16)__msa_vshf_b((v16i8)mask2, src1, src1);
    vec7 = (v8i16)__msa_vshf_b((v16i8)mask3, src1, src1);

    vec8 = (v8i16)__msa_vshf_b((v16i8)mask0, src2, src2);
    vec9 = (v8i16)__msa_vshf_b((v16i8)mask1, src2, src2);
    vec10 = (v8i16)__msa_vshf_b((v16i8)mask2, src2, src2);
    vec11 = (v8i16)__msa_vshf_b((v16i8)mask3, src2, src2);
    vec12 = (v8i16)__msa_vshf_b((v16i8)mask0, src3, src3);
    vec13 = (v8i16)__msa_vshf_b((v16i8)mask1, src3, src3);
    vec14 = (v8i16)__msa_vshf_b((v16i8)mask2, src3, src3);
    vec15 = (v8i16)__msa_vshf_b((v16i8)mask3, src3, src3);

    vec0 = __msa_dotp_s_h((v16i8)vec0, filt0);
    vec2 = __msa_dotp_s_h((v16i8)vec2, filt2);
    vec4 = __msa_dotp_s_h((v16i8)vec4, filt0);
    vec6 = __msa_dotp_s_h((v16i8)vec6, filt2);

    vec8 = __msa_dotp_s_h((v16i8)vec8, filt0);
    vec10 = __msa_dotp_s_h((v16i8)vec10, filt2);
    vec12 = __msa_dotp_s_h((v16i8)vec12, filt0);
    vec14 = __msa_dotp_s_h((v16i8)vec14, filt2);

    vec0 = __msa_dpadd_s_h(vec0, (v16i8)vec1, filt1);
    vec2 = __msa_dpadd_s_h(vec2, (v16i8)vec3, filt3);
    vec4 = __msa_dpadd_s_h(vec4, (v16i8)vec5, filt1);
    vec6 = __msa_dpadd_s_h(vec6, (v16i8)vec7, filt3);

    vec8 = __msa_dpadd_s_h(vec8, (v16i8)vec9, filt1);
    vec10 = __msa_dpadd_s_h(vec10, (v16i8)vec11, filt3);
    vec12 = __msa_dpadd_s_h(vec12, (v16i8)vec13, filt1);
    vec14 = __msa_dpadd_s_h(vec14, (v16i8)vec15, filt3);

    out0 = __msa_adds_s_h(vec0, vec2);
    out1 = __msa_adds_s_h(vec4, vec6);
    out2 = __msa_adds_s_h(vec8, vec10);
    out3 = __msa_adds_s_h(vec12, vec14);

    out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
    out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
    out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
    out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

    dst1 = LOAD_UB(dst);
    dst2 = LOAD_UB(dst + 16);

    PCKEV_B_XORI128_AVG_STORE_VEC(out1, out0, dst1, dst);
    PCKEV_B_XORI128_AVG_STORE_VEC(out3, out2, dst2, dst + 16);

    src += src_stride;
    dst += dst_stride;
  }
}

static void common_hz_8t_and_aver_dst_64w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt, mult16_cnt;
  v16i8 src0, src1, src2, src3;
  v16u8 dst1, dst2;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 filt, out0, out1, out2, out3;
  v8i16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8i16 vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= 3;

  /* rearranging filter */
  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  for (loop_cnt = height; loop_cnt--;) {
    for (mult16_cnt = 0; mult16_cnt < 2; ++mult16_cnt) {
      src0 = LOAD_SB(&src[mult16_cnt << 5]);
      src2 = LOAD_SB(&src[16 + (mult16_cnt << 5)]);
      src3 = LOAD_SB(&src[24 + (mult16_cnt << 5)]);

      src1 = __msa_sld_b(src2, src0, 8);

      XORI_B_4VECS_SB(src0, src1, src2, src3, src0, src1, src2, src3, 128);

      vec0 = (v8i16)__msa_vshf_b((v16i8)mask0, src0, src0);
      vec1 = (v8i16)__msa_vshf_b((v16i8)mask1, src0, src0);
      vec2 = (v8i16)__msa_vshf_b((v16i8)mask2, src0, src0);
      vec3 = (v8i16)__msa_vshf_b((v16i8)mask3, src0, src0);

      vec4 = (v8i16)__msa_vshf_b((v16i8)mask0, src1, src1);
      vec5 = (v8i16)__msa_vshf_b((v16i8)mask1, src1, src1);
      vec6 = (v8i16)__msa_vshf_b((v16i8)mask2, src1, src1);
      vec7 = (v8i16)__msa_vshf_b((v16i8)mask3, src1, src1);

      vec8 = (v8i16)__msa_vshf_b((v16i8)mask0, src2, src2);
      vec9 = (v8i16)__msa_vshf_b((v16i8)mask1, src2, src2);
      vec10 = (v8i16)__msa_vshf_b((v16i8)mask2, src2, src2);
      vec11 = (v8i16)__msa_vshf_b((v16i8)mask3, src2, src2);

      vec12 = (v8i16)__msa_vshf_b((v16i8)mask0, src3, src3);
      vec13 = (v8i16)__msa_vshf_b((v16i8)mask1, src3, src3);
      vec14 = (v8i16)__msa_vshf_b((v16i8)mask2, src3, src3);
      vec15 = (v8i16)__msa_vshf_b((v16i8)mask3, src3, src3);

      vec0 = __msa_dotp_s_h((v16i8)vec0, filt0);
      vec2 = __msa_dotp_s_h((v16i8)vec2, filt2);
      vec4 = __msa_dotp_s_h((v16i8)vec4, filt0);
      vec6 = __msa_dotp_s_h((v16i8)vec6, filt2);
      vec8 = __msa_dotp_s_h((v16i8)vec8, filt0);
      vec10 = __msa_dotp_s_h((v16i8)vec10, filt2);
      vec12 = __msa_dotp_s_h((v16i8)vec12, filt0);
      vec14 = __msa_dotp_s_h((v16i8)vec14, filt2);

      vec0 = __msa_dpadd_s_h(vec0, (v16i8)vec1, filt1);
      vec2 = __msa_dpadd_s_h(vec2, (v16i8)vec3, filt3);
      vec4 = __msa_dpadd_s_h(vec4, (v16i8)vec5, filt1);
      vec6 = __msa_dpadd_s_h(vec6, (v16i8)vec7, filt3);
      vec8 = __msa_dpadd_s_h(vec8, (v16i8)vec9, filt1);
      vec10 = __msa_dpadd_s_h(vec10, (v16i8)vec11, filt3);
      vec12 = __msa_dpadd_s_h(vec12, (v16i8)vec13, filt1);
      vec14 = __msa_dpadd_s_h(vec14, (v16i8)vec15, filt3);

      out0 = __msa_adds_s_h(vec0, vec2);
      out1 = __msa_adds_s_h(vec4, vec6);
      out2 = __msa_adds_s_h(vec8, vec10);
      out3 = __msa_adds_s_h(vec12, vec14);

      out0 = SRARI_SATURATE_SIGNED_H(out0, FILTER_BITS, 7);
      out1 = SRARI_SATURATE_SIGNED_H(out1, FILTER_BITS, 7);
      out2 = SRARI_SATURATE_SIGNED_H(out2, FILTER_BITS, 7);
      out3 = SRARI_SATURATE_SIGNED_H(out3, FILTER_BITS, 7);

      dst1 = LOAD_UB(&dst[mult16_cnt << 5]);
      dst2 = LOAD_UB(&dst[16 + (mult16_cnt << 5)]);

      PCKEV_B_XORI128_AVG_STORE_VEC(out1, out0, dst1,
                                    &dst[mult16_cnt << 5]);
      PCKEV_B_XORI128_AVG_STORE_VEC(out3, out2, dst2,
                                    &dst[16 + (mult16_cnt << 5)]);
    }

    src += src_stride;
    dst += dst_stride;
  }
}

static void common_vt_8t_and_aver_dst_4w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  uint32_t loop_cnt;
  uint32_t out0, out1, out2, out3;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16u8 dst0, dst1, dst2, dst3;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v16i8 src2110, src4332, src6554, src8776, src10998;
  v8i16 filt, out10, out32;
  v16i8 filt0, filt1, filt2, filt3;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                  src1, src3, src5, src2, src4, src6,
                  src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

  ILVR_D_3VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r,
                  src6554, src65_r, src54_r);

  XORI_B_3VECS_SB(src2110, src4332, src6554, src2110, src4332, src6554, 128);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                    src76_r, src87_r, src98_r, src109_r);

    ILVR_D_2VECS_SB(src8776, src87_r, src76_r, src10998, src109_r, src98_r);

    XORI_B_2VECS_SB(src8776, src10998, src8776, src10998, 128);

    out10 = FILT_8TAP_DPADD_S_H(src2110, src4332, src6554, src8776,
                                filt0, filt1, filt2, filt3);
    out32 = FILT_8TAP_DPADD_S_H(src4332, src6554, src8776, src10998,
                                filt0, filt1, filt2, filt3);

    out10 = SRARI_SATURATE_SIGNED_H(out10, FILTER_BITS, 7);
    out32 = SRARI_SATURATE_SIGNED_H(out32, FILTER_BITS, 7);

    out10 = (v8i16)__msa_pckev_b((v16i8)out32, (v16i8)out10);
    out10 = (v8i16)__msa_xori_b((v16u8)out10, 128);

    dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
    dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);
    dst0 = (v16u8)__msa_ilvr_d((v2i64)dst2, (v2i64)dst0);

    out10 = (v8i16)__msa_aver_u_b((v16u8)out10, (v16u8)dst0);

    out0 = __msa_copy_u_w((v4i32)out10, 0);
    out1 = __msa_copy_u_w((v4i32)out10, 1);
    out2 = __msa_copy_u_w((v4i32)out10, 2);
    out3 = __msa_copy_u_w((v4i32)out10, 3);

    STORE_WORD(dst, out0);
    dst += dst_stride;
    STORE_WORD(dst, out1);
    dst += dst_stride;
    STORE_WORD(dst, out2);
    dst += dst_stride;
    STORE_WORD(dst, out3);
    dst += dst_stride;

    src2110 = src6554;
    src4332 = src8776;
    src6554 = src10998;

    src6 = src10;
  }
}

static void common_vt_8t_and_aver_dst_8w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16u8 dst0, dst1, dst2, dst3;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v8i16 filt, out0_r, out1_r, out2_r, out3_r;
  v16i8 filt0, filt1, filt2, filt3;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                  src1, src3, src5, src2, src4, src6,
                  src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                    src76_r, src87_r, src98_r, src109_r);

    out0_r = FILT_8TAP_DPADD_S_H(src10_r, src32_r, src54_r, src76_r,
                                 filt0, filt1, filt2, filt3);
    out1_r = FILT_8TAP_DPADD_S_H(src21_r, src43_r, src65_r, src87_r,
                                 filt0, filt1, filt2, filt3);
    out2_r = FILT_8TAP_DPADD_S_H(src32_r, src54_r, src76_r, src98_r,
                                 filt0, filt1, filt2, filt3);
    out3_r = FILT_8TAP_DPADD_S_H(src43_r, src65_r, src87_r, src109_r,
                                 filt0, filt1, filt2, filt3);

    out0_r = SRARI_SATURATE_SIGNED_H(out0_r, FILTER_BITS, 7);
    out1_r = SRARI_SATURATE_SIGNED_H(out1_r, FILTER_BITS, 7);
    out2_r = SRARI_SATURATE_SIGNED_H(out2_r, FILTER_BITS, 7);
    out3_r = SRARI_SATURATE_SIGNED_H(out3_r, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_AVG_STORE_8_BYTES_4(out0_r, dst0, out1_r, dst1,
                                          out2_r, dst2, out3_r, dst3,
                                          dst, dst_stride);
    dst += (4 * dst_stride);

    src10_r = src54_r;
    src32_r = src76_r;
    src54_r = src98_r;
    src21_r = src65_r;
    src43_r = src87_r;
    src65_r = src109_r;

    src6 = src10;
  }
}

static void common_vt_8t_and_aver_dst_16w_mult_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter,
                                                   int32_t height,
                                                   int32_t width) {
  const uint8_t *src_tmp;
  uint8_t *dst_tmp;
  uint32_t loop_cnt, cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16u8 dst0, dst1, dst2, dst3;
  v16i8 src10_r, src32_r, src54_r, src76_r, src98_r;
  v16i8 src21_r, src43_r, src65_r, src87_r, src109_r;
  v16i8 src10_l, src32_l, src54_l, src76_l, src98_l;
  v16i8 src21_l, src43_l, src65_l, src87_l, src109_l;
  v8i16 out0_r, out1_r, out2_r, out3_r, out0_l, out1_l, out2_l, out3_l;
  v8i16 filt;
  v16i8 filt0, filt1, filt2, filt3;
  v16u8 tmp0, tmp1, tmp2, tmp3;

  src -= (3 * src_stride);

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);
  filt1 = (v16i8)__msa_splati_h(filt, 1);
  filt2 = (v16i8)__msa_splati_h(filt, 2);
  filt3 = (v16i8)__msa_splati_h(filt, 3);

  for (cnt = (width >> 4); cnt--;) {
    src_tmp = src;
    dst_tmp = dst;

    LOAD_7VECS_SB(src_tmp, src_stride,
                  src0, src1, src2, src3, src4, src5, src6);
    src_tmp += (7 * src_stride);

    XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                    src0, src1, src2, src3, src4, src5, src6, 128);

    ILVR_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                    src1, src3, src5, src2, src4, src6,
                    src10_r, src32_r, src54_r, src21_r, src43_r, src65_r);

    ILVL_B_6VECS_SB(src0, src2, src4, src1, src3, src5,
                    src1, src3, src5, src2, src4, src6,
                    src10_l, src32_l, src54_l, src21_l, src43_l, src65_l);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
      LOAD_4VECS_SB(src_tmp, src_stride, src7, src8, src9, src10);
      src_tmp += (4 * src_stride);

      LOAD_4VECS_UB(dst_tmp, dst_stride, dst0, dst1, dst2, dst3);

      XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

      ILVR_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                      src76_r, src87_r, src98_r, src109_r);

      ILVL_B_4VECS_SB(src6, src7, src8, src9, src7, src8, src9, src10,
                      src76_l, src87_l, src98_l, src109_l);

      out0_r = FILT_8TAP_DPADD_S_H(src10_r, src32_r, src54_r, src76_r,
                                   filt0, filt1, filt2, filt3);
      out1_r = FILT_8TAP_DPADD_S_H(src21_r, src43_r, src65_r, src87_r,
                                   filt0, filt1, filt2, filt3);
      out2_r = FILT_8TAP_DPADD_S_H(src32_r, src54_r, src76_r, src98_r,
                                   filt0, filt1, filt2, filt3);
      out3_r = FILT_8TAP_DPADD_S_H(src43_r, src65_r, src87_r, src109_r,
                                   filt0, filt1, filt2, filt3);

      out0_l = FILT_8TAP_DPADD_S_H(src10_l, src32_l, src54_l, src76_l,
                                   filt0, filt1, filt2, filt3);
      out1_l = FILT_8TAP_DPADD_S_H(src21_l, src43_l, src65_l, src87_l,
                                   filt0, filt1, filt2, filt3);
      out2_l = FILT_8TAP_DPADD_S_H(src32_l, src54_l, src76_l, src98_l,
                                   filt0, filt1, filt2, filt3);
      out3_l = FILT_8TAP_DPADD_S_H(src43_l, src65_l, src87_l, src109_l,
                                   filt0, filt1, filt2, filt3);

      out0_r = SRARI_SATURATE_SIGNED_H(out0_r, FILTER_BITS, 7);
      out1_r = SRARI_SATURATE_SIGNED_H(out1_r, FILTER_BITS, 7);
      out2_r = SRARI_SATURATE_SIGNED_H(out2_r, FILTER_BITS, 7);
      out3_r = SRARI_SATURATE_SIGNED_H(out3_r, FILTER_BITS, 7);

      out0_l = SRARI_SATURATE_SIGNED_H(out0_l, FILTER_BITS, 7);
      out1_l = SRARI_SATURATE_SIGNED_H(out1_l, FILTER_BITS, 7);
      out2_l = SRARI_SATURATE_SIGNED_H(out2_l, FILTER_BITS, 7);
      out3_l = SRARI_SATURATE_SIGNED_H(out3_l, FILTER_BITS, 7);

      out0_r = (v8i16)__msa_pckev_b((v16i8)out0_l, (v16i8)out0_r);
      out1_r = (v8i16)__msa_pckev_b((v16i8)out1_l, (v16i8)out1_r);
      out2_r = (v8i16)__msa_pckev_b((v16i8)out2_l, (v16i8)out2_r);
      out3_r = (v8i16)__msa_pckev_b((v16i8)out3_l, (v16i8)out3_r);

      XORI_B_4VECS_UB(out0_r, out1_r, out2_r, out3_r,
                      tmp0, tmp1, tmp2, tmp3, 128);

      dst0 = __msa_aver_u_b((v16u8)tmp0, (v16u8)dst0);
      dst1 = __msa_aver_u_b((v16u8)tmp1, (v16u8)dst1);
      dst2 = __msa_aver_u_b((v16u8)tmp2, (v16u8)dst2);
      dst3 = __msa_aver_u_b((v16u8)tmp3, (v16u8)dst3);

      STORE_4VECS_UB(dst_tmp, dst_stride, dst0, dst1, dst2, dst3);
      dst_tmp += (4 * dst_stride);

      src10_r = src54_r;
      src32_r = src76_r;
      src54_r = src98_r;
      src21_r = src65_r;
      src43_r = src87_r;
      src65_r = src109_r;

      src10_l = src54_l;
      src32_l = src76_l;
      src54_l = src98_l;
      src21_l = src65_l;
      src43_l = src87_l;
      src65_l = src109_l;

      src6 = src10;
    }

    src += 16;
    dst += 16;
  }
}

static void common_vt_8t_and_aver_dst_16w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  common_vt_8t_and_aver_dst_16w_mult_msa(src, src_stride, dst, dst_stride,
                                         filter, height, 16);
}

static void common_vt_8t_and_aver_dst_32w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  common_vt_8t_and_aver_dst_16w_mult_msa(src, src_stride, dst, dst_stride,
                                         filter, height, 32);
}

static void common_vt_8t_and_aver_dst_64w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  common_vt_8t_and_aver_dst_16w_mult_msa(src, src_stride, dst, dst_stride,
                                         filter, height, 64);
}

static void common_hv_8ht_8vt_and_aver_dst_4w_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter_horiz,
                                                  int8_t *filter_vert,
                                                  int32_t height) {
  uint32_t loop_cnt, out0, out1, out2, out3;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16u8 dst0, dst1, dst2, dst3;
  v16i8 filt_horiz0, filt_horiz1, filt_horiz2, filt_horiz3;
  v16u8 mask0, mask1, mask2, mask3;
  v16i8 tmp0, tmp1;
  v8i16 horiz_out0, horiz_out1, horiz_out2, horiz_out3, horiz_out4;
  v8i16 horiz_out5, horiz_out6, horiz_out7, horiz_out8, horiz_out9;
  v8i16 res0, res1, vec0, vec1, vec2, vec3, vec4;
  v8i16 filt_horiz, filt, filt_vert0, filt_vert1, filt_vert2, filt_vert3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[16]);

  src -= (3 + 3 * src_stride);

  /* rearranging filter */
  filt_horiz = LOAD_SH(filter_horiz);
  filt_horiz0 = (v16i8)__msa_splati_h(filt_horiz, 0);
  filt_horiz1 = (v16i8)__msa_splati_h(filt_horiz, 1);
  filt_horiz2 = (v16i8)__msa_splati_h(filt_horiz, 2);
  filt_horiz3 = (v16i8)__msa_splati_h(filt_horiz, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  horiz_out0 = HORIZ_8TAP_FILT_2VECS(src0, src1, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out2 = HORIZ_8TAP_FILT_2VECS(src2, src3, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out4 = HORIZ_8TAP_FILT_2VECS(src4, src5, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out5 = HORIZ_8TAP_FILT_2VECS(src5, src6, mask0, mask1, mask2, mask3,
                                     filt_horiz0, filt_horiz1,
                                     filt_horiz2, filt_horiz3);

  horiz_out1 = (v8i16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8i16)__msa_sldi_b((v16i8)horiz_out4, (v16i8)horiz_out2, 8);

  filt = LOAD_SH(filter_vert);
  filt_vert0 = __msa_splati_h(filt, 0);
  filt_vert1 = __msa_splati_h(filt, 1);
  filt_vert2 = __msa_splati_h(filt, 2);
  filt_vert3 = __msa_splati_h(filt, 3);

  vec0 = (v8i16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  vec1 = (v8i16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  vec2 = (v8i16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    horiz_out7 = HORIZ_8TAP_FILT_2VECS(src7, src8,
                                       mask0, mask1, mask2, mask3,
                                       filt_horiz0, filt_horiz1,
                                       filt_horiz2, filt_horiz3);

    horiz_out6 = (v8i16)__msa_sldi_b((v16i8)horiz_out7, (v16i8)horiz_out5, 8);

    vec3 = (v8i16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);

    res0 = FILT_8TAP_DPADD_S_H(vec0, vec1, vec2, vec3,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    horiz_out9 = HORIZ_8TAP_FILT_2VECS(src9, src10,
                                       mask0, mask1, mask2, mask3,
                                       filt_horiz0, filt_horiz1,
                                       filt_horiz2, filt_horiz3);

    horiz_out8 = (v8i16)__msa_sldi_b((v16i8)horiz_out9, (v16i8)horiz_out7, 8);

    vec4 = (v8i16)__msa_ilvev_b((v16i8)horiz_out9, (v16i8)horiz_out8);

    res1 = FILT_8TAP_DPADD_S_H(vec1, vec2, vec3, vec4,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
    dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);

    res0 = SRARI_SATURATE_SIGNED_H(res0, FILTER_BITS, 7);
    res1 = SRARI_SATURATE_SIGNED_H(res1, FILTER_BITS, 7);

    tmp0 = __msa_pckev_b((v16i8)res0, (v16i8)res0);
    tmp1 = __msa_pckev_b((v16i8)res1, (v16i8)res1);

    tmp0 = (v16i8)__msa_xori_b((v16u8)tmp0, 128);
    tmp1 = (v16i8)__msa_xori_b((v16u8)tmp1, 128);

    tmp0 = (v16i8)__msa_aver_u_b((v16u8)tmp0, (v16u8)dst0);
    tmp1 = (v16i8)__msa_aver_u_b((v16u8)tmp1, (v16u8)dst2);

    out0 = __msa_copy_u_w((v4i32)tmp0, 0);
    out1 = __msa_copy_u_w((v4i32)tmp0, 1);
    out2 = __msa_copy_u_w((v4i32)tmp1, 0);
    out3 = __msa_copy_u_w((v4i32)tmp1, 1);

    STORE_WORD(dst, out0);
    dst += dst_stride;
    STORE_WORD(dst, out1);
    dst += dst_stride;
    STORE_WORD(dst, out2);
    dst += dst_stride;
    STORE_WORD(dst, out3);
    dst += dst_stride;

    horiz_out5 = horiz_out9;

    vec0 = vec2;
    vec1 = vec3;
    vec2 = vec4;
  }
}

static void common_hv_8ht_8vt_and_aver_dst_8w_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter_horiz,
                                                  int8_t *filter_vert,
                                                  int32_t height) {
  uint32_t loop_cnt;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  v16i8 filt_horiz, filt_horiz0, filt_horiz1, filt_horiz2, filt_horiz3;
  v8i16 filt, filt_vert0, filt_vert1, filt_vert2, filt_vert3;
  v16u8 mask0, mask1, mask2, mask3;
  v8i16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8i16 horiz_out4, horiz_out5, horiz_out6, horiz_out7;
  v8i16 horiz_out8, horiz_out9, horiz_out10;
  v8i16 out0, out1, out2, out3, out4, out5, out6, out7, out8, out9;
  v8i16 tmp0, tmp1, tmp2, tmp3;
  v16u8 dst0, dst1, dst2, dst3;

  mask0 = LOAD_UB(&mc_filt_mask_arr[0]);

  src -= (3 + 3 * src_stride);

  /* rearranging filter */
  filt_horiz = LOAD_SB(filter_horiz);
  filt_horiz0 = (v16i8)__msa_splati_h((v8i16)filt_horiz, 0);
  filt_horiz1 = (v16i8)__msa_splati_h((v8i16)filt_horiz, 1);
  filt_horiz2 = (v16i8)__msa_splati_h((v8i16)filt_horiz, 2);
  filt_horiz3 = (v16i8)__msa_splati_h((v8i16)filt_horiz, 3);

  mask1 = mask0 + 2;
  mask2 = mask0 + 4;
  mask3 = mask0 + 6;

  LOAD_7VECS_SB(src, src_stride, src0, src1, src2, src3, src4, src5, src6);
  src += (7 * src_stride);

  XORI_B_7VECS_SB(src0, src1, src2, src3, src4, src5, src6,
                  src0, src1, src2, src3, src4, src5, src6, 128);

  horiz_out0 = HORIZ_8TAP_FILT(src0, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out1 = HORIZ_8TAP_FILT(src1, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out2 = HORIZ_8TAP_FILT(src2, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out3 = HORIZ_8TAP_FILT(src3, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out4 = HORIZ_8TAP_FILT(src4, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out5 = HORIZ_8TAP_FILT(src5, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  horiz_out6 = HORIZ_8TAP_FILT(src6, mask0, mask1, mask2, mask3,
                               filt_horiz0, filt_horiz1,
                               filt_horiz2, filt_horiz3);

  filt = LOAD_SH(filter_vert);
  filt_vert0 = __msa_splati_h(filt, 0);
  filt_vert1 = __msa_splati_h(filt, 1);
  filt_vert2 = __msa_splati_h(filt, 2);
  filt_vert3 = __msa_splati_h(filt, 3);

  out0 = (v8i16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  out1 = (v8i16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  out2 = (v8i16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);

  out4 = (v8i16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out1);
  out5 = (v8i16)__msa_ilvev_b((v16i8)horiz_out4, (v16i8)horiz_out3);
  out6 = (v8i16)__msa_ilvev_b((v16i8)horiz_out6, (v16i8)horiz_out5);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_SB(src, src_stride, src7, src8, src9, src10);
    src += (4 * src_stride);

    XORI_B_4VECS_SB(src7, src8, src9, src10, src7, src8, src9, src10, 128);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    horiz_out7 = HORIZ_8TAP_FILT(src7, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out3 = (v8i16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);
    tmp0 = FILT_8TAP_DPADD_S_H(out0, out1, out2, out3,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp0 = SRARI_SATURATE_SIGNED_H(tmp0, FILTER_BITS, 7);

    horiz_out8 = HORIZ_8TAP_FILT(src8, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out7 = (v8i16)__msa_ilvev_b((v16i8)horiz_out8, (v16i8)horiz_out7);
    tmp1 = FILT_8TAP_DPADD_S_H(out4, out5, out6, out7,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp1 = SRARI_SATURATE_SIGNED_H(tmp1, FILTER_BITS, 7);

    horiz_out9 = HORIZ_8TAP_FILT(src9, mask0, mask1, mask2, mask3,
                                 filt_horiz0, filt_horiz1,
                                 filt_horiz2, filt_horiz3);

    out8 = (v8i16)__msa_ilvev_b((v16i8)horiz_out9, (v16i8)horiz_out8);
    tmp2 = FILT_8TAP_DPADD_S_H(out1, out2, out3, out8,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp2 = SRARI_SATURATE_SIGNED_H(tmp2, FILTER_BITS, 7);

    horiz_out10 = HORIZ_8TAP_FILT(src10, mask0, mask1, mask2, mask3,
                                  filt_horiz0, filt_horiz1,
                                  filt_horiz2, filt_horiz3);

    out9 = (v8i16)__msa_ilvev_b((v16i8)horiz_out10, (v16i8)horiz_out9);
    tmp3 = FILT_8TAP_DPADD_S_H(out5, out6, out7, out9,
                               filt_vert0, filt_vert1,
                               filt_vert2, filt_vert3);

    tmp3 = SRARI_SATURATE_SIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_4_XORI128_AVG_STORE_8_BYTES_4(tmp0, dst0, tmp1, dst1,
                                          tmp2, dst2, tmp3, dst3,
                                          dst, dst_stride);
    dst += (4 * dst_stride);

    horiz_out6 = horiz_out10;

    out0 = out2;
    out1 = out3;
    out2 = out8;
    out4 = out6;
    out5 = out7;
    out6 = out9;
  }
}

static void common_hv_8ht_8vt_and_aver_dst_16w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 2; multiple8_cnt--;) {
    common_hv_8ht_8vt_and_aver_dst_8w_msa(src, src_stride, dst, dst_stride,
                                          filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hv_8ht_8vt_and_aver_dst_32w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 4; multiple8_cnt--;) {
    common_hv_8ht_8vt_and_aver_dst_8w_msa(src, src_stride, dst, dst_stride,
                                          filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hv_8ht_8vt_and_aver_dst_64w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 8; multiple8_cnt--;) {
    common_hv_8ht_8vt_and_aver_dst_8w_msa(src, src_stride, dst, dst_stride,
                                          filter_horiz, filter_vert, height);

    src += 8;
    dst += 8;
  }
}

static void common_hz_2t_4x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v8u16 vec0, vec1, vec2, vec3;
  v16u8 res0, res1;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);

  vec2 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec1, filt0);

  vec2 = (v8u16)__msa_srari_h((v8i16)vec2, FILTER_BITS);
  vec3 = (v8u16)__msa_srari_h((v8i16)vec3, FILTER_BITS);

  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  res0 = (v16u8)__msa_pckev_b((v16i8)vec2, (v16i8)vec2);
  res1 = (v16u8)__msa_pckev_b((v16i8)vec3, (v16i8)vec3);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_2t_4x8_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v16u8 res0, res1, res2, res3;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_8VECS_UB(src, src_stride,
                src0, src1, src2, src3, src4, src5, src6, src7);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src4);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src6);

  vec4 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec5 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec6 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec7 = __msa_dotp_u_h((v16u8)vec3, filt0);

  vec4 = (v8u16)__msa_srari_h((v8i16)vec4, FILTER_BITS);
  vec5 = (v8u16)__msa_srari_h((v8i16)vec5, FILTER_BITS);
  vec6 = (v8u16)__msa_srari_h((v8i16)vec6, FILTER_BITS);
  vec7 = (v8u16)__msa_srari_h((v8i16)vec7, FILTER_BITS);

  vec4 = __msa_min_u_h(vec4, const255);
  vec5 = __msa_min_u_h(vec5, const255);
  vec6 = __msa_min_u_h(vec6, const255);
  vec7 = __msa_min_u_h(vec7, const255);

  res0 = (v16u8)__msa_pckev_b((v16i8)vec4, (v16i8)vec4);
  res1 = (v16u8)__msa_pckev_b((v16i8)vec5, (v16i8)vec5);
  res2 = (v16u8)__msa_pckev_b((v16i8)vec6, (v16i8)vec6);
  res3 = (v16u8)__msa_pckev_b((v16i8)vec7, (v16i8)vec7);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_2t_4w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_hz_2t_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_hz_2t_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_hz_2t_8x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 out0, out1, out2, out3;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  out0 = __msa_min_u_h(vec0, const255);
  out1 = __msa_min_u_h(vec1, const255);
  out2 = __msa_min_u_h(vec2, const255);
  out3 = __msa_min_u_h(vec3, const255);

  PCKEV_B_STORE_8_BYTES_4(out0, out1, out2, out3, dst, dst_stride);
}

static void common_hz_2t_8x8mult_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter, int32_t height) {
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  vec0 = __msa_min_u_h(vec0, const255);
  vec1 = __msa_min_u_h(vec1, const255);
  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  PCKEV_B_STORE_8_BYTES_4(vec0, vec1, vec2, vec3, dst, dst_stride);
  dst += (4 * dst_stride);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  vec0 = __msa_min_u_h(vec0, const255);
  vec1 = __msa_min_u_h(vec1, const255);
  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  PCKEV_B_STORE_8_BYTES_4(vec0, vec1, vec2, vec3, dst, dst_stride);
  dst += (4 * dst_stride);

  if (16 == height) {
    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

    SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3,
                     vec0, vec1, vec2, vec3, FILTER_BITS);

    vec0 = __msa_min_u_h(vec0, const255);
    vec1 = __msa_min_u_h(vec1, const255);
    vec2 = __msa_min_u_h(vec2, const255);
    vec3 = __msa_min_u_h(vec3, const255);

    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    PCKEV_B_STORE_8_BYTES_4(vec0, vec1, vec2, vec3, dst, dst_stride);
    dst += (4 * dst_stride);

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

    SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3,
                     vec0, vec1, vec2, vec3, FILTER_BITS);

    vec0 = __msa_min_u_h(vec0, const255);
    vec1 = __msa_min_u_h(vec1, const255);
    vec2 = __msa_min_u_h(vec2, const255);
    vec3 = __msa_min_u_h(vec3, const255);

    PCKEV_B_STORE_8_BYTES_4(vec0, vec1, vec2, vec3, dst, dst_stride);
  }
}

static void common_hz_2t_8w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_hz_2t_8x4_msa(src, src_stride, dst, dst_stride, filter);
  } else {
    common_hz_2t_8x8mult_msa(src, src_stride, dst, dst_stride, filter, height);
  }
}

static void common_hz_2t_16w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 filt0, mask;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 out0, out1, out2, out3, out4, out5, out6, out7;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  loop_cnt = (height >> 2) - 1;

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  src0 = LOAD_UB(src);
  src1 = LOAD_UB(src + 8);
  src += src_stride;
  src2 = LOAD_UB(src);
  src3 = LOAD_UB(src + 8);
  src += src_stride;
  src4 = LOAD_UB(src);
  src5 = LOAD_UB(src + 8);
  src += src_stride;
  src6 = LOAD_UB(src);
  src7 = LOAD_UB(src + 8);
  src += src_stride;

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
  vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
  vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

  out0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  out1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  out2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  out3 = __msa_dotp_u_h((v16u8)vec3, filt0);
  out4 = __msa_dotp_u_h((v16u8)vec4, filt0);
  out5 = __msa_dotp_u_h((v16u8)vec5, filt0);
  out6 = __msa_dotp_u_h((v16u8)vec6, filt0);
  out7 = __msa_dotp_u_h((v16u8)vec7, filt0);

  out0 = (v8u16)__msa_srari_h((v8i16)out0, FILTER_BITS);
  out1 = (v8u16)__msa_srari_h((v8i16)out1, FILTER_BITS);
  out2 = (v8u16)__msa_srari_h((v8i16)out2, FILTER_BITS);
  out3 = (v8u16)__msa_srari_h((v8i16)out3, FILTER_BITS);
  out4 = (v8u16)__msa_srari_h((v8i16)out4, FILTER_BITS);
  out5 = (v8u16)__msa_srari_h((v8i16)out5, FILTER_BITS);
  out6 = (v8u16)__msa_srari_h((v8i16)out6, FILTER_BITS);
  out7 = (v8u16)__msa_srari_h((v8i16)out7, FILTER_BITS);

  out0 = __msa_min_u_h(out0, const255);
  out1 = __msa_min_u_h(out1, const255);
  out2 = __msa_min_u_h(out2, const255);
  out3 = __msa_min_u_h(out3, const255);
  out4 = __msa_min_u_h(out4, const255);
  out5 = __msa_min_u_h(out5, const255);
  out6 = __msa_min_u_h(out6, const255);
  out7 = __msa_min_u_h(out7, const255);

  PCKEV_B_STORE_VEC(out1, out0, dst);
  dst += dst_stride;
  PCKEV_B_STORE_VEC(out3, out2, dst);
  dst += dst_stride;
  PCKEV_B_STORE_VEC(out5, out4, dst);
  dst += dst_stride;
  PCKEV_B_STORE_VEC(out7, out6, dst);
  dst += dst_stride;

  for (; loop_cnt--;) {
    src0 = LOAD_UB(src);
    src1 = LOAD_UB(src + 8);
    src += src_stride;
    src2 = LOAD_UB(src);
    src3 = LOAD_UB(src + 8);
    src += src_stride;
    src4 = LOAD_UB(src);
    src5 = LOAD_UB(src + 8);
    src += src_stride;
    src6 = LOAD_UB(src);
    src7 = LOAD_UB(src + 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    out0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    out1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    out2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    out3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    out4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    out5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    out6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    out7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    out0 = (v8u16)__msa_srari_h((v8i16)out0, FILTER_BITS);
    out1 = (v8u16)__msa_srari_h((v8i16)out1, FILTER_BITS);
    out2 = (v8u16)__msa_srari_h((v8i16)out2, FILTER_BITS);
    out3 = (v8u16)__msa_srari_h((v8i16)out3, FILTER_BITS);
    out4 = (v8u16)__msa_srari_h((v8i16)out4, FILTER_BITS);
    out5 = (v8u16)__msa_srari_h((v8i16)out5, FILTER_BITS);
    out6 = (v8u16)__msa_srari_h((v8i16)out6, FILTER_BITS);
    out7 = (v8u16)__msa_srari_h((v8i16)out7, FILTER_BITS);

    out0 = __msa_min_u_h(out0, const255);
    out1 = __msa_min_u_h(out1, const255);
    out2 = __msa_min_u_h(out2, const255);
    out3 = __msa_min_u_h(out3, const255);
    out4 = __msa_min_u_h(out4, const255);
    out5 = __msa_min_u_h(out5, const255);
    out6 = __msa_min_u_h(out6, const255);
    out7 = __msa_min_u_h(out7, const255);

    PCKEV_B_STORE_VEC(out1, out0, dst);
    dst += dst_stride;
    PCKEV_B_STORE_VEC(out3, out2, dst);
    dst += dst_stride;
    PCKEV_B_STORE_VEC(out5, out4, dst);
    dst += dst_stride;
    PCKEV_B_STORE_VEC(out7, out6, dst);
    dst += dst_stride;
  }
}

static void common_hz_2t_32w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 filt0, mask;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 out0, out1, out2, out3, out4, out5, out6, out7;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  for (loop_cnt = height >> 1; loop_cnt--;) {
    src0 = LOAD_UB(src);
    src2 = LOAD_UB(src + 16);
    src3 = LOAD_UB(src + 24);
    src1 = (v16u8)__msa_sld_b((v16i8)src2, (v16i8)src0, 8);
    src += src_stride;
    src4 = LOAD_UB(src);
    src6 = LOAD_UB(src + 16);
    src7 = LOAD_UB(src + 24);
    src5 = (v16u8)__msa_sld_b((v16i8)src6, (v16i8)src4, 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    out0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    out1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    out2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    out3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    out4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    out5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    out6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    out7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    out0 = (v8u16)__msa_srari_h((v8i16)out0, FILTER_BITS);
    out1 = (v8u16)__msa_srari_h((v8i16)out1, FILTER_BITS);
    out2 = (v8u16)__msa_srari_h((v8i16)out2, FILTER_BITS);
    out3 = (v8u16)__msa_srari_h((v8i16)out3, FILTER_BITS);
    out4 = (v8u16)__msa_srari_h((v8i16)out4, FILTER_BITS);
    out5 = (v8u16)__msa_srari_h((v8i16)out5, FILTER_BITS);
    out6 = (v8u16)__msa_srari_h((v8i16)out6, FILTER_BITS);
    out7 = (v8u16)__msa_srari_h((v8i16)out7, FILTER_BITS);

    out0 = __msa_min_u_h(out0, const255);
    out1 = __msa_min_u_h(out1, const255);
    out2 = __msa_min_u_h(out2, const255);
    out3 = __msa_min_u_h(out3, const255);
    out4 = __msa_min_u_h(out4, const255);
    out5 = __msa_min_u_h(out5, const255);
    out6 = __msa_min_u_h(out6, const255);
    out7 = __msa_min_u_h(out7, const255);

    PCKEV_B_STORE_VEC(out1, out0, dst);
    PCKEV_B_STORE_VEC(out3, out2, dst + 16);
    dst += dst_stride;
    PCKEV_B_STORE_VEC(out5, out4, dst);
    PCKEV_B_STORE_VEC(out7, out6, dst + 16);
    dst += dst_stride;
  }
}

static void common_hz_2t_64w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 filt0, mask;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 out0, out1, out2, out3, out4, out5, out6, out7;
  v8u16 filt, const255;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  for (loop_cnt = height; loop_cnt--;) {
    src0 = LOAD_UB(src);
    src2 = LOAD_UB(src + 16);
    src4 = LOAD_UB(src + 32);
    src6 = LOAD_UB(src + 48);
    src7 = LOAD_UB(src + 56);
    src1 = (v16u8)__msa_sld_b((v16i8)src2, (v16i8)src0, 8);
    src3 = (v16u8)__msa_sld_b((v16i8)src4, (v16i8)src2, 8);
    src5 = (v16u8)__msa_sld_b((v16i8)src6, (v16i8)src4, 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    out0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    out1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    out2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    out3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    out4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    out5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    out6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    out7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    out0 = (v8u16)__msa_srari_h((v8i16)out0, FILTER_BITS);
    out1 = (v8u16)__msa_srari_h((v8i16)out1, FILTER_BITS);
    out2 = (v8u16)__msa_srari_h((v8i16)out2, FILTER_BITS);
    out3 = (v8u16)__msa_srari_h((v8i16)out3, FILTER_BITS);
    out4 = (v8u16)__msa_srari_h((v8i16)out4, FILTER_BITS);
    out5 = (v8u16)__msa_srari_h((v8i16)out5, FILTER_BITS);
    out6 = (v8u16)__msa_srari_h((v8i16)out6, FILTER_BITS);
    out7 = (v8u16)__msa_srari_h((v8i16)out7, FILTER_BITS);

    out0 = __msa_min_u_h(out0, const255);
    out1 = __msa_min_u_h(out1, const255);
    out2 = __msa_min_u_h(out2, const255);
    out3 = __msa_min_u_h(out3, const255);
    out4 = __msa_min_u_h(out4, const255);
    out5 = __msa_min_u_h(out5, const255);
    out6 = __msa_min_u_h(out6, const255);
    out7 = __msa_min_u_h(out7, const255);

    PCKEV_B_STORE_VEC(out1, out0, dst);
    PCKEV_B_STORE_VEC(out3, out2, dst + 16);
    PCKEV_B_STORE_VEC(out5, out4, dst + 32);
    PCKEV_B_STORE_VEC(out7, out6, dst + 48);
    dst += dst_stride;
  }
}

static void common_vt_2t_4x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16i8 src0, src1, src2, src3, src4;
  v16i8 src10_r, src32_r, src21_r, src43_r, src2110, src4332;
  v16i8 filt0;
  v8u16 filt;

  filt = LOAD_UH(filter);
  filt0 = (v16i8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_SB(src, src_stride, src0, src1, src2, src3, src4);
  src += (5 * src_stride);

  ILVR_B_4VECS_SB(src0, src1, src2, src3, src1, src2, src3, src4,
                  src10_r, src21_r, src32_r, src43_r);

  ILVR_D_2VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r);

  src2110 = (v16i8)__msa_dotp_u_h((v16u8)src2110, (v16u8)filt0);
  src4332 = (v16i8)__msa_dotp_u_h((v16u8)src4332, (v16u8)filt0);

  src2110 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src2110, FILTER_BITS, 7);
  src4332 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src4332, FILTER_BITS, 7);

  src2110 = (v16i8)__msa_pckev_b((v16i8)src4332, (v16i8)src2110);

  out0 = __msa_copy_u_w((v4i32)src2110, 0);
  out1 = __msa_copy_u_w((v4i32)src2110, 1);
  out2 = __msa_copy_u_w((v4i32)src2110, 2);
  out3 = __msa_copy_u_w((v4i32)src2110, 3);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_vt_2t_4x8_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  uint32_t out0, out1, out2, out3, out4, out5, out6, out7;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16i8 src10_r, src32_r, src54_r, src76_r, src21_r, src43_r;
  v16i8 src65_r, src87_r, src2110, src4332, src6554, src8776;
  v16i8 filt0;
  v8u16 filt;

  filt = LOAD_UH(filter);
  filt0 = (v16i8)__msa_splati_h((v8i16)filt, 0);

  LOAD_8VECS_SB(src, src_stride,
                src0, src1, src2, src3, src4, src5, src6, src7);
  src += (8 * src_stride);

  src8 = LOAD_SB(src);
  src += src_stride;

  ILVR_B_8VECS_SB(src0, src1, src2, src3, src4, src5, src6, src7,
                  src1, src2, src3, src4, src5, src6, src7, src8,
                  src10_r, src21_r, src32_r, src43_r,
                  src54_r, src65_r, src76_r, src87_r);

  ILVR_D_4VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r,
                  src6554, src65_r, src54_r, src8776, src87_r, src76_r);

  src2110 = (v16i8)__msa_dotp_u_h((v16u8)src2110, (v16u8)filt0);
  src4332 = (v16i8)__msa_dotp_u_h((v16u8)src4332, (v16u8)filt0);
  src6554 = (v16i8)__msa_dotp_u_h((v16u8)src6554, (v16u8)filt0);
  src8776 = (v16i8)__msa_dotp_u_h((v16u8)src8776, (v16u8)filt0);

  src2110 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src2110, FILTER_BITS, 7);
  src4332 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src4332, FILTER_BITS, 7);
  src6554 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src6554, FILTER_BITS, 7);
  src8776 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src8776, FILTER_BITS, 7);

  src2110 = (v16i8)__msa_pckev_b((v16i8)src4332, (v16i8)src2110);
  src4332 = (v16i8)__msa_pckev_b((v16i8)src8776, (v16i8)src6554);

  out0 = __msa_copy_u_w((v4i32)src2110, 0);
  out1 = __msa_copy_u_w((v4i32)src2110, 1);
  out2 = __msa_copy_u_w((v4i32)src2110, 2);
  out3 = __msa_copy_u_w((v4i32)src2110, 3);
  out4 = __msa_copy_u_w((v4i32)src4332, 0);
  out5 = __msa_copy_u_w((v4i32)src4332, 1);
  out6 = __msa_copy_u_w((v4i32)src4332, 2);
  out7 = __msa_copy_u_w((v4i32)src4332, 3);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;
  STORE_WORD(dst, out4);
  dst += dst_stride;
  STORE_WORD(dst, out5);
  dst += dst_stride;
  STORE_WORD(dst, out6);
  dst += dst_stride;
  STORE_WORD(dst, out7);
}

static void common_vt_2t_4w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_vt_2t_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_vt_2t_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_vt_2t_8x4_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter) {
  v16u8 src0, src1, src2, src3, src4;
  v16u8 vec0, vec1, vec2, vec3, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);

  ILVR_B_2VECS_UB(src0, src1, src1, src2, vec0, vec1);
  ILVR_B_2VECS_UB(src2, src3, src3, src4, vec2, vec3);

  /* filter calc */
  tmp0 = __msa_dotp_u_h(vec0, filt0);
  tmp1 = __msa_dotp_u_h(vec1, filt0);
  tmp2 = __msa_dotp_u_h(vec2, filt0);
  tmp3 = __msa_dotp_u_h(vec3, filt0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
  tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
  tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

  PCKEV_B_STORE_8_BYTES_4(tmp0, tmp1, tmp2, tmp3, dst, dst_stride);
}

static void common_vt_2t_8x8mult_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;

  for (loop_cnt = (height >> 3); loop_cnt--;) {
    LOAD_8VECS_UB(src, src_stride,
                  src1, src2, src3, src4, src5, src6, src7, src8);
    src += (8 * src_stride);

    ILVR_B_4VECS_UB(src0, src1, src2, src3, src1, src2, src3, src4,
                    vec0, vec1, vec2, vec3);

    ILVR_B_4VECS_UB(src4, src5, src6, src7, src5, src6, src7, src8,
                    vec4, vec5, vec6, vec7);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);
    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_8_BYTES_4(tmp0, tmp1, tmp2, tmp3, dst, dst_stride);
    dst += (4 * dst_stride);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);
    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_8_BYTES_4(tmp0, tmp1, tmp2, tmp3, dst, dst_stride);
    dst += (4 * dst_stride);

    src0 = src8;
  }
}

static void common_vt_2t_8w_msa(const uint8_t *src, int32_t src_stride,
                                uint8_t *dst, int32_t dst_stride,
                                int8_t *filter, int32_t height) {
  if (4 == height) {
    common_vt_2t_8x4_msa(src, src_stride, dst, dst_stride, filter);
  } else {
    common_vt_2t_8x8mult_msa(src, src_stride, dst, dst_stride, filter, height);
  }
}

static void common_vt_2t_16w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    src += (4 * src_stride);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst);
    dst += dst_stride;

    ILV_B_LRLR_UB(src2, src3, src3, src4, vec5, vec4, vec7, vec6);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst);
    dst += dst_stride;

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst);
    dst += dst_stride;

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst);
    dst += dst_stride;

    src0 = src4;
  }
}

static void common_vt_2t_32w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src5 = LOAD_UB(src + 16);
  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    LOAD_4VECS_UB(src + 16, src_stride, src6, src7, src8, src9);
    src += (4 * src_stride);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + dst_stride);

    ILV_B_LRLR_UB(src2, src3, src3, src4, vec5, vec4, vec7, vec6);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst + 2 * dst_stride);

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + 3 * dst_stride);

    ILV_B_LRLR_UB(src5, src6, src6, src7, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst + 16);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + 16 + dst_stride);

    ILV_B_LRLR_UB(src7, src8, src8, src9, vec5, vec4, vec7, vec6);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst + 16 + 2 * dst_stride);

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + 16 + 3 * dst_stride);
    dst += (4 * dst_stride);

    src0 = src4;
    src5 = src9;
  }
}

static void common_vt_2t_64w_msa(const uint8_t *src, int32_t src_stride,
                                 uint8_t *dst, int32_t dst_stride,
                                 int8_t *filter, int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 src8, src9, src10, src11;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_4VECS_UB(src, 16, src0, src3, src6, src9);
  src += src_stride;

  for (loop_cnt = (height >> 1); loop_cnt--;) {
    LOAD_2VECS_UB(src, src_stride, src1, src2);
    LOAD_2VECS_UB(src + 16, src_stride, src4, src5);
    LOAD_2VECS_UB(src + 32, src_stride, src7, src8);
    LOAD_2VECS_UB(src + 48, src_stride, src10, src11);
    src += (2 * src_stride);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + dst_stride);

    ILV_B_LRLR_UB(src3, src4, src4, src5, vec5, vec4, vec7, vec6);

    tmp4 = __msa_dotp_u_h(vec4, filt0);
    tmp5 = __msa_dotp_u_h(vec5, filt0);

    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);
    tmp5 = SRARI_SATURATE_UNSIGNED_H(tmp5, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp5, tmp4, dst + 16);

    tmp6 = __msa_dotp_u_h(vec6, filt0);
    tmp7 = __msa_dotp_u_h(vec7, filt0);

    tmp6 = SRARI_SATURATE_UNSIGNED_H(tmp6, FILTER_BITS, 7);
    tmp7 = SRARI_SATURATE_UNSIGNED_H(tmp7, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp7, tmp6, dst + 16 + dst_stride);

    ILV_B_LRLR_UB(src6, src7, src7, src8, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp1, tmp0, dst + 32);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp3, tmp2, dst + 32 + dst_stride);

    ILV_B_LRLR_UB(src9, src10, src10, src11, vec5, vec4, vec7, vec6);

    tmp4 = __msa_dotp_u_h(vec4, filt0);
    tmp5 = __msa_dotp_u_h(vec5, filt0);

    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);
    tmp5 = SRARI_SATURATE_UNSIGNED_H(tmp5, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp5, tmp4, dst + 48);

    tmp6 = __msa_dotp_u_h(vec6, filt0);
    tmp7 = __msa_dotp_u_h(vec7, filt0);

    tmp6 = SRARI_SATURATE_UNSIGNED_H(tmp6, FILTER_BITS, 7);
    tmp7 = SRARI_SATURATE_UNSIGNED_H(tmp7, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp7, tmp6, dst + 48 + dst_stride);
    dst += (2 * dst_stride);

    src0 = src2;
    src3 = src5;
    src6 = src8;
    src9 = src11;
  }
}

static void common_hv_2ht_2vt_4x4_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz,
                                      int8_t *filter_vert) {
  uint32_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 res0, res1, mask, horiz_vec;
  v16u8 filt_vert0, filt_horiz0;
  v16u8 vec0, vec1;
  v8u16 filt;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3, horiz_out4;
  v8u16 tmp0, tmp1;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  horiz_out2 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_out2, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  horiz_out4 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out4 = SRARI_SATURATE_UNSIGNED_H(horiz_out4, FILTER_BITS, 7);

  horiz_out1 = (v8u16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8u16)__msa_pckod_d((v2i64)horiz_out4, (v2i64)horiz_out2);

  vec0 = (v16u8)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  vec1 = (v16u8)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);

  tmp0 = __msa_dotp_u_h(vec0, filt_vert0);
  tmp1 = __msa_dotp_u_h(vec1, filt_vert0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

  res0 = (v16u8)__msa_pckev_b((v16i8)tmp0, (v16i8)tmp0);
  res1 = (v16u8)__msa_pckev_b((v16i8)tmp1, (v16i8)tmp1);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hv_2ht_2vt_4x8_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz,
                                      int8_t *filter_vert) {
  uint32_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16u8 filt_horiz0, filt_vert0, horiz_vec, mask;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 horiz_out4, horiz_out5, horiz_out6, horiz_out7, horiz_out8;
  v8u16 filt;
  v16u8 res0, res1, res2, res3;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_8VECS_UB(src, src_stride,
               src0, src1, src2, src3, src4, src5, src6, src7);
  src += (8 * src_stride);

  src8 = LOAD_UB(src);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  horiz_out2 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_out2, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src4);
  horiz_out4 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out4 = SRARI_SATURATE_UNSIGNED_H(horiz_out4, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src6);
  horiz_out6 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out6 = SRARI_SATURATE_UNSIGNED_H(horiz_out6, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src8, (v16i8)src8);
  horiz_out8 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out8 = SRARI_SATURATE_UNSIGNED_H(horiz_out8, FILTER_BITS, 7);

  horiz_out1 = (v8u16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8u16)__msa_sldi_b((v16i8)horiz_out4, (v16i8)horiz_out2, 8);
  horiz_out5 = (v8u16)__msa_sldi_b((v16i8)horiz_out6, (v16i8)horiz_out4, 8);

  horiz_out7 = (v8u16)__msa_pckod_d((v2i64)horiz_out8, (v2i64)horiz_out6);

  vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  vec1 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  vec2 = (v8u16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);
  vec3 = (v8u16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);

  vec4 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);
  vec5 = __msa_dotp_u_h((v16u8)vec1, (v16u8)filt_vert0);
  vec6 = __msa_dotp_u_h((v16u8)vec2, (v16u8)filt_vert0);
  vec7 = __msa_dotp_u_h((v16u8)vec3, (v16u8)filt_vert0);

  vec4 = SRARI_SATURATE_UNSIGNED_H(vec4, FILTER_BITS, 7);
  vec5 = SRARI_SATURATE_UNSIGNED_H(vec5, FILTER_BITS, 7);
  vec6 = SRARI_SATURATE_UNSIGNED_H(vec6, FILTER_BITS, 7);
  vec7 = SRARI_SATURATE_UNSIGNED_H(vec7, FILTER_BITS, 7);

  res0 = (v16u8)__msa_pckev_b((v16i8)vec4, (v16i8)vec4);
  res1 = (v16u8)__msa_pckev_b((v16i8)vec5, (v16i8)vec5);
  res2 = (v16u8)__msa_pckev_b((v16i8)vec6, (v16i8)vec6);
  res3 = (v16u8)__msa_pckev_b((v16i8)vec7, (v16i8)vec7);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hv_2ht_2vt_4w_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter_horiz,
                                     int8_t *filter_vert,
                                     int32_t height) {
  if (4 == height) {
    common_hv_2ht_2vt_4x4_msa(src, src_stride, dst, dst_stride,
                              filter_horiz, filter_vert);
  } else if (8 == height) {
    common_hv_2ht_2vt_4x8_msa(src, src_stride, dst, dst_stride,
                              filter_horiz, filter_vert);
  }
}

static void common_hv_2ht_2vt_8x4_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz,
                                      int8_t *filter_vert) {
  v16u8 src0, src1, src2, src3, src4;
  v16u8 filt_horiz0, filt_vert0, mask, horiz_vec;
  v16u8 vec0, vec1, vec2, vec3;
  v8u16 horiz_out0, horiz_out1;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);
  src += (5 * src_stride);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  horiz_out1 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

  vec0 = (v16u8)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  tmp0 = __msa_dotp_u_h(vec0, filt_vert0);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  vec1 = (v16u8)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
  tmp1 = __msa_dotp_u_h(vec1, filt_vert0);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
  horiz_out1 = __msa_dotp_u_h(horiz_vec, (v16u8)filt_horiz0);
  horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

  vec2 = (v16u8)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  tmp2 = __msa_dotp_u_h(vec2, filt_vert0);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  vec3 = (v16u8)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
  tmp3 = __msa_dotp_u_h(vec3, filt_vert0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
  tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
  tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

  PCKEV_B_STORE_8_BYTES_4(tmp0, tmp1, tmp2, tmp3, dst, dst_stride);
}

static void common_hv_2ht_2vt_8x8mult_msa(const uint8_t *src,
                                          int32_t src_stride,
                                          uint8_t *dst,
                                          int32_t dst_stride,
                                          int8_t *filter_horiz,
                                          int8_t *filter_vert,
                                          int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 filt_horiz0, mask, horiz_vec;
  v8u16 horiz_out0, horiz_out1;
  v8u16 tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, vec0;
  v8u16 filt, filt_vert0;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;
  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);

  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  for (loop_cnt = (height >> 3); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    src += (4 * src_stride);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    horiz_out1 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp2 = (v8u16)__msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    horiz_out1 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp3 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    src += (4 * src_stride);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp4 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);
    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);

    PCKEV_B_STORE_8_BYTES_4(tmp1, tmp2, tmp3, tmp4, dst, dst_stride);
    dst += (4 * dst_stride);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    horiz_out1 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp5 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp6 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    horiz_out1 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_out1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp7 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp8 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp5 = SRARI_SATURATE_UNSIGNED_H(tmp5, FILTER_BITS, 7);
    tmp6 = SRARI_SATURATE_UNSIGNED_H(tmp6, FILTER_BITS, 7);
    tmp7 = SRARI_SATURATE_UNSIGNED_H(tmp7, FILTER_BITS, 7);
    tmp8 = SRARI_SATURATE_UNSIGNED_H(tmp8, FILTER_BITS, 7);

    PCKEV_B_STORE_8_BYTES_4(tmp5, tmp6, tmp7, tmp8, dst, dst_stride);
    dst += (4 * dst_stride);
  }
}

static void common_hv_2ht_2vt_8w_msa(const uint8_t *src, int32_t src_stride,
                                     uint8_t *dst, int32_t dst_stride,
                                     int8_t *filter_horiz, int8_t *filter_vert,
                                     int32_t height) {
  if (4 == height) {
    common_hv_2ht_2vt_8x4_msa(src, src_stride, dst, dst_stride,
                              filter_horiz, filter_vert);
  } else {
    common_hv_2ht_2vt_8x8mult_msa(src, src_stride, dst, dst_stride,
                                  filter_horiz, filter_vert, height);
  }
}

static void common_hv_2ht_2vt_16w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 filt_horiz0, mask;
  v8u16 horiz_vec0, horiz_vec1;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8u16 tmp1, tmp2, filt, filt_vert0, vec0;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src1 = LOAD_UB(src + 8);
  horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

  horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);
  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src0, src2, src4, src6);
    LOAD_4VECS_UB(src + 8, src_stride, src1, src3, src5, src7);
    src += (4 * src_stride);

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out3 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp2, tmp1, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, (v16u8)filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out3);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp2, tmp1, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, (v16u8)filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out3 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp2, tmp1, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out3);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_STORE_VEC(tmp2, tmp1, dst);
    dst += dst_stride;
  }
}

static void common_hv_2ht_2vt_32w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 2; multiple8_cnt--;) {
    common_hv_2ht_2vt_16w_msa(src, src_stride, dst, dst_stride,
                              filter_horiz, filter_vert, height);

    src += 16;
    dst += 16;
  }
}

static void common_hv_2ht_2vt_64w_msa(const uint8_t *src, int32_t src_stride,
                                      uint8_t *dst, int32_t dst_stride,
                                      int8_t *filter_horiz, int8_t *filter_vert,
                                      int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 4; multiple8_cnt--;) {
    common_hv_2ht_2vt_16w_msa(src, src_stride, dst, dst_stride,
                              filter_horiz, filter_vert, height);

    src += 16;
    dst += 16;
  }
}

static void common_hz_2t_and_aver_dst_4x4_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 const255, filt;
  v16u8 res0, res1;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);

  vec2 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec1, filt0);

  vec2 = (v8u16)__msa_srari_h((v8i16)vec2, FILTER_BITS);
  vec3 = (v8u16)__msa_srari_h((v8i16)vec3, FILTER_BITS);

  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  res0 = (v16u8)__msa_pckev_b((v16i8)vec2, (v16i8)vec2);
  res1 = (v16u8)__msa_pckev_b((v16i8)vec3, (v16i8)vec3);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);

  res0 = (v16u8)__msa_aver_u_b((v16u8)res0, (v16u8)dst0);
  res1 = (v16u8)__msa_aver_u_b((v16u8)res1, (v16u8)dst2);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_2t_and_aver_dst_4x8_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 const255, filt;
  v16u8 res0, res1, res2, res3;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_8VECS_UB(src, src_stride,
               src0, src1, src2, src3, src4, src5, src6, src7);

  LOAD_8VECS_UB(dst, dst_stride,
                dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src4);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src6);

  vec4 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec5 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec6 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec7 = __msa_dotp_u_h((v16u8)vec3, filt0);

  vec4 = (v8u16)__msa_srari_h((v8i16)vec4, FILTER_BITS);
  vec5 = (v8u16)__msa_srari_h((v8i16)vec5, FILTER_BITS);
  vec6 = (v8u16)__msa_srari_h((v8i16)vec6, FILTER_BITS);
  vec7 = (v8u16)__msa_srari_h((v8i16)vec7, FILTER_BITS);

  vec4 = __msa_min_u_h(vec4, const255);
  vec5 = __msa_min_u_h(vec5, const255);
  vec6 = __msa_min_u_h(vec6, const255);
  vec7 = __msa_min_u_h(vec7, const255);

  res0 = (v16u8)__msa_pckev_b((v16i8)vec4, (v16i8)vec4);
  res1 = (v16u8)__msa_pckev_b((v16i8)vec5, (v16i8)vec5);
  res2 = (v16u8)__msa_pckev_b((v16i8)vec6, (v16i8)vec6);
  res3 = (v16u8)__msa_pckev_b((v16i8)vec7, (v16i8)vec7);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);
  dst4 = (v16u8)__msa_ilvr_w((v4i32)dst5, (v4i32)dst4);
  dst6 = (v16u8)__msa_ilvr_w((v4i32)dst7, (v4i32)dst6);

  res0 = (v16u8)__msa_aver_u_b((v16u8)res0, (v16u8)dst0);
  res1 = (v16u8)__msa_aver_u_b((v16u8)res1, (v16u8)dst2);
  res2 = (v16u8)__msa_aver_u_b((v16u8)res2, (v16u8)dst4);
  res3 = (v16u8)__msa_aver_u_b((v16u8)res3, (v16u8)dst6);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hz_2t_and_aver_dst_4w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  if (4 == height) {
    common_hz_2t_and_aver_dst_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_hz_2t_and_aver_dst_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_hz_2t_and_aver_dst_8x4_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  vec0 = __msa_min_u_h(vec0, const255);
  vec1 = __msa_min_u_h(vec1, const255);
  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  PCKEV_B_AVG_STORE_8_BYTES_4(vec0, dst0, vec1, dst1, vec2, dst2, vec3, dst3,
                              dst, dst_stride);
}

static void common_hz_2t_and_aver_dst_8x8mult_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter,
                                                  int32_t height) {
  v16u8 filt0, mask;
  v16u8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  vec0 = __msa_min_u_h(vec0, const255);
  vec1 = __msa_min_u_h(vec1, const255);
  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  PCKEV_B_AVG_STORE_8_BYTES_4(vec0, dst0, vec1, dst1, vec2, dst2, vec3, dst3,
                              dst, dst_stride);
  dst += (4 * dst_stride);

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

  vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

  SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3, vec0, vec1, vec2, vec3, FILTER_BITS);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  vec0 = __msa_min_u_h(vec0, const255);
  vec1 = __msa_min_u_h(vec1, const255);
  vec2 = __msa_min_u_h(vec2, const255);
  vec3 = __msa_min_u_h(vec3, const255);

  PCKEV_B_AVG_STORE_8_BYTES_4(vec0, dst0, vec1, dst1, vec2, dst2, vec3, dst3,
                              dst, dst_stride);
  dst += (4 * dst_stride);

  if (16 == height) {
    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

    SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3,
                     vec0, vec1, vec2, vec3, FILTER_BITS);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    vec0 = __msa_min_u_h(vec0, const255);
    vec1 = __msa_min_u_h(vec1, const255);
    vec2 = __msa_min_u_h(vec2, const255);
    vec3 = __msa_min_u_h(vec3, const255);

    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);

    PCKEV_B_AVG_STORE_8_BYTES_4(vec0, dst0, vec1, dst1, vec2, dst2, vec3, dst3,
                                dst, dst_stride);
    dst += (4 * dst_stride);

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);

    vec0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    vec1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    vec2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    vec3 = __msa_dotp_u_h((v16u8)vec3, filt0);

    SRARI_H_4VECS_UH(vec0, vec1, vec2, vec3,
                     vec0, vec1, vec2, vec3, FILTER_BITS);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    vec0 = __msa_min_u_h(vec0, const255);
    vec1 = __msa_min_u_h(vec1, const255);
    vec2 = __msa_min_u_h(vec2, const255);
    vec3 = __msa_min_u_h(vec3, const255);

    PCKEV_B_AVG_STORE_8_BYTES_4(vec0, dst0, vec1, dst1, vec2, dst2, vec3, dst3,
                                dst, dst_stride);
  }
}

static void common_hz_2t_and_aver_dst_8w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  if (4 == height) {
    common_hz_2t_and_aver_dst_8x4_msa(src, src_stride, dst, dst_stride, filter);
  } else {
    common_hz_2t_and_aver_dst_8x8mult_msa(src, src_stride, dst, dst_stride,
                                          filter, height);
  }
}

static void common_hz_2t_and_aver_dst_16w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 dst0, dst1, dst2, dst3;
  v16u8 filt0, mask;
  v8u16 res0, res1, res2, res3, res4, res5, res6, res7;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  src0 = LOAD_UB(src);
  src1 = LOAD_UB(src + 8);
  src += src_stride;
  src2 = LOAD_UB(src);
  src3 = LOAD_UB(src + 8);
  src += src_stride;
  src4 = LOAD_UB(src);
  src5 = LOAD_UB(src + 8);
  src += src_stride;
  src6 = LOAD_UB(src);
  src7 = LOAD_UB(src + 8);
  src += src_stride;

  vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
  vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
  vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
  vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

  res0 = __msa_dotp_u_h((v16u8)vec0, filt0);
  res1 = __msa_dotp_u_h((v16u8)vec1, filt0);
  res2 = __msa_dotp_u_h((v16u8)vec2, filt0);
  res3 = __msa_dotp_u_h((v16u8)vec3, filt0);
  res4 = __msa_dotp_u_h((v16u8)vec4, filt0);
  res5 = __msa_dotp_u_h((v16u8)vec5, filt0);
  res6 = __msa_dotp_u_h((v16u8)vec6, filt0);
  res7 = __msa_dotp_u_h((v16u8)vec7, filt0);

  res0 = (v8u16)__msa_srari_h((v8i16)res0, FILTER_BITS);
  res1 = (v8u16)__msa_srari_h((v8i16)res1, FILTER_BITS);
  res2 = (v8u16)__msa_srari_h((v8i16)res2, FILTER_BITS);
  res3 = (v8u16)__msa_srari_h((v8i16)res3, FILTER_BITS);
  res4 = (v8u16)__msa_srari_h((v8i16)res4, FILTER_BITS);
  res5 = (v8u16)__msa_srari_h((v8i16)res5, FILTER_BITS);
  res6 = (v8u16)__msa_srari_h((v8i16)res6, FILTER_BITS);
  res7 = (v8u16)__msa_srari_h((v8i16)res7, FILTER_BITS);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  res0 = __msa_min_u_h(res0, const255);
  res1 = __msa_min_u_h(res1, const255);
  res2 = __msa_min_u_h(res2, const255);
  res3 = __msa_min_u_h(res3, const255);
  res4 = __msa_min_u_h(res4, const255);
  res5 = __msa_min_u_h(res5, const255);
  res6 = __msa_min_u_h(res6, const255);
  res7 = __msa_min_u_h(res7, const255);

  PCKEV_B_AVG_STORE_VEC(res1, res0, dst0, dst);
  dst += dst_stride;
  PCKEV_B_AVG_STORE_VEC(res3, res2, dst1, dst);
  dst += dst_stride;
  PCKEV_B_AVG_STORE_VEC(res5, res4, dst2, dst);
  dst += dst_stride;
  PCKEV_B_AVG_STORE_VEC(res7, res6, dst3, dst);
  dst += dst_stride;

  for (loop_cnt = (height >> 2) - 1; loop_cnt--;) {
    src0 = LOAD_UB(src);
    src1 = LOAD_UB(src + 8);
    src += src_stride;
    src2 = LOAD_UB(src);
    src3 = LOAD_UB(src + 8);
    src += src_stride;
    src4 = LOAD_UB(src);
    src5 = LOAD_UB(src + 8);
    src += src_stride;
    src6 = LOAD_UB(src);
    src7 = LOAD_UB(src + 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    res0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    res1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    res2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    res3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    res4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    res5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    res6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    res7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    res0 = (v8u16)__msa_srari_h((v8i16)res0, FILTER_BITS);
    res1 = (v8u16)__msa_srari_h((v8i16)res1, FILTER_BITS);
    res2 = (v8u16)__msa_srari_h((v8i16)res2, FILTER_BITS);
    res3 = (v8u16)__msa_srari_h((v8i16)res3, FILTER_BITS);
    res4 = (v8u16)__msa_srari_h((v8i16)res4, FILTER_BITS);
    res5 = (v8u16)__msa_srari_h((v8i16)res5, FILTER_BITS);
    res6 = (v8u16)__msa_srari_h((v8i16)res6, FILTER_BITS);
    res7 = (v8u16)__msa_srari_h((v8i16)res7, FILTER_BITS);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    res0 = __msa_min_u_h(res0, const255);
    res1 = __msa_min_u_h(res1, const255);
    res2 = __msa_min_u_h(res2, const255);
    res3 = __msa_min_u_h(res3, const255);
    res4 = __msa_min_u_h(res4, const255);
    res5 = __msa_min_u_h(res5, const255);
    res6 = __msa_min_u_h(res6, const255);
    res7 = __msa_min_u_h(res7, const255);

    PCKEV_B_AVG_STORE_VEC(res1, res0, dst0, dst);
    dst += dst_stride;
    PCKEV_B_AVG_STORE_VEC(res3, res2, dst1, dst);
    dst += dst_stride;
    PCKEV_B_AVG_STORE_VEC(res5, res4, dst2, dst);
    dst += dst_stride;
    PCKEV_B_AVG_STORE_VEC(res7, res6, dst3, dst);
    dst += dst_stride;
  }
}

static void common_hz_2t_and_aver_dst_32w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 dst0, dst1, dst2, dst3;
  v16u8 filt0, mask;
  v8u16 res0, res1, res2, res3, res4, res5, res6, res7;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  for (loop_cnt = (height >> 1); loop_cnt--;) {
    src0 = LOAD_UB(src);
    src2 = LOAD_UB(src + 16);
    src3 = LOAD_UB(src + 24);
    src1 = (v16u8)__msa_sld_b((v16i8)src2, (v16i8)src0, 8);
    src += src_stride;
    src4 = LOAD_UB(src);
    src6 = LOAD_UB(src + 16);
    src7 = LOAD_UB(src + 24);
    src5 = (v16u8)__msa_sld_b((v16i8)src6, (v16i8)src4, 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    res0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    res1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    res2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    res3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    res4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    res5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    res6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    res7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    res0 = (v8u16)__msa_srari_h((v8i16)res0, FILTER_BITS);
    res1 = (v8u16)__msa_srari_h((v8i16)res1, FILTER_BITS);
    res2 = (v8u16)__msa_srari_h((v8i16)res2, FILTER_BITS);
    res3 = (v8u16)__msa_srari_h((v8i16)res3, FILTER_BITS);
    res4 = (v8u16)__msa_srari_h((v8i16)res4, FILTER_BITS);
    res5 = (v8u16)__msa_srari_h((v8i16)res5, FILTER_BITS);
    res6 = (v8u16)__msa_srari_h((v8i16)res6, FILTER_BITS);
    res7 = (v8u16)__msa_srari_h((v8i16)res7, FILTER_BITS);

    res0 = __msa_min_u_h(res0, const255);
    res1 = __msa_min_u_h(res1, const255);
    res2 = __msa_min_u_h(res2, const255);
    res3 = __msa_min_u_h(res3, const255);
    res4 = __msa_min_u_h(res4, const255);
    res5 = __msa_min_u_h(res5, const255);
    res6 = __msa_min_u_h(res6, const255);
    res7 = __msa_min_u_h(res7, const255);

    LOAD_2VECS_UB(dst, 16, dst0, dst1);
    PCKEV_B_AVG_STORE_VEC(res1, res0, dst0, dst);
    PCKEV_B_AVG_STORE_VEC(res3, res2, dst1, (dst + 16));
    dst += dst_stride;
    LOAD_2VECS_UB(dst, 16, dst2, dst3);
    PCKEV_B_AVG_STORE_VEC(res5, res4, dst2, dst);
    PCKEV_B_AVG_STORE_VEC(res7, res6, dst3, (dst + 16));
    dst += dst_stride;
  }
}

static void common_hz_2t_and_aver_dst_64w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 dst0, dst1, dst2, dst3;
  v16u8 filt0, mask;
  v8u16 out0, out1, out2, out3, out4, out5, out6, out7;
  v8u16 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 const255, filt;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  const255 = (v8u16)__msa_ldi_h(255);

  for (loop_cnt = height; loop_cnt--;) {
    LOAD_4VECS_UB(src, 16, src0, src2, src4, src6);
    src7 = LOAD_UB(src + 56);
    src1 = (v16u8)__msa_sld_b((v16i8)src2, (v16i8)src0, 8);
    src3 = (v16u8)__msa_sld_b((v16i8)src4, (v16i8)src2, 8);
    src5 = (v16u8)__msa_sld_b((v16i8)src6, (v16i8)src4, 8);
    src += src_stride;

    vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    vec2 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    vec3 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    vec4 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    vec5 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    vec6 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    vec7 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);

    out0 = __msa_dotp_u_h((v16u8)vec0, filt0);
    out1 = __msa_dotp_u_h((v16u8)vec1, filt0);
    out2 = __msa_dotp_u_h((v16u8)vec2, filt0);
    out3 = __msa_dotp_u_h((v16u8)vec3, filt0);
    out4 = __msa_dotp_u_h((v16u8)vec4, filt0);
    out5 = __msa_dotp_u_h((v16u8)vec5, filt0);
    out6 = __msa_dotp_u_h((v16u8)vec6, filt0);
    out7 = __msa_dotp_u_h((v16u8)vec7, filt0);

    out0 = (v8u16)__msa_srari_h((v8i16)out0, FILTER_BITS);
    out1 = (v8u16)__msa_srari_h((v8i16)out1, FILTER_BITS);
    out2 = (v8u16)__msa_srari_h((v8i16)out2, FILTER_BITS);
    out3 = (v8u16)__msa_srari_h((v8i16)out3, FILTER_BITS);
    out4 = (v8u16)__msa_srari_h((v8i16)out4, FILTER_BITS);
    out5 = (v8u16)__msa_srari_h((v8i16)out5, FILTER_BITS);
    out6 = (v8u16)__msa_srari_h((v8i16)out6, FILTER_BITS);
    out7 = (v8u16)__msa_srari_h((v8i16)out7, FILTER_BITS);

    LOAD_4VECS_UB(dst, 16, dst0, dst1, dst2, dst3);

    out0 = __msa_min_u_h(out0, const255);
    out1 = __msa_min_u_h(out1, const255);
    out2 = __msa_min_u_h(out2, const255);
    out3 = __msa_min_u_h(out3, const255);
    out4 = __msa_min_u_h(out4, const255);
    out5 = __msa_min_u_h(out5, const255);
    out6 = __msa_min_u_h(out6, const255);
    out7 = __msa_min_u_h(out7, const255);

    PCKEV_B_AVG_STORE_VEC(out1, out0, dst0, dst);
    PCKEV_B_AVG_STORE_VEC(out3, out2, dst1, dst + 16);
    PCKEV_B_AVG_STORE_VEC(out5, out4, dst2, dst + 32);
    PCKEV_B_AVG_STORE_VEC(out7, out6, dst3, dst + 48);
    dst += dst_stride;
  }
}

static void common_vt_2t_and_aver_dst_4x4_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3;
  v16i8 src0, src1, src2, src3, src4;
  v16u8 dst0, dst1, dst2, dst3;
  v16i8 src10_r, src32_r, src21_r, src43_r, src2110, src4332;
  v8i16 filt;
  v16i8 filt0;

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);

  LOAD_4VECS_SB(src, src_stride, src0, src1, src2, src3);
  src += (4 * src_stride);

  src4 = LOAD_SB(src);
  src += src_stride;

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst1 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);

  dst0 = (v16u8)__msa_ilvr_d((v2i64)dst1, (v2i64)dst0);

  ILVR_B_4VECS_SB(src0, src1, src2, src3, src1, src2, src3, src4,
                  src10_r, src21_r, src32_r, src43_r);

  ILVR_D_2VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r);

  src2110 = (v16i8)__msa_dotp_u_h((v16u8)src2110, (v16u8)filt0);
  src4332 = (v16i8)__msa_dotp_u_h((v16u8)src4332, (v16u8)filt0);

  src2110 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src2110, FILTER_BITS, 7);
  src4332 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src4332, FILTER_BITS, 7);

  src2110 = (v16i8)__msa_pckev_b((v16i8)src4332, src2110);
  src2110 = (v16i8)__msa_aver_u_b((v16u8)src2110, (v16u8)dst0);

  out0 = __msa_copy_u_w((v4i32)src2110, 0);
  out1 = __msa_copy_u_w((v4i32)src2110, 1);
  out2 = __msa_copy_u_w((v4i32)src2110, 2);
  out3 = __msa_copy_u_w((v4i32)src2110, 3);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_vt_2t_and_aver_dst_4x8_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  uint32_t out0, out1, out2, out3, out4, out5, out6, out7;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v16i8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16i8 src10_r, src32_r, src54_r, src76_r, src21_r, src43_r, src65_r, src87_r;
  v16i8 src2110, src4332, src6554, src8776;
  v16i8 filt0;
  v8i16 filt;

  filt = LOAD_SH(filter);
  filt0 = (v16i8)__msa_splati_h(filt, 0);

  LOAD_8VECS_SB(src, src_stride,
                src0, src1, src2, src3, src4, src5, src6, src7);
  src += (8 * src_stride);

  src8 = LOAD_SB(src);
  src += src_stride;

  LOAD_8VECS_UB(dst, dst_stride,
                dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst1 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst5, (v4i32)dst4);
  dst3 = (v16u8)__msa_ilvr_w((v4i32)dst7, (v4i32)dst6);

  dst0 = (v16u8)__msa_ilvr_d((v2i64)dst1, (v2i64)dst0);
  dst1 = (v16u8)__msa_ilvr_d((v2i64)dst3, (v2i64)dst2);

  ILVR_B_8VECS_SB(src0, src1, src2, src3, src4, src5, src6, src7,
                  src1, src2, src3, src4, src5, src6, src7, src8,
                  src10_r, src21_r, src32_r, src43_r,
                  src54_r, src65_r, src76_r, src87_r);

  ILVR_D_4VECS_SB(src2110, src21_r, src10_r, src4332, src43_r, src32_r,
                  src6554, src65_r, src54_r, src8776, src87_r, src76_r);

  src2110 = (v16i8)__msa_dotp_u_h((v16u8)src2110, (v16u8)filt0);
  src4332 = (v16i8)__msa_dotp_u_h((v16u8)src4332, (v16u8)filt0);
  src6554 = (v16i8)__msa_dotp_u_h((v16u8)src6554, (v16u8)filt0);
  src8776 = (v16i8)__msa_dotp_u_h((v16u8)src8776, (v16u8)filt0);

  src2110 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src2110, FILTER_BITS, 7);
  src4332 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src4332, FILTER_BITS, 7);
  src6554 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src6554, FILTER_BITS, 7);
  src8776 = (v16i8)SRARI_SATURATE_UNSIGNED_H(src8776, FILTER_BITS, 7);

  src2110 = (v16i8)__msa_pckev_b((v16i8)src4332, (v16i8)src2110);
  src4332 = (v16i8)__msa_pckev_b((v16i8)src8776, (v16i8)src6554);

  src2110 = (v16i8)__msa_aver_u_b((v16u8)src2110, (v16u8)dst0);
  src4332 = (v16i8)__msa_aver_u_b((v16u8)src4332, (v16u8)dst1);

  out0 = __msa_copy_u_w((v4i32)src2110, 0);
  out1 = __msa_copy_u_w((v4i32)src2110, 1);
  out2 = __msa_copy_u_w((v4i32)src2110, 2);
  out3 = __msa_copy_u_w((v4i32)src2110, 3);
  out4 = __msa_copy_u_w((v4i32)src4332, 0);
  out5 = __msa_copy_u_w((v4i32)src4332, 1);
  out6 = __msa_copy_u_w((v4i32)src4332, 2);
  out7 = __msa_copy_u_w((v4i32)src4332, 3);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;
  STORE_WORD(dst, out4);
  dst += dst_stride;
  STORE_WORD(dst, out5);
  dst += dst_stride;
  STORE_WORD(dst, out6);
  dst += dst_stride;
  STORE_WORD(dst, out7);
}

static void common_vt_2t_and_aver_dst_4w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  if (4 == height) {
    common_vt_2t_and_aver_dst_4x4_msa(src, src_stride, dst, dst_stride, filter);
  } else if (8 == height) {
    common_vt_2t_and_aver_dst_4x8_msa(src, src_stride, dst, dst_stride, filter);
  }
}

static void common_vt_2t_and_aver_dst_8x4_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter) {
  v16u8 src0, src1, src2, src3, src4;
  v16u8 dst0, dst1, dst2, dst3;
  v16u8 vec0, vec1, vec2, vec3, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  ILVR_B_2VECS_UB(src0, src1, src1, src2, vec0, vec1);
  ILVR_B_2VECS_UB(src2, src3, src3, src4, vec2, vec3);

  tmp0 = __msa_dotp_u_h(vec0, filt0);
  tmp1 = __msa_dotp_u_h(vec1, filt0);
  tmp2 = __msa_dotp_u_h(vec2, filt0);
  tmp3 = __msa_dotp_u_h(vec3, filt0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
  tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
  tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

  PCKEV_B_AVG_STORE_8_BYTES_4(tmp0, dst0, tmp1, dst1, tmp2, dst2, tmp3, dst3,
                              dst, dst_stride);
}

static void common_vt_2t_and_aver_dst_8x8mult_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter,
                                                  int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16u8 dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;

  for (loop_cnt = (height >> 3); loop_cnt--;) {
    LOAD_8VECS_UB(src, src_stride,
                 src1, src2, src3, src4, src5, src6, src7, src8);
    src += (8 * src_stride);

    LOAD_8VECS_UB(dst, dst_stride,
                  dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8);

    ILVR_B_4VECS_UB(src0, src1, src2, src3, src1, src2, src3, src4,
                    vec0, vec1, vec2, vec3);

    ILVR_B_4VECS_UB(src4, src5, src6, src7, src5, src6, src7, src8,
                    vec4, vec5, vec6, vec7);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);
    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_8_BYTES_4(tmp0, dst1, tmp1, dst2,
                                tmp2, dst3, tmp3, dst4,
                                dst, dst_stride);
    dst += (4 * dst_stride);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);
    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_8_BYTES_4(tmp0, dst5, tmp1, dst6,
                                tmp2, dst7, tmp3, dst8,
                                dst, dst_stride);
    dst += (4 * dst_stride);

    src0 = src8;
  }
}

static void common_vt_2t_and_aver_dst_8w_msa(const uint8_t *src,
                                             int32_t src_stride,
                                             uint8_t *dst,
                                             int32_t dst_stride,
                                             int8_t *filter,
                                             int32_t height) {
  if (4 == height) {
    common_vt_2t_and_aver_dst_8x4_msa(src, src_stride, dst, dst_stride,
                                      filter);
  } else {
    common_vt_2t_and_aver_dst_8x8mult_msa(src, src_stride, dst, dst_stride,
                                          filter, height);
  }
}

static void common_vt_2t_and_aver_dst_16w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 dst0, dst1, dst2, dst3, filt0;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst0, dst);
    dst += dst_stride;

    ILV_B_LRLR_UB(src2, src3, src3, src4, vec5, vec4, vec7, vec6);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst1, dst);
    dst += dst_stride;

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst2, dst);
    dst += dst_stride;

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst3, dst);
    dst += dst_stride;

    src0 = src4;
  }
}

static void common_vt_2t_and_aver_dst_32w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8, src9;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, filt0;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src5 = LOAD_UB(src + 16);
  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    LOAD_4VECS_UB(src + 16, src_stride, src6, src7, src8, src9);
    LOAD_4VECS_UB(dst + 16, dst_stride, dst4, dst5, dst6, dst7);

    src += (4 * src_stride);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst0, dst);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst1, dst + dst_stride);

    ILV_B_LRLR_UB(src2, src3, src3, src4, vec5, vec4, vec7, vec6);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst2, dst + 2 * dst_stride);

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst3, dst + 3 * dst_stride);

    ILV_B_LRLR_UB(src5, src6, src6, src7, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst4, dst + 16);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst5, dst + 16 + dst_stride);

    ILV_B_LRLR_UB(src7, src8, src8, src9, vec5, vec4, vec7, vec6);

    tmp0 = __msa_dotp_u_h(vec4, filt0);
    tmp1 = __msa_dotp_u_h(vec5, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst6, dst + 16 + 2 * dst_stride);

    tmp2 = __msa_dotp_u_h(vec6, filt0);
    tmp3 = __msa_dotp_u_h(vec7, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst7, dst + 16 + 3 * dst_stride);
    dst += (4 * dst_stride);

    src0 = src4;
    src5 = src9;
  }
}

static void common_vt_2t_and_aver_dst_64w_msa(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              int8_t *filter,
                                              int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5;
  v16u8 src6, src7, src8, src9, src10, src11, filt0;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v16u8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
  v8u16 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  v8u16 filt;

  /* rearranging filter_y */
  filt = LOAD_UH(filter);
  filt0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_4VECS_UB(src, 16, src0, src3, src6, src9);

  src += src_stride;

  for (loop_cnt = (height >> 1); loop_cnt--;) {
    LOAD_2VECS_UB(src, src_stride, src1, src2);
    LOAD_2VECS_UB(dst, dst_stride, dst0, dst1);

    LOAD_2VECS_UB(src + 16, src_stride, src4, src5);
    LOAD_2VECS_UB(dst + 16, dst_stride, dst2, dst3);

    LOAD_2VECS_UB(src + 32, src_stride, src7, src8);
    LOAD_2VECS_UB(dst + 32, dst_stride, dst4, dst5);

    LOAD_2VECS_UB(src + 48, src_stride, src10, src11);
    LOAD_2VECS_UB(dst + 48, dst_stride, dst6, dst7);

    src += (2 * src_stride);

    ILV_B_LRLR_UB(src0, src1, src1, src2, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst0, dst);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst1, dst + dst_stride);

    ILV_B_LRLR_UB(src3, src4, src4, src5, vec5, vec4, vec7, vec6);

    tmp4 = __msa_dotp_u_h(vec4, filt0);
    tmp5 = __msa_dotp_u_h(vec5, filt0);

    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);
    tmp5 = SRARI_SATURATE_UNSIGNED_H(tmp5, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp5, tmp4, dst2, dst + 16);

    tmp6 = __msa_dotp_u_h(vec6, filt0);
    tmp7 = __msa_dotp_u_h(vec7, filt0);

    tmp6 = SRARI_SATURATE_UNSIGNED_H(tmp6, FILTER_BITS, 7);
    tmp7 = SRARI_SATURATE_UNSIGNED_H(tmp7, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp7, tmp6, dst3, dst + 16 + dst_stride);

    ILV_B_LRLR_UB(src6, src7, src7, src8, vec1, vec0, vec3, vec2);

    tmp0 = __msa_dotp_u_h(vec0, filt0);
    tmp1 = __msa_dotp_u_h(vec1, filt0);

    tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp1, tmp0, dst4, dst + 32);

    tmp2 = __msa_dotp_u_h(vec2, filt0);
    tmp3 = __msa_dotp_u_h(vec3, filt0);

    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp3, tmp2, dst5, dst + 32 + dst_stride);

    ILV_B_LRLR_UB(src9, src10, src10, src11, vec5, vec4, vec7, vec6);

    tmp4 = __msa_dotp_u_h(vec4, filt0);
    tmp5 = __msa_dotp_u_h(vec5, filt0);

    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);
    tmp5 = SRARI_SATURATE_UNSIGNED_H(tmp5, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp5, tmp4, dst6, (dst + 48));

    tmp6 = __msa_dotp_u_h(vec6, filt0);
    tmp7 = __msa_dotp_u_h(vec7, filt0);

    tmp6 = SRARI_SATURATE_UNSIGNED_H(tmp6, FILTER_BITS, 7);
    tmp7 = SRARI_SATURATE_UNSIGNED_H(tmp7, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp7, tmp6, dst7, dst + 48 + dst_stride);
    dst += (2 * dst_stride);

    src0 = src2;
    src3 = src5;
    src6 = src8;
    src9 = src11;
  }
}

static void common_hv_2ht_2vt_and_aver_dst_4x4_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert) {
  uint32_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 filt_horiz0, filt_vert0, horiz_vec, vec0, vec1;
  v8u16 filt;
  v16u8 res0, res1, mask;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3, horiz_out4;
  v8u16 tmp0, tmp1;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  horiz_out2 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_out2, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  horiz_out4 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out4 = SRARI_SATURATE_UNSIGNED_H(horiz_out4, FILTER_BITS, 7);

  horiz_out1 = (v8u16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8u16)__msa_pckod_d((v2i64)horiz_out4, (v2i64)horiz_out2);

  vec0 = (v16u8)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  vec1 = (v16u8)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);

  tmp0 = __msa_dotp_u_h(vec0, filt_vert0);
  tmp1 = __msa_dotp_u_h(vec1, filt_vert0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);

  res0 = (v16u8)__msa_pckev_b((v16i8)tmp0, (v16i8)tmp0);
  res1 = (v16u8)__msa_pckev_b((v16i8)tmp1, (v16i8)tmp1);

  res0 = (v16u8)__msa_aver_u_b((v16u8)res0, (v16u8)dst0);
  res1 = (v16u8)__msa_aver_u_b((v16u8)res1, (v16u8)dst2);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hv_2ht_2vt_and_aver_dst_4x8_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert) {
  uint32_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7, src8;
  v16u8 filt_horiz0, mask, horiz_vec;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8u16 horiz_out4, horiz_out5, horiz_out6, horiz_out7, horiz_out8;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 filt, filt_vert0;
  v16u8 res0, res1, res2, res3;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;

  mask = LOAD_UB(&mc_filt_mask_arr[16]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  LOAD_8VECS_UB(src, src_stride,
                src0, src1, src2, src3, src4, src5, src6, src7);
  src += (8 * src_stride);

  src8 = LOAD_UB(src);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src0);
  horiz_out0 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_out0, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src2);
  horiz_out2 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_out2, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src4);
  horiz_out4 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out4 = SRARI_SATURATE_UNSIGNED_H(horiz_out4, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src6);
  horiz_out6 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out6 = SRARI_SATURATE_UNSIGNED_H(horiz_out6, FILTER_BITS, 7);

  horiz_vec = (v16u8)__msa_vshf_b((v16i8)mask, (v16i8)src8, (v16i8)src8);
  horiz_out8 = __msa_dotp_u_h(horiz_vec, filt_horiz0);
  horiz_out8 = SRARI_SATURATE_UNSIGNED_H(horiz_out8, FILTER_BITS, 7);

  horiz_out1 = (v8u16)__msa_sldi_b((v16i8)horiz_out2, (v16i8)horiz_out0, 8);
  horiz_out3 = (v8u16)__msa_sldi_b((v16i8)horiz_out4, (v16i8)horiz_out2, 8);
  horiz_out5 = (v8u16)__msa_sldi_b((v16i8)horiz_out6, (v16i8)horiz_out4, 8);
  horiz_out7 = (v8u16)__msa_pckod_d((v2i64)horiz_out8, (v2i64)horiz_out6);

  LOAD_8VECS_UB(dst, dst_stride,
                dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7);

  dst0 = (v16u8)__msa_ilvr_w((v4i32)dst1, (v4i32)dst0);
  dst2 = (v16u8)__msa_ilvr_w((v4i32)dst3, (v4i32)dst2);
  dst4 = (v16u8)__msa_ilvr_w((v4i32)dst5, (v4i32)dst4);
  dst6 = (v16u8)__msa_ilvr_w((v4i32)dst7, (v4i32)dst6);

  vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  vec1 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
  vec2 = (v8u16)__msa_ilvev_b((v16i8)horiz_out5, (v16i8)horiz_out4);
  vec3 = (v8u16)__msa_ilvev_b((v16i8)horiz_out7, (v16i8)horiz_out6);

  tmp0 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);
  tmp1 = __msa_dotp_u_h((v16u8)vec1, (v16u8)filt_vert0);
  tmp2 = __msa_dotp_u_h((v16u8)vec2, (v16u8)filt_vert0);
  tmp3 = __msa_dotp_u_h((v16u8)vec3, (v16u8)filt_vert0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
  tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
  tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

  res0 = (v16u8)__msa_pckev_b((v16i8)tmp0, (v16i8)tmp0);
  res1 = (v16u8)__msa_pckev_b((v16i8)tmp1, (v16i8)tmp1);
  res2 = (v16u8)__msa_pckev_b((v16i8)tmp2, (v16i8)tmp2);
  res3 = (v16u8)__msa_pckev_b((v16i8)tmp3, (v16i8)tmp3);

  res0 = (v16u8)__msa_aver_u_b((v16u8)res0, (v16u8)dst0);
  res1 = (v16u8)__msa_aver_u_b((v16u8)res1, (v16u8)dst2);
  res2 = (v16u8)__msa_aver_u_b((v16u8)res2, (v16u8)dst4);
  res3 = (v16u8)__msa_aver_u_b((v16u8)res3, (v16u8)dst6);

  out0 = __msa_copy_u_w((v4i32)res0, 0);
  out1 = __msa_copy_u_w((v4i32)res0, 1);
  out2 = __msa_copy_u_w((v4i32)res1, 0);
  out3 = __msa_copy_u_w((v4i32)res1, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
  dst += dst_stride;

  out0 = __msa_copy_u_w((v4i32)res2, 0);
  out1 = __msa_copy_u_w((v4i32)res2, 1);
  out2 = __msa_copy_u_w((v4i32)res3, 0);
  out3 = __msa_copy_u_w((v4i32)res3, 1);

  STORE_WORD(dst, out0);
  dst += dst_stride;
  STORE_WORD(dst, out1);
  dst += dst_stride;
  STORE_WORD(dst, out2);
  dst += dst_stride;
  STORE_WORD(dst, out3);
}

static void common_hv_2ht_2vt_and_aver_dst_4w_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter_horiz,
                                                  int8_t *filter_vert,
                                                  int32_t height) {
  if (4 == height) {
    common_hv_2ht_2vt_and_aver_dst_4x4_msa(src, src_stride, dst, dst_stride,
                                           filter_horiz, filter_vert);
  } else if (8 == height) {
    common_hv_2ht_2vt_and_aver_dst_4x8_msa(src, src_stride, dst, dst_stride,
                                           filter_horiz, filter_vert);
  }
}

static void common_hv_2ht_2vt_and_aver_dst_8x4_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert) {
  v16u8 src0, src1, src2, src3, src4;
  v16u8 filt_horiz0, mask;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 horiz_vec, horiz_out0, horiz_out1;
  v8u16 tmp0, tmp1, tmp2, tmp3;
  v8u16 vec0, vec1, vec2, vec3;
  v8u16 filt, filt_vert0;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  LOAD_5VECS_UB(src, src_stride, src0, src1, src2, src3, src4);
  src += (5 * src_stride);

  LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  tmp0 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  vec1 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
  tmp1 = __msa_dotp_u_h((v16u8)vec1, (v16u8)filt_vert0);

  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  vec2 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
  tmp2 = __msa_dotp_u_h((v16u8)vec2, (v16u8)filt_vert0);

  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  vec3 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
  tmp3 = __msa_dotp_u_h((v16u8)vec3, (v16u8)filt_vert0);

  tmp0 = SRARI_SATURATE_UNSIGNED_H(tmp0, FILTER_BITS, 7);
  tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
  tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);
  tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);

  PCKEV_B_AVG_STORE_8_BYTES_4(tmp0, dst0, tmp1, dst1, tmp2, dst2, tmp3, dst3,
                              dst, dst_stride);
}

static void common_hv_2ht_2vt_and_aver_dst_8x8mult_msa(const uint8_t *src,
                                                       int32_t src_stride,
                                                       uint8_t *dst,
                                                       int32_t dst_stride,
                                                       int8_t *filter_horiz,
                                                       int8_t *filter_vert,
                                                       int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4;
  v16u8 filt_horiz0, mask;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 horiz_vec, horiz_out0, horiz_out1;
  v8u16 tmp1, tmp2, tmp3, tmp4, vec0;
  v8u16 filt, filt_vert0;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src += src_stride;
  horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src1, src2, src3, src4);
    src += (4 * src_stride);

    horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);

    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp3 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    horiz_vec = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    horiz_vec = __msa_dotp_u_h((v16u8)horiz_vec, filt_horiz0);

    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp4 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp3 = SRARI_SATURATE_UNSIGNED_H(tmp3, FILTER_BITS, 7);
    tmp4 = SRARI_SATURATE_UNSIGNED_H(tmp4, FILTER_BITS, 7);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    PCKEV_B_AVG_STORE_8_BYTES_4(tmp1, dst0, tmp2, dst1, tmp3, dst2, tmp4, dst3,
                                dst, dst_stride);
    dst += (4 * dst_stride);
  }
}

static void common_hv_2ht_2vt_and_aver_dst_8w_msa(const uint8_t *src,
                                                  int32_t src_stride,
                                                  uint8_t *dst,
                                                  int32_t dst_stride,
                                                  int8_t *filter_horiz,
                                                  int8_t *filter_vert,
                                                  int32_t height) {
  if (4 == height) {
    common_hv_2ht_2vt_and_aver_dst_8x4_msa(src, src_stride, dst, dst_stride,
                                           filter_horiz, filter_vert);
  } else {
    common_hv_2ht_2vt_and_aver_dst_8x8mult_msa(src, src_stride, dst, dst_stride,
                                               filter_horiz, filter_vert,
                                               height);
  }
}

static void common_hv_2ht_2vt_and_aver_dst_16w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  uint32_t loop_cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 filt_horiz0, mask;
  v16u8 dst0, dst1, dst2, dst3;
  v8u16 horiz_vec0, horiz_vec1;
  v8u16 horiz_out0, horiz_out1, horiz_out2, horiz_out3;
  v8u16 tmp1, tmp2;
  v8u16 filt, filt_vert0, vec0;

  mask = LOAD_UB(&mc_filt_mask_arr[0]);

  /* rearranging filter */
  filt = LOAD_UH(filter_horiz);
  filt_horiz0 = (v16u8)__msa_splati_h((v8i16)filt, 0);

  filt = LOAD_UH(filter_vert);
  filt_vert0 = (v8u16)__msa_splati_h((v8i16)filt, 0);

  src0 = LOAD_UB(src);
  src1 = LOAD_UB(src + 8);

  horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
  horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
  horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

  horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
  horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
  horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

  src += src_stride;

  for (loop_cnt = (height >> 2); loop_cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src0, src2, src4, src6);

    LOAD_4VECS_UB(src + 8, src_stride, src1, src3, src5, src7);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src0, (v16i8)src0);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src1, (v16i8)src1);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out3 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp2, tmp1, dst0, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src2, (v16i8)src2);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src3, (v16i8)src3);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out3);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp2, tmp1, dst1, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src4, (v16i8)src4);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out1 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src5, (v16i8)src5);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out3 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out1, (v16i8)horiz_out0);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out3, (v16i8)horiz_out2);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp2, tmp1, dst2, dst);
    dst += dst_stride;

    horiz_vec0 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src6, (v16i8)src6);
    horiz_vec0 = __msa_dotp_u_h((v16u8)horiz_vec0, filt_horiz0);
    horiz_out0 = SRARI_SATURATE_UNSIGNED_H(horiz_vec0, FILTER_BITS, 7);

    horiz_vec1 = (v8u16)__msa_vshf_b((v16i8)mask, (v16i8)src7, (v16i8)src7);
    horiz_vec1 = __msa_dotp_u_h((v16u8)horiz_vec1, filt_horiz0);
    horiz_out2 = SRARI_SATURATE_UNSIGNED_H(horiz_vec1, FILTER_BITS, 7);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out0, (v16i8)horiz_out1);
    tmp1 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    vec0 = (v8u16)__msa_ilvev_b((v16i8)horiz_out2, (v16i8)horiz_out3);
    tmp2 = __msa_dotp_u_h((v16u8)vec0, (v16u8)filt_vert0);

    tmp1 = SRARI_SATURATE_UNSIGNED_H(tmp1, FILTER_BITS, 7);
    tmp2 = SRARI_SATURATE_UNSIGNED_H(tmp2, FILTER_BITS, 7);

    PCKEV_B_AVG_STORE_VEC(tmp2, tmp1, dst3, dst);
    dst += dst_stride;
  }
}

static void common_hv_2ht_2vt_and_aver_dst_32w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 2; multiple8_cnt--;) {
    common_hv_2ht_2vt_and_aver_dst_16w_msa(src, src_stride, dst, dst_stride,
                                           filter_horiz, filter_vert, height);

    src += 16;
    dst += 16;
  }
}

static void common_hv_2ht_2vt_and_aver_dst_64w_msa(const uint8_t *src,
                                                   int32_t src_stride,
                                                   uint8_t *dst,
                                                   int32_t dst_stride,
                                                   int8_t *filter_horiz,
                                                   int8_t *filter_vert,
                                                   int32_t height) {
  int32_t multiple8_cnt;

  for (multiple8_cnt = 4; multiple8_cnt--;) {
    common_hv_2ht_2vt_and_aver_dst_16w_msa(src, src_stride, dst, dst_stride,
                                           filter_horiz, filter_vert, height);

    src += 16;
    dst += 16;
  }
}

static void copy_width8_msa(const uint8_t *src, int32_t src_stride,
                            uint8_t *dst, int32_t dst_stride,
                            int32_t height) {
  int32_t cnt;
  uint64_t out0, out1, out2, out3, out4, out5, out6, out7;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

  if (0 == height % 12) {
    for (cnt = (height / 12); cnt--;) {
      LOAD_8VECS_UB(src, src_stride,
                    src0, src1, src2, src3, src4, src5, src6, src7);
      src += (8 * src_stride);

      out0 = __msa_copy_u_d((v2i64)src0, 0);
      out1 = __msa_copy_u_d((v2i64)src1, 0);
      out2 = __msa_copy_u_d((v2i64)src2, 0);
      out3 = __msa_copy_u_d((v2i64)src3, 0);
      out4 = __msa_copy_u_d((v2i64)src4, 0);
      out5 = __msa_copy_u_d((v2i64)src5, 0);
      out6 = __msa_copy_u_d((v2i64)src6, 0);
      out7 = __msa_copy_u_d((v2i64)src7, 0);

      STORE_DWORD(dst, out0);
      dst += dst_stride;
      STORE_DWORD(dst, out1);
      dst += dst_stride;
      STORE_DWORD(dst, out2);
      dst += dst_stride;
      STORE_DWORD(dst, out3);
      dst += dst_stride;
      STORE_DWORD(dst, out4);
      dst += dst_stride;
      STORE_DWORD(dst, out5);
      dst += dst_stride;
      STORE_DWORD(dst, out6);
      dst += dst_stride;
      STORE_DWORD(dst, out7);
      dst += dst_stride;

      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      src += (4 * src_stride);

      out0 = __msa_copy_u_d((v2i64)src0, 0);
      out1 = __msa_copy_u_d((v2i64)src1, 0);
      out2 = __msa_copy_u_d((v2i64)src2, 0);
      out3 = __msa_copy_u_d((v2i64)src3, 0);

      STORE_DWORD(dst, out0);
      dst += dst_stride;
      STORE_DWORD(dst, out1);
      dst += dst_stride;
      STORE_DWORD(dst, out2);
      dst += dst_stride;
      STORE_DWORD(dst, out3);
      dst += dst_stride;
    }
  } else if (0 == height % 8) {
    for (cnt = height >> 3; cnt--;) {
      LOAD_8VECS_UB(src, src_stride,
                    src0, src1, src2, src3, src4, src5, src6, src7);
      src += (8 * src_stride);

      out0 = __msa_copy_u_d((v2i64)src0, 0);
      out1 = __msa_copy_u_d((v2i64)src1, 0);
      out2 = __msa_copy_u_d((v2i64)src2, 0);
      out3 = __msa_copy_u_d((v2i64)src3, 0);
      out4 = __msa_copy_u_d((v2i64)src4, 0);
      out5 = __msa_copy_u_d((v2i64)src5, 0);
      out6 = __msa_copy_u_d((v2i64)src6, 0);
      out7 = __msa_copy_u_d((v2i64)src7, 0);

      STORE_DWORD(dst, out0);
      dst += dst_stride;
      STORE_DWORD(dst, out1);
      dst += dst_stride;
      STORE_DWORD(dst, out2);
      dst += dst_stride;
      STORE_DWORD(dst, out3);
      dst += dst_stride;
      STORE_DWORD(dst, out4);
      dst += dst_stride;
      STORE_DWORD(dst, out5);
      dst += dst_stride;
      STORE_DWORD(dst, out6);
      dst += dst_stride;
      STORE_DWORD(dst, out7);
      dst += dst_stride;
    }
  } else if (0 == height % 4) {
    for (cnt = (height / 4); cnt--;) {
      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      src += (4 * src_stride);

      out0 = __msa_copy_u_d((v2i64)src0, 0);
      out1 = __msa_copy_u_d((v2i64)src1, 0);
      out2 = __msa_copy_u_d((v2i64)src2, 0);
      out3 = __msa_copy_u_d((v2i64)src3, 0);

      STORE_DWORD(dst, out0);
      dst += dst_stride;
      STORE_DWORD(dst, out1);
      dst += dst_stride;
      STORE_DWORD(dst, out2);
      dst += dst_stride;
      STORE_DWORD(dst, out3);
      dst += dst_stride;
    }
  } else if (0 == height % 2) {
    for (cnt = (height / 2); cnt--;) {
      LOAD_2VECS_UB(src, src_stride, src0, src1);
      src += (2 * src_stride);

      out0 = __msa_copy_u_d((v2i64)src0, 0);
      out1 = __msa_copy_u_d((v2i64)src1, 0);

      STORE_DWORD(dst, out0);
      dst += dst_stride;
      STORE_DWORD(dst, out1);
      dst += dst_stride;
    }
  }
}

static void copy_16multx8mult_msa(const uint8_t *src, int32_t src_stride,
                                  uint8_t *dst, int32_t dst_stride,
                                  int32_t height, int32_t width) {
  int32_t cnt, loop_cnt;
  const uint8_t *src_tmp;
  uint8_t *dst_tmp;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

  for (cnt = (width >> 4); cnt--;) {
    src_tmp = src;
    dst_tmp = dst;

    for (loop_cnt = (height >> 3); loop_cnt--;) {
      LOAD_8VECS_UB(src_tmp, src_stride,
                    src0, src1, src2, src3, src4, src5, src6, src7);
      src_tmp += (8 * src_stride);

      STORE_8VECS_UB(dst_tmp, dst_stride,
                     src0, src1, src2, src3, src4, src5, src6, src7);
      dst_tmp += (8 * dst_stride);
    }

    src += 16;
    dst += 16;
  }
}

static void copy_width16_msa(const uint8_t *src, int32_t src_stride,
                             uint8_t *dst, int32_t dst_stride,
                             int32_t height) {
  int32_t cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

  if (0 == height % 12) {
    for (cnt = (height / 12); cnt--;) {
      LOAD_8VECS_UB(src, src_stride,
                    src0, src1, src2, src3, src4, src5, src6, src7);
      src += (8 * src_stride);

      STORE_8VECS_UB(dst, dst_stride,
                     src0, src1, src2, src3, src4, src5, src6, src7);
      dst += (8 * dst_stride);

      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      dst += (4 * dst_stride);
    }
  } else if (0 == height % 8) {
    copy_16multx8mult_msa(src, src_stride, dst, dst_stride, height, 16);
  } else if (0 == height % 4) {
    for (cnt = (height >> 2); cnt--;) {
      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      dst += (4 * dst_stride);
    }
  }
}

static void copy_width32_msa(const uint8_t *src, int32_t src_stride,
                             uint8_t *dst, int32_t dst_stride,
                             int32_t height) {
  int32_t cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;

  if (0 == height % 12) {
    for (cnt = (height / 12); cnt--;) {
      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      LOAD_4VECS_UB(src + 16, src_stride, src4, src5, src6, src7);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      STORE_4VECS_UB(dst + 16, dst_stride, src4, src5, src6, src7);
      dst += (4 * dst_stride);

      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      LOAD_4VECS_UB(src + 16, src_stride, src4, src5, src6, src7);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      STORE_4VECS_UB(dst + 16, dst_stride, src4, src5, src6, src7);
      dst += (4 * dst_stride);

      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      LOAD_4VECS_UB(src + 16, src_stride, src4, src5, src6, src7);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      STORE_4VECS_UB(dst + 16, dst_stride, src4, src5, src6, src7);
      dst += (4 * dst_stride);
    }
  } else if (0 == height % 8) {
    copy_16multx8mult_msa(src, src_stride, dst, dst_stride, height, 32);
  } else if (0 == height % 4) {
    for (cnt = (height >> 2); cnt--;) {
      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      LOAD_4VECS_UB(src + 16, src_stride, src4, src5, src6, src7);
      src += (4 * src_stride);

      STORE_4VECS_UB(dst, dst_stride, src0, src1, src2, src3);
      STORE_4VECS_UB(dst + 16, dst_stride, src4, src5, src6, src7);
      dst += (4 * dst_stride);
    }
  }
}

static void copy_width64_msa(const uint8_t *src, int32_t src_stride,
                             uint8_t *dst, int32_t dst_stride,
                             int32_t height) {
  copy_16multx8mult_msa(src, src_stride, dst, dst_stride, height, 64);
}

static void avg_width4_msa(const uint8_t *src, int32_t src_stride,
                           uint8_t *dst, int32_t dst_stride,
                           int32_t height) {
  int32_t cnt;
  uint32_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;

  if (0 == (height % 4)) {
    for (cnt = (height / 4); cnt--;) {
      LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
      src += (4 * src_stride);

      LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

      dst0 = __msa_aver_u_b(src0, dst0);
      dst1 = __msa_aver_u_b(src1, dst1);
      dst2 = __msa_aver_u_b(src2, dst2);
      dst3 = __msa_aver_u_b(src3, dst3);

      out0 = __msa_copy_u_w((v4i32)dst0, 0);
      out1 = __msa_copy_u_w((v4i32)dst1, 0);
      out2 = __msa_copy_u_w((v4i32)dst2, 0);
      out3 = __msa_copy_u_w((v4i32)dst3, 0);

      STORE_WORD(dst, out0);
      dst += dst_stride;
      STORE_WORD(dst, out1);
      dst += dst_stride;
      STORE_WORD(dst, out2);
      dst += dst_stride;
      STORE_WORD(dst, out3);
      dst += dst_stride;
    }
  } else if (0 == (height % 2)) {
    for (cnt = (height / 2); cnt--;) {
      LOAD_2VECS_UB(src, src_stride, src0, src1);
      src += (2 * src_stride);

      LOAD_2VECS_UB(dst, dst_stride, dst0, dst1);

      dst0 = __msa_aver_u_b(src0, dst0);
      dst1 = __msa_aver_u_b(src1, dst1);

      out0 = __msa_copy_u_w((v4i32)dst0, 0);
      out1 = __msa_copy_u_w((v4i32)dst1, 0);

      STORE_WORD(dst, out0);
      dst += dst_stride;
      STORE_WORD(dst, out1);
      dst += dst_stride;
    }
  }
}

static void avg_width8_msa(const uint8_t *src, int32_t src_stride,
                           uint8_t *dst, int32_t dst_stride,
                           int32_t height) {
  int32_t cnt;
  uint64_t out0, out1, out2, out3;
  v16u8 src0, src1, src2, src3;
  v16u8 dst0, dst1, dst2, dst3;

  for (cnt = (height / 4); cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    dst0 = __msa_aver_u_b(src0, dst0);
    dst1 = __msa_aver_u_b(src1, dst1);
    dst2 = __msa_aver_u_b(src2, dst2);
    dst3 = __msa_aver_u_b(src3, dst3);

    out0 = __msa_copy_u_d((v2i64)dst0, 0);
    out1 = __msa_copy_u_d((v2i64)dst1, 0);
    out2 = __msa_copy_u_d((v2i64)dst2, 0);
    out3 = __msa_copy_u_d((v2i64)dst3, 0);

    STORE_DWORD(dst, out0);
    dst += dst_stride;
    STORE_DWORD(dst, out1);
    dst += dst_stride;
    STORE_DWORD(dst, out2);
    dst += dst_stride;
    STORE_DWORD(dst, out3);
    dst += dst_stride;
  }
}

static void avg_width16_msa(const uint8_t *src, int32_t src_stride,
                            uint8_t *dst, int32_t dst_stride,
                            int32_t height) {
  int32_t cnt;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;

  for (cnt = (height / 8); cnt--;) {
    LOAD_4VECS_UB(src, src_stride, src0, src1, src2, src3);
    src += (4 * src_stride);

    LOAD_4VECS_UB(src, src_stride, src4, src5, src6, src7);
    src += (4 * src_stride);

    LOAD_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);

    LOAD_4VECS_UB(dst + (4 * dst_stride), dst_stride, dst4, dst5, dst6, dst7);

    dst0 = __msa_aver_u_b(src0, dst0);
    dst1 = __msa_aver_u_b(src1, dst1);
    dst2 = __msa_aver_u_b(src2, dst2);
    dst3 = __msa_aver_u_b(src3, dst3);
    dst4 = __msa_aver_u_b(src4, dst4);
    dst5 = __msa_aver_u_b(src5, dst5);
    dst6 = __msa_aver_u_b(src6, dst6);
    dst7 = __msa_aver_u_b(src7, dst7);

    STORE_4VECS_UB(dst, dst_stride, dst0, dst1, dst2, dst3);
    dst += (4 * dst_stride);

    STORE_4VECS_UB(dst, dst_stride, dst4, dst5, dst6, dst7);
    dst += (4 * dst_stride);
  }
}

static void avg_width32_msa(const uint8_t *src, int32_t src_stride,
                            uint8_t *dst, int32_t dst_stride,
                            int32_t height) {
  int32_t cnt;
  uint8_t *dst_dup = dst;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 src8, src9, src10, src11, src12, src13, src14, src15;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v16u8 dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15;

  for (cnt = (height / 8); cnt--;) {
    src0 = LOAD_UB(src);
    src1 = LOAD_UB(src + 16);
    src += src_stride;
    src2 = LOAD_UB(src);
    src3 = LOAD_UB(src + 16);
    src += src_stride;
    src4 = LOAD_UB(src);
    src5 = LOAD_UB(src + 16);
    src += src_stride;
    src6 = LOAD_UB(src);
    src7 = LOAD_UB(src + 16);
    src += src_stride;

    dst0 = LOAD_UB(dst_dup);
    dst1 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst2 = LOAD_UB(dst_dup);
    dst3 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst4 = LOAD_UB(dst_dup);
    dst5 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst6 = LOAD_UB(dst_dup);
    dst7 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;

    src8 = LOAD_UB(src);
    src9 = LOAD_UB(src + 16);
    src += src_stride;
    src10 = LOAD_UB(src);
    src11 = LOAD_UB(src + 16);
    src += src_stride;
    src12 = LOAD_UB(src);
    src13 = LOAD_UB(src + 16);
    src += src_stride;
    src14 = LOAD_UB(src);
    src15 = LOAD_UB(src + 16);
    src += src_stride;

    dst8 = LOAD_UB(dst_dup);
    dst9 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst10 = LOAD_UB(dst_dup);
    dst11 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst12 = LOAD_UB(dst_dup);
    dst13 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;
    dst14 = LOAD_UB(dst_dup);
    dst15 = LOAD_UB(dst_dup + 16);
    dst_dup += dst_stride;

    dst0 = __msa_aver_u_b(src0, dst0);
    dst1 = __msa_aver_u_b(src1, dst1);
    dst2 = __msa_aver_u_b(src2, dst2);
    dst3 = __msa_aver_u_b(src3, dst3);
    dst4 = __msa_aver_u_b(src4, dst4);
    dst5 = __msa_aver_u_b(src5, dst5);
    dst6 = __msa_aver_u_b(src6, dst6);
    dst7 = __msa_aver_u_b(src7, dst7);
    dst8 = __msa_aver_u_b(src8, dst8);
    dst9 = __msa_aver_u_b(src9, dst9);
    dst10 = __msa_aver_u_b(src10, dst10);
    dst11 = __msa_aver_u_b(src11, dst11);
    dst12 = __msa_aver_u_b(src12, dst12);
    dst13 = __msa_aver_u_b(src13, dst13);
    dst14 = __msa_aver_u_b(src14, dst14);
    dst15 = __msa_aver_u_b(src15, dst15);

    STORE_UB(dst0, dst);
    STORE_UB(dst1, dst + 16);
    dst += dst_stride;
    STORE_UB(dst2, dst);
    STORE_UB(dst3, dst + 16);
    dst += dst_stride;
    STORE_UB(dst4, dst);
    STORE_UB(dst5, dst + 16);
    dst += dst_stride;
    STORE_UB(dst6, dst);
    STORE_UB(dst7, dst + 16);
    dst += dst_stride;
    STORE_UB(dst8, dst);
    STORE_UB(dst9, dst + 16);
    dst += dst_stride;
    STORE_UB(dst10, dst);
    STORE_UB(dst11, dst + 16);
    dst += dst_stride;
    STORE_UB(dst12, dst);
    STORE_UB(dst13, dst + 16);
    dst += dst_stride;
    STORE_UB(dst14, dst);
    STORE_UB(dst15, dst + 16);
    dst += dst_stride;
  }
}

static void avg_width64_msa(const uint8_t *src, int32_t src_stride,
                            uint8_t *dst, int32_t dst_stride,
                            int32_t height) {
  int32_t cnt;
  uint8_t *dst_dup = dst;
  v16u8 src0, src1, src2, src3, src4, src5, src6, src7;
  v16u8 src8, src9, src10, src11, src12, src13, src14, src15;
  v16u8 dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
  v16u8 dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15;

  for (cnt = (height / 4); cnt--;) {
    src0 = LOAD_UB(src);
    src1 = LOAD_UB(src + 16);
    src2 = LOAD_UB(src + 32);
    src3 = LOAD_UB(src + 48);
    src += src_stride;
    src4 = LOAD_UB(src);
    src5 = LOAD_UB(src + 16);
    src6 = LOAD_UB(src + 32);
    src7 = LOAD_UB(src + 48);
    src += src_stride;
    src8 = LOAD_UB(src);
    src9 = LOAD_UB(src + 16);
    src10 = LOAD_UB(src + 32);
    src11 = LOAD_UB(src + 48);
    src += src_stride;
    src12 = LOAD_UB(src);
    src13 = LOAD_UB(src + 16);
    src14 = LOAD_UB(src + 32);
    src15 = LOAD_UB(src + 48);
    src += src_stride;

    dst0 = LOAD_UB(dst_dup);
    dst1 = LOAD_UB(dst_dup + 16);
    dst2 = LOAD_UB(dst_dup + 32);
    dst3 = LOAD_UB(dst_dup + 48);
    dst_dup += dst_stride;
    dst4 = LOAD_UB(dst_dup);
    dst5 = LOAD_UB(dst_dup + 16);
    dst6 = LOAD_UB(dst_dup + 32);
    dst7 = LOAD_UB(dst_dup + 48);
    dst_dup += dst_stride;
    dst8 = LOAD_UB(dst_dup);
    dst9 = LOAD_UB(dst_dup + 16);
    dst10 = LOAD_UB(dst_dup + 32);
    dst11 = LOAD_UB(dst_dup + 48);
    dst_dup += dst_stride;
    dst12 = LOAD_UB(dst_dup);
    dst13 = LOAD_UB(dst_dup + 16);
    dst14 = LOAD_UB(dst_dup + 32);
    dst15 = LOAD_UB(dst_dup + 48);
    dst_dup += dst_stride;

    dst0 = __msa_aver_u_b(src0, dst0);
    dst1 = __msa_aver_u_b(src1, dst1);
    dst2 = __msa_aver_u_b(src2, dst2);
    dst3 = __msa_aver_u_b(src3, dst3);
    dst4 = __msa_aver_u_b(src4, dst4);
    dst5 = __msa_aver_u_b(src5, dst5);
    dst6 = __msa_aver_u_b(src6, dst6);
    dst7 = __msa_aver_u_b(src7, dst7);
    dst8 = __msa_aver_u_b(src8, dst8);
    dst9 = __msa_aver_u_b(src9, dst9);
    dst10 = __msa_aver_u_b(src10, dst10);
    dst11 = __msa_aver_u_b(src11, dst11);
    dst12 = __msa_aver_u_b(src12, dst12);
    dst13 = __msa_aver_u_b(src13, dst13);
    dst14 = __msa_aver_u_b(src14, dst14);
    dst15 = __msa_aver_u_b(src15, dst15);

    STORE_UB(dst0, dst);
    STORE_UB(dst1, dst + 16);
    STORE_UB(dst2, dst + 32);
    STORE_UB(dst3, dst + 48);
    dst += dst_stride;
    STORE_UB(dst4, dst);
    STORE_UB(dst5, dst + 16);
    STORE_UB(dst6, dst + 32);
    STORE_UB(dst7, dst + 48);
    dst += dst_stride;
    STORE_UB(dst8, dst);
    STORE_UB(dst9, dst + 16);
    STORE_UB(dst10, dst + 32);
    STORE_UB(dst11, dst + 48);
    dst += dst_stride;
    STORE_UB(dst12, dst);
    STORE_UB(dst13, dst + 16);
    STORE_UB(dst14, dst + 32);
    STORE_UB(dst15, dst + 48);
    dst += dst_stride;
  }
}

void vp9_convolve8_horiz_msa(const uint8_t *src, ptrdiff_t src_stride,
                             uint8_t *dst, ptrdiff_t dst_stride,
                             const int16_t *filter_x, int x_step_q4,
                             const int16_t *filter_y, int y_step_q4,
                             int w, int h) {
  int8_t cnt, filt_hor[16];

  if (16 != x_step_q4) {
    return vp9_convolve8_horiz_c(src, src_stride, dst, dst_stride,
                                 filter_x, x_step_q4, filter_y, y_step_q4,
                                 w, h);
  }

  if (((const int32_t *)filter_x)[1] == 0x800000) {
    return vp9_convolve_copy(src, src_stride, dst, dst_stride,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_hor[cnt] = filter_x[cnt];
  }

  if (((const int32_t *)filter_x)[0] == 0) {
    switch (w) {
      case 4:
        common_hz_2t_4w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            &filt_hor[3], h);
        break;
      case 8:
        common_hz_2t_8w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            &filt_hor[3], h);
        break;
      case 16:
        common_hz_2t_16w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_hor[3], h);
        break;
      case 32:
        common_hz_2t_32w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_hor[3], h);
        break;
      case 64:
        common_hz_2t_64w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_hor[3], h);
        break;
      default:
        vp9_convolve8_horiz_c(src, src_stride, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h);
        break;
    }
  } else {
    switch (w) {
      case 4:
        common_hz_8t_4w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            filt_hor, h);
        break;
      case 8:
        common_hz_8t_8w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            filt_hor, h);
        break;
      case 16:
        common_hz_8t_16w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             filt_hor, h);
        break;
      case 32:
        common_hz_8t_32w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             filt_hor, h);
        break;
      case 64:
        common_hz_8t_64w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             filt_hor, h);
        break;
      default:
        vp9_convolve8_horiz_c(src, src_stride, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h);
        break;
    }
  }
}

void vp9_convolve8_vert_msa(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int x_step_q4,
                            const int16_t *filter_y, int y_step_q4,
                            int w, int h) {
  int8_t cnt, filt_ver[16];

  if (16 != y_step_q4) {
    return vp9_convolve8_vert_c(src, src_stride, dst, dst_stride,
                                filter_x, x_step_q4, filter_y, y_step_q4,
                                w, h);
  }

  if (((const int32_t *)filter_y)[1] == 0x800000) {
    return vp9_convolve_copy(src, src_stride, dst, dst_stride,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_ver[cnt] = filter_y[cnt];
  }

  if (((const int32_t *)filter_y)[0] == 0) {
    switch (w) {
      case 4:
        common_vt_2t_4w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            &filt_ver[3], h);
        break;
      case 8:
        common_vt_2t_8w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            &filt_ver[3], h);
        break;
      case 16:
        common_vt_2t_16w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_ver[3], h);
        break;
      case 32:
        common_vt_2t_32w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_ver[3], h);
        break;
      case 64:
        common_vt_2t_64w_msa(src, (int32_t)src_stride,
                             dst, (int32_t)dst_stride,
                             &filt_ver[3], h);
        break;
      default:
        vp9_convolve8_vert_c(src, src_stride, dst, dst_stride,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, h);
        break;
    }
  } else {
    switch (w) {
      case 4:
        common_vt_8t_4w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            filt_ver, h);
        break;
      case 8:
        common_vt_8t_8w_msa(src, (int32_t)src_stride,
                            dst, (int32_t)dst_stride,
                            filt_ver, h);
        break;
      case 16:
        common_vt_8t_16w_mult_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_ver, h, 16);
        break;
      case 32:
        common_vt_8t_16w_mult_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_ver, h, 32);
        break;
      case 64:
        common_vt_8t_16w_mult_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_ver, h, 64);
        break;
      default:
        vp9_convolve8_vert_c(src, src_stride, dst, dst_stride,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, h);
        break;
    }
  }
}

void vp9_convolve8_msa(const uint8_t *src, ptrdiff_t src_stride,
                       uint8_t *dst, ptrdiff_t dst_stride,
                       const int16_t *filter_x, int32_t x_step_q4,
                       const int16_t *filter_y, int32_t y_step_q4,
                       int32_t w, int32_t h) {
  int8_t cnt;
  int8_t filt_hor[16];
  int8_t filt_ver[16];

  if (16 != x_step_q4 || 16 != y_step_q4) {
    return vp9_convolve8_c(src, src_stride, dst, dst_stride,
                           filter_x, x_step_q4, filter_y, y_step_q4,
                           w, h);
  }

  if (((const int32_t *)filter_x)[1] == 0x800000 &&
      ((const int32_t *)filter_y)[1] == 0x800000) {
    return vp9_convolve_copy(src, src_stride, dst, dst_stride,
                             filter_x, x_step_q4, filter_y, y_step_q4,
                             w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_hor[cnt] = filter_x[cnt];
    filt_ver[cnt] = filter_y[cnt];
  }

  if (((const int32_t *)filter_x)[0] == 0 &&
      ((const int32_t *)filter_y)[0] == 0) {
    switch (w) {
      case 4:
        common_hv_2ht_2vt_4w_msa(src, (int32_t)src_stride,
                                 dst, (int32_t)dst_stride,
                                 &filt_hor[3], &filt_ver[3],
                                 (int32_t)h);
        break;
      case 8:
        common_hv_2ht_2vt_8w_msa(src, (int32_t)src_stride,
                                 dst, (int32_t)dst_stride,
                                 &filt_hor[3], &filt_ver[3],
                                 (int32_t)h);
        break;
      case 16:
        common_hv_2ht_2vt_16w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  &filt_hor[3], &filt_ver[3],
                                  (int32_t)h);
        break;
      case 32:
        common_hv_2ht_2vt_32w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  &filt_hor[3], &filt_ver[3],
                                  (int32_t)h);
        break;
      case 64:
        common_hv_2ht_2vt_64w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  &filt_hor[3], &filt_ver[3],
                                  (int32_t)h);
        break;
      default:
        vp9_convolve8_c(src, src_stride, dst, dst_stride,
                        filter_x, x_step_q4, filter_y, y_step_q4,
                        w, h);
        break;
    }
  } else if (((const int32_t *)filter_x)[0] == 0 ||
             ((const int32_t *)filter_y)[0] == 0) {
    vp9_convolve8_c(src, src_stride, dst, dst_stride,
                    filter_x, x_step_q4, filter_y, y_step_q4,
                    w, h);
  } else {
    switch (w) {
      case 4:
        common_hv_8ht_8vt_4w_msa(src, (int32_t)src_stride,
                                 dst, (int32_t)dst_stride,
                                 filt_hor, filt_ver,
                                 (int32_t)h);
        break;
      case 8:
        common_hv_8ht_8vt_8w_msa(src, (int32_t)src_stride,
                                 dst, (int32_t)dst_stride,
                                 filt_hor, filt_ver,
                                 (int32_t)h);
        break;
      case 16:
        common_hv_8ht_8vt_16w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_hor, filt_ver,
                                  (int32_t)h);
        break;
      case 32:
        common_hv_8ht_8vt_32w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_hor, filt_ver,
                                  (int32_t)h);
        break;
      case 64:
        common_hv_8ht_8vt_64w_msa(src, (int32_t)src_stride,
                                  dst, (int32_t)dst_stride,
                                  filt_hor, filt_ver,
                                  (int32_t)h);
        break;
      default:
        vp9_convolve8_c(src, src_stride, dst, dst_stride,
                        filter_x, x_step_q4, filter_y, y_step_q4,
                        w, h);
        break;
    }
  }
}

void vp9_convolve8_avg_horiz_msa(const uint8_t *src, ptrdiff_t src_stride,
                                 uint8_t *dst, ptrdiff_t dst_stride,
                                 const int16_t *filter_x, int x_step_q4,
                                 const int16_t *filter_y, int y_step_q4,
                                 int w, int h) {
  int8_t cnt, filt_hor[16];

  if (16 != x_step_q4) {
    return vp9_convolve8_avg_horiz_c(src, src_stride, dst, dst_stride,
                                     filter_x, x_step_q4, filter_y, y_step_q4,
                                     w, h);
  }

  if (((const int32_t *)filter_x)[1] == 0x800000) {
    return vp9_convolve_avg(src, src_stride, dst, dst_stride,
                            filter_x, x_step_q4, filter_y, y_step_q4,
                            w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_hor[cnt] = filter_x[cnt];
  }

  if (((const int32_t *)filter_x)[0] == 0) {
    switch (w) {
      case 4:
        common_hz_2t_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         &filt_hor[3], h);
        break;
      case 8:
        common_hz_2t_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         &filt_hor[3], h);
        break;
      case 16:
        common_hz_2t_and_aver_dst_16w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_hor[3], h);
        break;
      case 32:
        common_hz_2t_and_aver_dst_32w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_hor[3], h);
        break;
      case 64:
        common_hz_2t_and_aver_dst_64w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_hor[3], h);
        break;
      default:
        vp9_convolve8_avg_horiz_c(src, src_stride, dst, dst_stride,
                                  filter_x, x_step_q4, filter_y, y_step_q4,
                                  w, h);
        break;
    }
  } else {
    switch (w) {
      case 4:
        common_hz_8t_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         filt_hor, h);
        break;
      case 8:
        common_hz_8t_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         filt_hor, h);
        break;
      case 16:
        common_hz_8t_and_aver_dst_16w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          filt_hor, h);
        break;
      case 32:
        common_hz_8t_and_aver_dst_32w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          filt_hor, h);
        break;
      case 64:
        common_hz_8t_and_aver_dst_64w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          filt_hor, h);
        break;
      default:
        vp9_convolve8_avg_horiz_c(src, src_stride, dst, dst_stride,
                                  filter_x, x_step_q4, filter_y, y_step_q4,
                                  w, h);
        break;
    }
  }
}

void vp9_convolve8_avg_vert_msa(const uint8_t *src, ptrdiff_t src_stride,
                                uint8_t *dst, ptrdiff_t dst_stride,
                                const int16_t *filter_x, int x_step_q4,
                                const int16_t *filter_y, int y_step_q4,
                                int w, int h) {
  int8_t cnt, filt_ver[8];

  if (16 != y_step_q4) {
    return vp9_convolve8_avg_vert_c(src, src_stride, dst, dst_stride,
                                    filter_x, x_step_q4, filter_y, y_step_q4,
                                    w, h);
  }

  if (((const int32_t *)filter_y)[1] == 0x800000) {
    return vp9_convolve_avg(src, src_stride, dst, dst_stride,
                            filter_x, x_step_q4, filter_y, y_step_q4,
                            w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_ver[cnt] = filter_y[cnt];
  }

  if (((const int32_t *)filter_y)[0] == 0) {
    switch (w) {
      case 4:
        common_vt_2t_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         &filt_ver[3], h);
        break;
      case 8:
        common_vt_2t_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         &filt_ver[3], h);
        break;
      case 16:
        common_vt_2t_and_aver_dst_16w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_ver[3], h);
        break;
      case 32:
        common_vt_2t_and_aver_dst_32w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_ver[3], h);
        break;
      case 64:
        common_vt_2t_and_aver_dst_64w_msa(src, (int32_t)src_stride,
                                          dst, (int32_t)dst_stride,
                                          &filt_ver[3], h);
        break;
      default:
        vp9_convolve8_avg_vert_c(src, src_stride, dst, dst_stride,
                                 filter_x, x_step_q4, filter_y, y_step_q4,
                                 w, h);
        break;
    }
  } else {
    switch (w) {
      case 4:
        common_vt_8t_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         filt_ver, h);
        break;
      case 8:
        common_vt_8t_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                         dst, (int32_t)dst_stride,
                                         filt_ver, h);
        break;
      case 16:
        common_vt_8t_and_aver_dst_16w_mult_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_ver, h, 16);

        break;
      case 32:
        common_vt_8t_and_aver_dst_16w_mult_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_ver, h, 32);
        break;
      case 64:
        common_vt_8t_and_aver_dst_16w_mult_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_ver, h, 64);
        break;
      default:
        vp9_convolve8_avg_vert_c(src, src_stride, dst, dst_stride,
                                 filter_x, x_step_q4, filter_y, y_step_q4,
                                 w, h);
        break;
    }
  }
}

void vp9_convolve8_avg_msa(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const int16_t *filter_x, int x_step_q4,
                           const int16_t *filter_y, int y_step_q4,
                           int w, int h) {
  int8_t cnt, filt_hor[8], filt_ver[8];

  if (16 != x_step_q4 || 16 != y_step_q4) {
    return vp9_convolve8_avg_c(src, src_stride, dst, dst_stride,
                               filter_x, x_step_q4, filter_y, y_step_q4,
                               w, h);
  }

  if (((const int32_t *)filter_x)[1] == 0x800000 &&
      ((const int32_t *)filter_y)[1] == 0x800000) {
    return vp9_convolve_avg(src, src_stride, dst, dst_stride,
                            filter_x, x_step_q4, filter_y, y_step_q4,
                            w, h);
  }

  for (cnt = 0; cnt < 8; cnt++) {
    filt_hor[cnt] = filter_x[cnt];
    filt_ver[cnt] = filter_y[cnt];
  }

  if (((const int32_t *)filter_x)[0] == 0 &&
      ((const int32_t *)filter_y)[0] == 0) {
    switch (w) {
      case 4:
        common_hv_2ht_2vt_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                              dst, (int32_t)dst_stride,
                                              &filt_hor[3], &filt_ver[3], h);
        break;
      case 8:
        common_hv_2ht_2vt_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                              dst, (int32_t)dst_stride,
                                              &filt_hor[3], &filt_ver[3], h);
        break;
      case 16:
        common_hv_2ht_2vt_and_aver_dst_16w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               &filt_hor[3], &filt_ver[3], h);
        break;
      case 32:
        common_hv_2ht_2vt_and_aver_dst_32w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               &filt_hor[3], &filt_ver[3], h);
        break;
      case 64:
        common_hv_2ht_2vt_and_aver_dst_64w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               &filt_hor[3], &filt_ver[3], h);
        break;
      default:
        vp9_convolve8_avg_c(src, src_stride, dst, dst_stride,
                            filter_x, x_step_q4, filter_y, y_step_q4,
                            w, h);
        break;
    }
  } else if (((const int32_t *)filter_x)[0] == 0 ||
             ((const int32_t *)filter_y)[0] == 0) {
    vp9_convolve8_avg_c(src, src_stride, dst, dst_stride,
                        filter_x, x_step_q4, filter_y, y_step_q4,
                        w, h);
  } else {
    switch (w) {
      case 4:
        common_hv_8ht_8vt_and_aver_dst_4w_msa(src, (int32_t)src_stride,
                                              dst, (int32_t)dst_stride,
                                              filt_hor, filt_ver, h);
        break;
      case 8:
        common_hv_8ht_8vt_and_aver_dst_8w_msa(src, (int32_t)src_stride,
                                              dst, (int32_t)dst_stride,
                                              filt_hor, filt_ver, h);
        break;
      case 16:
        common_hv_8ht_8vt_and_aver_dst_16w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_hor, filt_ver, h);
        break;
      case 32:
        common_hv_8ht_8vt_and_aver_dst_32w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_hor, filt_ver, h);
        break;
      case 64:
        common_hv_8ht_8vt_and_aver_dst_64w_msa(src, (int32_t)src_stride,
                                               dst, (int32_t)dst_stride,
                                               filt_hor, filt_ver, h);
        break;
      default:
        vp9_convolve8_avg_c(src, src_stride, dst, dst_stride,
                            filter_x, x_step_q4, filter_y, y_step_q4,
                            w, h);
        break;
    }
  }
}

void vp9_convolve_copy_msa(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const int16_t *filter_x, int32_t filter_x_stride,
                           const int16_t *filter_y, int32_t filter_y_stride,
                           int32_t w, int32_t h) {
  int32_t cnt;
  (void)filter_x;  (void)filter_x_stride;
  (void)filter_y;  (void)filter_y_stride;

  switch (w) {
    case 4: {
      uint32_t tmp;

      /* 1 word storage */
      for (cnt = h; cnt--;) {
        tmp = LOAD_WORD(src);
        STORE_WORD(dst, tmp);

        src += src_stride;
        dst += dst_stride;
      }
    }
    break;
    case 8: {
      copy_width8_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 16: {
      copy_width16_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 32: {
      copy_width32_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 64: {
      copy_width64_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    default: {
      int32_t cnt;

      for (cnt = h; cnt--;) {
        vpx_memcpy(dst, src, w);
        src += src_stride;
        dst += dst_stride;
      }
    }
    break;
  }
}

void vp9_convolve_avg_msa(const uint8_t *src, ptrdiff_t src_stride,
                          uint8_t *dst, ptrdiff_t dst_stride,
                          const int16_t *filter_x, int32_t filter_x_stride,
                          const int16_t *filter_y, int32_t filter_y_stride,
                          int32_t w, int32_t h) {
  (void)filter_x;  (void)filter_x_stride;
  (void)filter_y;  (void)filter_y_stride;

  switch (w) {
    case 4: {
      avg_width4_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 8: {
      avg_width8_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 16: {
      avg_width16_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 32: {
      avg_width32_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    case 64: {
      avg_width64_msa(src, src_stride, dst, dst_stride, h);
    }
    break;
    default: {
      int32_t lp, cnt;

      for (cnt = h; cnt--;) {
        for (lp = 0; lp < w; ++lp) {
          dst[lp] = (((dst[lp] + src[lp]) + 1) >> 1);
        }

        src += src_stride;
        dst += dst_stride;
      }
    }
    break;
  }
}
#endif  /* HAVE_MSA */
