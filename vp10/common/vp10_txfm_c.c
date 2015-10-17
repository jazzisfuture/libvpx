#include "vp10/common/vp10_txfm_c.h"
#define range_check(stage_idx, input, buf, size, bit)                        \
{                                                                            \
  int i, j;                                                                  \
  for (i = 0; i < size; ++i) {                                               \
    int buf_bit = get_max_bit(abs(buf[i]))+1;                                \
    if(buf_bit > bit) {                                                      \
      printf("======== %s overflow ========\n", __func__);                   \
      printf("stage: %d node: %d\n", stage_idx, i);                          \
      printf("bit: %d buf_bit: %d buf[i]: %d\n", bit, buf_bit, buf[i]);      \
      printf("input:\n");                                                    \
      for(j = 0; j < size; j++){                                             \
        printf("%d,", input[j]);                                             \
      }                                                                      \
      printf("\n");                                                          \
    }                                                                        \
  }                                                                          \
}                                                                            \

void vp10_fdct4_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 4;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[4];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0] + input[3];
  buf1[1] = input[1] + input[2];
  buf1[2] =-input[2] + input[1];
  buf1[3] =-input[3] + input[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[32], buf0[1],  cospi[32], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2],  cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[48], buf0[3], -cospi[16], buf0[2], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[2];
  buf1[2] = buf0[1];
  buf1[3] = buf0[3];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fdct8_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 8;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[8];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0] + input[7];
  buf1[1] = input[1] + input[6];
  buf1[2] = input[2] + input[5];
  buf1[3] = input[3] + input[4];
  buf1[4] =-input[4] + input[3];
  buf1[5] =-input[5] + input[2];
  buf1[6] =-input[6] + input[1];
  buf1[7] =-input[7] + input[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] =-buf0[2] + buf0[1];
  buf1[3] =-buf0[3] + buf0[0];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[5], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[32], buf0[1],  cospi[32], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2],  cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[48], buf0[3], -cospi[16], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] =-buf0[5] + buf0[4];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[7] + buf0[6];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4],  cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5],  cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[24], buf0[6], -cospi[40], buf0[5], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[56], buf0[7], -cospi[ 8], buf0[4], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[4];
  buf1[2] = buf0[2];
  buf1[3] = buf0[6];
  buf1[4] = buf0[1];
  buf1[5] = buf0[5];
  buf1[6] = buf0[3];
  buf1[7] = buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fdct16_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 16;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[16];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0] + input[15];
  buf1[1] = input[1] + input[14];
  buf1[2] = input[2] + input[13];
  buf1[3] = input[3] + input[12];
  buf1[4] = input[4] + input[11];
  buf1[5] = input[5] + input[10];
  buf1[6] = input[6] + input[9];
  buf1[7] = input[7] + input[8];
  buf1[8] =-input[8] + input[7];
  buf1[9] =-input[9] + input[6];
  buf1[10] =-input[10] + input[5];
  buf1[11] =-input[11] + input[4];
  buf1[12] =-input[12] + input[3];
  buf1[13] =-input[13] + input[2];
  buf1[14] =-input[14] + input[1];
  buf1[15] =-input[15] + input[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[7];
  buf1[1] = buf0[1] + buf0[6];
  buf1[2] = buf0[2] + buf0[5];
  buf1[3] = buf0[3] + buf0[4];
  buf1[4] =-buf0[4] + buf0[3];
  buf1[5] =-buf0[5] + buf0[2];
  buf1[6] =-buf0[6] + buf0[1];
  buf1[7] =-buf0[7] + buf0[0];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf(-cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[32], buf0[12],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[32], buf0[13],  cospi[32], buf0[10], cos_bit[stage_idx]);
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] =-buf0[2] + buf0[1];
  buf1[3] =-buf0[3] + buf0[0];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[5], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  buf1[8] = buf0[8] + buf0[11];
  buf1[9] = buf0[9] + buf0[10];
  buf1[10] =-buf0[10] + buf0[9];
  buf1[11] =-buf0[11] + buf0[8];
  buf1[12] =-buf0[12] + buf0[15];
  buf1[13] =-buf0[13] + buf0[14];
  buf1[14] = buf0[14] + buf0[13];
  buf1[15] = buf0[15] + buf0[12];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[32], buf0[1],  cospi[32], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2],  cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[48], buf0[3], -cospi[16], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] =-buf0[5] + buf0[4];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[7] + buf0[6];
  buf1[8] = buf0[8];
  buf1[9] = half_btf(-cospi[16], buf0[9],  cospi[48], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf(-cospi[48], buf0[10], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = half_btf( cospi[48], buf0[13], -cospi[16], buf0[10], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[16], buf0[14],  cospi[48], buf0[9], cos_bit[stage_idx]);
  buf1[15] = buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4],  cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5],  cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[24], buf0[6], -cospi[40], buf0[5], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[56], buf0[7], -cospi[ 8], buf0[4], cos_bit[stage_idx]);
  buf1[8] = buf0[8] + buf0[9];
  buf1[9] =-buf0[9] + buf0[8];
  buf1[10] =-buf0[10] + buf0[11];
  buf1[11] = buf0[11] + buf0[10];
  buf1[12] = buf0[12] + buf0[13];
  buf1[13] =-buf0[13] + buf0[12];
  buf1[14] =-buf0[14] + buf0[15];
  buf1[15] = buf0[15] + buf0[14];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[60], buf0[8],  cospi[ 4], buf0[15], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[28], buf0[9],  cospi[36], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[44], buf0[10],  cospi[20], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[12], buf0[11],  cospi[52], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[12], buf0[12], -cospi[52], buf0[11], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[44], buf0[13], -cospi[20], buf0[10], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[28], buf0[14], -cospi[36], buf0[9], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[60], buf0[15], -cospi[ 4], buf0[8], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[8];
  buf1[2] = buf0[4];
  buf1[3] = buf0[12];
  buf1[4] = buf0[2];
  buf1[5] = buf0[10];
  buf1[6] = buf0[6];
  buf1[7] = buf0[14];
  buf1[8] = buf0[1];
  buf1[9] = buf0[9];
  buf1[10] = buf0[5];
  buf1[11] = buf0[13];
  buf1[12] = buf0[3];
  buf1[13] = buf0[11];
  buf1[14] = buf0[7];
  buf1[15] = buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fdct32_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 32;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[32];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0] + input[31];
  buf1[1] = input[1] + input[30];
  buf1[2] = input[2] + input[29];
  buf1[3] = input[3] + input[28];
  buf1[4] = input[4] + input[27];
  buf1[5] = input[5] + input[26];
  buf1[6] = input[6] + input[25];
  buf1[7] = input[7] + input[24];
  buf1[8] = input[8] + input[23];
  buf1[9] = input[9] + input[22];
  buf1[10] = input[10] + input[21];
  buf1[11] = input[11] + input[20];
  buf1[12] = input[12] + input[19];
  buf1[13] = input[13] + input[18];
  buf1[14] = input[14] + input[17];
  buf1[15] = input[15] + input[16];
  buf1[16] =-input[16] + input[15];
  buf1[17] =-input[17] + input[14];
  buf1[18] =-input[18] + input[13];
  buf1[19] =-input[19] + input[12];
  buf1[20] =-input[20] + input[11];
  buf1[21] =-input[21] + input[10];
  buf1[22] =-input[22] + input[9];
  buf1[23] =-input[23] + input[8];
  buf1[24] =-input[24] + input[7];
  buf1[25] =-input[25] + input[6];
  buf1[26] =-input[26] + input[5];
  buf1[27] =-input[27] + input[4];
  buf1[28] =-input[28] + input[3];
  buf1[29] =-input[29] + input[2];
  buf1[30] =-input[30] + input[1];
  buf1[31] =-input[31] + input[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[15];
  buf1[1] = buf0[1] + buf0[14];
  buf1[2] = buf0[2] + buf0[13];
  buf1[3] = buf0[3] + buf0[12];
  buf1[4] = buf0[4] + buf0[11];
  buf1[5] = buf0[5] + buf0[10];
  buf1[6] = buf0[6] + buf0[9];
  buf1[7] = buf0[7] + buf0[8];
  buf1[8] =-buf0[8] + buf0[7];
  buf1[9] =-buf0[9] + buf0[6];
  buf1[10] =-buf0[10] + buf0[5];
  buf1[11] =-buf0[11] + buf0[4];
  buf1[12] =-buf0[12] + buf0[3];
  buf1[13] =-buf0[13] + buf0[2];
  buf1[14] =-buf0[14] + buf0[1];
  buf1[15] =-buf0[15] + buf0[0];
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = half_btf(-cospi[32], buf0[20],  cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[32], buf0[21],  cospi[32], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[32], buf0[22],  cospi[32], buf0[25], cos_bit[stage_idx]);
  buf1[23] = half_btf(-cospi[32], buf0[23],  cospi[32], buf0[24], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[32], buf0[24],  cospi[32], buf0[23], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[32], buf0[25],  cospi[32], buf0[22], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[32], buf0[26],  cospi[32], buf0[21], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[32], buf0[27],  cospi[32], buf0[20], cos_bit[stage_idx]);
  buf1[28] = buf0[28];
  buf1[29] = buf0[29];
  buf1[30] = buf0[30];
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[7];
  buf1[1] = buf0[1] + buf0[6];
  buf1[2] = buf0[2] + buf0[5];
  buf1[3] = buf0[3] + buf0[4];
  buf1[4] =-buf0[4] + buf0[3];
  buf1[5] =-buf0[5] + buf0[2];
  buf1[6] =-buf0[6] + buf0[1];
  buf1[7] =-buf0[7] + buf0[0];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf(-cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[32], buf0[12],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[32], buf0[13],  cospi[32], buf0[10], cos_bit[stage_idx]);
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = buf0[16] + buf0[23];
  buf1[17] = buf0[17] + buf0[22];
  buf1[18] = buf0[18] + buf0[21];
  buf1[19] = buf0[19] + buf0[20];
  buf1[20] =-buf0[20] + buf0[19];
  buf1[21] =-buf0[21] + buf0[18];
  buf1[22] =-buf0[22] + buf0[17];
  buf1[23] =-buf0[23] + buf0[16];
  buf1[24] =-buf0[24] + buf0[31];
  buf1[25] =-buf0[25] + buf0[30];
  buf1[26] =-buf0[26] + buf0[29];
  buf1[27] =-buf0[27] + buf0[28];
  buf1[28] = buf0[28] + buf0[27];
  buf1[29] = buf0[29] + buf0[26];
  buf1[30] = buf0[30] + buf0[25];
  buf1[31] = buf0[31] + buf0[24];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] =-buf0[2] + buf0[1];
  buf1[3] =-buf0[3] + buf0[0];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[5], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  buf1[8] = buf0[8] + buf0[11];
  buf1[9] = buf0[9] + buf0[10];
  buf1[10] =-buf0[10] + buf0[9];
  buf1[11] =-buf0[11] + buf0[8];
  buf1[12] =-buf0[12] + buf0[15];
  buf1[13] =-buf0[13] + buf0[14];
  buf1[14] = buf0[14] + buf0[13];
  buf1[15] = buf0[15] + buf0[12];
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = half_btf(-cospi[16], buf0[18],  cospi[48], buf0[29], cos_bit[stage_idx]);
  buf1[19] = half_btf(-cospi[16], buf0[19],  cospi[48], buf0[28], cos_bit[stage_idx]);
  buf1[20] = half_btf(-cospi[48], buf0[20], -cospi[16], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[48], buf0[21], -cospi[16], buf0[26], cos_bit[stage_idx]);
  buf1[22] = buf0[22];
  buf1[23] = buf0[23];
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = half_btf( cospi[48], buf0[26], -cospi[16], buf0[21], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[48], buf0[27], -cospi[16], buf0[20], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[16], buf0[28],  cospi[48], buf0[19], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[16], buf0[29],  cospi[48], buf0[18], cos_bit[stage_idx]);
  buf1[30] = buf0[30];
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[32], buf0[1],  cospi[32], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2],  cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[48], buf0[3], -cospi[16], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] =-buf0[5] + buf0[4];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[7] + buf0[6];
  buf1[8] = buf0[8];
  buf1[9] = half_btf(-cospi[16], buf0[9],  cospi[48], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf(-cospi[48], buf0[10], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = half_btf( cospi[48], buf0[13], -cospi[16], buf0[10], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[16], buf0[14],  cospi[48], buf0[9], cos_bit[stage_idx]);
  buf1[15] = buf0[15];
  buf1[16] = buf0[16] + buf0[19];
  buf1[17] = buf0[17] + buf0[18];
  buf1[18] =-buf0[18] + buf0[17];
  buf1[19] =-buf0[19] + buf0[16];
  buf1[20] =-buf0[20] + buf0[23];
  buf1[21] =-buf0[21] + buf0[22];
  buf1[22] = buf0[22] + buf0[21];
  buf1[23] = buf0[23] + buf0[20];
  buf1[24] = buf0[24] + buf0[27];
  buf1[25] = buf0[25] + buf0[26];
  buf1[26] =-buf0[26] + buf0[25];
  buf1[27] =-buf0[27] + buf0[24];
  buf1[28] =-buf0[28] + buf0[31];
  buf1[29] =-buf0[29] + buf0[30];
  buf1[30] = buf0[30] + buf0[29];
  buf1[31] = buf0[31] + buf0[28];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4],  cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5],  cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[24], buf0[6], -cospi[40], buf0[5], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[56], buf0[7], -cospi[ 8], buf0[4], cos_bit[stage_idx]);
  buf1[8] = buf0[8] + buf0[9];
  buf1[9] =-buf0[9] + buf0[8];
  buf1[10] =-buf0[10] + buf0[11];
  buf1[11] = buf0[11] + buf0[10];
  buf1[12] = buf0[12] + buf0[13];
  buf1[13] =-buf0[13] + buf0[12];
  buf1[14] =-buf0[14] + buf0[15];
  buf1[15] = buf0[15] + buf0[14];
  buf1[16] = buf0[16];
  buf1[17] = half_btf(-cospi[ 8], buf0[17],  cospi[56], buf0[30], cos_bit[stage_idx]);
  buf1[18] = half_btf(-cospi[56], buf0[18], -cospi[ 8], buf0[29], cos_bit[stage_idx]);
  buf1[19] = buf0[19];
  buf1[20] = buf0[20];
  buf1[21] = half_btf(-cospi[40], buf0[21],  cospi[24], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[24], buf0[22], -cospi[40], buf0[25], cos_bit[stage_idx]);
  buf1[23] = buf0[23];
  buf1[24] = buf0[24];
  buf1[25] = half_btf( cospi[24], buf0[25], -cospi[40], buf0[22], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[40], buf0[26],  cospi[24], buf0[21], cos_bit[stage_idx]);
  buf1[27] = buf0[27];
  buf1[28] = buf0[28];
  buf1[29] = half_btf( cospi[56], buf0[29], -cospi[ 8], buf0[18], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[ 8], buf0[30],  cospi[56], buf0[17], cos_bit[stage_idx]);
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[60], buf0[8],  cospi[ 4], buf0[15], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[28], buf0[9],  cospi[36], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[44], buf0[10],  cospi[20], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[12], buf0[11],  cospi[52], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[12], buf0[12], -cospi[52], buf0[11], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[44], buf0[13], -cospi[20], buf0[10], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[28], buf0[14], -cospi[36], buf0[9], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[60], buf0[15], -cospi[ 4], buf0[8], cos_bit[stage_idx]);
  buf1[16] = buf0[16] + buf0[17];
  buf1[17] =-buf0[17] + buf0[16];
  buf1[18] =-buf0[18] + buf0[19];
  buf1[19] = buf0[19] + buf0[18];
  buf1[20] = buf0[20] + buf0[21];
  buf1[21] =-buf0[21] + buf0[20];
  buf1[22] =-buf0[22] + buf0[23];
  buf1[23] = buf0[23] + buf0[22];
  buf1[24] = buf0[24] + buf0[25];
  buf1[25] =-buf0[25] + buf0[24];
  buf1[26] =-buf0[26] + buf0[27];
  buf1[27] = buf0[27] + buf0[26];
  buf1[28] = buf0[28] + buf0[29];
  buf1[29] =-buf0[29] + buf0[28];
  buf1[30] =-buf0[30] + buf0[31];
  buf1[31] = buf0[31] + buf0[30];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = half_btf( cospi[62], buf0[16],  cospi[ 2], buf0[31], cos_bit[stage_idx]);
  buf1[17] = half_btf( cospi[30], buf0[17],  cospi[34], buf0[30], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[46], buf0[18],  cospi[18], buf0[29], cos_bit[stage_idx]);
  buf1[19] = half_btf( cospi[14], buf0[19],  cospi[50], buf0[28], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[54], buf0[20],  cospi[10], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf( cospi[22], buf0[21],  cospi[42], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[38], buf0[22],  cospi[26], buf0[25], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[ 6], buf0[23],  cospi[58], buf0[24], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[ 6], buf0[24], -cospi[58], buf0[23], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[38], buf0[25], -cospi[26], buf0[22], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[22], buf0[26], -cospi[42], buf0[21], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[54], buf0[27], -cospi[10], buf0[20], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[14], buf0[28], -cospi[50], buf0[19], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[46], buf0[29], -cospi[18], buf0[18], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[30], buf0[30], -cospi[34], buf0[17], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[62], buf0[31], -cospi[ 2], buf0[16], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[16];
  buf1[2] = buf0[8];
  buf1[3] = buf0[24];
  buf1[4] = buf0[4];
  buf1[5] = buf0[20];
  buf1[6] = buf0[12];
  buf1[7] = buf0[28];
  buf1[8] = buf0[2];
  buf1[9] = buf0[18];
  buf1[10] = buf0[10];
  buf1[11] = buf0[26];
  buf1[12] = buf0[6];
  buf1[13] = buf0[22];
  buf1[14] = buf0[14];
  buf1[15] = buf0[30];
  buf1[16] = buf0[1];
  buf1[17] = buf0[17];
  buf1[18] = buf0[9];
  buf1[19] = buf0[25];
  buf1[20] = buf0[5];
  buf1[21] = buf0[21];
  buf1[22] = buf0[13];
  buf1[23] = buf0[29];
  buf1[24] = buf0[3];
  buf1[25] = buf0[19];
  buf1[26] = buf0[11];
  buf1[27] = buf0[27];
  buf1[28] = buf0[7];
  buf1[29] = buf0[23];
  buf1[30] = buf0[15];
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fadst4_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 4;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[4];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[3];
  buf1[1] = input[0];
  buf1[2] = input[1];
  buf1[3] = input[2];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 8], buf0[0],  cospi[56], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[ 8], buf0[1],  cospi[56], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[40], buf0[2],  cospi[24], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[40], buf0[3],  cospi[24], buf0[2], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] =-buf0[2] + buf0[0];
  buf1[3] =-buf0[3] + buf0[1];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[32], buf0[3],  cospi[32], buf0[2], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] =-buf0[2];
  buf1[2] = buf0[3];
  buf1[3] =-buf0[1];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fadst8_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 8;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[8];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[7];
  buf1[1] = input[0];
  buf1[2] = input[5];
  buf1[3] = input[2];
  buf1[4] = input[3];
  buf1[5] = input[4];
  buf1[6] = input[1];
  buf1[7] = input[6];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 4], buf0[0],  cospi[60], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[ 4], buf0[1],  cospi[60], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[20], buf0[2],  cospi[44], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[20], buf0[3],  cospi[44], buf0[2], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[36], buf0[4],  cospi[28], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[36], buf0[5],  cospi[28], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[52], buf0[6],  cospi[12], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[52], buf0[7],  cospi[12], buf0[6], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] =-buf0[4] + buf0[0];
  buf1[5] =-buf0[5] + buf0[1];
  buf1[6] =-buf0[6] + buf0[2];
  buf1[7] =-buf0[7] + buf0[3];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[16], buf0[5],  cospi[48], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[48], buf0[7],  cospi[16], buf0[6], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] =-buf0[2] + buf0[0];
  buf1[3] =-buf0[3] + buf0[1];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] =-buf0[6] + buf0[4];
  buf1[7] =-buf0[7] + buf0[5];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[32], buf0[3],  cospi[32], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[32], buf0[7],  cospi[32], buf0[6], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] =-buf0[4];
  buf1[2] = buf0[6];
  buf1[3] =-buf0[2];
  buf1[4] = buf0[3];
  buf1[5] =-buf0[7];
  buf1[6] = buf0[5];
  buf1[7] =-buf0[1];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fadst16_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 16;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[16];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[15];
  buf1[1] = input[0];
  buf1[2] = input[13];
  buf1[3] = input[2];
  buf1[4] = input[11];
  buf1[5] = input[4];
  buf1[6] = input[9];
  buf1[7] = input[6];
  buf1[8] = input[7];
  buf1[9] = input[8];
  buf1[10] = input[5];
  buf1[11] = input[10];
  buf1[12] = input[3];
  buf1[13] = input[12];
  buf1[14] = input[1];
  buf1[15] = input[14];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 2], buf0[0],  cospi[62], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[ 2], buf0[1],  cospi[62], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[10], buf0[2],  cospi[54], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[10], buf0[3],  cospi[54], buf0[2], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[18], buf0[4],  cospi[46], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[18], buf0[5],  cospi[46], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[26], buf0[6],  cospi[38], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[26], buf0[7],  cospi[38], buf0[6], cos_bit[stage_idx]);
  buf1[8] = half_btf( cospi[34], buf0[8],  cospi[30], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf(-cospi[34], buf0[9],  cospi[30], buf0[8], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[42], buf0[10],  cospi[22], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[42], buf0[11],  cospi[22], buf0[10], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[50], buf0[12],  cospi[14], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf(-cospi[50], buf0[13],  cospi[14], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[58], buf0[14],  cospi[ 6], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf(-cospi[58], buf0[15],  cospi[ 6], buf0[14], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[8];
  buf1[1] = buf0[1] + buf0[9];
  buf1[2] = buf0[2] + buf0[10];
  buf1[3] = buf0[3] + buf0[11];
  buf1[4] = buf0[4] + buf0[12];
  buf1[5] = buf0[5] + buf0[13];
  buf1[6] = buf0[6] + buf0[14];
  buf1[7] = buf0[7] + buf0[15];
  buf1[8] =-buf0[8] + buf0[0];
  buf1[9] =-buf0[9] + buf0[1];
  buf1[10] =-buf0[10] + buf0[2];
  buf1[11] =-buf0[11] + buf0[3];
  buf1[12] =-buf0[12] + buf0[4];
  buf1[13] =-buf0[13] + buf0[5];
  buf1[14] =-buf0[14] + buf0[6];
  buf1[15] =-buf0[15] + buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[ 8], buf0[8],  cospi[56], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf(-cospi[ 8], buf0[9],  cospi[56], buf0[8], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[40], buf0[10],  cospi[24], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[40], buf0[11],  cospi[24], buf0[10], cos_bit[stage_idx]);
  buf1[12] = half_btf(-cospi[56], buf0[12],  cospi[ 8], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[56], buf0[13],  cospi[ 8], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[24], buf0[14],  cospi[40], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[24], buf0[15],  cospi[40], buf0[14], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] =-buf0[4] + buf0[0];
  buf1[5] =-buf0[5] + buf0[1];
  buf1[6] =-buf0[6] + buf0[2];
  buf1[7] =-buf0[7] + buf0[3];
  buf1[8] = buf0[8] + buf0[12];
  buf1[9] = buf0[9] + buf0[13];
  buf1[10] = buf0[10] + buf0[14];
  buf1[11] = buf0[11] + buf0[15];
  buf1[12] =-buf0[12] + buf0[8];
  buf1[13] =-buf0[13] + buf0[9];
  buf1[14] =-buf0[14] + buf0[10];
  buf1[15] =-buf0[15] + buf0[11];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[16], buf0[5],  cospi[48], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[48], buf0[7],  cospi[16], buf0[6], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = half_btf( cospi[16], buf0[12],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf(-cospi[16], buf0[13],  cospi[48], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[48], buf0[14],  cospi[16], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[48], buf0[15],  cospi[16], buf0[14], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] =-buf0[2] + buf0[0];
  buf1[3] =-buf0[3] + buf0[1];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] =-buf0[6] + buf0[4];
  buf1[7] =-buf0[7] + buf0[5];
  buf1[8] = buf0[8] + buf0[10];
  buf1[9] = buf0[9] + buf0[11];
  buf1[10] =-buf0[10] + buf0[8];
  buf1[11] =-buf0[11] + buf0[9];
  buf1[12] = buf0[12] + buf0[14];
  buf1[13] = buf0[13] + buf0[15];
  buf1[14] =-buf0[14] + buf0[12];
  buf1[15] =-buf0[15] + buf0[13];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[32], buf0[3],  cospi[32], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[32], buf0[7],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[10], cos_bit[stage_idx]);
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = half_btf( cospi[32], buf0[14],  cospi[32], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf(-cospi[32], buf0[15],  cospi[32], buf0[14], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] =-buf0[8];
  buf1[2] = buf0[12];
  buf1[3] =-buf0[4];
  buf1[4] = buf0[6];
  buf1[5] =-buf0[14];
  buf1[6] = buf0[10];
  buf1[7] =-buf0[2];
  buf1[8] = buf0[3];
  buf1[9] =-buf0[11];
  buf1[10] = buf0[15];
  buf1[11] =-buf0[7];
  buf1[12] = buf0[5];
  buf1[13] =-buf0[13];
  buf1[14] = buf0[9];
  buf1[15] =-buf0[1];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_fadst32_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 32;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[32];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[31];
  buf1[1] = input[0];
  buf1[2] = input[29];
  buf1[3] = input[2];
  buf1[4] = input[27];
  buf1[5] = input[4];
  buf1[6] = input[25];
  buf1[7] = input[6];
  buf1[8] = input[23];
  buf1[9] = input[8];
  buf1[10] = input[21];
  buf1[11] = input[10];
  buf1[12] = input[19];
  buf1[13] = input[12];
  buf1[14] = input[17];
  buf1[15] = input[14];
  buf1[16] = input[15];
  buf1[17] = input[16];
  buf1[18] = input[13];
  buf1[19] = input[18];
  buf1[20] = input[11];
  buf1[21] = input[20];
  buf1[22] = input[9];
  buf1[23] = input[22];
  buf1[24] = input[7];
  buf1[25] = input[24];
  buf1[26] = input[5];
  buf1[27] = input[26];
  buf1[28] = input[3];
  buf1[29] = input[28];
  buf1[30] = input[1];
  buf1[31] = input[30];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 1], buf0[0],  cospi[63], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf(-cospi[ 1], buf0[1],  cospi[63], buf0[0], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[ 5], buf0[2],  cospi[59], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[ 5], buf0[3],  cospi[59], buf0[2], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[ 9], buf0[4],  cospi[55], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[ 9], buf0[5],  cospi[55], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[13], buf0[6],  cospi[51], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[13], buf0[7],  cospi[51], buf0[6], cos_bit[stage_idx]);
  buf1[8] = half_btf( cospi[17], buf0[8],  cospi[47], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf(-cospi[17], buf0[9],  cospi[47], buf0[8], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[21], buf0[10],  cospi[43], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[21], buf0[11],  cospi[43], buf0[10], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[25], buf0[12],  cospi[39], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf(-cospi[25], buf0[13],  cospi[39], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[29], buf0[14],  cospi[35], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf(-cospi[29], buf0[15],  cospi[35], buf0[14], cos_bit[stage_idx]);
  buf1[16] = half_btf( cospi[33], buf0[16],  cospi[31], buf0[17], cos_bit[stage_idx]);
  buf1[17] = half_btf(-cospi[33], buf0[17],  cospi[31], buf0[16], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[37], buf0[18],  cospi[27], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf(-cospi[37], buf0[19],  cospi[27], buf0[18], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[41], buf0[20],  cospi[23], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[41], buf0[21],  cospi[23], buf0[20], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[45], buf0[22],  cospi[19], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf(-cospi[45], buf0[23],  cospi[19], buf0[22], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[49], buf0[24],  cospi[15], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf(-cospi[49], buf0[25],  cospi[15], buf0[24], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[53], buf0[26],  cospi[11], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf(-cospi[53], buf0[27],  cospi[11], buf0[26], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[57], buf0[28],  cospi[ 7], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf(-cospi[57], buf0[29],  cospi[ 7], buf0[28], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[61], buf0[30],  cospi[ 3], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf(-cospi[61], buf0[31],  cospi[ 3], buf0[30], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[16];
  buf1[1] = buf0[1] + buf0[17];
  buf1[2] = buf0[2] + buf0[18];
  buf1[3] = buf0[3] + buf0[19];
  buf1[4] = buf0[4] + buf0[20];
  buf1[5] = buf0[5] + buf0[21];
  buf1[6] = buf0[6] + buf0[22];
  buf1[7] = buf0[7] + buf0[23];
  buf1[8] = buf0[8] + buf0[24];
  buf1[9] = buf0[9] + buf0[25];
  buf1[10] = buf0[10] + buf0[26];
  buf1[11] = buf0[11] + buf0[27];
  buf1[12] = buf0[12] + buf0[28];
  buf1[13] = buf0[13] + buf0[29];
  buf1[14] = buf0[14] + buf0[30];
  buf1[15] = buf0[15] + buf0[31];
  buf1[16] =-buf0[16] + buf0[0];
  buf1[17] =-buf0[17] + buf0[1];
  buf1[18] =-buf0[18] + buf0[2];
  buf1[19] =-buf0[19] + buf0[3];
  buf1[20] =-buf0[20] + buf0[4];
  buf1[21] =-buf0[21] + buf0[5];
  buf1[22] =-buf0[22] + buf0[6];
  buf1[23] =-buf0[23] + buf0[7];
  buf1[24] =-buf0[24] + buf0[8];
  buf1[25] =-buf0[25] + buf0[9];
  buf1[26] =-buf0[26] + buf0[10];
  buf1[27] =-buf0[27] + buf0[11];
  buf1[28] =-buf0[28] + buf0[12];
  buf1[29] =-buf0[29] + buf0[13];
  buf1[30] =-buf0[30] + buf0[14];
  buf1[31] =-buf0[31] + buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = half_btf( cospi[ 4], buf0[16],  cospi[60], buf0[17], cos_bit[stage_idx]);
  buf1[17] = half_btf(-cospi[ 4], buf0[17],  cospi[60], buf0[16], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[20], buf0[18],  cospi[44], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf(-cospi[20], buf0[19],  cospi[44], buf0[18], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[36], buf0[20],  cospi[28], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[36], buf0[21],  cospi[28], buf0[20], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[52], buf0[22],  cospi[12], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf(-cospi[52], buf0[23],  cospi[12], buf0[22], cos_bit[stage_idx]);
  buf1[24] = half_btf(-cospi[60], buf0[24],  cospi[ 4], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[60], buf0[25],  cospi[ 4], buf0[24], cos_bit[stage_idx]);
  buf1[26] = half_btf(-cospi[44], buf0[26],  cospi[20], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[44], buf0[27],  cospi[20], buf0[26], cos_bit[stage_idx]);
  buf1[28] = half_btf(-cospi[28], buf0[28],  cospi[36], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[28], buf0[29],  cospi[36], buf0[28], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[12], buf0[30],  cospi[52], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[12], buf0[31],  cospi[52], buf0[30], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[8];
  buf1[1] = buf0[1] + buf0[9];
  buf1[2] = buf0[2] + buf0[10];
  buf1[3] = buf0[3] + buf0[11];
  buf1[4] = buf0[4] + buf0[12];
  buf1[5] = buf0[5] + buf0[13];
  buf1[6] = buf0[6] + buf0[14];
  buf1[7] = buf0[7] + buf0[15];
  buf1[8] =-buf0[8] + buf0[0];
  buf1[9] =-buf0[9] + buf0[1];
  buf1[10] =-buf0[10] + buf0[2];
  buf1[11] =-buf0[11] + buf0[3];
  buf1[12] =-buf0[12] + buf0[4];
  buf1[13] =-buf0[13] + buf0[5];
  buf1[14] =-buf0[14] + buf0[6];
  buf1[15] =-buf0[15] + buf0[7];
  buf1[16] = buf0[16] + buf0[24];
  buf1[17] = buf0[17] + buf0[25];
  buf1[18] = buf0[18] + buf0[26];
  buf1[19] = buf0[19] + buf0[27];
  buf1[20] = buf0[20] + buf0[28];
  buf1[21] = buf0[21] + buf0[29];
  buf1[22] = buf0[22] + buf0[30];
  buf1[23] = buf0[23] + buf0[31];
  buf1[24] =-buf0[24] + buf0[16];
  buf1[25] =-buf0[25] + buf0[17];
  buf1[26] =-buf0[26] + buf0[18];
  buf1[27] =-buf0[27] + buf0[19];
  buf1[28] =-buf0[28] + buf0[20];
  buf1[29] =-buf0[29] + buf0[21];
  buf1[30] =-buf0[30] + buf0[22];
  buf1[31] =-buf0[31] + buf0[23];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[ 8], buf0[8],  cospi[56], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf(-cospi[ 8], buf0[9],  cospi[56], buf0[8], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[40], buf0[10],  cospi[24], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[40], buf0[11],  cospi[24], buf0[10], cos_bit[stage_idx]);
  buf1[12] = half_btf(-cospi[56], buf0[12],  cospi[ 8], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[56], buf0[13],  cospi[ 8], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[24], buf0[14],  cospi[40], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[24], buf0[15],  cospi[40], buf0[14], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = buf0[20];
  buf1[21] = buf0[21];
  buf1[22] = buf0[22];
  buf1[23] = buf0[23];
  buf1[24] = half_btf( cospi[ 8], buf0[24],  cospi[56], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf(-cospi[ 8], buf0[25],  cospi[56], buf0[24], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[40], buf0[26],  cospi[24], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf(-cospi[40], buf0[27],  cospi[24], buf0[26], cos_bit[stage_idx]);
  buf1[28] = half_btf(-cospi[56], buf0[28],  cospi[ 8], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[56], buf0[29],  cospi[ 8], buf0[28], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[24], buf0[30],  cospi[40], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[24], buf0[31],  cospi[40], buf0[30], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] =-buf0[4] + buf0[0];
  buf1[5] =-buf0[5] + buf0[1];
  buf1[6] =-buf0[6] + buf0[2];
  buf1[7] =-buf0[7] + buf0[3];
  buf1[8] = buf0[8] + buf0[12];
  buf1[9] = buf0[9] + buf0[13];
  buf1[10] = buf0[10] + buf0[14];
  buf1[11] = buf0[11] + buf0[15];
  buf1[12] =-buf0[12] + buf0[8];
  buf1[13] =-buf0[13] + buf0[9];
  buf1[14] =-buf0[14] + buf0[10];
  buf1[15] =-buf0[15] + buf0[11];
  buf1[16] = buf0[16] + buf0[20];
  buf1[17] = buf0[17] + buf0[21];
  buf1[18] = buf0[18] + buf0[22];
  buf1[19] = buf0[19] + buf0[23];
  buf1[20] =-buf0[20] + buf0[16];
  buf1[21] =-buf0[21] + buf0[17];
  buf1[22] =-buf0[22] + buf0[18];
  buf1[23] =-buf0[23] + buf0[19];
  buf1[24] = buf0[24] + buf0[28];
  buf1[25] = buf0[25] + buf0[29];
  buf1[26] = buf0[26] + buf0[30];
  buf1[27] = buf0[27] + buf0[31];
  buf1[28] =-buf0[28] + buf0[24];
  buf1[29] =-buf0[29] + buf0[25];
  buf1[30] =-buf0[30] + buf0[26];
  buf1[31] =-buf0[31] + buf0[27];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf(-cospi[16], buf0[5],  cospi[48], buf0[4], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[48], buf0[7],  cospi[16], buf0[6], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = half_btf( cospi[16], buf0[12],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf(-cospi[16], buf0[13],  cospi[48], buf0[12], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[48], buf0[14],  cospi[16], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[48], buf0[15],  cospi[16], buf0[14], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = half_btf( cospi[16], buf0[20],  cospi[48], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[16], buf0[21],  cospi[48], buf0[20], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[48], buf0[22],  cospi[16], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[48], buf0[23],  cospi[16], buf0[22], cos_bit[stage_idx]);
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = buf0[26];
  buf1[27] = buf0[27];
  buf1[28] = half_btf( cospi[16], buf0[28],  cospi[48], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf(-cospi[16], buf0[29],  cospi[48], buf0[28], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[48], buf0[30],  cospi[16], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[48], buf0[31],  cospi[16], buf0[30], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] =-buf0[2] + buf0[0];
  buf1[3] =-buf0[3] + buf0[1];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] =-buf0[6] + buf0[4];
  buf1[7] =-buf0[7] + buf0[5];
  buf1[8] = buf0[8] + buf0[10];
  buf1[9] = buf0[9] + buf0[11];
  buf1[10] =-buf0[10] + buf0[8];
  buf1[11] =-buf0[11] + buf0[9];
  buf1[12] = buf0[12] + buf0[14];
  buf1[13] = buf0[13] + buf0[15];
  buf1[14] =-buf0[14] + buf0[12];
  buf1[15] =-buf0[15] + buf0[13];
  buf1[16] = buf0[16] + buf0[18];
  buf1[17] = buf0[17] + buf0[19];
  buf1[18] =-buf0[18] + buf0[16];
  buf1[19] =-buf0[19] + buf0[17];
  buf1[20] = buf0[20] + buf0[22];
  buf1[21] = buf0[21] + buf0[23];
  buf1[22] =-buf0[22] + buf0[20];
  buf1[23] =-buf0[23] + buf0[21];
  buf1[24] = buf0[24] + buf0[26];
  buf1[25] = buf0[25] + buf0[27];
  buf1[26] =-buf0[26] + buf0[24];
  buf1[27] =-buf0[27] + buf0[25];
  buf1[28] = buf0[28] + buf0[30];
  buf1[29] = buf0[29] + buf0[31];
  buf1[30] =-buf0[30] + buf0[28];
  buf1[31] =-buf0[31] + buf0[29];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 10
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf(-cospi[32], buf0[3],  cospi[32], buf0[2], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf(-cospi[32], buf0[7],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[10], cos_bit[stage_idx]);
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = half_btf( cospi[32], buf0[14],  cospi[32], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf(-cospi[32], buf0[15],  cospi[32], buf0[14], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = half_btf( cospi[32], buf0[18],  cospi[32], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf(-cospi[32], buf0[19],  cospi[32], buf0[18], cos_bit[stage_idx]);
  buf1[20] = buf0[20];
  buf1[21] = buf0[21];
  buf1[22] = half_btf( cospi[32], buf0[22],  cospi[32], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf(-cospi[32], buf0[23],  cospi[32], buf0[22], cos_bit[stage_idx]);
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = half_btf( cospi[32], buf0[26],  cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf(-cospi[32], buf0[27],  cospi[32], buf0[26], cos_bit[stage_idx]);
  buf1[28] = buf0[28];
  buf1[29] = buf0[29];
  buf1[30] = half_btf( cospi[32], buf0[30],  cospi[32], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf(-cospi[32], buf0[31],  cospi[32], buf0[30], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 11
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] =-buf0[16];
  buf1[2] = buf0[24];
  buf1[3] =-buf0[8];
  buf1[4] = buf0[12];
  buf1[5] =-buf0[28];
  buf1[6] = buf0[20];
  buf1[7] =-buf0[4];
  buf1[8] = buf0[6];
  buf1[9] =-buf0[22];
  buf1[10] = buf0[30];
  buf1[11] =-buf0[14];
  buf1[12] = buf0[10];
  buf1[13] =-buf0[26];
  buf1[14] = buf0[18];
  buf1[15] =-buf0[2];
  buf1[16] = buf0[3];
  buf1[17] =-buf0[19];
  buf1[18] = buf0[27];
  buf1[19] =-buf0[11];
  buf1[20] = buf0[15];
  buf1[21] =-buf0[31];
  buf1[22] = buf0[23];
  buf1[23] =-buf0[7];
  buf1[24] = buf0[5];
  buf1[25] =-buf0[21];
  buf1[26] = buf0[29];
  buf1[27] =-buf0[13];
  buf1[28] = buf0[9];
  buf1[29] =-buf0[25];
  buf1[30] = buf0[17];
  buf1[31] =-buf0[1];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_idct4_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 4;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[4];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] = input[2];
  buf1[2] = input[1];
  buf1[3] = input[3];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[32], buf0[0], -cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2], -cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[16], buf0[2],  cospi[48], buf0[3], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] = buf0[1] - buf0[2];
  buf1[3] = buf0[0] - buf0[3];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_idct8_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 8;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[8];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] = input[4];
  buf1[2] = input[2];
  buf1[3] = input[6];
  buf1[4] = input[1];
  buf1[5] = input[5];
  buf1[6] = input[3];
  buf1[7] = input[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4], -cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5], -cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[40], buf0[5],  cospi[24], buf0[6], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[ 8], buf0[4],  cospi[56], buf0[7], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[32], buf0[0], -cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2], -cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[16], buf0[2],  cospi[48], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] = buf0[4] - buf0[5];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[6] + buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] = buf0[1] - buf0[2];
  buf1[3] = buf0[0] - buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[7];
  buf1[1] = buf0[1] + buf0[6];
  buf1[2] = buf0[2] + buf0[5];
  buf1[3] = buf0[3] + buf0[4];
  buf1[4] = buf0[3] - buf0[4];
  buf1[5] = buf0[2] - buf0[5];
  buf1[6] = buf0[1] - buf0[6];
  buf1[7] = buf0[0] - buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_idct16_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 16;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[16];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] = input[8];
  buf1[2] = input[4];
  buf1[3] = input[12];
  buf1[4] = input[2];
  buf1[5] = input[10];
  buf1[6] = input[6];
  buf1[7] = input[14];
  buf1[8] = input[1];
  buf1[9] = input[9];
  buf1[10] = input[5];
  buf1[11] = input[13];
  buf1[12] = input[3];
  buf1[13] = input[11];
  buf1[14] = input[7];
  buf1[15] = input[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[60], buf0[8], -cospi[ 4], buf0[15], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[28], buf0[9], -cospi[36], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[44], buf0[10], -cospi[20], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[12], buf0[11], -cospi[52], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[52], buf0[11],  cospi[12], buf0[12], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[20], buf0[10],  cospi[44], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[36], buf0[9],  cospi[28], buf0[14], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[ 4], buf0[8],  cospi[60], buf0[15], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4], -cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5], -cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[40], buf0[5],  cospi[24], buf0[6], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[ 8], buf0[4],  cospi[56], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8] + buf0[9];
  buf1[9] = buf0[8] - buf0[9];
  buf1[10] =-buf0[10] + buf0[11];
  buf1[11] = buf0[10] + buf0[11];
  buf1[12] = buf0[12] + buf0[13];
  buf1[13] = buf0[12] - buf0[13];
  buf1[14] =-buf0[14] + buf0[15];
  buf1[15] = buf0[14] + buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[32], buf0[0], -cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2], -cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[16], buf0[2],  cospi[48], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] = buf0[4] - buf0[5];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[6] + buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = half_btf(-cospi[16], buf0[9],  cospi[48], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf(-cospi[48], buf0[10], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = half_btf(-cospi[16], buf0[10],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[48], buf0[9],  cospi[16], buf0[14], cos_bit[stage_idx]);
  buf1[15] = buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] = buf0[1] - buf0[2];
  buf1[3] = buf0[0] - buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  buf1[8] = buf0[8] + buf0[11];
  buf1[9] = buf0[9] + buf0[10];
  buf1[10] = buf0[9] - buf0[10];
  buf1[11] = buf0[8] - buf0[11];
  buf1[12] =-buf0[12] + buf0[15];
  buf1[13] =-buf0[13] + buf0[14];
  buf1[14] = buf0[13] + buf0[14];
  buf1[15] = buf0[12] + buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[7];
  buf1[1] = buf0[1] + buf0[6];
  buf1[2] = buf0[2] + buf0[5];
  buf1[3] = buf0[3] + buf0[4];
  buf1[4] = buf0[3] - buf0[4];
  buf1[5] = buf0[2] - buf0[5];
  buf1[6] = buf0[1] - buf0[6];
  buf1[7] = buf0[0] - buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf(-cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[15];
  buf1[1] = buf0[1] + buf0[14];
  buf1[2] = buf0[2] + buf0[13];
  buf1[3] = buf0[3] + buf0[12];
  buf1[4] = buf0[4] + buf0[11];
  buf1[5] = buf0[5] + buf0[10];
  buf1[6] = buf0[6] + buf0[9];
  buf1[7] = buf0[7] + buf0[8];
  buf1[8] = buf0[7] - buf0[8];
  buf1[9] = buf0[6] - buf0[9];
  buf1[10] = buf0[5] - buf0[10];
  buf1[11] = buf0[4] - buf0[11];
  buf1[12] = buf0[3] - buf0[12];
  buf1[13] = buf0[2] - buf0[13];
  buf1[14] = buf0[1] - buf0[14];
  buf1[15] = buf0[0] - buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_idct32_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 32;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[32];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] = input[16];
  buf1[2] = input[8];
  buf1[3] = input[24];
  buf1[4] = input[4];
  buf1[5] = input[20];
  buf1[6] = input[12];
  buf1[7] = input[28];
  buf1[8] = input[2];
  buf1[9] = input[18];
  buf1[10] = input[10];
  buf1[11] = input[26];
  buf1[12] = input[6];
  buf1[13] = input[22];
  buf1[14] = input[14];
  buf1[15] = input[30];
  buf1[16] = input[1];
  buf1[17] = input[17];
  buf1[18] = input[9];
  buf1[19] = input[25];
  buf1[20] = input[5];
  buf1[21] = input[21];
  buf1[22] = input[13];
  buf1[23] = input[29];
  buf1[24] = input[3];
  buf1[25] = input[19];
  buf1[26] = input[11];
  buf1[27] = input[27];
  buf1[28] = input[7];
  buf1[29] = input[23];
  buf1[30] = input[15];
  buf1[31] = input[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = half_btf( cospi[62], buf0[16], -cospi[ 2], buf0[31], cos_bit[stage_idx]);
  buf1[17] = half_btf( cospi[30], buf0[17], -cospi[34], buf0[30], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[46], buf0[18], -cospi[18], buf0[29], cos_bit[stage_idx]);
  buf1[19] = half_btf( cospi[14], buf0[19], -cospi[50], buf0[28], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[54], buf0[20], -cospi[10], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf( cospi[22], buf0[21], -cospi[42], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[38], buf0[22], -cospi[26], buf0[25], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[ 6], buf0[23], -cospi[58], buf0[24], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[58], buf0[23],  cospi[ 6], buf0[24], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[26], buf0[22],  cospi[38], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[42], buf0[21],  cospi[22], buf0[26], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[10], buf0[20],  cospi[54], buf0[27], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[50], buf0[19],  cospi[14], buf0[28], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[18], buf0[18],  cospi[46], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[34], buf0[17],  cospi[30], buf0[30], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[ 2], buf0[16],  cospi[62], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[60], buf0[8], -cospi[ 4], buf0[15], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[28], buf0[9], -cospi[36], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[44], buf0[10], -cospi[20], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[12], buf0[11], -cospi[52], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[52], buf0[11],  cospi[12], buf0[12], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[20], buf0[10],  cospi[44], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[36], buf0[9],  cospi[28], buf0[14], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[ 4], buf0[8],  cospi[60], buf0[15], cos_bit[stage_idx]);
  buf1[16] = buf0[16] + buf0[17];
  buf1[17] = buf0[16] - buf0[17];
  buf1[18] =-buf0[18] + buf0[19];
  buf1[19] = buf0[18] + buf0[19];
  buf1[20] = buf0[20] + buf0[21];
  buf1[21] = buf0[20] - buf0[21];
  buf1[22] =-buf0[22] + buf0[23];
  buf1[23] = buf0[22] + buf0[23];
  buf1[24] = buf0[24] + buf0[25];
  buf1[25] = buf0[24] - buf0[25];
  buf1[26] =-buf0[26] + buf0[27];
  buf1[27] = buf0[26] + buf0[27];
  buf1[28] = buf0[28] + buf0[29];
  buf1[29] = buf0[28] - buf0[29];
  buf1[30] =-buf0[30] + buf0[31];
  buf1[31] = buf0[30] + buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[56], buf0[4], -cospi[ 8], buf0[7], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[24], buf0[5], -cospi[40], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[40], buf0[5],  cospi[24], buf0[6], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[ 8], buf0[4],  cospi[56], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8] + buf0[9];
  buf1[9] = buf0[8] - buf0[9];
  buf1[10] =-buf0[10] + buf0[11];
  buf1[11] = buf0[10] + buf0[11];
  buf1[12] = buf0[12] + buf0[13];
  buf1[13] = buf0[12] - buf0[13];
  buf1[14] =-buf0[14] + buf0[15];
  buf1[15] = buf0[14] + buf0[15];
  buf1[16] = buf0[16];
  buf1[17] = half_btf(-cospi[ 8], buf0[17],  cospi[56], buf0[30], cos_bit[stage_idx]);
  buf1[18] = half_btf(-cospi[56], buf0[18], -cospi[ 8], buf0[29], cos_bit[stage_idx]);
  buf1[19] = buf0[19];
  buf1[20] = buf0[20];
  buf1[21] = half_btf(-cospi[40], buf0[21],  cospi[24], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[24], buf0[22], -cospi[40], buf0[25], cos_bit[stage_idx]);
  buf1[23] = buf0[23];
  buf1[24] = buf0[24];
  buf1[25] = half_btf(-cospi[40], buf0[22],  cospi[24], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[24], buf0[21],  cospi[40], buf0[26], cos_bit[stage_idx]);
  buf1[27] = buf0[27];
  buf1[28] = buf0[28];
  buf1[29] = half_btf(-cospi[ 8], buf0[18],  cospi[56], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[56], buf0[17],  cospi[ 8], buf0[30], cos_bit[stage_idx]);
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = half_btf( cospi[32], buf0[0],  cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[32], buf0[0], -cospi[32], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[48], buf0[2], -cospi[16], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[16], buf0[2],  cospi[48], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4] + buf0[5];
  buf1[5] = buf0[4] - buf0[5];
  buf1[6] =-buf0[6] + buf0[7];
  buf1[7] = buf0[6] + buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = half_btf(-cospi[16], buf0[9],  cospi[48], buf0[14], cos_bit[stage_idx]);
  buf1[10] = half_btf(-cospi[48], buf0[10], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = half_btf(-cospi[16], buf0[10],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[48], buf0[9],  cospi[16], buf0[14], cos_bit[stage_idx]);
  buf1[15] = buf0[15];
  buf1[16] = buf0[16] + buf0[19];
  buf1[17] = buf0[17] + buf0[18];
  buf1[18] = buf0[17] - buf0[18];
  buf1[19] = buf0[16] - buf0[19];
  buf1[20] =-buf0[20] + buf0[23];
  buf1[21] =-buf0[21] + buf0[22];
  buf1[22] = buf0[21] + buf0[22];
  buf1[23] = buf0[20] + buf0[23];
  buf1[24] = buf0[24] + buf0[27];
  buf1[25] = buf0[25] + buf0[26];
  buf1[26] = buf0[25] - buf0[26];
  buf1[27] = buf0[24] - buf0[27];
  buf1[28] =-buf0[28] + buf0[31];
  buf1[29] =-buf0[29] + buf0[30];
  buf1[30] = buf0[29] + buf0[30];
  buf1[31] = buf0[28] + buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[3];
  buf1[1] = buf0[1] + buf0[2];
  buf1[2] = buf0[1] - buf0[2];
  buf1[3] = buf0[0] - buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = half_btf(-cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[32], buf0[5],  cospi[32], buf0[6], cos_bit[stage_idx]);
  buf1[7] = buf0[7];
  buf1[8] = buf0[8] + buf0[11];
  buf1[9] = buf0[9] + buf0[10];
  buf1[10] = buf0[9] - buf0[10];
  buf1[11] = buf0[8] - buf0[11];
  buf1[12] =-buf0[12] + buf0[15];
  buf1[13] =-buf0[13] + buf0[14];
  buf1[14] = buf0[13] + buf0[14];
  buf1[15] = buf0[12] + buf0[15];
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = half_btf(-cospi[16], buf0[18],  cospi[48], buf0[29], cos_bit[stage_idx]);
  buf1[19] = half_btf(-cospi[16], buf0[19],  cospi[48], buf0[28], cos_bit[stage_idx]);
  buf1[20] = half_btf(-cospi[48], buf0[20], -cospi[16], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[48], buf0[21], -cospi[16], buf0[26], cos_bit[stage_idx]);
  buf1[22] = buf0[22];
  buf1[23] = buf0[23];
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = half_btf(-cospi[16], buf0[21],  cospi[48], buf0[26], cos_bit[stage_idx]);
  buf1[27] = half_btf(-cospi[16], buf0[20],  cospi[48], buf0[27], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[48], buf0[19],  cospi[16], buf0[28], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[48], buf0[18],  cospi[16], buf0[29], cos_bit[stage_idx]);
  buf1[30] = buf0[30];
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[7];
  buf1[1] = buf0[1] + buf0[6];
  buf1[2] = buf0[2] + buf0[5];
  buf1[3] = buf0[3] + buf0[4];
  buf1[4] = buf0[3] - buf0[4];
  buf1[5] = buf0[2] - buf0[5];
  buf1[6] = buf0[1] - buf0[6];
  buf1[7] = buf0[0] - buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf(-cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[11] = half_btf(-cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[32], buf0[11],  cospi[32], buf0[12], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[13], cos_bit[stage_idx]);
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = buf0[16] + buf0[23];
  buf1[17] = buf0[17] + buf0[22];
  buf1[18] = buf0[18] + buf0[21];
  buf1[19] = buf0[19] + buf0[20];
  buf1[20] = buf0[19] - buf0[20];
  buf1[21] = buf0[18] - buf0[21];
  buf1[22] = buf0[17] - buf0[22];
  buf1[23] = buf0[16] - buf0[23];
  buf1[24] =-buf0[24] + buf0[31];
  buf1[25] =-buf0[25] + buf0[30];
  buf1[26] =-buf0[26] + buf0[29];
  buf1[27] =-buf0[27] + buf0[28];
  buf1[28] = buf0[27] + buf0[28];
  buf1[29] = buf0[26] + buf0[29];
  buf1[30] = buf0[25] + buf0[30];
  buf1[31] = buf0[24] + buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0] + buf0[15];
  buf1[1] = buf0[1] + buf0[14];
  buf1[2] = buf0[2] + buf0[13];
  buf1[3] = buf0[3] + buf0[12];
  buf1[4] = buf0[4] + buf0[11];
  buf1[5] = buf0[5] + buf0[10];
  buf1[6] = buf0[6] + buf0[9];
  buf1[7] = buf0[7] + buf0[8];
  buf1[8] = buf0[7] - buf0[8];
  buf1[9] = buf0[6] - buf0[9];
  buf1[10] = buf0[5] - buf0[10];
  buf1[11] = buf0[4] - buf0[11];
  buf1[12] = buf0[3] - buf0[12];
  buf1[13] = buf0[2] - buf0[13];
  buf1[14] = buf0[1] - buf0[14];
  buf1[15] = buf0[0] - buf0[15];
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = half_btf(-cospi[32], buf0[20],  cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[21] = half_btf(-cospi[32], buf0[21],  cospi[32], buf0[26], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[32], buf0[22],  cospi[32], buf0[25], cos_bit[stage_idx]);
  buf1[23] = half_btf(-cospi[32], buf0[23],  cospi[32], buf0[24], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[32], buf0[23],  cospi[32], buf0[24], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[32], buf0[22],  cospi[32], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[32], buf0[21],  cospi[32], buf0[26], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[32], buf0[20],  cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[28] = buf0[28];
  buf1[29] = buf0[29];
  buf1[30] = buf0[30];
  buf1[31] = buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[31];
  buf1[1] = buf0[1] + buf0[30];
  buf1[2] = buf0[2] + buf0[29];
  buf1[3] = buf0[3] + buf0[28];
  buf1[4] = buf0[4] + buf0[27];
  buf1[5] = buf0[5] + buf0[26];
  buf1[6] = buf0[6] + buf0[25];
  buf1[7] = buf0[7] + buf0[24];
  buf1[8] = buf0[8] + buf0[23];
  buf1[9] = buf0[9] + buf0[22];
  buf1[10] = buf0[10] + buf0[21];
  buf1[11] = buf0[11] + buf0[20];
  buf1[12] = buf0[12] + buf0[19];
  buf1[13] = buf0[13] + buf0[18];
  buf1[14] = buf0[14] + buf0[17];
  buf1[15] = buf0[15] + buf0[16];
  buf1[16] = buf0[15] - buf0[16];
  buf1[17] = buf0[14] - buf0[17];
  buf1[18] = buf0[13] - buf0[18];
  buf1[19] = buf0[12] - buf0[19];
  buf1[20] = buf0[11] - buf0[20];
  buf1[21] = buf0[10] - buf0[21];
  buf1[22] = buf0[9] - buf0[22];
  buf1[23] = buf0[8] - buf0[23];
  buf1[24] = buf0[7] - buf0[24];
  buf1[25] = buf0[6] - buf0[25];
  buf1[26] = buf0[5] - buf0[26];
  buf1[27] = buf0[4] - buf0[27];
  buf1[28] = buf0[3] - buf0[28];
  buf1[29] = buf0[2] - buf0[29];
  buf1[30] = buf0[1] - buf0[30];
  buf1[31] = buf0[0] - buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_iadst4_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 4;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[4];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] =-input[3];
  buf1[2] =-input[1];
  buf1[3] = input[2];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[32], buf0[2], -cospi[32], buf0[3], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] = buf0[0] - buf0[2];
  buf1[3] = buf0[1] - buf0[3];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 8], buf0[0],  cospi[56], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[56], buf0[0], -cospi[ 8], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[40], buf0[2],  cospi[24], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[24], buf0[2], -cospi[40], buf0[3], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[1];
  buf1[1] = buf0[2];
  buf1[2] = buf0[3];
  buf1[3] = buf0[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_iadst8_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 8;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[8];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] =-input[7];
  buf1[2] =-input[3];
  buf1[3] = input[4];
  buf1[4] =-input[1];
  buf1[5] = input[6];
  buf1[6] = input[2];
  buf1[7] =-input[5];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[32], buf0[2], -cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[32], buf0[6], -cospi[32], buf0[7], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] = buf0[0] - buf0[2];
  buf1[3] = buf0[1] - buf0[3];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] = buf0[4] - buf0[6];
  buf1[7] = buf0[5] - buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[48], buf0[4], -cospi[16], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[16], buf0[6],  cospi[48], buf0[7], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] = buf0[0] - buf0[4];
  buf1[5] = buf0[1] - buf0[5];
  buf1[6] = buf0[2] - buf0[6];
  buf1[7] = buf0[3] - buf0[7];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 4], buf0[0],  cospi[60], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[60], buf0[0], -cospi[ 4], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[20], buf0[2],  cospi[44], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[44], buf0[2], -cospi[20], buf0[3], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[36], buf0[4],  cospi[28], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[28], buf0[4], -cospi[36], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[52], buf0[6],  cospi[12], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[12], buf0[6], -cospi[52], buf0[7], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[1];
  buf1[1] = buf0[6];
  buf1[2] = buf0[3];
  buf1[3] = buf0[4];
  buf1[4] = buf0[5];
  buf1[5] = buf0[2];
  buf1[6] = buf0[7];
  buf1[7] = buf0[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_iadst16_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 16;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[16];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] =-input[15];
  buf1[2] =-input[7];
  buf1[3] = input[8];
  buf1[4] =-input[3];
  buf1[5] = input[12];
  buf1[6] = input[4];
  buf1[7] =-input[11];
  buf1[8] =-input[1];
  buf1[9] = input[14];
  buf1[10] = input[6];
  buf1[11] =-input[9];
  buf1[12] = input[2];
  buf1[13] =-input[13];
  buf1[14] =-input[5];
  buf1[15] = input[10];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[32], buf0[2], -cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[32], buf0[6], -cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[32], buf0[10], -cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = half_btf( cospi[32], buf0[14],  cospi[32], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[32], buf0[14], -cospi[32], buf0[15], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] = buf0[0] - buf0[2];
  buf1[3] = buf0[1] - buf0[3];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] = buf0[4] - buf0[6];
  buf1[7] = buf0[5] - buf0[7];
  buf1[8] = buf0[8] + buf0[10];
  buf1[9] = buf0[9] + buf0[11];
  buf1[10] = buf0[8] - buf0[10];
  buf1[11] = buf0[9] - buf0[11];
  buf1[12] = buf0[12] + buf0[14];
  buf1[13] = buf0[13] + buf0[15];
  buf1[14] = buf0[12] - buf0[14];
  buf1[15] = buf0[13] - buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[48], buf0[4], -cospi[16], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[16], buf0[6],  cospi[48], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = half_btf( cospi[16], buf0[12],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[48], buf0[12], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[48], buf0[14],  cospi[16], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[16], buf0[14],  cospi[48], buf0[15], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] = buf0[0] - buf0[4];
  buf1[5] = buf0[1] - buf0[5];
  buf1[6] = buf0[2] - buf0[6];
  buf1[7] = buf0[3] - buf0[7];
  buf1[8] = buf0[8] + buf0[12];
  buf1[9] = buf0[9] + buf0[13];
  buf1[10] = buf0[10] + buf0[14];
  buf1[11] = buf0[11] + buf0[15];
  buf1[12] = buf0[8] - buf0[12];
  buf1[13] = buf0[9] - buf0[13];
  buf1[14] = buf0[10] - buf0[14];
  buf1[15] = buf0[11] - buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[ 8], buf0[8],  cospi[56], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[56], buf0[8], -cospi[ 8], buf0[9], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[40], buf0[10],  cospi[24], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[24], buf0[10], -cospi[40], buf0[11], cos_bit[stage_idx]);
  buf1[12] = half_btf(-cospi[56], buf0[12],  cospi[ 8], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[ 8], buf0[12],  cospi[56], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[24], buf0[14],  cospi[40], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[40], buf0[14],  cospi[24], buf0[15], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[8];
  buf1[1] = buf0[1] + buf0[9];
  buf1[2] = buf0[2] + buf0[10];
  buf1[3] = buf0[3] + buf0[11];
  buf1[4] = buf0[4] + buf0[12];
  buf1[5] = buf0[5] + buf0[13];
  buf1[6] = buf0[6] + buf0[14];
  buf1[7] = buf0[7] + buf0[15];
  buf1[8] = buf0[0] - buf0[8];
  buf1[9] = buf0[1] - buf0[9];
  buf1[10] = buf0[2] - buf0[10];
  buf1[11] = buf0[3] - buf0[11];
  buf1[12] = buf0[4] - buf0[12];
  buf1[13] = buf0[5] - buf0[13];
  buf1[14] = buf0[6] - buf0[14];
  buf1[15] = buf0[7] - buf0[15];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 2], buf0[0],  cospi[62], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[62], buf0[0], -cospi[ 2], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[10], buf0[2],  cospi[54], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[54], buf0[2], -cospi[10], buf0[3], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[18], buf0[4],  cospi[46], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[46], buf0[4], -cospi[18], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[26], buf0[6],  cospi[38], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[38], buf0[6], -cospi[26], buf0[7], cos_bit[stage_idx]);
  buf1[8] = half_btf( cospi[34], buf0[8],  cospi[30], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[30], buf0[8], -cospi[34], buf0[9], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[42], buf0[10],  cospi[22], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[22], buf0[10], -cospi[42], buf0[11], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[50], buf0[12],  cospi[14], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[14], buf0[12], -cospi[50], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[58], buf0[14],  cospi[ 6], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[ 6], buf0[14], -cospi[58], buf0[15], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[1];
  buf1[1] = buf0[14];
  buf1[2] = buf0[3];
  buf1[3] = buf0[12];
  buf1[4] = buf0[5];
  buf1[5] = buf0[10];
  buf1[6] = buf0[7];
  buf1[7] = buf0[8];
  buf1[8] = buf0[9];
  buf1[9] = buf0[6];
  buf1[10] = buf0[11];
  buf1[11] = buf0[4];
  buf1[12] = buf0[13];
  buf1[13] = buf0[2];
  buf1[14] = buf0[15];
  buf1[15] = buf0[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}

void vp10_iadst32_new(const int32_t *input, int32_t *output, const int8_t* cos_bit, const int8_t* stage_range) {
  const int32_t size = 32;
  const int32_t *cospi;

  int32_t stage_idx = 0;
  int32_t *buf0, *buf1;
  int32_t step[32];

  // stage 0;
  range_check(stage_idx, input, input, size, stage_range[stage_idx]);

  // stage 1;
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf1 = output;
  buf1[0] = input[0];
  buf1[1] =-input[31];
  buf1[2] =-input[15];
  buf1[3] = input[16];
  buf1[4] =-input[7];
  buf1[5] = input[24];
  buf1[6] = input[8];
  buf1[7] =-input[23];
  buf1[8] =-input[3];
  buf1[9] = input[28];
  buf1[10] = input[12];
  buf1[11] =-input[19];
  buf1[12] = input[4];
  buf1[13] =-input[27];
  buf1[14] =-input[11];
  buf1[15] = input[20];
  buf1[16] =-input[1];
  buf1[17] = input[30];
  buf1[18] = input[14];
  buf1[19] =-input[17];
  buf1[20] = input[6];
  buf1[21] =-input[25];
  buf1[22] =-input[9];
  buf1[23] = input[22];
  buf1[24] = input[2];
  buf1[25] =-input[29];
  buf1[26] =-input[13];
  buf1[27] = input[18];
  buf1[28] =-input[5];
  buf1[29] = input[26];
  buf1[30] = input[10];
  buf1[31] =-input[21];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 2
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = half_btf( cospi[32], buf0[2],  cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[32], buf0[2], -cospi[32], buf0[3], cos_bit[stage_idx]);
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = half_btf( cospi[32], buf0[6],  cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[32], buf0[6], -cospi[32], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = half_btf( cospi[32], buf0[10],  cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[32], buf0[10], -cospi[32], buf0[11], cos_bit[stage_idx]);
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = half_btf( cospi[32], buf0[14],  cospi[32], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[32], buf0[14], -cospi[32], buf0[15], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = half_btf( cospi[32], buf0[18],  cospi[32], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf( cospi[32], buf0[18], -cospi[32], buf0[19], cos_bit[stage_idx]);
  buf1[20] = buf0[20];
  buf1[21] = buf0[21];
  buf1[22] = half_btf( cospi[32], buf0[22],  cospi[32], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[32], buf0[22], -cospi[32], buf0[23], cos_bit[stage_idx]);
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = half_btf( cospi[32], buf0[26],  cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[32], buf0[26], -cospi[32], buf0[27], cos_bit[stage_idx]);
  buf1[28] = buf0[28];
  buf1[29] = buf0[29];
  buf1[30] = half_btf( cospi[32], buf0[30],  cospi[32], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[32], buf0[30], -cospi[32], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 3
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[2];
  buf1[1] = buf0[1] + buf0[3];
  buf1[2] = buf0[0] - buf0[2];
  buf1[3] = buf0[1] - buf0[3];
  buf1[4] = buf0[4] + buf0[6];
  buf1[5] = buf0[5] + buf0[7];
  buf1[6] = buf0[4] - buf0[6];
  buf1[7] = buf0[5] - buf0[7];
  buf1[8] = buf0[8] + buf0[10];
  buf1[9] = buf0[9] + buf0[11];
  buf1[10] = buf0[8] - buf0[10];
  buf1[11] = buf0[9] - buf0[11];
  buf1[12] = buf0[12] + buf0[14];
  buf1[13] = buf0[13] + buf0[15];
  buf1[14] = buf0[12] - buf0[14];
  buf1[15] = buf0[13] - buf0[15];
  buf1[16] = buf0[16] + buf0[18];
  buf1[17] = buf0[17] + buf0[19];
  buf1[18] = buf0[16] - buf0[18];
  buf1[19] = buf0[17] - buf0[19];
  buf1[20] = buf0[20] + buf0[22];
  buf1[21] = buf0[21] + buf0[23];
  buf1[22] = buf0[20] - buf0[22];
  buf1[23] = buf0[21] - buf0[23];
  buf1[24] = buf0[24] + buf0[26];
  buf1[25] = buf0[25] + buf0[27];
  buf1[26] = buf0[24] - buf0[26];
  buf1[27] = buf0[25] - buf0[27];
  buf1[28] = buf0[28] + buf0[30];
  buf1[29] = buf0[29] + buf0[31];
  buf1[30] = buf0[28] - buf0[30];
  buf1[31] = buf0[29] - buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 4
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = half_btf( cospi[16], buf0[4],  cospi[48], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[48], buf0[4], -cospi[16], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf(-cospi[48], buf0[6],  cospi[16], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[16], buf0[6],  cospi[48], buf0[7], cos_bit[stage_idx]);
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = half_btf( cospi[16], buf0[12],  cospi[48], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[48], buf0[12], -cospi[16], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[48], buf0[14],  cospi[16], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[16], buf0[14],  cospi[48], buf0[15], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = half_btf( cospi[16], buf0[20],  cospi[48], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf( cospi[48], buf0[20], -cospi[16], buf0[21], cos_bit[stage_idx]);
  buf1[22] = half_btf(-cospi[48], buf0[22],  cospi[16], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[16], buf0[22],  cospi[48], buf0[23], cos_bit[stage_idx]);
  buf1[24] = buf0[24];
  buf1[25] = buf0[25];
  buf1[26] = buf0[26];
  buf1[27] = buf0[27];
  buf1[28] = half_btf( cospi[16], buf0[28],  cospi[48], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[48], buf0[28], -cospi[16], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[48], buf0[30],  cospi[16], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[16], buf0[30],  cospi[48], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 5
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[4];
  buf1[1] = buf0[1] + buf0[5];
  buf1[2] = buf0[2] + buf0[6];
  buf1[3] = buf0[3] + buf0[7];
  buf1[4] = buf0[0] - buf0[4];
  buf1[5] = buf0[1] - buf0[5];
  buf1[6] = buf0[2] - buf0[6];
  buf1[7] = buf0[3] - buf0[7];
  buf1[8] = buf0[8] + buf0[12];
  buf1[9] = buf0[9] + buf0[13];
  buf1[10] = buf0[10] + buf0[14];
  buf1[11] = buf0[11] + buf0[15];
  buf1[12] = buf0[8] - buf0[12];
  buf1[13] = buf0[9] - buf0[13];
  buf1[14] = buf0[10] - buf0[14];
  buf1[15] = buf0[11] - buf0[15];
  buf1[16] = buf0[16] + buf0[20];
  buf1[17] = buf0[17] + buf0[21];
  buf1[18] = buf0[18] + buf0[22];
  buf1[19] = buf0[19] + buf0[23];
  buf1[20] = buf0[16] - buf0[20];
  buf1[21] = buf0[17] - buf0[21];
  buf1[22] = buf0[18] - buf0[22];
  buf1[23] = buf0[19] - buf0[23];
  buf1[24] = buf0[24] + buf0[28];
  buf1[25] = buf0[25] + buf0[29];
  buf1[26] = buf0[26] + buf0[30];
  buf1[27] = buf0[27] + buf0[31];
  buf1[28] = buf0[24] - buf0[28];
  buf1[29] = buf0[25] - buf0[29];
  buf1[30] = buf0[26] - buf0[30];
  buf1[31] = buf0[27] - buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 6
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = half_btf( cospi[ 8], buf0[8],  cospi[56], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[56], buf0[8], -cospi[ 8], buf0[9], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[40], buf0[10],  cospi[24], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[24], buf0[10], -cospi[40], buf0[11], cos_bit[stage_idx]);
  buf1[12] = half_btf(-cospi[56], buf0[12],  cospi[ 8], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[ 8], buf0[12],  cospi[56], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf(-cospi[24], buf0[14],  cospi[40], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[40], buf0[14],  cospi[24], buf0[15], cos_bit[stage_idx]);
  buf1[16] = buf0[16];
  buf1[17] = buf0[17];
  buf1[18] = buf0[18];
  buf1[19] = buf0[19];
  buf1[20] = buf0[20];
  buf1[21] = buf0[21];
  buf1[22] = buf0[22];
  buf1[23] = buf0[23];
  buf1[24] = half_btf( cospi[ 8], buf0[24],  cospi[56], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[56], buf0[24], -cospi[ 8], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[40], buf0[26],  cospi[24], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[24], buf0[26], -cospi[40], buf0[27], cos_bit[stage_idx]);
  buf1[28] = half_btf(-cospi[56], buf0[28],  cospi[ 8], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[ 8], buf0[28],  cospi[56], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[24], buf0[30],  cospi[40], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[40], buf0[30],  cospi[24], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 7
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[8];
  buf1[1] = buf0[1] + buf0[9];
  buf1[2] = buf0[2] + buf0[10];
  buf1[3] = buf0[3] + buf0[11];
  buf1[4] = buf0[4] + buf0[12];
  buf1[5] = buf0[5] + buf0[13];
  buf1[6] = buf0[6] + buf0[14];
  buf1[7] = buf0[7] + buf0[15];
  buf1[8] = buf0[0] - buf0[8];
  buf1[9] = buf0[1] - buf0[9];
  buf1[10] = buf0[2] - buf0[10];
  buf1[11] = buf0[3] - buf0[11];
  buf1[12] = buf0[4] - buf0[12];
  buf1[13] = buf0[5] - buf0[13];
  buf1[14] = buf0[6] - buf0[14];
  buf1[15] = buf0[7] - buf0[15];
  buf1[16] = buf0[16] + buf0[24];
  buf1[17] = buf0[17] + buf0[25];
  buf1[18] = buf0[18] + buf0[26];
  buf1[19] = buf0[19] + buf0[27];
  buf1[20] = buf0[20] + buf0[28];
  buf1[21] = buf0[21] + buf0[29];
  buf1[22] = buf0[22] + buf0[30];
  buf1[23] = buf0[23] + buf0[31];
  buf1[24] = buf0[16] - buf0[24];
  buf1[25] = buf0[17] - buf0[25];
  buf1[26] = buf0[18] - buf0[26];
  buf1[27] = buf0[19] - buf0[27];
  buf1[28] = buf0[20] - buf0[28];
  buf1[29] = buf0[21] - buf0[29];
  buf1[30] = buf0[22] - buf0[30];
  buf1[31] = buf0[23] - buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 8
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = buf0[0];
  buf1[1] = buf0[1];
  buf1[2] = buf0[2];
  buf1[3] = buf0[3];
  buf1[4] = buf0[4];
  buf1[5] = buf0[5];
  buf1[6] = buf0[6];
  buf1[7] = buf0[7];
  buf1[8] = buf0[8];
  buf1[9] = buf0[9];
  buf1[10] = buf0[10];
  buf1[11] = buf0[11];
  buf1[12] = buf0[12];
  buf1[13] = buf0[13];
  buf1[14] = buf0[14];
  buf1[15] = buf0[15];
  buf1[16] = half_btf( cospi[ 4], buf0[16],  cospi[60], buf0[17], cos_bit[stage_idx]);
  buf1[17] = half_btf( cospi[60], buf0[16], -cospi[ 4], buf0[17], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[20], buf0[18],  cospi[44], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf( cospi[44], buf0[18], -cospi[20], buf0[19], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[36], buf0[20],  cospi[28], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf( cospi[28], buf0[20], -cospi[36], buf0[21], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[52], buf0[22],  cospi[12], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[12], buf0[22], -cospi[52], buf0[23], cos_bit[stage_idx]);
  buf1[24] = half_btf(-cospi[60], buf0[24],  cospi[ 4], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[ 4], buf0[24],  cospi[60], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf(-cospi[44], buf0[26],  cospi[20], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[20], buf0[26],  cospi[44], buf0[27], cos_bit[stage_idx]);
  buf1[28] = half_btf(-cospi[28], buf0[28],  cospi[36], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[36], buf0[28],  cospi[28], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf(-cospi[12], buf0[30],  cospi[52], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[52], buf0[30],  cospi[12], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 9
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[0] + buf0[16];
  buf1[1] = buf0[1] + buf0[17];
  buf1[2] = buf0[2] + buf0[18];
  buf1[3] = buf0[3] + buf0[19];
  buf1[4] = buf0[4] + buf0[20];
  buf1[5] = buf0[5] + buf0[21];
  buf1[6] = buf0[6] + buf0[22];
  buf1[7] = buf0[7] + buf0[23];
  buf1[8] = buf0[8] + buf0[24];
  buf1[9] = buf0[9] + buf0[25];
  buf1[10] = buf0[10] + buf0[26];
  buf1[11] = buf0[11] + buf0[27];
  buf1[12] = buf0[12] + buf0[28];
  buf1[13] = buf0[13] + buf0[29];
  buf1[14] = buf0[14] + buf0[30];
  buf1[15] = buf0[15] + buf0[31];
  buf1[16] = buf0[0] - buf0[16];
  buf1[17] = buf0[1] - buf0[17];
  buf1[18] = buf0[2] - buf0[18];
  buf1[19] = buf0[3] - buf0[19];
  buf1[20] = buf0[4] - buf0[20];
  buf1[21] = buf0[5] - buf0[21];
  buf1[22] = buf0[6] - buf0[22];
  buf1[23] = buf0[7] - buf0[23];
  buf1[24] = buf0[8] - buf0[24];
  buf1[25] = buf0[9] - buf0[25];
  buf1[26] = buf0[10] - buf0[26];
  buf1[27] = buf0[11] - buf0[27];
  buf1[28] = buf0[12] - buf0[28];
  buf1[29] = buf0[13] - buf0[29];
  buf1[30] = buf0[14] - buf0[30];
  buf1[31] = buf0[15] - buf0[31];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 10
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = output;
  buf1 = step;
  buf1[0] = half_btf( cospi[ 1], buf0[0],  cospi[63], buf0[1], cos_bit[stage_idx]);
  buf1[1] = half_btf( cospi[63], buf0[0], -cospi[ 1], buf0[1], cos_bit[stage_idx]);
  buf1[2] = half_btf( cospi[ 5], buf0[2],  cospi[59], buf0[3], cos_bit[stage_idx]);
  buf1[3] = half_btf( cospi[59], buf0[2], -cospi[ 5], buf0[3], cos_bit[stage_idx]);
  buf1[4] = half_btf( cospi[ 9], buf0[4],  cospi[55], buf0[5], cos_bit[stage_idx]);
  buf1[5] = half_btf( cospi[55], buf0[4], -cospi[ 9], buf0[5], cos_bit[stage_idx]);
  buf1[6] = half_btf( cospi[13], buf0[6],  cospi[51], buf0[7], cos_bit[stage_idx]);
  buf1[7] = half_btf( cospi[51], buf0[6], -cospi[13], buf0[7], cos_bit[stage_idx]);
  buf1[8] = half_btf( cospi[17], buf0[8],  cospi[47], buf0[9], cos_bit[stage_idx]);
  buf1[9] = half_btf( cospi[47], buf0[8], -cospi[17], buf0[9], cos_bit[stage_idx]);
  buf1[10] = half_btf( cospi[21], buf0[10],  cospi[43], buf0[11], cos_bit[stage_idx]);
  buf1[11] = half_btf( cospi[43], buf0[10], -cospi[21], buf0[11], cos_bit[stage_idx]);
  buf1[12] = half_btf( cospi[25], buf0[12],  cospi[39], buf0[13], cos_bit[stage_idx]);
  buf1[13] = half_btf( cospi[39], buf0[12], -cospi[25], buf0[13], cos_bit[stage_idx]);
  buf1[14] = half_btf( cospi[29], buf0[14],  cospi[35], buf0[15], cos_bit[stage_idx]);
  buf1[15] = half_btf( cospi[35], buf0[14], -cospi[29], buf0[15], cos_bit[stage_idx]);
  buf1[16] = half_btf( cospi[33], buf0[16],  cospi[31], buf0[17], cos_bit[stage_idx]);
  buf1[17] = half_btf( cospi[31], buf0[16], -cospi[33], buf0[17], cos_bit[stage_idx]);
  buf1[18] = half_btf( cospi[37], buf0[18],  cospi[27], buf0[19], cos_bit[stage_idx]);
  buf1[19] = half_btf( cospi[27], buf0[18], -cospi[37], buf0[19], cos_bit[stage_idx]);
  buf1[20] = half_btf( cospi[41], buf0[20],  cospi[23], buf0[21], cos_bit[stage_idx]);
  buf1[21] = half_btf( cospi[23], buf0[20], -cospi[41], buf0[21], cos_bit[stage_idx]);
  buf1[22] = half_btf( cospi[45], buf0[22],  cospi[19], buf0[23], cos_bit[stage_idx]);
  buf1[23] = half_btf( cospi[19], buf0[22], -cospi[45], buf0[23], cos_bit[stage_idx]);
  buf1[24] = half_btf( cospi[49], buf0[24],  cospi[15], buf0[25], cos_bit[stage_idx]);
  buf1[25] = half_btf( cospi[15], buf0[24], -cospi[49], buf0[25], cos_bit[stage_idx]);
  buf1[26] = half_btf( cospi[53], buf0[26],  cospi[11], buf0[27], cos_bit[stage_idx]);
  buf1[27] = half_btf( cospi[11], buf0[26], -cospi[53], buf0[27], cos_bit[stage_idx]);
  buf1[28] = half_btf( cospi[57], buf0[28],  cospi[ 7], buf0[29], cos_bit[stage_idx]);
  buf1[29] = half_btf( cospi[ 7], buf0[28], -cospi[57], buf0[29], cos_bit[stage_idx]);
  buf1[30] = half_btf( cospi[61], buf0[30],  cospi[ 3], buf0[31], cos_bit[stage_idx]);
  buf1[31] = half_btf( cospi[ 3], buf0[30], -cospi[61], buf0[31], cos_bit[stage_idx]);
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);

  // stage 11
  stage_idx++;
  cospi = cospi_arr[cos_bit[stage_idx] - cos_bit_min];
  buf0 = step;
  buf1 = output;
  buf1[0] = buf0[1];
  buf1[1] = buf0[30];
  buf1[2] = buf0[3];
  buf1[3] = buf0[28];
  buf1[4] = buf0[5];
  buf1[5] = buf0[26];
  buf1[6] = buf0[7];
  buf1[7] = buf0[24];
  buf1[8] = buf0[9];
  buf1[9] = buf0[22];
  buf1[10] = buf0[11];
  buf1[11] = buf0[20];
  buf1[12] = buf0[13];
  buf1[13] = buf0[18];
  buf1[14] = buf0[15];
  buf1[15] = buf0[16];
  buf1[16] = buf0[17];
  buf1[17] = buf0[14];
  buf1[18] = buf0[19];
  buf1[19] = buf0[12];
  buf1[20] = buf0[21];
  buf1[21] = buf0[10];
  buf1[22] = buf0[23];
  buf1[23] = buf0[8];
  buf1[24] = buf0[25];
  buf1[25] = buf0[6];
  buf1[26] = buf0[27];
  buf1[27] = buf0[4];
  buf1[28] = buf0[29];
  buf1[29] = buf0[2];
  buf1[30] = buf0[31];
  buf1[31] = buf0[0];
  range_check(stage_idx, input, buf1, size, stage_range[stage_idx]);
}
