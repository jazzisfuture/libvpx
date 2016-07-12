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

#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/util.h"

using libvpx_test::ACMRandom;

namespace libvpx_test {
// Base class for tests that compare 2 implementations of the same function
// for equivalence. The template parameter should be pointer to a function
// that is being tested.
//
// The test takes 3 parameters:
//   - Pointer to reference function
//   - Pointer to tested function
//   - Integer bit depth
//
// These values are then accessible in the tests as members
// ref_func_, tst_func_, and bit_depth_.
//
// Use the MakeParam static method to construct parameters. The bit depth
// is then optional and defaults to 0.
template <typename T>
class FunctionEquivalenceTest :
  public ::testing::TestWithParam< std::tr1::tuple< T, T, int > > {
 public:
  typedef typename FunctionEquivalenceTest<T>::ParamType ParamType;

  FunctionEquivalenceTest() : rng_(ACMRandom::DeterministicSeed()) {}

  virtual ~FunctionEquivalenceTest() {}

  static ParamType MakeParam(T ref, T tst, int bit_depth = 0) {
    return std::tr1::make_tuple(ref, tst, bit_depth);
  }

  virtual void SetUp() {
    ref_func_ = std::tr1::get<0>(this->GetParam());
    tst_func_ = std::tr1::get<1>(this->GetParam());
    bit_depth_ = std::tr1::get<2>(this->GetParam());
  }

  virtual void TearDown() {
    libvpx_test::ClearSystemState();
  }

 protected:
  ACMRandom rng_;

  T ref_func_;
  T tst_func_;

  int bit_depth_;
};

}   // namespace libvpx_test
#endif  // TEST_FUNCTION_EQUIVALENCE_TEST_H_
