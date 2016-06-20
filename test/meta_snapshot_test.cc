/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "test/array.h"
#include "test/random.h"
#include "test/snapshot.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

using ::libvpx_test::Array;
using ::libvpx_test::Random;
using ::libvpx_test::Snapshot;
using ::testing::StaticAssertTypeEq;

namespace {

template<typename T>
class METASnapshotPrimitiveTypes : public ::testing::Test {
 public:
  static const int kIterations = 1000;
};

typedef ::testing::Types<bool,
                         uint8_t, uint16_t, uint32_t, uint64_t,
                         int8_t, int16_t, int32_t, int64_t> PrimitiveTypes;

TYPED_TEST_CASE(METASnapshotPrimitiveTypes, PrimitiveTypes);

//////////////////////////////////////////////////////////////////////////////
// Snapshot primitive types
//////////////////////////////////////////////////////////////////////////////

TYPED_TEST(METASnapshotPrimitiveTypes, PrimitiveType) {
  Random random, randomRef;
  Snapshot snapshot;

  for (int i = 0 ; i < this->kIterations ; ++i) {
    TypeParam variable = random.Uniform<TypeParam>();;

    snapshot(variable);

    variable = !variable;

    ASSERT_NE(variable, snapshot.Get(variable));
    ASSERT_EQ(randomRef.Uniform<TypeParam>(), snapshot.Get(variable));
  }
}

//////////////////////////////////////////////////////////////////////////////
// Snapshot native arrays
//////////////////////////////////////////////////////////////////////////////

TYPED_TEST(METASnapshotPrimitiveTypes, NativeArray1D) {
  Random random, randomRef;
  Snapshot snapshot;

  for (int i = 0 ; i < this->kIterations ; ++i) {
    TypeParam array[200];

    random.Uniform(&array);

    snapshot(array);

    for (size_t i = 0 ; i < 200 ; ++i)
      array[i] = !array[i];

    for (size_t i = 0 ; i < 200 ; ++i) {
      ASSERT_NE(array[i], snapshot.Get(array)[i]);
      ASSERT_EQ(randomRef.Uniform<TypeParam>(), snapshot.Get(array)[i]);
    }
  }
}

TYPED_TEST(METASnapshotPrimitiveTypes, NativeArray2D) {
  Random random, randomRef;
  Snapshot snapshot;

  for (int i = 0 ; i < this->kIterations ; ++i) {
    TypeParam array[200][100];

    random.Uniform(&array);

    snapshot(array);

    for (size_t i = 0 ; i < 200 ; ++i)
      for (size_t j = 0 ; j < 100 ; ++j)
        array[i][j] = !array[i][j];

    for (size_t i = 0 ; i < 200 ; ++i) {
      for (size_t j = 0 ; j < 100 ; ++j) {
        ASSERT_NE(array[i][j], snapshot.Get(array)[i][j]);
        ASSERT_EQ(randomRef.Uniform<TypeParam>(), snapshot.Get(array)[i][j]);
      }
    }
  }
}

}  // namespace
