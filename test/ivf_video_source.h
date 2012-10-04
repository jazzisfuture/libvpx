/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_IVF_VIDEO_SOURCE_H_
#define TEST_IVF_VIDEO_SOURCE_H_
#include <cstdio>
#include <cstdlib>
#include <string>
#include "test/video_source.h"

namespace libvpx_test {
const unsigned int kCodeBufferSize = 256 * 1024;

// This class extends VideoSource to allow parsing of ivf files,
// so that we can do actual file decodes. The compressed frame
// buffer is allocated outside the class, which allows us to
// allocate only once for all test files.
class IVFVideoSource : public CompressedVideoSource {
 public:
  IVFVideoSource(const std::string &file_name, uint8_t *frame_buf)
      : file_name_(file_name),
        input_file_(NULL),
        compressed_frame_buf_(frame_buf),
        frame_sz_(0),
        frame_(0),
        end_of_file_(false) {
  }

  virtual ~IVFVideoSource() {
    if (input_file_)
      fclose(input_file_);
  }

  virtual void Begin() {
    input_file_ = OpenTestDataFile(file_name_);
    ASSERT_TRUE(input_file_) << "Input file open failed. Filename: "
        << file_name_;

    // Read file header
    uint8_t file_hdr[kIvfFileHdrSize];
    ASSERT_EQ(kIvfFileHdrSize, fread(file_hdr, 1, kIvfFileHdrSize, input_file_))
        << "File header read failed.";
    // Check file header
    ASSERT_TRUE(file_hdr[0] == 'D' && file_hdr[1] == 'K' && file_hdr[2] == 'I'
        && file_hdr[3] == 'F') << "Input is not an IVF file.";

    FillFrame();
  }

  virtual void Next() {
    ++frame_;
    FillFrame();
  }

  void FillFrame() {
    uint8_t frame_hdr[kIvfFrameHdrSize];
    // Check frame header and read a frame from input_file.
    if (fread(frame_hdr, 1, kIvfFrameHdrSize, input_file_)
        != kIvfFrameHdrSize) {
      end_of_file_ = true;
    } else {
      end_of_file_ = false;

      frame_sz_ = MemGetLe32(frame_hdr);
      ASSERT_LE(frame_sz_, kCodeBufferSize)
          << "Frame is too big for allocated code buffer";
      ASSERT_EQ(frame_sz_,
          fread(compressed_frame_buf_, 1, frame_sz_, input_file_))
          << "Failed to read complete frame";
    }
  }

  virtual const uint8_t *cxdata() const {
    return end_of_file_ ? NULL : compressed_frame_buf_;
  }
  virtual const unsigned int frame_size() const { return frame_sz_; }

 protected:
  std::string file_name_;
  FILE *input_file_;
  uint8_t *compressed_frame_buf_;
  unsigned int frame_sz_;
  unsigned int frame_;
  bool end_of_file_;

 private:
  static unsigned int MemGetLe32(const uint8_t *mem) {
    return (mem[3] << 24) | (mem[2] << 16) | (mem[1] << 8) | (mem[0]);
  }

  static const unsigned int kIvfFileHdrSize;
  static const unsigned int kIvfFrameHdrSize;
};

const unsigned int IVFVideoSource::kIvfFileHdrSize = 32;
const unsigned int IVFVideoSource::kIvfFrameHdrSize = 12;
}  // namespace libvpx_test

#endif  // TEST_IVF_VIDEO_SOURCE_H_
