#include <string>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/decode_test_driver.h"
#include "test/i420_video_source.h"
#include "vpx/vp8cx.h"
#include "vpx/vpx_encoder.h"
#include "vpx/svc_context.h"

namespace {

using libvpx_test::CodecFactory;
using libvpx_test::Decoder;
using libvpx_test::VP9CodecFactory;

class SvcTest : public ::testing::Test {
 protected:
  enum {
    kWidth = 352,
    kHeight = 288,
  };

  SvcTest()
      : codec_iface_(0),
        test_file_name_("hantro_collage_w352h288.yuv"),
        decoder_(0) {}

  virtual ~SvcTest() {}

  virtual void SetUp() {
    memset(&svc_, 0, sizeof(svc_));
    svc_.first_frame_full_size = 1;
    svc_.encoding_mode = INTER_LAYER_PREDICTION_IP;
    svc_.log_level = SVC_LOG_DEBUG;
    svc_.log_print = 0;
    svc_.gop_size = 100;

    codec_iface_ = vpx_codec_vp9_cx();
    const vpx_codec_err_t res =
        vpx_codec_enc_config_default(codec_iface_, &codec_enc_, 0);
    EXPECT_EQ(VPX_CODEC_OK, res);

    codec_enc_.g_w = kWidth;
    codec_enc_.g_h = kHeight;
    codec_enc_.g_timebase.num = 1;
    codec_enc_.g_timebase.den = 60;

    vpx_codec_dec_cfg_t dec_cfg = {0};
    VP9CodecFactory codec_factory;
    decoder_ = codec_factory.CreateDecoder(dec_cfg, 0);
  }

  SvcContext svc_;
  vpx_codec_ctx_t codec_;
  struct vpx_codec_enc_cfg codec_enc_;
  vpx_codec_iface_t *codec_iface_;
  std::string test_file_name_;

  Decoder *decoder_;
};

TEST_F(SvcTest, SvcInit) {
  svc_.spatial_layers = 0;  // not enough layers
  vpx_codec_err_t res = vpx_svc_init(&svc_, &codec_, codec_iface_, &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.spatial_layers = 6;  // too many layers
  res = vpx_svc_init(&svc_, &codec_, codec_iface_, &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.spatial_layers = 2;
  svc_.scale_factors = "4/16,16*16";  // invalid scale values
  res = vpx_svc_init(&svc_, &codec_, codec_iface_, &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.scale_factors = "4/16,16/16";  // valid scale values
  res = vpx_svc_init(&svc_, &codec_, codec_iface_, &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);
}

TEST_F(SvcTest, CheckOptions) {
  svc_.options = "layers=3";
  vpx_codec_err_t res =
      vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(3, svc_.spatial_layers);

  svc_.options = "not-an-option=1";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.options = "encoding-mode=alt-ip";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(ALT_INTER_LAYER_PREDICTION_IP, svc_.encoding_mode);

  svc_.options = "layers=2 encoding-mode=ip";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(2, svc_.spatial_layers);
  EXPECT_EQ(INTER_LAYER_PREDICTION_IP, svc_.encoding_mode);

  svc_.options = "scale-factors=not-scale-factors";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.options = "scale-factors=1/3,2/3";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);

  svc_.options = "quantizers=not-quantizers";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  svc_.options = "quantizers=40,45";
  res = vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);
}

// test that decoder can handle an SVC frame as the first frame in a sequence
// this test is disabled since it always fails because of a decoder issue
TEST_F(SvcTest, DISABLED_FirstFrameHasLayers) {
  svc_.first_frame_full_size = 0;
  svc_.spatial_layers = 2;
  svc_.scale_factors = "4/16,16/16";
  svc_.quantizer_values = "40,30";

  vpx_codec_err_t res =
      vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);

  libvpx_test::I420VideoSource video(test_file_name_, kWidth, kHeight,
                                     codec_enc_.g_timebase.den,
                                     codec_enc_.g_timebase.num, 0, 30);
  video.Begin();

  res = vpx_svc_encode(&svc_, &codec_, video.img(), video.pts(),
                       video.duration(), VPX_DL_REALTIME);
  EXPECT_EQ(VPX_CODEC_OK, res);

  vpx_codec_err_t res_dec = decoder_->DecodeFrame(
      static_cast<const uint8_t *>(vpx_svc_get_buffer(&svc_)),
      vpx_svc_get_frame_size(&svc_));

  // this test fails with a decoder error
  ASSERT_EQ(VPX_CODEC_OK, res_dec) << decoder_->DecodeError();
}

TEST_F(SvcTest, EncodeThreeFrames) {
  svc_.first_frame_full_size = 1;
  svc_.spatial_layers = 2;
  svc_.scale_factors = "4/16,16/16";
  svc_.quantizer_values = "40,30";

  vpx_codec_err_t res =
      vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  ASSERT_EQ(VPX_CODEC_OK, res);

  libvpx_test::I420VideoSource video(test_file_name_, kWidth, kHeight,
                                     codec_enc_.g_timebase.den,
                                     codec_enc_.g_timebase.num, 0, 30);
  // FRAME 1
  video.Begin();
  // this frame is full size, with only one layer
  res = vpx_svc_encode(&svc_, &codec_, video.img(), video.pts(),
                       video.duration(), VPX_DL_REALTIME);
  ASSERT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(1, vpx_svc_is_keyframe(&svc_));

  vpx_codec_err_t res_dec = decoder_->DecodeFrame(
      static_cast<const uint8_t *>(vpx_svc_get_buffer(&svc_)),
      vpx_svc_get_frame_size(&svc_));
  ASSERT_EQ(VPX_CODEC_OK, res_dec) << decoder_->DecodeError();

  // FRAME 2
  video.Next();
  // this is an I-frame
  res = vpx_svc_encode(&svc_, &codec_, video.img(), video.pts(),
                       video.duration(), VPX_DL_REALTIME);
  ASSERT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(1, vpx_svc_is_keyframe(&svc_));

  res_dec = decoder_->DecodeFrame(
      static_cast<const uint8_t *>(vpx_svc_get_buffer(&svc_)),
      vpx_svc_get_frame_size(&svc_));
  ASSERT_EQ(VPX_CODEC_OK, res_dec) << decoder_->DecodeError();

  // FRAME 2
  video.Next();
  // this is a P-frame
  res = vpx_svc_encode(&svc_, &codec_, video.img(), video.pts(),
                       video.duration(), VPX_DL_REALTIME);
  ASSERT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(0, vpx_svc_is_keyframe(&svc_));

  res_dec = decoder_->DecodeFrame(
      static_cast<const uint8_t *>(vpx_svc_get_buffer(&svc_)),
      vpx_svc_get_frame_size(&svc_));
  ASSERT_EQ(VPX_CODEC_OK, res_dec) << decoder_->DecodeError();
}

TEST_F(SvcTest, GetLayerResolution) {
  svc_.first_frame_full_size = 0;
  svc_.spatial_layers = 2;
  svc_.scale_factors = "4/16,8/16";
  svc_.quantizer_values = "40,30";

  vpx_codec_err_t res =
      vpx_svc_init(&svc_, &codec_, vpx_codec_vp9_cx(), &codec_enc_);
  EXPECT_EQ(VPX_CODEC_OK, res);

  // ensure that requested layer is a valid layer
  uint32_t layer_width, layer_height;
  res = vpx_svc_get_layer_resolution(&svc_, svc_.spatial_layers,  //
                                     &layer_width, &layer_height);
  EXPECT_EQ(VPX_CODEC_INVALID_PARAM, res);

  res = vpx_svc_get_layer_resolution(&svc_, 0, &layer_width, &layer_height);
  EXPECT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(static_cast<uint32_t>(kWidth * 4 / 16), layer_width);
  EXPECT_EQ(static_cast<uint32_t>(kHeight * 4 / 16), layer_height);

  res = vpx_svc_get_layer_resolution(&svc_, 1, &layer_width, &layer_height);
  EXPECT_EQ(VPX_CODEC_OK, res);
  EXPECT_EQ(static_cast<uint32_t>(kWidth * 8 / 16), layer_width);
  EXPECT_EQ(static_cast<uint32_t>(kHeight * 8 / 16), layer_height);
}

}  // namespace
