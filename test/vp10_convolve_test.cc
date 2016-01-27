#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_config.h"
#include "./vp9_rtcd.h"
#include "./vpx_dsp_rtcd.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/util.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_dsp/vpx_filter.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/mem.h"
#include "vpx_dsp/vpx_convolve.h"

#include "vp10/common/filter.h"

#if CONFIG_INTERPOLATION_FILTER_12
TEST(VP10ConvolveTest, vpx_convolve12_c) {
  uint8_t src[12*12];
  ptrdiff_t filter_size = 12;
  int filter_center = filter_size/2 - 1;
  uint8_t dst[1] = {0};
  const int16_t* filter = vp10_filter_kernels[0][3];
  int step = 16;
  int w = 1;
  int h = 1;

  for(int i = 0; i < filter_size * filter_size; i++) {
    src[i] = rand() % (1<<8);
  }

  vpx_convolve12_c(src + filter_size * filter_center + filter_center , filter_size,
                   dst, 1,
                   filter, step,
                   filter, step,
                   w, h);

  int temp[12];
  int dst_ref = 0;
  for(int r = 0; r < filter_size; r++) {
    temp[r] = 0;
    for(int c = 0; c < filter_size; c++) {
      temp[r] += filter[c] * src[r*filter_size + c];
    }
    temp[r] = clip_pixel(ROUND_POWER_OF_TWO(temp[r], 7));
    dst_ref += temp[r] * filter[r];
  }
  dst_ref = clip_pixel(ROUND_POWER_OF_TWO(dst_ref, 7));
  EXPECT_EQ(dst[0], dst_ref);
}

TEST(VP10ConvolveTest, vpx_convolve12_avg_c2) {
  uint8_t src0[12*12];
  uint8_t src1[12*12];
  ptrdiff_t filter_size = 12;
  int filter_center = filter_size/2 - 1;
  uint8_t dst0[1] = {0};
  uint8_t dst1[1] = {0};
  uint8_t dst[1] = {0};
  const int16_t* filter = vp10_filter_kernels[0][3];
  int step = 16;
  int w = 1;
  int h = 1;

  for(int i = 0; i < filter_size * filter_size; i++) {
    src0[i] = rand() % (1<<8);
    src1[i] = rand() % (1<<8);
  }

  vpx_convolve12_c(src0 + filter_size * filter_center + filter_center , filter_size,
                   dst0, 1,
                   filter, step,
                   filter, step,
                   w, h);

  vpx_convolve12_c(src1 + filter_size * filter_center + filter_center , filter_size,
                   dst1, 1,
                   filter, step,
                   filter, step,
                   w, h);

  vpx_convolve12_c(src0 + filter_size * filter_center + filter_center , filter_size,
                   dst, 1,
                   filter, step,
                   filter, step,
                   w, h);

  vpx_convolve12_avg_c(src1 + filter_size * filter_center + filter_center , filter_size,
                       dst, 1,
                       filter, step,
                       filter, step,
                       w, h);

  EXPECT_EQ(dst[0], ROUND_POWER_OF_TWO(dst0[0] + dst1[0], 1));

}
#if CONFIG_VP9_HIGHBITDEPTH
TEST(VP10ConvolveTest, vpx_highbd_convolve12_c) {
  uint16_t src[12*12];
  ptrdiff_t filter_size = 12;
  int filter_center = filter_size/2 - 1;
  uint16_t dst[1] = {0};
  const int16_t* filter = vp10_filter_kernels[0][0];
  int step = 16;
  int w = 1;
  int h = 1;
  int bd = 10;

  for(int i = 0; i < filter_size * filter_size; i++) {
    src[i] = rand() % (1<<8);
  }

  vpx_highbd_convolve12_c(CONVERT_TO_BYTEPTR(src + filter_size * filter_center + filter_center) , filter_size,
                   CONVERT_TO_BYTEPTR(dst), 1,
                   filter, step,
                   filter, step,
                   w, h, bd);

  int temp[12];
  int dst_ref = 0;
  for(int r = 0; r < filter_size; r++) {
    temp[r] = 0;
    for(int c = 0; c < filter_size; c++) {
      temp[r] += filter[c] * src[r*filter_size + c];
    }
    temp[r] = clip_pixel(ROUND_POWER_OF_TWO(temp[r], 7));
    dst_ref += temp[r] * filter[r];
  }
  dst_ref = clip_pixel(ROUND_POWER_OF_TWO(dst_ref, 7));
  EXPECT_EQ(dst[0], dst_ref);
}

TEST(VP10ConvolveTest, vpx_highbd_convolve12_avg_c2) {
  uint16_t src0[12*12];
  uint16_t src1[12*12];
  ptrdiff_t filter_size = 12;
  int filter_center = filter_size/2 - 1;
  uint16_t dst0[1] = {0};
  uint16_t dst1[1] = {0};
  uint16_t dst[1] = {0};
  const int16_t* filter = vp10_filter_kernels[0][3];
  int step = 16;
  int w = 1;
  int h = 1;
  int bd = 10;

  for(int i = 0; i < filter_size * filter_size; i++) {
    src0[i] = rand() % (1<<8);
    src1[i] = rand() % (1<<8);
  }

  vpx_highbd_convolve12_c(CONVERT_TO_BYTEPTR(src0 + filter_size * filter_center + filter_center) , filter_size,
                   CONVERT_TO_BYTEPTR(dst0), 1,
                   filter, step,
                   filter, step,
                   w, h, bd);

  vpx_highbd_convolve12_c(CONVERT_TO_BYTEPTR(src1 + filter_size * filter_center + filter_center) , filter_size,
                   CONVERT_TO_BYTEPTR(dst1), 1,
                   filter, step,
                   filter, step,
                   w, h, bd);

  vpx_highbd_convolve12_c(CONVERT_TO_BYTEPTR(src0 + filter_size * filter_center + filter_center) , filter_size,
                   CONVERT_TO_BYTEPTR(dst), 1,
                   filter, step,
                   filter, step,
                   w, h, bd);

  vpx_highbd_convolve12_avg_c(CONVERT_TO_BYTEPTR(src1 + filter_size * filter_center + filter_center) , filter_size,
                       CONVERT_TO_BYTEPTR(dst), 1,
                       filter, step,
                       filter, step,
                       w, h, bd);

  EXPECT_EQ(dst[0], ROUND_POWER_OF_TWO(dst0[0] + dst1[0], 1));

}
#endif
#endif

