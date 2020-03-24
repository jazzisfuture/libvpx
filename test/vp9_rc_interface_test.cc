/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <iostream>  // NOLINT
#include <fstream>  // NOLINT

#include "./vpx_config.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "test/y4m_video_source.h"
#include "vp9/ratectrl_rtc.h"
#include "vpx/vpx_codec.h"
#include "vpx_ports/bitops.h"

namespace {

const size_t kNumFrame = 850;

struct FrameInfo {
  friend std::istream &operator>>(std::istream &is, FrameInfo &info) {
    is >> info.frame_id_ >> info.spatial_id_ >> info.temporal_id_ >>
        info.base_q_ >> info.target_bandwidth_ >> info.buffer_level_ >>
        info.filter_level_ >> info.bytes_used_;
    return is;
  }
  size_t frame_id_;
  int spatial_id_;
  int temporal_id_;
  // Base QP
  int base_q_;
  size_t target_bandwidth_;
  size_t buffer_level_;
  // Loopfilter level
  int filter_level_;
  // Frame size for current frame, used for pose encode update
  size_t bytes_used_;
};

class RcInterfaceTest {
 public:
  explicit RcInterfaceTest() {}

  virtual ~RcInterfaceTest() {}

 protected:
  void SetConfigOneLayer() {
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
    rc_cfg_.ss_number_layers = 1;
    rc_cfg_.ts_number_layers = 1;
    rc_cfg_.scaling_factor_num[0] = 1;
    rc_cfg_.scaling_factor_den[0] = 1;
    rc_cfg_.layer_target_bitrate[0] = 1000;
    rc_cfg_.max_quantizers[0] = 52;
    rc_cfg_.min_quantizers[0] = 2;
  }

  void SetConfigSVC() {
    rc_cfg_.width = 1280;
    rc_cfg_.height = 720;
    rc_cfg_.max_quantizer = 56;
    rc_cfg_.min_quantizer = 2;
    rc_cfg_.target_bandwidth = 1600;
    rc_cfg_.buf_initial_sz = 500;
    rc_cfg_.buf_optimal_sz = 600;
    rc_cfg_.buf_sz = 1000;
    rc_cfg_.undershoot_pct = 50;
    rc_cfg_.overshoot_pct = 50;
    rc_cfg_.max_intra_bitrate_pct = 900;
    rc_cfg_.framerate = 30.0;
    rc_cfg_.ss_number_layers = 3;
    rc_cfg_.ts_number_layers = 3;

    rc_cfg_.scaling_factor_num[0] = 1;
    rc_cfg_.scaling_factor_den[0] = 4;
    rc_cfg_.scaling_factor_num[1] = 2;
    rc_cfg_.scaling_factor_den[1] = 4;
    rc_cfg_.scaling_factor_num[2] = 4;
    rc_cfg_.scaling_factor_den[2] = 4;

    rc_cfg_.ts_rate_decimator[0] = 4;
    rc_cfg_.ts_rate_decimator[1] = 2;
    rc_cfg_.ts_rate_decimator[2] = 1;

    rc_cfg_.layer_target_bitrate[0] = 100;
    rc_cfg_.layer_target_bitrate[1] = 140;
    rc_cfg_.layer_target_bitrate[2] = 200;
    rc_cfg_.layer_target_bitrate[3] = 250;
    rc_cfg_.layer_target_bitrate[4] = 350;
    rc_cfg_.layer_target_bitrate[5] = 500;
    rc_cfg_.layer_target_bitrate[6] = 450;
    rc_cfg_.layer_target_bitrate[7] = 630;
    rc_cfg_.layer_target_bitrate[8] = 900;

    for (int sl = 0; sl < rc_cfg_.ss_number_layers; ++sl) {
      for (int tl = 0; tl < rc_cfg_.ts_number_layers; ++tl) {
        const int i = sl * rc_cfg_.ts_number_layers + tl;
        rc_cfg_.max_quantizers[i] = 56;
        rc_cfg_.min_quantizers[i] = 2;
      }
    }
  }

 public:
  void RunOneLayer() {
    SetConfigOneLayer();
    rc_api_.InitRateControl(rc_cfg_);
    FrameInfo frame_info;
    vp9::FrameParamsQpRTC frame_params;
    frame_params.frame_type = KEY_FRAME;
    frame_params.spatial_layer_id = 0;
    frame_params.temporal_layer_id = 0;
    std::ifstream one_layer_file;
    one_layer_file.open(std::string(std::getenv("LIBVPX_TEST_DATA_PATH")) +
                        "/rc_interface_test_one_layer");
    for (size_t i = 0; i < kNumFrame; i++) {
      one_layer_file >> frame_info;
      if (frame_info.frame_id_ > 0) frame_params.frame_type = INTER_FRAME;
      if (frame_info.frame_id_ == 200) {
        rc_cfg_.target_bandwidth = rc_cfg_.target_bandwidth * 2;
        rc_api_.UpdateRateControl(rc_cfg_);
      } else if (frame_info.frame_id_ == 400) {
        rc_cfg_.target_bandwidth = rc_cfg_.target_bandwidth / 4;
        rc_api_.UpdateRateControl(rc_cfg_);
      }
      ASSERT_EQ(frame_info.spatial_id_, 0);
      ASSERT_EQ(frame_info.temporal_id_, 0);
      rc_api_.ComputeQP(frame_params);
      ASSERT_EQ(rc_api_.GetQP(), frame_info.base_q_);
      ASSERT_EQ(rc_api_.GetLoopfilterLevel(), frame_info.filter_level_);
      rc_api_.PostEncode(frame_info.bytes_used_);
    }
  }

  void RunSVC() {
    SetConfigSVC();
    rc_api_.InitRateControl(rc_cfg_);
    FrameInfo frame_info;
    vp9::FrameParamsQpRTC frame_params;
    frame_params.frame_type = KEY_FRAME;
    std::ifstream svc_file;
    svc_file.open(std::string(std::getenv("LIBVPX_TEST_DATA_PATH")) +
                  "/rc_interface_test_svc");
    for (size_t i = 0; i < kNumFrame * 3; i++) {
      svc_file >> frame_info;
      if (frame_info.frame_id_ > 0) frame_params.frame_type = INTER_FRAME;
      if (frame_info.frame_id_ == 600) {
        for (int layer = 0;
             layer < rc_cfg_.ss_number_layers * rc_cfg_.ts_number_layers;
             layer++)
          rc_cfg_.layer_target_bitrate[layer] *= 2;
        rc_cfg_.target_bandwidth *= 2;
        rc_api_.UpdateRateControl(rc_cfg_);
      } else if (frame_info.frame_id_ == 1200) {
        for (int layer = 0;
             layer < rc_cfg_.ss_number_layers * rc_cfg_.ts_number_layers;
             layer++)
          rc_cfg_.layer_target_bitrate[layer] /= 4;
        rc_cfg_.target_bandwidth /= 4;
        rc_api_.UpdateRateControl(rc_cfg_);
      }
      frame_params.spatial_layer_id = frame_info.spatial_id_;
      frame_params.temporal_layer_id = frame_info.temporal_id_;
      rc_api_.ComputeQP(frame_params);
      ASSERT_EQ(rc_api_.GetQP(), frame_info.base_q_);
      ASSERT_EQ(rc_api_.GetLoopfilterLevel(), frame_info.filter_level_);
      rc_api_.PostEncode(frame_info.bytes_used_);
    }
  }

 protected:
  vp9::VP9RateControlRTC rc_api_;
  vp9::RateControlRtcConfig rc_cfg_;
};
}  // namespace

int main(void) {
  RcInterfaceTest rc_interface_test_1layer, rc_interface_test_svc;
  rc_interface_test_1layer.RunOneLayer();
  rc_interface_test_svc.RunSVC();
  return 0;
}
