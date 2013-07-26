/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdio.h>

#include "vpx_config.h"
#include "vp9_rtcd.h"
#include "vp9/common/vp9_common.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/mem.h"

#include "vp9/common/mips/dspr2/vp9_common_dspr2.h"
#include "vp9/common/vp9_convolve.h"

#if HAVE_DSPR2
#define VP9_FILTER_WEIGHT 128
#define VP9_FILTER_SHIFT  7

void vp9_dsputil_static_init(void) {
  int i;

  for (i = 0; i < 256; i++) vp9_ff_cropTbl[i + CROP_WIDTH] = i;

  for (i = 0; i < CROP_WIDTH; i++) {
    vp9_ff_cropTbl[i] = 0;
    vp9_ff_cropTbl[i + CROP_WIDTH + 256] = 255;
  }
}

static void convolve_horiz_4_dspr2(const uint8_t *src,
                                   int32_t src_stride,
                                   uint8_t *dst,
                                   int32_t dst_stride,
                                   const int16_t *filter_x0,
                                   int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3, Temp4;
    uint32_t vector4a = 64;
    uint32_t tp1, tp2;
    uint32_t p1, p2, p3, p4;
    uint32_t n1, n2, n3, n4;
    uint32_t tn1, tn2;

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp1],      -3(%[src])                     \n\t"
          "ulw              %[tp2],      1(%[src])                      \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a], $ac3                           \n\t"
          "preceu.ph.qbr    %[p1],       %[tp1]                         \n\t"
          "preceu.ph.qbl    %[p2],       %[tp1]                         \n\t"
          "preceu.ph.qbr    %[p3],       %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p4],       %[tp2]                         \n\t"
          "dpa.w.ph         $ac3,        %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,        %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,        %[p3],          %[vector3b]    \n\t"
          "ulw              %[tn2],      5(%[src])                      \n\t"
          "dpa.w.ph         $ac3,        %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp1],    $ac3,           9              \n\t"

          /* even 2. pixel */
          "mtlo             %[vector4a], $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],       %[tn2]                         \n\t"
          "balign           %[tn1],      %[tn2],         3              \n\t"
          "balign           %[tn2],      %[tp2],         3              \n\t"
          "balign           %[tp2],      %[tp1],         3              \n\t"
          "dpa.w.ph         $ac2,        %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,        %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,        %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,        %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],    $ac2,           9              \n\t"

          /* odd 1. pixel */
          "lbux             %[tp1],      %[Temp1](%[cm])                \n\t"
          "mtlo             %[vector4a], $ac3                           \n\t"
          "preceu.ph.qbr    %[n1],       %[tp2]                         \n\t"
          "preceu.ph.qbl    %[n2],       %[tp2]                         \n\t"
          "preceu.ph.qbr    %[n3],       %[tn2]                         \n\t"
          "preceu.ph.qbl    %[n4],       %[tn2]                         \n\t"
          "dpa.w.ph         $ac3,        %[n1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,        %[n2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,        %[n3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,        %[n4],          %[vector4b]    \n\t"
          "extp             %[Temp2],    $ac3,           9              \n\t"

          /* odd 2. pixel */
          "lbux             %[tp2],      %[Temp3](%[cm])                \n\t"
          "mtlo             %[vector4a], $ac2                           \n\t"
          "preceu.ph.qbr    %[n1],       %[tn1]                         \n\t"
          "dpa.w.ph         $ac2,        %[n2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,        %[n3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,        %[n4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,        %[n1],          %[vector4b]    \n\t"
          "extp             %[Temp4],    $ac2,           9              \n\t"

          /* clamp */
          "lbux             %[tn1],      %[Temp2](%[cm])                \n\t"
          "lbux             %[n2],       %[Temp4](%[cm])                \n\t"

          /* store bytes */
          "sb               %[tp1],      0(%[dst])                      \n\t"
          "sb               %[tn1],      1(%[dst])                      \n\t"
          "sb               %[tp2],      2(%[dst])                      \n\t"
          "sb               %[n2],       3(%[dst])                      \n\t"

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
            [tn1] "=&r" (tn1), [tn2] "=&r" (tn2),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [n1] "=&r" (n1), [n2] "=&r" (n2), [n3] "=&r" (n3), [n4] "=&r" (n4),
            [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2),
            [Temp3] "=&r" (Temp3), [Temp4] "=&r" (Temp4)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a),
            [cm] "r" (cm), [dst] "r" (dst), [src] "r" (src)
      );

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_horiz_8_dspr2(const uint8_t *src,
                                   int32_t src_stride,
                                   uint8_t *dst,
                                   int32_t dst_stride,
                                   const int16_t *filter_x0,
                                   int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      dst[3] = src[3];
      dst[4] = src[4];
      dst[5] = src[5];
      dst[6] = src[6];
      dst[7] = src[7];

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3;
    uint32_t tp1, tp2;
    uint32_t p1, p2, p3, p4, n1;
    uint32_t tn1, tn2, tn3;
    uint32_t st0, st1;

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp1],      -3(%[src])                     \n\t"
          "ulw              %[tp2],      1(%[src])                      \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a], $ac3                           \n\t"
          "mtlo             %[vector4a], $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],       %[tp1]                         \n\t"
          "preceu.ph.qbl    %[p2],       %[tp1]                         \n\t"
          "preceu.ph.qbr    %[p3],       %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p4],       %[tp2]                         \n\t"
          "ulw              %[tn2],      5(%[src])                      \n\t"
          "dpa.w.ph         $ac3,        %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,        %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,        %[p3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,        %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp1],    $ac3,           9              \n\t"

          /* even 2. pixel */
          "preceu.ph.qbr    %[p1],       %[tn2]                         \n\t"
          "preceu.ph.qbl    %[n1],       %[tn2]                         \n\t"
          "ulw              %[tn1],      9(%[src])                      \n\t"
          "dpa.w.ph         $ac2,        %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,        %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,        %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,        %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],    $ac2,           9              \n\t"

          /* even 3. pixel */
          "lbux             %[st0],      %[Temp1](%[cm])                \n\t"
          "mtlo             %[vector4a], $ac1                           \n\t"
          "preceu.ph.qbr    %[p2],       %[tn1]                         \n\t"
          "dpa.w.ph         $ac1,        %[p3],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac1,        %[p4],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac1,        %[p1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac1,        %[n1],          %[vector4b]    \n\t"
          "extp             %[Temp1],    $ac1,           9              \n\t"

          /* even 4. pixel */
          "mtlo             %[vector4a], $ac2                           \n\t"
          "mtlo             %[vector4a], $ac3                           \n\t"
          "sb               %[st0],      0(%[dst])                      \n\t"
          "lbux             %[st1],      %[Temp3](%[cm])                \n\t"

          "balign           %[tn3],      %[tn1],         3              \n\t"
          "balign           %[tn1],      %[tn2],         3              \n\t"
          "balign           %[tn2],      %[tp2],         3              \n\t"
          "balign           %[tp2],      %[tp1],         3              \n\t"

          "dpa.w.ph         $ac2,        %[p4],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,        %[p1],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,        %[n1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,        %[p2],          %[vector4b]    \n\t"
          "extp             %[Temp3],    $ac2,           9              \n\t"

          "lbux             %[st0],      %[Temp1](%[cm])                \n\t"

          /* odd 1. pixel */
          "mtlo             %[vector4a], $ac1                           \n\t"
          "sb               %[st1],      2(%[dst])                      \n\t"
          "preceu.ph.qbr    %[p1],       %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p2],       %[tp2]                         \n\t"
          "preceu.ph.qbr    %[p3],       %[tn2]                         \n\t"
          "preceu.ph.qbl    %[p4],       %[tn2]                         \n\t"
          "sb               %[st0],      4(%[dst])                      \n\t"
          "dpa.w.ph         $ac3,        %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,        %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,        %[p3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,        %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp2],    $ac3,           9              \n\t"

          /* odd 2. pixel */
          "mtlo             %[vector4a], $ac3                           \n\t"
          "mtlo             %[vector4a], $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],       %[tn1]                         \n\t"
          "preceu.ph.qbl    %[n1],       %[tn1]                         \n\t"
          "lbux             %[st0],      %[Temp3](%[cm])                \n\t"
          "dpa.w.ph         $ac1,        %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac1,        %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac1,        %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac1,        %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],    $ac1,           9              \n\t"

          /* odd 3. pixel */
          "lbux             %[st1],      %[Temp2](%[cm])                \n\t"
          "preceu.ph.qbr    %[p2],       %[tn3]                         \n\t"
          "dpa.w.ph         $ac3,        %[p3],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,        %[p4],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,        %[p1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,        %[n1],          %[vector4b]    \n\t"
          "extp             %[Temp2],    $ac3,           9              \n\t"

          /* odd 4. pixel */
          "sb               %[st1],      1(%[dst])                      \n\t"
          "sb               %[st0],      6(%[dst])                      \n\t"
          "dpa.w.ph         $ac2,        %[p4],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,        %[p1],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,        %[n1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,        %[p2],          %[vector4b]    \n\t"
          "extp             %[Temp1],    $ac2,           9              \n\t"

          /* clamp */
          "lbux             %[p4],       %[Temp3](%[cm])                \n\t"
          "lbux             %[p2],       %[Temp2](%[cm])                \n\t"
          "lbux             %[n1],       %[Temp1](%[cm])                \n\t"

          /* store bytes */
          "sb               %[p4],       3(%[dst])                      \n\t"
          "sb               %[p2],       5(%[dst])                      \n\t"
          "sb               %[n1],       7(%[dst])                      \n\t"

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
            [tn1] "=&r" (tn1), [tn2] "=&r" (tn2), [tn3] "=&r" (tn3),
            [st0] "=&r" (st0), [st1] "=&r" (st1),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [n1] "=&r" (n1),
            [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a),
            [cm] "r" (cm), [dst] "r" (dst),
            [src] "r" (src)
      );

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_horiz_16_dspr2(const uint8_t *src_ptr,
                                    int32_t src_stride,
                                    uint8_t *dst_ptr,
                                    int32_t dst_stride,
                                    const int16_t *filter_x0,
                                    int32_t h,
                                    int32_t count) {
  int32_t y;
  int32_t c;
  const uint8_t *src;
  uint8_t *dst;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t filter78;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  filter78 = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(filter78 == 0) {
    uint32_t load1, load2, load3, load4;

    for (y = h; y --;) {
      src = (const uint8_t *)src_ptr;
      dst = dst_ptr;

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr + src_stride));

      for (c = 0; c < count; c++) {
        __asm__ __volatile__ (
            "ulw     %[load1],     0(%[src])          \n\t"
            "ulw     %[load2],     4(%[src])          \n\t"
            "ulw     %[load3],     8(%[src])          \n\t"
            "ulw     %[load4],     12(%[src])         \n\t"

            "sw      %[load1],     0(%[dst])          \n\t"
            "sw      %[load2],     4(%[dst])          \n\t"
            "sw      %[load3],     8(%[dst])          \n\t"
            "sw      %[load4],     12(%[dst])         \n\t"

            : [load1] "=&r" (load1), [load2] "=&r" (load2),
              [load3] "=&r" (load3), [load4] "=&r" (load4)
            : [src] "r" (src), [dst] "r" (dst)
        );

        src += 16;
        dst += 16;
      }

      /* next row... */
      src_ptr += src_stride;
      dst_ptr += dst_stride;
    }
  } else { /* if(filter78 != 0) */
    uint32_t vector_64 = 64;
    int32_t filter12, filter34, filter56;
    int32_t Temp1, Temp2, Temp3;
    uint32_t qload1, qload2, qload3;
    uint32_t p1, p2, p3, p4, p5;
    uint32_t st1, st2, st3;

    filter12 = ((const int32_t *)filter_x0)[0];
    filter34 = ((const int32_t *)filter_x0)[1];
    filter56 = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      src = src_ptr;
      dst = dst_ptr;

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr  - 3 + src_stride));

      for (c = 0; c < count; c++) {
        __asm__ __volatile__ (
            "ulw              %[qload1],    -3(%[src])                   \n\t"
            "ulw              %[qload2],    1(%[src])                    \n\t"

            /* even 1. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 1 */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 2 */
            "preceu.ph.qbr    %[p1],        %[qload1]                    \n\t"
            "preceu.ph.qbl    %[p2],        %[qload1]                    \n\t"
            "preceu.ph.qbr    %[p3],        %[qload2]                    \n\t"
            "preceu.ph.qbl    %[p4],        %[qload2]                    \n\t"
            "ulw              %[qload3],    5(%[src])                    \n\t"
            "dpa.w.ph         $ac1,         %[p1],          %[filter12]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter34]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter56]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter78]  \n\t" /* even 1 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 1 */

            /* even 2. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* even 3 */
            "preceu.ph.qbr    %[p1],        %[qload3]                    \n\t"
            "preceu.ph.qbl    %[p5],        %[qload3]                    \n\t"
            "ulw              %[qload1],    9(%[src])                    \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[filter12]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter34]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter56]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter78]  \n\t" /* even 1 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 1 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 1 */

            /* even 3. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 4 */
            "preceu.ph.qbr    %[p2],        %[qload1]                    \n\t"
            "sb               %[st1],       0(%[dst])                    \n\t" /* even 1 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter12]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter34]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter56]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p5],          %[filter78]  \n\t" /* even 3 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* even 3 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 1 */

            /* even 4. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 5 */
            "preceu.ph.qbl    %[p3],        %[qload1]                    \n\t"
            "sb               %[st2],       2(%[dst])                    \n\t" /* even 1 */
            "ulw              %[qload2],    13(%[src])                   \n\t"
            "dpa.w.ph         $ac1,         %[p4],          %[filter12]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter34]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter56]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter78]  \n\t" /* even 4 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 4 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* even 3 */

            /* even 5. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* even 6 */
            "preceu.ph.qbr    %[p4],        %[qload2]                    \n\t"
            "sb               %[st3],       4(%[dst])                    \n\t" /* even 3 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter12]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter34]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p2],          %[filter56]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter78]  \n\t" /* even 5 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 5 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 4 */

            /* even 6. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 7 */
            "preceu.ph.qbl    %[p1],        %[qload2]                    \n\t"
            "sb               %[st1],       6(%[dst])                    \n\t" /* even 4 */
            "ulw              %[qload3],    17(%[src])                   \n\t"
            "dpa.w.ph         $ac3,         %[p5],          %[filter12]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter34]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter56]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter78]  \n\t" /* even 6 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* even 6 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 5 */

            /* even 7. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 8 */
            "preceu.ph.qbr    %[p5],        %[qload3]                    \n\t"
            "sb               %[st2],       8(%[dst])                    \n\t" /* even 5 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter12]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter34]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter56]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter78]  \n\t" /* even 7 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 7 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* even 6 */

            /* even 8. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 1 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter12]  \n\t" /* even 8 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter34]  \n\t" /* even 8 */
            "sb               %[st3],       10(%[dst])                   \n\t" /* even 6 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter56]  \n\t" /* even 8 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter78]  \n\t" /* even 8 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 8 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 7 */

            /* ODD pixels */
            "ulw              %[qload1],    -2(%[src])                   \n\t"
            "ulw              %[qload2],    2(%[src])                    \n\t"

            /* odd 1. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 2 */
            "preceu.ph.qbr    %[p1],        %[qload1]                    \n\t"
            "preceu.ph.qbl    %[p2],        %[qload1]                    \n\t"
            "preceu.ph.qbr    %[p3],        %[qload2]                    \n\t"
            "preceu.ph.qbl    %[p4],        %[qload2]                    \n\t"
            "sb               %[st1],       12(%[dst])                   \n\t" /* even 7 */
            "ulw              %[qload3],    6(%[src])                    \n\t"
            "dpa.w.ph         $ac3,         %[p1],          %[filter12]  \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter34]  \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter56]  \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter78]  \n\t" /* odd 1 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 1 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 8 */

            /* odd 2. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* odd 3 */
            "preceu.ph.qbr    %[p1],        %[qload3]                    \n\t"
            "preceu.ph.qbl    %[p5],        %[qload3]                    \n\t"
            "sb               %[st2],       14(%[dst])                   \n\t" /* even 8 */
            "ulw              %[qload1],    10(%[src])                   \n\t"
            "dpa.w.ph         $ac1,         %[p2],          %[filter12]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter34]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter56]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter78]  \n\t" /* odd 2 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 2 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 1 */

            /* odd 3. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 4 */
            "preceu.ph.qbr    %[p2],        %[qload1]                    \n\t"
            "sb               %[st3],       1(%[dst])                    \n\t" /* odd 1 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter12]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter34]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter56]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter78]  \n\t" /* odd 3 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* odd 3 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 2 */

            /* odd 4. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 5 */
            "preceu.ph.qbl    %[p3],        %[qload1]                    \n\t"
            "sb               %[st1],       3(%[dst])                    \n\t" /* odd 2 */
            "ulw              %[qload2],    14(%[src])                   \n\t"
            "dpa.w.ph         $ac3,         %[p4],          %[filter12]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter34]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p5],          %[filter56]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter78]  \n\t" /* odd 4 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 4 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* odd 3 */

            /* odd 5. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* odd 6 */
            "preceu.ph.qbr    %[p4],        %[qload2]                    \n\t"
            "sb               %[st2],       5(%[dst])                    \n\t" /* odd 3 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter12]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter34]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter56]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter78]  \n\t" /* odd 5 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 5 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 4 */

            /* odd 6. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 7 */
            "preceu.ph.qbl    %[p1],        %[qload2]                    \n\t"
            "sb               %[st3],       7(%[dst])                    \n\t" /* odd 4 */
            "ulw              %[qload3],    18(%[src])                   \n\t"
            "dpa.w.ph         $ac2,         %[p5],          %[filter12]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p2],          %[filter34]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter56]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter78]  \n\t" /* odd 6 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* odd 6 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 5 */

            /* odd 7. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 8 */
            "preceu.ph.qbr    %[p5],        %[qload3]                    \n\t"
            "sb               %[st1],       9(%[dst])                    \n\t" /* odd 5 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter12]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter34]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter56]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter78]  \n\t" /* odd 7 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 7 */

            /* odd 8. pixel */
            "dpa.w.ph         $ac1,         %[p3],          %[filter12]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter34]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter56]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter78]  \n\t" /* odd 8 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 8 */

            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* odd 6 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 7 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 8 */

            "sb               %[st2],       11(%[dst])                   \n\t" /* odd 6 */
            "sb               %[st3],       13(%[dst])                   \n\t" /* odd 7 */
            "sb               %[st1],       15(%[dst])                   \n\t" /* odd 8 */

            : [qload1] "=&r" (qload1), [qload2] "=&r" (qload2), [qload3] "=&r" (qload3),
              [st1] "=&r" (st1), [st2] "=&r" (st2), [st3] "=&r" (st3),
              [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
              [p5] "=&r" (p5),
              [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3)
            : [filter12] "r" (filter12), [filter34] "r" (filter34),
              [filter56] "r" (filter56), [filter78] "r" (filter78),
              [vector_64] "r" (vector_64),
              [cm] "r" (cm), [dst] "r" (dst),
              [src] "r" (src)
        );

        src += 16;
        dst += 16;
      }

      /* Next row... */
      src_ptr += src_stride;
      dst_ptr += dst_stride;
    }
  }
}

static void convolve_vert_4_dspr2(const uint8_t *src,
                                  int32_t src_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int16_t *filter_y,
                                  int32_t w,
                                  int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_y)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    int32_t x;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      for (x = 0; x < w; x += 4) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        dst_ptr[0] = src_ptr[0];
        dst_ptr[1] = src_ptr[1];
        dst_ptr[2] = src_ptr[2];
        dst_ptr[3] = src_ptr[3];
      }

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    uint32_t load1, load2, load3, load4;
    uint32_t p1, p2;
    uint32_t n1, n2;
    uint32_t scratch1, scratch2;
    uint32_t store1, store2;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2;
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    int32_t x;

    vector1b = ((const int32_t *)filter_y)[0];
    vector2b = ((const int32_t *)filter_y)[1];
    vector3b = ((const int32_t *)filter_y)[2];

    src -= 3 * src_stride;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));
      for (x = 0; x < w; x += 4) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        __asm__ __volatile__ (
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "mtlo             %[vector4a],  $ac0                            \n\t"
            "mtlo             %[vector4a],  $ac1                            \n\t"
            "mtlo             %[vector4a],  $ac2                            \n\t"
            "mtlo             %[vector4a],  $ac3                            \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac0,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector2b]     \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac2,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector2b]     \n\t"

            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac0,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector4b]     \n\t"
            "extp             %[Temp1],     $ac0,           9               \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector4b]     \n\t"
            "extp             %[Temp2],     $ac1,           9               \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "dpa.w.ph         $ac2,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector4b]     \n\t"
            "extp             %[Temp1],     $ac2,           9               \n\t"

            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector4b]     \n\t"
            "extp             %[Temp2],     $ac3,           9               \n\t"

            "sb               %[store1],    0(%[dst_ptr])                   \n\t"
            "sb               %[store2],    1(%[dst_ptr])                   \n\t"

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"

            "sb               %[store1],    2(%[dst_ptr])                   \n\t"
            "sb               %[store2],    3(%[dst_ptr])                   \n\t" /* 79 cycles */

            : [load1] "=&r" (load1), [load2] "=&r" (load2), [load3] "=&r" (load3), [load4] "=&r" (load4),
              [p1] "=&r" (p1), [p2] "=&r" (p2),
              [n1] "=&r" (n1), [n2] "=&r" (n2),
              [scratch1] "=&r" (scratch1), [scratch2] "=&r" (scratch2),
              [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2),
              [store1] "=&r" (store1), [store2] "=&r" (store2),
              [src_ptr] "+r" (src_ptr)
            : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b), [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
              [vector4a] "r" (vector4a),
              [src_stride] "r" (src_stride),
              [cm] "r" (cm),
              [dst_ptr] "r" (dst_ptr)
        );
      }

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_vert_16_dspr2(const uint8_t *src,
                                   int32_t src_stride,
                                   uint8_t *dst,
                                   int32_t dst_stride,
                                   const int16_t *filter_y,
                                   int32_t w,
                                   int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_y)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    uint32_t load1, load2, load3, load4;
    int32_t x;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      for (x = 0; x < w; x += 16) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        __asm__ __volatile__ (
            "ulw     %[load1],     0(%[src_ptr])          \n\t"
            "ulw     %[load2],     4(%[src_ptr])          \n\t"
            "ulw     %[load3],     8(%[src_ptr])          \n\t"
            "ulw     %[load4],     12(%[src_ptr])         \n\t"

            "sw      %[load1],     0(%[dst_ptr])          \n\t"
            "sw      %[load2],     4(%[dst_ptr])          \n\t"
            "sw      %[load3],     8(%[dst_ptr])          \n\t"
            "sw      %[load4],     12(%[dst_ptr])         \n\t"

            : [load1] "=&r" (load1), [load2] "=&r" (load2), [load3] "=&r" (load3), [load4] "=&r" (load4)
            : [src_ptr] "r" (src_ptr), [dst_ptr] "r" (dst_ptr)
        );
      }

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    uint32_t load1, load2, load3, load4;
    uint32_t p1, p2;
    uint32_t n1, n2;
    uint32_t scratch1, scratch2;
    uint32_t store1, store2;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2;
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    int32_t x;

    vector1b = ((const int32_t *)filter_y)[0];
    vector2b = ((const int32_t *)filter_y)[1];
    vector3b = ((const int32_t *)filter_y)[2];

    src -= 3 * src_stride;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));
      for (x = 0; x < w; x += 4) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        __asm__ __volatile__ (
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "mtlo             %[vector4a],  $ac0                            \n\t"
            "mtlo             %[vector4a],  $ac1                            \n\t"
            "mtlo             %[vector4a],  $ac2                            \n\t"
            "mtlo             %[vector4a],  $ac3                            \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t"
            "append           %[p1],        %[scratch1],    16              \n\t"
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t"
            "append           %[p2],        %[scratch2],    16              \n\t"

            "dpa.w.ph         $ac0,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector2b]     \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t"
            "append           %[p1],        %[scratch1],    16              \n\t"
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t"
            "append           %[p2],        %[scratch2],    16              \n\t"

            "dpa.w.ph         $ac2,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector2b]     \n\t"

            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t"
            "append           %[p1],        %[scratch1],    16              \n\t"
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t"
            "append           %[p2],        %[scratch2],    16              \n\t"

            "dpa.w.ph         $ac0,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector4b]     \n\t"
            "extp             %[Temp1],     $ac0,           9               \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector4b]     \n\t"
            "extp             %[Temp2],     $ac1,           9               \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t"
            "append           %[p1],        %[scratch1],    16              \n\t"
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t"
            "append           %[p2],        %[scratch2],    16              \n\t"

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "dpa.w.ph         $ac2,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector4b]     \n\t"
            "extp             %[Temp1],     $ac2,           9               \n\t"

            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector4b]     \n\t"
            "extp             %[Temp2],     $ac3,           9               \n\t"

            "sb               %[store1],    0(%[dst_ptr])                   \n\t"
            "sb               %[store2],    1(%[dst_ptr])                   \n\t"

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"

            "sb               %[store1],    2(%[dst_ptr])                   \n\t"
            "sb               %[store2],    3(%[dst_ptr])                   \n\t" /* 79 cycles */

            : [load1] "=&r" (load1), [load2] "=&r" (load2), [load3] "=&r" (load3), [load4] "=&r" (load4),
              [p1] "=&r" (p1), [p2] "=&r" (p2), [n1] "=&r" (n1), [n2] "=&r" (n2),
              [scratch1] "=&r" (scratch1), [scratch2] "=&r" (scratch2),
              [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2),
              [store1] "=&r" (store1), [store2] "=&r" (store2),
              [src_ptr] "+r" (src_ptr)
            : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b), [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
              [vector4a] "r" (vector4a),
              [src_stride] "r" (src_stride),
              [cm] "r" (cm),
              [dst_ptr] "r" (dst_ptr)
        );
      }

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_horiz_4_transposed_dspr2(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              const int16_t *filter_x0,
                                              int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;
  uint8_t *dst_ptr;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((src + src_stride));

      dst[0] = src[0];
      dst[1 + dst_stride] = src[1];
      dst[2 + dst_stride] = src[2];
      dst[3 + dst_stride] = src[3];

      /* next row... */
      src += src_stride;
      dst += 1;
    }
  } else { /* if(vector4b != 0) */
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3, Temp4;
    uint32_t vector4a = 64;
    uint32_t tp1, tp2;
    uint32_t p1, p2, p3, p4;
    uint32_t tn1, tn2;
    uint32_t dst_pitch = dst_stride;

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      dst_ptr = dst;

      /* prefetch src data to cache memory */
      vp9_prefetch_load(src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp1],         -3(%[src])                     \n\t"
          "ulw              %[tp2],         1(%[src])                      \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tp1]                         \n\t"
          "preceu.ph.qbl    %[p2],          %[tp1]                         \n\t"
          "preceu.ph.qbr    %[p3],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p4],          %[tp2]                         \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]    \n\t"
          "ulw              %[tn2],         5(%[src])                      \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp1],       $ac3,           9              \n\t"

          /* even 2. pixel */
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tn2]                         \n\t"
          "balign           %[tn1],         %[tn2],         3              \n\t"
          "balign           %[tn2],         %[tp2],         3              \n\t"
          "balign           %[tp2],         %[tp1],         3              \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],       $ac2,           9              \n\t"

          /* odd 1. pixel */
          "lbux             %[tp1],         %[Temp1](%[cm])                \n\t"
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p2],          %[tp2]                         \n\t"
          "preceu.ph.qbr    %[p3],          %[tn2]                         \n\t"
          "preceu.ph.qbl    %[p4],          %[tn2]                         \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp2],       $ac3,           9              \n\t"

          /* odd 2. pixel */
          "lbux             %[tp2],         %[Temp3](%[cm])                \n\t"
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tn1]                         \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp4],       $ac2,           9              \n\t"

          /* clamp */
          "lbux             %[tn1],         %[Temp2](%[cm])                \n\t"
          "lbux             %[p2],          %[Temp4](%[cm])                \n\t"

          /* store bytes */
          "sb               %[tp1],         0(%[dst_ptr])                  \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch]   \n\t"

          "sb               %[tn1],         0(%[dst_ptr])                  \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch]   \n\t"

          "sb               %[tp2],         0(%[dst_ptr])                  \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch]   \n\t"

          "sb               %[p2],          0(%[dst_ptr])                  \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch]   \n\t"

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2), [tn1] "=&r" (tn1), [tn2] "=&r" (tn2),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3), [Temp4] "=&r" (Temp4),
            [dst_ptr] "+r" (dst_ptr)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a),
            [cm] "r" (cm), [src] "r" (src), [dst_pitch] "r" (dst_pitch)
      );

      /* Next row... */
      src += src_stride;
      dst += 1;
    }
  }
}

static void convolve_horiz_8_transposed_dspr2(const uint8_t *src,
                                              int32_t src_stride,
                                              uint8_t *dst,
                                              int32_t dst_stride,
                                              const int16_t *filter_x0,
                                              int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;
  uint8_t *dst_ptr;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      dst[0] = src[0];
      dst[1 + dst_stride] = src[1];
      dst[2 + dst_stride] = src[2];
      dst[3 + dst_stride] = src[3];
      dst[4 + dst_stride] = src[4];
      dst[5 + dst_stride] = src[5];
      dst[6 + dst_stride] = src[6];
      dst[7 + dst_stride] = src[7];

      /* next row... */
      src += src_stride;
      dst += 1;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3;
    uint32_t tp1, tp2, tp3;
    uint32_t p1, p2, p3, p4, n1;
    uint8_t *odd_dst;
    uint32_t dst_pitch_2 = (dst_stride << 1);

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      dst_ptr = dst;
      odd_dst = (dst_ptr + dst_stride);

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp2],         -3(%[src])                      \n\t"
          "ulw              %[tp1],         1(%[src])                       \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a],    $ac3                            \n\t"
          "mtlo             %[vector4a],    $ac2                            \n\t"
          "preceu.ph.qbr    %[p1],          %[tp2]                          \n\t"
          "preceu.ph.qbl    %[p2],          %[tp2]                          \n\t"
          "preceu.ph.qbr    %[p3],          %[tp1]                          \n\t"
          "preceu.ph.qbl    %[p4],          %[tp1]                          \n\t"
          "ulw              %[tp3],         5(%[src])                       \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]     \n\t"
          "extp             %[Temp1],       $ac3,           9               \n\t"

          /* even 2. pixel */
          "preceu.ph.qbr    %[p1],          %[tp3]                          \n\t"
          "preceu.ph.qbl    %[n1],          %[tp3]                          \n\t"
          "ulw              %[tp2],         9(%[src])                       \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac2,           %[p3],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector4b]     \n\t"
          "extp             %[Temp3],       $ac2,           9               \n\t"

          /* even 3. pixel */
          "lbux             %[Temp2],       %[Temp1](%[cm])                 \n\t"
          "mtlo             %[vector4a],    $ac1                            \n\t"
          "preceu.ph.qbr    %[p2],          %[tp2]                          \n\t"
          "dpa.w.ph         $ac1,           %[p3],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac1,           %[p4],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac1,           %[p1],          %[vector3b]     \n\t"
          "lbux             %[tp3],         %[Temp3](%[cm])                 \n\t"
          "dpa.w.ph         $ac1,           %[n1],          %[vector4b]     \n\t"
          "extp             %[p3],          $ac1,           9               \n\t"

          /* even 4. pixel */
          "mtlo             %[vector4a],    $ac2                            \n\t"
          "mtlo             %[vector4a],    $ac3                            \n\t"
          "sb               %[Temp2],       0(%[dst_ptr])                   \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch_2]  \n\t"
          "sb               %[tp3],         0(%[dst_ptr])                   \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch_2]  \n\t"

          "ulw              %[tp1],         -2(%[src])                      \n\t"
          "ulw              %[tp3],         2(%[src])                       \n\t"

          "dpa.w.ph         $ac2,           %[p4],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac2,           %[n1],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector4b]     \n\t"
          "extp             %[Temp3],       $ac2,           9               \n\t"

          "lbux             %[tp2],         %[p3](%[cm])                    \n\t"

          /* odd 1. pixel */
          "mtlo             %[vector4a],    $ac1                            \n\t"
          "preceu.ph.qbr    %[p1],          %[tp1]                          \n\t"
          "preceu.ph.qbl    %[p2],          %[tp1]                          \n\t"
          "preceu.ph.qbr    %[p3],          %[tp3]                          \n\t"
          "preceu.ph.qbl    %[p4],          %[tp3]                          \n\t"
          "sb               %[tp2],         0(%[dst_ptr])                   \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch_2]  \n\t"
          "ulw              %[tp2],         6(%[src])                       \n\t"

          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]     \n\t"
          "extp             %[Temp2],       $ac3,           9               \n\t"

          /* odd 2. pixel */
          "lbux             %[tp1],         %[Temp3](%[cm])                 \n\t"
          "mtlo             %[vector4a],    $ac3                            \n\t"
          "mtlo             %[vector4a],    $ac2                            \n\t"
          "preceu.ph.qbr    %[p1],          %[tp2]                          \n\t"
          "preceu.ph.qbl    %[n1],          %[tp2]                          \n\t"
          "ulw              %[Temp1],       10(%[src])                      \n\t"
          "dpa.w.ph         $ac1,           %[p2],          %[vector1b]     \n\t"
          "sb               %[tp1],         0(%[dst_ptr])                   \n\t"
          "addu             %[dst_ptr],     %[dst_ptr],     %[dst_pitch_2]  \n\t"
          "dpa.w.ph         $ac1,           %[p3],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac1,           %[p4],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac1,           %[p1],          %[vector4b]     \n\t"
          "extp             %[Temp3],       $ac1,           9               \n\t"

          /* odd 3. pixel */
          "lbux             %[tp3],         %[Temp2](%[cm])                 \n\t"
          "preceu.ph.qbr    %[p2],          %[Temp1]                        \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac3,           %[n1],          %[vector4b]     \n\t"
          "extp             %[Temp2],       $ac3,           9               \n\t"

          /* odd 4. pixel */
          "sb               %[tp3],         0(%[odd_dst])                   \n\t"
          "addu             %[odd_dst],     %[odd_dst],     %[dst_pitch_2]  \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector1b]     \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector2b]     \n\t"
          "dpa.w.ph         $ac2,           %[n1],          %[vector3b]     \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector4b]     \n\t"
          "extp             %[Temp1],       $ac2,           9               \n\t"

          /* clamp */
          "lbux             %[p4],          %[Temp3](%[cm])                 \n\t"
          "lbux             %[p2],          %[Temp2](%[cm])                 \n\t"
          "lbux             %[n1],          %[Temp1](%[cm])                 \n\t"

          /* store bytes */
          "sb               %[p4],          0(%[odd_dst])                   \n\t"
          "addu             %[odd_dst],     %[odd_dst],     %[dst_pitch_2]  \n\t"

          "sb               %[p2],          0(%[odd_dst])                   \n\t"
          "addu             %[odd_dst],     %[odd_dst],     %[dst_pitch_2]  \n\t"

          "sb               %[n1],          0(%[odd_dst])                   \n\t"

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2), [tp3] "=&r" (tp3),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [n1] "=&r" (n1), [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3),
            [dst_ptr] "+r" (dst_ptr), [odd_dst] "+r" (odd_dst)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a), [cm] "r" (cm), [src] "r" (src), [dst_pitch_2] "r" (dst_pitch_2)
      );

      /* Next row... */
      src += src_stride;
      dst += 1;
    }
  }
}

static void convolve_horiz_16_transposed_dspr2(const uint8_t *src_ptr,
                                               int32_t src_stride,
                                               uint8_t *dst_ptr,
                                               int32_t dst_stride,
                                               const int16_t *filter_x0,
                                               int32_t h,
                                               int32_t count) {
  int32_t y;
  int32_t c;
  const uint8_t *src;
  uint8_t *dst;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t filter78;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  filter78 = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(filter78 == 0) {
    for (y = h; y --;) {
      src = src_ptr;
      dst = dst_ptr;

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr + src_stride));

      for (c = 0; c < count; c++) {
        dst[0] = src[0];
        dst[1  + dst_stride] = src[1];
        dst[2  + dst_stride] = src[2];
        dst[3  + dst_stride] = src[3];
        dst[4  + dst_stride] = src[4];
        dst[5  + dst_stride] = src[5];
        dst[6  + dst_stride] = src[6];
        dst[7  + dst_stride] = src[7];

        dst[8  + dst_stride] = src[8];
        dst[9  + dst_stride] = src[9];
        dst[10 + dst_stride] = src[10];
        dst[11 + dst_stride] = src[11];
        dst[12 + dst_stride] = src[12];
        dst[13 + dst_stride] = src[13];
        dst[14 + dst_stride] = src[14];
        dst[15 + dst_stride] = src[15];

        src += 16;
        dst += (16*dst_stride);
      }

      /* next row... */
      src_ptr += src_stride;

      dst_ptr += 1;
    }
  } else { /* if(filter78 != 0) */
    uint32_t vector_64 = 64;
    int32_t filter12, filter34, filter56;
    int32_t Temp1, Temp2, Temp3;
    uint32_t qload1, qload2;
    uint32_t p1, p2, p3, p4, p5;
    uint32_t st1, st2, st3;
    uint32_t dst_pitch_2 = (dst_stride << 1);
    uint8_t *odd_dst;

    filter12 = ((const int32_t *)filter_x0)[0];
    filter34 = ((const int32_t *)filter_x0)[1];
    filter56 = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      src = src_ptr;
      dst = dst_ptr;

      odd_dst = (dst + dst_stride);

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr  - 3 + src_stride));

      for (c = 0; c < count; c++) {
        __asm__ __volatile__ (
            "ulw              %[qload1],        -3(%[src])                      \n\t"
            "ulw              %[qload2],        1(%[src])                       \n\t"

            /* even 1. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* even 1 */
            "mtlo             %[vector_64],     $ac2                            \n\t" /* even 2 */
            "preceu.ph.qbr    %[p3],            %[qload2]                       \n\t"
            "preceu.ph.qbl    %[p4],            %[qload2]                       \n\t"
            "preceu.ph.qbr    %[p1],            %[qload1]                       \n\t"
            "preceu.ph.qbl    %[p2],            %[qload1]                       \n\t"
            "ulw              %[qload2],        5(%[src])                       \n\t"
            "dpa.w.ph         $ac1,             %[p1],          %[filter12]     \n\t" /* even 1 */
            "dpa.w.ph         $ac1,             %[p2],          %[filter34]     \n\t" /* even 1 */
            "dpa.w.ph         $ac1,             %[p3],          %[filter56]     \n\t" /* even 1 */
            "dpa.w.ph         $ac1,             %[p4],          %[filter78]     \n\t" /* even 1 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* even 1 */

            /* even 2. pixel */
            "mtlo             %[vector_64],     $ac3                            \n\t" /* even 3 */
            "preceu.ph.qbr    %[p1],            %[qload2]                       \n\t"
            "preceu.ph.qbl    %[p5],            %[qload2]                       \n\t"
            "ulw              %[qload1],        9(%[src])                       \n\t"
            "dpa.w.ph         $ac2,             %[p2],          %[filter12]     \n\t" /* even 1 */
            "dpa.w.ph         $ac2,             %[p3],          %[filter34]     \n\t" /* even 1 */
            "dpa.w.ph         $ac2,             %[p4],          %[filter56]     \n\t" /* even 1 */
            "dpa.w.ph         $ac2,             %[p1],          %[filter78]     \n\t" /* even 1 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* even 1 */
            "extp             %[Temp2],         $ac2,           9               \n\t" /* even 1 */

            /* even 3. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* even 4 */
            "preceu.ph.qbr    %[p2],            %[qload1]                       \n\t"
            "sb               %[st1],           0(%[dst])                       \n\t" /* even 1 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]             \n\t"
            "dpa.w.ph         $ac3,             %[p3],          %[filter12]     \n\t" /* even 3 */
            "dpa.w.ph         $ac3,             %[p4],          %[filter34]     \n\t" /* even 3 */
            "dpa.w.ph         $ac3,             %[p1],          %[filter56]     \n\t" /* even 3 */
            "dpa.w.ph         $ac3,             %[p5],          %[filter78]     \n\t" /* even 3 */
            "extp             %[Temp3],         $ac3,           9               \n\t" /* even 3 */
            "lbux             %[st2],           %[Temp2](%[cm])                 \n\t" /* even 1 */

            /* even 4. pixel */
            "mtlo             %[vector_64],     $ac2                            \n\t" /* even 5 */
            "preceu.ph.qbl    %[p3],            %[qload1]                       \n\t"
            "sb               %[st2],           0(%[dst])                       \n\t" /* even 2 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "ulw              %[qload2],        13(%[src])                      \n\t"
            "dpa.w.ph         $ac1,             %[p4],          %[filter12]     \n\t" /* even 4 */
            "dpa.w.ph         $ac1,             %[p1],          %[filter34]     \n\t" /* even 4 */
            "dpa.w.ph         $ac1,             %[p5],          %[filter56]     \n\t" /* even 4 */
            "dpa.w.ph         $ac1,             %[p2],          %[filter78]     \n\t" /* even 4 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* even 4 */
            "lbux             %[st3],           %[Temp3](%[cm])                 \n\t" /* even 3 */

            /* even 5. pixel */
            "mtlo             %[vector_64],     $ac3                            \n\t" /* even 6 */
            "preceu.ph.qbr    %[p4],            %[qload2]                       \n\t"
            "sb               %[st3],           0(%[dst])                       \n\t" /* even 3 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac2,             %[p1],          %[filter12]     \n\t" /* even 5 */
            "dpa.w.ph         $ac2,             %[p5],          %[filter34]     \n\t" /* even 5 */
            "dpa.w.ph         $ac2,             %[p2],          %[filter56]     \n\t" /* even 5 */
            "dpa.w.ph         $ac2,             %[p3],          %[filter78]     \n\t" /* even 5 */
            "extp             %[Temp2],         $ac2,           9               \n\t" /* even 5 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* even 4 */

            /* even 6. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* even 7 */
            "preceu.ph.qbl    %[p1],            %[qload2]                       \n\t"
            "sb               %[st1],           0(%[dst])                       \n\t" /* even 4 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "ulw              %[qload1],        17(%[src])                      \n\t"
            "dpa.w.ph         $ac3,             %[p5],          %[filter12]     \n\t" /* even 6 */
            "dpa.w.ph         $ac3,             %[p2],          %[filter34]     \n\t" /* even 6 */
            "dpa.w.ph         $ac3,             %[p3],          %[filter56]     \n\t" /* even 6 */
            "dpa.w.ph         $ac3,             %[p4],          %[filter78]     \n\t" /* even 6 */
            "extp             %[Temp3],         $ac3,           9               \n\t" /* even 6 */
            "lbux             %[st2],           %[Temp2](%[cm])                 \n\t" /* even 5 */

            /* even 7. pixel */
            "mtlo             %[vector_64],     $ac2                            \n\t" /* even 8 */
            "preceu.ph.qbr    %[p5],            %[qload1]                       \n\t"
            "sb               %[st2],           0(%[dst])                       \n\t" /* even 5 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac1,             %[p2],          %[filter12]     \n\t" /* even 7 */
            "dpa.w.ph         $ac1,             %[p3],          %[filter34]     \n\t" /* even 7 */
            "dpa.w.ph         $ac1,             %[p4],          %[filter56]     \n\t" /* even 7 */
            "dpa.w.ph         $ac1,             %[p1],          %[filter78]     \n\t" /* even 7 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* even 7 */
            "lbux             %[st3],           %[Temp3](%[cm])                 \n\t" /* even 6 */

            /* even 8. pixel */
            "mtlo             %[vector_64],     $ac3                            \n\t" /* odd 1 */
            "dpa.w.ph         $ac2,             %[p3],          %[filter12]     \n\t" /* even 8 */
            "dpa.w.ph         $ac2,             %[p4],          %[filter34]     \n\t" /* even 8 */
            "sb               %[st3],           0(%[dst])                       \n\t" /* even 6 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac2,             %[p1],          %[filter56]     \n\t" /* even 8 */
            "dpa.w.ph         $ac2,             %[p5],          %[filter78]     \n\t" /* even 8 */
            "extp             %[Temp2],         $ac2,           9               \n\t" /* even 8 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* even 7 */

            /* ODD pixels */
            "ulw              %[qload1],        -2(%[src])                      \n\t"
            "ulw              %[qload2],        2(%[src])                       \n\t"

            /* odd 1. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* odd 2 */
            "preceu.ph.qbr    %[p1],            %[qload1]                       \n\t"
            "preceu.ph.qbl    %[p2],            %[qload1]                       \n\t"
            "preceu.ph.qbr    %[p3],            %[qload2]                       \n\t"
            "preceu.ph.qbl    %[p4],            %[qload2]                       \n\t"
            "sb               %[st1],           0(%[dst])                       \n\t" /* even 7 */
            "addu             %[dst],           %[dst],         %[dst_pitch_2]  \n\t"
            "ulw              %[qload2],        6(%[src])                       \n\t"
            "dpa.w.ph         $ac3,             %[p1],          %[filter12]     \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,             %[p2],          %[filter34]     \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,             %[p3],          %[filter56]     \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,             %[p4],          %[filter78]     \n\t" /* odd 1 */
            "extp             %[Temp3],         $ac3,           9               \n\t" /* odd 1 */
            "lbux             %[st2],           %[Temp2](%[cm])                 \n\t" /* even 8 */

            /* odd 2. pixel */
            "mtlo             %[vector_64],     $ac2                            \n\t" /* odd 3 */
            "preceu.ph.qbr    %[p1],            %[qload2]                       \n\t"
            "preceu.ph.qbl    %[p5],            %[qload2]                       \n\t"
            "sb               %[st2],           0(%[dst])                       \n\t" /* even 8 */
            "ulw              %[qload1],        10(%[src])                      \n\t"
            "dpa.w.ph         $ac1,             %[p2],          %[filter12]     \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,             %[p3],          %[filter34]     \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,             %[p4],          %[filter56]     \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,             %[p1],          %[filter78]     \n\t" /* odd 2 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* odd 2 */
            "lbux             %[st3],           %[Temp3](%[cm])                 \n\t" /* odd 1 */

            /* odd 3. pixel */
            "mtlo             %[vector_64],     $ac3                            \n\t" /* odd 4 */
            "preceu.ph.qbr    %[p2],            %[qload1]                       \n\t"
            "sb               %[st3],           0(%[odd_dst])                   \n\t" /* odd 1 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac2,             %[p3],          %[filter12]     \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,             %[p4],          %[filter34]     \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,             %[p1],          %[filter56]     \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,             %[p5],          %[filter78]     \n\t" /* odd 3 */
            "extp             %[Temp2],         $ac2,           9               \n\t" /* odd 3 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* odd 2 */

            /* odd 4. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* odd 5 */
            "preceu.ph.qbl    %[p3],            %[qload1]                       \n\t"
            "sb               %[st1],           0(%[odd_dst])                   \n\t" /* odd 2 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"
            "ulw              %[qload2],        14(%[src])                      \n\t"
            "dpa.w.ph         $ac3,             %[p4],          %[filter12]     \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,             %[p1],          %[filter34]     \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,             %[p5],          %[filter56]     \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,             %[p2],          %[filter78]     \n\t" /* odd 4 */
            "extp             %[Temp3],         $ac3,           9               \n\t" /* odd 4 */
            "lbux             %[st2],           %[Temp2](%[cm])                 \n\t" /* odd 3 */

            /* odd 5. pixel */
            "mtlo             %[vector_64],     $ac2                            \n\t" /* odd 6 */
            "preceu.ph.qbr    %[p4],            %[qload2]                       \n\t"
            "sb               %[st2],           0(%[odd_dst])                   \n\t" /* odd 3 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac1,             %[p1],          %[filter12]     \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,             %[p5],          %[filter34]     \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,             %[p2],          %[filter56]     \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,             %[p3],          %[filter78]     \n\t" /* odd 5 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* odd 5 */
            "lbux             %[st3],           %[Temp3](%[cm])                 \n\t" /* odd 4 */

            /* odd 6. pixel */
            "mtlo             %[vector_64],     $ac3                            \n\t" /* odd 7 */
            "preceu.ph.qbl    %[p1],            %[qload2]                       \n\t"
            "sb               %[st3],           0(%[odd_dst])                   \n\t" /* odd 4 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"
            "ulw              %[qload1],        18(%[src])                      \n\t"
            "dpa.w.ph         $ac2,             %[p5],          %[filter12]     \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,             %[p2],          %[filter34]     \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,             %[p3],          %[filter56]     \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,             %[p4],          %[filter78]     \n\t" /* odd 6 */
            "extp             %[Temp2],         $ac2,           9               \n\t" /* odd 6 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* odd 5 */

            /* odd 7. pixel */
            "mtlo             %[vector_64],     $ac1                            \n\t" /* odd 8 */
            "preceu.ph.qbr    %[p5],            %[qload1]                       \n\t"
            "sb               %[st1],           0(%[odd_dst])                   \n\t" /* odd 5 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"
            "dpa.w.ph         $ac3,             %[p2],          %[filter12]     \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,             %[p3],          %[filter34]     \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,             %[p4],          %[filter56]     \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,             %[p1],          %[filter78]     \n\t" /* odd 7 */
            "extp             %[Temp3],         $ac3,           9               \n\t" /* odd 7 */

            /* odd 8. pixel */
            "dpa.w.ph         $ac1,             %[p3],          %[filter12]     \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,             %[p4],          %[filter34]     \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,             %[p1],          %[filter56]     \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,             %[p5],          %[filter78]     \n\t" /* odd 8 */
            "extp             %[Temp1],         $ac1,           9               \n\t" /* odd 8 */

            "lbux             %[st2],           %[Temp2](%[cm])                 \n\t" /* odd 6 */
            "lbux             %[st3],           %[Temp3](%[cm])                 \n\t" /* odd 7 */
            "lbux             %[st1],           %[Temp1](%[cm])                 \n\t" /* odd 8 */

            "sb               %[st2],           0(%[odd_dst])                   \n\t" /* odd 6 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"

            "sb               %[st3],           0(%[odd_dst])                   \n\t" /* odd 7 */
            "addu             %[odd_dst],       %[odd_dst],     %[dst_pitch_2]  \n\t"

            "sb               %[st1],           0(%[odd_dst])                   \n\t" /* odd 8 */

            : [qload1] "=&r" (qload1), [qload2] "=&r" (qload2),
              [st1] "=&r" (st1), [st2] "=&r" (st2), [st3] "=&r" (st3),
              [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
              [p5] "=&r" (p5), [dst] "+r" (dst),
              [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3),
              [odd_dst] "+r" (odd_dst)
            : [filter12] "r" (filter12), [filter34] "r" (filter34), [filter56] "r" (filter56), [filter78] "r" (filter78),
              [vector_64] "r" (vector_64), [cm] "r" (cm), [src] "r" (src), [dst_pitch_2] "r" (dst_pitch_2)
        );

        src += 16;
        dst = (dst_ptr + ((c+1)*16*dst_stride));
        odd_dst = (dst + dst_stride);
      }

      /* Next row... */
      src_ptr += src_stride;

      dst_ptr += 1;
    }
  }
}

static void convolve_horiz_c_dspr2(const uint8_t *src, int32_t src_stride,
                                   uint8_t *dst, int32_t dst_stride,
                                   const int16_t *filter_x0, int32_t x_step_q4,
                                   const int16_t *filter_y, int32_t y_step_q4,
                                   int32_t w, int32_t h, int32_t taps) {
  int32_t x, y, k, sum;
  const int16_t *filter_x_base = filter_x0;

#if ALIGN_FILTERS_256
  filter_x_base = (const int16_t *)(((intptr_t)filter_x0) & ~(intptr_t)0xff);
#endif

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */

  /* Adjust base pointer address for this source line */
  src -= (taps / 2 - 1);

  for (y = 0; y < h; ++y) {
    /* Pointer to filter to use */
    const int16_t *filter_x = filter_x0;

    /* Initial phase offset */
    int32_t x0_q4 = (filter_x - filter_x_base) / taps;
    int32_t x_q4 = x0_q4;

    for (x = 0; x < w; ++x) {
      /* Per-pixel src offset */
      int32_t src_x = (x_q4 - x0_q4) >> 4;

      for (sum = 0, k = 0; k < taps; ++k) {
        sum += src[src_x + k] * filter_x[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[x] = clip_pixel(sum >> VP9_FILTER_SHIFT);

      /* Adjust source and filter to use for the next pixel */
      x_q4 += x_step_q4;
      filter_x = filter_x_base + (x_q4 & 0xf) * taps;
    }
    src += src_stride;
    dst += dst_stride;
  }
}

static void convolve_vert_c_dspr2(const uint8_t *src, int32_t src_stride,
                                  uint8_t *dst, int32_t dst_stride,
                                  const int16_t *filter_x, int32_t x_step_q4,
                                  const int16_t *filter_y0, int32_t y_step_q4,
                                  int32_t w, int32_t h, int32_t taps) {
  int32_t x, y, k, sum;
  const int16_t *filter_y_base = filter_y0;

#if ALIGN_FILTERS_256
  filter_y_base = (const int16_t *)(((intptr_t)filter_y0) & ~(intptr_t)0xff);
#endif

  /* Adjust base pointer address for this source column */
  src -= src_stride * (taps / 2 - 1);
  for (x = 0; x < w; ++x) {
    /* Pointer to filter to use */
    const int16_t *filter_y = filter_y0;

    /* Initial phase offset */
    int32_t y0_q4 = (filter_y - filter_y_base) / taps;
    int32_t y_q4 = y0_q4;

    for (y = 0; y < h; ++y) {
      /* Per-pixel src offset */
      int32_t src_y = (y_q4 - y0_q4) >> 4;

      for (sum = 0, k = 0; k < taps; ++k) {
        sum += src[(src_y + k) * src_stride] * filter_y[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[y * dst_stride] = clip_pixel(sum >> VP9_FILTER_SHIFT);

      /* Adjust source and filter to use for the next pixel */
      y_q4 += y_step_q4;
      filter_y = filter_y_base + (y_q4 & 0xf) * taps;
    }
    ++src;
    ++dst;
  }
}

void vp9_convolve8_horiz_dspr2(const uint8_t *src, int src_stride,
                               uint8_t *dst, int dst_stride,
                               const int16_t *filter_x, int x_step_q4,
                               const int16_t *filter_y, int y_step_q4,
                               int w, int h) {
  uint32_t pos = 16;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if(16 == x_step_q4) {
    switch(w) {
      case 4:
        convolve_horiz_4_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride, filter_x, (int32_t)h);
        break;
      case 8:
        convolve_horiz_8_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride, filter_x, (int32_t)h);
        break;
      case 16:
        convolve_horiz_16_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride, filter_x, (int32_t)h, 1);
        break;
      case 32:
        convolve_horiz_16_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride, filter_x, (int32_t)h, 2);
        break;
      case 64:
        convolve_horiz_16_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride, filter_x, (int32_t)h, 4);
        break;
      default:
        convolve_horiz_c_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride,
                               filter_x, (int32_t)x_step_q4, (const int16_t *)filter_y, (int32_t)y_step_q4,
                               (int32_t)w, (int32_t)h, 8);
        break;
    }
  } else {
    convolve_horiz_c_dspr2(src, (int32_t)src_stride, dst, (int32_t)dst_stride,
                           filter_x, (int32_t)x_step_q4, (const int16_t *)filter_y, (int32_t)y_step_q4,
                           (int32_t)w, (int32_t)h, 8);
  }
}

void vp9_convolve8_dspr2(const uint8_t *src, int src_stride,
                         uint8_t *dst, int dst_stride,
                         const int16_t *filter_x, int x_step_q4,
                         const int16_t *filter_y, int y_step_q4,
                         int w, int h) {

  /* Fixed size intermediate buffer places limits on parameters.
   * Maximum intermediate_height is 135, for y_step_q4 == 32,
   * h == 64
   */
  uint8_t temp[64 * 135];
  int32_t intermediate_height = ((h * y_step_q4) >> 4) + 7;
  uint32_t pos = 16;

  assert(w <= 64);
  assert(h <= 64);
  assert(y_step_q4 <= 32);

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if (intermediate_height < h)
    intermediate_height = h;

  if(16 == x_step_q4) {
    switch(w) {
      case 4:
        convolve_horiz_4_transposed_dspr2(src - src_stride * 3, src_stride,
                                          temp, intermediate_height, filter_x, intermediate_height);
        break;
      case 8:
        convolve_horiz_8_transposed_dspr2(src - src_stride * 3, src_stride,
                                          temp, intermediate_height, filter_x, intermediate_height);
        break;
      case 16:
      case 32:
      case 64:
        convolve_horiz_16_transposed_dspr2(src - src_stride * 3, src_stride,
                                           temp, intermediate_height, filter_x, intermediate_height, (w/16));
        break;
      default:
        convolve_horiz_c_dspr2(src - src_stride * 3, src_stride,
                               temp, 64,
                               filter_x, x_step_q4, filter_y, y_step_q4,
                               w, intermediate_height, 8);
        break;
    }

    switch(h) {
      case 4:
        convolve_horiz_4_transposed_dspr2(temp + 3, intermediate_height,
                                          dst, dst_stride, filter_y, w);
        break;
      case 8:
        convolve_horiz_8_transposed_dspr2(temp + 3, intermediate_height,
                                          dst, dst_stride, filter_y, w);
        break;
      case 16:
      case 32:
      case 64:
        convolve_horiz_16_transposed_dspr2(temp + 3, intermediate_height,
                                           dst, dst_stride, filter_y, w, (h/16));
        break;
      default:
        convolve_vert_c_dspr2(temp + 64 * 3, 64, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h, 8);
        break;
    }
  } else {
    convolve_horiz_c_dspr2(src - src_stride * 3, src_stride,
                           temp, 64,
                           filter_x, x_step_q4, filter_y, y_step_q4,
                           w, intermediate_height, 8);

    convolve_vert_c_dspr2(temp + 64 * 3, 64, dst, dst_stride,
                          filter_x, x_step_q4, filter_y, y_step_q4,
                          w, h, 8);
  }
}

void vp9_convolve8_vert_dspr2(const uint8_t *src, int src_stride,
                              uint8_t *dst, int dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y, int y_step_q4,
                              int w, int h) {
  uint32_t pos = 16;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if(16 == y_step_q4) {
    switch(w) {
      case 4 :
      case 8 :
        convolve_vert_4_dspr2(src, src_stride, dst, dst_stride,
                              filter_y, w, h);
        break;
      case 16 :
      case 32 :
      case 64 :
        convolve_vert_16_dspr2(src, src_stride, dst, dst_stride,
                               filter_y, w, h);
        break;
      default:
        convolve_vert_c_dspr2(src, src_stride, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h, 8);
        break;
    }
  } else {
    convolve_vert_c_dspr2(src, src_stride, dst, dst_stride,
                          filter_x, x_step_q4, filter_y, y_step_q4,
                          w, h, 8);
  }
}

static void convolve_avg_horiz_4_dspr2(const uint8_t *src,
                                       int32_t src_stride,
                                       uint8_t *dst,
                                       int32_t dst_stride,
                                       const int16_t *filter_x0,
                                       int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      dst[0] = ((dst[0] + src[0] + 1) >> 1);
      dst[1] = ((dst[1] + src[1] + 1) >> 1);
      dst[2] = ((dst[2] + src[2] + 1) >> 1);
      dst[3] = ((dst[3] + src[3] + 1) >> 1);

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3, Temp4;
    uint32_t vector4a = 64;
    uint32_t tp1, tp2;
    uint32_t p1, p2, p3, p4;
    uint32_t n1, n2, n3, n4;
    uint32_t tn1, tn2;

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp1],         -3(%[src])                     \n\t"
          "ulw              %[tp2],         1(%[src])                      \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tp1]                         \n\t"
          "preceu.ph.qbl    %[p2],          %[tp1]                         \n\t"
          "preceu.ph.qbr    %[p3],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p4],          %[tp2]                         \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]    \n\t"
          "ulw              %[tn2],         5(%[src])                      \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp1],       $ac3,           9              \n\t"

          /* even 2. pixel */
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tn2]                         \n\t"
          "balign           %[tn1],         %[tn2],         3              \n\t"
          "balign           %[tn2],         %[tp2],         3              \n\t"
          "balign           %[tp2],         %[tp1],         3              \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],       $ac2,           9              \n\t"

          "lbu              %[p2],          3(%[dst])                      \n\t"  /* load odd 2 */

          /* odd 1. pixel */
          "lbux             %[tp1],         %[Temp1](%[cm])                \n\t"  /* even 1 */
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "lbu              %[Temp1],       1(%[dst])                      \n\t"  /* load odd 1 */
          "preceu.ph.qbr    %[n1],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[n2],          %[tp2]                         \n\t"
          "preceu.ph.qbr    %[n3],          %[tn2]                         \n\t"
          "preceu.ph.qbl    %[n4],          %[tn2]                         \n\t"
          "dpa.w.ph         $ac3,           %[n1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[n2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[n3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,           %[n4],          %[vector4b]    \n\t"
          "extp             %[Temp2],       $ac3,           9              \n\t"

          "lbu              %[tn2],         0(%[dst])                      \n\t"  /* load even 1 */

          /* odd 2. pixel */
          "lbux             %[tp2],         %[Temp3](%[cm])                \n\t"  /* even 2 */
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[n1],          %[tn1]                         \n\t"
          "lbux             %[tn1],         %[Temp2](%[cm])                \n\t"  /* odd 1 */
          "addqh_r.w        %[tn2],         %[tn2],         %[tp1]         \n\t"  /* average even 1 */
          "dpa.w.ph         $ac2,           %[n2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[n3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[n4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[n1],          %[vector4b]    \n\t"
          "extp             %[Temp4],       $ac2,           9              \n\t"

          "lbu              %[tp1],         2(%[dst])                      \n\t"  /* load even 2 */
          "sb               %[tn2],         0(%[dst])                      \n\t"  /* store even 1 */

          /* clamp */
          "addqh_r.w        %[Temp1],       %[Temp1],       %[tn1]         \n\t"  /* average odd 1 */
          "lbux             %[n2],          %[Temp4](%[cm])                \n\t"  /* odd 2 */
          "sb               %[Temp1],       1(%[dst])                      \n\t"  /* store odd 1 */

          "addqh_r.w        %[tp1],         %[tp1],         %[tp2]         \n\t"  /* average even 2 */
          "sb               %[tp1],         2(%[dst])                      \n\t"  /* store even 2 */

          "addqh_r.w        %[p2],          %[p2],          %[n2]          \n\t"  /* average odd 2 */
          "sb               %[p2],          3(%[dst])                      \n\t"  /* store odd 2 */

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
            [tn1] "=&r" (tn1), [tn2] "=&r" (tn2),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [n1] "=&r" (n1), [n2] "=&r" (n2), [n3] "=&r" (n3), [n4] "=&r" (n4),
            [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3), [Temp4] "=&r" (Temp4)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a),
            [cm] "r" (cm),
            [dst] "r" (dst), [src] "r" (src)
      );

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_avg_horiz_8_dspr2(const uint8_t *src,
                                       int32_t src_stride,
                                       uint8_t *dst,
                                       int32_t dst_stride,
                                       const int16_t *filter_x0,
                                       int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      dst[0] = ((dst[0] + src[0] + 1) >> 1);
      dst[1] = ((dst[1] + src[1] + 1) >> 1);
      dst[2] = ((dst[2] + src[2] + 1) >> 1);
      dst[3] = ((dst[3] + src[3] + 1) >> 1);
      dst[4] = ((dst[4] + src[4] + 1) >> 1);
      dst[5] = ((dst[5] + src[5] + 1) >> 1);
      dst[6] = ((dst[6] + src[6] + 1) >> 1);
      dst[7] = ((dst[7] + src[7] + 1) >> 1);

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2, Temp3;
    uint32_t tp1, tp2;
    uint32_t p1, p2, p3, p4, n1;
    uint32_t tn1, tn2, tn3;
    uint32_t st0, st1;

    vector1b = ((const int32_t *)filter_x0)[0];
    vector2b = ((const int32_t *)filter_x0)[1];
    vector3b = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)src  - 3 + src_stride);

      __asm__ __volatile__ (
          "ulw              %[tp1],         -3(%[src])                     \n\t"
          "ulw              %[tp2],         1(%[src])                      \n\t"

          /* even 1. pixel */
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tp1]                         \n\t"
          "preceu.ph.qbl    %[p2],          %[tp1]                         \n\t"
          "preceu.ph.qbr    %[p3],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p4],          %[tp2]                         \n\t"
          "ulw              %[tn2],         5(%[src])                      \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp1],       $ac3,           9              \n\t"
          "lbu              %[Temp2],       0(%[dst])                      \n\t"
          "lbu              %[tn3],         2(%[dst])                      \n\t"

          /* even 2. pixel */
          "preceu.ph.qbr    %[p1],          %[tn2]                         \n\t"
          "preceu.ph.qbl    %[n1],          %[tn2]                         \n\t"
          "ulw              %[tn1],         9(%[src])                      \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],       $ac2,           9              \n\t"

          /* even 3. pixel */
          "lbux             %[st0],         %[Temp1](%[cm])                \n\t"
          "mtlo             %[vector4a],    $ac1                           \n\t"
          "preceu.ph.qbr    %[p2],          %[tn1]                         \n\t"
          "lbux             %[st1],         %[Temp3](%[cm])                \n\t"
          "dpa.w.ph         $ac1,           %[p3],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac1,           %[p4],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac1,           %[p1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac1,           %[n1],          %[vector4b]    \n\t"
          "extp             %[Temp1],       $ac1,           9              \n\t"

          "addqh_r.w        %[Temp2],       %[Temp2],       %[st0]         \n\t"
          "addqh_r.w        %[tn3],         %[tn3],         %[st1]         \n\t"
          "sb               %[Temp2],       0(%[dst])                      \n\t"
          "sb               %[tn3],         2(%[dst])                      \n\t"

          /* even 4. pixel */
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "mtlo             %[vector4a],    $ac3                           \n\t"

          "balign           %[tn3],         %[tn1],         3              \n\t"
          "balign           %[tn1],         %[tn2],         3              \n\t"
          "balign           %[tn2],         %[tp2],         3              \n\t"
          "balign           %[tp2],         %[tp1],         3              \n\t"

          "lbux             %[st0],         %[Temp1](%[cm])                \n\t"
          "lbu              %[Temp2],       4(%[dst])                      \n\t"
          "addqh_r.w        %[Temp2],       %[Temp2],       %[st0]         \n\t"

          "dpa.w.ph         $ac2,           %[p4],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[n1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector4b]    \n\t"
          "extp             %[Temp3],       $ac2,           9              \n\t"

          /* odd 1. pixel */
          "mtlo             %[vector4a],    $ac1                           \n\t"
          "sb               %[Temp2],       4(%[dst])                      \n\t"
          "preceu.ph.qbr    %[p1],          %[tp2]                         \n\t"
          "preceu.ph.qbl    %[p2],          %[tp2]                         \n\t"
          "preceu.ph.qbr    %[p3],          %[tn2]                         \n\t"
          "preceu.ph.qbl    %[p4],          %[tn2]                         \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p2],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector4b]    \n\t"
          "extp             %[Temp2],       $ac3,           9              \n\t"

          "lbu              %[tp1],         6(%[dst])                      \n\t"

          /* odd 2. pixel */
          "mtlo             %[vector4a],    $ac3                           \n\t"
          "mtlo             %[vector4a],    $ac2                           \n\t"
          "preceu.ph.qbr    %[p1],          %[tn1]                         \n\t"
          "preceu.ph.qbl    %[n1],          %[tn1]                         \n\t"
          "lbux             %[st0],         %[Temp3](%[cm])                \n\t"
          "dpa.w.ph         $ac1,           %[p2],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac1,           %[p3],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac1,           %[p4],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac1,           %[p1],          %[vector4b]    \n\t"
          "extp             %[Temp3],       $ac1,           9              \n\t"

          "lbu              %[tp2],         1(%[dst])                      \n\t"
          "lbu              %[tn2],         3(%[dst])                      \n\t"
          "addqh_r.w        %[tp1],         %[tp1],         %[st0]         \n\t"

          /* odd 3. pixel */
          "lbux             %[st1],         %[Temp2](%[cm])                \n\t"
          "preceu.ph.qbr    %[p2],          %[tn3]                         \n\t"
          "dpa.w.ph         $ac3,           %[p3],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac3,           %[p4],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac3,           %[p1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac3,           %[n1],          %[vector4b]    \n\t"
          "addqh_r.w        %[tp2],         %[tp2],         %[st1]         \n\t"
          "extp             %[Temp2],       $ac3,           9              \n\t"

          "lbu              %[tn3],         5(%[dst])                      \n\t"

          /* odd 4. pixel */
          "sb               %[tp2],         1(%[dst])                      \n\t"
          "sb               %[tp1],         6(%[dst])                      \n\t"
          "dpa.w.ph         $ac2,           %[p4],          %[vector1b]    \n\t"
          "dpa.w.ph         $ac2,           %[p1],          %[vector2b]    \n\t"
          "dpa.w.ph         $ac2,           %[n1],          %[vector3b]    \n\t"
          "dpa.w.ph         $ac2,           %[p2],          %[vector4b]    \n\t"
          "extp             %[Temp1],       $ac2,           9              \n\t"

          "lbu              %[tn1],         7(%[dst])                      \n\t"

          /* clamp */
          "lbux             %[p4],          %[Temp3](%[cm])                \n\t"
          "addqh_r.w        %[tn2],         %[tn2],         %[p4]          \n\t"

          "lbux             %[p2],          %[Temp2](%[cm])                \n\t"
          "addqh_r.w        %[tn3],         %[tn3],         %[p2]          \n\t"

          "lbux             %[n1],          %[Temp1](%[cm])                \n\t"
          "addqh_r.w        %[tn1],         %[tn1],         %[n1]          \n\t"

          /* store bytes */
          "sb               %[tn2],         3(%[dst])                      \n\t"
          "sb               %[tn3],         5(%[dst])                      \n\t"
          "sb               %[tn1],         7(%[dst])                      \n\t"

          : [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
            [tn1] "=&r" (tn1), [tn2] "=&r" (tn2), [tn3] "=&r" (tn3),
            [st0] "=&r" (st0), [st1] "=&r" (st1),
            [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
            [n1] "=&r" (n1),
            [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3)
          : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
            [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
            [vector4a] "r" (vector4a),
            [cm] "r" (cm), [dst] "r" (dst),
            [src] "r" (src)
      );

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

static void convolve_avg_horiz_16_dspr2(const uint8_t *src_ptr,
                                        int32_t src_stride,
                                        uint8_t *dst_ptr,
                                        int32_t dst_stride,
                                        const int16_t *filter_x0,
                                        int32_t h,
                                        int32_t count) {
  int32_t y;
  int32_t c;
  const uint8_t *src;
  uint8_t *dst;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t filter78;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  filter78 = ((const int32_t *)filter_x0)[3];

  /* copy the src to det */
  if(filter78 == 0) {
    for (y = h; y --;) {
      src = src_ptr;
      dst = dst_ptr;

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr + src_stride));

      for (c = 0; c < count; c++) {
        dst[0]  = ((dst[0]  + src[0]  + 1) >> 1);
        dst[1]  = ((dst[1]  + src[1]  + 1) >> 1);
        dst[2]  = ((dst[2]  + src[2]  + 1) >> 1);
        dst[3]  = ((dst[3]  + src[3]  + 1) >> 1);
        dst[4]  = ((dst[4]  + src[4]  + 1) >> 1);
        dst[5]  = ((dst[5]  + src[5]  + 1) >> 1);
        dst[6]  = ((dst[6]  + src[6]  + 1) >> 1);
        dst[7]  = ((dst[7]  + src[7]  + 1) >> 1);
        dst[8]  = ((dst[8]  + src[8]  + 1) >> 1);
        dst[9]  = ((dst[9]  + src[9]  + 1) >> 1);
        dst[10] = ((dst[10] + src[10] + 1) >> 1);
        dst[11] = ((dst[11] + src[11] + 1) >> 1);
        dst[12] = ((dst[12] + src[12] + 1) >> 1);
        dst[13] = ((dst[13] + src[13] + 1) >> 1);
        dst[14] = ((dst[14] + src[14] + 1) >> 1);
        dst[15] = ((dst[15] + src[15] + 1) >> 1);

        src += 16;
        dst += 16;
      }

      /* next row... */
      src_ptr += src_stride;
      dst_ptr += dst_stride;
    }
  } else { /* if(filter78 != 0) */
    uint32_t vector_64 = 64;
    int32_t filter12, filter34, filter56;
    int32_t Temp1, Temp2, Temp3;
    uint32_t qload1, qload2, qload3;
    uint32_t p1, p2, p3, p4, p5;
    uint32_t st1, st2, st3;

    filter12 = ((const int32_t *)filter_x0)[0];
    filter34 = ((const int32_t *)filter_x0)[1];
    filter56 = ((const int32_t *)filter_x0)[2];

    for (y = h; y --;) {
      src = src_ptr;
      dst = dst_ptr;

      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src_ptr  - 3 + src_stride));

      for (c = 0; c < count; c++) {
        __asm__ __volatile__ (
            "ulw              %[qload1],    -3(%[src])                   \n\t"
            "ulw              %[qload2],    1(%[src])                    \n\t"

            /* even 1. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 1 */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 2 */
            "preceu.ph.qbr    %[p1],        %[qload1]                    \n\t"
            "preceu.ph.qbl    %[p2],        %[qload1]                    \n\t"
            "preceu.ph.qbr    %[p3],        %[qload2]                    \n\t"
            "preceu.ph.qbl    %[p4],        %[qload2]                    \n\t"
            "ulw              %[qload3],    5(%[src])                    \n\t"
            "dpa.w.ph         $ac1,         %[p1],          %[filter12]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter34]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter56]  \n\t" /* even 1 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter78]  \n\t" /* even 1 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 1 */
            "lbu              %[st2],       0(%[dst])                    \n\t" /* load even 1 from dst */

            /* even 2. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* even 3 */
            "preceu.ph.qbr    %[p1],        %[qload3]                    \n\t"
            "preceu.ph.qbl    %[p5],        %[qload3]                    \n\t"
            "ulw              %[qload1],    9(%[src])                    \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[filter12]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter34]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter56]  \n\t" /* even 1 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter78]  \n\t" /* even 1 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 1 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 1 */

            "lbu              %[qload3],    2(%[dst])                    \n\t" /* load even 2 from dst */

            /* even 3. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 4 */
            "addqh_r.w        %[st2],       %[st2],         %[st1]       \n\t" /* average even 1 */
            "preceu.ph.qbr    %[p2],        %[qload1]                    \n\t"
            "sb               %[st2],       0(%[dst])                    \n\t" /* even 1 */  /* store even 1 to dst */
            "dpa.w.ph         $ac3,         %[p3],          %[filter12]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter34]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter56]  \n\t" /* even 3 */
            "dpa.w.ph         $ac3,         %[p5],          %[filter78]  \n\t" /* even 3 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* even 3 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 1 */

            /* even 4. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 5 */
            "addqh_r.w        %[qload3],    %[qload3],      %[st2]       \n\t" /* average even 2 */
            "preceu.ph.qbl    %[p3],        %[qload1]                    \n\t"
            "sb               %[qload3],    2(%[dst])                    \n\t" /* even 2 */  /* store even 2 to dst */
            "ulw              %[qload2],    13(%[src])                   \n\t"
            "lbu              %[qload3],    4(%[dst])                    \n\t" /* load even 3 from dst */
            "lbu              %[qload1],    6(%[dst])                    \n\t" /* load even 4 from dst */
            "dpa.w.ph         $ac1,         %[p4],          %[filter12]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter34]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter56]  \n\t" /* even 4 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter78]  \n\t" /* even 4 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 4 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* even 3 */

            /* even 5. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* even 6 */
            "addqh_r.w        %[qload3],    %[qload3],      %[st3]       \n\t" /* average even 3 */
            "preceu.ph.qbr    %[p4],        %[qload2]                    \n\t"
            "sb               %[qload3],    4(%[dst])                    \n\t" /* even 3 */  /* store even 3 to dst */
            "dpa.w.ph         $ac2,         %[p1],          %[filter12]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter34]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p2],          %[filter56]  \n\t" /* even 5 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter78]  \n\t" /* even 5 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 5 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 4 */

            /* even 6. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* even 7 */
            "addqh_r.w        %[qload1],    %[qload1],      %[st1]       \n\t" /* average even 4 */
            "preceu.ph.qbl    %[p1],        %[qload2]                    \n\t"
            "sb               %[qload1],    6(%[dst])                    \n\t" /* even 4 */  /* store even 4 to dst */
            "ulw              %[qload3],    17(%[src])                   \n\t"
            "dpa.w.ph         $ac3,         %[p5],          %[filter12]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter34]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter56]  \n\t" /* even 6 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter78]  \n\t" /* even 6 */
            "lbu              %[qload2],    8(%[dst])                    \n\t" /* load even 5 from dst */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* even 6 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 5 */

            /* even 7. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* even 8 */
            "addqh_r.w        %[qload2],    %[qload2],      %[st2]       \n\t" /* average even 5 */
            "preceu.ph.qbr    %[p5],        %[qload3]                    \n\t"
            "sb               %[qload2],    8(%[dst])                    \n\t" /* even 5 */  /* store even 5 to dst */
            "dpa.w.ph         $ac1,         %[p2],          %[filter12]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter34]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter56]  \n\t" /* even 7 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter78]  \n\t" /* even 7 */
            "lbu              %[qload3],    10(%[dst])                   \n\t" /* load even 6 from dst */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* even 7 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* even 6 */

            "lbu              %[st2],       12(%[dst])                   \n\t" /* load even 7 from dst */

            /* even 8. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 1 */
            "addqh_r.w        %[qload3],    %[qload3],      %[st3]       \n\t" /* average even 6 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter12]  \n\t" /* even 8 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter34]  \n\t" /* even 8 */
            "sb               %[qload3],    10(%[dst])                   \n\t" /* even 6 */  /* store even 6 to dst */
            "dpa.w.ph         $ac2,         %[p1],          %[filter56]  \n\t" /* even 8 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter78]  \n\t" /* even 8 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* even 8 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* even 7 */

            /* ODD pixels */
            "ulw              %[qload1],    -2(%[src])                   \n\t"
            "ulw              %[qload2],    2(%[src])                    \n\t"

            "addqh_r.w        %[st2],       %[st2],         %[st1]       \n\t" /* average even 7 */

            /* odd 1. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 2 */
            "preceu.ph.qbr    %[p1],        %[qload1]                    \n\t"
            "preceu.ph.qbl    %[p2],        %[qload1]                    \n\t"
            "preceu.ph.qbr    %[p3],        %[qload2]                    \n\t"
            "preceu.ph.qbl    %[p4],        %[qload2]                    \n\t"
            "sb               %[st2],       12(%[dst])                   \n\t" /* even 7 */  /* store even 7 to dst */
            "ulw              %[qload3],    6(%[src])                    \n\t"
            "dpa.w.ph         $ac3,         %[p1],          %[filter12]  \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter34]  \n\t" /* odd 1 */
            "lbu              %[qload2],    14(%[dst])                   \n\t" /* load even 8 from dst */
            "dpa.w.ph         $ac3,         %[p3],          %[filter56]  \n\t" /* odd 1 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter78]  \n\t" /* odd 1 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 1 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* even 8 */

            "lbu              %[st1],       1(%[dst])                    \n\t" /* load odd 1 from dst */

            /* odd 2. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* odd 3 */
            "addqh_r.w        %[qload2],    %[qload2],      %[st2]       \n\t" /* average even 8 */
            "preceu.ph.qbr    %[p1],        %[qload3]                    \n\t"
            "preceu.ph.qbl    %[p5],        %[qload3]                    \n\t"
            "sb               %[qload2],    14(%[dst])                   \n\t" /* even 8 */  /* store even 8 to dst */
            "ulw              %[qload1],    10(%[src])                   \n\t"
            "dpa.w.ph         $ac1,         %[p2],          %[filter12]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter34]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter56]  \n\t" /* odd 2 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter78]  \n\t" /* odd 2 */
            "lbu              %[qload3],    3(%[dst])                    \n\t" /* load odd 2 from dst */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 2 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 1 */

            /* odd 3. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 4 */
            "addqh_r.w        %[st3],       %[st3],         %[st1]       \n\t" /* average odd 1 */
            "preceu.ph.qbr    %[p2],        %[qload1]                    \n\t"
            "dpa.w.ph         $ac2,         %[p3],          %[filter12]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter34]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p1],          %[filter56]  \n\t" /* odd 3 */
            "dpa.w.ph         $ac2,         %[p5],          %[filter78]  \n\t" /* odd 3 */
            "sb               %[st3],       1(%[dst])                    \n\t" /* odd 1 */  /* store odd 1 to dst */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* odd 3 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 2 */

            /* odd 4. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 5 */
            "addqh_r.w        %[qload3],    %[qload3],      %[st1]       \n\t" /* average odd 2 */
            "preceu.ph.qbl    %[p3],        %[qload1]                    \n\t"
            "sb               %[qload3],    3(%[dst])                    \n\t" /* odd 2 */  /* store odd 2 to dst */
            "lbu              %[qload1],    5(%[dst])                    \n\t" /* load odd 3 from dst */
            "ulw              %[qload2],    14(%[src])                   \n\t"
            "dpa.w.ph         $ac3,         %[p4],          %[filter12]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter34]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p5],          %[filter56]  \n\t" /* odd 4 */
            "dpa.w.ph         $ac3,         %[p2],          %[filter78]  \n\t" /* odd 4 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 4 */
            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* odd 3 */

            "lbu              %[st1],       7(%[dst])                    \n\t" /* load odd 4 from dst */

            /* odd 5. pixel */
            "mtlo             %[vector_64], $ac2                         \n\t" /* odd 6 */
            "addqh_r.w        %[qload1],    %[qload1],      %[st2]       \n\t" /* average odd 3 */
            "preceu.ph.qbr    %[p4],        %[qload2]                    \n\t"
            "sb               %[qload1],    5(%[dst])                    \n\t" /* odd 3 */  /* store odd 3 to dst */
            "dpa.w.ph         $ac1,         %[p1],          %[filter12]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter34]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p2],          %[filter56]  \n\t" /* odd 5 */
            "dpa.w.ph         $ac1,         %[p3],          %[filter78]  \n\t" /* odd 5 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 5 */
            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 4 */

            "lbu              %[qload1],    9(%[dst])                    \n\t" /* load odd 5 from dst */

            /* odd 6. pixel */
            "mtlo             %[vector_64], $ac3                         \n\t" /* odd 7 */
            "addqh_r.w        %[st1],       %[st1],         %[st3]       \n\t" /* average odd 4 */
            "preceu.ph.qbl    %[p1],        %[qload2]                    \n\t"
            "sb               %[st1],       7(%[dst])                    \n\t" /* odd 4 */  /* store odd 4 to dst */
            "ulw              %[qload3],    18(%[src])                   \n\t"
            "dpa.w.ph         $ac2,         %[p5],          %[filter12]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p2],          %[filter34]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p3],          %[filter56]  \n\t" /* odd 6 */
            "dpa.w.ph         $ac2,         %[p4],          %[filter78]  \n\t" /* odd 6 */
            "extp             %[Temp2],     $ac2,           9            \n\t" /* odd 6 */
            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 5 */

            /* odd 7. pixel */
            "mtlo             %[vector_64], $ac1                         \n\t" /* odd 8 */
            "addqh_r.w        %[qload1],    %[qload1],      %[st1]       \n\t" /* average odd 5 */
            "preceu.ph.qbr    %[p5],        %[qload3]                    \n\t"
            "sb               %[qload1],    9(%[dst])                    \n\t" /* odd 5 */  /* store odd 5 to dst */
            "lbu              %[qload2],    11(%[dst])                   \n\t" /* load odd 6 from dst */
            "dpa.w.ph         $ac3,         %[p2],          %[filter12]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p3],          %[filter34]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p4],          %[filter56]  \n\t" /* odd 7 */
            "dpa.w.ph         $ac3,         %[p1],          %[filter78]  \n\t" /* odd 7 */
            "extp             %[Temp3],     $ac3,           9            \n\t" /* odd 7 */

            "lbu              %[qload3],    13(%[dst])                   \n\t" /* load odd 7 from dst */

            /* odd 8. pixel */
            "dpa.w.ph         $ac1,         %[p3],          %[filter12]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p4],          %[filter34]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p1],          %[filter56]  \n\t" /* odd 8 */
            "dpa.w.ph         $ac1,         %[p5],          %[filter78]  \n\t" /* odd 8 */
            "extp             %[Temp1],     $ac1,           9            \n\t" /* odd 8 */

            "lbu              %[qload1],    15(%[dst])                   \n\t" /* load odd 8 from dst */

            "lbux             %[st2],       %[Temp2](%[cm])              \n\t" /* odd 6 */
            "addqh_r.w        %[qload2],    %[qload2],      %[st2]       \n\t" /* average odd 6 */

            "lbux             %[st3],       %[Temp3](%[cm])              \n\t" /* odd 7 */
            "addqh_r.w        %[qload3],    %[qload3],      %[st3]       \n\t" /* average odd 7 */

            "lbux             %[st1],       %[Temp1](%[cm])              \n\t" /* odd 8 */
            "addqh_r.w        %[qload1],    %[qload1],      %[st1]       \n\t" /* average odd 8 */

            "sb               %[qload2],    11(%[dst])                   \n\t" /* odd 6 */  /* store odd 6 to dst */
            "sb               %[qload3],    13(%[dst])                   \n\t" /* odd 7 */  /* store odd 7 to dst */
            "sb               %[qload1],    15(%[dst])                   \n\t" /* odd 8 */  /* store odd 8 to dst */

            : [qload1] "=&r" (qload1), [qload2] "=&r" (qload2), [qload3] "=&r" (qload3),
              [st1] "=&r" (st1), [st2] "=&r" (st2), [st3] "=&r" (st3),
              [p1] "=&r" (p1), [p2] "=&r" (p2), [p3] "=&r" (p3), [p4] "=&r" (p4),
              [p5] "=&r" (p5),
              [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3)
            : [filter12] "r" (filter12), [filter34] "r" (filter34),
              [filter56] "r" (filter56), [filter78] "r" (filter78),
              [vector_64] "r" (vector_64),
              [cm] "r" (cm), [dst] "r" (dst),
              [src] "r" (src)
        );

        src += 16;
        dst += 16;
      }

      /* Next row... */
      src_ptr += src_stride;
      dst_ptr += dst_stride;
    }
  }
}

static void convolve_avg_horiz_c_dspr2(const uint8_t *src, int src_stride,
                                       uint8_t *dst, int dst_stride,
                                       const int16_t *filter_x0, int x_step_q4,
                                       const int16_t *filter_y, int y_step_q4,
                                       int w, int h, int taps) {
  int x, y, k, sum;
  const int16_t *filter_x_base = filter_x0;

#if ALIGN_FILTERS_256
  filter_x_base = (const int16_t *)(((intptr_t)filter_x0) & ~(intptr_t)0xff);
#endif

  /* Adjust base pointer address for this source line */
  src -= taps / 2 - 1;

  for (y = 0; y < h; ++y) {
    /* Pointer to filter to use */
    const int16_t *filter_x = filter_x0;

    /* Initial phase offset */
    int x0_q4 = (filter_x - filter_x_base) / taps;
    int x_q4 = x0_q4;

    for (x = 0; x < w; ++x) {
      /* Per-pixel src offset */
      int src_x = (x_q4 - x0_q4) >> 4;

      for (sum = 0, k = 0; k < taps; ++k) {
        sum += src[src_x + k] * filter_x[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[x] = (dst[x] + clip_pixel(sum >> VP9_FILTER_SHIFT) + 1) >> 1;

      /* Adjust source and filter to use for the next pixel */
      x_q4 += x_step_q4;
      filter_x = filter_x_base + (x_q4 & 0xf) * taps;
    }
    src += src_stride;
    dst += dst_stride;
  }
}

void vp9_convolve8_avg_horiz_dspr2(const uint8_t *src, int src_stride,
                                   uint8_t *dst, int dst_stride,
                                   const int16_t *filter_x, int x_step_q4,
                                   const int16_t *filter_y, int y_step_q4,
                                   int w, int h) {
  uint32_t pos = 16;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if(16 == x_step_q4) {
    switch(w) {
      case 4:
        convolve_avg_horiz_4_dspr2(src, src_stride, dst, dst_stride, filter_x, h);
        break;
      case 8:
        convolve_avg_horiz_8_dspr2(src, src_stride, dst, dst_stride, filter_x, h);
        break;
      case 16:
        convolve_avg_horiz_16_dspr2(src, src_stride, dst, dst_stride, filter_x, h, 1);
        break;
      case 32:
        convolve_avg_horiz_16_dspr2(src, src_stride, dst, dst_stride, filter_x, h, 2);
        break;
      case 64:
        convolve_avg_horiz_16_dspr2(src, src_stride, dst, dst_stride, filter_x, h, 4);
        break;
      default:
        convolve_avg_horiz_c_dspr2(src, src_stride, dst, dst_stride,
                                   filter_x, x_step_q4, filter_y, y_step_q4,
                                   w, h, 8);
        break;
    }
  } else {
    convolve_avg_horiz_c_dspr2(src, src_stride, dst, dst_stride,
                               filter_x, x_step_q4, filter_y, y_step_q4,
                               w, h, 8);
  }
}

static void convolve_avg_vert_c_dspr2(const uint8_t *src, int src_stride,
                                      uint8_t *dst, int dst_stride,
                                      const int16_t *filter_x, int x_step_q4,
                                      const int16_t *filter_y0, int y_step_q4,
                                      int w, int h, int taps) {
  int x, y, k, sum;
  const int16_t *filter_y_base = filter_y0;

#if ALIGN_FILTERS_256
  filter_y_base = (const int16_t *)(((intptr_t)filter_y0) & ~(intptr_t)0xff);
#endif

  /* Adjust base pointer address for this source column */
  src -= src_stride * (taps / 2 - 1);
  for (x = 0; x < w; ++x) {
    /* Pointer to filter to use */
    const int16_t *filter_y = filter_y0;

    /* Initial phase offset */
    int y0_q4 = (filter_y - filter_y_base) / taps;
    int y_q4 = y0_q4;

    for (y = 0; y < h; ++y) {
      /* Per-pixel src offset */
      int src_y = (y_q4 - y0_q4) >> 4;

      for (sum = 0, k = 0; k < taps; ++k) {
        sum += src[(src_y + k) * src_stride] * filter_y[k];
      }
      sum += (VP9_FILTER_WEIGHT >> 1);
      dst[y * dst_stride] =
          (dst[y * dst_stride] + clip_pixel(sum >> VP9_FILTER_SHIFT) + 1) >> 1;

      /* Adjust source and filter to use for the next pixel */
      y_q4 += y_step_q4;
      filter_y = filter_y_base + (y_q4 & 0xf) * taps;
    }
    ++src;
    ++dst;
  }
}

static void convolve_avg_vert_4_dspr2(const uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int16_t *filter_y,
                                      int32_t w,
                                      int32_t h) {
  int32_t y;
  uint8_t *cm = vp9_ff_cropTbl + CROP_WIDTH;
  int32_t vector4b;

  /* x_step_q4 is always 16 .for equal y_crop_width & width. calculated at vp9_setup_scale_factors_for_frame function */
  /* for hv cases, intermediate h is non-multiple of 2 */
  vector4b = ((const int32_t *)filter_y)[3];

  /* copy the src to det */
  if(vector4b == 0) {
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    int32_t x;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));

      for (x = 0; x < w; x += 4) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        dst_ptr[0] = (src_ptr[0] + dst_ptr[0] + 1) >> 1;
        dst_ptr[1] = (src_ptr[1] + dst_ptr[1] + 1) >> 1;
        dst_ptr[2] = (src_ptr[2] + dst_ptr[2] + 1) >> 1;
        dst_ptr[3] = (src_ptr[3] + dst_ptr[3] + 1) >> 1;
      }

      /* next row... */
      src += src_stride;
      dst += dst_stride;
    }
  } else { /* if(vector4b != 0) */
    uint32_t vector4a = 64;
    uint32_t load1, load2, load3, load4;
    uint32_t p1, p2;
    uint32_t n1, n2;
    uint32_t scratch1, scratch2;
    uint32_t store1, store2;
    int32_t vector1b, vector2b, vector3b;
    int32_t Temp1, Temp2;
    const uint8_t *src_ptr;
    uint8_t *dst_ptr;
    int32_t x;

    vector1b = ((const int32_t *)filter_y)[0];
    vector2b = ((const int32_t *)filter_y)[1];
    vector3b = ((const int32_t *)filter_y)[2];

    src -= 3 * src_stride;

    for (y = h; y --;) {
      /* prefetch src data to cache memory */
      vp9_prefetch_load((const uint8_t *)(src + src_stride));
      for (x = 0; x < w; x += 4) {
        src_ptr = src + x;
        dst_ptr = dst + x;

        __asm__ __volatile__ (
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "mtlo             %[vector4a],  $ac0                            \n\t"
            "mtlo             %[vector4a],  $ac1                            \n\t"
            "mtlo             %[vector4a],  $ac2                            \n\t"
            "mtlo             %[vector4a],  $ac3                            \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac0,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector2b]     \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac2,         %[p1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector2b]     \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector1b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector2b]     \n\t"

            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load1],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load2],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load3],     0(%[src_ptr])                   \n\t"
            "add              %[src_ptr],   %[src_ptr],     %[src_stride]   \n\t"
            "ulw              %[load4],     0(%[src_ptr])                   \n\t"

            "preceu.ph.qbr    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbr    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "preceu.ph.qbr    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbr    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */

            "dpa.w.ph         $ac0,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac0,         %[p2],          %[vector4b]     \n\t"
            "extp             %[Temp1],     $ac0,           9               \n\t"
            "dpa.w.ph         $ac1,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac1,         %[n2],          %[vector4b]     \n\t"
            "extp             %[Temp2],     $ac1,           9               \n\t"

            "preceu.ph.qbl    %[scratch1],  %[load1]                        \n\t"
            "preceu.ph.qbl    %[p1],        %[load2]                        \n\t"
            "precrq.ph.w      %[n1],        %[p1],          %[scratch1]     \n\t" /* pixel 2 */
            "append           %[p1],        %[scratch1],    16              \n\t" /* pixel 1 */
            "lbu              %[scratch1],  0(%[dst_ptr])                   \n\t"
            "preceu.ph.qbl    %[scratch2],  %[load3]                        \n\t"
            "preceu.ph.qbl    %[p2],        %[load4]                        \n\t"
            "precrq.ph.w      %[n2],        %[p2],          %[scratch2]     \n\t" /* pixel 2 */
            "append           %[p2],        %[scratch2],    16              \n\t" /* pixel 1 */
            "lbu              %[scratch2],  1(%[dst_ptr])                   \n\t"

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "dpa.w.ph         $ac2,         %[p1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac2,         %[p2],          %[vector4b]     \n\t"
            "addqh_r.w        %[store1],    %[store1],      %[scratch1]     \n\t" /* pixel 1 */
            "extp             %[Temp1],     $ac2,           9               \n\t"

            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"
            "dpa.w.ph         $ac3,         %[n1],          %[vector3b]     \n\t"
            "dpa.w.ph         $ac3,         %[n2],          %[vector4b]     \n\t"
            "addqh_r.w        %[store2],    %[store2],      %[scratch2]     \n\t" /* pixel 2 */
            "extp             %[Temp2],     $ac3,           9               \n\t"
            "lbu              %[scratch1],  2(%[dst_ptr])                   \n\t"

            "sb               %[store1],    0(%[dst_ptr])                   \n\t"
            "sb               %[store2],    1(%[dst_ptr])                   \n\t"
            "lbu              %[scratch2],  3(%[dst_ptr])                   \n\t"

            "lbux             %[store1],    %[Temp1](%[cm])                 \n\t"
            "lbux             %[store2],    %[Temp2](%[cm])                 \n\t"
            "addqh_r.w        %[store1],    %[store1],      %[scratch1]     \n\t" /* pixel 3 */
            "addqh_r.w        %[store2],    %[store2],      %[scratch2]     \n\t" /* pixel 4 */

            "sb               %[store1],    2(%[dst_ptr])                   \n\t"
            "sb               %[store2],    3(%[dst_ptr])                   \n\t" /* 79 cycles */

            : [load1] "=&r" (load1), [load2] "=&r" (load2), [load3] "=&r" (load3), [load4] "=&r" (load4),
                [p1] "=&r" (p1), [p2] "=&r" (p2), [n1] "=&r" (n1), [n2] "=&r" (n2),
                [scratch1] "=&r" (scratch1), [scratch2] "=&r" (scratch2),
                [Temp1] "=&r" (Temp1), [Temp2] "=&r" (Temp2),
                [store1] "=&r" (store1), [store2] "=&r" (store2),
                [src_ptr] "+r" (src_ptr)
            : [vector1b] "r" (vector1b), [vector2b] "r" (vector2b),
                [vector3b] "r" (vector3b), [vector4b] "r" (vector4b),
                [vector4a] "r" (vector4a),
                [src_stride] "r" (src_stride), [cm] "r" (cm), [dst_ptr] "r" (dst_ptr)
        );
      }

      /* Next row... */
      src += src_stride;
      dst += dst_stride;
    }
  }
}

void vp9_convolve8_avg_vert_dspr2(const uint8_t *src, int src_stride,
                                  uint8_t *dst, int dst_stride,
                                  const int16_t *filter_x, int x_step_q4,
                                  const int16_t *filter_y, int y_step_q4,
                                  int w, int h) {
  uint32_t pos = 16;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if(16 == y_step_q4) {
    switch(w) {
      case 4:
      case 8:
      case 16:
      case 32:
      case 64:
        convolve_avg_vert_4_dspr2(src, src_stride, dst, dst_stride,
                                  filter_y, w, h);
        break;
      default:
        convolve_avg_vert_c_dspr2(src, src_stride, dst, dst_stride,
                                  filter_x, x_step_q4, filter_y, y_step_q4,
                                  w, h, 8);
        break;
    }
  } else {
    convolve_avg_vert_c_dspr2(src, src_stride, dst, dst_stride,
                              filter_x, x_step_q4, filter_y, y_step_q4,
                              w, h, 8);
  }
}

void vp9_convolve8_avg_dspr2(const uint8_t *src, int src_stride,
                             uint8_t *dst, int dst_stride,
                             const int16_t *filter_x, int x_step_q4,
                             const int16_t *filter_y, int y_step_q4,
                             int w, int h) {
  /* Fixed size intermediate buffer places limits on parameters. */
  DECLARE_ALIGNED_ARRAY(16, uint8_t, temp, 64 * 135);
  int32_t intermediate_height = ((h * y_step_q4) >> 4) + 7;

  assert(w <= 64);
  assert(h <= 64);

  if (intermediate_height < h)
    intermediate_height = h;

  vp9_convolve8_horiz_dspr2(src - (src_stride * 3), src_stride,
                            temp, 64,
                            filter_x, x_step_q4,
                            filter_y, y_step_q4,
                            w, intermediate_height);

  vp9_convolve8_avg_vert_dspr2(temp + (64*3), 64,
                               dst, dst_stride,
                               filter_x, x_step_q4,
                               filter_y, y_step_q4,
                               w, h);
}

void vp9_convolve_avg_dspr2(const uint8_t *src, ptrdiff_t src_stride,
                            uint8_t *dst, ptrdiff_t dst_stride,
                            const int16_t *filter_x, int filter_x_stride,
                            const int16_t *filter_y, int filter_y_stride,
                            int w, int h) {
  int x, y;
  uint32_t tp1, tp2, tn1;
  uint32_t tp3, tp4, tn2;

  switch(w) {
    case 4:
      /* 1 word storage */
      for (y = h; y > 0; --y) {
      __asm__ __volatile__ (
        "ulw              %[tp1],         0(%[src])      \n\t"
        "ulw              %[tp2],         0(%[dst])      \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "sw               %[tn1],         0(%[dst])      \n\t"  /* store */

        : [tn1] "=&r" (tn1), [tp1] "=&r" (tp1), [tp2] "=&r" (tp2), [dst] "+r" (dst)
        : [src] "r" (src)
        );

        src += src_stride;
        dst += dst_stride;
      }
      break;
    case 8:
      /* 2 word storage */
      for (y = h; y > 0; --y) {
      __asm__ __volatile__ (
        "ulw              %[tp1],         0(%[src])      \n\t"
        "ulw              %[tp2],         0(%[dst])      \n\t"
        "ulw              %[tp3],         4(%[src])      \n\t"
        "ulw              %[tp4],         4(%[dst])      \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "sw               %[tn1],         0(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         4(%[dst])      \n\t"  /* store */

        : [tn1] "=&r" (tn1), [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
          [tn2] "=&r" (tn2), [tp3] "=&r" (tp3), [tp4] "=&r" (tp4),
          [dst] "+r" (dst)
        : [src] "r" (src)
        );
        src += src_stride;
        dst += dst_stride;
      }
      break;
    case 16:
      /* 4 word storage */
      for (y = h; y > 0; --y) {
      __asm__ __volatile__ (
        "ulw              %[tp1],         0(%[src])      \n\t"
        "ulw              %[tp2],         0(%[dst])      \n\t"
        "ulw              %[tp3],         4(%[src])      \n\t"
        "ulw              %[tp4],         4(%[dst])      \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         8(%[src])      \n\t"
        "ulw              %[tp2],         8(%[dst])      \n\t"
        "sw               %[tn1],         0(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         4(%[dst])      \n\t"  /* store */

        "ulw              %[tp3],         12(%[src])     \n\t"
        "ulw              %[tp4],         12(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "sw               %[tn1],         8(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         12(%[dst])     \n\t"  /* store */

        : [tn1] "=&r" (tn1), [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
          [tn2] "=&r" (tn2), [tp3] "=&r" (tp3), [tp4] "=&r" (tp4),
          [dst] "+r" (dst)
        : [src] "r" (src)
        );
        src += src_stride;
        dst += dst_stride;
      }
      break;
    case 32:
      /* 8 word storage */
      for (y = h; y > 0; --y) {
      __asm__ __volatile__ (
        "ulw              %[tp1],         0(%[src])      \n\t"
        "ulw              %[tp2],         0(%[dst])      \n\t"
        "ulw              %[tp3],         4(%[src])      \n\t"
        "ulw              %[tp4],         4(%[dst])      \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         8(%[src])      \n\t"
        "ulw              %[tp2],         8(%[dst])      \n\t"
        "sw               %[tn1],         0(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         4(%[dst])      \n\t"  /* store */

        "ulw              %[tp3],         12(%[src])     \n\t"
        "ulw              %[tp4],         12(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         16(%[src])     \n\t"
        "ulw              %[tp2],         16(%[dst])     \n\t"
        "sw               %[tn1],         8(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         12(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         20(%[src])     \n\t"
        "ulw              %[tp4],         20(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         24(%[src])     \n\t"
        "ulw              %[tp2],         24(%[dst])     \n\t"
        "sw               %[tn1],         16(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         20(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         28(%[src])     \n\t"
        "ulw              %[tp4],         28(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "sw               %[tn1],         24(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         28(%[dst])     \n\t"  /* store */

        : [tn1] "=&r" (tn1), [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
          [tn2] "=&r" (tn2), [tp3] "=&r" (tp3), [tp4] "=&r" (tp4),
          [dst] "+r" (dst)
        : [src] "r" (src)
        );
        src += src_stride;
        dst += dst_stride;
      }
      break;
    case 64:
      /* 16 word storage */
      for (y = h; y > 0; --y) {
      __asm__ __volatile__ (
        "ulw              %[tp1],         0(%[src])      \n\t"
        "ulw              %[tp2],         0(%[dst])      \n\t"
        "ulw              %[tp3],         4(%[src])      \n\t"
        "ulw              %[tp4],         4(%[dst])      \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         8(%[src])      \n\t"
        "ulw              %[tp2],         8(%[dst])      \n\t"
        "sw               %[tn1],         0(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         4(%[dst])      \n\t"  /* store */

        "ulw              %[tp3],         12(%[src])     \n\t"
        "ulw              %[tp4],         12(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         16(%[src])     \n\t"
        "ulw              %[tp2],         16(%[dst])     \n\t"
        "sw               %[tn1],         8(%[dst])      \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         12(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         20(%[src])     \n\t"
        "ulw              %[tp4],         20(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         24(%[src])     \n\t"
        "ulw              %[tp2],         24(%[dst])     \n\t"
        "sw               %[tn1],         16(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         20(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         28(%[src])     \n\t"
        "ulw              %[tp4],         28(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         32(%[src])     \n\t"
        "ulw              %[tp2],         32(%[dst])     \n\t"
        "sw               %[tn1],         24(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         28(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         36(%[src])     \n\t"
        "ulw              %[tp4],         36(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         40(%[src])     \n\t"
        "ulw              %[tp2],         40(%[dst])     \n\t"
        "sw               %[tn1],         32(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         36(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         44(%[src])     \n\t"
        "ulw              %[tp4],         44(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         48(%[src])     \n\t"
        "ulw              %[tp2],         48(%[dst])     \n\t"
        "sw               %[tn1],         40(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         44(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         52(%[src])     \n\t"
        "ulw              %[tp4],         52(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "ulw              %[tp1],         56(%[src])     \n\t"
        "ulw              %[tp2],         56(%[dst])     \n\t"
        "sw               %[tn1],         48(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         52(%[dst])     \n\t"  /* store */

        "ulw              %[tp3],         60(%[src])     \n\t"
        "ulw              %[tp4],         60(%[dst])     \n\t"
        "adduh_r.qb       %[tn1], %[tp2], %[tp1]         \n\t"  /* average */
        "sw               %[tn1],         56(%[dst])     \n\t"  /* store */
        "adduh_r.qb       %[tn2], %[tp3], %[tp4]         \n\t"  /* average */
        "sw               %[tn2],         60(%[dst])     \n\t"  /* store */

        : [tn1] "=&r" (tn1), [tp1] "=&r" (tp1), [tp2] "=&r" (tp2),
          [tn2] "=&r" (tn2), [tp3] "=&r" (tp3), [tp4] "=&r" (tp4),
          [dst] "+r" (dst)
        : [src] "r" (src)
        );
        src += src_stride;
        dst += dst_stride;
      }
      break;
    default:
      for (y = h; y > 0; --y) {
        for (x = 0; x < w; ++x) {
          dst[x] = (dst[x] + src[x] + 1) >> 1;
        }
        src += src_stride;
        dst += dst_stride;
      }
      break;
  }
}
#endif