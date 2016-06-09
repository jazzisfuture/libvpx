/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "test/codec_factory.h"
#include "test/video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {

const int kVideoSourceWidth = 320;
const int kVideoSourceHeight = 240;
const int kFramesToEncode = 2;
const int kNumPassConfigs = 3;
const vpx_enc_pass kPassValues[kNumPassConfigs] = {
    VPX_RC_FIRST_PASS, VPX_RC_FIRST_PASS, VPX_RC_ONE_PASS};

#if CONFIG_VP8_ENCODER
class VP8RealTimeTest : public ::libvpx_test::EncoderTest,
                        public ::testing::Test {
protected:
  VP8RealTimeTest()
      : EncoderTest(&::libvpx_test::kVP8), expected_res_(VPX_CODEC_OK),
        expected_num_decoded_(kFramesToEncode), num_decoded_(0),
        frame_packets_(0), pass_value_index_(0) {}
  virtual ~VP8RealTimeTest() {}

  virtual void SetUp() {
    num_decoded_ = 0;
    frame_packets_ = 0;
    InitializeConfig();
    SetMode(::libvpx_test::kRealTime);
  }

  virtual void BeginPassHook(unsigned int /*pass*/) {
    // TODO(tomfinegan): We're changing the pass value here to make sure
    // we get frames when real time mode is combined with |g_pass| set to
    // VPX_RC_FIRST_PASS or VPX_RC_LAST_PASS. This is necessary because
    // EncoderTest::RunLoop() sets the pass value based on the mode passed
    // into EncoderTest::SetMode(), which overrides the one specified in
    // SetUp() above.
    cfg_.g_pass = kPassValues[pass_value_index_];
  }
  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) { frame_packets_++; }
  virtual void DecompressedFrameHook(const vpx_image_t &img,
                                     vpx_codec_pts_t pts) {
    num_decoded_++;
  }
  virtual bool HandleDecodeResult(const vpx_codec_err_t res_dec,
                                  const libvpx_test::VideoSource & /*video*/,
                                  libvpx_test::Decoder *decoder) {
    EXPECT_EQ(expected_res_, res_dec) << decoder->DecodeError();
    return !::testing::Test::HasFailure();
  }

  int expected_res_;
  int expected_num_decoded_;
  int num_decoded_;
  int frame_packets_;

  // Index into |kPassValues| used to set the actual |g_pass| value sent to
  // the encoder via BeginPassHook().
  size_t pass_value_index_;
};

TEST_F(VP8RealTimeTest, VP8RealTimeAlwaysProducesFrames) {
  for (; pass_value_index_ < kNumPassConfigs; ++pass_value_index_) {
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

} // namespace
