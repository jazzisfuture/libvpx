#pragma version(1)
#pragma rs java_package_name(com.example.vp9)
#pragma rs_fp_relaxed
#include "inter_rs.rsh"

uchar *pool_buf;
uchar *pred_param;
uchar *block_index;
uchar *dst_buf;
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
    float4 res;
    float4 value_part1, value_part2, value_part3, value_part4;
    value_part1 = convert_float4(*((uchar4 *)(src + 0)));
    value_part2 = convert_float4(*((uchar4 *)(src + 4)));
    value_part3 = convert_float4(*((uchar4 *)(src + 8)));
    value_part4 = convert_float4(*((uchar4 *)(src + 12)));

    res.x = value_part1.x * filter_part1.x;
    res.x += value_part1.y * filter_part1.y;
    res.x += value_part1.z * filter_part1.z;
    res.x += value_part1.w * filter_part1.w;
    res.x += value_part2.x * filter_part2.x;
    res.x += value_part2.y * filter_part2.y;
    res.x += value_part2.z * filter_part2.z;
    res.x += value_part2.w * filter_part2.w;

    res.y = value_part1.y * filter_part1.x;
    res.y += value_part1.z * filter_part1.y;
    res.y += value_part1.w * filter_part1.z;
    res.y += value_part2.x * filter_part1.w;
    res.y += value_part2.y * filter_part2.x;
    res.y += value_part2.z * filter_part2.y;
    res.y += value_part2.w * filter_part2.z;
    res.y += value_part3.x * filter_part2.w;

    res.z = value_part1.z * filter_part1.x;
    res.z += value_part1.w * filter_part1.y;
    res.z += value_part2.x * filter_part1.z;
    res.z += value_part2.y * filter_part1.w;
    res.z += value_part2.z * filter_part2.x;
    res.z += value_part2.w * filter_part2.y;
    res.z += value_part3.x * filter_part2.z;
    res.z += value_part3.y * filter_part2.w;

    res.w = value_part1.w * filter_part1.x;
    res.w += value_part2.x * filter_part1.y;
    res.w += value_part2.y * filter_part1.z;
    res.w += value_part2.z * filter_part1.w;
    res.w += value_part2.w * filter_part2.x;
    res.w += value_part3.x * filter_part2.y;
    res.w += value_part3.y * filter_part2.z;
    res.w += value_part3.z * filter_part2.w;

    *dst_base = convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    dst_base += 1;

    res.x = value_part2.x * filter_part1.x;
    res.x += value_part2.y * filter_part1.y;
    res.x += value_part2.z * filter_part1.z;
    res.x += value_part2.w * filter_part1.w;
    res.x += value_part3.x * filter_part2.x;
    res.x += value_part3.y * filter_part2.y;
    res.x += value_part3.z * filter_part2.z;
    res.x += value_part3.w * filter_part2.w;

    res.y = value_part2.y * filter_part1.x;
    res.y += value_part2.z * filter_part1.y;
    res.y += value_part2.w * filter_part1.z;
    res.y += value_part3.x * filter_part1.w;
    res.y += value_part3.y * filter_part2.x;
    res.y += value_part3.z * filter_part2.y;
    res.y += value_part3.w * filter_part2.z;
    res.y += value_part4.x * filter_part2.w;

    res.z = value_part2.z * filter_part1.x;
    res.z += value_part2.w * filter_part1.y;
    res.z += value_part3.x * filter_part1.z;
    res.z += value_part3.y * filter_part1.w;
    res.z += value_part3.z * filter_part2.x;
    res.z += value_part3.w * filter_part2.y;
    res.z += value_part4.x * filter_part2.z;
    res.z += value_part4.y * filter_part2.w;

    res.w = value_part2.w * filter_part1.x;
    res.w += value_part3.x * filter_part1.y;
    res.w += value_part3.y * filter_part1.z;
    res.w += value_part3.z * filter_part1.w;
    res.w += value_part3.w * filter_part2.x;
    res.w += value_part4.x * filter_part2.y;
    res.w += value_part4.y * filter_part2.z;
    res.w += value_part4.z * filter_part2.w;

    *dst_base = convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    dst_base += 1;

    src += src_stride;
  }
}

static void vert_rs(const uint8_t *src,
                    uint8_t *dst, int dst_stride,
                    const int16_t *filter_y0,
                    int w, int h) {
  int x, y, k;
  dst += (h * dst_stride + w);

  short4 *filter_part1 = (short4 *)filter_y0;
  short4 *filter_part2 = (short4 *)(filter_y0 + 4);
  float4 res0;
  for (int i = 0; i < 8; i += 4) {
    uchar4 *src_base = (uchar4 *)(src + i);
    uchar4 *dst_base = (uchar4 *)(dst + i);
    float4 value0 = convert_float4(*src_base);
    src_base += 2;
    float4 value1 = convert_float4(*src_base);
    src_base += 2;
    float4 value2 = convert_float4(*src_base);
    src_base += 2;
    float4 value3 = convert_float4(*src_base);
    src_base += 2;
    float4 value4 = convert_float4(*src_base);
    src_base += 2;
    float4 value5 = convert_float4(*src_base);
    src_base += 2;
    float4 value6 = convert_float4(*src_base);
    for (int j = 0; j < 8; j += 4) {
      src_base += 2;
      float4 value7 = convert_float4(*src_base);
      src_base += 2;
      float4 value8 = convert_float4(*src_base);
      src_base += 2;
      float4 value9 = convert_float4(*src_base);
      src_base += 2;
      float4 value10 = convert_float4(*src_base);
      res0  = value0 * (float)filter_part1->x;
      res0 += value1 * (float)filter_part1->y;
      res0 += value2 * (float)filter_part1->z;

      res0 += value3 * (float)filter_part1->w;
      res0 += value4 * (float)filter_part2->x;
      res0 += value5 * (float)filter_part2->y;
      res0 += value6 * (float)filter_part2->z;
      res0 += value7 * (float)filter_part2->w;
      *dst_base = convert_uchar4(clamp(convert_int4(res0 + 64) >> 7, 0, 255));
      dst_base = (uchar4 *)((uchar *)(dst_base) + dst_stride);
      res0  = value1 * (float)filter_part1->x;
      res0 += value2 * (float)filter_part1->y;
      res0 += value3 * (float)filter_part1->z;
      res0 += value4 * (float)filter_part1->w;
      res0 += value5 * (float)filter_part2->x;
      res0 += value6 * (float)filter_part2->y;
      res0 += value7 * (float)filter_part2->z;
      res0 += value8 * (float)filter_part2->w;
      *dst_base = convert_uchar4(clamp(convert_int4(res0 + 64) >> 7, 0, 255));
      dst_base = (uchar4 *)((uchar *)(dst_base) + dst_stride);
      res0  = value2 * (float)filter_part1->x;
      res0 += value3 * (float)filter_part1->y;
      res0 += value4 * (float)filter_part1->z;
      res0 += value5 * (float)filter_part1->w;
      res0 += value6 * (float)filter_part2->x;
      res0 += value7 * (float)filter_part2->y;
      res0 += value8 * (float)filter_part2->z;
      res0 += value9 * (float)filter_part2->w;
      *dst_base = convert_uchar4(clamp(convert_int4(res0 + 64) >> 7, 0, 255));
      dst_base = (uchar4 *)((uchar *)(dst_base) + dst_stride);
      res0  = value3 * (float)filter_part1->x;
      res0 += value4 * (float)filter_part1->y;
      res0 += value5 * (float)filter_part1->z;
      res0 += value6 * (float)filter_part1->w;
      res0 += value7 * (float)filter_part2->x;
      res0 += value8 * (float)filter_part2->y;
      res0 += value9 * (float)filter_part2->z;
      res0 += value10 * (float)filter_part2->w;
      *dst_base = convert_uchar4(clamp(convert_int4(res0 + 64) >> 7, 0, 255));
      dst_base = (uchar4 *)((uchar *)(dst_base) + dst_stride);
      value0 = value4;
      value1 = value5;
      value2 = value6;
      value3 = value7;
      value4 = value8;
      value5 = value9;
      value6 = value10;
    }
  }
}

void root(uchar *count, uint32_t x) {

  if (x >= counts_8x8) return;
  uchar *base_param = &block_index[x * sizeof(INTER_PARAM_INDEX)];
  INTER_PARAM_INDEX *param = (INTER_PARAM_INDEX *)base_param;

  uint block_num = param->param_num;
  uchar buf[120];
  base_param = &pred_param[block_num * sizeof(INTER_PRED_PARAM)];

  const INTER_PRED_PARAM *block_param = (INTER_PRED_PARAM *)base_param;
  const uint8_t *pre_src_buffer =
      &pool_buf[block_param->src_num * buffer_size];
  const uint8_t *src = &pre_src_buffer[block_param->src_mv];
  uint8_t *dst = dst_buf + block_param->dst_mv;
  const int16_t *filter_x = vp9_filters + block_param->filter_x_mv;
  const int16_t *filter_y = vp9_filters + block_param->filter_y_mv;
  hor_rs(
      src - 3 * block_param->src_stride,
      block_param->src_stride, buf,
      filter_x,
      param->x_mv << 3,
      param->y_mv << 3);

  vert_rs(
      buf, dst, block_param->dst_stride, filter_y,
      param->x_mv << 3,
      param->y_mv << 3);
}
