/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_ALIGNED_H_
#define TEST_ALIGNED_H_

#include <cassert>
#include <new>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "vpx_dsp/vpx_dsp_common.h"

namespace libvpx_test {

template<int A, typename T>
static T* align(void *addr) {
  GTEST_COMPILE_ASSERT_(IS_POWER_OF_TWO(A), alignment_must_be_power_of_2);

  const intptr_t aligned_addr = (((intptr_t)addr + A - 1) & -(intptr_t)A);

  return reinterpret_cast<T*>(aligned_addr);
}

// Explicitly invoke destructor at address
template<typename T>
static void invoke_dtor(T *addr) {
  addr->~T();
}

// Array version
template<typename T, int n>
static void invoke_dtor(T (*addr)[n]) {
  for (int i = 0 ; i < n ; ++i)
    invoke_dtor(&(*addr)[i]);
}

/**
 * Proxy class that contains one instance of T, aligned at A bytes.
 *
 * The instance is accessible as member 'instance'. It is guaranteed that
 * '&instance' is aligned at the alignment boundary 'A', which must be a
 * power of 2.
 *
 * 'instance' is initialised using its default constructor.
 */
template<int A, typename T>
class Aligned {
 private:
  T *addr_;

 public:
  Aligned() : addr_(align<A, T>(storage_)), instance(*addr_) {
    T* a = new(addr_) T();   // Call constructor at the aligned address
    (void)a;  // For when assert is compiled out
    assert(a == addr_);
  }

  ~Aligned() {
    // We are responsible for calling the destructor explicitly.
    invoke_dtor(addr_);
  }

  // Conversion to the underlying type
  operator T&() { return instance; }

  // Note: provide other operators as required
  T& operator=(const T &rhs) { instance = rhs; return instance; }

  T &instance;   // The instance

 private:
  char storage_[sizeof(T) + A - 1];
};


/**
 * Specialisation for arrays
 */
template<int A, typename E, int n>
class Aligned<A, E[n]> {
 private:
  E (*addr_)[n];

 public:
  Aligned() : addr_(align<A, E[n]>(storage_)), instance(*addr_) {
    E *a = new(addr_) E[n];   // Call constructor at the aligned address
    (void)a;  // For when assert is compiled out
    assert(a == &(*addr_)[0]);
  }

  ~Aligned() {
    // We are responsible for calling the destructor explicitly.
    invoke_dtor(addr_);
  }

  // Decay to pointer. This also covers indexing.
  operator E*() { return &(*addr_)[0]; }

  E (&instance)[n];  // The instance

 private:
  char storage_[sizeof(E[n]) + A - 1];
};

}   // namespace libvpx_test

#endif  // TEST_ALIGNED_H_
