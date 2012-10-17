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
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
namespace {

class DatarateTest : public ::libvpx_test::EncoderTest,
    public ::testing::TestWithParam<enum libvpx_test::TestMode> {
 protected:
  virtual void SetUp() {
    InitializeConfig();
    SetMode(GetParam());
    ResetModel();
  }

  virtual void ResetModel() {
    last_pts_ = 0;
    bytes_in_buffer_model_ = cfg_.rc_target_bitrate * cfg_.rc_buf_initial_sz
        / 1000  /* milliseconds per second */
        * 1000  /* bits per kibibit  */
        / 8  /* bits per byte */;
    frame_number_ = 0;
    first_drop_ = 0;
    bytes_total_ = 0;
    duration_ = 0.0;
  }

  virtual bool Continue() const {
    return !HasFatalFailure() && !abort_;
  }

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    const vpx_rational_t tb = video->timebase();
    timebase_ = static_cast<double>(tb.num) / tb.den;
    duration_ = video->limit() * timebase_;
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    // Time since last timestamp = duration.
    const vpx_codec_pts_t duration = pkt->data.frame.pts - last_pts_;

    // Add to the buffer the bytes we'd expect from a constant bitrate server.
    bytes_in_buffer_model_ += duration * timebase_ * cfg_.rc_target_bitrate
        * 1024 / 8;

    /* Test the buffer model here before subtracting the frame. Do so because
     * the way the leaky bucket model works in libvpx is to allow the buffer to
     * empty - and then stop showing frames until we've got enough bytes to
     * show one. */
    ASSERT_GE(bytes_in_buffer_model_, 0) << "Buffer Underrun at frame "
        << frame_number_;

    // Subtract from the buffer the bytes associated with a played back frame.
    bytes_in_buffer_model_ -= pkt->data.frame.sz;

    // Update the running total of bytes for end of test datarate checks.
    bytes_total_ += pkt->data.frame.sz;

    // If first drop not set and we have a drop set it to this time.
    if (!first_drop_ && duration > 1)
      first_drop_ = last_pts_ + 1;

    // Update the most recent pts.
    last_pts_ = pkt->data.frame.pts;

    ++frame_number_;
  }

  virtual void EndPassHook(void) {
    if (bytes_total_) {
      const double file_size_in_kb = bytes_total_ * 8  /* bits per byte */
          / 1024  /* bits per kilobit */;

      // Effective file datarate includes the time spent prebuffering.
      effective_datarate_ = file_size_in_kb
          / (cfg_.rc_buf_initial_sz / 1000.0 + duration_);

      file_datarate_ = file_size_in_kb / duration_;
    }
  }

  vpx_codec_pts_t last_pts_;
  int bytes_in_buffer_model_;
  double timebase_;
  int frame_number_;
  vpx_codec_pts_t first_drop_;
  int64_t bytes_total_;
  double duration_;
  double file_datarate_;
  double effective_datarate_;
};

TEST_P(DatarateTest, BasicBufferModel) {
  cfg_.rc_buf_initial_sz = 500;
  cfg_.rc_dropframe_thresh = 1;
  cfg_.rc_max_quantizer = 56;
  cfg_.rc_end_usage = VPX_CBR;

  ::libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                       30, 1, 0, 140);
  cfg_.rc_target_bitrate = 70;
  ResetModel();
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  /* Don't check overall bitrate because at the end of the clip it's left in a
   * dropping frames state. */

  for (int i = 100; i < 700; i += 200) {
    cfg_.rc_target_bitrate = i;
    ResetModel();
    ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
    ASSERT_GE(cfg_.rc_target_bitrate, effective_datarate_)
        << " The datarate for the file exceeds the target!";

    ASSERT_LE(cfg_.rc_target_bitrate, file_datarate_ * 1.3)
        << " The datarate for the file missed the target!";
  }
}

TEST_P(DatarateTest, ChangingDropFrameThresh) {
  cfg_.rc_buf_initial_sz = 500;
  cfg_.rc_max_quantizer = 36;
  cfg_.rc_end_usage = VPX_CBR;
  cfg_.rc_target_bitrate = 200;
  cfg_.kf_mode = VPX_KF_DISABLED;

  ::libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                       30, 1, 0, 40);

  // Here we check that the first dropped frame gets earlier and earlier
  // as the drop frame threshold is increased.

  const int kDropFrameThreshTestStep = 30;
  vpx_codec_pts_t last_drop = 0;
  for (int i = 1; i < 91; i += kDropFrameThreshTestStep) {
    cfg_.rc_dropframe_thresh = i;
    ResetModel();
    ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
    ASSERT_GE(first_drop_, last_drop)
        << " The first dropped frame for drop_thresh " << i
        << " > first dropped frame for drop_thresh "
        << i - kDropFrameThreshTestStep;
  }
}

INSTANTIATE_TEST_CASE_P(AllModes, DatarateTest, ALL_TEST_MODES);
}  // namespace
