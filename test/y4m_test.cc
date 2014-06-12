/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string>
#include "test/y4m_video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "./vpx_config.h"

namespace {

static const int kWidth = 160;
static const int kHeight = 90;
static const int kFrames = 10;

class Y4mVideoSourceTest
    : public ::testing::Test, public ::libvpx_test::Y4mVideoSource {
 protected:
  Y4mVideoSourceTest() : Y4mVideoSource("", 0, 0) {}

  virtual void Init(const std::string &file_name,
                    unsigned int start, int limit) {
    file_name_ = file_name;
    start_ = start;
    limit_ = limit;
    OpenSource();
  }

  void HeaderChecks(unsigned int bit_depth, vpx_img_fmt_t fmt) {
    ASSERT_EQ(y4m_.pic_w, kWidth);
    ASSERT_EQ(y4m_.pic_h, kHeight);
    ASSERT_EQ(y4m_.vpx_bit_depth, bit_depth);
    ASSERT_EQ(y4m_.vpx_fmt, fmt);
    if (fmt == VPX_IMG_FMT_I420 || fmt == VPX_IMG_FMT_I42016) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 3 / 2);
    }
    if (fmt == VPX_IMG_FMT_I422 || fmt == VPX_IMG_FMT_I42216) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 2);
    }
    if (fmt == VPX_IMG_FMT_I444 || fmt == VPX_IMG_FMT_I44416) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 3);
    }
  }
};

TEST_F(Y4mVideoSourceTest, Y4m420_8) {
  Init("park_joy_90p_8_420.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I420);
}

TEST_F(Y4mVideoSourceTest, Y4m422_8) {
  Init("park_joy_90p_8_422.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I422);
}

TEST_F(Y4mVideoSourceTest, Y4m444_8) {
  Init("park_joy_90p_8_444.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I444);
}

TEST_F(Y4mVideoSourceTest, Y4m420_10) {
  Init("park_joy_90p_10_420.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42016);
}

TEST_F(Y4mVideoSourceTest, Y4m422_10) {
  Init("park_joy_90p_10_422.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42216);
}

TEST_F(Y4mVideoSourceTest, Y4m444_10) {
  Init("park_joy_90p_10_444.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I44416);
}

TEST_F(Y4mVideoSourceTest, Y4m420_12) {
  Init("park_joy_90p_12_420.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42016);
}

TEST_F(Y4mVideoSourceTest, Y4m422_12) {
  Init("park_joy_90p_12_422.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42216);
}

TEST_F(Y4mVideoSourceTest, Y4m444_12) {
  Init("park_joy_90p_12_444.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I44416);
}
}  // namespace
