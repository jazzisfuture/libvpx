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
#include "vpx_ports/mem.h"

#include "./vpx_dsp_rtcd.h"
#include "./vp10_rtcd.h"

#include "vpx_dsp/vpx_dsp_common.h"

#include "vp10/common/enums.h"

#include "test/array_utils.h"
#include "test/assertion_helpers.h"
#include "test/function_equivalence_test.h"
#include "test/randomise.h"
#include "test/register_state_check.h"
#include "test/snapshot.h"

#define WEDGE_WEIGHT_BITS 6

using std::tr1::make_tuple;
using libvpx_test::FunctionEquivalenceTest;
using libvpx_test::Snapshot;
using libvpx_test::Randomise;
using libvpx_test::array_utils::arraySet;
using libvpx_test::assertion_helpers::ArraysEq;
using libvpx_test::assertion_helpers::ArraysEqWithin;

namespace {

static const int16_t int13_max = (1<<12) - 1;

//////////////////////////////////////////////////////////////////////////////
// vp10_wedge_sse_from_residuals
//////////////////////////////////////////////////////////////////////////////

typedef uint64_t (*FSSE)(const int16_t *r1,
                         const int16_t *d,
                         const uint8_t *m,
                         int N);

class WedgeUtilsSSETest : public FunctionEquivalenceTest<FSSE> {
 protected:
  void Common() {
    const int N = 64 * randomise.uniform<uint32_t>(1, MAX_SB_SQUARE/64);

    snapshot(r1);
    snapshot(d);
    snapshot(m);

    uint64_t ref_res, tst_res;

    ref_res = ref_func_(r1, d, m, N);
    ASM_REGISTER_STATE_CHECK(tst_res = tst_func_(r1, d, m, N));

    ASSERT_EQ(ref_res, tst_res);

    ASSERT_TRUE(ArraysEq(snapshot.get(r1), r1));
    ASSERT_TRUE(ArraysEq(snapshot.get(d), d));
    ASSERT_TRUE(ArraysEq(snapshot.get(m), m));
  }

  Snapshot snapshot;
  Randomise randomise;

  DECLARE_ALIGNED(16, int16_t, r1[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, d[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, uint8_t, m[MAX_SB_SQUARE]);
};

TEST_P(WedgeUtilsSSETest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    randomise(r1, -(1<<12)+1, 1<<12);
    randomise(d, -(1<<12)+1, 1<<12);
    randomise(m, 0, 65);

    Common();
  }
}

TEST_P(WedgeUtilsSSETest, ExtremeValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    if (randomise.uniform<bool>())
      arraySet(r1, int13_max);
    else
      arraySet(r1, -int13_max);

    if (randomise.uniform<bool>())
      arraySet(d, int13_max);
    else
      arraySet(d, -int13_max);

    arraySet(m, 64);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsSSETest,
    ::testing::Values(
        make_tuple(&vp10_wedge_sse_from_residuals_c,
                   &vp10_wedge_sse_from_residuals_sse2)
    )
);
#endif  // HAVE_SSE2

//////////////////////////////////////////////////////////////////////////////
// vp10_wedge_sign_from_residuals
//////////////////////////////////////////////////////////////////////////////

typedef int (*FSign)(const int16_t *ds,
                     const uint8_t *m,
                     int N,
                     int64_t limit);

class WedgeUtilsSignTest : public FunctionEquivalenceTest<FSign> {
 protected:
  static const int maxSize = 8196;  // Size limited by SIMD implementation.

  void Common() {
    const int maxN = VPXMIN(maxSize, MAX_SB_SQUARE);
    const int N = 64 * randomise.uniform<uint32_t>(1, maxN/64);

    int64_t limit;
    limit = (int64_t)vpx_sum_squares_i16(r0, N);
    limit -= (int64_t)vpx_sum_squares_i16(r1, N);
    limit *= (1 << WEDGE_WEIGHT_BITS) / 2;

    for (int i = 0 ; i < N ; i++)
      ds[i] = clamp(r0[i]*r0[i] - r1[i]*r1[i], INT16_MIN, INT16_MAX);

    snapshot(r0);
    snapshot(r1);
    snapshot(ds);
    snapshot(m);

    int ref_res, tst_res;

    ref_res = ref_func_(ds, m, N, limit);
    ASM_REGISTER_STATE_CHECK(tst_res = tst_func_(ds, m, N, limit));

    ASSERT_EQ(ref_res, tst_res);

    ASSERT_TRUE(ArraysEq(snapshot.get(r0), r0));
    ASSERT_TRUE(ArraysEq(snapshot.get(r1), r1));
    ASSERT_TRUE(ArraysEq(snapshot.get(ds), ds));
    ASSERT_TRUE(ArraysEq(snapshot.get(m), m));
  }

  Snapshot snapshot;
  Randomise randomise;

  DECLARE_ALIGNED(16, int16_t, r0[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, r1[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, ds[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, uint8_t, m[MAX_SB_SQUARE]);
};

TEST_P(WedgeUtilsSignTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    randomise(r0, -int13_max, int13_max+1);
    randomise(r1, -int13_max, int13_max+1);
    randomise(m, 0, 65);

    Common();
  }
}

TEST_P(WedgeUtilsSignTest, ExtremeValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    switch (randomise.uniform<int>(4)) {
    case 0:
      arraySet(r0, 0);
      arraySet(r1, int13_max);
      break;
    case 1:
      arraySet(r0, int13_max);
      arraySet(r1, 0);
      break;
    case 2:
      arraySet(r0, 0);
      arraySet(r1, -int13_max);
      break;
    default:
      arraySet(r0, -int13_max);
      arraySet(r1, 0);
      break;
    }

    arraySet(m, 64);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsSignTest,
    ::testing::Values(
        make_tuple(&vp10_wedge_sign_from_residuals_c,
                   &vp10_wedge_sign_from_residuals_sse2)
    )
);
#endif  // HAVE_SSE2

//////////////////////////////////////////////////////////////////////////////
// vp10_wedge_compute_delta_squares
//////////////////////////////////////////////////////////////////////////////

typedef void (*FDS)(int16_t *d,
                    const int16_t *a,
                    const int16_t *b,
                    int N);

class WedgeUtilsDeltaSquaresTest : public FunctionEquivalenceTest<FDS> {
 protected:
  void Common() {
    const int N = 64 * randomise.uniform<uint32_t>(1, MAX_SB_SQUARE/64);

    randomise(d_ref);
    randomise(d_tst);

    snapshot(a);
    snapshot(b);

    ref_func_(d_ref, a, b, N);
    ASM_REGISTER_STATE_CHECK(tst_func_(d_tst, a, b, N));

    ASSERT_TRUE(ArraysEqWithin(d_ref, d_tst, 0, N));

    ASSERT_TRUE(ArraysEq(snapshot.get(a), a));
    ASSERT_TRUE(ArraysEq(snapshot.get(b), b));
  }

  Snapshot snapshot;
  Randomise randomise;

  DECLARE_ALIGNED(16, int16_t, a[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, b[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, d_ref[MAX_SB_SQUARE]);
  DECLARE_ALIGNED(16, int16_t, d_tst[MAX_SB_SQUARE]);
};

TEST_P(WedgeUtilsDeltaSquaresTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    randomise(a);
    randomise(b, -INT16_MAX, INT16_MAX + 1);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsDeltaSquaresTest,
    ::testing::Values(
        make_tuple(&vp10_wedge_compute_delta_squares_c,
                   &vp10_wedge_compute_delta_squares_sse2)
    )
);
#endif  // HAVE_SSE2


}  // namespace
