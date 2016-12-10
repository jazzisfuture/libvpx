/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_BUFFER_H_
#define TEST_BUFFER_H_

#include <limits>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "test/acm_random.h"
#include "vpx/vpx_integer.h"

namespace libvpx_test {

template <typename T>
class Buffer {
 public:
  Buffer(int width, int height, int top_padding, int left_padding,
         int right_padding, int bottom_padding)
      : width_(width), height_(height), top_padding_(top_padding),
        left_padding_(left_padding), right_padding_(right_padding),
        bottom_padding_(bottom_padding) {
    assert(width_ > 0);
    assert(height_ > 0);
    assert(top_padding_ >= 0);
    assert(left_padding_ >= 0);
    assert(right_padding_ >= 0);
    assert(bottom_padding_ >= 0);
    stride_ = left_padding_ + width_ + right_padding_;
    raw_size_ = stride_ * (top_padding_ + height_ + bottom_padding_);
    raw_buffer_ = new T[raw_size_];
    SetPadding(std::numeric_limits<T>::max());
  }

  Buffer(int width, int height, int padding)
      : width_(width), height_(height), top_padding_(padding),
        left_padding_(padding), right_padding_(padding),
        bottom_padding_(padding) {
    assert(width_ > 0);
    assert(height_ > 0);
    assert(top_padding_ >= 0);
    stride_ = left_padding_ + width_ + right_padding_;
    raw_size_ = stride_ * (top_padding_ + height_ + bottom_padding_);
    raw_buffer_ = new T[raw_size_];
    SetPadding(std::numeric_limits<T>::max());
  }

  T *TopLeftPixel() const;

  int Stride() const { return stride_; }

  // Set the buffer (excluding padding) to 'value'.
  void Set(const int value);

  // Set the buffer (excluding padding) to the output of ACMRandom function b.
  void Set(ACMRandom *a, T (ACMRandom::*b)());

  // Set the buffer (excluding pattern) to the contents of Buffer a.
  void Set(const Buffer<T> &a);

  void DumpBuffer() const;

  // Highlight the differences between two buffers.
  void PrintDifference(const Buffer<T> &a) const;

  bool HasPadding() const;

  // Sets all the values in the buffer to 'padding_value'.
  void SetPadding(const int padding_value);

  // Checks if all the values (excluding padding) are equal to 'value'.
  bool CheckValues(const int value) const;

  // Check that padding matches the expected value or there is no padding.
  bool CheckPadding() const;

  // Compare the non-padding portion of two buffers.
  bool CheckValues(const Buffer<T> &a) const;

  ~Buffer() {
    if (raw_buffer_) {
      delete[] raw_buffer_;
    }
    raw_buffer_ = NULL;
  }

 private:
  const int width_;
  const int height_;
  const int top_padding_;
  const int left_padding_;
  const int right_padding_;
  const int bottom_padding_;
  int padding_value_;
  int stride_;
  int raw_size_;
  T *raw_buffer_;
};

template <typename T>
T *Buffer<T>::TopLeftPixel() const {
  return raw_buffer_ ? raw_buffer_ + (top_padding_ * Stride()) + left_padding_
                     : NULL;
}

template <typename T>
void Buffer<T>::Set(const int value) {
  if (HasPadding()) {
    T *src = TopLeftPixel();
    for (int i = 0; i < height_; ++i) {
      memset(src, value, sizeof(T) * width_);
      src += Stride();
    }
  } else {
    memset(raw_buffer_, value, sizeof(T) * raw_size_);
  }
}

template <typename T>
void Buffer<T>::Set(ACMRandom *RandClass, T (ACMRandom::*RandFunc)()) {
  T *src = TopLeftPixel();
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < width_; ++width) {
      src[width] = (*RandClass.*RandFunc)();
    }
    src += Stride();
  }
}

template <typename T>
void Buffer<T>::Set(const Buffer<T> &SourceBuffer) {
  if ((SourceBuffer.width_ != this->width_) ||
      (SourceBuffer.width_ != this->width_)) {
    printf(
        "Unable to copy buffer. Reference buffer of size %dx%d does not match "
        "this buffer which is size %dx%d\n",
        SourceBuffer.width_, SourceBuffer.width_, this->width_, this->width_);
    return;
  }

  T *a_src = SourceBuffer.TopLeftPixel();
  T *b_src = this->TopLeftPixel();
  for (int height = 0; height < height_; height++) {
    for (int width = 0; width < width_; width++) {
      b_src[width] = a_src[width];
    }
    a_src += SourceBuffer.Stride();
    b_src += this->Stride();
  }
}

template <typename T>
void Buffer<T>::DumpBuffer() const {
  for (int height = 0; height < height_ + top_padding_ + bottom_padding_;
       ++height) {
    for (int width = 0; width < Stride(); ++width) {
      printf("%4d", raw_buffer_[height + width * Stride()]);
    }
    printf("\n");
  }
}

template <typename T>
bool Buffer<T>::HasPadding() const {
  return top_padding_ || left_padding_ || right_padding_ || bottom_padding_;
}

template <typename T>
void Buffer<T>::PrintDifference(const Buffer<T> &RefBuffer) const {
  if ((RefBuffer.width_ != this->width_) ||
      (RefBuffer.width_ != this->width_)) {
    printf(
        "Unable to print buffers. Reference buffer of size %dx%d does not "
        "match this buffer which is size %dx%d\n",
        RefBuffer.width_, RefBuffer.width_, this->width_, this->width_);
    return;
  }

  T *a_src = RefBuffer.TopLeftPixel();
  T *b_src = TopLeftPixel();

  printf("This buffer:\n");
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < width_; ++width) {
      if (a_src[width] != b_src[width]) {
        printf("*%3d", b_src[width]);
      } else {
        printf("%4d", b_src[width]);
      }
    }
    printf("\n");
    a_src += RefBuffer.Stride();
    b_src += this->Stride();
  }

  a_src = RefBuffer.TopLeftPixel();
  b_src = TopLeftPixel();

  printf("Reference buffer:\n");
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < width_; ++width) {
      if (a_src[width] != b_src[width]) {
        printf("*%3d", a_src[width]);
      } else {
        printf("%4d", a_src[width]);
      }
    }
    printf("\n");
    a_src += RefBuffer.Stride();
    b_src += this->Stride();
  }
}

template <typename T>
void Buffer<T>::SetPadding(const int padding_value) {
  if (!HasPadding()) {
    return;
  }

  padding_value_ = padding_value;
  memset(raw_buffer_, padding_value_, sizeof(T) * raw_size_);
}

template <typename T>
bool Buffer<T>::CheckValues(const int value) const {
  T *src = TopLeftPixel();
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < width_; ++width) {
      if (value != src[width]) {
        fprintf(stderr, "Buffer does not match %d.\n", value);
        DumpBuffer();
        return false;
      }
    }
    src += Stride();
  }
  return true;
}

template <typename T>
bool Buffer<T>::CheckPadding() const {
  if (!HasPadding()) {
    return true;
  }

  // Top padding.
  T *top = raw_buffer_;
  for (int i = 0; i < Stride() * top_padding_; ++i) {
    if (padding_value_ != top[i]) {
      return false;
    }
  }

  // Left padding.
  T *left = TopLeftPixel() - left_padding_;
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < left_padding_; ++width) {
      if (padding_value_ != left[width]) {
        return false;
      }
    }
    left += Stride();
  }

  // Right padding.
  T *right = TopLeftPixel() + width_;
  for (int height = 0; height < height_; ++height) {
    for (int width = 0; width < right_padding_; ++width) {
      if (padding_value_ != right[width]) {
        return false;
      }
    }
    right += Stride();
  }

  // Bottom padding
  T *bottom = raw_buffer_ + (top_padding_ + height_) * Stride();
  for (int i = 0; i < Stride() * bottom_padding_; ++i) {
    if (padding_value_ != bottom[i]) {
      return false;
    }
  }

  return true;
}

template <typename T>
bool Buffer<T>::CheckValues(const Buffer<T> &RefBuffer) const {
  if ((RefBuffer.width_ != this->width_) ||
      (RefBuffer.width_ != this->width_)) {
    printf(
        "Unable to compare buffers. Reference buffer of size %dx%d does not "
        "match this buffer which is size %dx%d\n",
        RefBuffer.width_, RefBuffer.width_, this->width_, this->width_);
    return false;
  }

  T *a_src = RefBuffer.TopLeftPixel();
  T *b_src = this->TopLeftPixel();
  for (int height = 0; height < height_; height++) {
    for (int width = 0; width < width_; width++) {
      if (a_src[width] != b_src[width]) {
        printf("Buffers do not match\n");
        PrintDifference(RefBuffer);
        return false;
      }
    }
    a_src += RefBuffer.Stride();
    b_src += this->Stride();
  }
  return true;
}
}  // namespace libvpx_test
#endif  // TEST_BUFFER_H_
