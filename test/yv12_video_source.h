/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_YV12_VIDEO_SOURCE_H_
#define TEST_YV12_VIDEO_SOURCE_H_
#include "test/video_source.h"
#include <stdio.h>

namespace libvpx_test {

// This class extends VideoSource to allow parsing of raw yv12
// so that we can do actual file encodes.
class YV12VideoSource : public VideoSource {
 public:
  YV12VideoSource(std::string file_name, unsigned int width,
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

  ~YV12VideoSource() {
    vpx_img_free(img_);
    fclose(input_file_);
  }

  virtual int Begin() {
    // These calls will only work on non multiple gig files.
    input_file_ = fopen(file_name_.c_str(), "rb");

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
      raw_sz_ = ((width + 31) & ~31) * height * 3 / 2;
      img_ = vpx_img_alloc(NULL, VPX_IMG_FMT_VPXI420, width, height, 32);
      width_ = width;
      height_ = height;
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
