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
#include "third_party/googletest/src/include/gtest/gtest.h"

using ::libvpx_test::Array;
using ::testing::StaticAssertTypeEq;

namespace {

//////////////////////////////////////////////////////////////////////////////
// Storage alignment
//////////////////////////////////////////////////////////////////////////////

template<typename T, int A>
class AlignmentCheck {
 public:
  void Run() {
    const intptr_t mask = static_cast<intptr_t>(A) - 1;

    // Allocate on heap to avoid stack overflow at large iteration counts
    Array<T, A> **p = new Array<T, A> *[kIterations];

    // Allocate instances and check alignments
    for (int i = 0 ; i < kIterations ; ++i) {
      p[i] = new Array<T, A>;

      Array<T, A> &v = *p[i];

      ASSERT_EQ(0, reinterpret_cast<intptr_t>(v.AddrOf()) & mask);
    }

    for (int i = 0 ; i < kIterations ; ++i)
      delete p[i];

    delete[] p;
  }

 private:
  static const int kIterations = 1000000;
};

TEST(METAArray, Align1) {
  AlignmentCheck<char[32], 1> test;
  test.Run();
}

TEST(METAArray, Align2) {
  AlignmentCheck<char[32], 2> test;
  test.Run();
}

TEST(METAArray, Align4) {
  AlignmentCheck<char[32], 4> test;
  test.Run();
}

TEST(METAArray, Align8) {
  AlignmentCheck<char[32], 8> test;
  test.Run();
}

TEST(METAArray, Align16) {
  AlignmentCheck<char[32], 16> test;
  test.Run();
}

TEST(METAArray, Align32) {
  AlignmentCheck<char[32], 32> test;
  test.Run();
}

TEST(METAArray, Align64) {
  AlignmentCheck<char[32], 64> test;
  test.Run();
}

TEST(METAArray, Align128) {
  AlignmentCheck<char[32], 128> test;
  test.Run();
}

//////////////////////////////////////////////////////////////////////////////
// Alignment
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, Alignment) {
  int a;

  a = Array<char[32]>::Alignment();  // Default alignment
  ASSERT_EQ(1, a);
  a = Array<char[32], 2>::Alignment();
  ASSERT_EQ(2, a);
  a = Array<char[32], 4>::Alignment();
  ASSERT_EQ(4, a);
  a = Array<char[32], 8>::Alignment();
  ASSERT_EQ(8, a);
  a = Array<char[32], 16>::Alignment();
  ASSERT_EQ(16, a);
}

//////////////////////////////////////////////////////////////////////////////
// SizeOf
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, SizeOf) {
  EXPECT_EQ(sizeof(char[32]), Array<char[32]>::SizeOf());
  EXPECT_EQ(sizeof(int[32]), Array<int[32]>::SizeOf());
  EXPECT_EQ(sizeof(double[32]), Array<double[32]>::SizeOf());

  EXPECT_EQ(sizeof(char[3][2]), Array<char[3][2]>::SizeOf());
  EXPECT_EQ(sizeof(int[3][2]), Array<int[3][2]>::SizeOf());
  EXPECT_EQ(sizeof(double[3][2]), Array<double[3][2]>::SizeOf());
}

//////////////////////////////////////////////////////////////////////////////
// Dim
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, Dim) {
  EXPECT_EQ(1u, Array<int[1]>::Dim());
  EXPECT_EQ(2u, Array<int[1][2]>::Dim());
  EXPECT_EQ(3u, Array<int[1][2][3]>::Dim());
  EXPECT_EQ(4u, Array<int[1][2][3][4]>::Dim());
  EXPECT_EQ(5u, Array<int[1][2][3][4][5]>::Dim());
  EXPECT_EQ(6u, Array<int[1][2][3][4][5][6]>::Dim());
  EXPECT_EQ(7u, Array<int[1][2][3][4][5][6][7]>::Dim());
  EXPECT_EQ(8u, Array<int[1][2][3][4][5][6][7][8]>::Dim());
}

//////////////////////////////////////////////////////////////////////////////
// Size
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, Size) {
  typedef Array<int[2][3][5][7][11][13][17][19]> TestArray;

  EXPECT_EQ(2u, TestArray::Size(0));
  EXPECT_EQ(3u, TestArray::Size(1));
  EXPECT_EQ(5u, TestArray::Size(2));
  EXPECT_EQ(7u, TestArray::Size(3));
  EXPECT_EQ(11u, TestArray::Size(4));
  EXPECT_EQ(13u, TestArray::Size(5));
  EXPECT_EQ(17u, TestArray::Size(6));
  EXPECT_EQ(19u, TestArray::Size(7));
}

//////////////////////////////////////////////////////////////////////////////
// Indexing
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, IndexedAccess) {
  Array<uint32_t[32], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    v[i] = 42 * i;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    EXPECT_EQ(42 * i, v[i]);
}

TEST(METAArray2D, IndexedAccess) {
  Array<uint32_t[32][16], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      v[i][j] = 42 * i + 25 * j;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      EXPECT_EQ(42 * i + 25 * j, v[i][j]);
}

TEST(METAArray, IndexAddress) {
  Array<uint32_t[32], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i) {
    EXPECT_EQ(&v[0] + i, &v[i]);
    EXPECT_EQ(static_cast<ptrdiff_t>(i), &v[i] - &v[0]);
  }
}

TEST(METAArray2D, IndexAddress) {
  Array<uint32_t[32][16], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i) {
    EXPECT_EQ(&v[0] + i, &v[i]);
    EXPECT_EQ(static_cast<ptrdiff_t>(i), &v[i] - &v[0]);
    for (size_t j = 0 ; j < v.Size(1) ; ++j) {
      EXPECT_EQ(&v[i][0] + j, &v[i][j]);
      EXPECT_EQ(static_cast<ptrdiff_t>(j), &v[i][j] - &v[i][0]);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// Pointer access
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, DirectPointerAccess) {
  Array<uint32_t[32], 16> v;

  for (size_t i = 0 ; i < v.Size() ; ++i)
    *(v + i) = 42 * i;

  for (size_t i = 0 ; i < v.Size(); ++i)
    EXPECT_EQ(42 * i, *(v + i));
}

TEST(METAArray2D, DirectPointerAccess) {
  Array<uint32_t[32][16], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      *(*(v + i) + j) = 42 * i + 25 * j;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      EXPECT_EQ(42 * i + 25 * j, *(*(v + i) + j));
}

TEST(METAArray, IndirectPointerAccess) {
  Array<uint32_t[32], 16> v;

  uint32_t *p = v;

  for (size_t i = 0 ; i < v.Size() ; ++i)
    *p++ = 42 * i;

  for (size_t i = 0 ; i < v.Size() ; ++i)
    EXPECT_EQ(42 * i, v[i]);
}

TEST(METAArray2D, IndirectPointerAccess) {
  Array<uint32_t[32][16], 16> v;

  uint32_t *p = v[0];

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      *p++ = 42 * i + 25 * j;

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      EXPECT_EQ(42 * i + 25 * j, v[i][j]);
}

//////////////////////////////////////////////////////////////////////////////
// Pointer arithmetic
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, PointerArithmetic) {
  Array<int[32], 16> v;

  for (size_t i = 0 ; i < v.Size() ; ++i) {
    EXPECT_EQ(&v[i], v + i);
  }
}

TEST(METAArray2D, PointerArithmetic) {
  Array<int[32][16], 16> v;

  for (size_t i = 0 ; i < v.Size(0) ; ++i) {
    EXPECT_EQ(&v[i], v + i);
    for (size_t j = 0 ; j < v.Size(1) ; ++j) {
      EXPECT_EQ(&v[i][j], v[i] + j);
      EXPECT_EQ(&v[i][j], *(v + i) + j);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// Base address
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, BaseAddressExplicit) {
  Array<int[32], 16> v;
  Array<int[32][12], 16> w;

  EXPECT_TRUE(&v[0] == reinterpret_cast<int*>(v.AddrOf()));
  EXPECT_TRUE(&w[0] == reinterpret_cast<int(*)[12]>(w.AddrOf()));

  EXPECT_FALSE(&v[0] == &w[0][0]);
}

TEST(METAArray, BaseAddressImplicit) {
  Array<int[32], 16> v;
  Array<int[32][12], 16> w;

  EXPECT_TRUE(&v[0] == v);
  EXPECT_TRUE(v == &v[0]);
  EXPECT_TRUE(&w[0] == w);
  EXPECT_TRUE(w == &w[0]);

  EXPECT_FALSE(v == w[0]);
  EXPECT_FALSE(w[0] == v);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, AddressEq) {
  Array<int[32], 16> v;
  Array<int[12], 16> w;

  EXPECT_TRUE(v == v);
  EXPECT_FALSE(v == w);

  EXPECT_FALSE(v == &v[1]);
  EXPECT_TRUE(v == v);
  EXPECT_FALSE(v == &v[-1]);
}

TEST(METAArray, AddressNe) {
  Array<int[32], 16> v;
  Array<int[12], 16> w;

  EXPECT_FALSE(v != v);
  EXPECT_TRUE(v != w);

  EXPECT_TRUE(v != &v[1]);
  EXPECT_FALSE(v != v);
  EXPECT_TRUE(v != &v[-1]);
}

TEST(METAArray, AddressGt) {
  Array<int[32], 16> v;

  EXPECT_FALSE(v > &v[1]);
  EXPECT_FALSE(v > v);
  EXPECT_TRUE(v > &v[-1]);
}

TEST(METAArray, AddressGe) {
  Array<int[32], 16> v;

  EXPECT_FALSE(v >= &v[1]);
  EXPECT_TRUE(v >= v);
  EXPECT_TRUE(v >= &v[-1]);
}

TEST(METAArray, AddressLt) {
  Array<int[32], 16> v;

  EXPECT_TRUE(v < &v[1]);
  EXPECT_FALSE(v < v);
  EXPECT_FALSE(v < &v[-1]);
}

TEST(METAArray, AddressLe) {
  Array<int[32], 16> v;

  EXPECT_TRUE(v <= &v[1]);
  EXPECT_TRUE(v <= v);
  EXPECT_FALSE(v <= &v[-1]);
}

//////////////////////////////////////////////////////////////////////////////
// Pointer difference
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, PointerDiff) {
  Array<int[32], 16> v;

  for (size_t i = 0 ; i < v.Size() ; ++i) {
    EXPECT_EQ(static_cast<ptrdiff_t>(-i), v - &v[i]);
    EXPECT_EQ(static_cast<ptrdiff_t>(i), &v[i] - v);
  }
}

//////////////////////////////////////////////////////////////////////////////
// sizeof operator
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, SizeofOperator) {
  Array<int[32], 16> v;

  EXPECT_LT(sizeof(int[32]), sizeof(v));
  EXPECT_EQ(sizeof(int[32]), v.SizeOf());
  EXPECT_EQ(sizeof(int[32]), sizeof(*v.AddrOf()));
}

//////////////////////////////////////////////////////////////////////////////
// Element type
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, ElementType) {
  StaticAssertTypeEq<int, Array<int[1]>::Element>();
  StaticAssertTypeEq<int, Array<int[1][2]>::Element>();
  StaticAssertTypeEq<int, Array<int[1][2][3]>::Element>();

  StaticAssertTypeEq<char, Array<char[1]>::Element>();
  StaticAssertTypeEq<char, Array<char[1][2]>::Element>();
  StaticAssertTypeEq<char, Array<char[1][2][3]>::Element>();

  StaticAssertTypeEq<uint64_t, Array<uint64_t[1]>::Element>();
  StaticAssertTypeEq<uint64_t, Array<uint64_t[1][2]>::Element>();
  StaticAssertTypeEq<uint64_t, Array<uint64_t[1][2][3]>::Element>();
}

//////////////////////////////////////////////////////////////////////////////
// First/Last element pointers
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, FirstElement) {
  Array<int[32]> v;

  ASSERT_EQ(&v[0], v.First());
}

TEST(METAArray2D, FirstElement) {
  Array<int[32][16]> v;

  ASSERT_EQ(&v[0][0], v.First());
}

TEST(METAArray, LastElement) {
  Array<int[32]> v;

  ASSERT_EQ(&v[31], v.Last());
}

TEST(METAArray2D, LastElement) {
  Array<int[32][16]> v;

  ASSERT_EQ(&v[31][15], v.Last());
}

//////////////////////////////////////////////////////////////////////////////
// Set all elements
//////////////////////////////////////////////////////////////////////////////

TEST(METAArray, Set) {
  Array<int[32]> v;

  v.Set(-42);

  for (size_t i = 0 ; i < v.Size() ; ++i)
    EXPECT_EQ(-42, v[i]);

  unsigned int a = 25;

  v.Set(a);

  for (size_t i = 0 ; i < v.Size() ; ++i)
    EXPECT_EQ(25, v[i]);
}

TEST(METAArray2D, Set) {
  Array<int[32][16]> v;

  v.Set(-42);

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      EXPECT_EQ(-42, v[i][j]);

  unsigned int a = 25;

  v.Set(a);

  for (size_t i = 0 ; i < v.Size(0) ; ++i)
    for (size_t j = 0 ; j < v.Size(1) ; ++j)
      EXPECT_EQ(25, v[i][j]);
}

}  // namespace
