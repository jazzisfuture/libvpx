#include "test/vp10_txfm_test.h"
#include "vp10/common/vp10_fwd_txfm2d_cfg.h"
#include "./vp10_rtcd.h"


namespace {
TEST(dct_sse2, vp10_fwd_txfm2d_4x4_c) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_4;
  int16_t input[1024] = { 7, 7, 7, 7,
                       -2,-2,-2,-2,
                        3, 3, 3, 3,
                        1, 1, 1, 1};
  int32_t output[1024] = {0};
  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_4x4_c(input, output, cfg.txfm_size, &cfg , 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_8x8_c) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_8;
  int16_t input[1024] = { 7, 7, 7, 7,
                       -2,-2,-2,-2,
                        3, 3, 3, 3,
                        1, 1, 1, 1};
  int32_t output[1024] = {0};
  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_8x8_c(input, output, cfg.txfm_size, &cfg , 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_16x16_c) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_16;
  int16_t input[1024] = { 7, 7, 7, 7,
                       -2,-2,-2,-2,
                        3, 3, 3, 3,
                        1, 1, 1, 1};
  int32_t output[1024] = {0};
  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_16x16_c(input, output, cfg.txfm_size, &cfg , 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_32x32_c) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_32;
  int16_t input[1024] = { 7, 7, 7, 7,
                       -2,-2,-2,-2,
                        3, 3, 3, 3,
                        1, 1, 1, 1};
  int32_t output[1024] = {0};
  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_32x32_c(input, output, cfg.txfm_size, &cfg , 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_4x4_sse4_1) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_4;
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_4x4_sse4_1(input_sse2, output_sse2, cfg.txfm_size, &cfg, 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_8x8_sse4_1) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_8;
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_8x8_sse4_1(input_sse2, output_sse2, cfg.txfm_size, &cfg, 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_16x16_sse4_1) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_16;
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_16x16_sse4_1(input_sse2, output_sse2, cfg.txfm_size, &cfg, 10);
  }
}

TEST(dct_sse2, vp10_fwd_txfm2d_32x32_sse4_1) {
  TXFM_2D_CFG cfg = fwd_txfm_2d_cfg_dct_dct_32;
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_fwd_txfm2d_32x32_sse4_1(input_sse2, output_sse2, cfg.txfm_size, &cfg, 10);
  }
}

TEST(dct_sse2, vp10_highbd_fdct4x4_c) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct4x4_c(input_sse2, output_sse2, 4);
  }
}

TEST(dct_sse2, vp10_highbd_fdct8x8_c) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct8x8_c(input_sse2, output_sse2, 8);
  }
}

TEST(dct_sse2, vp10_highbd_fdct16x16_c) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct16x16_c(input_sse2, output_sse2, 16);
  }
}

TEST(dct_sse2, highbd_fdct32x32_c) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct32x32_c(input_sse2, output_sse2, 32);
  }
}

TEST(dct_sse2, vp10_highbd_fdct4x4_sse2) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct4x4_sse2(input_sse2, output_sse2, 4);
  }
}

TEST(dct_sse2, vp10_highbd_fdct8x8_sse2) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct8x8_sse2(input_sse2, output_sse2, 8);
  }
}

TEST(dct_sse2, vp10_highbd_fdct16x16_sse2) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct16x16_sse2(input_sse2, output_sse2, 16);
  }
}

TEST(dct_sse2, highbd_fdct32x32_sse2) {
  int16_t input_sse2[1024] = { 7, 7, 7, 7,
                            -2,-2,-2,-2,
                             3, 3, 3, 3,
                             1, 1, 1, 1};
  int32_t output_sse2[1024] = {0};

  for(int i = 0; i < 20000; i++) {
    vp10_highbd_fdct32x32_sse2(input_sse2, output_sse2, 32);
  }
}
}
