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
static void idct8_1d_rows_dspr2(int16_t *input, int16_t *output) {
  int step1_0, step1_1, step1_2, step1_3, step1_4, step1_5, step1_6, step1_7;
  const int const_2_power_13 = 8192;
  int Temp0, Temp1, Temp2, Temp3, Temp4;
  int i;
  int32_t *input_int;

  for(i=0; i<8; ++i) {
    input_int = (int32_t *)input;

    if( !(input_int[0] | input_int[1] | input_int[2] | input_int[3] ) ) {
      input += 8;

      __asm__ __volatile__ (
          "sh  $zero,   0(%[output])  \n\t"
          "sh  $zero,  16(%[output])  \n\t"
          "sh  $zero,  32(%[output])  \n\t"
          "sh  $zero,  48(%[output])  \n\t"
          "sh  $zero,  64(%[output])  \n\t"
          "sh  $zero,  80(%[output])  \n\t"
          "sh  $zero,  96(%[output])  \n\t"
          "sh  $zero, 112(%[output])  \n\t"

          :
          : [output] "r" (output)
      );

      output += 1;

      continue;
    }

    __asm__ __volatile__ (
        /*
          temp_1 = (input[0] + input[4]) * cospi_16_64;
          step2_0 = dct_const_round_shift(temp_1);

          temp_2 = (input[0] - input[4]) * cospi_16_64;
          step2_1 = dct_const_round_shift(temp_2);
        */
        "lh       %[Temp0],             0(%[input])                     \n\t"
        "lh       %[Temp1],             8(%[input])                     \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "add      %[Temp2],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac0,                 %[Temp2],       %[cospi_16_64]  \n\t"
        "extp     %[Temp4],             $ac0,           31              \n\t"

        "sub      %[Temp3],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac1,                 %[Temp3],       %[cospi_16_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "extp     %[Temp2],             $ac1,           31              \n\t"

        /*
          temp_1 = input[2] * cospi_24_64 - input[6] * cospi_8_64;
          step2_2 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             4(%[input])                     \n\t"
        "lh       %[Temp1],             12(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_24_64]  \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_8_64]   \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "extp     %[Temp3],             $ac0,           31              \n\t"

        /*
          step1_1 = step2_1 + step2_2;
          step1_2 = step2_1 - step2_2;
        */
        "add      %[step1_1],           %[Temp2],       %[Temp3]        \n\t"
        "sub      %[step1_2],           %[Temp2],       %[Temp3]        \n\t"

        /*
          temp_2 = input[2] * cospi_8_64 + input[6] * cospi_24_64;
          step2_3 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_8_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_24_64]  \n\t"
        "extp     %[Temp1],             $ac1,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"

        /*
          step1_0 = step2_0 + step2_3;
          step1_3 = step2_0 - step2_3;
        */
        "add      %[step1_0],           %[Temp4],       %[Temp1]        \n\t"
        "sub      %[step1_3],           %[Temp4],       %[Temp1]        \n\t"

        /*
          temp_1 = input[1] * cospi_28_64 - input[7] * cospi_4_64;
          step1_4 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_28_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp1],             14(%[input])                    \n\t"
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_4_64]   \n\t"
        "extp     %[step1_4],           $ac0,           31              \n\t"

        /*
          temp_2 = input[1] * cospi_4_64 + input[7] * cospi_28_64;
          step1_7 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_4_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_28_64]  \n\t"
        "extp     %[step1_7],           $ac1,           31              \n\t"

        /*
          temp_1 = input[5] * cospi_12_64 - input[3] * cospi_20_64;
          step1_5 = dct_const_round_shift(temp_1);
        */
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_12_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_20_64]  \n\t"
        "extp     %[step1_5],           $ac0,           31              \n\t"

        /*
          temp_2 = input[5] * cospi_20_64 + input[3] * cospi_12_64;
          step1_6 = dct_const_round_shift(temp_2);
        */
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac1,                 %[Temp0],       %[cospi_20_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_12_64]  \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        /*
          temp_1 = (step1_7 - step1_6 - step1_4 + step1_5) * cospi_16_64;
          temp_2 = (step1_4 - step1_5 - step1_6 + step1_7) * cospi_16_64;
        */
        "sub      %[Temp0],             %[step1_7],     %[step1_6]      \n\t"
        "sub      %[Temp0],             %[Temp0],       %[step1_4]      \n\t"
        "add      %[Temp0],             %[Temp0],       %[step1_5]      \n\t"
        "sub      %[Temp1],             %[step1_4],     %[step1_5]      \n\t"
        "sub      %[Temp1],             %[Temp1],       %[step1_6]      \n\t"
        "add      %[Temp1],             %[Temp1],       %[step1_7]      \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"

        "madd     $ac0,                 %[Temp0],       %[cospi_16_64]  \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_16_64]  \n\t"

        /*
          step1_4 = step1_4 + step1_5;
          step1_7 = step1_6 + step1_7;
        */
        "add      %[step1_4],           %[step1_4],     %[step1_5]      \n\t"
        "add      %[step1_7],           %[step1_7],     %[step1_6]      \n\t"

        "extp     %[step1_5],           $ac0,           31              \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        "add      %[Temp0],             %[step1_0],     %[step1_7]      \n\t"
        "sh       %[Temp0],             0(%[output])                    \n\t"
        "add      %[Temp1],             %[step1_1],     %[step1_6]      \n\t"
        "sh       %[Temp1],             16(%[output])                   \n\t"
        "add      %[Temp0],             %[step1_2],     %[step1_5]      \n\t"
        "sh       %[Temp0],             32(%[output])                   \n\t"
        "add      %[Temp1],             %[step1_3],     %[step1_4]      \n\t"
        "sh       %[Temp1],             48(%[output])                   \n\t"

        "sub      %[Temp0],             %[step1_3],     %[step1_4]      \n\t"
        "sh       %[Temp0],             64(%[output])                   \n\t"
        "sub      %[Temp1],             %[step1_2],     %[step1_5]      \n\t"
        "sh       %[Temp1],             80(%[output])                   \n\t"
        "sub      %[Temp0],             %[step1_1],     %[step1_6]      \n\t"
        "sh       %[Temp0],             96(%[output])                   \n\t"
        "sub      %[Temp1],             %[step1_0],     %[step1_7]      \n\t"
        "sh       %[Temp1],             112(%[output])                  \n\t"

        : [step1_0] "=&r" (step1_0), [step1_1] "=&r" (step1_1),
          [step1_2] "=&r" (step1_2), [step1_3] "=&r" (step1_3),
          [step1_4] "=&r" (step1_4), [step1_5] "=&r" (step1_5),
          [step1_6] "=&r" (step1_6), [step1_7] "=&r" (step1_7),
          [Temp0] "=&r" (Temp0), [Temp1] "=&r" (Temp1),
          [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3),
          [Temp4] "=&r" (Temp4)
        : [const_2_power_13] "r" (const_2_power_13),
          [cospi_16_64] "r" (cospi_16_64), [cospi_28_64] "r" (cospi_28_64),
          [cospi_4_64] "r" (cospi_4_64), [cospi_12_64] "r" (cospi_12_64),
          [cospi_20_64] "r" (cospi_20_64), [cospi_8_64] "r" (cospi_8_64),
          [cospi_24_64] "r" (cospi_24_64),
          [output] "r" (output), [input] "r" (input)
    );

    input += 8;
    output += 1;
  }
}

void idct8_1d_columns_add_blk_dspr2(int16_t *input, uint8_t *dest, int dest_stride) {
  int step1_0, step1_1, step1_2, step1_3, step1_4, step1_5, step1_6, step1_7;
  int Temp0, Temp1, Temp2, Temp3;
  int i;
  const int const_2_power_13 = 8192;
  uint8_t *dest_pix;
  uint8_t *cm;

  cm = vp9_ff_cropTbl + CROP_WIDTH;

  for(i=0; i<8; ++i) {
      dest_pix = (dest + i);

    __asm__ __volatile__ (
        /*
          temp_1 = (input[0] + input[4]) * cospi_16_64;
          step2_0 = dct_const_round_shift(temp_1);

          temp_2 = (input[0] - input[4]) * cospi_16_64;
          step2_1 = dct_const_round_shift(temp_2);
        */
        "lh       %[Temp0],             0(%[input])                     \n\t"
        "lh       %[Temp1],             8(%[input])                     \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "add      %[Temp2],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac0,                 %[Temp2],       %[cospi_16_64]  \n\t"
        "extp     %[step1_6],           $ac0,           31              \n\t"

        "sub      %[Temp3],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac1,                 %[Temp3],       %[cospi_16_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "extp     %[Temp2],             $ac1,           31              \n\t"

        /*
          temp_1 = input[2] * cospi_24_64 - input[6] * cospi_8_64;
          step2_2 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             4(%[input])                     \n\t"
        "lh       %[Temp1],             12(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_24_64]  \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_8_64]   \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "extp     %[Temp3],             $ac0,           31              \n\t"

        /*
          step1_1 = step2_1 + step2_2;
          step1_2 = step2_1 - step2_2;
        */
        "add      %[step1_1],           %[Temp2],       %[Temp3]        \n\t"
        "sub      %[step1_2],           %[Temp2],       %[Temp3]        \n\t"

        /*
          temp_2 = input[2] * cospi_8_64 + input[6] * cospi_24_64;
          step2_3 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_8_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_24_64]  \n\t"
        "extp     %[Temp1],             $ac1,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"

        /*
          step1_0 = step2_0 + step2_3;
          step1_3 = step2_0 - step2_3;
        */
        "add      %[step1_0],           %[step1_6],     %[Temp1]        \n\t"
        "sub      %[step1_3],           %[step1_6],     %[Temp1]        \n\t"

        /*
          temp_1 = input[1] * cospi_28_64 - input[7] * cospi_4_64;
          step1_4 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_28_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp1],             14(%[input])                    \n\t"
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_4_64]   \n\t"
        "extp     %[step1_4],           $ac0,           31              \n\t"

        /*
          temp_2 = input[1] * cospi_4_64 + input[7] * cospi_28_64;
          step1_7 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_4_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_28_64]  \n\t"
        "extp     %[step1_7],           $ac1,           31              \n\t"

        /*
          temp_1 = input[5] * cospi_12_64 - input[3] * cospi_20_64;
          step1_5 = dct_const_round_shift(temp_1);
        */
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_12_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_20_64]  \n\t"
        "extp     %[step1_5],           $ac0,           31              \n\t"

        /*
          temp_2 = input[5] * cospi_20_64 + input[3] * cospi_12_64;
          step1_6 = dct_const_round_shift(temp_2);
        */
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac1,                 %[Temp0],       %[cospi_20_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_12_64]  \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        /*
          temp_1 = (step1_7 - step1_6 - step1_4 + step1_5) * cospi_16_64;
          temp_2 = (step1_4 - step1_5 - step1_6 + step1_7) * cospi_16_64;
        */
        "sub      %[Temp0],             %[step1_7],     %[step1_6]      \n\t"
        "sub      %[Temp0],             %[Temp0],       %[step1_4]      \n\t"
        "add      %[Temp0],             %[Temp0],       %[step1_5]      \n\t"
        "sub      %[Temp1],             %[step1_4],     %[step1_5]      \n\t"
        "sub      %[Temp1],             %[Temp1],       %[step1_6]      \n\t"
        "add      %[Temp1],             %[Temp1],       %[step1_7]      \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"

        "madd     $ac0,                 %[Temp0],       %[cospi_16_64]  \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_16_64]  \n\t"

        /*
          step1_4 = step1_4 + step1_5;
          step1_7 = step1_6 + step1_7;
        */
        "add      %[step1_4],           %[step1_4],     %[step1_5]      \n\t"
        "add      %[step1_7],           %[step1_7],     %[step1_6]      \n\t"

        "extp     %[step1_5],           $ac0,           31              \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        /* add block */
        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "add      %[Temp0],             %[step1_0],     %[step1_7]      \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "add      %[Temp0],             %[step1_1],     %[step1_6]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "add      %[Temp0],             %[step1_2],     %[step1_5]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "add      %[Temp0],             %[step1_3],     %[step1_4]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "sub      %[Temp0],             %[step1_3],     %[step1_4]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "sub      %[Temp0],             %[step1_2],     %[step1_5]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "sub      %[Temp0],             %[step1_1],     %[step1_6]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "sub      %[Temp0],             %[step1_0],     %[step1_7]      \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"
        "addu     %[dest_pix],          %[dest_pix],    %[dest_stride]  \n\t"

        "lbu      %[Temp1],             0(%[dest_pix])                  \n\t"
        "addi     %[Temp0],             %[Temp0],       16              \n\t"
        "sra      %[Temp0],             %[Temp0],       5               \n\t"
        "add      %[Temp1],             %[Temp1],       %[Temp0]        \n\t"
        "lbux     %[Temp2],             %[Temp1](%[cm])                 \n\t"
        "sb       %[Temp2],             0(%[dest_pix])                  \n\t"

        : [step1_0] "=&r" (step1_0), [step1_1] "=&r" (step1_1),
          [step1_2] "=&r" (step1_2), [step1_3] "=&r" (step1_3),
          [step1_4] "=&r" (step1_4), [step1_5] "=&r" (step1_5),
          [step1_6] "=&r" (step1_6), [step1_7] "=&r" (step1_7),
          [Temp0] "=&r" (Temp0), [Temp1] "=&r" (Temp1),
          [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3),
          [dest_pix] "+r" (dest_pix)
        : [const_2_power_13] "r" (const_2_power_13),
          [cospi_16_64] "r" (cospi_16_64), [cospi_28_64] "r" (cospi_28_64),
          [cospi_4_64] "r" (cospi_4_64), [cospi_12_64] "r" (cospi_12_64),
          [cospi_20_64] "r" (cospi_20_64), [cospi_8_64] "r" (cospi_8_64),
          [cospi_24_64] "r" (cospi_24_64),
          [input] "r" (input), [cm] "r" (cm), [dest_stride] "r" (dest_stride)
    );

    input += 8;
  }
}

void vp9_short_idct8x8_add_dspr2(int16_t *input, uint8_t *dest, int dest_stride) {
  int16_t out[8 * 8];
  int16_t *outptr = out;
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  // First transform rows
  idct8_1d_rows_dspr2(input, outptr);

  // Then transform columns and add to dest
  idct8_1d_columns_add_blk_dspr2(&out[0], dest, dest_stride);
}

static void iadst8_1d_dspr2(int16_t *input, int16_t *output) {
  int s0, s1, s2, s3, s4, s5, s6, s7;
  int x0, x1, x2, x3, x4, x5, x6, x7;

  x0 = input[7];
  x1 = input[0];
  x2 = input[5];
  x3 = input[2];
  x4 = input[3];
  x5 = input[4];
  x6 = input[1];
  x7 = input[6];

  if (!(x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7)) {
    output[0] = output[1] = output[2] = output[3] = output[4]
              = output[5] = output[6] = output[7] = 0;
    return;
  }

  // stage 1
  s0 = cospi_2_64  * x0 + cospi_30_64 * x1;
  s1 = cospi_30_64 * x0 - cospi_2_64  * x1;
  s2 = cospi_10_64 * x2 + cospi_22_64 * x3;
  s3 = cospi_22_64 * x2 - cospi_10_64 * x3;
  s4 = cospi_18_64 * x4 + cospi_14_64 * x5;
  s5 = cospi_14_64 * x4 - cospi_18_64 * x5;
  s6 = cospi_26_64 * x6 + cospi_6_64  * x7;
  s7 = cospi_6_64  * x6 - cospi_26_64 * x7;

  x0 = dct_const_round_shift(s0 + s4);
  x1 = dct_const_round_shift(s1 + s5);
  x2 = dct_const_round_shift(s2 + s6);
  x3 = dct_const_round_shift(s3 + s7);
  x4 = dct_const_round_shift(s0 - s4);
  x5 = dct_const_round_shift(s1 - s5);
  x6 = dct_const_round_shift(s2 - s6);
  x7 = dct_const_round_shift(s3 - s7);

  // stage 2
  s0 = x0;
  s1 = x1;
  s2 = x2;
  s3 = x3;
  s4 =  cospi_8_64  * x4 + cospi_24_64 * x5;
  s5 =  cospi_24_64 * x4 - cospi_8_64  * x5;
  s6 = -cospi_24_64 * x6 + cospi_8_64  * x7;
  s7 =  cospi_8_64  * x6 + cospi_24_64 * x7;

  x0 = s0 + s2;
  x1 = s1 + s3;
  x2 = s0 - s2;
  x3 = s1 - s3;
  x4 = dct_const_round_shift(s4 + s6);
  x5 = dct_const_round_shift(s5 + s7);
  x6 = dct_const_round_shift(s4 - s6);
  x7 = dct_const_round_shift(s5 - s7);

  // stage 3
  s2 = cospi_16_64 * (x2 + x3);
  s3 = cospi_16_64 * (x2 - x3);
  s6 = cospi_16_64 * (x6 + x7);
  s7 = cospi_16_64 * (x6 - x7);

  x2 = dct_const_round_shift(s2);
  x3 = dct_const_round_shift(s3);
  x6 = dct_const_round_shift(s6);
  x7 = dct_const_round_shift(s7);

  output[0] =  x0;
  output[1] = -x4;
  output[2] =  x6;
  output[3] = -x2;
  output[4] =  x3;
  output[5] = -x7;
  output[6] =  x5;
  output[7] = -x1;
}

void vp9_short_iht8x8_add_dspr2(int16_t *input, uint8_t *dest, int dest_stride, int tx_type) {
  int i, j;
  int16_t out[8 * 8];
  int16_t *outptr = out;
  int16_t temp_in[8 * 8], temp_out[8];
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  switch(tx_type) {
    case DCT_DCT:     // DCT in both horizontal and vertical
      idct8_1d_rows_dspr2(input, outptr);
      idct8_1d_columns_add_blk_dspr2(&out[0], dest, dest_stride);
      break;
    case ADST_DCT:    // ADST in vertical, DCT in horizontal
      idct8_1d_rows_dspr2(input, outptr);

      for (i = 0; i < 8; ++i) {
        iadst8_1d_dspr2(&out[i * 8], temp_out);
        for (j = 0; j < 8; ++j)
          dest[j * dest_stride + i] = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 5) + dest[j * dest_stride + i]);
      }
      break;
    case DCT_ADST:    // DCT in vertical, ADST in horizontal
      for (i = 0; i < 8; ++i) {
        iadst8_1d_dspr2(input, outptr);
        input += 8;
        outptr += 8;
      }

      for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
          temp_in[i * 8 + j] = out[j * 8 + i];
        }
      }
      idct8_1d_columns_add_blk_dspr2(&temp_in[0], dest, dest_stride);
      break;
    case ADST_ADST:   // ADST in both directions
      for (i = 0; i < 8; ++i) {
        iadst8_1d_dspr2(input, outptr);
        input += 8;
        outptr += 8;
      }

      for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j)
          temp_in[j] = out[j * 8 + i];
        iadst8_1d_dspr2(temp_in, temp_out);
        for (j = 0; j < 8; ++j)
          dest[j * dest_stride + i] = clip_pixel(ROUND_POWER_OF_TWO(temp_out[j], 5) + dest[j * dest_stride + i]);
      }
      break;

    default:
      printf("vp9_short_iht8x8_add_dspr2 : Invalid tx_type\n");
      break;
  }
}

static void idct8_1d_4rows_dspr2(int16_t *input, int16_t *output) {
  int step1_0, step1_1, step1_2, step1_3, step1_4, step1_5, step1_6, step1_7;
  int Temp0, Temp1, Temp2, Temp3, Temp4;
  int i;
  const int const_2_power_13 = 8192;

  for(i=0; i<4; ++i) {
    __asm__ __volatile__ (
        /*
          temp_1 = (input[0] + input[4]) * cospi_16_64;
          step2_0 = dct_const_round_shift(temp_1);

          temp_2 = (input[0] - input[4]) * cospi_16_64;
          step2_1 = dct_const_round_shift(temp_2);
        */
        "lh       %[Temp0],             0(%[input])                     \n\t"
        "lh       %[Temp1],             8(%[input])                     \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "add      %[Temp2],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac0,                 %[Temp2],       %[cospi_16_64]  \n\t"
        "extp     %[Temp4],             $ac0,           31              \n\t"

        "sub      %[Temp3],             %[Temp0],       %[Temp1]        \n\t"
        "madd     $ac1,                 %[Temp3],       %[cospi_16_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "extp     %[Temp2],             $ac1,           31              \n\t"

        /*
          temp_1 = input[2] * cospi_24_64 - input[6] * cospi_8_64;
          step2_2 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             4(%[input])                     \n\t"
        "lh       %[Temp1],             12(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_24_64]  \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_8_64]   \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "extp     %[Temp3],             $ac0,           31              \n\t"

        /*
          step1_1 = step2_1 + step2_2;
          step1_2 = step2_1 - step2_2;
        */
        "add      %[step1_1],           %[Temp2],       %[Temp3]        \n\t"
        "sub      %[step1_2],           %[Temp2],       %[Temp3]        \n\t"

        /*
          temp_2 = input[2] * cospi_8_64 + input[6] * cospi_24_64;
          step2_3 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_8_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_24_64]  \n\t"
        "extp     %[Temp1],             $ac1,           31              \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"

        /*
          step1_0 = step2_0 + step2_3;
          step1_3 = step2_0 - step2_3;
        */
        "add      %[step1_0],           %[Temp4],       %[Temp1]        \n\t"
        "sub      %[step1_3],           %[Temp4],       %[Temp1]        \n\t"

        /*
          temp_1 = input[1] * cospi_28_64 - input[7] * cospi_4_64;
          step1_4 = dct_const_round_shift(temp_1);
        */
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_28_64]  \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp1],             14(%[input])                    \n\t"
        "lh       %[Temp0],             2(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_4_64]   \n\t"
        "extp     %[step1_4],           $ac0,           31              \n\t"

        /*
          temp_2 = input[1] * cospi_4_64 + input[7] * cospi_28_64;
          step1_7 = dct_const_round_shift(temp_2);
        */
        "madd     $ac1,                 %[Temp0],       %[cospi_4_64]   \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_28_64]  \n\t"
        "extp     %[step1_7],           $ac1,           31              \n\t"

        /*
          temp_1 = input[5] * cospi_12_64 - input[3] * cospi_20_64;
          step1_5 = dct_const_round_shift(temp_1);
        */
        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac0,                 %[Temp0],       %[cospi_12_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "msub     $ac0,                 %[Temp1],       %[cospi_20_64]  \n\t"
        "extp     %[step1_5],           $ac0,           31              \n\t"

        /*
          temp_2 = input[5] * cospi_20_64 + input[3] * cospi_12_64;
          step1_6 = dct_const_round_shift(temp_2);
        */
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"
        "lh       %[Temp0],             10(%[input])                    \n\t"
        "madd     $ac1,                 %[Temp0],       %[cospi_20_64]  \n\t"
        "lh       %[Temp1],             6(%[input])                     \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_12_64]  \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        /*
          temp_1 = (step1_7 - step1_6 - step1_4 + step1_5) * cospi_16_64;
          temp_2 = (step1_4 - step1_5 - step1_6 + step1_7) * cospi_16_64;
        */
        "sub      %[Temp0],             %[step1_7],     %[step1_6]      \n\t"
        "sub      %[Temp0],             %[Temp0],       %[step1_4]      \n\t"
        "add      %[Temp0],             %[Temp0],       %[step1_5]      \n\t"
        "sub      %[Temp1],             %[step1_4],     %[step1_5]      \n\t"
        "sub      %[Temp1],             %[Temp1],       %[step1_6]      \n\t"
        "add      %[Temp1],             %[Temp1],       %[step1_7]      \n\t"

        "mtlo     %[const_2_power_13],  $ac0                            \n\t"
        "mthi     $zero,                $ac0                            \n\t"
        "mtlo     %[const_2_power_13],  $ac1                            \n\t"
        "mthi     $zero,                $ac1                            \n\t"

        "madd     $ac0,                 %[Temp0],       %[cospi_16_64]  \n\t"
        "madd     $ac1,                 %[Temp1],       %[cospi_16_64]  \n\t"

        /*
          step1_4 = step1_4 + step1_5;
          step1_7 = step1_6 + step1_7;
        */
        "add      %[step1_4],           %[step1_4],     %[step1_5]      \n\t"
        "add      %[step1_7],           %[step1_7],     %[step1_6]      \n\t"

        "extp     %[step1_5],           $ac0,           31              \n\t"
        "extp     %[step1_6],           $ac1,           31              \n\t"

        "add      %[Temp0],             %[step1_0],     %[step1_7]      \n\t"
        "sh       %[Temp0],             0(%[output])                    \n\t"
        "add      %[Temp1],             %[step1_1],     %[step1_6]      \n\t"
        "sh       %[Temp1],             16(%[output])                   \n\t"
        "add      %[Temp0],             %[step1_2],     %[step1_5]      \n\t"
        "sh       %[Temp0],             32(%[output])                   \n\t"
        "add      %[Temp1],             %[step1_3],     %[step1_4]      \n\t"
        "sh       %[Temp1],             48(%[output])                   \n\t"

        "sub      %[Temp0],             %[step1_3],     %[step1_4]      \n\t"
        "sh       %[Temp0],             64(%[output])                   \n\t"
        "sub      %[Temp1],             %[step1_2],     %[step1_5]      \n\t"
        "sh       %[Temp1],             80(%[output])                   \n\t"
        "sub      %[Temp0],             %[step1_1],     %[step1_6]      \n\t"
        "sh       %[Temp0],             96(%[output])                   \n\t"
        "sub      %[Temp1],             %[step1_0],     %[step1_7]      \n\t"
        "sh       %[Temp1],             112(%[output])                  \n\t"

        : [step1_0] "=&r" (step1_0), [step1_1] "=&r" (step1_1),
          [step1_2] "=&r" (step1_2), [step1_3] "=&r" (step1_3),
          [step1_4] "=&r" (step1_4), [step1_5] "=&r" (step1_5),
          [step1_6] "=&r" (step1_6), [step1_7] "=&r" (step1_7),
          [Temp0] "=&r" (Temp0), [Temp1] "=&r" (Temp1),
          [Temp2] "=&r" (Temp2), [Temp3] "=&r" (Temp3),
          [Temp4] "=&r" (Temp4)
        : [const_2_power_13] "r" (const_2_power_13),
          [cospi_16_64] "r" (cospi_16_64), [cospi_28_64] "r" (cospi_28_64),
          [cospi_4_64] "r" (cospi_4_64), [cospi_12_64] "r" (cospi_12_64),
          [cospi_20_64] "r" (cospi_20_64), [cospi_8_64] "r" (cospi_8_64),
          [cospi_24_64] "r" (cospi_24_64),
          [output] "r" (output), [input] "r" (input)
    );

    input += 8;
    output += 1;
  }
}

void vp9_short_idct10_8x8_add_dspr2(int16_t *input, uint8_t *dest,
                                int dest_stride) {
  int16_t out[8 * 8];
  int16_t *outptr = out;
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"
    :
    : [pos] "r" (pos)
  );

  // First transform rows
  idct8_1d_4rows_dspr2(input, outptr);

  // Then transform columns and add to dest
  idct8_1d_columns_add_blk_dspr2(&out[0], dest, dest_stride);
}

void vp9_short_idct1_8x8_dspr2(int16_t *input, int16_t *output) {
  int a1, out;
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp    %[pos],    1    \n\t"

    :
    : [pos] "r" (pos)
  );

  DCT_CONST_ROUND_SHIFT_TWICE(input[0], out, DCT_CONST_ROUNDING, cospi_16_64);
  __asm__ __volatile__ (
      "addi     %[out],    %[out],    16      \n\t"
      "sra      %[a1],     %[out],    5       \n\t"

      : [out] "+r" (out), [a1] "=r" (a1)
      :
  );

  output[0] = a1;
}
#endif // #if HAVE_DSPR2
