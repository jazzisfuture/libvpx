#pragma version(1)
#pragma rs java_package_name(com.example.vp9)
#pragma rs_fp_relaxed
#include "inter_rs.rsh"

uchar *pred_param;
uchar *block_index;
uchar *dst_buf;
uchar *mid_buf;
int buffer_size;
int counts_8x8;

static void vert_rs(const uint8_t *src,
                    uint8_t *dst, int dst_stride,
                    const int16_t *filter_y0,
                    int w, int h) {
  dst += (h * dst_stride + w);

  float4 filter_part1, filter_part2;
  short4 *filter_tmp = (short4 *)filter_y0;
  filter_part1 = convert_float4(*filter_tmp);
  filter_tmp = (short4 *)(filter_y0 + 4);
  filter_part2 = convert_float4(*filter_tmp);

  float4 value[15][2];
  float4 sum1 = {0}, sum2 = {0};
  int4   value_temp;
  int    i = 0, j = 0;
  
  for (i = 0; i < 8; i++)
  {
    sum1 =  convert_float4((*(uchar4 *)(src + (i+0)*8 + 0))) * filter_part1.x;
    sum1 += convert_float4((*(uchar4 *)(src + (i+1)*8 + 0))) * filter_part1.y;
    sum1 += convert_float4((*(uchar4 *)(src + (i+2)*8 + 0))) * filter_part1.z;
    sum1 += convert_float4((*(uchar4 *)(src + (i+3)*8 + 0))) * filter_part1.w;
    sum1 += convert_float4((*(uchar4 *)(src + (i+4)*8 + 0))) * filter_part2.x;
    sum1 += convert_float4((*(uchar4 *)(src + (i+5)*8 + 0))) * filter_part2.y;
    sum1 += convert_float4((*(uchar4 *)(src + (i+6)*8 + 0))) * filter_part2.z;
    sum1 += convert_float4((*(uchar4 *)(src + (i+7)*8 + 0))) * filter_part2.w;
    
    value_temp = (convert_int4(sum1 + 64)) >> 7;
    ((uchar4 *)dst)[((i*dst_stride)>>2) + 0] =
        convert_uchar4(clamp(value_temp, MIN_VAL, MAX_VAL));
    
    sum2 =  convert_float4((*(uchar4 *)(src + (i+0)*8 + 4))) * filter_part1.x;
    sum2 += convert_float4((*(uchar4 *)(src + (i+1)*8 + 4))) * filter_part1.y;
    sum2 += convert_float4((*(uchar4 *)(src + (i+2)*8 + 4))) * filter_part1.z;
    sum2 += convert_float4((*(uchar4 *)(src + (i+3)*8 + 4))) * filter_part1.w;
    sum2 += convert_float4((*(uchar4 *)(src + (i+4)*8 + 4))) * filter_part2.x;
    sum2 += convert_float4((*(uchar4 *)(src + (i+5)*8 + 4))) * filter_part2.y;
    sum2 += convert_float4((*(uchar4 *)(src + (i+6)*8 + 4))) * filter_part2.z;
    sum2 += convert_float4((*(uchar4 *)(src + (i+7)*8 + 4))) * filter_part2.w;
    
    value_temp = (convert_int4(sum2 + 64)) >> 7;
    ((uchar4 *)dst)[((i*dst_stride)>>2) + 1] =
        convert_uchar4(clamp(value_temp, MIN_VAL, MAX_VAL));
  }

}

void root(uchar *count, uint32_t x) {

  if (x >= counts_8x8) return;
  uchar *base_param = &block_index[x * sizeof(INTER_PARAM_INDEX)];
  INTER_PARAM_INDEX *param = (INTER_PARAM_INDEX *)base_param;

  uint block_num = param->param_num;
  base_param = &pred_param[block_num * sizeof(INTER_PRED_PARAM)];

  const INTER_PRED_PARAM *block_param = (INTER_PRED_PARAM *)base_param;
  uint8_t *buf = mid_buf + x * 120;
  uint8_t *dst = dst_buf + block_param->dst_mv;
  const int16_t *filter_x = vp9_filters + block_param->filter_x_mv;
  const int16_t *filter_y = vp9_filters + block_param->filter_y_mv;

  vert_rs(
      buf, dst, block_param->dst_stride, filter_y,
      param->x_mv << 3,
      param->y_mv << 3);
}
