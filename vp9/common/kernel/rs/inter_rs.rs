#pragma version(1)
#pragma rs java_package_name(com.mcw.gpu.vp9)
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

  float4 filter_part[2];
  float4 value_part[4];
  float4 res;
  uint4  src_temp;
  src += (h * src_stride + w - 3);
  filter_part[0] = convert_float4(*((short4 *)filter_x0));
  filter_part[1] = convert_float4(*((short4 *)(filter_x0 + 4)));

  for (int y = 0; y < 15; y++) {
    src_temp = *((uint4 *)((uchar4 *)src));
    value_part[0] = convert_float4((uchar4){
        (src_temp.x)&0xFF, (src_temp.x >> 8)&0xFF,
        (src_temp.x >> 16)&0xFF, (src_temp.x >> 24)&0xFF});
    value_part[1] = convert_float4((uchar4){
        (src_temp.y)&0xFF, (src_temp.y >> 8)&0xFF,
        (src_temp.y >> 16)&0xFF, (src_temp.y >> 24)&0xFF});
    value_part[2] = convert_float4((uchar4){
        (src_temp.z)&0xFF, (src_temp.z >> 8)&0xFF,
        (src_temp.z >> 16)&0xFF, (src_temp.z >> 24)&0xFF});
    value_part[3] = convert_float4((uchar4){
        (src_temp.w)&0xFF, (src_temp.w >> 8)&0xFF,
        (src_temp.w >> 16)&0xFF, (src_temp.w >> 24)&0xFF});

    res.x =  value_part[0].x * filter_part[0].x;
    res.x += value_part[0].y * filter_part[0].y;
    res.x += value_part[0].z * filter_part[0].z;
    res.x += value_part[0].w * filter_part[0].w;
    res.x += value_part[1].x * filter_part[1].x;
    res.x += value_part[1].y * filter_part[1].y;
    res.x += value_part[1].z * filter_part[1].z;
    res.x += value_part[1].w * filter_part[1].w;

    res.y =  value_part[0].y * filter_part[0].x;
    res.y += value_part[0].z * filter_part[0].y;
    res.y += value_part[0].w * filter_part[0].z;
    res.y += value_part[1].x * filter_part[0].w;
    res.y += value_part[1].y * filter_part[1].x;
    res.y += value_part[1].z * filter_part[1].y;
    res.y += value_part[1].w * filter_part[1].z;
    res.y += value_part[2].x * filter_part[1].w;

    res.z =  value_part[0].z * filter_part[0].x;
    res.z += value_part[0].w * filter_part[0].y;
    res.z += value_part[1].x * filter_part[0].z;
    res.z += value_part[1].y * filter_part[0].w;
    res.z += value_part[1].z * filter_part[1].x;
    res.z += value_part[1].w * filter_part[1].y;
    res.z += value_part[2].x * filter_part[1].z;
    res.z += value_part[2].y * filter_part[1].w;

    res.w =  value_part[0].w * filter_part[0].x;
    res.w += value_part[1].x * filter_part[0].y;
    res.w += value_part[1].y * filter_part[0].z;
    res.w += value_part[1].z * filter_part[0].w;
    res.w += value_part[1].w * filter_part[1].x;
    res.w += value_part[2].x * filter_part[1].y;
    res.w += value_part[2].y * filter_part[1].z;
    res.w += value_part[2].z * filter_part[1].w;

    *((uchar4 *)&dst[0] + y) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));

    res.x =  value_part[1].x * filter_part[0].x;
    res.x += value_part[1].y * filter_part[0].y;
    res.x += value_part[1].z * filter_part[0].z;
    res.x += value_part[1].w * filter_part[0].w;
    res.x += value_part[2].x * filter_part[1].x;
    res.x += value_part[2].y * filter_part[1].y;
    res.x += value_part[2].z * filter_part[1].z;
    res.x += value_part[2].w * filter_part[1].w;

    res.y =  value_part[1].y * filter_part[0].x;
    res.y += value_part[1].z * filter_part[0].y;
    res.y += value_part[1].w * filter_part[0].z;
    res.y += value_part[2].x * filter_part[0].w;
    res.y += value_part[2].y * filter_part[1].x;
    res.y += value_part[2].z * filter_part[1].y;
    res.y += value_part[2].w * filter_part[1].z;
    res.y += value_part[3].x * filter_part[1].w;

    res.z =  value_part[1].z * filter_part[0].x;
    res.z += value_part[1].w * filter_part[0].y;
    res.z += value_part[2].x * filter_part[0].z;
    res.z += value_part[2].y * filter_part[0].w;
    res.z += value_part[2].z * filter_part[1].x;
    res.z += value_part[2].w * filter_part[1].y;
    res.z += value_part[3].x * filter_part[1].z;
    res.z += value_part[3].y * filter_part[1].w;

    res.w =  value_part[1].w * filter_part[0].x;
    res.w += value_part[2].x * filter_part[0].y;
    res.w += value_part[2].y * filter_part[0].z;
    res.w += value_part[2].z * filter_part[0].w;
    res.w += value_part[2].w * filter_part[1].x;
    res.w += value_part[3].x * filter_part[1].y;
    res.w += value_part[3].y * filter_part[1].z;
    res.w += value_part[3].z * filter_part[1].w;

    *((uchar4 *)&dst[60] + y) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));

    src += src_stride;
  }
}

static void vert_rs(const uint8_t *src,
                    uint8_t *dst, int dst_stride,
                    const int16_t *filter_y0,
                    int w, int h) {
  int x, y, k;
  dst += (h * dst_stride + w);

  short4 filter_part[2];
  uint4  src_temp;
  float4 res;
  float4 value[15];
  filter_part[0] = *(short4 *)filter_y0;
  filter_part[1] = *(short4 *)(filter_y0 + 4);
  for(int i = 0; i < 2; ++i)
  {
    src_temp = *((uint4 *)((uchar4 *)&src[i*60] +  0 + 0));
    value [0] = convert_float4((uchar4){
        (src_temp.x)&0xFF, (src_temp.x >> 8)&0xFF,
        (src_temp.x >> 16)&0xFF, (src_temp.x >> 24)&0xFF});
    value [1] = convert_float4((uchar4){
        (src_temp.y)&0xFF, (src_temp.y >> 8)&0xFF,
        (src_temp.y >> 16)&0xFF, (src_temp.y >> 24)&0xFF});
    value [2] = convert_float4((uchar4){
        (src_temp.z)&0xFF, (src_temp.z >> 8)&0xFF,
        (src_temp.z >> 16)&0xFF, (src_temp.z >> 24)&0xFF});
    value [3] = convert_float4((uchar4){
        (src_temp.w)&0xFF, (src_temp.w >> 8)&0xFF,
        (src_temp.w >> 16)&0xFF, (src_temp.w >> 24)&0xFF});
    src_temp = *((uint4 *)((uchar4 *)&src[i*60] +  4 + 0));
    value [4] = convert_float4((uchar4){
        (src_temp.x)&0xFF, (src_temp.x >> 8)&0xFF,
        (src_temp.x >> 16)&0xFF, (src_temp.x >> 24)&0xFF});
    value [5] = convert_float4((uchar4){
        (src_temp.y)&0xFF, (src_temp.y >> 8)&0xFF,
        (src_temp.y >> 16)&0xFF, (src_temp.y >> 24)&0xFF});
    value [6] = convert_float4((uchar4){
        (src_temp.z)&0xFF, (src_temp.z >> 8)&0xFF,
        (src_temp.z >> 16)&0xFF, (src_temp.z >> 24)&0xFF});
    value [7] = convert_float4((uchar4){
        (src_temp.w)&0xFF, (src_temp.w >> 8)&0xFF,
        (src_temp.w >> 16)&0xFF, (src_temp.w >> 24)&0xFF});
    src_temp = *((uint4 *)((uchar4 *)&src[i*60] +  8 + 0));
    value [8] = convert_float4((uchar4){
        (src_temp.x)&0xFF, (src_temp.x >> 8)&0xFF,
        (src_temp.x >> 16)&0xFF, (src_temp.x >> 24)&0xFF});
    value [9] = convert_float4((uchar4){
        (src_temp.y)&0xFF, (src_temp.y >> 8)&0xFF,
        (src_temp.y >> 16)&0xFF, (src_temp.y >> 24)&0xFF});
    value[10] = convert_float4((uchar4){
        (src_temp.z)&0xFF, (src_temp.z >> 8)&0xFF,
        (src_temp.z >> 16)&0xFF, (src_temp.z >> 24)&0xFF});
    value[11] = convert_float4((uchar4){
        (src_temp.w)&0xFF, (src_temp.w >> 8)&0xFF,
        (src_temp.w >> 16)&0xFF, (src_temp.w >> 24)&0xFF});
    value[12] = convert_float4(*((uchar4 *)&src[i*60] + 12));
    value[13] = convert_float4(*((uchar4 *)&src[i*60] + 13));
    value[14] = convert_float4(*((uchar4 *)&src[i*60] + 14));
    res  = value [0] * (float)filter_part[0].x;
    res += value [1] * (float)filter_part[0].y;
    res += value [2] * (float)filter_part[0].z;
    res += value [3] * (float)filter_part[0].w;
    res += value [4] * (float)filter_part[1].x;
    res += value [5] * (float)filter_part[1].y;
    res += value [6] * (float)filter_part[1].z;
    res += value [7] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*0) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [1] * (float)filter_part[0].x;
    res += value [2] * (float)filter_part[0].y;
    res += value [3] * (float)filter_part[0].z;
    res += value [4] * (float)filter_part[0].w;
    res += value [5] * (float)filter_part[1].x;
    res += value [6] * (float)filter_part[1].y;
    res += value [7] * (float)filter_part[1].z;
    res += value [8] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*1) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [2] * (float)filter_part[0].x;
    res += value [3] * (float)filter_part[0].y;
    res += value [4] * (float)filter_part[0].z;
    res += value [5] * (float)filter_part[0].w;
    res += value [6] * (float)filter_part[1].x;
    res += value [7] * (float)filter_part[1].y;
    res += value [8] * (float)filter_part[1].z;
    res += value [9] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*2) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [3] * (float)filter_part[0].x;
    res += value [4] * (float)filter_part[0].y;
    res += value [5] * (float)filter_part[0].z;
    res += value [6] * (float)filter_part[0].w;
    res += value [7] * (float)filter_part[1].x;
    res += value [8] * (float)filter_part[1].y;
    res += value [9] * (float)filter_part[1].z;
    res += value[10] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*3) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [4] * (float)filter_part[0].x;
    res += value [5] * (float)filter_part[0].y;
    res += value [6] * (float)filter_part[0].z;
    res += value [7] * (float)filter_part[0].w;
    res += value [8] * (float)filter_part[1].x;
    res += value [9] * (float)filter_part[1].y;
    res += value[10] * (float)filter_part[1].z;
    res += value[11] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*4) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [5] * (float)filter_part[0].x;
    res += value [6] * (float)filter_part[0].y;
    res += value [7] * (float)filter_part[0].z;
    res += value [8] * (float)filter_part[0].w;
    res += value [9] * (float)filter_part[1].x;
    res += value[10] * (float)filter_part[1].y;
    res += value[11] * (float)filter_part[1].z;
    res += value[12] * (float)filter_part[1].w;
    *((uchar4 *)((uchar *)dst + dst_stride*5) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
    res  = value [6] * (float)filter_part[0].x;
    res += value [7] * (float)filter_part[0].y;
    res += value [8] * (float)filter_part[0].z;
    res += value [9] * (float)filter_part[0].w;
    res += value[10] * (float)filter_part[1].x;
    res += value[11] * (float)filter_part[1].y;
    res += value[12] * (float)filter_part[1].z;
    res += value[13] * (float)filter_part[1].w;

    *((uchar4 *)((uchar *)dst + dst_stride*6) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));

    res  = value [7] * (float)filter_part[0].x;
    res += value [8] * (float)filter_part[0].y;
    res += value [9] * (float)filter_part[0].z;
    res += value[10] * (float)filter_part[0].w;
    res += value[11] * (float)filter_part[1].x;
    res += value[12] * (float)filter_part[1].y;
    res += value[13] * (float)filter_part[1].z;
    res += value[14] * (float)filter_part[1].w;

    *((uchar4 *)((uchar *)dst + dst_stride*7) + i) =
        convert_uchar4(clamp(convert_int4(res + 64) >> 7, 0, 255));
  }
}


void root(uchar *count, uint32_t x) {

  if (x >= counts_8x8) return;
  uchar *base_param = &block_index[x * sizeof(INTER_PARAM_INDEX)];
  INTER_PARAM_INDEX *param = (INTER_PARAM_INDEX *)base_param;

  uint block_num = param->param_num;
  uint buf[30];
  base_param = &pred_param[block_num * sizeof(INTER_PRED_PARAM)];

  const INTER_PRED_PARAM *block_param = (INTER_PRED_PARAM *)base_param;
  const uint8_t *src = pool_buf + block_param->src_mv;
  uint8_t *dst = dst_buf + block_param->dst_mv;
  const int16_t *filter_x = vp9_filters + block_param->filter_x_mv;
  const int16_t *filter_y = vp9_filters + block_param->filter_y_mv;
  hor_rs(
      src - 3 * block_param->src_stride,
      block_param->src_stride, (uint8_t *)buf,
      filter_x,
      param->x_mv << 3,
      param->y_mv << 3);

  vert_rs(
      (uint8_t *)buf, dst, block_param->dst_stride, filter_y,
      param->x_mv << 3,
      param->y_mv << 3);
}
