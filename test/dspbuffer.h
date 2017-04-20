/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_DSPBUFFER_H_
#define TEST_DSPBUFFER_H_

#ifdef _WIN32
#include <malloc.h>
#endif  // _WIN32
#include <stdlib.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "test/buffer.h"
#include "vpx/vpx_integer.h"

namespace libvpx_test {

template <typename T>
class DSPBuffer : public Buffer<T> {
 public:
  DSPBuffer(int width, int height, int padding, int alignment_bits)
      : Buffer<T>(width, height, padding), alignment_bits_(alignment_bits) {
    delete[] Buffer<T>::raw_buffer_;
    Init();
  }

  virtual ~DSPBuffer() {
    T *buffer = Buffer<T>::raw_buffer_;
#ifdef _WIN32
    _aligned_free(buffer);
#else
    free(buffer);
#endif  // _WIN32
    Buffer<T>::raw_buffer_ = NULL;
  }

 protected:
  void Init() {
    const int type_size = sizeof(T);
    const int stride = Buffer<T>::stride_;
    const int width = Buffer<T>::width_;
    const int left_padding = Buffer<T>::left_padding_;
    // Ensure alignment of the first value will be preserved.
    ASSERT_EQ((left_padding * type_size) % alignment_bits_, 0);
    // Ensure alignment of the subsequent rows will be preserved when there is a
    // stride.
    if (stride != width) {
      ASSERT_EQ((stride * type_size) % alignment_bits_, 0);
    }
    const size_t size = Buffer<T>::raw_size_ * sizeof(T);
    T *buffer;
#ifdef _WIN32
    buffer = reinterpret_cast<T *>(_aligned_malloc(size, alignment_bits_));
#else
    buffer = reinterpret_cast<T *>(aligned_alloc(alignment_bits_, size));
#endif  // _WIN32
    ASSERT_TRUE(buffer != NULL);
    Buffer<T>::raw_buffer_ = buffer;
  }

  const int alignment_bits_;
};

}  // namespace libvpx_test
#endif  // TEST_DSPBUFFER_H_
