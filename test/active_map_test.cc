/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <climits>
#include <vector>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"

namespace {

class ActiveMapTest : public ::libvpx_test::EncoderTest,
    public ::libvpx_test::CodecTestWith2Params<
        libvpx_test::TestMode, int> {
 protected:
  ActiveMapTest() : EncoderTest(GET_PARAM(0)) {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(GET_PARAM(1));
    set_cpu_used_ = GET_PARAM(2);
  }

  static const int kWidth = 208;
  static const int kHeight = 144;

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(VP8E_SET_CPUUSED, set_cpu_used_);
    } else if (video->frame() == 3) {
      vpx_active_map_t map = {0};
      uint8_t active_map[9*13] = {
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1,
        0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0,
      };
      map.cols = (kWidth + 15) / 16;
      map.rows = (kHeight + 15) / 16;
      ASSERT_EQ(map.cols, 13u);
      ASSERT_EQ(map.rows, 9u);
      map.active_map = active_map;
      encoder->Control(VP8E_SET_ACTIVEMAP, &map);
    } else if (video->frame() == 15) {
      vpx_active_map_t map = {0};
      map.cols = (kWidth + 15) / 16;
      map.rows = (kHeight + 15) / 16;
      map.active_map = NULL;
      encoder->Control(VP8E_SET_ACTIVEMAP, &map);
    }
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    if (pkt->data.frame.flags & VPX_FRAME_IS_KEY) {
    }
  }
  int set_cpu_used_;
};

TEST_P(ActiveMapTest, Test) {
  // Validate that this non multiple of 64 wide clip encodes
  cfg_.g_lag_in_frames = 0;
  cfg_.rc_target_bitrate = 400;
  cfg_.rc_resize_allowed = 0;
  cfg_.g_pass = VPX_RC_ONE_PASS;
  cfg_.rc_end_usage = VPX_CBR;
  cfg_.kf_max_dist = 90000;

  ::libvpx_test::I420VideoSource video("hantro_odd.yuv", 208, 144, 30, 1, 0,
                                       20);

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}


using std::tr1::make_tuple;

#define VP9_FACTORY \
  static_cast<const libvpx_test::CodecFactory*> (&libvpx_test::kVP9)

VP9_INSTANTIATE_TEST_CASE(
    ActiveMapTest,
    ::testing::Values(::libvpx_test::kRealTime),
    ::testing::Range(0, 6));
}  // namespace
