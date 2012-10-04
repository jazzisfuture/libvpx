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
#include <cstdio>
#include <cstdlib>

#include "test/video_source.h"
extern "C" {
#include "vpx_mem/vpx_mem.h"
}

namespace libvpx_test {

// This class extends VideoSource to allow parsing of raw yv12
// so that we can do actual file encodes.
class I420VideoSource : public VideoSource {
 public:
  I420VideoSource(const std::string &file_name,
                  unsigned int width, unsigned int height,
                  int rate_numerator, int rate_denominator,
                  unsigned int start, int limit)
      : file_name_(file_name),
        img_(NULL),
        start_(start),
        limit_(limit),
        frame_(0),
        width_(0),
        height_(0),
        framerate_numerator_(rate_numerator),
        framerate_denominator_(rate_denominator) {

    // This initializes raw_sz_, width_, height_ and allocates an img.
    SetSize(width, height);
  }

  virtual ~I420VideoSource() {
    vpx_img_free(img_);
    if (input_file_)
      fclose(input_file_);
  }

  virtual void Begin() {
    std::string path_to_source = file_name_;
    const char *kDataPath = getenv("LIBVPX_TEST_DATA_PATH");
    if (kDataPath) {
      path_to_source = kDataPath;
      path_to_source += "/";
      path_to_source += file_name_;
    }

    input_file_ = fopen(path_to_source.c_str(), "rb");
    ASSERT_TRUE(input_file_) << "File open failed.";

    if (start_) {
      fseek(input_file_, raw_sz_ * start_, SEEK_SET);
    }

    frame_ = start_;
    FillFrame();
  }

  virtual void Next() {
    ++frame_;
    FillFrame();
  }

  virtual vpx_image_t *img() const { return (frame_ < limit_) ? img_ : NULL;  }

  // Models a stream where Timebase = 1/FPS, so pts == frame.
  virtual vpx_codec_pts_t pts() const { return frame_; }

  virtual unsigned long duration() const { return 1; }

  virtual vpx_rational_t timebase() const {
    const vpx_rational_t t = { framerate_denominator_, framerate_numerator_ };
    return t;
  }

  virtual unsigned int frame() const { return frame_; }

  void SetSize(unsigned int width, unsigned int height) {
    if (width != width_ || height != height_) {
      vpx_img_free(img_);
      img_ = vpx_img_alloc(NULL, VPX_IMG_FMT_VPXI420, width, height, 1);
      ASSERT_TRUE(img_ != NULL);
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

#define IVF_FILE_HDR_SZ  (32)
#define IVF_FRAME_HDR_SZ (12)
#define CODE_BUFFER_SIZE (256*1024)

static unsigned int mem_get_le32(const unsigned char *mem) {
  return (mem[3] << 24)|(mem[2] << 16)|(mem[1] << 8)|(mem[0]);
}

// This class extends VideoSource to allow parsing of ivf files,
// so that we can do actual file decodes.
class IVFVideoSource : public DecoderVideoSource {
 public:
  IVFVideoSource(const std::string &file_name, unsigned int frame,
                 bool end_of_file)
  : file_name_(file_name),
  frame_(frame),
  end_of_file_(end_of_file) {
    // allocates a frame_buffer.
    AllocCodeBuffer();
  }

  virtual ~IVFVideoSource() {
    vpx_free(frame_buf_);
    if (input_file_)
    fclose(input_file_);
  }

  virtual void Begin() {
    std::string path_to_source = file_name_;
    const char *kDataPath = getenv("LIBVPX_TEST_DATA_PATH");
    if (kDataPath) {
      path_to_source = kDataPath;
      path_to_source += "/";
      path_to_source += file_name_;
    }

    input_file_ = fopen(path_to_source.c_str(), "rb");
    ASSERT_TRUE(input_file_) << "Input file open failed.";

    // Read file header
    ASSERT_TRUE(fread(file_hdr, 1, IVF_FILE_HDR_SZ, input_file_)
        == IVF_FILE_HDR_SZ && file_hdr[0] == 'D' && file_hdr[1] == 'K'
        && file_hdr[2] == 'I' && file_hdr[3] == 'F')
        << "Input is not an IVF file.";

    FillFrame();
  }

  virtual void Next() {
    ++frame_;
    FillFrame();
  }

  void AllocCodeBuffer() {
    frame_buf_ = (unsigned char *)vpx_calloc(CODE_BUFFER_SIZE,
                                             sizeof(unsigned char));
    ASSERT_TRUE(frame_buf_) << "Allocate failed";
  }

  virtual void FillFrame() {
    // Read a frame from input_file.
    if (!(fread(frame_hdr, 1, IVF_FRAME_HDR_SZ, input_file_)
        == IVF_FRAME_HDR_SZ)) {
      end_of_file_ = true;
    } else {
      end_of_file_ = false;

    frame_sz = mem_get_le32(frame_hdr);
    ASSERT_TRUE(frame_sz <= CODE_BUFFER_SIZE)
        << "Frame is too big for allocated code buffer";
    ASSERT_TRUE(fread(frame_buf_, 1, frame_sz, input_file_) == frame_sz)
        << "Failed to read complete frame";
    }
  }

  virtual unsigned char *cxdata() {return (end_of_file_) ? NULL : frame_buf_;}
  virtual unsigned int GetFrameSize() const {return frame_sz;}

 protected:
  std::string file_name_;
  FILE *input_file_;
  unsigned char *frame_buf_;
  unsigned int frame_sz;
  unsigned int frame_;
  bool end_of_file_;
  unsigned char file_hdr[IVF_FILE_HDR_SZ];
  unsigned char frame_hdr[IVF_FRAME_HDR_SZ];
};

}  // namespace libvpx_test

#endif  // TEST_I420_VIDEO_SOURCE_H_
