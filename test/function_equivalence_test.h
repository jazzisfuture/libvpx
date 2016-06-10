/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_FUNCTION_EQUIVALENCE_TEST_H_
#define TEST_FUNCTION_EQUIVALENCE_TEST_H_

#include <list>

#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/clear_system_state.h"
#include "test/util.h"

#include "vpx_mem/vpx_mem.h"

namespace libvpx_test {
template <typename T>
class FunctionEquivalenceTest :
  public ::testing::TestWithParam< std::tr1::tuple< T, T > > {
 public:
  virtual ~FunctionEquivalenceTest() {
    // Free all storage allocated with 'aligned'
    std::list<void*>::iterator it;
    for (it = allocations_.begin() ; it != allocations_.end() ; it++)
      vpx_free(*it);
    allocations_.clear();
  }

  virtual void SetUp() {
    ref_func_ = std::tr1::get<0>(this->GetParam());
    tst_func_ = std::tr1::get<1>(this->GetParam());
  }

  virtual void TearDown() {
    libvpx_test::ClearSystemState();
  }

 protected:
  /**
   * Allocate an area of memory big enough to hold an instance of U,
   * starting at the specified alignment. Keep track of the allocated
   * address and free the area when this object is destroyed.
   *
   * A reference to type U must be passed, but it is only used to
   * derive the size of the desired object based on its static type,
   * so it can be an uninitialised reference.
   *
   * Return a reference to the allocated area.
   */
  template<typename U>
  U& aligned(size_t alignment, const U &u) {
    void *const p = vpx_memalign(alignment, sizeof(u));
    allocations_.push_back(p);
    return *reinterpret_cast<U*>(p);
  }

  T ref_func_;
  T tst_func_;

 private:
  std::list<void*>  allocations_;
};

}   // namespace libvpx_test
#endif  // TEST_FUNCTION_EQUIVALENCE_TEST_H_
