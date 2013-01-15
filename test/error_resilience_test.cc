/*
  Copyright (c) 2012 The WebM project authors. All Rights Reserved.

  Use of this source code is governed by a BSD-style license
  that can be found in the LICENSE file in the root of the source
  tree. An additional intellectual property rights grant can be found
  in the file PATENTS.  All contributing project authors may
  be found in the AUTHORS file in the root of the source tree.
*/

#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {

const int kMaxErrorFrames = 8;


class ErrorResilienceTest : public libvpx_test::EncoderTest,
    public ::testing::TestWithParam<int> {
 protected:
  ErrorResilienceTest() {
    psnr_ = 0.0;
    nframes_ = 0;
    error_nframes_ = 0;
    mismatch_psnr_ = 0.0;
    mismatch_nframes_ = 0;
    encoding_mode_ = static_cast<libvpx_test::TestMode>(GetParam());
  }
  virtual ~ErrorResilienceTest() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(encoding_mode_);
  }

  virtual void BeginPassHook(unsigned int /*pass*/) {
    psnr_ = 0.0;
    nframes_ = 0;
    mismatch_psnr_ = 0.0;
    mismatch_nframes_ = 0;
  }

  virtual bool Continue() const {
    return !HasFatalFailure() && !abort_;
  }

  virtual void PSNRPktHook(const vpx_codec_cx_pkt_t *pkt) {
    psnr_ += pkt->data.psnr.psnr[0];
    nframes_++;
  }

  double GetAveragePsnr() const {
    if (nframes_)
      return psnr_ / nframes_;
    return 0.0;
  }

  double GetAverageMismatchPsnr() const {
    if (mismatch_nframes_)
      return mismatch_psnr_ / mismatch_nframes_;
    return 0.0;
  }

  virtual bool DoDecode() const {
    if (error_nframes_ > 0 &&
        (cfg_.g_pass == VPX_RC_LAST_PASS || cfg_.g_pass == VPX_RC_ONE_PASS)) {
      for (int i = 0; i < error_nframes_; ++i) {
        if (error_frames_[i] == nframes_) {
          std::cout << "             Skipping decoding frame: "
                    << error_frames_[i] << "\n";
          return 0;
        }
      }
    }
    return 1;
  }

  virtual void MismatchHook(const vpx_image_t *img1,
                            const vpx_image_t *img2) {
    double mismatch_psnr = compute_psnr(img1, img2);
    mismatch_psnr_ += mismatch_psnr;
    ++mismatch_nframes_;
    // std::cout << "Mismatch frame psnr: " << mismatch_psnr << "\n";
  }

  void SetErrorFrames(int num, unsigned int *list) {
    if (num > kMaxErrorFrames)
      num = kMaxErrorFrames;
    else if (num < 0)
      num = 0;
    error_nframes_ = num;
    for (int i = 0; i < error_nframes_; ++i)
      error_frames_[i] = list[i];
  }

 private:
  double psnr_;
  unsigned int nframes_;
  int error_nframes_;
  double mismatch_psnr_;
  int mismatch_nframes_;
  unsigned int error_frames_[kMaxErrorFrames];
  libvpx_test::TestMode encoding_mode_;
};

TEST_P(ErrorResilienceTest, OnVersusOff) {
  const vpx_rational timebase = { 33333333, 1000000000 };
  cfg_.g_timebase = timebase;
  cfg_.rc_target_bitrate = 2000;
  cfg_.g_lag_in_frames = 25;

  init_flags_ = VPX_CODEC_USE_PSNR;

  libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                     timebase.den, timebase.num, 0, 30);

  // Error resilient mode OFF.
  cfg_.g_error_resilient = 0;
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  const double psnr_resilience_off = GetAveragePsnr();
  EXPECT_GT(psnr_resilience_off, 25.0);

  // Error resilient mode ON.
  cfg_.g_error_resilient = 1;
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  const double psnr_resilience_on = GetAveragePsnr();
  EXPECT_GT(psnr_resilience_on, 25.0);

  // Test that turning on error resilient mode hurts by 10% at most.
  if (psnr_resilience_off > 0.0) {
    const double psnr_ratio = psnr_resilience_on / psnr_resilience_off;
    EXPECT_GE(psnr_ratio, 0.9);
    EXPECT_LE(psnr_ratio, 1.1);
  }
}

TEST_P(ErrorResilienceTest, DropFramesWithoutRecovery) {
  const vpx_rational timebase = { 33333333, 1000000000 };
  cfg_.g_timebase = timebase;
  cfg_.rc_target_bitrate = 2000;
  cfg_.g_lag_in_frames = 5;

  init_flags_ = VPX_CODEC_USE_PSNR;

  libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                     timebase.den, timebase.num, 0, 30);

  // Error resilient mode ON.
  cfg_.g_error_resilient = 1;
  // Set error frames
  unsigned int num_error_frames = 3;
  unsigned int error_frame_list[] = {3, 10, 20};
  SetErrorFrames(num_error_frames, error_frame_list);

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  // Test that dropping an arbitrary set of inter frames does not hurt too much
  // Note the Average Mismatch PSNR is the average of the PSNR between
  // decoded frame and encoder's version of the same frame for all frames
  // with mismatch.
  const double psnr_resilience_mismatch = GetAverageMismatchPsnr();
  EXPECT_GT(psnr_resilience_mismatch, 20.0);
  std::cout << "             Mismatch PSNR: "
            << psnr_resilience_mismatch << "\n";
}

INSTANTIATE_TEST_CASE_P(OnOffTest, ErrorResilienceTest,
                        ONE_PASS_TEST_MODES);
}  // namespace
