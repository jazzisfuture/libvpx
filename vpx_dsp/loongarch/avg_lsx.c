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
  __lsx_vst(tmp0, dst, 0);
  __lsx_vst(tmp1, dst, 16);
  __lsx_vst(tmp2, dst, 32);
  __lsx_vst(tmp3, dst, 48);
  __lsx_vst(tmp4, dst, 64);
  __lsx_vst(tmp5, dst, 80);
  __lsx_vst(tmp6, dst, 96);
  __lsx_vst(tmp7, dst, 112);
}

void vpx_hadamard_16x16_lsx(const int16_t *src, ptrdiff_t src_stride,
                            int16_t *dst) {
  int i;
  __m128i a0, a1, a2, a3, b0, b1, b2, b3;

  /* Rearrange 16x16 to 8x32 and remove stride.
   * Top left first. */
  vpx_hadamard_8x8_lsx(src + 0 + 0 * src_stride, src_stride, dst + 0);
  /* Top right. */
  vpx_hadamard_8x8_lsx(src + 8 + 0 * src_stride, src_stride, dst + 64);
  /* Bottom left. */
  vpx_hadamard_8x8_lsx(src + 0 + 8 * src_stride, src_stride, dst + 128);
  /* Bottom right. */
  vpx_hadamard_8x8_lsx(src + 8 + 8 * src_stride, src_stride, dst + 192);

  for (i = 0; i < 64; i += 8) {
    DUP4_ARG2(__lsx_vld, dst, 0, dst, 128, dst, 256, dst, 384, a0, a1, a2, a3);
    LSX_BUTTERFLY_4_H(a0, a2, a3, a1, b0, b2, b3, b1);
    DUP4_ARG2(__lsx_vsrai_h, b0, 1, b1, 1, b2, 1, b3, 1, b0, b1, b2, b3);
    LSX_BUTTERFLY_4_H(b0, b1, b3, b2, a0, a1, a3, a2);

    __lsx_vst(a0, dst, 0);
    __lsx_vst(a1, dst, 128);
    __lsx_vst(a2, dst, 256);
    __lsx_vst(a3, dst, 384);

    dst += 8;
  }
}
#endif  // !CONFIG_VP9_HIGHBITDEPTH
