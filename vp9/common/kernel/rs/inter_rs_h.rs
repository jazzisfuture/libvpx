#pragma version(1)
#pragma rs java_package_name(com.example.vp9)
#pragma rs_fp_relaxed
#include "inter_rs.rsh"

uchar *pool_buf;
uchar *pred_param;
uchar *block_index;
uchar *mid_buf;
int buffer_size;
int counts_8x8;

static void hor_rs(const uint8_t *src, int src_stride,
              uint8_t *dst, const int16_t *filter_x0,
              int w, int h) {

  src += (h * src_stride + w - 3);

  float4 filter_part1, filter_part2;
  short4 *filter_tmp = (short4 *)filter_x0;
  filter_part1 = convert_float4(*filter_tmp);
  filter_tmp = (short4 *)(filter_x0 + 4);
  filter_part2 = convert_float4(*filter_tmp);
  uchar4 *dst_base = (uchar4 *)dst;

  for (int y = 0; y < 15; y++) {
    float4 res1, res2;
    float4 value_part1, value_part2, value_part3, value_part4;
    int4   value_temp;

    value_part1 = convert_float4(*(uchar4 *)(src + 0));
    value_part2 = convert_float4(*(uchar4 *)(src + 4));
    value_part3 = convert_float4(*(uchar4 *)(src + 8));
    value_part4 = convert_float4(*(uchar4 *)(src + 12));

    res1.x = value_part1.x * filter_part1.x;
    res1.x += value_part1.y * filter_part1.y;
    res1.x += value_part1.z * filter_part1.z;
    res1.x += value_part1.w * filter_part1.w;
    res1.x += value_part2.x * filter_part2.x;
    res1.x += value_part2.y * filter_part2.y;
    res1.x += value_part2.z * filter_part2.z;
    res1.x += value_part2.w * filter_part2.w;

    res1.y = value_part1.y * filter_part1.x;
    res1.y += value_part1.z * filter_part1.y;
    res1.y += value_part1.w * filter_part1.z;
    res1.y += value_part2.x * filter_part1.w;
    res1.y += value_part2.y * filter_part2.x;
    res1.y += value_part2.z * filter_part2.y;
    res1.y += value_part2.w * filter_part2.z;
    res1.y += value_part3.x * filter_part2.w;

    res1.z = value_part1.z * filter_part1.x;
    res1.z += value_part1.w * filter_part1.y;
    res1.z += value_part2.x * filter_part1.z;
    res1.z += value_part2.y * filter_part1.w;
    res1.z += value_part2.z * filter_part2.x;
    res1.z += value_part2.w * filter_part2.y;
    res1.z += value_part3.x * filter_part2.z;
    res1.z += value_part3.y * filter_part2.w;

    res1.w = value_part1.w * filter_part1.x;
    res1.w += value_part2.x * filter_part1.y;
    res1.w += value_part2.y * filter_part1.z;
    res1.w += value_part2.z * filter_part1.w;
    res1.w += value_part2.w * filter_part2.x;
    res1.w += value_part3.x * filter_part2.y;
    res1.w += value_part3.y * filter_part2.z;
    res1.w += value_part3.z * filter_part2.w;

    value_temp = (convert_int4(res1 + 64)) >> 7;
    ((uchar4 *)dst)[0] = convert_uchar4(clamp(value_temp, MIN_VAL, MAX_VAL));

    res2.x = value_part2.x * filter_part1.x;
    res2.x += value_part2.y * filter_part1.y;
    res2.x += value_part2.z * filter_part1.z;
    res2.x += value_part2.w * filter_part1.w;
    res2.x += value_part3.x * filter_part2.x;
    res2.x += value_part3.y * filter_part2.y;
    res2.x += value_part3.z * filter_part2.z;
    res2.x += value_part3.w * filter_part2.w;

    res2.y = value_part2.y * filter_part1.x;
    res2.y += value_part2.z * filter_part1.y;
    res2.y += value_part2.w * filter_part1.z;
    res2.y += value_part3.x * filter_part1.w;
    res2.y += value_part3.y * filter_part2.x;
    res2.y += value_part3.z * filter_part2.y;
    res2.y += value_part3.w * filter_part2.z;
    res2.y += value_part4.x * filter_part2.w;

    res2.z = value_part2.z * filter_part1.x;
    res2.z += value_part2.w * filter_part1.y;
    res2.z += value_part3.x * filter_part1.z;
    res2.z += value_part3.y * filter_part1.w;
    res2.z += value_part3.z * filter_part2.x;
    res2.z += value_part3.w * filter_part2.y;
    res2.z += value_part4.x * filter_part2.z;
    res2.z += value_part4.y * filter_part2.w;

    res2.w = value_part2.w * filter_part1.x;
    res2.w += value_part3.x * filter_part1.y;
    res2.w += value_part3.y * filter_part1.z;
    res2.w += value_part3.z * filter_part1.w;
    res2.w += value_part3.w * filter_part2.x;
    res2.w += value_part4.x * filter_part2.y;
    res2.w += value_part4.y * filter_part2.z;
    res2.w += value_part4.z * filter_part2.w;

    value_temp = (convert_int4(res2 + 64)) >> 7;
    ((uchar4 *)dst)[1] = convert_uchar4(clamp(value_temp, MIN_VAL, MAX_VAL));

    src += src_stride;
    dst += 8;
  }
}

void root(uchar *count, uint32_t x) {

  if (x >= counts_8x8) return;
  uchar *base_param = &block_index[x * sizeof(INTER_PARAM_INDEX)];
  INTER_PARAM_INDEX *param = (INTER_PARAM_INDEX *)base_param;

  uint block_num = param->param_num;
  base_param = &pred_param[block_num * sizeof(INTER_PRED_PARAM)];

  const INTER_PRED_PARAM *block_param = (INTER_PRED_PARAM *)base_param;
  const uint8_t *pre_src_buffer =
      &pool_buf[block_param->src_num * buffer_size];
  const uint8_t *src = &pre_src_buffer[block_param->src_mv];
  uint8_t *buf = mid_buf + x * 120;
  const int16_t *filter_x = vp9_filters + block_param->filter_x_mv;
  const int16_t *filter_y = vp9_filters + block_param->filter_y_mv;
  hor_rs(
      src - 3 * block_param->src_stride,
      block_param->src_stride, buf,
      filter_x,
      param->x_mv << 3,
      param->y_mv << 3);
}
