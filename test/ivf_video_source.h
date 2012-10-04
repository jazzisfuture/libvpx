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
#include "test/video_source.h"

namespace libvpx_test {

#define IVF_FILE_HDR_SZ  (32)
#define IVF_FRAME_HDR_SZ (12)
#define CODE_BUFFER_SIZE (256*1024)

static unsigned int mem_get_le32(const unsigned char *mem) {
  return (mem[3] << 24)|(mem[2] << 16)|(mem[1] << 8)|(mem[0]);
}

// This class extends VideoSource to allow parsing of ivf files,
// so that we can do actual file decodes.
class IVFVideoSource : public CompressedVideoSource {
 public:
  IVFVideoSource(const std::string &file_name, unsigned char *frame_buf)
  : file_name_(file_name),
    compressed_frame_buf_(frame_buf),
  frame_(0),
  end_of_file_(false) {
  }

  virtual ~IVFVideoSource() {
    if (input_file_)
    fclose(input_file_);
  }

  virtual void Begin() {
    OpenTestDataFile(file_name_, &input_file_);

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

  void FillFrame() {
    // Read a frame from input_file.
    if (!(fread(frame_hdr, 1, IVF_FRAME_HDR_SZ, input_file_)
        == IVF_FRAME_HDR_SZ)) {
      end_of_file_ = true;
    } else {
      end_of_file_ = false;

    frame_sz = mem_get_le32(frame_hdr);
    ASSERT_TRUE(frame_sz <= CODE_BUFFER_SIZE)
        << "Frame is too big for allocated code buffer";
    ASSERT_TRUE(fread(compressed_frame_buf_, 1, frame_sz, input_file_) == frame_sz)
        << "Failed to read complete frame";
    }
  }

  virtual unsigned char *cxdata() {return (end_of_file_) ? NULL : compressed_frame_buf_;}
  virtual unsigned int frame_size() const {return frame_sz;}

 protected:
  std::string file_name_;
  FILE *input_file_;
  unsigned char *compressed_frame_buf_;
  unsigned int frame_sz;
  unsigned int frame_;
  bool end_of_file_;
  unsigned char file_hdr[IVF_FILE_HDR_SZ];
  unsigned char frame_hdr[IVF_FRAME_HDR_SZ];
};

}  // namespace libvpx_test

#endif  // TEST_IVF_VIDEO_SOURCE_H_
