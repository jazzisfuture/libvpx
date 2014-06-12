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
#include "test/md5_helper.h"
#include "test/y4m_video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "./vpx_config.h"

namespace {

using std::string;

static const unsigned int kWidth = 160;
static const unsigned int kHeight = 90;
static const unsigned int kFrames = 10;

void SaveImage(const vpx_image_t *img) {
  FILE *fp = fopen("/tmp/test.img", "ab");
  for (int plane = 0; plane < 3; ++plane) {
    const uint8_t *buf = img->planes[plane];
    const int bytes_per_sample = (img->fmt & VPX_IMG_FMT_HIGH) ? 2 : 1;
    const int h = plane ? (img->d_h + img->y_chroma_shift) >>
        img->y_chroma_shift : img->d_h;
    const int w = (plane ? (img->d_w + img->x_chroma_shift) >>
                   img->x_chroma_shift : img->d_w) * bytes_per_sample;
    for (int y = 0; y < h; ++y) {
      fwrite(buf, w, 1, fp);
      buf += img->stride[plane];
    }
  }
}

class Y4mVideoSourceTest
    : public ::testing::Test, public ::libvpx_test::Y4mVideoSource {
 protected:
  Y4mVideoSourceTest() : Y4mVideoSource("", 0, 0), fp(NULL) {}

  virtual void Init(const std::string &file_name,
                    unsigned int start, int limit) {
    file_name_ = file_name;
    start_ = start;
    limit_ = limit;
    OpenSource();
    ReadSourceToStart();
  }

  void HeaderChecks(unsigned int bit_depth, vpx_img_fmt_t fmt) {
    ASSERT_TRUE(input_file_ != NULL);
    ASSERT_EQ(y4m_.pic_w, (int)kWidth);
    ASSERT_EQ(y4m_.pic_h, (int)kHeight);
    ASSERT_EQ(img()->d_w, kWidth);
    ASSERT_EQ(img()->d_h, kHeight);
    ASSERT_EQ(y4m_.vpx_bit_depth, bit_depth);
    ASSERT_EQ(y4m_.vpx_fmt, fmt);
    if (fmt == VPX_IMG_FMT_I420 || fmt == VPX_IMG_FMT_I42016) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 3 / 2);
      ASSERT_EQ(img()->x_chroma_shift, 1U);
      ASSERT_EQ(img()->y_chroma_shift, 1U);
    }
    if (fmt == VPX_IMG_FMT_I422 || fmt == VPX_IMG_FMT_I42216) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 2);
      ASSERT_EQ(img()->x_chroma_shift, 1U);
      ASSERT_EQ(img()->y_chroma_shift, 0U);
    }
    if (fmt == VPX_IMG_FMT_I444 || fmt == VPX_IMG_FMT_I44416) {
      ASSERT_EQ(y4m_.vpx_bps, (int)y4m_.vpx_bit_depth * 3);
      ASSERT_EQ(img()->x_chroma_shift, 0U);
      ASSERT_EQ(img()->y_chroma_shift, 0U);
    }
  }

  void Md5Check(const string &expected_md5, int write = 0) {
    ASSERT_TRUE(input_file_ != NULL);
    libvpx_test::MD5 md5;
    // ReadSourceToStart();
    for (unsigned int i = start_; i < limit_; i++) {
      md5.Add(img());
      if (write) SaveImage(img());
      Next();
    }
    ASSERT_EQ(string(md5.Get()), expected_md5);
  }

  FILE *fp;
};

TEST_F(Y4mVideoSourceTest, Y4m420_8) {
  Init("park_joy_90p_8_420.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I420);
  Md5Check("e5406275b9fc6bb3436c31d4a05c1cab");
}

TEST_F(Y4mVideoSourceTest, Y4m422_8) {
  Init("park_joy_90p_8_422.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I422);
  Md5Check("284a47a47133b12884ec3a14e959a0b6");
}

TEST_F(Y4mVideoSourceTest, Y4m444_8) {
  Init("park_joy_90p_8_444.y4m", 0, kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I444);
  Md5Check("90517ff33843d85de712fd4fe60dbed0");
}

TEST_F(Y4mVideoSourceTest, Y4m420_10) {
  Init("park_joy_90p_10_420.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42016);
  Md5Check("63f21f9f717d8b8631bd2288ee87137b", 1);
}

TEST_F(Y4mVideoSourceTest, Y4m422_10) {
  Init("park_joy_90p_10_422.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42216);
  Md5Check("48ab51fb540aed07f7ff5af130c9b605");
}

TEST_F(Y4mVideoSourceTest, Y4m444_10) {
  Init("park_joy_90p_10_444.y4m", 0, kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I44416);
  Md5Check("067bfd75aa85ff9bae91fa3e0edd1e3e");
}

TEST_F(Y4mVideoSourceTest, Y4m420_12) {
  Init("park_joy_90p_12_420.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42016);
  Md5Check("9e6d8f6508c6e55625f6b697bc461cef");
}

TEST_F(Y4mVideoSourceTest, Y4m422_12) {
  Init("park_joy_90p_12_422.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42216);
  Md5Check("b239c6b301c0b835485be349ca83a7e3");
}

TEST_F(Y4mVideoSourceTest, Y4m444_12) {
  Init("park_joy_90p_12_444.y4m", 0, kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I44416);
  Md5Check("5a6481a550821dab6d0192f5c63845e9");
}
}  // namespace
