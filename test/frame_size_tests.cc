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

class FrameSizeTests : public ::libvpx_test::EncoderTest,
    public ::libvpx_test::CodecTestWithParam<libvpx_test::TestMode> {
 protected:
  FrameSizeTests() : EncoderTest(GET_PARAM(0)) {}
  virtual ~FrameSizeTests() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(GET_PARAM(1));
  }


  virtual bool HandleDecodeResult(
      const vpx_codec_err_t res_dec,
      const libvpx_test::VideoSource &video,
      libvpx_test::Decoder *decoder) {
    // Check results match.
    EXPECT_EQ(expected_res_, res_dec)
        << "Expected " << expected_res_
        << "but got " << res_dec;

    return !HasFailure();
  }

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(VP8E_SET_CPUUSED, 7);
      encoder->Control(VP8E_SET_ENABLEAUTOALTREF, 1);
      encoder->Control(VP8E_SET_ARNR_MAXFRAMES, 7);
      encoder->Control(VP8E_SET_ARNR_STRENGTH, 5);
      encoder->Control(VP8E_SET_ARNR_TYPE, 3);
    }
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
  }

  int expected_res_;
};

TEST_P(FrameSizeTests, TestRandomVideoSource) {
  ::libvpx_test::RandomVideoSource video;

#if CONFIG_SIZE_LIMIT
  video.SetSize(DECODE_WIDTH_LIMIT + 16, DECODE_HEIGHT_LIMIT+16);
  video.SetLimit(2);
  expected_res_ = VPX_CODEC_CORRUPT_FRAME;
#else
  video.SetSize(65535, 65535);
  video.SetLimit(2);
  expected_res_ = VPX_CODEC_OK;
#if UINT_MAX == SIZE_MAX
  expected_res_ = VPX_CODEC_MEM_ERROR;
#endif
#endif

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

VP9_INSTANTIATE_TEST_CASE(FrameSizeTests, ::testing::Values(::libvpx_test::kRealTime));
}  // namespace
