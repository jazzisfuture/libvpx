/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
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
#include "./vp10_rtcd.h"

#include "vpx_dsp/vpx_dsp_common.h"

#include "vp10/common/enums.h"

#include "test/array.h"
#include "test/assertion_helpers.h"
#include "test/function_equivalence_test.h"
#include "test/random.h"
#include "test/register_state_check.h"
#include "test/snapshot.h"

#define WEDGE_WEIGHT_BITS 6
#define MAX_MASK_VALUE  (1 << (WEDGE_WEIGHT_BITS))

using std::tr1::make_tuple;
using libvpx_test::Array;
using libvpx_test::FunctionEquivalenceTest;
using libvpx_test::Snapshot;
using libvpx_test::Random;
using libvpx_test::assertion_helpers::ArraysEq;
using libvpx_test::assertion_helpers::ArraysEqWithin;

namespace {

static const int16_t int13_max = (1 << 12) - 1;

typedef int16_t int16_buf_t[MAX_SB_SQUARE];
typedef uint8_t uint8_buf_t[MAX_SB_SQUARE];

//////////////////////////////////////////////////////////////////////////////
// vp10_wedge_sse_from_residuals - functionality
//////////////////////////////////////////////////////////////////////////////

class WedgeUtilsSSEFuncTest : public testing::Test {
 protected:
  Snapshot snapshot;
  Random random;
};

static void equiv_blend_residuals(int16_t *r,
                                  const int16_t *r0,
                                  const int16_t *r1,
                                  const uint8_t *m,
                                  int N) {
  for (int i = 0 ; i < N ; i++) {
    const int32_t m0 = m[i];
    const int32_t m1 = MAX_MASK_VALUE - m0;
    const int16_t R = m0 * r0[i] + m1 * r1[i];
    // Note that this rounding is designed to match the result
    // you would get when actually blending the 2 predictors and computing
    // the residuals.
    r[i] = ROUND_POWER_OF_TWO(R - 1, WEDGE_WEIGHT_BITS);
  }
}

static uint64_t equiv_sse_from_residuals(const int16_t *r0,
                                         const int16_t *r1,
                                         const uint8_t *m,
                                         int N) {
  uint64_t acc = 0;
  for (int i = 0 ; i < N ; i++) {
    const int32_t m0 = m[i];
    const int32_t m1 = MAX_MASK_VALUE - m0;
    const int16_t R = m0 * r0[i] + m1 * r1[i];
    const int32_t r = ROUND_POWER_OF_TWO(R - 1, WEDGE_WEIGHT_BITS);
    acc += r * r;
  }
  return acc;
}

TEST_F(WedgeUtilsSSEFuncTest, ResidualBlendingEquiv) {
  for (int i = 0 ; i < 1000 && !HasFatalFailure(); i++) {
    Array<uint8_buf_t, 32> s;
    Array<uint8_buf_t, 32> p0;
    Array<uint8_buf_t, 32> p1;
    Array<uint8_buf_t, 32> p;

    Array<int16_buf_t, 32> r0;
    Array<int16_buf_t, 32> r1;
    Array<int16_buf_t, 32> r_ref;
    Array<int16_buf_t, 32> r_tst;
    Array<uint8_buf_t, 32> m;

    random.Uniform(&s);
    random.Uniform(&m, 0, MAX_MASK_VALUE);

    const int w = 1 << random.Uniform(3, MAX_SB_SIZE_LOG2);
    const int h = 1 << random.Uniform(3, MAX_SB_SIZE_LOG2);
    const int N = w * h;

    for (int j = 0 ; j < N ; j++) {
      p0[j] = clamp(s[j] + random.Uniform(-16, 16), 0, UINT8_MAX);
      p1[j] = clamp(s[j] + random.Uniform(-16, 16), 0, UINT8_MAX);
    }

    vpx_blend_mask6(p, w, p0, w, p1, w, m, w, h, w, 0, 0);

    vpx_subtract_block(h, w, r0, w, s, w, p0, w);
    vpx_subtract_block(h, w, r1, w, s, w, p1, w);

    vpx_subtract_block(h, w, r_ref, w, s, w, p, w);
    equiv_blend_residuals(r_tst, r0, r1, m, N);

    ASSERT_TRUE(ArraysEqWithin(r_ref, r_tst, 0, N));

    uint64_t ref_sse = vpx_sum_squares_i16(r_ref, N);
    uint64_t tst_sse = equiv_sse_from_residuals(r0, r1, m, N);

    ASSERT_EQ(ref_sse, tst_sse);
  }
}

static uint64_t sse_from_residuals(const int16_t *r0,
                                   const int16_t *r1,
                                   const uint8_t *m,
                                   int N) {
  uint64_t acc = 0;
  for (int i = 0 ; i < N ; i++) {
    const int32_t m0 = m[i];
    const int32_t m1 = MAX_MASK_VALUE - m0;
    const int32_t r = m0 * r0[i] + m1 * r1[i];
    acc += r * r;
  }
  return ROUND_POWER_OF_TWO(acc, 2 * WEDGE_WEIGHT_BITS);
}

TEST_F(WedgeUtilsSSEFuncTest, ResidualBlendingMethod) {
  for (int i = 0 ; i < 1000 && !HasFatalFailure(); i++) {
    Array<int16_buf_t, 32> r0;
    Array<int16_buf_t, 32> r1;
    Array<int16_buf_t, 32> d;
    Array<uint8_buf_t, 32> m;

    random.Uniform(&r1, 2 * INT8_MIN, 2 * INT8_MAX);
    random.Uniform(&d, 2 * INT8_MIN, 2 * INT8_MAX);
    random.Uniform(&m, 0, MAX_MASK_VALUE);

    const int N = 64 * random.Uniform(1, MAX_SB_SQUARE/64);

    for (int j = 0 ; j < N ; j++)
      r0[j] = r1[j] + d[j];

    uint64_t ref_res, tst_res;

    ref_res = sse_from_residuals(r0, r1, m, N);
    tst_res = vp10_wedge_sse_from_residuals(r1, d, m, N);

    ASSERT_EQ(ref_res, tst_res);
  }
}

//////////////////////////////////////////////////////////////////////////////
// vp10_wedge_sse_from_residuals - optimizations
//////////////////////////////////////////////////////////////////////////////

typedef uint64_t (*FSSE)(const int16_t *r1,
                         const int16_t *d,
                         const uint8_t *m,
                         int N);

class WedgeUtilsSSEOptTest : public FunctionEquivalenceTest<FSSE> {
 protected:
  void Common() {
    const int N = 64 * random.Uniform(1, MAX_SB_SQUARE/64);

    snapshot(r1);
    snapshot(d);
    snapshot(m);

    uint64_t ref_res, tst_res;

    ref_res = ref_func_(r1, d, m, N);
    ASM_REGISTER_STATE_CHECK(tst_res = tst_func_(r1, d, m, N));

    ASSERT_EQ(ref_res, tst_res);

    ASSERT_TRUE(ArraysEq(snapshot.Get(r1), r1));
    ASSERT_TRUE(ArraysEq(snapshot.Get(d), d));
    ASSERT_TRUE(ArraysEq(snapshot.Get(m), m));
  }

  Snapshot snapshot;
  Random random;

  Array<int16_buf_t, 32> r1;
  Array<int16_buf_t, 32> d;
  Array<uint8_buf_t, 32> m;
};

TEST_P(WedgeUtilsSSEOptTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    random.Uniform(&r1, -int13_max, int13_max);
    random.Uniform(&d, -int13_max, int13_max);
    random.Uniform(&m, 0, 64);

    Common();
  }
}

TEST_P(WedgeUtilsSSEOptTest, ExtremeValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    r1.Set(random.Uniform<bool>() ? int13_max : -int13_max);
    d.Set(random.Uniform<bool>() ? int13_max : -int13_max);
    m.Set(MAX_MASK_VALUE);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsSSEOptTest,
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

class WedgeUtilsSignOptTest : public FunctionEquivalenceTest<FSign> {
 protected:
  static const int maxSize = 8196;  // Size limited by SIMD implementation.

  void Common() {
    const int maxN = VPXMIN(maxSize, MAX_SB_SQUARE);
    const int N = 64 * random.Uniform(1, maxN/64);

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

    ASSERT_TRUE(ArraysEq(snapshot.Get(r0), r0));
    ASSERT_TRUE(ArraysEq(snapshot.Get(r1), r1));
    ASSERT_TRUE(ArraysEq(snapshot.Get(ds), ds));
    ASSERT_TRUE(ArraysEq(snapshot.Get(m), m));
  }

  Snapshot snapshot;
  Random random;

  Array<int16_buf_t, 32> r0;
  Array<int16_buf_t, 32> r1;
  Array<int16_buf_t, 32> ds;
  Array<uint8_buf_t, 32> m;
};

TEST_P(WedgeUtilsSignOptTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    random.Uniform(&r0, -int13_max, int13_max);
    random.Uniform(&r1, -int13_max, int13_max);
    random.Uniform(&m, 0, MAX_MASK_VALUE);

    Common();
  }
}

TEST_P(WedgeUtilsSignOptTest, ExtremeValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    switch (random.Uniform(4)) {
    case 0:
      r0.Set(0);
      r1.Set(int13_max);
      break;
    case 1:
      r0.Set(int13_max);
      r1.Set(0);
      break;
    case 2:
      r0.Set(0);
      r1.Set(-int13_max);
      break;
    default:
      r0.Set(-int13_max);
      r1.Set(0);
      break;
    }

    m.Set(MAX_MASK_VALUE);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsSignOptTest,
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

class WedgeUtilsDeltaSquaresOptTest : public FunctionEquivalenceTest<FDS> {
 protected:
  void Common() {
    const int N = 64 * random.Uniform(1, MAX_SB_SQUARE/64);

    random.Uniform(&d_ref);
    random.Uniform(&d_tst);

    snapshot(a);
    snapshot(b);

    ref_func_(d_ref, a, b, N);
    ASM_REGISTER_STATE_CHECK(tst_func_(d_tst, a, b, N));

    ASSERT_TRUE(ArraysEqWithin(d_ref, d_tst, 0, N));

    ASSERT_TRUE(ArraysEq(snapshot.Get(a), a));
    ASSERT_TRUE(ArraysEq(snapshot.Get(b), b));
  }

  Snapshot snapshot;
  Random random;

  Array<int16_buf_t, 32> a;
  Array<int16_buf_t, 32> b;
  Array<int16_buf_t, 32> d_ref;
  Array<int16_buf_t, 32> d_tst;
};

TEST_P(WedgeUtilsDeltaSquaresOptTest, RandomValues) {
  for (int i = 0 ; i < 10000 && !HasFatalFailure(); i++) {
    random.Uniform(&a);
    random.Uniform(&b, -INT16_MAX, INT16_MAX);

    Common();
  }
}

#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(
    SSE2, WedgeUtilsDeltaSquaresOptTest,
    ::testing::Values(
        make_tuple(&vp10_wedge_compute_delta_squares_c,
                   &vp10_wedge_compute_delta_squares_sse2)
    )
);
#endif  // HAVE_SSE2


}  // namespace
