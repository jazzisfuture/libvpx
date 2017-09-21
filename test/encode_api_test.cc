/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "../tools_common.h"

#include "./vpx_config.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "test/y4m_video_source.h"

#include "vpx/vp8cx.h"
#include "vpx/vpx_encoder.h"
#include "vpx_mem/vpx_mem.h"

namespace {

#define NELEMENTS(x) static_cast<int>(sizeof(x) / sizeof(x[0]))

static const VpxInterface vpx_encoders[] = {
#if CONFIG_VP8_ENCODER
  { "vp8", VP8_FOURCC, &vpx_codec_vp8_cx },
#endif

#if CONFIG_VP9_ENCODER
  { "vp9", VP9_FOURCC, &vpx_codec_vp9_cx },
#endif
};

TEST(EncodeAPI, InvalidParams) {
  static const vpx_codec_iface_t *kCodecs[] = {
#if CONFIG_VP8_ENCODER
    &vpx_codec_vp8_cx_algo,
#endif
#if CONFIG_VP9_ENCODER
    &vpx_codec_vp9_cx_algo,
#endif
  };
  uint8_t buf[1] = { 0 };
  vpx_image_t img;
  vpx_codec_ctx_t enc;
  vpx_codec_enc_cfg_t cfg;

  EXPECT_EQ(&img, vpx_img_wrap(&img, VPX_IMG_FMT_I420, 1, 1, 1, buf));

  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, vpx_codec_enc_init(NULL, NULL, NULL, 0));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, vpx_codec_enc_init(&enc, NULL, NULL, 0));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, vpx_codec_encode(NULL, NULL, 0, 0, 0, 0));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, vpx_codec_encode(NULL, &img, 0, 0, 0, 0));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, vpx_codec_destroy(NULL));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM,
            vpx_codec_enc_config_default(NULL, NULL, 0));
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM,
            vpx_codec_enc_config_default(NULL, &cfg, 0));
  EXPECT_TRUE(vpx_codec_error(NULL) != NULL);

  for (int i = 0; i < NELEMENTS(kCodecs); ++i) {
    SCOPED_TRACE(vpx_codec_iface_name(kCodecs[i]));
    EXPECT_EQ(VPX_CODEC_INVALID_PARAM,
              vpx_codec_enc_init(NULL, kCodecs[i], NULL, 0));
    EXPECT_EQ(VPX_CODEC_INVALID_PARAM,
              vpx_codec_enc_init(&enc, kCodecs[i], NULL, 0));
    EXPECT_EQ(VPX_CODEC_INVALID_PARAM,
              vpx_codec_enc_config_default(kCodecs[i], &cfg, 1));

    EXPECT_EQ(VPX_CODEC_OK, vpx_codec_enc_config_default(kCodecs[i], &cfg, 0));
    EXPECT_EQ(VPX_CODEC_OK, vpx_codec_enc_init(&enc, kCodecs[i], &cfg, 0));
    EXPECT_EQ(VPX_CODEC_OK, vpx_codec_encode(&enc, NULL, 0, 0, 0, 0));

    EXPECT_EQ(VPX_CODEC_OK, vpx_codec_destroy(&enc));
  }
}

TEST(EncodeAPI, HighBitDepthCapability) {
// VP8 should not claim VP9 HBD as a capability.
#if CONFIG_VP8_ENCODER
  const vpx_codec_caps_t vp8_caps = vpx_codec_get_caps(&vpx_codec_vp8_cx_algo);
  EXPECT_EQ(vp8_caps & VPX_CODEC_CAP_HIGHBITDEPTH, 0);
#endif

#if CONFIG_VP9_ENCODER
  const vpx_codec_caps_t vp9_caps = vpx_codec_get_caps(&vpx_codec_vp9_cx_algo);
#if CONFIG_VP9_HIGHBITDEPTH
  EXPECT_EQ(vp9_caps & VPX_CODEC_CAP_HIGHBITDEPTH, VPX_CODEC_CAP_HIGHBITDEPTH);
#else
  EXPECT_EQ(vp9_caps & VPX_CODEC_CAP_HIGHBITDEPTH, 0);
#endif
#endif
}

TEST(EncodeAPI, DISABLED_ImageSizeSetting) {
#if CONFIG_VP8_ENCODER
  uint8_t *img_buf;
  int width = 711;
  int height = 360;
  int bps = 12;
  libvpx_test::I420VideoSource video("desktop_640_360_30.yuv", width, height,
                                     30, 1, 0, 5);
  vpx_image_t img = *(video.img());
  vpx_codec_ctx_t enc;
  vpx_codec_enc_cfg_t cfg;
  const VpxInterface *encoder = &vpx_encoders[0];
  img_buf = reinterpret_cast<uint8_t *>(
      vpx_calloc(width * height * bps / 8, sizeof(*img_buf)));
  vpx_codec_enc_config_default(encoder->codec_interface(), &cfg, 0);

  cfg.g_w = width;
  cfg.g_h = height;

  vpx_img_wrap(&img, VPX_IMG_FMT_I420, width, height, 1, img_buf);

  vpx_codec_enc_init(&enc, encoder->codec_interface(), &cfg, 0);

  vpx_codec_encode(&enc, &img, 0, 1, 0, 0);

  vpx_free(img_buf);

  vpx_codec_destroy(&enc);
#endif
}

}  // namespace
