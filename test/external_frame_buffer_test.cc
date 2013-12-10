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
#include "test/util.h"
#include "test/webm_video_source.h"

namespace {

const int VIDEO_NAME_PARAM = 1;

#if CONFIG_VP9_DECODER
const char *kVP9TestVectors[] = {
  "vp90-2-01-sharpness-7.webm", "vp90-2-02-size-08x08.webm",
  "vp90-2-02-size-08x10.webm", "vp90-2-02-size-08x16.webm",
  "vp90-2-02-size-08x18.webm", "vp90-2-02-size-08x32.webm",
  "vp90-2-02-size-08x34.webm", "vp90-2-02-size-08x64.webm",
  "vp90-2-02-size-08x66.webm", "vp90-2-02-size-10x08.webm",
  "vp90-2-02-size-10x10.webm", "vp90-2-02-size-10x16.webm",
  "vp90-2-02-size-10x18.webm", "vp90-2-02-size-10x32.webm",
  "vp90-2-02-size-10x34.webm", "vp90-2-02-size-10x64.webm",
  "vp90-2-02-size-10x66.webm", "vp90-2-02-size-16x08.webm",
  "vp90-2-02-size-16x10.webm", "vp90-2-02-size-16x16.webm",
  "vp90-2-02-size-16x18.webm", "vp90-2-02-size-16x32.webm",
  "vp90-2-02-size-16x34.webm", "vp90-2-02-size-16x64.webm",
  "vp90-2-02-size-16x66.webm", "vp90-2-02-size-18x08.webm",
  "vp90-2-02-size-18x10.webm", "vp90-2-02-size-18x16.webm",
  "vp90-2-02-size-18x18.webm", "vp90-2-02-size-18x32.webm",
  "vp90-2-02-size-18x34.webm", "vp90-2-02-size-18x64.webm",
  "vp90-2-02-size-18x66.webm", "vp90-2-02-size-32x08.webm",
  "vp90-2-02-size-32x10.webm", "vp90-2-02-size-32x16.webm",
  "vp90-2-02-size-32x18.webm", "vp90-2-02-size-32x32.webm",
  "vp90-2-02-size-32x34.webm", "vp90-2-02-size-32x64.webm",
  "vp90-2-02-size-32x66.webm", "vp90-2-02-size-34x08.webm",
  "vp90-2-02-size-34x10.webm", "vp90-2-02-size-34x16.webm",
  "vp90-2-02-size-34x18.webm", "vp90-2-02-size-34x32.webm",
  "vp90-2-02-size-34x34.webm", "vp90-2-02-size-34x64.webm",
  "vp90-2-02-size-34x66.webm", "vp90-2-02-size-64x08.webm",
  "vp90-2-02-size-64x10.webm", "vp90-2-02-size-64x16.webm",
  "vp90-2-02-size-64x18.webm", "vp90-2-02-size-64x32.webm",
  "vp90-2-02-size-64x34.webm", "vp90-2-02-size-64x64.webm",
  "vp90-2-02-size-64x66.webm", "vp90-2-02-size-66x08.webm",
  "vp90-2-02-size-66x10.webm", "vp90-2-02-size-66x16.webm",
  "vp90-2-02-size-66x18.webm", "vp90-2-02-size-66x32.webm",
  "vp90-2-02-size-66x34.webm", "vp90-2-02-size-66x64.webm",
  "vp90-2-02-size-66x66.webm", "vp90-2-03-size-196x196.webm",
  "vp90-2-03-size-196x198.webm", "vp90-2-03-size-196x200.webm",
  "vp90-2-03-size-196x202.webm", "vp90-2-03-size-196x208.webm",
  "vp90-2-03-size-196x210.webm", "vp90-2-03-size-196x224.webm",
  "vp90-2-03-size-196x226.webm", "vp90-2-03-size-198x196.webm",
  "vp90-2-03-size-198x198.webm", "vp90-2-03-size-198x200.webm",
  "vp90-2-03-size-198x202.webm", "vp90-2-03-size-198x208.webm",
  "vp90-2-03-size-198x210.webm", "vp90-2-03-size-198x224.webm",
  "vp90-2-03-size-198x226.webm", "vp90-2-03-size-200x196.webm",
  "vp90-2-03-size-200x198.webm", "vp90-2-03-size-200x200.webm",
  "vp90-2-03-size-200x202.webm", "vp90-2-03-size-200x208.webm",
  "vp90-2-03-size-200x210.webm", "vp90-2-03-size-200x224.webm",
  "vp90-2-03-size-200x226.webm", "vp90-2-03-size-202x196.webm",
  "vp90-2-03-size-202x198.webm", "vp90-2-03-size-202x200.webm",
  "vp90-2-03-size-202x202.webm", "vp90-2-03-size-202x208.webm",
  "vp90-2-03-size-202x210.webm", "vp90-2-03-size-202x224.webm",
  "vp90-2-03-size-202x226.webm", "vp90-2-03-size-208x196.webm",
  "vp90-2-03-size-208x198.webm", "vp90-2-03-size-208x200.webm",
  "vp90-2-03-size-208x202.webm", "vp90-2-03-size-208x208.webm",
  "vp90-2-03-size-208x210.webm", "vp90-2-03-size-208x224.webm",
  "vp90-2-03-size-208x226.webm", "vp90-2-03-size-210x196.webm",
  "vp90-2-03-size-210x198.webm", "vp90-2-03-size-210x200.webm",
  "vp90-2-03-size-210x202.webm", "vp90-2-03-size-210x208.webm",
  "vp90-2-03-size-210x210.webm", "vp90-2-03-size-210x224.webm",
  "vp90-2-03-size-210x226.webm", "vp90-2-03-size-224x196.webm",
  "vp90-2-03-size-224x198.webm", "vp90-2-03-size-224x200.webm",
  "vp90-2-03-size-224x202.webm", "vp90-2-03-size-224x208.webm",
  "vp90-2-03-size-224x210.webm", "vp90-2-03-size-224x224.webm",
  "vp90-2-03-size-224x226.webm", "vp90-2-03-size-226x196.webm",
  "vp90-2-03-size-226x198.webm", "vp90-2-03-size-226x200.webm",
  "vp90-2-03-size-226x202.webm", "vp90-2-03-size-226x208.webm",
  "vp90-2-03-size-226x210.webm", "vp90-2-03-size-226x224.webm",
  "vp90-2-03-size-226x226.webm", "vp90-2-05-resize.ivf",
  "vp90-2-07-frame_parallel.webm",
  "vp90-2-08-tile_1x2_frame_parallel.webm", "vp90-2-08-tile_1x2.webm",
  "vp90-2-08-tile_1x4_frame_parallel.webm", "vp90-2-08-tile_1x4.webm",
  "vp90-2-08-tile-4x4.webm", "vp90-2-08-tile-4x1.webm",
  "vp90-2-09-subpixel-00.ivf",
};
#endif

// Callback used by libvpx to request the application to allocate a frame
// buffer of at least |new_size| in bytes.
int realloc_vp9_frame_buffer(void *user_priv, int new_size,
                             vpx_codec_frame_buffer_t *fb) {
  (void)user_priv;
  if (!fb)
    return -1;

  delete [] fb->data;
  fb->data = new unsigned char[new_size];
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
    for (int i = 0; i < num_buffers_; ++i) {
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
    const int res = fscanf(md5_file_, "%s  %s", expected_md5, junk);
    ASSERT_NE(res, EOF) << "Read md5 data failed";
    expected_md5[32] = '\0';

    ::libvpx_test::MD5 md5_res;
    md5_res.Add(&img);
    const char *actual_md5 = md5_res.Get();

    // Check md5 match.
    ASSERT_STREQ(expected_md5, actual_md5)
        << "Md5 checksums don't match: frame number = " << frame_number;
  }

  void set_num_buffers(int num_buffers) { num_buffers_ = num_buffers; }
  int num_buffers() const { return num_buffers_; }

 private:
  FILE *md5_file_;
  int num_buffers_;
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
  const int num_buffers = 13;
  set_num_buffers(num_buffers);

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
