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
  Buffer(int x, int y, int p) : x_(x), y_(y), p_(p) {
    assert(x_ > 0);
    assert(y_ > 0);
    assert(p_ >= 0);
    stride_ = x_ + 2 * p_;
    raw_size_ = stride_ * (y_ + 2 * p_);
    b_ = new T[raw_size_];
    SetPadding();
  }

  // Return the address of the top left pixel.
  T *Src() const { return b_ ? b_ + (p_ * stride_) + p_ : NULL; }

  int Stride() const { return stride_; }

  // Set the buffer (excluding padding) to a.
  void Set(int a);

  // Set the buffer (excluding padding) to the output of ACMRandom function b.
  void Set(ACMRandom *a, T (ACMRandom::*b)());

  // Set the buffer (excluding pattern) to the contents of Buffer a.
  void Set(const Buffer<T> &a);

  void DumpBuffer() const;

  // Highlight the differences between two buffers.
  void PrintDifference(const Buffer<T> &a) const;

  void SetPadding() { SetPadding(std::numeric_limits<T>::max()); }

  // Sets all the values in the padding to 'a'.
  void SetPadding(int a);

  // Returns 0 if all the values (excluding padding) are equal to 'a'.
  int CheckValues(int a) const;

  // Returns 0 when padding matches the expected value or there is no padding.
  int CheckPadding() const;

  // Compare the non-padding portion of two buffers.
  int CompareBuffer(const Buffer<T> &a) const;

  ~Buffer() {
    if (b_) {
      delete[] b_;
    }
    b_ = NULL;
  }

 private:
  int x_;         // Columns or width.
  int y_;         // Rows or height.
  int p_;         // Padding on each side.
  int p_v_;       // Value stored in padding.
  int stride_;    // Padding + width + padding.
  int raw_size_;  // The total number of bytes including padding.
  T *b_;          // The actual buffer.
};

template <typename T>
void Buffer<T>::Set(int a) {
  if (p_) {
    T *src = Src();
    for (int i = 0; i < y_; ++i) {
      memset(src, a, sizeof(T) * x_);
      src += stride_;
    }
  } else {  // No border.
    memset(b_, a, sizeof(T) * raw_size_);
  }
}

template <typename T>
void Buffer<T>::Set(ACMRandom *a, T (ACMRandom::*b)()) {
  T *src = Src();
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < x_; ++x) {
      src[x + y * stride_] = (*a.*b)();
    }
  }
}

template <typename T>
void Buffer<T>::Set(const Buffer<T> &a) {
  if ((a.x_ != this->x_) || (a.y_ != this->y_)) {
    printf(
        "Unable to copy buffer. Reference buffer of size %dx%d does not match "
        "this buffer which is size %dx%d\n",
        a.x_, a.y_, this->x_, this->y_);
    return;
  }

  T *a_src = a.Src();
  T *b_src = this->Src();
  for (int y = 0; y < y_; y++) {
    for (int x = 0; x < x_; x++) {
      b_src[x + y * a.Stride()] = a_src[x + y * this->Stride()];
    }
  }
}

template <typename T>
void Buffer<T>::DumpBuffer() const {
  for (int y = 0; y < y_ + 2 * p_; ++y) {
    for (int x = 0; x < stride_; ++x) {
      printf("%4d", b_[x + y * stride_]);
    }
    printf("\n");
  }
}

template <typename T>
void Buffer<T>::PrintDifference(const Buffer<T> &a) const {
  if ((a.x_ != this->x_) || (a.y_ != this->y_)) {
    printf(
        "Unable to print buffers. Reference buffer of size %dx%d does not "
        "match this buffer which is size %dx%d\n",
        a.x_, a.y_, this->x_, this->y_);
    return;
  }

  T *a_src = a.Src();
  T *b_src = Src();

  printf("This buffer:\n");
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < x_; ++x) {
      if (a_src[x + y * a.Stride()] != b_src[x + y * Stride()]) {
        printf("*%3d", b_src[x + y * Stride()]);
      } else {
        printf("%4d", b_src[x + y * Stride()]);
      }
    }
    printf("\n");
  }

  printf("Reference buffer:\n");
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < x_; ++x) {
      if (a_src[x + y * a.Stride()] != b_src[x + y * Stride()]) {
        printf("*%3d", a_src[x + y * a.Stride()]);
      } else {
        printf("%4d", a_src[x + y * a.Stride()]);
      }
    }
    printf("\n");
  }
}

template <typename T>
void Buffer<T>::SetPadding(int a) {
  if (0 == p_) {
    return;
  }

  p_v_ = a;

  // Top padding.
  memset(b_, p_v_, stride_ * p_);

  // Left padding.
  T *left = Src() - p_;
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < p_; ++x) {
      left[x] = p_v_;
    }
    left += stride_;
  }

  // Right padding.
  T *right = Src() + x_;
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < p_; ++x) {
      right[x] = p_v_;
    }
    right += stride_;
  }

  // Bottom padding
  T *bottom = b_ + (p_ + y_) * stride_;
  memset(bottom, p_v_, stride_ * p_);
}

template <typename T>
int Buffer<T>::CheckValues(int a) const {
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < x_; ++x) {
      if (a != *(Src() + x + y * stride_)) {
        printf("Buffer does not match %d.\n", a);
        DumpBuffer();
        return 1;
      }
    }
  }
  return 0;
}

template <typename T>
int Buffer<T>::CheckPadding() const {
  if (0 == p_) {
    return 0;
  }

  int error = 0;
  // Top padding.
  T *top = b_;
  for (int i = 0; i < stride_ * p_; ++i) {
    if (p_v_ != top[i]) {
      error = 1;
    }
  }

  // Left padding.
  T *left = Src() - p_;
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < p_; ++x) {
      if (p_v_ != left[x]) {
        error = 1;
      }
    }
    left += stride_;
  }

  // Right padding.
  T *right = Src() + x_;
  for (int y = 0; y < y_; ++y) {
    for (int x = 0; x < p_; ++x) {
      if (p_v_ != right[x]) {
        error = 1;
      }
    }
    right += stride_;
  }

  // Bottom padding
  T *bottom = b_ + (p_ + y_) * stride_;
  for (int i = 0; i < stride_ * p_; ++i) {
    if (p_v_ != bottom[i]) {
      error = 1;
    }
  }

  if (error) {
    printf("Padding does not match %d.\n", p_v_);
    DumpBuffer();
  }

  return error;
}

template <typename T>
int Buffer<T>::CompareBuffer(const Buffer<T> &a) const {
  if ((a.x_ != this->x_) || (a.y_ != this->y_)) {
    printf(
        "Unable to compare buffers. Reference buffer of size %dx%d does not "
        "match this buffer which is size %dx%d\n",
        a.x_, a.y_, this->x_, this->y_);
    return 1;
  }

  T *a_src = a.Src();
  T *b_src = this->Src();
  for (int y = 0; y < y_; y++) {
    for (int x = 0; x < x_; x++) {
      if (a_src[x + y * a.Stride()] != b_src[x + y * this->Stride()]) {
        printf("Buffers do not match\n");
        PrintDifference(a);
        return 1;
      }
    }
  }
  return 0;
}
}  // namespace libvpx_test
#endif  // TEST_BUFFER_H_
