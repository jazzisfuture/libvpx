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

#ifdef _MSC_VER
#include <malloc.h>
#endif  // _MSC_VER
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
    Buffer<T>::Init();
  }

 protected:
  virtual T *AllocRawBuffer(size_t size) {
#ifdef _MSC_VER
    return reinterpret_cast<T *>(_aligned_malloc(size, alignment_bits_));
#else
    return reinterpret_cast<T *>(aligned_alloc(alignment_bits_, size));
#endif  // _MSC_VER
  }

  virtual void FreeRawBuffer(T *buffer) {
#ifdef _MSC_VER
    _aligned_free(buffer);
#else
    free(buffer);
#endif  // _MSC_VER
  }

  const int alignment_bits_;
};

}  // namespace libvpx_test
#endif  // TEST_DSPBUFFER_H_
