/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/video_source.h"

namespace {

class VP8FramgmentsTest
    : public ::libvpx_test::EncoderTest,
      public ::testing::Test {
 protected:
  VP8FramgmentsTest() : EncoderTest(&::libvpx_test::kVP8),
                        expected_res_(VPX_CODEC_OK) {}
  virtual ~VP8FramgmentsTest() {}

  virtual void SetUp() {
    unsigned long init_flags = VPX_CODEC_USE_OUTPUT_PARTITION;
    InitializeConfig();
    SetMode(::libvpx_test::kRealTime);
    set_init_flags(init_flags);
  }

  virtual bool HandleDecodeResult(const vpx_codec_err_t res_dec,
                                  const libvpx_test::VideoSource& /*video*/,
                                  libvpx_test::Decoder *decoder) {
    EXPECT_EQ(expected_res_, res_dec) << decoder->DecodeError();
    return !::testing::Test::HasFailure();
  }

  int expected_res_;
};

TEST_F(VP8FramgmentsTest, TestFragmentsEncodeDecode) {
  ::libvpx_test::RandomVideoSource video;
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

}  // namespace
