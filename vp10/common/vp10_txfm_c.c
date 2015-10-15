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
  buf1[0] = round_shift( cospi[32] * buf0[0] +  cospi[32] * buf0[1], cos_bit[stage_idx]);
  buf1[1] = round_shift(-cospi[32] * buf0[1] +  cospi[32] * buf0[0], cos_bit[stage_idx]);
  buf1[2] = round_shift( cospi[48] * buf0[2] +  cospi[16] * buf0[3], cos_bit[stage_idx]);
  buf1[3] = round_shift( cospi[48] * buf0[3] + -cospi[16] * buf0[2], cos_bit[stage_idx]);
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
  buf1[0] = round_shift( cospi[32] * buf0[0] +  cospi[32] * buf0[1], cos_bit[stage_idx]);
  buf1[1] = round_shift( cospi[32] * buf0[0] + -cospi[32] * buf0[1], cos_bit[stage_idx]);
  buf1[2] = round_shift( cospi[48] * buf0[2] + -cospi[16] * buf0[3], cos_bit[stage_idx]);
  buf1[3] = round_shift( cospi[16] * buf0[2] +  cospi[48] * buf0[3], cos_bit[stage_idx]);
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
