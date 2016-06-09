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
#include "test/codec_factory.h"
#include "test/video_source.h"

namespace {

const int kVideoSourceWidth = 320;
const int kVideoSourceHeight = 240;
const int kFramesToEncode = 2;
const int kNumPassConfigs = 3;
const vpx_enc_pass kPassValues[kNumPassConfigs] =
    {VPX_RC_FIRST_PASS, VPX_RC_LAST_PASS, VPX_RC_ONE_PASS};

#if CONFIG_VP8_ENCODER
class VP8RealTimeTest
    : public ::libvpx_test::EncoderTest,
      public ::testing::Test {
 protected:
  VP8RealTimeTest() : EncoderTest(&::libvpx_test::kVP8),
                             expected_res_(VPX_CODEC_OK),
                             expected_num_decoded_(kFramesToEncode),
                             num_decoded_(0),
                             frame_packets_(0),
                             pass_(VPX_RC_ONE_PASS) {}
  virtual ~VP8RealTimeTest() {}

  virtual void SetUp() {
    num_decoded_ = 0;
    frame_packets_ = 0;
    InitializeConfig();
    SetMode(::libvpx_test::kRealTime);
    cfg_.g_pass = pass_;
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t* pkt) {
    frame_packets_++;
  }
  virtual void DecompressedFrameHook(const vpx_image_t& img,
                                     vpx_codec_pts_t pts) {
    num_decoded_++;
  }
  virtual bool HandleDecodeResult(const vpx_codec_err_t res_dec,
                                  const libvpx_test::VideoSource& /*video*/,
                                  libvpx_test::Decoder *decoder) {
    EXPECT_EQ(expected_res_, res_dec) << decoder->DecodeError();
    return !::testing::Test::HasFailure();
  }

  vpx_enc_pass pass() const { return pass_; }
  void set_pass(vpx_enc_pass pass_val) { pass_ = pass_val; }

  int expected_res_;
  int expected_num_decoded_;
  int num_decoded_;
  int frame_packets_;
  vpx_enc_pass pass_;
};

TEST_F(VP8RealTimeTest, VP8RealTimeAlwaysProducesFrames) {
  for (int i = 0; i < kNumPassConfigs; ++i) {
    set_pass(kPassValues[i]);
    SetUp();
    ::libvpx_test::RandomVideoSource video;
    video.SetSize(kVideoSourceWidth, kVideoSourceHeight);
    video.set_limit(kFramesToEncode);
    expected_res_ = VPX_CODEC_OK;
    ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
    EXPECT_EQ(kFramesToEncode, num_decoded_);
    EXPECT_EQ(kFramesToEncode, frame_packets_);
  }
}
#endif

}  // namespace
