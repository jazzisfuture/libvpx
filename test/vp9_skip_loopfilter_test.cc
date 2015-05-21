/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string>

#include "./vpx_config.h"
#include "test/codec_factory.h"
#include "test/decode_test_driver.h"
#include "test/md5_helper.h"
#include "test/util.h"
#if CONFIG_WEBM_IO
#include "test/webm_video_source.h"
#endif

namespace {

//const char kVP9TestFile[] = "vp90-2-02-size-lf-1920x1080.webm";
//const char kVP9Md5File[] = "vp90-2-02-size-lf-1920x1080.webm.md5";

const char kVP9TestFile[] = "vp90-2-08-tile_1x8_frame_parallel.webm";
const char kVP9Md5File[] = "vp90-2-08-tile_1x8_frame_parallel.webm.md5";

#if CONFIG_WEBM_IO
// Class for testing shutting off the loop filter.
class SkipLoopFilterTest {
 public:
  SkipLoopFilterTest()
      : video_(NULL),
        decoder_(NULL),
        md5_file_(NULL) {}

  // If |threads| > 0 then set the decoder with that number of threads.
  virtual void SetUp(int num_threads) {
    expected_md5_[0] = '\0';
    junk_[0] = '\0';
    video_ = new libvpx_test::WebMVideoSource(kVP9TestFile);
    ASSERT_TRUE(video_ != NULL);
    video_->Init();
    video_->Begin();

    vpx_codec_dec_cfg_t cfg = vpx_codec_dec_cfg_t();
    if (num_threads > 0)
      cfg.threads = num_threads;
    decoder_ = new libvpx_test::VP9Decoder(cfg, 0);
    ASSERT_TRUE(decoder_ != NULL);

    OpenMd5File(kVP9Md5File);
  }

  virtual void TearDown() {
    if (md5_file_ != NULL)
      fclose(md5_file_);

    delete decoder_;
    delete video_;
  }

  void SetSkipLoopFilter(int value, vpx_codec_err_t expected_value) {
    decoder_->Control(VP9_SET_SKIP_LOOP_FILTER, value, expected_value);
  }

  vpx_codec_err_t DecodeOneFrame() {
    const vpx_codec_err_t res =
        decoder_->DecodeFrame(video_->cxdata(), video_->frame_size());
    if (res == VPX_CODEC_OK) {
      ReadMd5();
      video_->Next();
    }
    return res;
  }

  vpx_codec_err_t DecodeRemainingFrames() {
    for (; video_->cxdata() != NULL; video_->Next()) {
      const vpx_codec_err_t res =
          decoder_->DecodeFrame(video_->cxdata(), video_->frame_size());
      if (res != VPX_CODEC_OK)
        return res;
      ReadMd5();
    }
    return VPX_CODEC_OK;
  }

  void CheckMd5(bool matches) {
    libvpx_test::DxDataIterator dec_iter = decoder_->GetDxData();
    const vpx_image_t *img = dec_iter.Next();
    CheckMd5Vpx(*img, matches);
  }

 private:
  // TODO(fgalligan): Move the MD5 testing code into another class.
  void OpenMd5File(const std::string &md5_file_name_) {
    md5_file_ = libvpx_test::OpenTestDataFile(md5_file_name_);
    ASSERT_TRUE(md5_file_ != NULL) << "MD5 file open failed. Filename: "
        << md5_file_name_;
  }

  void ReadMd5() {
    ASSERT_TRUE(md5_file_ != NULL);
    const int res = fscanf(md5_file_, "%s  %s", expected_md5_, junk_);
    ASSERT_NE(EOF, res) << "Read md5 data failed";
    expected_md5_[32] = '\0';
  }

  void CheckMd5Vpx(const vpx_image_t &img, bool matches) {
    ::libvpx_test::MD5 md5_res;
    md5_res.Add(&img);
    const char *const actual_md5 = md5_res.Get();

    // Check md5.
    if (matches)
      ASSERT_STREQ(expected_md5_, actual_md5) << "MD5 checksums don't match";
    else
      ASSERT_STRNE(expected_md5_, actual_md5) << "MD5 checksums match";
  }

  libvpx_test::WebMVideoSource *video_;
  libvpx_test::VP9Decoder *decoder_;
  FILE *md5_file_;
  char expected_md5_[33];
  char junk_[128];
};

TEST(SkipLoopFilterTest, ShutOffLoopFilter) {
  const int non_zero_value = 1;
  const int num_threads = 0;
  SkipLoopFilterTest skip_loop_filter;
  skip_loop_filter.SetUp(num_threads);
  skip_loop_filter.SetSkipLoopFilter(non_zero_value, VPX_CODEC_OK);
  ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeRemainingFrames());
  skip_loop_filter.CheckMd5(false);
}

TEST(SkipLoopFilterTest, ShutOffLoopFilterSingleThread) {
  const int non_zero_value = 1;
  const int num_threads = 1;
  SkipLoopFilterTest skip_loop_filter;
  skip_loop_filter.SetUp(num_threads);
  skip_loop_filter.SetSkipLoopFilter(non_zero_value, VPX_CODEC_OK);
  ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeRemainingFrames());
  skip_loop_filter.CheckMd5(false);
}

TEST(SkipLoopFilterTest, ShutOffLoopFilter8Threads) {
  const int non_zero_value = 1;
  const int num_threads = 8;
  SkipLoopFilterTest skip_loop_filter;
  skip_loop_filter.SetUp(num_threads);
  skip_loop_filter.SetSkipLoopFilter(non_zero_value, VPX_CODEC_OK);
  ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeRemainingFrames());
  skip_loop_filter.CheckMd5(false);
}

TEST(SkipLoopFilterTest, WithLoopFilter) {
  const int non_zero_value = 1;
  const int num_threads = 0;
  SkipLoopFilterTest skip_loop_filter;
  skip_loop_filter.SetUp(num_threads);
  skip_loop_filter.SetSkipLoopFilter(non_zero_value, VPX_CODEC_OK);
  skip_loop_filter.SetSkipLoopFilter(0, VPX_CODEC_OK);
  ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeRemainingFrames());
  skip_loop_filter.CheckMd5(true);
}

TEST(SkipLoopFilterTest, ToggleLoopFilter) {
  const int num_threads = 0;
  SkipLoopFilterTest skip_loop_filter;
  skip_loop_filter.SetUp(num_threads);

  for (int i = 0; i < 10; ++i) {
    skip_loop_filter.SetSkipLoopFilter(i % 2, VPX_CODEC_OK);
    ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeOneFrame());
  }
  ASSERT_EQ(VPX_CODEC_OK, skip_loop_filter.DecodeRemainingFrames());
  skip_loop_filter.CheckMd5(false);
}

#endif  // CONFIG_WEBM_IO

}  // namespace
