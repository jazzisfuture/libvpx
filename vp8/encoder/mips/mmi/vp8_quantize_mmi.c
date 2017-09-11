/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/asmdefs_mmi.h"
#include "vp8/encoder/onyx_int.h"
#include "vp8/encoder/quantize.h"
#include "vp8/common/quant_common.h"

#define SET_EOB(i, value_eob) \
  if (y[i]) {                 \
    *d->eob = value_eob;      \
    return;                   \
  }

#define REGULAR_SELECT_EOB(i, rc)                                        \
  z = coeff_ptr[rc];                                                     \
  sz = (z >> 31);                                                        \
  x = (z ^ sz) - sz;                                                     \
  if (x >= (zbin_ptr[rc] + *(zbin_boost_ptr++) + zbin_oq_value)) {       \
    x += round_ptr[rc];                                                  \
    y = ((((x * quant_ptr[rc]) >> 16) + x) * quant_shift_ptr[rc]) >> 16; \
    x = (y ^ sz) - sz;                                                   \
    qcoeff_ptr[rc] = x;                                                  \
    dqcoeff_ptr[rc] = x * dequant_ptr[rc];                               \
    if (y) {                                                             \
      eob = i;                                                           \
      zbin_boost_ptr = b->zrun_zbin_boost;                               \
    }                                                                    \
  }

void vp8_fast_quantize_b_mmi(BLOCK *b, BLOCKD *d) {
  int16_t *coeff_ptr = b->coeff;
  int16_t *round_ptr = b->round;
  int16_t *quant_ptr = b->quant_fast;
  int16_t *qcoeff_ptr = d->qcoeff;
  int16_t *dqcoeff_ptr = d->dqcoeff;
  int16_t *dequant_ptr = d->dequant;

  double ftmp[10];
  uint64_t tmp[1];
  int16_t y[16];

  __asm__ volatile(
      // loop 0 ~ 7
      "gsldlc1    %[ftmp1],   0x07(%[coeff_ptr])              \n\t"
      "gsldrc1    %[ftmp1],   0x00(%[coeff_ptr])              \n\t"
      "li         %[tmp0],    0x0f                            \n\t"
      "xor        %[ftmp0],   %[ftmp0],       %[ftmp0]        \n\t"
      "mtc1       %[tmp0],    %[ftmp9]                        \n\t"
      "gsldlc1    %[ftmp2],   0x0f(%[coeff_ptr])              \n\t"
      "gsldrc1    %[ftmp2],   0x08(%[coeff_ptr])              \n\t"

      "psrah      %[ftmp3],   %[ftmp1],       %[ftmp9]        \n\t"
      "xor        %[ftmp1],   %[ftmp3],       %[ftmp1]        \n\t"
      "psubh      %[ftmp1],   %[ftmp1],       %[ftmp3]        \n\t"
      "psrah      %[ftmp4],   %[ftmp2],       %[ftmp9]        \n\t"
      "xor        %[ftmp2],   %[ftmp4],       %[ftmp2]        \n\t"
      "psubh      %[ftmp2],   %[ftmp2],       %[ftmp4]        \n\t"

      "gsldlc1    %[ftmp5],   0x07(%[round_ptr])              \n\t"
      "gsldrc1    %[ftmp5],   0x00(%[round_ptr])              \n\t"
      "gsldlc1    %[ftmp6],   0x0f(%[round_ptr])              \n\t"
      "gsldrc1    %[ftmp6],   0x08(%[round_ptr])              \n\t"
      "paddh      %[ftmp5],   %[ftmp5],       %[ftmp1]        \n\t"
      "paddh      %[ftmp6],   %[ftmp6],       %[ftmp2]        \n\t"
      "gsldlc1    %[ftmp7],   0x07(%[quant_ptr])              \n\t"
      "gsldrc1    %[ftmp7],   0x00(%[quant_ptr])              \n\t"
      "gsldlc1    %[ftmp8],   0x0f(%[quant_ptr])              \n\t"
      "gsldrc1    %[ftmp8],   0x08(%[quant_ptr])              \n\t"
      "pmulhuh    %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
      "pmulhuh    %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"

      "gssdlc1    %[ftmp5],   0x07(%[y])                      \n\t"
      "gssdrc1    %[ftmp5],   0x00(%[y])                      \n\t"
      "gssdlc1    %[ftmp6],   0x0f(%[y])                      \n\t"
      "gssdrc1    %[ftmp6],   0x08(%[y])                      \n\t"
      "xor        %[ftmp7],   %[ftmp5],       %[ftmp3]        \n\t"
      "xor        %[ftmp8],   %[ftmp6],       %[ftmp4]        \n\t"
      "psubh      %[ftmp7],   %[ftmp7],       %[ftmp3]        \n\t"
      "psubh      %[ftmp8],   %[ftmp8],       %[ftmp4]        \n\t"
      "gssdlc1    %[ftmp7],   0x07(%[qcoeff_ptr])             \n\t"
      "gssdrc1    %[ftmp7],   0x00(%[qcoeff_ptr])             \n\t"
      "gssdlc1    %[ftmp8],   0x0f(%[qcoeff_ptr])             \n\t"
      "gssdrc1    %[ftmp8],   0x08(%[qcoeff_ptr])             \n\t"

      "gsldlc1    %[ftmp5],   0x07(%[dequant_ptr])            \n\t"
      "gsldrc1    %[ftmp5],   0x00(%[dequant_ptr])            \n\t"
      "gsldlc1    %[ftmp6],   0x0f(%[dequant_ptr])            \n\t"
      "gsldrc1    %[ftmp6],   0x08(%[dequant_ptr])            \n\t"
      "pmullh     %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
      "pmullh     %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
      "gssdlc1    %[ftmp5],   0x07(%[dqcoeff_ptr])            \n\t"
      "gssdrc1    %[ftmp5],   0x00(%[dqcoeff_ptr])            \n\t"
      "gssdlc1    %[ftmp6],   0x0f(%[dqcoeff_ptr])            \n\t"
      "gssdrc1    %[ftmp6],   0x08(%[dqcoeff_ptr])            \n\t"

      // loop 8 ~ 15
      "gsldlc1    %[ftmp1],   0x17(%[coeff_ptr])              \n\t"
      "gsldrc1    %[ftmp1],   0x10(%[coeff_ptr])              \n\t"
      "gsldlc1    %[ftmp2],   0x1f(%[coeff_ptr])              \n\t"
      "gsldrc1    %[ftmp2],   0x18(%[coeff_ptr])              \n\t"

      "psrah      %[ftmp3],   %[ftmp1],       %[ftmp9]        \n\t"
      "xor        %[ftmp1],   %[ftmp3],       %[ftmp1]        \n\t"
      "psubh      %[ftmp1],   %[ftmp1],       %[ftmp3]        \n\t"
      "psrah      %[ftmp4],   %[ftmp2],       %[ftmp9]        \n\t"
      "xor        %[ftmp2],   %[ftmp4],       %[ftmp2]        \n\t"
      "psubh      %[ftmp2],   %[ftmp2],       %[ftmp4]        \n\t"

      "gsldlc1    %[ftmp5],   0x17(%[round_ptr])              \n\t"
      "gsldrc1    %[ftmp5],   0x10(%[round_ptr])              \n\t"
      "gsldlc1    %[ftmp6],   0x1f(%[round_ptr])              \n\t"
      "gsldrc1    %[ftmp6],   0x18(%[round_ptr])              \n\t"
      "paddh      %[ftmp5],   %[ftmp5],       %[ftmp1]        \n\t"
      "paddh      %[ftmp6],   %[ftmp6],       %[ftmp2]        \n\t"
      "gsldlc1    %[ftmp7],   0x17(%[quant_ptr])              \n\t"
      "gsldrc1    %[ftmp7],   0x10(%[quant_ptr])              \n\t"
      "gsldlc1    %[ftmp8],   0x1f(%[quant_ptr])              \n\t"
      "gsldrc1    %[ftmp8],   0x18(%[quant_ptr])              \n\t"
      "pmulhuh    %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
      "pmulhuh    %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"

      "gssdlc1    %[ftmp5],   0x17(%[y])                      \n\t"
      "gssdrc1    %[ftmp5],   0x10(%[y])                      \n\t"
      "gssdlc1    %[ftmp6],   0x1f(%[y])                      \n\t"
      "gssdrc1    %[ftmp6],   0x18(%[y])                      \n\t"
      "xor        %[ftmp7],   %[ftmp5],       %[ftmp3]        \n\t"
      "xor        %[ftmp8],   %[ftmp6],       %[ftmp4]        \n\t"
      "psubh      %[ftmp7],   %[ftmp7],       %[ftmp3]        \n\t"
      "psubh      %[ftmp8],   %[ftmp8],       %[ftmp4]        \n\t"
      "gssdlc1    %[ftmp7],   0x17(%[qcoeff_ptr])             \n\t"
      "gssdrc1    %[ftmp7],   0x10(%[qcoeff_ptr])             \n\t"
      "gssdlc1    %[ftmp8],   0x1f(%[qcoeff_ptr])             \n\t"
      "gssdrc1    %[ftmp8],   0x18(%[qcoeff_ptr])             \n\t"

      "gsldlc1    %[ftmp5],   0x17(%[dequant_ptr])            \n\t"
      "gsldrc1    %[ftmp5],   0x10(%[dequant_ptr])            \n\t"
      "gsldlc1    %[ftmp6],   0x1f(%[dequant_ptr])            \n\t"
      "gsldrc1    %[ftmp6],   0x18(%[dequant_ptr])            \n\t"
      "pmullh     %[ftmp5],   %[ftmp5],       %[ftmp7]        \n\t"
      "pmullh     %[ftmp6],   %[ftmp6],       %[ftmp8]        \n\t"
      "gssdlc1    %[ftmp5],   0x17(%[dqcoeff_ptr])            \n\t"
      "gssdrc1    %[ftmp5],   0x10(%[dqcoeff_ptr])            \n\t"
      "gssdlc1    %[ftmp6],   0x1f(%[dqcoeff_ptr])            \n\t"
      "gssdrc1    %[ftmp6],   0x18(%[dqcoeff_ptr])            \n\t"
      : [ftmp0] "=&f"(ftmp[0]), [ftmp1] "=&f"(ftmp[1]), [ftmp2] "=&f"(ftmp[2]),
        [ftmp3] "=&f"(ftmp[3]), [ftmp4] "=&f"(ftmp[4]), [ftmp5] "=&f"(ftmp[5]),
        [ftmp6] "=&f"(ftmp[6]), [ftmp7] "=&f"(ftmp[7]), [ftmp8] "=&f"(ftmp[8]),
        [ftmp9] "=&f"(ftmp[9]), [tmp0] "=&r"(tmp[0])
      : [coeff_ptr] "r"((mips_reg)coeff_ptr),
        [qcoeff_ptr] "r"((mips_reg)qcoeff_ptr),
        [dequant_ptr] "r"((mips_reg)dequant_ptr),
        [round_ptr] "r"((mips_reg)round_ptr),
        [quant_ptr] "r"((mips_reg)quant_ptr),
        [dqcoeff_ptr] "r"((mips_reg)dqcoeff_ptr), [y] "r"(y)
      : "memory");

  // get last nonzero coeffs
  SET_EOB(15, 16);
  SET_EOB(14, 15);
  SET_EOB(11, 14);
  SET_EOB(7, 13);
  SET_EOB(10, 12);
  SET_EOB(13, 11);
  SET_EOB(12, 10);
  SET_EOB(9, 9);
  SET_EOB(6, 8);
  SET_EOB(3, 7);
  SET_EOB(2, 6);
  SET_EOB(5, 5);
  SET_EOB(8, 4);
  SET_EOB(4, 3);
  SET_EOB(1, 2);
  SET_EOB(0, 1);
  // all zero case, set 0.
  *d->eob = 0;
}

void vp8_regular_quantize_b_mmi(BLOCK *b, BLOCKD *d) {
  int eob;
  int x, y, z, sz;
  int16_t *zbin_boost_ptr = b->zrun_zbin_boost;
  int16_t *coeff_ptr = b->coeff;
  int16_t *zbin_ptr = b->zbin;
  int16_t *round_ptr = b->round;
  int16_t *quant_ptr = b->quant;
  int16_t *quant_shift_ptr = b->quant_shift;
  int16_t *qcoeff_ptr = d->qcoeff;
  int16_t *dqcoeff_ptr = d->dqcoeff;
  int16_t *dequant_ptr = d->dequant;
  int16_t zbin_oq_value = b->zbin_extra;

  memset(qcoeff_ptr, 0, 32);
  memset(dqcoeff_ptr, 0, 32);

  eob = -1;

  REGULAR_SELECT_EOB(0, 0);
  REGULAR_SELECT_EOB(1, 1);
  REGULAR_SELECT_EOB(2, 4);
  REGULAR_SELECT_EOB(3, 8);
  REGULAR_SELECT_EOB(4, 5);
  REGULAR_SELECT_EOB(5, 2);
  REGULAR_SELECT_EOB(6, 3);
  REGULAR_SELECT_EOB(7, 6);
  REGULAR_SELECT_EOB(8, 9);
  REGULAR_SELECT_EOB(9, 12);
  REGULAR_SELECT_EOB(10, 13);
  REGULAR_SELECT_EOB(11, 10);
  REGULAR_SELECT_EOB(12, 7);
  REGULAR_SELECT_EOB(13, 11);
  REGULAR_SELECT_EOB(14, 14);
  REGULAR_SELECT_EOB(15, 15);

  *d->eob = (char)(eob + 1);
}
