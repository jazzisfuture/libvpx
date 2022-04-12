/*
 *  Copyright (c) 2022 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_util/loongson_intrinsics.h"

#if !CONFIG_VP9_HIGHBITDEPTH
void vpx_hadamard_8x8_lsx(const int16_t *src, ptrdiff_t src_stride,
                          int16_t *dst) {
  __m128i src0, src1, src2, src3, src4, src5, src6, src7;
  __m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  ptrdiff_t src_stride2 = src_stride << 1;
  ptrdiff_t src_stride3 = src_stride2 + src_stride;
  ptrdiff_t src_stride4 = src_stride2 << 1;
  ptrdiff_t src_stride6 = src_stride3 << 1;

  int16_t *src_tmp = (int16_t *)src;
  src0 = __lsx_vld(src_tmp, 0);
  DUP2_ARG2(__lsx_vldx, src_tmp, src_stride2, src_tmp, src_stride4, src1, src2);
  src3 = __lsx_vldx(src_tmp, src_stride6);
  src_tmp += src_stride4;
  src4 = __lsx_vld(src_tmp, 0);
  DUP2_ARG2(__lsx_vldx, src_tmp, src_stride2, src_tmp, src_stride4, src5, src6);
  src7 = __lsx_vldx(src_tmp, src_stride6);

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src3, src4, src5, src6, src7);
  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src3, src4, src5, src6, src7);
  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 16);
  __lsx_vst(src2, dst, 32);
  __lsx_vst(src3, dst, 48);
  __lsx_vst(src4, dst, 64);
  __lsx_vst(src5, dst, 80);
  __lsx_vst(src6, dst, 96);
  __lsx_vst(src7, dst, 112);
}

void vpx_hadamard_16x16_lsx(const int16_t *src, ptrdiff_t src_stride,
                            int16_t *dst) {
  __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
  __m128i src11, src12, src13, src14, src15, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
  __m128i tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15;
  __m128i res0, res1, res2, res3, res4, res5, res6, res7;

  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src8);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src1, src9);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src2, src10);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src3, src11);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src4, src12);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src5, src13);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src6, src14);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src7, src15);
  src += src_stride;

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(src8, src10, src12, src14, src15, src13, src11, src9, tmp8,
                    tmp10, tmp12, tmp14, tmp15, tmp13, tmp11, tmp9);

  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src3, src4, src5, src6, src7);
  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src11, src4, src5, src6, src7);

  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 16);
  __lsx_vst(src2, dst, 32);
  __lsx_vst(src11, dst, 48);
  __lsx_vst(src4, dst, 64);
  __lsx_vst(src5, dst, 80);
  __lsx_vst(src6, dst, 96);
  __lsx_vst(src7, dst, 112);

  LSX_BUTTERFLY_8_H(tmp8, tmp9, tmp12, tmp13, tmp15, tmp14, tmp11, tmp10, src8,
                    src9, src12, src13, src15, src14, src11, src10);
  LSX_BUTTERFLY_8_H(src8, src9, src10, src11, src15, src14, src13, src12, tmp8,
                    tmp15, tmp11, tmp12, tmp13, tmp9, tmp14, tmp10);
  LSX_TRANSPOSE8x8_H(tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, src8,
                     src9, src10, src11, src12, src13, src14, src15);
  LSX_BUTTERFLY_8_H(src8, src10, src12, src14, src15, src13, src11, src9, tmp8,
                    tmp10, tmp12, tmp14, tmp15, tmp13, tmp11, tmp9);
  LSX_BUTTERFLY_8_H(tmp8, tmp9, tmp12, tmp13, tmp15, tmp14, tmp11, tmp10, src8,
                    src9, src12, src13, src15, src14, src11, src10);
  LSX_BUTTERFLY_8_H(src8, src9, src10, src11, src15, src14, src13, src12, tmp8,
                    tmp15, tmp11, tmp12, tmp13, tmp9, tmp14, tmp10);
  LSX_TRANSPOSE8x8_H(tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, res0,
                     res1, res2, res3, res4, res5, res6, res7);

  __lsx_vst(res0, dst, 128);
  __lsx_vst(res1, dst, 144);
  __lsx_vst(res2, dst, 160);
  __lsx_vst(res3, dst, 176);
  __lsx_vst(res4, dst, 192);
  __lsx_vst(res5, dst, 208);
  __lsx_vst(res6, dst, 224);
  __lsx_vst(res7, dst, 240);

  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src8);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src1, src9);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src2, src10);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src3, src11);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src4, src12);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src5, src13);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src6, src14);
  src += src_stride;
  DUP2_ARG2(__lsx_vld, src, 0, src, 16, src7, src15);
  src += src_stride;

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(src8, src10, src12, src14, src15, src13, src11, src9, tmp8,
                    tmp10, tmp12, tmp14, tmp15, tmp13, tmp11, tmp9);

  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src3, src4, src5, src6, src7);
  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);
  LSX_BUTTERFLY_8_H(src0, src1, src2, src3, src7, src6, src5, src4, tmp0, tmp7,
                    tmp3, tmp4, tmp5, tmp1, tmp6, tmp2);
  LSX_TRANSPOSE8x8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src0, src1,
                     src2, src3, src4, src5, src6, src7);

  __lsx_vst(src0, dst, 256);
  __lsx_vst(src1, dst, 272);
  __lsx_vst(src2, dst, 288);
  __lsx_vst(src3, dst, 304);
  __lsx_vst(src4, dst, 320);
  __lsx_vst(src5, dst, 336);
  __lsx_vst(src6, dst, 352);
  __lsx_vst(src7, dst, 368);

  LSX_BUTTERFLY_8_H(tmp8, tmp9, tmp12, tmp13, tmp15, tmp14, tmp11, tmp10, src8,
                    src9, src12, src13, src15, src14, src11, src10);
  LSX_BUTTERFLY_8_H(src8, src9, src10, src11, src15, src14, src13, src12, tmp8,
                    tmp15, tmp11, tmp12, tmp13, tmp9, tmp14, tmp10);
  LSX_TRANSPOSE8x8_H(tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, src8,
                     src9, src10, src11, src12, src13, src14, src15);
  LSX_BUTTERFLY_8_H(src8, src10, src12, src14, src15, src13, src11, src9, tmp8,
                    tmp10, tmp12, tmp14, tmp15, tmp13, tmp11, tmp9);
  LSX_BUTTERFLY_8_H(tmp8, tmp9, tmp12, tmp13, tmp15, tmp14, tmp11, tmp10, src8,
                    src9, src12, src13, src15, src14, src11, src10);
  LSX_BUTTERFLY_8_H(src8, src9, src10, src11, src15, src14, src13, src12, tmp8,
                    tmp15, tmp11, tmp12, tmp13, tmp9, tmp14, tmp10);
  LSX_TRANSPOSE8x8_H(tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15, res0,
                     res1, res2, res3, res4, res5, res6, res7);

  __lsx_vst(res0, dst, 384);
  __lsx_vst(res1, dst, 400);
  __lsx_vst(res2, dst, 416);
  __lsx_vst(res3, dst, 432);
  __lsx_vst(res4, dst, 448);
  __lsx_vst(res5, dst, 464);
  __lsx_vst(res6, dst, 480);
  __lsx_vst(res7, dst, 496);

  DUP4_ARG2(__lsx_vld, dst, 0, dst, 128, dst, 256, dst, 384, src0, src1, src2,
            src3);
  DUP4_ARG2(__lsx_vld, dst, 16, dst, 144, dst, 272, dst, 400, src4, src5, src6,
            src7);

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  DUP4_ARG2(__lsx_vsrai_h, tmp0, 1, tmp1, 1, tmp2, 1, tmp3, 1, tmp0, tmp1, tmp2,
            tmp3);
  DUP4_ARG2(__lsx_vsrai_h, tmp4, 1, tmp5, 1, tmp6, 1, tmp7, 1, tmp4, tmp5, tmp6,
            tmp7);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);

  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 128);
  __lsx_vst(src2, dst, 256);
  __lsx_vst(src3, dst, 384);
  __lsx_vst(src4, dst, 16);
  __lsx_vst(src5, dst, 144);
  __lsx_vst(src6, dst, 272);
  __lsx_vst(src7, dst, 400);
  dst += 16;

  DUP4_ARG2(__lsx_vld, dst, 0, dst, 128, dst, 256, dst, 384, src0, src1, src2,
            src3);
  DUP4_ARG2(__lsx_vld, dst, 16, dst, 144, dst, 272, dst, 400, src4, src5, src6,
            src7);

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  DUP4_ARG2(__lsx_vsrai_h, tmp0, 1, tmp1, 1, tmp2, 1, tmp3, 1, tmp0, tmp1, tmp2,
            tmp3);
  DUP4_ARG2(__lsx_vsrai_h, tmp4, 1, tmp5, 1, tmp6, 1, tmp7, 1, tmp4, tmp5, tmp6,
            tmp7);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);

  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 128);
  __lsx_vst(src2, dst, 256);
  __lsx_vst(src3, dst, 384);
  __lsx_vst(src4, dst, 16);
  __lsx_vst(src5, dst, 144);
  __lsx_vst(src6, dst, 272);
  __lsx_vst(src7, dst, 400);
  dst += 16;

  DUP4_ARG2(__lsx_vld, dst, 0, dst, 128, dst, 256, dst, 384, src0, src1, src2,
            src3);
  DUP4_ARG2(__lsx_vld, dst, 16, dst, 144, dst, 272, dst, 400, src4, src5, src6,
            src7);

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  DUP4_ARG2(__lsx_vsrai_h, tmp0, 1, tmp1, 1, tmp2, 1, tmp3, 1, tmp0, tmp1, tmp2,
            tmp3);
  DUP4_ARG2(__lsx_vsrai_h, tmp4, 1, tmp5, 1, tmp6, 1, tmp7, 1, tmp4, tmp5, tmp6,
            tmp7);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);

  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 128);
  __lsx_vst(src2, dst, 256);
  __lsx_vst(src3, dst, 384);
  __lsx_vst(src4, dst, 16);
  __lsx_vst(src5, dst, 144);
  __lsx_vst(src6, dst, 272);
  __lsx_vst(src7, dst, 400);
  dst += 16;

  DUP4_ARG2(__lsx_vld, dst, 0, dst, 128, dst, 256, dst, 384, src0, src1, src2,
            src3);
  DUP4_ARG2(__lsx_vld, dst, 16, dst, 144, dst, 272, dst, 400, src4, src5, src6,
            src7);

  LSX_BUTTERFLY_8_H(src0, src2, src4, src6, src7, src5, src3, src1, tmp0, tmp2,
                    tmp4, tmp6, tmp7, tmp5, tmp3, tmp1);
  DUP4_ARG2(__lsx_vsrai_h, tmp0, 1, tmp1, 1, tmp2, 1, tmp3, 1, tmp0, tmp1, tmp2,
            tmp3);
  DUP4_ARG2(__lsx_vsrai_h, tmp4, 1, tmp5, 1, tmp6, 1, tmp7, 1, tmp4, tmp5, tmp6,
            tmp7);
  LSX_BUTTERFLY_8_H(tmp0, tmp1, tmp4, tmp5, tmp7, tmp6, tmp3, tmp2, src0, src1,
                    src4, src5, src7, src6, src3, src2);

  __lsx_vst(src0, dst, 0);
  __lsx_vst(src1, dst, 128);
  __lsx_vst(src2, dst, 256);
  __lsx_vst(src3, dst, 384);
  __lsx_vst(src4, dst, 16);
  __lsx_vst(src5, dst, 144);
  __lsx_vst(src6, dst, 272);
  __lsx_vst(src7, dst, 400);
}
#endif  // !CONFIG_VP9_HIGHBITDEPTH
