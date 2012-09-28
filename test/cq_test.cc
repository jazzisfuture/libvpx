/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <cmath>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"

// CQ level range: [kCQLevelMin, kCQLevelMax).
const int kCQLevelMin = 4;
const int kCQLevelMax = 63;
const int kCQLevelStep = 8;

namespace {

class CQTest : public libvpx_test::EncoderTest,
    public ::testing::TestWithParam<int> {
 protected:
  CQTest() { cq_level_ = GetParam(); }
  virtual ~CQTest() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(libvpx_test::kTwoPassGood);
  }

  virtual void BeginPassHook(unsigned int pass) {
    sz_ = 0;
    psnr_ = 0.0;
    nframes_ = 0;
  }

  virtual bool Continue() const {
    return !HasFatalFailure() && !abort_;
  }

  virtual void PreEncodeFrameHook(libvpx_test::VideoSource *video,
                                  libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      if (cfg_.rc_end_usage == VPX_CQ) {
        encoder->Control(VP8E_SET_CQ_LEVEL, cq_level_);
      }
      encoder->Control(VP8E_SET_CPUUSED, 3);
    }
  }

  virtual void PSNRPktHook(const vpx_codec_cx_pkt_t *pkt) {
    psnr_ += pow(10.0, pkt->data.psnr.psnr[0] / 10.0);
    nframes_++;
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    sz_ += pkt->data.frame.sz;
  }

  double getLinearPSNROverBitrate() {
    double avg_psnr = log10(psnr_ / nframes_) * 10.0;
    return pow(10.0, avg_psnr / 10.0) / sz_;
  }

  int getFilesize() { return sz_; }
  int getNFrames() { return nframes_; }

 private:
  int cq_level_;
  int sz_;
  int psnr_;
  double nframes_;
};

TEST_P(CQTest, MonotonicTimestamps) {
  const vpx_rational timebase = { 33333333, 1000000000 };
  cfg_.g_timebase = timebase;
  cfg_.rc_target_bitrate = 2000;
  cfg_.g_lag_in_frames = 25;

  cfg_.rc_end_usage = VPX_CQ;
  libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                     timebase.den, timebase.num, 0, 30);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  double cq_psnr_lin = getLinearPSNROverBitrate();

  cfg_.rc_end_usage = VPX_VBR;
  // try targeting the approximate same bitrate with VBR mode
  cfg_.rc_target_bitrate = getFilesize() * 8 * 30 / (getNFrames() * 1000);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  double vbr_psnr_lin = getLinearPSNROverBitrate();
  EXPECT_GE(cq_psnr_lin, vbr_psnr_lin);
  EXPECT_NE(cq_psnr_lin, vbr_psnr_lin);
}

INSTANTIATE_TEST_CASE_P(LinearPSNRisHigher, CQTest,
                        ::testing::Range(kCQLevelMin, kCQLevelMax,
                                         kCQLevelStep));
}  // namespace
