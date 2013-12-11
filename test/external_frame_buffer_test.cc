/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <string>

#include "test/codec_factory.h"
#include "test/decode_test_driver.h"
#include "test/ivf_video_source.h"
#include "test/md5_helper.h"
#include "test/test_vectors.h"
#include "test/util.h"
#include "test/webm_video_source.h"

namespace {

const int32_t VIDEO_NAME_PARAM = 1;

// Callback used by libvpx to request the application to allocate a frame
// buffer of at least |new_size| in bytes.
int32_t realloc_vp9_frame_buffer(void *user_priv, size_t new_size,
                                 vpx_codec_frame_buffer_t *fb) {
  (void)user_priv;
  if (!fb)
    return -1;

  delete [] fb->data;
  fb->data = new uint8_t[new_size];
  fb->size = new_size;
  return VPX_CODEC_OK;
}

// Class for testing passing in external frame buffers to libvpx.
class ExternalFrameBufferMD5Test : public ::libvpx_test::DecoderTest,
    public ::libvpx_test::CodecTestWithParam<const char*> {
 protected:
  ExternalFrameBufferMD5Test()
      : DecoderTest(GET_PARAM(::libvpx_test::CODEC_FACTORY_PARAM)),
        md5_file_(NULL),
        num_buffers_(0),
        frame_buffers_(NULL) {}

  virtual ~ExternalFrameBufferMD5Test() {
    for (int32_t i = 0; i < num_buffers_; ++i) {
      delete [] frame_buffers_[i].data;
    }
    delete [] frame_buffers_;

    if (md5_file_)
      fclose(md5_file_);
  }

  virtual void PreDecodeFrameHook(
      const libvpx_test::CompressedVideoSource& video,
      libvpx_test::Decoder *decoder) {
    if (num_buffers_ > 0 && video.frame_number() == 0) {
      // Have libvpx use frame buffers we create.
      frame_buffers_ = new vpx_codec_frame_buffer_t[num_buffers_];
      memset(frame_buffers_, 0, sizeof(frame_buffers_[0]) * num_buffers_);

      decoder->SetExternalFrameBuffers(frame_buffers_, num_buffers_,
                                       realloc_vp9_frame_buffer, NULL);
    }
  }

  void OpenMD5File(const std::string& md5_file_name_) {
    md5_file_ = libvpx_test::OpenTestDataFile(md5_file_name_);
    ASSERT_TRUE(md5_file_) << "Md5 file open failed. Filename: "
        << md5_file_name_;
  }

  virtual void DecompressedFrameHook(const vpx_image_t& img,
                                     const unsigned int frame_number) {
    ASSERT_TRUE(md5_file_ != NULL);
    char expected_md5[33];
    char junk[128];

    // Read correct md5 checksums.
    const int32_t res = fscanf(md5_file_, "%s  %s", expected_md5, junk);
    ASSERT_NE(res, EOF) << "Read md5 data failed";
    expected_md5[32] = '\0';

    ::libvpx_test::MD5 md5_res;
    md5_res.Add(&img);
    const char * const actual_md5 = md5_res.Get();

    // Check md5 match.
    ASSERT_STREQ(expected_md5, actual_md5)
        << "Md5 checksums don't match: frame number = " << frame_number;
  }

  void set_num_buffers(int32_t num_buffers) { num_buffers_ = num_buffers; }
  int32_t num_buffers() const { return num_buffers_; }

 private:
  FILE *md5_file_;
  int32_t num_buffers_;
  vpx_codec_frame_buffer_t *frame_buffers_;
};

// This test runs through the a set of test vectors, and decodes them.
// Libvpx will call into the application to allocate a frame buffer when
// needed. The md5 checksums are computed for each frame in the video file.
// If md5 checksums match the correct md5 data, then the test is passed.
// Otherwise, the test failed.
TEST_P(ExternalFrameBufferMD5Test, ExtFBMD5Match) {
  const std::string filename = GET_PARAM(VIDEO_NAME_PARAM);
  libvpx_test::CompressedVideoSource *video = NULL;

  // Number of buffers equals number of possible reference buffers(8), plus
  // one working buffer, plus four jitter buffers.
  const int32_t num_buffers = 13;
  set_num_buffers(num_buffers);

  // Tell compiler we are not using kVP8TestVectors.
  (void)kVP8TestVectors;

  // Open compressed video file.
  if (filename.substr(filename.length() - 3, 3) == "ivf") {
    video = new libvpx_test::IVFVideoSource(filename);
  } else if (filename.substr(filename.length() - 4, 4) == "webm") {
    video = new libvpx_test::WebMVideoSource(filename);
  }
  video->Init();

  // Construct md5 file name.
  const std::string md5_filename = filename + ".md5";
  OpenMD5File(md5_filename);

  // Decode frame, and check the md5 matching.
  ASSERT_NO_FATAL_FAILURE(RunLoop(video));
  delete video;
}

VP9_INSTANTIATE_TEST_CASE(ExternalFrameBufferMD5Test,
                          ::testing::ValuesIn(kVP9TestVectors));
}  // namespace
