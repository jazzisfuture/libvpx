/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
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
#include "test/ivf_video_source.h"
#include "test/md5_helper.h"
#include "test/test_vectors.h"
#include "test/util.h"
#if CONFIG_WEBM_IO
#include "test/webm_video_source.h"
#endif

namespace {

const int kVideoNameParam = 1;

struct ExternalFrameBuffer {
  uint8_t *plane[4];
  int stride[4];
  int in_use;
};

// Class to manipulate a list of external frame buffers.
class ExternalFrameBufferList {
 public:
  ExternalFrameBufferList()
      : num_buffers_(0),
        ext_fb_list_(NULL) {}

  virtual ~ExternalFrameBufferList() {
    for (int i = 0; i < num_buffers_; ++i) {
      for (int plane = 0; plane < 4; ++plane) {
	delete [] ext_fb_list_[i].plane[plane];
      }
    }
    delete [] ext_fb_list_;
  }

  // Creates the list to hold the external buffers. Returns true on success.
  bool CreateBufferList(int num_buffers) {
    if (num_buffers < 0)
      return false;

    num_buffers_ = num_buffers;
    ext_fb_list_ = new ExternalFrameBuffer[num_buffers_];
    EXPECT_TRUE(ext_fb_list_ != NULL);
    memset(ext_fb_list_, 0, sizeof(ext_fb_list_[0]) * num_buffers_);
    return true;
  }

  int GetFreeFrameBuffer(vpx_img_fmt_t fmt, size_t width, size_t height, vpx_codec_frame_buffer_t *fb) {
    EXPECT_TRUE(fb != NULL);
    EXPECT_TRUE(fmt == VPX_IMG_FMT_I420 || fmt == VPX_IMG_FMT_I444 ||
		fmt == VPX_IMG_FMT_I422 || fmt == VPX_IMG_FMT_I440) << " fmt is: " << fmt;

    const int idx = FindFreeBufferIndex();

    if (idx == num_buffers_)
      return -1;

    for (int plane = 0; plane < 3; ++plane) {
      size_t plane_stride = width;
      size_t plane_height = height;

      if (plane > 0 && fmt != VPX_IMG_FMT_I444) {
	if (fmt != VPX_IMG_FMT_I440)
	  plane_stride /= 2;
	if (fmt == VPX_IMG_FMT_I420 || fmt == VPX_IMG_FMT_I440)
	  plane_height /= 2;
      }
      // TODO(dcastagna): stride should change.
      plane_stride = (plane_stride + 31) & ~31;

      delete [] ext_fb_list_[idx].plane[plane];
      ext_fb_list_[idx].plane[plane] = new uint8_t[plane_height * plane_stride];
      memset(ext_fb_list_[idx].plane[plane], 0, plane_height * plane_stride);
      ext_fb_list_[idx].stride[plane] = plane_stride;
    }

    SetFrameBuffer(idx, fb);
    return 0;
  }

  int ReturnFrameBuffer(vpx_codec_frame_buffer_t *fb) {
    if (fb == NULL) {
      EXPECT_TRUE(fb != NULL);
      return -1;
    }
    ExternalFrameBuffer *const ext_fb =
        reinterpret_cast<ExternalFrameBuffer*>(fb->priv);
    if (ext_fb == NULL) {
      EXPECT_TRUE(ext_fb != NULL);
      return -1;
    }
    EXPECT_EQ(1, ext_fb->in_use);
    ext_fb->in_use = 0;
    return 0;
  }

  // Checks that the ximage data is contained within the external frame buffer
  // private data passed back in the ximage.
  void CheckXImageFrameBuffer(const vpx_image_t *img) {
    if (img->fb_priv != NULL) {
      const struct ExternalFrameBuffer *const ext_fb =
          reinterpret_cast<ExternalFrameBuffer*>(img->fb_priv);

      for (int plane=0; plane<4; ++plane) {
	if (ext_fb->stride[plane]) {
	  EXPECT_TRUE(img->planes[plane] == ext_fb->plane[plane] &&
		      img->stride[plane] == ext_fb->stride[plane]);
	}
      }
    }
  }

 private:
  // Returns the index of the first free frame buffer. Returns |num_buffers_|
  // if there are no free frame buffers.
  int FindFreeBufferIndex() {
    int i;
    for (i = 0; i < num_buffers_; ++i) {
      if (!ext_fb_list_[i].in_use)
        break;
    }
    return i;
  }

  void SetFrameBuffer(int idx, vpx_codec_frame_buffer_t *fb) {
    ASSERT_TRUE(fb != NULL);
    memcpy(fb->plane, ext_fb_list_[idx].plane, sizeof(fb->plane));
    memcpy(fb->stride, ext_fb_list_[idx].stride, sizeof(fb->stride));
    ASSERT_EQ(0, ext_fb_list_[idx].in_use);
    ext_fb_list_[idx].in_use = 1;
    fb->priv = &ext_fb_list_[idx];
  }

  int num_buffers_;
  ExternalFrameBuffer *ext_fb_list_;
};

#if CONFIG_WEBM_IO
const char kVP9TestFile[] = "vp90-2-02-size-lf-1920x1080.webm";

// Class for testing passing in external frame buffers to libvpx.
class ExternalFrameBufferPlanesTest : public ::testing::Test {
 protected:
  ExternalFrameBufferPlanesTest()
      : video_(NULL),
        decoder_(NULL),
        num_buffers_(0) {}

  virtual void SetUp() {
    video_ = new libvpx_test::WebMVideoSource(kVP9TestFile);
    ASSERT_TRUE(video_ != NULL);
    video_->Init();
    video_->Begin();

    vpx_codec_dec_cfg_t cfg = vpx_codec_dec_cfg_t();
    decoder_ = new libvpx_test::VP9Decoder(cfg, 0);
    ASSERT_TRUE(decoder_ != NULL);
  }

  // Passes the external frame buffer information to libvpx.
  vpx_codec_err_t SetFrameBufferPlanesFunctions(
      int num_buffers,
      vpx_get_frame_buffer_planes_cb_fn_t cb_get,
      vpx_release_frame_buffer_planes_cb_fn_t cb_release) {
    if (num_buffers > 0) {
      num_buffers_ = num_buffers;
      EXPECT_TRUE(fb_list_.CreateBufferList(num_buffers_));
    }
    return decoder_->SetFrameBufferPlanesFunctions(cb_get, cb_release, &fb_list_);
  }

  virtual void TearDown() {
    delete decoder_;
    delete video_;
  }

  vpx_codec_err_t DecodeOneFrame() {
    const vpx_codec_err_t res =
        decoder_->DecodeFrame(video_->cxdata(), video_->frame_size());
    CheckDecodedFrames();
    if (res == VPX_CODEC_OK)
      video_->Next();
    return res;
  }

  vpx_codec_err_t DecodeRemainingFrames() {
    for (; video_->cxdata() != NULL; video_->Next()) {
      const vpx_codec_err_t res =
          decoder_->DecodeFrame(video_->cxdata(), video_->frame_size());
      if (res != VPX_CODEC_OK)
        return res;
      CheckDecodedFrames();
    }
    return VPX_CODEC_OK;
  }

 protected:
  void CheckDecodedFrames() {
    libvpx_test::DxDataIterator dec_iter = decoder_->GetDxData();
    const vpx_image_t *img = NULL;

    // Get decompressed data
    while ((img = dec_iter.Next()) != NULL) {
      fb_list_.CheckXImageFrameBuffer(img);
    }
  }

  libvpx_test::WebMVideoSource *video_;
  libvpx_test::VP9Decoder *decoder_;
  int num_buffers_;
  ExternalFrameBufferList fb_list_;
};

int get_vp9_frame_buffer_planes(void *priv, size_t width, size_t height, vpx_img_fmt_t fmt, vpx_codec_frame_buffer_t *fb) {
  ExternalFrameBufferList *const fb_list =
      reinterpret_cast<ExternalFrameBufferList*>(priv);
  return fb_list->GetFreeFrameBuffer(fmt, width, height, fb);
}

int release_vp9_frame_buffer_planes(void *priv, vpx_codec_frame_buffer_t *fb) {
  ExternalFrameBufferList *const fb_list =
      reinterpret_cast<ExternalFrameBufferList*>(priv);
  return fb_list->ReturnFrameBuffer(fb);
}

#endif  // CONFIG_WEBM_IO


// Class for testing passing in external frame buffers to libvpx.
class ExternalFramePlanesMD5Test
    : public ::libvpx_test::DecoderTest,
      public ::libvpx_test::CodecTestWithParam<const char*> {
 protected:
  ExternalFramePlanesMD5Test()
      : DecoderTest(GET_PARAM(::libvpx_test::kCodecFactoryParam)),
        md5_file_(NULL),
        num_buffers_(0) {}

  virtual ~ExternalFramePlanesMD5Test() {
    if (md5_file_ != NULL)
      fclose(md5_file_);
  }

  virtual void PreDecodeFrameHook(
      const libvpx_test::CompressedVideoSource &video,
      libvpx_test::Decoder *decoder) {
    if (num_buffers_ > 0 && video.frame_number() == 0) {
      // Have libvpx use frame buffers we create.
      ASSERT_TRUE(fb_list_.CreateBufferList(num_buffers_));
      ASSERT_EQ(VPX_CODEC_OK,
                decoder->SetFrameBufferPlanesFunctions(
                    GetVP9FramePlanes, ReleaseVP9FramePlanes, this));
    }
  }

  void OpenMD5File(const std::string &md5_file_name_) {
    md5_file_ = libvpx_test::OpenTestDataFile(md5_file_name_);
    ASSERT_TRUE(md5_file_ != NULL) << "Md5 file open failed. Filename: "
        << md5_file_name_;
  }

  virtual void DecompressedFrameHook(const vpx_image_t &img,
                                     const unsigned int frame_number) {
    ASSERT_TRUE(md5_file_ != NULL);
    char expected_md5[33];
    char junk[128];

    // Read correct md5 checksums.
    const int res = fscanf(md5_file_, "%s  %s", expected_md5, junk);
    ASSERT_NE(EOF, res) << "Read md5 data failed";
    expected_md5[32] = '\0';

    ::libvpx_test::MD5 md5_res;
    md5_res.Add(&img);
    const char *const actual_md5 = md5_res.Get();

    // Check md5 match.
    ASSERT_STREQ(expected_md5, actual_md5)
        << "Md5 checksums don't match: frame number = " << frame_number;
  }

  // Callback to get a free external frame buffer. Return value < 0 is an
  // error.
  static int GetVP9FramePlanes(void *priv, size_t width,
			       size_t height, vpx_img_fmt_t fmt, vpx_codec_frame_buffer_t *fb) {
    ExternalFramePlanesMD5Test *const md5Test =
        reinterpret_cast<ExternalFramePlanesMD5Test*>(priv);
    return md5Test->fb_list_.GetFreeFrameBuffer(fmt, width, height, fb);
  }

  // Callback to release an external frame buffer. Return value < 0 is an
  // error.
  static int ReleaseVP9FramePlanes(void *priv,
                                   vpx_codec_frame_buffer_t *fb) {
    ExternalFramePlanesMD5Test *const md5Test =
        reinterpret_cast<ExternalFramePlanesMD5Test*>(priv);
    return md5Test->fb_list_.ReturnFrameBuffer(fb);
  }

  void set_num_buffers(int num_buffers) { num_buffers_ = num_buffers; }
  int num_buffers() const { return num_buffers_; }

 private:
  FILE *md5_file_;
  int num_buffers_;
  ExternalFrameBufferList fb_list_;
};


// This test runs through the set of test vectors, and decodes them.
// Libvpx will call into the application to allocate a frame buffer when
// needed. The md5 checksums are computed for each frame in the video file.
// If md5 checksums match the correct md5 data, then the test is passed.
// Otherwise, the test failed.
TEST_P(ExternalFramePlanesMD5Test, ExtFBMD5Match) {
  const std::string filename = GET_PARAM(kVideoNameParam);
  
  libvpx_test::CompressedVideoSource *video = NULL;
  fprintf(stderr, "%s", filename.c_str());
  // Number of buffers equals #VP9_MAXIMUM_REF_BUFFERS +
  // #VPX_MAXIMUM_WORK_BUFFERS + four jitter buffers.
  const int jitter_buffers = 4;
  const int num_buffers =
      VP9_MAXIMUM_REF_BUFFERS + VPX_MAXIMUM_WORK_BUFFERS + jitter_buffers;
  set_num_buffers(num_buffers);

#if CONFIG_VP8_DECODER
  // Tell compiler we are not using kVP8TestVectors.
  (void)libvpx_test::kVP8TestVectors;
#endif

  // Open compressed video file.
  if (filename.substr(filename.length() - 3, 3) == "ivf") {
    video = new libvpx_test::IVFVideoSource(filename);
  } else {
#if CONFIG_WEBM_IO
    video = new libvpx_test::WebMVideoSource(filename);
#else
    fprintf(stderr, "WebM IO is disabled, skipping test vector %s\n",
            filename.c_str());
    return;
#endif
  }
  ASSERT_TRUE(video != NULL);
  video->Init();

  // Construct md5 file name.
  const std::string md5_filename = filename + ".md5";
  OpenMD5File(md5_filename);

  // Decode frame, and check the md5 matching.
  ASSERT_NO_FATAL_FAILURE(RunLoop(video));
  delete video;
}

#ifdef CONFIG_WEBM_IO
TEST_F(ExternalFrameBufferPlanesTest, MinFrameBuffers) {
  const int num_buffers = VP9_MAXIMUM_REF_BUFFERS + VPX_MAXIMUM_WORK_BUFFERS;
  ASSERT_EQ(VPX_CODEC_OK,
            SetFrameBufferPlanesFunctions(num_buffers,
                get_vp9_frame_buffer_planes, release_vp9_frame_buffer_planes));
  ASSERT_EQ(VPX_CODEC_OK, DecodeRemainingFrames());
}

TEST_F(ExternalFrameBufferPlanesTest, EightJitterBuffers) {
  // Number of buffers equals #VP9_MAXIMUM_REF_BUFFERS +
  // #VPX_MAXIMUM_WORK_BUFFERS + eight jitter buffers.
  const int jitter_buffers = 8;
  const int num_buffers =
      VP9_MAXIMUM_REF_BUFFERS + VPX_MAXIMUM_WORK_BUFFERS + jitter_buffers;
  ASSERT_EQ(VPX_CODEC_OK,
            SetFrameBufferPlanesFunctions(
                num_buffers, get_vp9_frame_buffer_planes, release_vp9_frame_buffer_planes));
  ASSERT_EQ(VPX_CODEC_OK, DecodeRemainingFrames());
}

TEST_F(ExternalFrameBufferPlanesTest, NotEnoughBuffers) {
  // Minimum number of external frame buffers for VP9 is
  // #VP9_MAXIMUM_REF_BUFFERS + #VPX_MAXIMUM_WORK_BUFFERS. Most files will
  // only use 5 frame buffers at one time.
  const int num_buffers = 2;
  ASSERT_EQ(VPX_CODEC_OK,
            SetFrameBufferPlanesFunctions(
                num_buffers, get_vp9_frame_buffer_planes, release_vp9_frame_buffer_planes));
  ASSERT_EQ(VPX_CODEC_OK, DecodeOneFrame());
  ASSERT_EQ(VPX_CODEC_MEM_ERROR, DecodeRemainingFrames());
}
#endif  // CONFIG_WEBM_IO

VP9_INSTANTIATE_TEST_CASE(ExternalFramePlanesMD5Test,
                          ::testing::ValuesIn(libvpx_test::kVP9TestVectors,
                                              libvpx_test::kVP9TestVectors +
                                              libvpx_test::kNumVP9TestVectors));

}  // namespace
