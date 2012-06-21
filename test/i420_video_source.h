/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_I420_VIDEO_SOURCE_H_
#define TEST_I420_VIDEO_SOURCE_H_
#include "test/video_source.h"
#include <stdio.h>

#if defined(_MSC_VER)
/* MSVS doesn't define off_t, and uses _f{seek,tell}i64 */
typedef __int64 off_t;
#define fseeko _fseeki64
#define ftello _ftelli64
#elif defined(_WIN32)
/* MinGW defines off_t as long
   and uses f{seek,tell}o64/off64_t for large files */
#define fseeko fseeko64
#define ftello ftello64
#define off_t off64_t
#endif

#define LITERALU64(hi,lo) ((((uint64_t)hi)<<32)|lo)

/* We should use 32-bit file operations in WebM file format
 * when building ARM executable file (.axf) with RVCT */
#if !CONFIG_OS_SUPPORT
typedef long off_t;
#define fseeko fseek
#define ftello ftell
#endif

namespace libvpx_test {

// This class extends VideoSource to allow parsing of raw yv12
// so that we can do actual file encodes.
class I420VideoSource : public VideoSource {
 public:
  I420VideoSource(std::string file_name, unsigned int width,
                  unsigned int height, int rate_numerator, int rate_denominator,
                  unsigned int start = 0, int limit = 1000)
      : file_name_(file_name),
        img_(NULL),
        start_(start),
        limit_(limit),
        framerate_numerator_(rate_numerator),
        framerate_denominator_(rate_denominator) {
    SetSize(width, height);
  }
  ~I420VideoSource() {
    vpx_img_free(img_);
    if (input_file_) fclose(input_file_);
  }

  virtual int Begin() {

    std::string path_to_source = file_name_;

    // These calls will only work on non multiple gig files.
    if (getenv("LIBVPX_TEST_DATA_PATH")) {
      path_to_source = getenv("LIBVPX_TEST_DATA_PATH");
      path_to_source += file_name_;
    }

    input_file_ = fopen(path_to_source.c_str(), "rb");

    if(!input_file_)
      return -1;

    if (start_) {
      fseek(input_file_, raw_sz_ * start_, SEEK_SET);
    }

    frame_ = start_;
    FillFrame();
    return 0;
  }

  virtual void Next() {
    ++frame_;
    FillFrame();
  }

  virtual vpx_image_t *img() const {
    return (frame_ < limit_) ? img_ : NULL;
  }

  // Models a stream where Timebase = 1/FPS, so pts == frame.
  virtual vpx_codec_pts_t pts() const {
    return frame_;
  }

  virtual unsigned long duration() const {
    return 1;
  }

  virtual vpx_rational_t timebase() const {
    const vpx_rational_t t = { framerate_denominator_, framerate_numerator_ };
    return t;
  }

  virtual unsigned int frame() const {
    return frame_;
  }

  void SetSize(unsigned int width, unsigned int height) {
    if (width != width_ || height != height_) {
      vpx_img_free(img_);
      img_ = vpx_img_alloc(NULL, VPX_IMG_FMT_VPXI420, width, height, 1);
      width_ = width;
      height_ = height;
      raw_sz_ = width * height * 3 / 2;
    }
  }
  virtual void FillFrame() {

    // Read a frame from input_file.
    if (fread(img_->img_data, raw_sz_, 1, input_file_) == 0) {
      limit_ = frame_;
    }
  }

 protected:

  std::string file_name_;
  FILE *input_file_;
  vpx_image_t *img_;
  size_t raw_sz_;
  unsigned int start_;
  unsigned int limit_;
  unsigned int frame_;
  unsigned int width_;
  unsigned int height_;
  unsigned int framerate_numerator_;
  unsigned int framerate_denominator_;
};

}  // namespace libvpx_test

#endif
