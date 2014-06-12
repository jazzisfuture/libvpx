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
#include "./y4menc.h"

namespace {

using std::string;

static const unsigned int   kWidth = 160;
static const unsigned int  kHeight = 90;
static const unsigned int  kFrames = 10;
static const string kOutFilePrefix = "y4m_test_out";

static void write_image_file(const vpx_image_t *img, FILE *file) {
  int plane, y;
  for (plane = 0; plane < 3; ++plane) {
    const unsigned char *buf = img->planes[plane];
    const int stride = img->stride[plane];
    const int bytes_per_sample = (img->fmt & VPX_IMG_FMT_HIGH) ? 2 : 1;
    const int h = (plane ? (img->d_h + img->y_chroma_shift) >>
                   img->y_chroma_shift : img->d_h);
    const int w = (plane ? (img->d_w + img->x_chroma_shift) >>
                   img->x_chroma_shift : img->d_w);
    for (y = 0; y < h; ++y) {
      fwrite(buf, bytes_per_sample, w, file);
      buf += stride;
    }
  }
}

class Y4mVideoSourceTest
: public ::testing::Test, public ::libvpx_test::Y4mVideoSource {
 protected:
  Y4mVideoSourceTest() : Y4mVideoSource("", 0, 0), out_file_name_("") {}

  virtual ~Y4mVideoSourceTest() {
    CloseSource();
    if (!out_file_name_.empty())
      remove(out_file_name_.c_str());
  }

  virtual void Init(const std::string &file_name, int limit) {
    file_name_ = file_name;
    start_ = 0;
    limit_ = limit;
    Begin();
  }

  // Checks y4m header information
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

  // Checks MD5 of the raw frame data
  void Md5Check(const string &expected_md5) {
    ASSERT_TRUE(input_file_ != NULL);
    libvpx_test::MD5 md5;
    for (unsigned int i = start_; i < limit_; i++) {
      md5.Add(img());
      Next();
    }
    ASSERT_EQ(string(md5.Get()), expected_md5);
  }

  // Writes out a y4m file and then reads it back
  void WriteY4mAndReadBack(const string &kOutFileName) {
    Begin();
    ASSERT_TRUE(input_file_ != NULL);
    if (!out_file_name_.empty())
      remove(out_file_name_.c_str());
    char buf[Y4M_BUFFER_SIZE] = {0};
    const struct VpxRational framerate = {y4m_.fps_n, y4m_.fps_d};
    vpx_bit_depth_t vpx_bit_depth;
    if (y4m_.vpx_bit_depth == 8)
      vpx_bit_depth = VPX_BITS_8;
    else if (y4m_.vpx_bit_depth == 10)
      vpx_bit_depth = VPX_BITS_10;
    else if (y4m_.vpx_bit_depth == 12)
      vpx_bit_depth = VPX_BITS_12;
    else
      ASSERT_TRUE(0);
    FILE *outfile = libvpx_test::OpenTestOutFile(kOutFileName);
    y4m_write_file_header(buf, sizeof(buf),
                          kWidth, kHeight,
                          &framerate, y4m_.vpx_fmt,
                          vpx_bit_depth);
    fputs(buf, outfile);
    for (unsigned int i = start_; i < limit_; i++) {
      y4m_write_frame_header(buf, sizeof(buf));
      fputs(buf, outfile);
      write_image_file(img(), outfile);
      Next();
    }
    fclose(outfile);
    out_file_name_ = kOutFileName;
    Init(out_file_name_, kFrames);
  }

  string out_file_name_;
};

TEST_F(Y4mVideoSourceTest, Y4m420_8) {
  const string kMd5Raw = "e5406275b9fc6bb3436c31d4a05c1cab";
  Init("park_joy_90p_8_420.y4m", kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I420);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_420_8.y4m");
  HeaderChecks(8, VPX_IMG_FMT_I420);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m422_8) {
  const string kMd5Raw = "284a47a47133b12884ec3a14e959a0b6";
  Init("park_joy_90p_8_422.y4m", kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I422);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_422_8.y4m");
  HeaderChecks(8, VPX_IMG_FMT_I422);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m444_8) {
  const string kMd5Raw = "90517ff33843d85de712fd4fe60dbed0";
  Init("park_joy_90p_8_444.y4m", kFrames);
  HeaderChecks(8, VPX_IMG_FMT_I444);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_444_8.y4m");
  HeaderChecks(8, VPX_IMG_FMT_I444);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m420_10) {
  const string kMd5Raw = "63f21f9f717d8b8631bd2288ee87137b";
  Init("park_joy_90p_10_420.y4m", kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42016);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_420_10.y4m");
  HeaderChecks(10, VPX_IMG_FMT_I42016);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m422_10) {
  const string kMd5Raw = "48ab51fb540aed07f7ff5af130c9b605";
  Init("park_joy_90p_10_422.y4m", kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I42216);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_422_10.y4m");
  HeaderChecks(10, VPX_IMG_FMT_I42216);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m444_10) {
  const string kMd5Raw = "067bfd75aa85ff9bae91fa3e0edd1e3e";
  Init("park_joy_90p_10_444.y4m", kFrames);
  HeaderChecks(10, VPX_IMG_FMT_I44416);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_444_10.y4m");
  HeaderChecks(10, VPX_IMG_FMT_I44416);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m420_12) {
  const string kMd5Raw = "9e6d8f6508c6e55625f6b697bc461cef";
  Init("park_joy_90p_12_420.y4m", kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42016);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_420_12.y4m");
  HeaderChecks(12, VPX_IMG_FMT_I42016);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m422_12) {
  const string kMd5Raw = "b239c6b301c0b835485be349ca83a7e3";
  Init("park_joy_90p_12_422.y4m", kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I42216);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_422_12.y4m");
  HeaderChecks(12, VPX_IMG_FMT_I42216);
  Md5Check(kMd5Raw);
}

TEST_F(Y4mVideoSourceTest, Y4m444_12) {
  const string kMd5Raw = "5a6481a550821dab6d0192f5c63845e9";
  Init("park_joy_90p_12_444.y4m", kFrames);
  HeaderChecks(12, VPX_IMG_FMT_I44416);
  Md5Check(kMd5Raw);
  WriteY4mAndReadBack(kOutFilePrefix + "_444_12.y4m");
  HeaderChecks(12, VPX_IMG_FMT_I44416);
  Md5Check(kMd5Raw);
}
}  // namespace
