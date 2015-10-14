/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include <string.h>

#include "vpx_dsp/inv_txfm.h"

void idct32_135_c(const tran_low_t *input, tran_low_t *output) {
  tran_low_t step1[32], step2[32];
  tran_high_t temp1, temp2;

  // stage 1
#if 0
  step1[0] = input[0];
  step1[1] = 0; //input[16];
  step1[2] = input[8];
  step1[3] = 0; //input[24];
  step1[4] = input[4];
  step1[5] = 0; //input[20];
  step1[6] = input[12];
  step1[7] = 0; //input[28];
#endif

  //step1[8] = input[2];
  //step1[9] = 0; //input[18];
  //step1[10] = input[10];
  //step1[11] = 0; //input[26];
  //step1[12] = input[6];
  //step1[13] = 0; //input[22];
  //step1[14] = input[14];
  //step1[15] = 0; //input[30];

  temp1 = input[1] * cospi_31_64;
  temp2 = input[1] * cospi_1_64;
  step1[16] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[31] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[15] * cospi_17_64;
  temp2 = input[15] * cospi_15_64;
  step1[17] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[30] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = input[9] * cospi_23_64;
  temp2 = input[9] * cospi_9_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[29] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[7] * cospi_25_64;
  temp2 = input[7] * cospi_7_64;
  step1[19] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[28] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = input[5] * cospi_27_64;
  temp2 = input[5] * cospi_5_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[27] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[11] * cospi_21_64;
  temp2 = input[11] * cospi_11_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[26] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = input[13] * cospi_19_64;
  temp2 = input[13] * cospi_13_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[25] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[3] * cospi_29_64;
  temp2 = input[3] * cospi_3_64;
  step1[23] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[24] = WRAPLOW(dct_const_round_shift(temp2), 8);

  // stage 2
  //step2[0] = input[0]; //step1[0];
  //step2[1] = 0; //step1[1];
  //step2[2] = input[8]; //step1[2];
  //step2[3] = 0; //step1[3];
  //step2[4] = input[4]; //step1[4];
  //step2[5] = 0; //step1[5];
  //step2[6] = input[12]; //step1[6];
  //step2[7] = 0; //step1[7];

  temp1 = input[2] * cospi_30_64;
  temp2 = input[2] * cospi_2_64;
  step2[8] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[15] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[14] * cospi_18_64;
  temp2 = input[14] * cospi_14_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[14] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = input[10] * cospi_22_64;
  temp2 = input[10] * cospi_10_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[13] = WRAPLOW(dct_const_round_shift(temp2), 8);

  temp1 = - input[6] * cospi_26_64;
  temp2 = input[6] * cospi_6_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[12] = WRAPLOW(dct_const_round_shift(temp2), 8);

  step2[16] = WRAPLOW(step1[16] + step1[17], 8);
  step2[17] = WRAPLOW(step1[16] - step1[17], 8);

  step2[18] = WRAPLOW(-step1[18] + step1[19], 8);
  step2[19] = WRAPLOW(step1[18] + step1[19], 8);

  step2[20] = WRAPLOW(step1[20] + step1[21], 8);
  step2[21] = WRAPLOW(step1[20] - step1[21], 8);

  step2[22] = WRAPLOW(-step1[22] + step1[23], 8);
  step2[23] = WRAPLOW(step1[22] + step1[23], 8);

  step2[24] = WRAPLOW(step1[24] + step1[25], 8);
  step2[25] = WRAPLOW(step1[24] - step1[25], 8);

  step2[26] = WRAPLOW(-step1[26] + step1[27], 8);
  step2[27] = WRAPLOW(step1[26] + step1[27], 8);

  step2[28] = WRAPLOW(step1[28] + step1[29], 8);
  step2[29] = WRAPLOW(step1[28] - step1[29], 8);

  step2[30] = WRAPLOW(-step1[30] + step1[31], 8);
  step2[31] = WRAPLOW(step1[30] + step1[31], 8);

  // stage 3
  //step1[0] = input[0]; //step2[0];
  //step1[1] = 0; //step2[1];
  //step1[2] = input[8]; //step2[2];
  //step1[3] = 0; //step2[3];

  temp1 = input[4] * cospi_28_64;
  temp2 = input[4] * cospi_4_64;
  step1[4] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[7] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = - input[12] * cospi_20_64;
  temp2 = input[12] * cospi_12_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[6] = WRAPLOW(dct_const_round_shift(temp2), 8);

  step1[8] = WRAPLOW(step2[8] + step2[9], 8);
  step1[9] = WRAPLOW(step2[8] - step2[9], 8);
  step1[10] = WRAPLOW(-step2[10] + step2[11], 8);
  step1[11] = WRAPLOW(step2[10] + step2[11], 8);
  step1[12] = WRAPLOW(step2[12] + step2[13], 8);
  step1[13] = WRAPLOW(step2[12] - step2[13], 8);
  step1[14] = WRAPLOW(-step2[14] + step2[15], 8);
  step1[15] = WRAPLOW(step2[14] + step2[15], 8);

  step1[16] = step2[16];
  step1[31] = step2[31];
  temp1 = -step2[17] * cospi_4_64 + step2[30] * cospi_28_64;
  temp2 = step2[17] * cospi_28_64 + step2[30] * cospi_4_64;
  step1[17] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[30] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step2[18] * cospi_28_64 - step2[29] * cospi_4_64;
  temp2 = -step2[18] * cospi_4_64 + step2[29] * cospi_28_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[29] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step1[19] = step2[19];
  step1[20] = step2[20];
  temp1 = -step2[21] * cospi_20_64 + step2[26] * cospi_12_64;
  temp2 = step2[21] * cospi_12_64 + step2[26] * cospi_20_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[26] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step2[22] * cospi_12_64 - step2[25] * cospi_20_64;
  temp2 = -step2[22] * cospi_20_64 + step2[25] * cospi_12_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[25] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step1[23] = step2[23];
  step1[24] = step2[24];
  step1[27] = step2[27];
  step1[28] = step2[28];

  // stage 4
  temp1 = (input[0]) * cospi_16_64;
  temp2 = (input[0]) * cospi_16_64;
  step2[0] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[1] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = input[8] * cospi_24_64;
  temp2 = input[8] * cospi_8_64;
  step2[2] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[3] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step2[4] = WRAPLOW(step1[4] + step1[5], 8);
  step2[5] = WRAPLOW(step1[4] - step1[5], 8);
  step2[6] = WRAPLOW(-step1[6] + step1[7], 8);
  step2[7] = WRAPLOW(step1[6] + step1[7], 8);

  step2[8] = step1[8];
  step2[15] = step1[15];
  temp1 = -step1[9] * cospi_8_64 + step1[14] * cospi_24_64;
  temp2 = step1[9] * cospi_24_64 + step1[14] * cospi_8_64;
  step2[9] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[14] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step1[10] * cospi_24_64 - step1[13] * cospi_8_64;
  temp2 = -step1[10] * cospi_8_64 + step1[13] * cospi_24_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[13] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step2[11] = step1[11];
  step2[12] = step1[12];

  step2[16] = WRAPLOW(step1[16] + step1[19], 8);
  step2[17] = WRAPLOW(step1[17] + step1[18], 8);
  step2[18] = WRAPLOW(step1[17] - step1[18], 8);
  step2[19] = WRAPLOW(step1[16] - step1[19], 8);
  step2[20] = WRAPLOW(-step1[20] + step1[23], 8);
  step2[21] = WRAPLOW(-step1[21] + step1[22], 8);
  step2[22] = WRAPLOW(step1[21] + step1[22], 8);
  step2[23] = WRAPLOW(step1[20] + step1[23], 8);

  step2[24] = WRAPLOW(step1[24] + step1[27], 8);
  step2[25] = WRAPLOW(step1[25] + step1[26], 8);
  step2[26] = WRAPLOW(step1[25] - step1[26], 8);
  step2[27] = WRAPLOW(step1[24] - step1[27], 8);
  step2[28] = WRAPLOW(-step1[28] + step1[31], 8);
  step2[29] = WRAPLOW(-step1[29] + step1[30], 8);
  step2[30] = WRAPLOW(step1[29] + step1[30], 8);
  step2[31] = WRAPLOW(step1[28] + step1[31], 8);

  // stage 5
  step1[0] = WRAPLOW(step2[0] + step2[3], 8);
  step1[1] = WRAPLOW(step2[1] + step2[2], 8);
  step1[2] = WRAPLOW(step2[1] - step2[2], 8);
  step1[3] = WRAPLOW(step2[0] - step2[3], 8);
  step1[4] = step2[4];
  temp1 = (step2[6] - step2[5]) * cospi_16_64;
  temp2 = (step2[5] + step2[6]) * cospi_16_64;
  step1[5] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[6] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step1[7] = step2[7];

  step1[8] = WRAPLOW(step2[8] + step2[11], 8);
  step1[9] = WRAPLOW(step2[9] + step2[10], 8);
  step1[10] = WRAPLOW(step2[9] - step2[10], 8);
  step1[11] = WRAPLOW(step2[8] - step2[11], 8);
  step1[12] = WRAPLOW(-step2[12] + step2[15], 8);
  step1[13] = WRAPLOW(-step2[13] + step2[14], 8);
  step1[14] = WRAPLOW(step2[13] + step2[14], 8);
  step1[15] = WRAPLOW(step2[12] + step2[15], 8);

  step1[16] = step2[16];
  step1[17] = step2[17];
  temp1 = -step2[18] * cospi_8_64 + step2[29] * cospi_24_64;
  temp2 = step2[18] * cospi_24_64 + step2[29] * cospi_8_64;
  step1[18] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[29] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step2[19] * cospi_8_64 + step2[28] * cospi_24_64;
  temp2 = step2[19] * cospi_24_64 + step2[28] * cospi_8_64;
  step1[19] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[28] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step2[20] * cospi_24_64 - step2[27] * cospi_8_64;
  temp2 = -step2[20] * cospi_8_64 + step2[27] * cospi_24_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[27] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = -step2[21] * cospi_24_64 - step2[26] * cospi_8_64;
  temp2 = -step2[21] * cospi_8_64 + step2[26] * cospi_24_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[26] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step1[22] = step2[22];
  step1[23] = step2[23];
  step1[24] = step2[24];
  step1[25] = step2[25];
  step1[30] = step2[30];
  step1[31] = step2[31];

  // stage 6
  step2[0] = WRAPLOW(step1[0] + step1[7], 8);
  step2[1] = WRAPLOW(step1[1] + step1[6], 8);
  step2[2] = WRAPLOW(step1[2] + step1[5], 8);
  step2[3] = WRAPLOW(step1[3] + step1[4], 8);
  step2[4] = WRAPLOW(step1[3] - step1[4], 8);
  step2[5] = WRAPLOW(step1[2] - step1[5], 8);
  step2[6] = WRAPLOW(step1[1] - step1[6], 8);
  step2[7] = WRAPLOW(step1[0] - step1[7], 8);
  step2[8] = step1[8];
  step2[9] = step1[9];
  temp1 = (-step1[10] + step1[13]) * cospi_16_64;
  temp2 = (step1[10] + step1[13]) * cospi_16_64;
  step2[10] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[13] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = (-step1[11] + step1[12]) * cospi_16_64;
  temp2 = (step1[11] + step1[12]) * cospi_16_64;
  step2[11] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step2[12] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step2[14] = step1[14];
  step2[15] = step1[15];

  step2[16] = WRAPLOW(step1[16] + step1[23], 8);
  step2[17] = WRAPLOW(step1[17] + step1[22], 8);
  step2[18] = WRAPLOW(step1[18] + step1[21], 8);
  step2[19] = WRAPLOW(step1[19] + step1[20], 8);
  step2[20] = WRAPLOW(step1[19] - step1[20], 8);
  step2[21] = WRAPLOW(step1[18] - step1[21], 8);
  step2[22] = WRAPLOW(step1[17] - step1[22], 8);
  step2[23] = WRAPLOW(step1[16] - step1[23], 8);

  step2[24] = WRAPLOW(-step1[24] + step1[31], 8);
  step2[25] = WRAPLOW(-step1[25] + step1[30], 8);
  step2[26] = WRAPLOW(-step1[26] + step1[29], 8);
  step2[27] = WRAPLOW(-step1[27] + step1[28], 8);
  step2[28] = WRAPLOW(step1[27] + step1[28], 8);
  step2[29] = WRAPLOW(step1[26] + step1[29], 8);
  step2[30] = WRAPLOW(step1[25] + step1[30], 8);
  step2[31] = WRAPLOW(step1[24] + step1[31], 8);

  // stage 7
  step1[0] = WRAPLOW(step2[0] + step2[15], 8);
  step1[1] = WRAPLOW(step2[1] + step2[14], 8);
  step1[2] = WRAPLOW(step2[2] + step2[13], 8);
  step1[3] = WRAPLOW(step2[3] + step2[12], 8);
  step1[4] = WRAPLOW(step2[4] + step2[11], 8);
  step1[5] = WRAPLOW(step2[5] + step2[10], 8);
  step1[6] = WRAPLOW(step2[6] + step2[9], 8);
  step1[7] = WRAPLOW(step2[7] + step2[8], 8);
  step1[8] = WRAPLOW(step2[7] - step2[8], 8);
  step1[9] = WRAPLOW(step2[6] - step2[9], 8);
  step1[10] = WRAPLOW(step2[5] - step2[10], 8);
  step1[11] = WRAPLOW(step2[4] - step2[11], 8);
  step1[12] = WRAPLOW(step2[3] - step2[12], 8);
  step1[13] = WRAPLOW(step2[2] - step2[13], 8);
  step1[14] = WRAPLOW(step2[1] - step2[14], 8);
  step1[15] = WRAPLOW(step2[0] - step2[15], 8);

  step1[16] = step2[16];
  step1[17] = step2[17];
  step1[18] = step2[18];
  step1[19] = step2[19];
  temp1 = (-step2[20] + step2[27]) * cospi_16_64;
  temp2 = (step2[20] + step2[27]) * cospi_16_64;
  step1[20] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[27] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = (-step2[21] + step2[26]) * cospi_16_64;
  temp2 = (step2[21] + step2[26]) * cospi_16_64;
  step1[21] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[26] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = (-step2[22] + step2[25]) * cospi_16_64;
  temp2 = (step2[22] + step2[25]) * cospi_16_64;
  step1[22] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[25] = WRAPLOW(dct_const_round_shift(temp2), 8);
  temp1 = (-step2[23] + step2[24]) * cospi_16_64;
  temp2 = (step2[23] + step2[24]) * cospi_16_64;
  step1[23] = WRAPLOW(dct_const_round_shift(temp1), 8);
  step1[24] = WRAPLOW(dct_const_round_shift(temp2), 8);
  step1[28] = step2[28];
  step1[29] = step2[29];
  step1[30] = step2[30];
  step1[31] = step2[31];

  // final stage
  output[0] = WRAPLOW(step1[0] + step1[31], 8);
  output[1] = WRAPLOW(step1[1] + step1[30], 8);
  output[2] = WRAPLOW(step1[2] + step1[29], 8);
  output[3] = WRAPLOW(step1[3] + step1[28], 8);
  output[4] = WRAPLOW(step1[4] + step1[27], 8);
  output[5] = WRAPLOW(step1[5] + step1[26], 8);
  output[6] = WRAPLOW(step1[6] + step1[25], 8);
  output[7] = WRAPLOW(step1[7] + step1[24], 8);
  output[8] = WRAPLOW(step1[8] + step1[23], 8);
  output[9] = WRAPLOW(step1[9] + step1[22], 8);
  output[10] = WRAPLOW(step1[10] + step1[21], 8);
  output[11] = WRAPLOW(step1[11] + step1[20], 8);
  output[12] = WRAPLOW(step1[12] + step1[19], 8);
  output[13] = WRAPLOW(step1[13] + step1[18], 8);
  output[14] = WRAPLOW(step1[14] + step1[17], 8);
  output[15] = WRAPLOW(step1[15] + step1[16], 8);

  output[16] = WRAPLOW(step1[15] - step1[16], 8);
  output[17] = WRAPLOW(step1[14] - step1[17], 8);
  output[18] = WRAPLOW(step1[13] - step1[18], 8);
  output[19] = WRAPLOW(step1[12] - step1[19], 8);
  output[20] = WRAPLOW(step1[11] - step1[20], 8);
  output[21] = WRAPLOW(step1[10] - step1[21], 8);
  output[22] = WRAPLOW(step1[9] - step1[22], 8);
  output[23] = WRAPLOW(step1[8] - step1[23], 8);
  output[24] = WRAPLOW(step1[7] - step1[24], 8);
  output[25] = WRAPLOW(step1[6] - step1[25], 8);
  output[26] = WRAPLOW(step1[5] - step1[26], 8);
  output[27] = WRAPLOW(step1[4] - step1[27], 8);
  output[28] = WRAPLOW(step1[3] - step1[28], 8);
  output[29] = WRAPLOW(step1[2] - step1[29], 8);
  output[30] = WRAPLOW(step1[1] - step1[30], 8);
  output[31] = WRAPLOW(step1[0] - step1[31], 8);
}

void vpx_idct32x32_135_add_c(const tran_low_t *input, uint8_t *dest,
                              int stride) {
  tran_low_t out[32 * 32];
  tran_low_t *outptr = out;
  int i, j;
  tran_low_t temp_in[32], temp_out[32];

  // Rows
  for (i = 0; i < 32; ++i) {
    idct32_135_c(input, outptr);
    input += 32;
    outptr += 32;
  }

  // Columns
  for (i = 0; i < 32; ++i) {
    for (j = 0; j < 32; ++j)
      temp_in[j] = out[j * 32 + i];
    idct32_c(temp_in, temp_out);
    for (j = 0; j < 32; ++j) {
      dest[j * stride + i] = clip_pixel_add(dest[j * stride + i],
                                            ROUND_POWER_OF_TWO(temp_out[j], 6));
    }
  }
}

