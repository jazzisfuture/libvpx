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
void vp9_add_constant_residual_8x8_dspr2(const int16_t diff, uint8_t *dest, int stride) {
  int32_t r, absa1;
  int32_t t1, t2, vector_a1, vector_1, vector_2;

  if(diff < 0) {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "abs        %[absa1],       %[diff]     \n\t"
        "replv.qb   %[vector_a1],   %[absa1]    \n\t"

        : [absa1] "=r" (absa1), [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
    );

    for (r = 8; r--;) {
      __asm__ __volatile__ (
          "lw           %[t1],          0(%[dest])                      \n\t"
          "lw           %[t2],          4(%[dest])                      \n\t"
          "subu_s.qb    %[vector_1],    %[t1],          %[vector_a1]    \n\t"
          "subu_s.qb    %[vector_2],    %[t2],          %[vector_a1]    \n\t"
          "sw           %[vector_1],    0(%[dest])                      \n\t"
          "sw           %[vector_2],    4(%[dest])                      \n\t"
          "add          %[dest],        %[dest],        %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [dest] "+&r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  } else {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "replv.qb   %[vector_a1],   %[diff]   \n\t"

        : [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
    );

    for (r = 8; r--;) {
      __asm__ __volatile__ (
          "lw           %[t1],          0(%[dest])                      \n\t"
          "lw           %[t2],          4(%[dest])                      \n\t"
          "addu_s.qb    %[vector_1],    %[t1],          %[vector_a1]    \n\t"
          "addu_s.qb    %[vector_2],    %[t2],          %[vector_a1]    \n\t"
          "sw           %[vector_1],    0(%[dest])                      \n\t"
          "sw           %[vector_2],    4(%[dest])                      \n\t"
          "add          %[dest],        %[dest],        %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [dest] "+r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  }
}

void vp9_add_constant_residual_16x16_dspr2(const int16_t diff, uint8_t *dest, int stride) {
  int32_t r, absa1;
  int32_t vector_a1;
  int32_t t1, t2, t3, t4;
  int32_t vector_1, vector_2, vector_3, vector_4;

  if(diff < 0) {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "abs        %[absa1],     %[diff]       \n\t"
        "replv.qb   %[vector_a1], %[absa1]      \n\t"

        : [absa1] "=r" (absa1), [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
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
          "add            %[dest],        %[dest],          %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  } else {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "replv.qb       %[vector_a1],   %[diff]     \n\t"

        : [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
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
          "add            %[dest],        %[dest],          %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  }
}

void vp9_add_constant_residual_32x32_dspr2(const int16_t diff,  uint8_t *dest, int stride) {
  int32_t r, absa1;
  int32_t vector_a1;
  int32_t t1, t2, t3, t4;
  int32_t vector_1, vector_2, vector_3, vector_4;

  if(diff < 0) {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "abs        %[absa1],     %[diff]       \n\t"
        "replv.qb   %[vector_a1], %[absa1]      \n\t"

        : [absa1] "=r" (absa1), [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
    );

    for (r = 32; r--;) {
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

          "lw             %[t1],          16(%[dest])                       \n\t"
          "lw             %[t2],          20(%[dest])                       \n\t"
          "lw             %[t3],          24(%[dest])                       \n\t"
          "lw             %[t4],          28(%[dest])                       \n\t"
          "subu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
          "subu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
          "sw             %[vector_1],    16(%[dest])                       \n\t"
          "sw             %[vector_2],    20(%[dest])                       \n\t"
          "sw             %[vector_3],    24(%[dest])                       \n\t"
          "sw             %[vector_4],    28(%[dest])                       \n\t"

          "add            %[dest],        %[dest],          %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  } else {
    /* use quad-byte
     * input and output memory are four byte aligned */
    __asm__ __volatile__ (
        "replv.qb       %[vector_a1],   %[diff]     \n\t"

        : [vector_a1] "=r" (vector_a1)
        : [diff] "r" (diff)
    );

    for (r = 32; r--;) {
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

          "lw             %[t1],          16(%[dest])                       \n\t"
          "lw             %[t2],          20(%[dest])                       \n\t"
          "lw             %[t3],          24(%[dest])                       \n\t"
          "lw             %[t4],          28(%[dest])                       \n\t"
          "addu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
          "addu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
          "sw             %[vector_1],    16(%[dest])                       \n\t"
          "sw             %[vector_2],    20(%[dest])                       \n\t"
          "sw             %[vector_3],    24(%[dest])                       \n\t"
          "sw             %[vector_4],    28(%[dest])                       \n\t"

          "add            %[dest],        %[dest],          %[stride]       \n\t"

          : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
            [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
            [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
            [dest] "+&r" (dest)
          : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
      );
    }
  }
}

void vp9_idct_add_dspr2(int16_t *input, uint8_t *dest, int stride, int eob) {
  if (eob > 1) {
    vp9_short_idct4x4_add(input, dest, stride);

      __asm__ __volatile__ (
          "sw     $zero,    0(%[input])     \n\t"
          "sw     $zero,    4(%[input])     \n\t"
          "sw     $zero,    8(%[input])     \n\t"
          "sw     $zero,   12(%[input])     \n\t"
          "sw     $zero,   16(%[input])     \n\t"
          "sw     $zero,   20(%[input])     \n\t"
          "sw     $zero,   24(%[input])     \n\t"
          "sw     $zero,   28(%[input])     \n\t"

          :
          : [input] "r" (input)
      );
  } else {
    vp9_short_idct4x4_1_add(input, dest, stride);
    ((int *)input)[0] = 0;
  }
}

void vp9_idct_add_8x8_dspr2(int16_t *input, uint8_t *dest, int stride, int eob) {
  // If dc is 1, then input[0] is the reconstructed value, do not need
  // dequantization. Also, when dc is 1, dc is counted in eobs, namely eobs >=1.

  // The calculation can be simplified if there are not many non-zero dct
  // coefficients. Use eobs to decide what to do.
  // TODO(yunqingwang): "eobs = 1" case is also handled in vp9_short_idct8x8_c.
  // Combine that with code here.
  if (eob) {
    if (eob == 1) {
      // DC only DCT coefficient
      vp9_short_idct8x8_1_add(input, dest, stride);
      input[0] = 0;
    } else if (eob <= 10) {
      vp9_short_idct10_8x8_add(input, dest, stride);
      __asm__ __volatile__ (
          "sw     $zero,    0(%[input])     \n\t"
          "sw     $zero,    4(%[input])     \n\t"
          "sw     $zero,    8(%[input])     \n\t"
          "sw     $zero,   12(%[input])     \n\t"
          "sw     $zero,   16(%[input])     \n\t"
          "sw     $zero,   20(%[input])     \n\t"
          "sw     $zero,   24(%[input])     \n\t"
          "sw     $zero,   28(%[input])     \n\t"

          "sw     $zero,   32(%[input])     \n\t"
          "sw     $zero,   36(%[input])     \n\t"
          "sw     $zero,   40(%[input])     \n\t"
          "sw     $zero,   44(%[input])     \n\t"
          "sw     $zero,   48(%[input])     \n\t"
          "sw     $zero,   52(%[input])     \n\t"
          "sw     $zero,   56(%[input])     \n\t"
          "sw     $zero,   60(%[input])     \n\t"

          "sw     $zero,   64(%[input])     \n\t"
          "sw     $zero,   68(%[input])     \n\t"
          "sw     $zero,   72(%[input])     \n\t"
          "sw     $zero,   76(%[input])     \n\t"
          "sw     $zero,   80(%[input])     \n\t"
          "sw     $zero,   84(%[input])     \n\t"
          "sw     $zero,   88(%[input])     \n\t"
          "sw     $zero,   92(%[input])     \n\t"

          "sw     $zero,   96(%[input])     \n\t"
          "sw     $zero,  100(%[input])     \n\t"
          "sw     $zero,  104(%[input])     \n\t"
          "sw     $zero,  108(%[input])     \n\t"
          "sw     $zero,  112(%[input])     \n\t"
          "sw     $zero,  116(%[input])     \n\t"
          "sw     $zero,  120(%[input])     \n\t"
          "sw     $zero,  124(%[input])     \n\t"

          :
          : [input] "r" (input)
      );
    } else {
      vp9_short_idct8x8_add(input, dest, stride);

      __asm__ __volatile__ (
          "sw     $zero,    0(%[input])     \n\t"
          "sw     $zero,    4(%[input])     \n\t"
          "sw     $zero,    8(%[input])     \n\t"
          "sw     $zero,   12(%[input])     \n\t"
          "sw     $zero,   16(%[input])     \n\t"
          "sw     $zero,   20(%[input])     \n\t"
          "sw     $zero,   24(%[input])     \n\t"
          "sw     $zero,   28(%[input])     \n\t"

          "sw     $zero,   32(%[input])     \n\t"
          "sw     $zero,   36(%[input])     \n\t"
          "sw     $zero,   40(%[input])     \n\t"
          "sw     $zero,   44(%[input])     \n\t"
          "sw     $zero,   48(%[input])     \n\t"
          "sw     $zero,   52(%[input])     \n\t"
          "sw     $zero,   56(%[input])     \n\t"
          "sw     $zero,   60(%[input])     \n\t"

          "sw     $zero,   64(%[input])     \n\t"
          "sw     $zero,   68(%[input])     \n\t"
          "sw     $zero,   72(%[input])     \n\t"
          "sw     $zero,   76(%[input])     \n\t"
          "sw     $zero,   80(%[input])     \n\t"
          "sw     $zero,   84(%[input])     \n\t"
          "sw     $zero,   88(%[input])     \n\t"
          "sw     $zero,   92(%[input])     \n\t"

          "sw     $zero,   96(%[input])     \n\t"
          "sw     $zero,  100(%[input])     \n\t"
          "sw     $zero,  104(%[input])     \n\t"
          "sw     $zero,  108(%[input])     \n\t"
          "sw     $zero,  112(%[input])     \n\t"
          "sw     $zero,  116(%[input])     \n\t"
          "sw     $zero,  120(%[input])     \n\t"
          "sw     $zero,  124(%[input])     \n\t"

          :
          : [input] "r" (input)
      );
    }
  }
}

void vp9_idct_add_16x16_dspr2(int16_t *input, uint8_t *dest, int stride, int eob) {
  int32_t r;
  /* The calculation can be simplified if there are not many non-zero dct
   * coefficients. Use eobs to separate different cases. */
  if (eob) {
    if (eob == 1) {
      /* DC only DCT coefficient. */
      vp9_short_idct16x16_1_add(input, dest, stride);
      input[0] = 0;
    } else if (eob <= 10) {
      vp9_short_idct10_16x16_add(input, dest, stride);
      for (r = 4; r--;) {
        __asm__ __volatile__ (
            "sw     $zero,    0(%[input])     \n\t"
            "sw     $zero,    4(%[input])     \n\t"
            "sw     $zero,    8(%[input])     \n\t"
            "sw     $zero,   12(%[input])     \n\t"
            "sw     $zero,   16(%[input])     \n\t"
            "sw     $zero,   20(%[input])     \n\t"
            "sw     $zero,   24(%[input])     \n\t"
            "sw     $zero,   28(%[input])     \n\t"

            "sw     $zero,   32(%[input])     \n\t"
            "sw     $zero,   36(%[input])     \n\t"
            "sw     $zero,   40(%[input])     \n\t"
            "sw     $zero,   44(%[input])     \n\t"
            "sw     $zero,   48(%[input])     \n\t"
            "sw     $zero,   52(%[input])     \n\t"
            "sw     $zero,   56(%[input])     \n\t"
            "sw     $zero,   60(%[input])     \n\t"

            "sw     $zero,   64(%[input])     \n\t"
            "sw     $zero,   68(%[input])     \n\t"
            "sw     $zero,   72(%[input])     \n\t"
            "sw     $zero,   76(%[input])     \n\t"
            "sw     $zero,   80(%[input])     \n\t"
            "sw     $zero,   84(%[input])     \n\t"
            "sw     $zero,   88(%[input])     \n\t"
            "sw     $zero,   92(%[input])     \n\t"

            "sw     $zero,   96(%[input])     \n\t"
            "sw     $zero,  100(%[input])     \n\t"
            "sw     $zero,  104(%[input])     \n\t"
            "sw     $zero,  108(%[input])     \n\t"
            "sw     $zero,  112(%[input])     \n\t"
            "sw     $zero,  116(%[input])     \n\t"
            "sw     $zero,  120(%[input])     \n\t"
            "sw     $zero,  124(%[input])     \n\t"

            :
            : [input] "r" (input)
        );

        input += 64;
      }
    } else {
      vp9_short_idct16x16_add(input, dest, stride);
      for (r = 4; r--;) {
        __asm__ __volatile__ (
            "sw     $zero,    0(%[input])     \n\t"
            "sw     $zero,    4(%[input])     \n\t"
            "sw     $zero,    8(%[input])     \n\t"
            "sw     $zero,   12(%[input])     \n\t"
            "sw     $zero,   16(%[input])     \n\t"
            "sw     $zero,   20(%[input])     \n\t"
            "sw     $zero,   24(%[input])     \n\t"
            "sw     $zero,   28(%[input])     \n\t"

            "sw     $zero,   32(%[input])     \n\t"
            "sw     $zero,   36(%[input])     \n\t"
            "sw     $zero,   40(%[input])     \n\t"
            "sw     $zero,   44(%[input])     \n\t"
            "sw     $zero,   48(%[input])     \n\t"
            "sw     $zero,   52(%[input])     \n\t"
            "sw     $zero,   56(%[input])     \n\t"
            "sw     $zero,   60(%[input])     \n\t"

            "sw     $zero,   64(%[input])     \n\t"
            "sw     $zero,   68(%[input])     \n\t"
            "sw     $zero,   72(%[input])     \n\t"
            "sw     $zero,   76(%[input])     \n\t"
            "sw     $zero,   80(%[input])     \n\t"
            "sw     $zero,   84(%[input])     \n\t"
            "sw     $zero,   88(%[input])     \n\t"
            "sw     $zero,   92(%[input])     \n\t"

            "sw     $zero,   96(%[input])     \n\t"
            "sw     $zero,  100(%[input])     \n\t"
            "sw     $zero,  104(%[input])     \n\t"
            "sw     $zero,  108(%[input])     \n\t"
            "sw     $zero,  112(%[input])     \n\t"
            "sw     $zero,  116(%[input])     \n\t"
            "sw     $zero,  120(%[input])     \n\t"
            "sw     $zero,  124(%[input])     \n\t"

            :
            : [input] "r" (input)
        );

        input += 64;
      }
    }
  }
}

void vp9_idct_add_32x32_dspr2(int16_t *input, uint8_t *dest, int stride, int eob) {
  int32_t r;
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__ (
    "wrdsp      %[pos],     1           \n\t"
    :
    : [pos] "r" (pos)
  );

  if (eob) {
    if (eob == 1) {
      /* DC only DCT coefficient. */
      int32_t out;
      int32_t a1, absa1;
      int32_t vector_a1;
      int32_t t1, t2, t3, t4;
      int32_t vector_1, vector_2, vector_3, vector_4;

      DCT_CONST_ROUND_SHIFT_TWICE(input[0], out, DCT_CONST_ROUNDING, cospi_16_64);
      __asm__ __volatile__ (
          "addi     %[out],    %[out],    32      \n\t"
          "sra      %[a1],     %[out],    6       \n\t"

          : [out] "+r" (out), [a1] "=r" (a1)
          :
      );

      input[0] = 0;

      if(a1 < 0) {
        /* use quad-byte
         * input and output memory are four byte aligned */
        __asm__ __volatile__ (
            "abs        %[absa1],     %[a1]         \n\t"
            "replv.qb   %[vector_a1], %[absa1]      \n\t"

            : [absa1] "=r" (absa1), [vector_a1] "=r" (vector_a1)
            : [a1] "r" (a1)
        );

        for (r = 32; r--;) {
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

              "lw             %[t1],          16(%[dest])                       \n\t"
              "lw             %[t2],          20(%[dest])                       \n\t"
              "lw             %[t3],          24(%[dest])                       \n\t"
              "lw             %[t4],          28(%[dest])                       \n\t"
              "subu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
              "subu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
              "subu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
              "subu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
              "sw             %[vector_1],    16(%[dest])                       \n\t"
              "sw             %[vector_2],    20(%[dest])                       \n\t"
              "sw             %[vector_3],    24(%[dest])                       \n\t"
              "sw             %[vector_4],    28(%[dest])                       \n\t"

              "add            %[dest],        %[dest],          %[stride]       \n\t"

              : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
                [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
                [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
                [dest] "+&r" (dest)
              : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
          );
        }
      } else {
        /* use quad-byte
         * input and output memory are four byte aligned */
        __asm__ __volatile__ (
            "replv.qb       %[vector_a1],   %[a1]     \n\t"

            : [vector_a1] "=r" (vector_a1)
            : [a1] "r" (a1)
        );

        for (r = 32; r--;) {
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

              "lw             %[t1],          16(%[dest])                       \n\t"
              "lw             %[t2],          20(%[dest])                       \n\t"
              "lw             %[t3],          24(%[dest])                       \n\t"
              "lw             %[t4],          28(%[dest])                       \n\t"
              "addu_s.qb      %[vector_1],    %[t1],            %[vector_a1]    \n\t"
              "addu_s.qb      %[vector_2],    %[t2],            %[vector_a1]    \n\t"
              "addu_s.qb      %[vector_3],    %[t3],            %[vector_a1]    \n\t"
              "addu_s.qb      %[vector_4],    %[t4],            %[vector_a1]    \n\t"
              "sw             %[vector_1],    16(%[dest])                       \n\t"
              "sw             %[vector_2],    20(%[dest])                       \n\t"
              "sw             %[vector_3],    24(%[dest])                       \n\t"
              "sw             %[vector_4],    28(%[dest])                       \n\t"

              "add            %[dest],        %[dest],          %[stride]       \n\t"

              : [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4),
                [vector_1] "=&r" (vector_1), [vector_2] "=&r" (vector_2),
                [vector_3] "=&r" (vector_3), [vector_4] "=&r" (vector_4),
                [dest] "+&r" (dest)
              : [stride] "r" (stride), [vector_a1] "r" (vector_a1)
          );
        }
      }
    } else {
      vp9_short_idct32x32_add(input, dest, stride);

      for (r = 16; r--;) {
        __asm__ __volatile__ (
            "sw     $zero,    0(%[input])     \n\t"
            "sw     $zero,    4(%[input])     \n\t"
            "sw     $zero,    8(%[input])     \n\t"
            "sw     $zero,   12(%[input])     \n\t"
            "sw     $zero,   16(%[input])     \n\t"
            "sw     $zero,   20(%[input])     \n\t"
            "sw     $zero,   24(%[input])     \n\t"
            "sw     $zero,   28(%[input])     \n\t"

            "sw     $zero,   32(%[input])     \n\t"
            "sw     $zero,   36(%[input])     \n\t"
            "sw     $zero,   40(%[input])     \n\t"
            "sw     $zero,   44(%[input])     \n\t"
            "sw     $zero,   48(%[input])     \n\t"
            "sw     $zero,   52(%[input])     \n\t"
            "sw     $zero,   56(%[input])     \n\t"
            "sw     $zero,   60(%[input])     \n\t"

            "sw     $zero,   64(%[input])     \n\t"
            "sw     $zero,   68(%[input])     \n\t"
            "sw     $zero,   72(%[input])     \n\t"
            "sw     $zero,   76(%[input])     \n\t"
            "sw     $zero,   80(%[input])     \n\t"
            "sw     $zero,   84(%[input])     \n\t"
            "sw     $zero,   88(%[input])     \n\t"
            "sw     $zero,   92(%[input])     \n\t"

            "sw     $zero,   96(%[input])     \n\t"
            "sw     $zero,  100(%[input])     \n\t"
            "sw     $zero,  104(%[input])     \n\t"
            "sw     $zero,  108(%[input])     \n\t"
            "sw     $zero,  112(%[input])     \n\t"
            "sw     $zero,  116(%[input])     \n\t"
            "sw     $zero,  120(%[input])     \n\t"
            "sw     $zero,  124(%[input])     \n\t"

            :
            : [input] "r" (input)
        );

        input += 64;
      }
    }
  }
}
#endif // #if HAVE_DSPR2
