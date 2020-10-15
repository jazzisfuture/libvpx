/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "vpx/vpx_ext_ratectrl.h"

#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
// #include "test/encode_test_driver.h"
#include "test/util.h"
#include "test/yuv_video_source.h"

namespace {

int rc_send_firstpass_stats(vpx_rc_model_t rate_ctrl_model,
                            const vpx_rc_firstpass_stats_t *first_pass_stats) {
  (void)rate_ctrl_model;
  (void)first_pass_stats;
  return 0;
}

int rc_get_encodeframe_decision(
    vpx_rc_model_t rate_ctrl_model,
    const vpx_rc_encodeframe_info_t *encode_frame_info,
    vpx_rc_encodeframe_decision_t *frame_decision) {
  (void)rate_ctrl_model;
  (void)encode_frame_info;
  (void)frame_decision;
  return 0;
}

int rc_update_encodeframe_result(
    vpx_rc_model_t rate_ctrl_model,
    const vpx_rc_encodeframe_result_t *encode_frame_result) {
  (void)rate_ctrl_model;
  (void)encode_frame_result;
  return 0;
}

int rc_delete_model(vpx_rc_model_t rate_ctrl_model) {
  (void)rate_ctrl_model;
  return 0;
}

class ExtRateCtrlTest : public ::libvpx_test::EncoderTest,
                        public ::testing::Test {
 protected:
  ExtRateCtrlTest() : EncoderTest(&::libvpx_test::kVP9) {}

  ~ExtRateCtrlTest() override {}

  void SetUp() {
    InitializeConfig();
    SetMode(::libvpx_test::kTwoPassGood);
  }

  void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                          ::libvpx_test::Encoder *encoder) override {
    if (video->frame() == 0) {
      vpx_rc_funcs_t rc_funcs;
      rc_funcs.create_model = rc_create_model;
      rc_funcs.send_firstpass_stats = rc_send_firstpass_stats;
      rc_funcs.get_encodeframe_decision = rc_get_encodeframe_decision;
      rc_funcs.update_encodeframe_result = rc_update_encodeframe_result;
      rc_funcs.delete_model = rc_delete_model;

      encoder->Control(VP9E_SET_EXTERNAL_RATE_CONTROL, &rc_funcs);
    }
  }
};

TEST_F(ExtRateCtrlTest, XYZ) {
  cfg_.rc_target_bitrate = 24000;

  std::unique_ptr<libvpx_test::VideoSource> video;
  video.reset(new libvpx_test::YUVVideoSource(
      "bus_352x288_420_f20_b8.yuv", VPX_IMG_FMT_I420, 352, 288, 30, 1, 0, 5));

  ASSERT_NE(video.get(), nullptr);
  ASSERT_NO_FATAL_FAILURE(RunLoop(video.get()));
}

// VP9_INSTANTIATE_TEST_SUITE(ExtRateCtrlTest);
}  // namespace
