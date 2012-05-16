/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_VIDEO_SOURCE_H_
#define TEST_VIDEO_SOURCE_H_
#include "vpx/vpx_encoder.h"

class VideoSource {
public:
  ~VideoSource() {}
  virtual void begin() = 0;
  virtual void next() = 0;
  virtual vpx_image_t *img() = 0;
  virtual vpx_codec_pts_t pts() = 0;
  virtual unsigned long duration() = 0;
  virtual vpx_rational_t timebase() = 0;
  virtual unsigned int frame() = 0;
};


class DummyVideoSource : public VideoSource {
public:
  DummyVideoSource() {
    img_ = NULL;
    set_size(80, 64);
    limit_ = 100;
  }

  ~DummyVideoSource() {
    vpx_img_free(img_);
  }

  virtual void begin() {
    frame_ = 0;
    FillFrame();
  }

  virtual void next() {
    ++frame_;
    FillFrame();
  }

  virtual vpx_image_t *img() {
    return frame_ < limit_ ? img_ : NULL;
  }

  virtual vpx_codec_pts_t pts() {
    return frame_;
  }

  virtual unsigned long duration() {
    return 1;
  }

  virtual vpx_rational_t timebase() {
    vpx_rational_t t = {1, 30};
    return t;
  }

  virtual unsigned int frame() {
    return frame_;
  }

  void set_size(unsigned int width, unsigned int height) {
    vpx_img_free(img_);
    raw_sz_ = ((width + 31)&~31) * height * 3 / 2;
    img_ = vpx_img_alloc(NULL, VPX_IMG_FMT_VPXI420, width, height, 32);
  }

protected:
  virtual void FillFrame() {
    memset(img_->img_data, 0, raw_sz_);
  }

  vpx_image_t *img_;
  size_t       raw_sz_;
  unsigned int limit_;
  unsigned int frame_;
};


class RandomVideoSource : public DummyVideoSource {
protected:
  virtual void FillFrame() {
    if (frame_ % 30 < 15)
      for (size_t i = 0; i < raw_sz_; ++i)
        img_->img_data[i] = rand();
    else
      memset(img_->img_data, 0, raw_sz_);
  }
};

#endif
