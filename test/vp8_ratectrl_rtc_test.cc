/*
 *  Copyright (c) 2021 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "vp8/vp8_ratectrl_rtc.h"

#include <fstream>  // NOLINT
#include <string>

#include "./vpx_config.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "test/video_source.h"
#include "vpx/vpx_codec.h"
#include "vpx_ports/bitops.h"

namespace {

const int kNumFrames = 300;

class Vp8RcInterfaceTest
    : public ::libvpx_test::EncoderTest,
      public ::testing::TestWithParam<const libvpx_test::CodecFactory *> {
 public:
  Vp8RcInterfaceTest()
      : EncoderTest(GetParam()), key_interval_(3000), encoder_exit_(false){};
  virtual ~Vp8RcInterfaceTest(){};

 protected:
  virtual void SetUp() {
    InitializeConfig();
    SetMode(::libvpx_test::kRealTime);
  }

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    if (video->frame() == 0) {
      encoder->Control(VP8E_SET_CPUUSED, -8);
      encoder->Control(VP8E_SET_RTC_EXTERNAL_RATECTRL, 1);
    }
    frame_params_.frame_type =
        video->frame() % key_interval_ == 0 ? KEY_FRAME : INTER_FRAME;
    if (frame_params_.frame_type == INTER_FRAME) {
      // Disable golden frame update.
      frame_flags_ |= VP8_EFLAG_NO_UPD_GF;
      frame_flags_ |= VP8_EFLAG_NO_UPD_ARF;
    }
    encoder_exit_ = video->frame() == kNumFrames;
  }

  virtual void PostEncodeFrameHook(::libvpx_test::Encoder *encoder) {
    if (encoder_exit_) {
      return;
    }
    int qp;
    encoder->Control(VP8E_GET_LAST_QUANTIZER, &qp);
    rc_api_->ComputeQP(frame_params_);
    ASSERT_EQ(rc_api_->GetQP(), qp);
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    rc_api_->PostEncodeUpdate(pkt->data.frame.sz);
  }

  void RunOneLayer() {
    SetConfig();
    rc_api_ = libvpx::VP8RateControlRTC::Create(rc_cfg_);

    ::libvpx_test::I420VideoSource video("desktop_office1.1280_720-020.yuv",
                                         1280, 720, 30, 1, 0, kNumFrames);

    ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  }

 private:
  void SetConfig() {
    rc_cfg_.width = 1280;
    rc_cfg_.height = 720;
    rc_cfg_.max_quantizer = 52;
    rc_cfg_.min_quantizer = 2;
    rc_cfg_.target_bandwidth = 1000;
    rc_cfg_.buf_initial_sz = 600;
    rc_cfg_.buf_optimal_sz = 600;
    rc_cfg_.buf_sz = 1000;
    rc_cfg_.undershoot_pct = 50;
    rc_cfg_.overshoot_pct = 50;
    rc_cfg_.max_intra_bitrate_pct = 1000;
    rc_cfg_.framerate = 30.0;
    rc_cfg_.layer_target_bitrate[0] = 1000;

    // Encoder settings for ground truth.
    cfg_.g_w = 1280;
    cfg_.g_h = 720;
    cfg_.rc_undershoot_pct = 50;
    cfg_.rc_overshoot_pct = 50;
    cfg_.rc_buf_initial_sz = 600;
    cfg_.rc_buf_optimal_sz = 600;
    cfg_.rc_buf_sz = 1000;
    cfg_.rc_dropframe_thresh = 0;
    cfg_.rc_min_quantizer = 2;
    cfg_.rc_max_quantizer = 52;
    cfg_.rc_end_usage = VPX_CBR;
    cfg_.g_lag_in_frames = 0;
    cfg_.g_error_resilient = 1;
    cfg_.rc_target_bitrate = 1000;
    cfg_.kf_min_dist = key_interval_;
    cfg_.kf_max_dist = key_interval_;
  }

  std::unique_ptr<libvpx::VP8RateControlRTC> rc_api_;
  libvpx::VP8RateControlRtcConfig rc_cfg_;
  int key_interval_;
  libvpx::VP8FrameParamsQpRTC frame_params_;
  bool encoder_exit_;
};

TEST_P(Vp8RcInterfaceTest, OneLayer) { RunOneLayer(); }

INSTANTIATE_TEST_SUITE_P(
    VP8, Vp8RcInterfaceTest,
    ::testing::Values(
        static_cast<const libvpx_test::CodecFactory *>(&libvpx_test::kVP8)));

}  // namespace
