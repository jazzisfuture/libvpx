/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"

#include "test/array.h"
#include "test/assertion_helpers.h"
#include "test/function_equivalence_test.h"
#include "test/random.h"
#include "test/register_state_check.h"
#include "test/snapshot.h"

using libvpx_test::Array;
using libvpx_test::FunctionEquivalenceTest;
using libvpx_test::Snapshot;
using libvpx_test::Random;
using libvpx_test::assertion_helpers::ArraysEq;

namespace {

static const int16_t int13_max = (1 << 12) - 1;

//////////////////////////////////////////////////////////////////////////////
// 2D version
//////////////////////////////////////////////////////////////////////////////

typedef uint64_t (*F2D)(const int16_t *src, int stride, uint32_t size);

class SumSquares2DTest : public FunctionEquivalenceTest<F2D> {
 protected:
  void Common() {
    const int sizelog2 = random.Uniform<int>(2, 7);

    const uint32_t size = 1 << sizelog2;
    const int stride = 1 << random.Uniform<int>(sizelog2, 8);

    snapshot(src);

    uint64_t ref_res, tst_res;

    ref_res = ref_func_(src.First(), stride, size);
    ASM_REGISTER_STATE_CHECK(tst_res = tst_func_(src.First(), stride, size));

    ASSERT_EQ(ref_res, tst_res);

    ASSERT_TRUE(ArraysEq(snapshot.Get(src), src));
  }

  Snapshot snapshot;
  Random random;

  Array<int16_t[256][256], 32> src;
};

TEST_P(SumSquares2DTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    random.Uniform(&src, -int13_max, int13_max + 1);

    Common();
  }
}

TEST_P(SumSquares2DTest, ExtremeValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    src.Set(random.Uniform<bool>() ? int13_max : -int13_max);

    Common();
  }
}
using std::tr1::make_tuple;

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, SumSquares2DTest,
    ::testing::Values(
        make_tuple(&vpx_sum_squares_2d_i16_c, &vpx_sum_squares_2d_i16_sse2)
    )
);
#endif  // HAVE_SSE2
}  // namespace
