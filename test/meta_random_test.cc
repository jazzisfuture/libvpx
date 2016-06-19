/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits>

#include "test/array.h"
#include "test/random.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

using ::libvpx_test::Array;
using ::libvpx_test::Random;

namespace {

template<typename T>
class METARandomAllTypes : public ::testing::Test {};

typedef ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t,
                         int8_t, int16_t, int32_t, int64_t> AllTypes;

TYPED_TEST_CASE(METARandomAllTypes, AllTypes);

//////////////////////////////////////////////////////////////////////////////
// Bool values
//////////////////////////////////////////////////////////////////////////////

TEST(METARandom, Bool) {
  Random random;

  bool seenTrue = false;
  bool seenFalse = false;

  for (int i = 0 ; i < 1000 ; ++i) {
    if (random.Uniform<bool>())
      seenTrue = true;
    else
      seenFalse = true;
  }

  EXPECT_TRUE(seenTrue);
  EXPECT_TRUE(seenFalse);
}

//////////////////////////////////////////////////////////////////////////////
// All possible values are yielded
//////////////////////////////////////////////////////////////////////////////

TEST(METARandom, UInt8AllValues) {
  Random random;

  Array<int32_t[UINT8_MAX + 1]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i)
    hist[random.Uniform<uint8_t>()] += 1;

  for (size_t i = 0 ; i < hist.Size() ; ++i)
    EXPECT_LT(0, hist[i]);
}

TEST(METARandom, UInt16AllValues) {
  Random random;

  Array<int32_t[UINT16_MAX + 1]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 10000000 ; ++i)
    hist[random.Uniform<uint16_t>()] += 1;

  for (size_t i = 0 ; i < hist.Size() ; ++i)
    EXPECT_LT(0, hist[i]);
}

TEST(METARandom, Int8AllValues) {
  Random random;

  Array<int32_t[INT8_MAX - INT8_MIN + 1]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i)
    hist[random.Uniform<int8_t>() - INT8_MIN] += 1;

  for (size_t i = 0 ; i < hist.Size(); ++i)
    EXPECT_LT(0, hist[i]);
}

TEST(METARandom, Int16AllValues) {
  Random random;

  Array<int32_t[INT16_MAX - INT16_MIN + 1]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 10000000 ; ++i)
    hist[random.Uniform<int16_t>() - INT16_MIN] += 1;

  for (size_t i = 0 ; i < hist.Size(); ++i)
    EXPECT_LT(0, hist[i]);
}

//////////////////////////////////////////////////////////////////////////////
// All bits at least toggle
//////////////////////////////////////////////////////////////////////////////

TEST(METARandom, UInt32AllBits) {
  Random random;

  Array<int32_t[4][256]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i) {
    uint32_t v = random.Uniform<uint32_t>();
    hist[0][v & 0xff] += 1;
    hist[1][(v >> 8) & 0xff] += 1;
    hist[2][(v >> 16) & 0xff] += 1;
    hist[3][(v >> 24) & 0xff] += 1;
  }

  for (int j = 0 ; j < 256; ++j) {
    EXPECT_LT(0, hist[0][j]);
    EXPECT_LT(0, hist[1][j]);
    EXPECT_LT(0, hist[2][j]);
    EXPECT_LT(0, hist[3][j]);
  }
}

TEST(METARandom, UInt64AllBits) {
  Random random;

  Array<int32_t[8][256]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i) {
    uint64_t v = random.Uniform<uint64_t>();
    hist[0][v & 0xff] += 1;
    hist[1][(v >> 8) & 0xff] += 1;
    hist[2][(v >> 16) & 0xff] += 1;
    hist[3][(v >> 24) & 0xff] += 1;
    hist[4][(v >> 32) & 0xff] += 1;
    hist[5][(v >> 40) & 0xff] += 1;
    hist[6][(v >> 48) & 0xff] += 1;
    hist[7][(v >> 56) & 0xff] += 1;
  }

  for (int j = 0 ; j < 256; ++j) {
    EXPECT_LT(0, hist[0][j]);
    EXPECT_LT(0, hist[1][j]);
    EXPECT_LT(0, hist[2][j]);
    EXPECT_LT(0, hist[3][j]);
    EXPECT_LT(0, hist[4][j]);
    EXPECT_LT(0, hist[5][j]);
    EXPECT_LT(0, hist[6][j]);
    EXPECT_LT(0, hist[7][j]);
  }
}

TEST(METARandom, Int32AllBits) {
  Random random;

  Array<int32_t[4][256]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i) {
    int32_t v = random.Uniform<int32_t>();
    hist[0][v & 0xff] += 1;
    hist[1][(v >> 8) & 0xff] += 1;
    hist[2][(v >> 16) & 0xff] += 1;
    hist[3][(v >> 24) & 0xff] += 1;
  }

  for (int j = 0 ; j < 256; ++j) {
    EXPECT_LT(0, hist[0][j]);
    EXPECT_LT(0, hist[1][j]);
    EXPECT_LT(0, hist[2][j]);
    EXPECT_LT(0, hist[3][j]);
  }
}

TEST(METARandom, Int64AllBits) {
  Random random;

  Array<int32_t[8][256]> hist;

  hist.Set(0);

  for (int i = 0 ; i < 1000000 ; ++i) {
    int64_t v = random.Uniform<int64_t>();
    hist[0][v & 0xff] += 1;
    hist[1][(v >> 8) & 0xff] += 1;
    hist[2][(v >> 16) & 0xff] += 1;
    hist[3][(v >> 24) & 0xff] += 1;
    hist[4][(v >> 32) & 0xff] += 1;
    hist[5][(v >> 40) & 0xff] += 1;
    hist[6][(v >> 48) & 0xff] += 1;
    hist[7][(v >> 56) & 0xff] += 1;
  }

  for (int j = 0 ; j < 256; ++j) {
    EXPECT_LT(0, hist[0][j]);
    EXPECT_LT(0, hist[1][j]);
    EXPECT_LT(0, hist[2][j]);
    EXPECT_LT(0, hist[3][j]);
    EXPECT_LT(0, hist[4][j]);
    EXPECT_LT(0, hist[5][j]);
    EXPECT_LT(0, hist[6][j]);
    EXPECT_LT(0, hist[7][j]);
  }
}

//////////////////////////////////////////////////////////////////////////////
// All values from range are yielded
//////////////////////////////////////////////////////////////////////////////

template<typename R, uint64_t I = 1000000>
class YieldsWholeRange {
 public:
  void Run(R hi) {
    uint64_t n = I;
    uint32_t *hist = new uint32_t[hi + 1];

    for (uint64_t i = 0 ; i <= static_cast<uint64_t>(hi) ; ++i)
      hist[i] = 0;

    while (n--) {
      R val = random_.Uniform<R>(hi);
      hist[val] += 1;

      ASSERT_LE(static_cast<R>(0), val);
      ASSERT_GE(hi, val);
    }

    for (uint64_t i = 0 ; i <= static_cast<uint64_t>(hi) ; ++i)
      EXPECT_LT(0u, hist[i]) << i;

    delete[] hist;
  }

  void Run(R lo, R hi) {
    uint64_t n = I;
    uint32_t *hist = new uint32_t[hi - lo + 1];

    for (uint64_t i = 0 ; i <= static_cast<uint64_t>(hi - lo) ; ++i)
      hist[i] = 0;

    while (n--) {
      R val = random_.Uniform<R>(lo, hi);
      hist[val - lo] += 1;

      ASSERT_LE(lo, val);
      ASSERT_GE(hi, val);
    }

    for (uint64_t i = 0 ; i <= static_cast<uint64_t>(hi - lo) ; ++i)
      EXPECT_LT(0u, hist[i]);

    delete[] hist;
  }

  void Run() {
    R min = std::numeric_limits<R>::min();
    R max = std::numeric_limits<R>::max();

    Run(1);
    Run(100);
    if (std::numeric_limits<R>::digits <= 16)
      Run(max);

    if (std::numeric_limits<R>::is_signed) {
      Run(min, min + 1);
      Run(min, min + 100);
      Run(-100, 0);
      Run(-100, 100);
      if (std::numeric_limits<R>::digits <= 16)
        Run(min, max);
    }

    Run(0, 1);
    Run(1, 2);
    Run(0, 100);
    Run(1, 100);
    Run(50, 100);
    Run(max - 100, max);
    Run(max - 1, max);
  }

 private:
  Random random_;
};

TYPED_TEST(METARandomAllTypes, YieldsWholeRange) {
  YieldsWholeRange<TypeParam> test; test.Run();
}

//////////////////////////////////////////////////////////////////////////////
// Extreme values from range are yielded
//////////////////////////////////////////////////////////////////////////////

template<typename R, uint64_t I = 1000000>
class YieldsRangeExtremes {
 public:
  void Run(R hi) {
    int64_t n = I;

    R band = hi / 16;

    assert(band > 0);

    while (n-- > 0) {
      R val = random_.Uniform<R>(hi);
      ASSERT_LE(static_cast<R>(0), val);
      ASSERT_GE(hi, val);
      if (hi - band < val)
        break;
    }

    while (n-- > 0) {
      R val = random_.Uniform<R>(hi);
      ASSERT_LE(static_cast<R>(0), val);
      ASSERT_GE(hi, val);
      if (val < band)
        break;
    }

    ASSERT_LT(0, n) << "Did not yield extreme values from Range";

    while (n-- > 0) {
      R val = random_.Uniform<R>(hi);
      ASSERT_LE(static_cast<R>(0), val);
      ASSERT_GE(hi, val);
    }
  }

  void Run(R lo, R hi) {
    int64_t n = I;

    R band = hi / 16 - lo / 16;

    assert(band > 0);

    while (n-- > 0) {
      R val = random_.Uniform<R>(lo, hi);
      ASSERT_LE(lo, val);
      ASSERT_GE(hi, val);
      if (hi - band < val)
        break;
    }

    while (n-- > 0) {
      R val = random_.Uniform<R>(lo, hi);
      ASSERT_LE(lo, val);
      ASSERT_GE(hi, val);
      if (val < lo + band)
        break;
    }

    ASSERT_LT(0, n) << "Did not yield extreme values from Range";

    while (n-- > 0) {
      R val = random_.Uniform<R>(hi);
      ASSERT_LE(lo, val);
      ASSERT_GE(hi, val);
    }
  }

  void Run() {
    R min = std::numeric_limits<R>::min();
    R max = std::numeric_limits<R>::max();

    Run(100);
    Run(max);
    Run(max/4*3);

    if (std::numeric_limits<R>::is_signed)
      Run(-100, 100);
    Run(min, max);
    Run(min/4*3, max/4*3);
  }

 private:
  Random random_;
};

TYPED_TEST(METARandomAllTypes, YieldsRangeExtremes) {
  YieldsRangeExtremes<TypeParam> test; test.Run();
}

//////////////////////////////////////////////////////////////////////////////
// Randomise native arrays
//////////////////////////////////////////////////////////////////////////////

TYPED_TEST(METARandomAllTypes, NativeArray1D) {
  TypeParam a[100];

  for (size_t i = 0 ; i < 100 ; ++i)
    a[i] = 0;

  Random random;

  random.Uniform(&a, 1, UINT8_MAX);

  for (size_t i = 0 ; i < 100 ; ++i)
    EXPECT_NE(static_cast<TypeParam>(0), a[i]);
}

TYPED_TEST(METARandomAllTypes, NativeArray2D) {
  TypeParam a[100][20];

  for (size_t i = 0 ; i < 100 ; ++i)
    for (size_t j = 0 ; j < 20 ; ++j)
      a[i][j] = 0;

  Random random;

  random.Uniform(&a, 1, UINT8_MAX);

  for (size_t i = 0 ; i < 100 ; ++i)
    for (size_t j = 0 ; j < 20 ; ++j)
      EXPECT_NE(static_cast<TypeParam>(0), a[i][j]);
}

//////////////////////////////////////////////////////////////////////////////
// Randomise Array
//////////////////////////////////////////////////////////////////////////////

TYPED_TEST(METARandomAllTypes, Array1D) {
  Array<TypeParam[100]> a;

  a.Set(0);

  Random random;

  random.Uniform(&a, 1, UINT8_MAX);

  for (size_t i = 0 ; i < a.Size() ; ++i)
    EXPECT_NE(static_cast<TypeParam>(0), a[i]);
}

TYPED_TEST(METARandomAllTypes, Array2D) {
    Array<TypeParam[100][20]> a;

    a.Set(0);

    Random random;

    random.Uniform(&a, 1, UINT8_MAX);

    for (size_t i = 0 ; i < a.Size(0) ; ++i)
      for (size_t j = 0 ; j < a.Size(1) ; ++j)
        EXPECT_NE(static_cast<TypeParam>(0), a[i][j]);
}

}  // namespace
