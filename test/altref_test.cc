/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"

namespace {

// lookahead range: [kLookAheadMin, kLookAheadMax).
const int kLookAheadMin = 1;
const int kLookAheadMax = 26;

class AltRefTest : public libvpx_test::EncoderTest,
    public ::testing::TestWithParam<int> {
 protected:
  AltRefTest() : last_pts_(0) {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(libvpx_test::kTwoPassGood);
  }

  virtual void BeginPassHook(unsigned int pass) {
    last_pts_ = 0;
  }

  virtual bool Continue() const {
    return !HasFatalFailure() && !abort_;
  }

  virtual void PreEncodeFrameHook(libvpx_test::VideoSource *video,
                                  libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(VP8E_SET_ENABLEAUTOALTREF, 1);
      encoder->Control(VP8E_SET_CPUUSED, 3);
    }
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    ASSERT_GE(pkt->data.frame.pts, last_pts_);
    last_pts_ = pkt->data.frame.pts;
  }

 private:
  vpx_codec_pts_t last_pts_;
};

TEST_P(AltRefTest, MonotonicTimestamps) {
  const vpx_rational timebase = { 33333, 1000000 };
  cfg_.g_timebase = timebase;
  cfg_.rc_target_bitrate = 1000;
  cfg_.g_lag_in_frames = GetParam();

  libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                     timebase.den, timebase.num, 0, 30);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

INSTANTIATE_TEST_CASE_P(NonZeroLag, AltRefTest,
                        ::testing::Range(kLookAheadMin, kLookAheadMax));
}  // namespace
