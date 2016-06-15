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
#include "test/encode_test_driver.h"
#include "test/util.h"
#include "test/video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {

const int kVideoSourceWidth = 320;
const int kVideoSourceHeight = 240;
const int kFramesToEncode = 2;

class RealTimeTest
    : public ::libvpx_test::EncoderTest,
      public ::libvpx_test::CodecTestWithParam<libvpx_test::TestMode> {
protected:
  RealTimeTest()
      : EncoderTest(GET_PARAM(0)), expected_res_(VPX_CODEC_OK),
        expected_num_decoded_(kFramesToEncode), num_decoded_(0),
        frame_packets_(0) {}
  virtual ~RealTimeTest() {}

  virtual void SetUp() {
    num_decoded_ = 0;
    frame_packets_ = 0;
    InitializeConfig();
    cfg_.g_lag_in_frames = 0;
    SetMode(::libvpx_test::kRealTime);
  }

  virtual void BeginPassHook(unsigned int /*pass*/) {
    // TODO(tomfinegan): We're changing the pass value here to make sure
    // we get frames when real time mode is combined with |g_pass| set to
    // VPX_RC_FIRST_PASS. This is necessary because EncoderTest::RunLoop() sets
    // the pass value based on the mode passed into EncoderTest::SetMode(),
    // which overrides the one specified in SetUp() above.
    cfg_.g_pass = VPX_RC_FIRST_PASS;
  }
  virtual void FramePktHook(const vpx_codec_cx_pkt_t * /*pkt*/) {
    frame_packets_++;
  }
  virtual void DecompressedFrameHook(const vpx_image_t & /*img*/,
                                     vpx_codec_pts_t /*pts*/) {
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
};

TEST_P(RealTimeTest, RealTimeFirstPassProducesFrames) {
  SetUp();
  ::libvpx_test::RandomVideoSource video;
  video.SetSize(kVideoSourceWidth, kVideoSourceHeight);
  video.set_limit(kFramesToEncode);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  EXPECT_EQ(kFramesToEncode, num_decoded_);
  EXPECT_EQ(kFramesToEncode, frame_packets_);
}

VP8_INSTANTIATE_TEST_CASE(RealTimeTest,
                          ::testing::Values(::libvpx_test::kRealTime));
VP9_INSTANTIATE_TEST_CASE(RealTimeTest,
                          ::testing::Values(::libvpx_test::kRealTime));

} // namespace
