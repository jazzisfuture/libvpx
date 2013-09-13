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
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_idct.h"
#include "vp9/decoder/vp9_idct_blk.h"
#include "vp9/common/mips/dspr2/vp9_common_dspr2.h"

#if HAVE_DSPR2
void idct16_1d_rows_dspr2(int16_t *input, int16_t *output, uint32_t no_rows) {
  int i;
  int step1_0, step1_1, step1_2, step1_3, step1_4, step1_5, step1_6, step1_7;
  int step1_10, step1_11, step1_12, step1_13;
  int step2_0, step2_1, step2_2, step2_3;
  int step2_8, step2_9, step2_10, step2_11, step2_12, step2_13, step2_14, step2_15;
  int load1, load2, load3, load4, load5, load6, load7, load8;
  int result1, result2, result3, result4;
  const int const_2_power_13 = 8192;

  for (i = no_rows; i--; ) {

    /* prefetch row */
    vp9_prefetch_load((uint8_t *)(input + 16));

    __asm__ __volatile__ (
        "lh       %[load1],              0(%[input])                    \n\t"
        "lh       %[load2],             16(%[input])                    \n\t"
        "lh       %[load3],              8(%[input])                    \n\t"
        "lh       %[load4],             24(%[input])                    \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"
        "add      %[result1],           %[load1],       %[load2]        \n\t"
        "sub      %[result2],           %[load1],       %[load2]        \n\t"
        "madd     $ac1,                 %[result1],     %[cospi_16_64]  \n\t"
        "madd     $ac2,                 %[result2],     %[cospi_16_64]  \n\t"
        "extp     %[step2_0],           $ac1,           31              \n\t"
        "extp     %[step2_1],           $ac2,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"
        "madd     $ac3,                 %[load3],       %[cospi_24_64]  \n\t"
        "msub     $ac3,                 %[load4],       %[cospi_8_64]   \n\t"
        "extp     %[step2_2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "madd     $ac1,                 %[load3],       %[cospi_8_64]   \n\t"
        "madd     $ac1,                 %[load4],       %[cospi_24_64]  \n\t"
        "extp     %[step2_3],           $ac1,           31              \n\t"

        "add      %[step1_0],           %[step2_0],     %[step2_3]      \n\t"
        "add      %[step1_1],           %[step2_1],     %[step2_2]      \n\t"
        "sub      %[step1_2],           %[step2_1],     %[step2_2]      \n\t"
        "sub      %[step1_3],           %[step2_0],     %[step2_3]      \n\t"

        : [load1] "=&r" (load1), [load2] "=&r" (load2),
          [load3] "=&r" (load3), [load4] "=&r" (load4),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [step2_0] "=&r" (step2_0), [step2_1] "=&r" (step2_1),
          [step2_2] "=&r" (step2_2), [step2_3] "=&r" (step2_3),
          [step1_0] "=r" (step1_0), [step1_1] "=r" (step1_1),
          [step1_2] "=r" (step1_2), [step1_3] "=r" (step1_3)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64),
          [cospi_16_64] "r" (cospi_16_64)
    );

    __asm__ __volatile__ (
        "lh       %[load5],             2(%[input])                     \n\t"
        "lh       %[load6],             30(%[input])                    \n\t"
        "lh       %[load7],             18(%[input])                    \n\t"
        "lh       %[load8],             14(%[input])                    \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load5],       %[cospi_30_64]  \n\t"
        "msub     $ac1,                 %[load6],       %[cospi_2_64]   \n\t"
        "extp     %[result1],           $ac1,           31              \n\t"

        "madd     $ac3,                 %[load7],       %[cospi_14_64]  \n\t"
        "msub     $ac3,                 %[load8],       %[cospi_18_64]  \n\t"
        "extp     %[result2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"

        "madd     $ac1,                 %[load7],       %[cospi_18_64]  \n\t"
        "madd     $ac1,                 %[load8],       %[cospi_14_64]  \n\t"
        "extp     %[result3],           $ac1,           31              \n\t"

        "madd     $ac2,                 %[load5],       %[cospi_2_64]   \n\t"
        "madd     $ac2,                 %[load6],       %[cospi_30_64]  \n\t"
        "extp     %[result4],           $ac2,           31              \n\t"

        "sub      %[load5],             %[result1],     %[result2]      \n\t"
        "sub      %[load6],             %[result4],     %[result3]      \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load6],       %[cospi_24_64]  \n\t"
        "msub     $ac1,                 %[load5],       %[cospi_8_64]   \n\t"
        "madd     $ac3,                 %[load5],       %[cospi_24_64]  \n\t"
        "madd     $ac3,                 %[load6],       %[cospi_8_64]   \n\t"

        "extp     %[step2_9],           $ac1,           31              \n\t"
        "extp     %[step2_14],          $ac3,           31              \n\t"
        "add      %[step2_8],           %[result1],     %[result2]      \n\t"
        "add      %[step2_15],          %[result4],     %[result3]      \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [load7] "=&r" (load7), [load8] "=&r" (load8),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step2_8] "=r" (step2_8), [step2_15] "=r" (step2_15),
          [step2_9] "=r" (step2_9), [step2_14] "=r" (step2_14)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_30_64] "r" (cospi_30_64), [cospi_2_64] "r" (cospi_2_64),
          [cospi_14_64] "r" (cospi_14_64), [cospi_18_64] "r" (cospi_18_64),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64)
    );

    __asm__ __volatile__ (
        "lh       %[load1],             10(%[input])                    \n\t"
        "lh       %[load2],             22(%[input])                    \n\t"
        "lh       %[load3],             26(%[input])                    \n\t"
        "lh       %[load4],             6(%[input])                     \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load1],       %[cospi_22_64]  \n\t"
        "msub     $ac1,                 %[load2],       %[cospi_10_64]  \n\t"
        "extp     %[result1],           $ac1,           31              \n\t"

        "madd     $ac3,                 %[load3],       %[cospi_6_64]   \n\t"
        "msub     $ac3,                 %[load4],       %[cospi_26_64]  \n\t"
        "extp     %[result2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"

        "madd     $ac1,                 %[load1],       %[cospi_10_64]  \n\t"
        "madd     $ac1,                 %[load2],       %[cospi_22_64]  \n\t"
        "extp     %[result3],           $ac1,           31              \n\t"

        "madd     $ac2,                 %[load3],       %[cospi_26_64]  \n\t"
        "madd     $ac2,                 %[load4],       %[cospi_6_64]   \n\t"
        "extp     %[result4],           $ac2,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load1],             %[result2],     %[result1]      \n\t"
        "sub      %[load2],             %[result4],     %[result3]      \n\t"

        "msub     $ac1,                 %[load1],       %[cospi_24_64]  \n\t"
        "msub     $ac1,                 %[load2],       %[cospi_8_64]   \n\t"
        "madd     $ac3,                 %[load2],       %[cospi_24_64]  \n\t"
        "msub     $ac3,                 %[load1],       %[cospi_8_64]   \n\t"

        "extp     %[step2_10],          $ac1,           31              \n\t"
        "extp     %[step2_13],          $ac3,           31              \n\t"
        "add      %[step2_11],          %[result1],     %[result2]      \n\t"
        "add      %[step2_12],          %[result4],     %[result3]      \n\t"

        : [load1] "=&r" (load1), [load2] "=&r" (load2),
          [load3] "=&r" (load3), [load4] "=&r" (load4),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step2_10] "=r" (step2_10), [step2_11] "=r" (step2_11),
          [step2_12] "=r" (step2_12), [step2_13] "=r" (step2_13)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_22_64] "r" (cospi_22_64), [cospi_10_64] "r" (cospi_10_64),
          [cospi_6_64] "r" (cospi_6_64), [cospi_26_64] "r" (cospi_26_64),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64)
    );

    __asm__ __volatile__ (
        "lh       %[load5],             4(%[input])                     \n\t"
        "lh       %[load6],             28(%[input])                    \n\t"
        "lh       %[load7],             20(%[input])                    \n\t"
        "lh       %[load8],             12(%[input])                    \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load5],       %[cospi_28_64]  \n\t"
        "msub     $ac1,                 %[load6],       %[cospi_4_64]   \n\t"
        "extp     %[result1],           $ac1,           31              \n\t"

        "madd     $ac3,                 %[load7],       %[cospi_12_64]  \n\t"
        "msub     $ac3,                 %[load8],       %[cospi_20_64]  \n\t"
        "extp     %[result2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"

        "madd     $ac1,                 %[load7],       %[cospi_20_64]  \n\t"
        "madd     $ac1,                 %[load8],       %[cospi_12_64]  \n\t"
        "extp     %[result3],           $ac1,           31              \n\t"

        "madd     $ac2,                 %[load5],       %[cospi_4_64]   \n\t"
        "madd     $ac2,                 %[load6],       %[cospi_28_64]  \n\t"
        "extp     %[result4],           $ac2,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load5],             %[result4],     %[result3]      \n\t"
        "sub      %[load5],             %[load5],       %[result1]      \n\t"
        "add      %[load5],             %[load5],       %[result2]      \n\t"

        "sub      %[load6],             %[result1],     %[result2]      \n\t"
        "sub      %[load6],             %[load6],       %[result3]      \n\t"
        "add      %[load6],             %[load6],       %[result4]      \n\t"

        "madd     $ac1,                 %[load5],       %[cospi_16_64]  \n\t"
        "madd     $ac3,                 %[load6],       %[cospi_16_64]  \n\t"

        "extp     %[step1_5],           $ac1,           31              \n\t"
        "extp     %[step1_6],           $ac3,           31              \n\t"
        "add      %[step1_4],           %[result1],     %[result2]      \n\t"
        "add      %[step1_7],           %[result4],     %[result3]      \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [load7] "=&r" (load7), [load8] "=&r" (load8),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step1_4] "=r" (step1_4), [step1_5] "=r" (step1_5),
          [step1_6] "=r" (step1_6), [step1_7] "=r" (step1_7)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_20_64] "r" (cospi_20_64), [cospi_12_64] "r" (cospi_12_64),
          [cospi_4_64] "r" (cospi_4_64), [cospi_28_64] "r" (cospi_28_64),
          [cospi_16_64] "r" (cospi_16_64)
    );

    __asm__ __volatile__ (
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"

        "sub      %[load5],             %[step2_14],    %[step2_13]     \n\t"
        "sub      %[load5],             %[load5],       %[step2_9]      \n\t"
        "add      %[load5],             %[load5],       %[step2_10]     \n\t"

        "madd     $ac0,                 %[load5],       %[cospi_16_64]  \n\t"

        "sub      %[load6],             %[step2_14],    %[step2_13]     \n\t"
        "sub      %[load6],             %[load6],       %[step2_10]     \n\t"
        "add      %[load6],             %[load6],       %[step2_9]      \n\t"

        "madd     $ac1,                 %[load6],       %[cospi_16_64]  \n\t"

        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load5],             %[step2_15],    %[step2_12]     \n\t"
        "sub      %[load5],             %[load5],       %[step2_8]      \n\t"
        "add      %[load5],             %[load5],       %[step2_11]     \n\t"

        "madd     $ac2,                 %[load5],       %[cospi_16_64]  \n\t"

        "sub      %[load6],             %[step2_15],    %[step2_12]     \n\t"
        "sub      %[load6],             %[load6],       %[step2_11]     \n\t"
        "add      %[load6],             %[load6],       %[step2_8]      \n\t"

        "madd     $ac3,                 %[load6],       %[cospi_16_64]  \n\t"

        "extp     %[step1_10],          $ac0,           31              \n\t"
        "extp     %[step1_13],          $ac1,           31              \n\t"
        "extp     %[step1_11],          $ac2,           31              \n\t"
        "extp     %[step1_12],          $ac3,           31              \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [step1_10] "=r" (step1_10), [step1_11] "=r" (step1_11),
          [step1_12] "=r" (step1_12), [step1_13] "=r" (step1_13)
        : [const_2_power_13] "r" (const_2_power_13),
          [step2_14] "r" (step2_14), [step2_13] "r" (step2_13),
          [step2_9] "r" (step2_9), [step2_10] "r" (step2_10),
          [step2_15] "r" (step2_15), [step2_12] "r" (step2_12),
          [step2_8] "r" (step2_8), [step2_11] "r" (step2_11),
          [cospi_16_64] "r" (cospi_16_64)
    );

    __asm__ __volatile__ (
        "add      %[load5],             %[step1_0],     %[step1_7]      \n\t"
        "add      %[load5],             %[load5],       %[step2_12]     \n\t"
        "add      %[load5],             %[load5],       %[step2_15]     \n\t"
        "add      %[load6],             %[step1_1],     %[step1_6]      \n\t"
        "add      %[load6],             %[load6],       %[step2_13]     \n\t"
        "add      %[load6],             %[load6],       %[step2_14]     \n\t"
        "sh       %[load5],             0(%[output])                    \n\t"
        "sh       %[load6],             32(%[output])                   \n\t"
        "sub      %[load5],             %[step1_1],     %[step1_6]      \n\t"
        "add      %[load5],             %[load5],       %[step2_9]      \n\t"
        "add      %[load5],             %[load5],       %[step2_10]     \n\t"
        "sub      %[load6],             %[step1_0],     %[step1_7]      \n\t"
        "add      %[load6],             %[load6],       %[step2_8]      \n\t"
        "add      %[load6],             %[load6],       %[step2_11]     \n\t"
        "sh       %[load5],             192(%[output])                  \n\t"
        "sh       %[load6],             224(%[output])                  \n\t"
        "sub      %[load5],             %[step1_0],     %[step1_7]      \n\t"
        "sub      %[load5],             %[load5],       %[step2_8]      \n\t"
        "sub      %[load5],             %[load5],       %[step2_11]     \n\t"
        "sub      %[load6],             %[step1_1],     %[step1_6]      \n\t"
        "sub      %[load6],             %[load6],       %[step2_9]      \n\t"
        "sub      %[load6],             %[load6],       %[step2_10]     \n\t"
        "sh       %[load5],             256(%[output])                  \n\t"
        "sh       %[load6],             288(%[output])                  \n\t"
        "add      %[load5],             %[step1_1],     %[step1_6]      \n\t"
        "sub      %[load5],             %[load5],       %[step2_13]     \n\t"
        "sub      %[load5],             %[load5],       %[step2_14]     \n\t"
        "add      %[load6],             %[step1_0],     %[step1_7]      \n\t"
        "sub      %[load6],             %[load6],       %[step2_12]     \n\t"
        "sub      %[load6],             %[load6],       %[step2_15]     \n\t"
        "sh       %[load5],             448(%[output])                  \n\t"
        "sh       %[load6],             480(%[output])                  \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6)
        : [output] "r" (output),
          [step1_0] "r" (step1_0), [step1_1] "r" (step1_1),
          [step1_6] "r" (step1_6), [step1_7] "r" (step1_7),
          [step2_8] "r" (step2_8), [step2_9] "r" (step2_9),
          [step2_10] "r" (step2_10), [step2_11] "r" (step2_11),
          [step2_12] "r" (step2_12), [step2_13] "r" (step2_13),
          [step2_14] "r" (step2_14), [step2_15] "r" (step2_15)
    );

    __asm__ __volatile__ (
        "add      %[load5],             %[step1_2],     %[step1_5]      \n\t"
        "add      %[load5],             %[load5],       %[step1_13]     \n\t"
        "add      %[load6],             %[step1_3],     %[step1_4]      \n\t"
        "add      %[load6],             %[load6],       %[step1_12]     \n\t"
        "sh       %[load5],             64(%[output])                   \n\t"
        "sh       %[load6],             96(%[output])                   \n\t"
        "sub      %[load5],             %[step1_3],     %[step1_4]      \n\t"
        "add      %[load5],             %[load5],       %[step1_11]     \n\t"
        "sub      %[load6],             %[step1_2],     %[step1_5]      \n\t"
        "add      %[load6],             %[load6],       %[step1_10]     \n\t"
        "sh       %[load5],             128(%[output])                  \n\t"
        "sh       %[load6],             160(%[output])                  \n\t"
        "sub      %[load5],             %[step1_2],     %[step1_5]      \n\t"
        "sub      %[load5],             %[load5],       %[step1_10]     \n\t"
        "sub      %[load6],             %[step1_3],     %[step1_4]      \n\t"
        "sub      %[load6],             %[load6],       %[step1_11]     \n\t"
        "sh       %[load5],             320(%[output])                  \n\t"
        "sh       %[load6],             352(%[output])                  \n\t"
        "add      %[load5],             %[step1_3],     %[step1_4]      \n\t"
        "sub      %[load5],             %[load5],       %[step1_12]     \n\t"
        "add      %[load6],             %[step1_2],     %[step1_5]      \n\t"
        "sub      %[load6],             %[load6],       %[step1_13]     \n\t"
        "sh       %[load5],             384(%[output])                  \n\t"
        "sh       %[load6],             416(%[output])                  \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6)
        : [output] "r" (output),
          [step1_2] "r" (step1_2), [step1_3] "r" (step1_3),
          [step1_4] "r" (step1_4), [step1_5] "r" (step1_5),
          [step1_10] "r" (step1_10), [step1_11] "r" (step1_11),
          [step1_12] "r" (step1_12), [step1_13] "r" (step1_13)
    );

    input += 16;
    output += 1;
  }
}

void idct16_1d_cols_add_blk_dspr2(int16_t *input, uint8_t *dest, int dest_stride) {
  int i;
  int step1_0, step1_1, step1_2, step1_3, step1_4, step1_5, step1_6, step1_7;
  int step1_8, step1_9, step1_10, step1_11, step1_12, step1_13, step1_14, step1_15;
  int step2_0, step2_1, step2_2, step2_3;
  int step2_8, step2_9, step2_10, step2_11, step2_12, step2_13, step2_14, step2_15;
  int load1, load2, load3, load4, load5, load6, load7, load8;
  int result1, result2, result3, result4;
  const int const_2_power_13 = 8192;
  uint8_t *dest_pix;
  uint8_t *cm = vp9_ff_cropTbl;

  /* prefetch vp9_ff_cropTbl */
  vp9_prefetch_load(vp9_ff_cropTbl);
  vp9_prefetch_load(vp9_ff_cropTbl +  32);
  vp9_prefetch_load(vp9_ff_cropTbl +  64);
  vp9_prefetch_load(vp9_ff_cropTbl +  96);
  vp9_prefetch_load(vp9_ff_cropTbl + 128);
  vp9_prefetch_load(vp9_ff_cropTbl + 160);
  vp9_prefetch_load(vp9_ff_cropTbl + 192);
  vp9_prefetch_load(vp9_ff_cropTbl + 224);

  for (i = 0; i < 16; ++i) {
    dest_pix = (dest + i);
    __asm__ __volatile__ (
        "lh       %[load1],              0(%[input])                    \n\t"
        "lh       %[load2],             16(%[input])                    \n\t"
        "lh       %[load3],              8(%[input])                    \n\t"
        "lh       %[load4],             24(%[input])                    \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"
        "add      %[result1],           %[load1],       %[load2]        \n\t"
        "sub      %[result2],           %[load1],       %[load2]        \n\t"
        "madd     $ac1,                 %[result1],     %[cospi_16_64]  \n\t"
        "madd     $ac2,                 %[result2],     %[cospi_16_64]  \n\t"
        "extp     %[step2_0],           $ac1,           31              \n\t"
        "extp     %[step2_1],           $ac2,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"
        "madd     $ac3,                 %[load3],       %[cospi_24_64]  \n\t"
        "msub     $ac3,                 %[load4],       %[cospi_8_64]   \n\t"
        "extp     %[step2_2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "madd     $ac1,                 %[load3],       %[cospi_8_64]   \n\t"
        "madd     $ac1,                 %[load4],       %[cospi_24_64]  \n\t"
        "extp     %[step2_3],           $ac1,           31              \n\t"

        "add      %[step1_0],           %[step2_0],     %[step2_3]      \n\t"
        "add      %[step1_1],           %[step2_1],     %[step2_2]      \n\t"
        "sub      %[step1_2],           %[step2_1],     %[step2_2]      \n\t"
        "sub      %[step1_3],           %[step2_0],     %[step2_3]      \n\t"

        : [load1] "=&r" (load1), [load2] "=&r" (load2),
          [load3] "=&r" (load3), [load4] "=&r" (load4),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [step2_0] "=&r" (step2_0), [step2_1] "=&r" (step2_1),
          [step2_2] "=&r" (step2_2), [step2_3] "=&r" (step2_3),
          [step1_0] "=r" (step1_0), [step1_1] "=r" (step1_1),
          [step1_2] "=r" (step1_2), [step1_3] "=r" (step1_3)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64),
          [cospi_16_64] "r" (cospi_16_64)
    );

    __asm__ __volatile__ (
        "lh       %[load5],             2(%[input])                     \n\t"
        "lh       %[load6],             30(%[input])                    \n\t"
        "lh       %[load7],             18(%[input])                    \n\t"
        "lh       %[load8],             14(%[input])                    \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load5],       %[cospi_30_64]  \n\t"
        "msub     $ac1,                 %[load6],       %[cospi_2_64]   \n\t"
        "extp     %[result1],           $ac1,           31              \n\t"

        "madd     $ac3,                 %[load7],       %[cospi_14_64]  \n\t"
        "msub     $ac3,                 %[load8],       %[cospi_18_64]  \n\t"
        "extp     %[result2],           $ac3,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"

        "madd     $ac1,                 %[load7],       %[cospi_18_64]  \n\t"
        "madd     $ac1,                 %[load8],       %[cospi_14_64]  \n\t"
        "extp     %[result3],           $ac1,           31              \n\t"

        "madd     $ac2,                 %[load5],        %[cospi_2_64]  \n\t"
        "madd     $ac2,                 %[load6],        %[cospi_30_64] \n\t"
        "extp     %[result4],           $ac2,            31             \n\t"

        "sub      %[load5],             %[result1],     %[result2]      \n\t"
        "sub      %[load6],             %[result4],     %[result3]      \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load6],       %[cospi_24_64]  \n\t"
        "msub     $ac1,                 %[load5],       %[cospi_8_64]   \n\t"
        "madd     $ac3,                 %[load5],       %[cospi_24_64]  \n\t"
        "madd     $ac3,                 %[load6],       %[cospi_8_64]   \n\t"

        "extp     %[step2_9],           $ac1,           31              \n\t"
        "extp     %[step2_14],          $ac3,           31              \n\t"
        "add      %[step2_8],           %[result1],     %[result2]      \n\t"
        "add      %[step2_15],          %[result4],     %[result3]      \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [load7] "=&r" (load7), [load8] "=&r" (load8),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step2_8] "=r" (step2_8), [step2_15] "=r" (step2_15),
          [step2_9] "=r" (step2_9), [step2_14] "=r" (step2_14)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_30_64] "r" (cospi_30_64), [cospi_2_64] "r" (cospi_2_64),
          [cospi_14_64] "r" (cospi_14_64), [cospi_18_64] "r" (cospi_18_64),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64)
    );

    __asm__ __volatile__ (
        "lh       %[load1],             10(%[input])                    \n\t"
        "lh       %[load2],             22(%[input])                    \n\t"
        "lh       %[load3],             26(%[input])                    \n\t"
        "lh       %[load4],             6(%[input])                     \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "madd     $ac1,                 %[load1],    %[cospi_22_64]     \n\t"
        "msub     $ac1,                 %[load2],    %[cospi_10_64]     \n\t"
        "extp     %[result1],           $ac1,        31                 \n\t"

        "madd     $ac3,                 %[load3],    %[cospi_6_64]      \n\t"
        "msub     $ac3,                 %[load4],    %[cospi_26_64]     \n\t"
        "extp     %[result2],           $ac3,        31                 \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"

        "madd     $ac1,                 %[load1],    %[cospi_10_64]     \n\t"
        "madd     $ac1,                 %[load2],    %[cospi_22_64]     \n\t"
        "extp     %[result3],           $ac1,        31                 \n\t"

        "madd     $ac2,                 %[load3],    %[cospi_26_64]     \n\t"
        "madd     $ac2,                 %[load4],    %[cospi_6_64]      \n\t"
        "extp     %[result4],           $ac2,        31                 \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load1],             %[result2],     %[result1]      \n\t"
        "sub      %[load2],             %[result4],     %[result3]      \n\t"

        "msub     $ac1,                 %[load1],       %[cospi_24_64]  \n\t"
        "msub     $ac1,                 %[load2],       %[cospi_8_64]   \n\t"
        "madd     $ac3,                 %[load2],       %[cospi_24_64]  \n\t"
        "msub     $ac3,                 %[load1],       %[cospi_8_64]   \n\t"

        "extp     %[step2_10],          $ac1,           31              \n\t"
        "extp     %[step2_13],          $ac3,           31              \n\t"
        "add      %[step2_11],          %[result1],     %[result2]      \n\t"
        "add      %[step2_12],          %[result4],     %[result3]      \n\t"

        : [load1] "=&r" (load1), [load2] "=&r" (load2),
          [load3] "=&r" (load3), [load4] "=&r" (load4),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step2_10] "=r" (step2_10), [step2_11] "=r" (step2_11),
          [step2_12] "=r" (step2_12), [step2_13] "=r" (step2_13)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_22_64] "r" (cospi_22_64), [cospi_10_64] "r" (cospi_10_64),
          [cospi_6_64] "r" (cospi_6_64), [cospi_26_64] "r" (cospi_26_64),
          [cospi_24_64] "r" (cospi_24_64), [cospi_8_64] "r" (cospi_8_64)
    );

    __asm__ __volatile__ (
        "lh       %[load5],             4(%[input])                   \n\t"
        "lh       %[load6],             28(%[input])                  \n\t"
        "lh       %[load7],             20(%[input])                  \n\t"
        "lh       %[load8],             12(%[input])                  \n\t"

        "mtlo     %[const_2_power_13],  $ac1                          \n\t"
        "mthi     $zero,                $ac1                          \n\t"
        "mtlo     %[const_2_power_13],  $ac3                          \n\t"
        "mthi     $zero,                $ac3                          \n\t"

        "madd     $ac1,                 %[load5],    %[cospi_28_64]   \n\t"
        "msub     $ac1,                 %[load6],    %[cospi_4_64]    \n\t"
        "extp     %[result1],           $ac1,        31               \n\t"

        "madd     $ac3,                 %[load7],    %[cospi_12_64]   \n\t"
        "msub     $ac3,                 %[load8],    %[cospi_20_64]   \n\t"
        "extp     %[result2],           $ac3,        31               \n\t"

        "mtlo     %[const_2_power_13],  $ac1                          \n\t"
        "mthi     $zero,                $ac1                          \n\t"
        "mtlo     %[const_2_power_13],  $ac2                          \n\t"
        "mthi     $zero,                $ac2                          \n\t"

        "madd     $ac1,                 %[load7],    %[cospi_20_64]   \n\t"
        "madd     $ac1,                 %[load8],    %[cospi_12_64]   \n\t"
        "extp     %[result3],           $ac1,        31               \n\t"

        "madd     $ac2,                 %[load5],    %[cospi_4_64]    \n\t"
        "madd     $ac2,                 %[load6],    %[cospi_28_64]   \n\t"
        "extp     %[result4],           $ac2,        31               \n\t"

        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load5],             %[result4],     %[result3]      \n\t"
        "sub      %[load5],             %[load5],       %[result1]      \n\t"
        "add      %[load5],             %[load5],       %[result2]      \n\t"

        "sub      %[load6],             %[result1],     %[result2]      \n\t"
        "sub      %[load6],             %[load6],       %[result3]      \n\t"
        "add      %[load6],             %[load6],       %[result4]      \n\t"

        "madd     $ac1,                 %[load5],       %[cospi_16_64]  \n\t"
        "madd     $ac3,                 %[load6],       %[cospi_16_64]  \n\t"

        "extp     %[step1_5],           $ac1,           31              \n\t"
        "extp     %[step1_6],           $ac3,           31              \n\t"

        "add      %[step1_4],           %[result1],     %[result2]      \n\t"
        "add      %[step1_7],           %[result4],     %[result3]      \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [load7] "=&r" (load7), [load8] "=&r" (load8),
          [result1] "=&r" (result1), [result2] "=&r" (result2),
          [result3] "=&r" (result3), [result4] "=&r" (result4),
          [step1_4] "=r" (step1_4), [step1_5] "=r" (step1_5),
          [step1_6] "=r" (step1_6), [step1_7] "=r" (step1_7)
        : [const_2_power_13] "r" (const_2_power_13), [input] "r" (input),
          [cospi_20_64] "r" (cospi_20_64), [cospi_12_64] "r" (cospi_12_64),
          [cospi_4_64] "r" (cospi_4_64), [cospi_28_64] "r" (cospi_28_64),
          [cospi_16_64] "r" (cospi_16_64)
    );

    __asm__ __volatile__ (
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"

        "sub      %[load5],             %[step2_14],    %[step2_13]     \n\t"
        "sub      %[load5],             %[load5],       %[step2_9]      \n\t"
        "add      %[load5],             %[load5],       %[step2_10]     \n\t"

        "madd     $ac0,                 %[load5],       %[cospi_16_64]  \n\t"

        "sub      %[load6],             %[step2_14],    %[step2_13]     \n\t"
        "sub      %[load6],             %[load6],       %[step2_10]     \n\t"
        "add      %[load6],             %[load6],       %[step2_9]      \n\t"

        "madd     $ac1,                 %[load6],       %[cospi_16_64]  \n\t"

        "mtlo     %[const_2_power_13],  $ac2                            \n\t"
        "mthi     $zero,                $ac2                            \n\t"
        "mtlo     %[const_2_power_13],  $ac3                            \n\t"
        "mthi     $zero,                $ac3                            \n\t"

        "sub      %[load5],             %[step2_15],    %[step2_12]     \n\t"
        "sub      %[load5],             %[load5],       %[step2_8]      \n\t"
        "add      %[load5],             %[load5],       %[step2_11]     \n\t"

        "madd     $ac2,                 %[load5],       %[cospi_16_64]  \n\t"

        "sub      %[load6],             %[step2_15],    %[step2_12]     \n\t"
        "sub      %[load6],             %[load6],       %[step2_11]     \n\t"
        "add      %[load6],             %[load6],       %[step2_8]      \n\t"

        "madd     $ac3,                 %[load6],       %[cospi_16_64]  \n\t"

        "extp     %[step1_10],          $ac0,           31              \n\t"
        "extp     %[step1_13],          $ac1,           31              \n\t"
        "extp     %[step1_11],          $ac2,           31              \n\t"
        "extp     %[step1_12],          $ac3,           31              \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6),
          [step1_10] "=r" (step1_10), [step1_11] "=r" (step1_11),
          [step1_12] "=r" (step1_12), [step1_13] "=r" (step1_13)
        : [const_2_power_13] "r" (const_2_power_13),
          [step2_14] "r" (step2_14), [step2_13] "r" (step2_13),
          [step2_9] "r" (step2_9), [step2_10] "r" (step2_10),
          [step2_15] "r" (step2_15), [step2_12] "r" (step2_12),
          [step2_8] "r" (step2_8), [step2_11] "r" (step2_11),
          [cospi_16_64] "r" (cospi_16_64)
    );

    step1_8 = step2_8 + step2_11;
    step1_9 = step2_9 + step2_10;
    step1_14 = step2_13 + step2_14;
    step1_15 = step2_12 + step2_15;

    __asm__ __volatile__ (
        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "add      %[load5],         %[step1_0],         %[step1_7]      \n\t"
        "add      %[load5],         %[load5],           %[step1_15]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "add      %[load6],         %[step1_1],         %[step1_6]      \n\t"
        "add      %[load6],         %[load6],           %[step1_14]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "add      %[load5],         %[step1_2],         %[step1_5]      \n\t"
        "add      %[load5],         %[load5],           %[step1_13]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "add      %[load6],         %[step1_3],         %[step1_4]      \n\t"
        "add      %[load6],         %[load6],           %[step1_12]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "sub      %[load5],         %[step1_3],         %[step1_4]      \n\t"
        "add      %[load5],         %[load5],           %[step1_11]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "sub      %[load6],         %[step1_2],         %[step1_5]      \n\t"
        "add      %[load6],         %[load6],           %[step1_10]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "sub      %[load5],         %[step1_1],         %[step1_6]      \n\t"
        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "add      %[load5],         %[load5],           %[step1_9]      \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "sub      %[load6],         %[step1_0],         %[step1_7]      \n\t"
        "add      %[load6],         %[load6],           %[step1_8]      \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "sub      %[load5],         %[step1_0],         %[step1_7]      \n\t"
        "sub      %[load5],         %[load5],           %[step1_8]      \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "sub      %[load6],         %[step1_1],         %[step1_6]      \n\t"
        "sub      %[load6],         %[load6],           %[step1_9]      \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "sub      %[load5],         %[step1_2],         %[step1_5]      \n\t"
        "sub      %[load5],         %[load5],           %[step1_10]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "sub      %[load6],         %[step1_3],         %[step1_4]      \n\t"
        "sub      %[load6],         %[load6],           %[step1_11]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "add      %[load5],         %[step1_3],         %[step1_4]      \n\t"
        "sub      %[load5],         %[load5],           %[step1_12]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "add      %[load6],         %[step1_2],         %[step1_5]      \n\t"
        "sub      %[load6],         %[load6],           %[step1_13]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"

        "lbu      %[load7],         0(%[dest_pix])                      \n\t"
        "add      %[load5],         %[step1_1],         %[step1_6]      \n\t"
        "sub      %[load5],         %[load5],           %[step1_14]     \n\t"
        "addi     %[load5],         %[load5],           32              \n\t"
        "sra      %[load5],         %[load5],           6               \n\t"
        "add      %[load7],         %[load7],           %[load5]        \n\t"
        "lbux     %[load5],         %[load7](%[cm])                     \n\t"
        "add      %[load6],         %[step1_0],         %[step1_7]      \n\t"
        "sub      %[load6],         %[load6],           %[step1_15]     \n\t"
        "sb       %[load5],         0(%[dest_pix])                      \n\t"
        "addu     %[dest_pix],      %[dest_pix],        %[dest_stride]  \n\t"
        "lbu      %[load8],         0(%[dest_pix])                      \n\t"
        "addi     %[load6],         %[load6],           32              \n\t"
        "sra      %[load6],         %[load6],           6               \n\t"
        "add      %[load8],         %[load8],           %[load6]        \n\t"
        "lbux     %[load6],         %[load8](%[cm])                     \n\t"
        "sb       %[load6],         0(%[dest_pix])                      \n\t"

        : [load5] "=&r" (load5), [load6] "=&r" (load6), [load7] "=&r" (load7),
          [load8] "=&r" (load8), [dest_pix] "+r" (dest_pix)
        : [cm] "r" (cm), [dest_stride] "r" (dest_stride),
          [step1_0] "r" (step1_0), [step1_1] "r" (step1_1),
          [step1_2] "r" (step1_2), [step1_3] "r" (step1_3),
          [step1_4] "r" (step1_4), [step1_5] "r" (step1_5),
          [step1_6] "r" (step1_6), [step1_7] "r" (step1_7),
          [step1_8] "r" (step1_8), [step1_9] "r" (step1_9),
          [step1_10] "r" (step1_10), [step1_11] "r" (step1_11),
          [step1_12] "r" (step1_12), [step1_13] "r" (step1_13),
          [step1_14] "r" (step1_14), [step1_15] "r" (step1_15)
    );

    input += 16;
  }
}

void vp9_short_idct16x16_add_dspr2(int16_t *input, uint8_t *dest, int dest_stride) {
  DECLARE_ALIGNED(32, int16_t,  out[16 * 16]);
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  // First transform rows
  idct16_1d_rows_dspr2(input, out, 16);

  // Then transform columns and add to dest
  idct16_1d_cols_add_blk_dspr2(out, dest, dest_stride);
}

void vp9_short_iht16x16_add_dspr2(int16_t *input, uint8_t *dest,
                                  int dest_stride, int tx_type) {
  int i, j;
  DECLARE_ALIGNED(32, int16_t,  out[16 * 16]);
  int16_t *outptr = out;
  int16_t temp_out[16];
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  switch(tx_type) {
    case DCT_DCT:     // DCT in both horizontal and vertical
      idct16_1d_rows_dspr2(input, outptr, 16);
      idct16_1d_cols_add_blk_dspr2(out, dest, dest_stride);
      break;
    case ADST_DCT:    // ADST in vertical, DCT in horizontal
      idct16_1d_rows_dspr2(input, outptr, 16);

      outptr = out;

      for (i = 0; i < 16; ++i) {
        iadst16_1d(outptr, temp_out);

        for (j = 0; j < 16; ++j)
          dest[j * dest_stride + i] = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                                      + dest[j * dest_stride + i]);
        outptr += 16;
      }
      break;
    case DCT_ADST:    // DCT in vertical, ADST in horizontal
    {
      int16_t temp_in[16 * 16];

      for (i = 0; i < 16; ++i) {
        /* prefetch row */
        vp9_prefetch_load((uint8_t *)(input + 16));

        iadst16_1d(input, outptr);
        input += 16;
        outptr += 16;
      }

      for (i = 0; i < 16; ++i)
        for (j = 0; j < 16; ++j)
            temp_in[j * 16 + i] = out[i * 16 + j];

      idct16_1d_cols_add_blk_dspr2(temp_in, dest, dest_stride);
    }
    break;
    case ADST_ADST:   // ADST in both directions
    {
      int16_t temp_in[16];

      for (i = 0; i < 16; ++i) {
        /* prefetch row */
        vp9_prefetch_load((uint8_t *)(input + 16));

        iadst16_1d(input, outptr);
        input += 16;
        outptr += 16;
      }

      for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j)
          temp_in[j] = out[j * 16 + i];
        iadst16_1d(temp_in, temp_out);
        for (j = 0; j < 16; ++j)
          dest[j * dest_stride + i] = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 6)
                                      + dest[j * dest_stride + i]);
      }
    }
    break;
    default:
      printf("vp9_short_iht16x16_add_dspr2 : Invalid tx_type\n");
      break;
  }
}

void vp9_short_idct10_16x16_add_dspr2(int16_t *input, uint8_t *dest,
                                      int dest_stride) {
  DECLARE_ALIGNED(32, int16_t,  out[16 * 16]);
  int16_t *outptr = out;
  uint32_t i;
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  // First transform rows. Since all non-zero dct coefficients are in
  // upper-left 4x4 area, we only need to calculate first 4 rows here.
  idct16_1d_rows_dspr2(input, outptr, 4);

  outptr += 4;
  for (i = 0; i < 6; ++i) {
    __asm__ __volatile__ (
        "sw     $zero,    0(%[outptr])     \n\t"
        "sw     $zero,   32(%[outptr])     \n\t"
        "sw     $zero,   64(%[outptr])     \n\t"
        "sw     $zero,   96(%[outptr])     \n\t"
        "sw     $zero,  128(%[outptr])     \n\t"
        "sw     $zero,  160(%[outptr])     \n\t"
        "sw     $zero,  192(%[outptr])     \n\t"
        "sw     $zero,  224(%[outptr])     \n\t"
        "sw     $zero,  256(%[outptr])     \n\t"
        "sw     $zero,  288(%[outptr])     \n\t"
        "sw     $zero,  320(%[outptr])     \n\t"
        "sw     $zero,  352(%[outptr])     \n\t"
        "sw     $zero,  384(%[outptr])     \n\t"
        "sw     $zero,  416(%[outptr])     \n\t"
        "sw     $zero,  448(%[outptr])     \n\t"
        "sw     $zero,  480(%[outptr])     \n\t"

        :
        : [outptr] "r" (outptr)
    );

    outptr += 2;
  }

  // Then transform columns
  idct16_1d_cols_add_blk_dspr2(out, dest, dest_stride);
}

void vp9_short_idct16x16_1_add_dspr2(int16_t *input, uint8_t *dest, int dest_stride) {
  uint32_t pos = 45;
  int32_t out;
  int32_t r;
  int32_t a1, absa1;
  int32_t vector_a1;
  int32_t t1, t2, t3, t4;
  int32_t vector_1, vector_2, vector_3, vector_4;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"

    :
    : [pos] "r" (pos)
  );

  out = DCT_CONST_ROUND_SHIFT_TWICE_COSPI_16_64(input[0]);
  __asm__ __volatile__ (
      "addi     %[out],     %[out],     32      \n\t"
      "sra      %[a1],      %[out],     6       \n\t"

      : [out] "+r" (out), [a1] "=r" (a1)
      :
  );

  if(a1 < 0) {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "abs        %[absa1],       %[a1]       \n\t"
        "replv.qb   %[vector_a1],   %[absa1]    \n\t"

        : [absa1] "=r" (absa1), [vector_a1] "=r" (vector_a1)
        : [a1] "r" (a1)
    );

    for (r = 16; r--;) {
      __asm__ __volatile__ (
          "lw             %[t1],          0(%[dest])                        \n\t"
          "lw             %[t2],          4(%[dest])                        \n\t"
          "lw             %[t3],          8(%[dest])                        \n\t"
          "lw             %[t4],          12(%[dest])                       \n\t"
          "subu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
          "sw             %[vector_1],    0(%[dest])                        \n\t"
          "sw             %[vector_2],    4(%[dest])                        \n\t"
          "sw             %[vector_3],    8(%[dest])                        \n\t"
          "sw             %[vector_4],    12(%[dest])                       \n\t"
          "add            %[dest],        %[dest],          %[dest_stride]  \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [dest_stride] "r" (dest_stride), [vector_a1] "r" (vector_a1)
      );
    }
  } else {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "replv.qb   %[vector_a1],   %[a1]   \n\t"

        : [vector_a1] "=r" (vector_a1)
        : [a1] "r" (a1)
    );

    for (r = 16; r--;) {
      __asm__ __volatile__ (
          "lw             %[t1],          0(%[dest])                        \n\t"
          "lw             %[t2],          4(%[dest])                        \n\t"
          "lw             %[t3],          8(%[dest])                        \n\t"
          "lw             %[t4],          12(%[dest])                       \n\t"
          "addu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
          "sw             %[vector_1],    0(%[dest])                        \n\t"
          "sw             %[vector_2],    4(%[dest])                        \n\t"
          "sw             %[vector_3],    8(%[dest])                        \n\t"
          "sw             %[vector_4],    12(%[dest])                       \n\t"
          "add            %[dest],        %[dest],          %[dest_stride]  \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [dest_stride] "r" (dest_stride), [vector_a1] "r" (vector_a1)
      );
    }
  }
}
#endif // #if HAVE_DSPR2
