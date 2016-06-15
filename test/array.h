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
#include <limits>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "vpx_dsp/vpx_dsp_common.h"

namespace libvpx_test {

//
// Utility template to manipulate type information about native language arrays.
//
// Primary uses:
//  - Derive further static type information
//  - Implement static methods that operate based on static type information.
//
template<typename T>
struct ArrayTypeUtil {
  static const bool is_array = false;
};

// Specialisation for 1 dimensional arrays.
template<typename E, size_t n>
struct ArrayTypeUtil<E[n]> {
  // Type of primitive element (after all indices have been applied).
  typedef E Element;

  // Pointer to first element of p, with type 'Element'.
  static Element *First(E (*p)[n]) { return &(*p)[0]; }

  // Pointer to last element of p, with type 'Element'.
  static Element *Last(E (*p)[n]) { return &(*p)[n-1]; }

  // Number of dimensions. Dimension 0 corresponds to the leftmost index.
  static size_t Dim() { return 1; }

  // Size in dimension 'dim'.
  static size_t Size(size_t dim = 0) { assert(dim == 0); return n; }

  // Is this an array type.
  static const bool is_array = true;
};

// Specialisation for 2+ dimensional arrays.
template<typename E, size_t n, size_t m>
struct ArrayTypeUtil<E[n][m]> {
  typedef typename ArrayTypeUtil<E[m]>::Element Element;

  static Element *First(E(*p)[n][m]) {
    return ArrayTypeUtil<E[m]>::First(&(*p)[0]);
  }

  static Element *Last(E(*p)[n][m]) {
    return ArrayTypeUtil<E[m]>::Last(&(*p)[n-1]);
  }

  static size_t Dim() { return ArrayTypeUtil<E[m]>::Dim() + 1; }

  static size_t Size(size_t dim = 0) {
    return dim > 0 ? ArrayTypeUtil<E[m]>::Size(dim - 1) : n;
  }

  static const bool is_array = true;
};

//
// Abstract base class for 'class Array', so pointers to all valid
// specialisations can be inserted into collections:
//
class ArrayBase {
 public:
  virtual ~ArrayBase() = 0;
};

inline ArrayBase::~ArrayBase() {}

//
//  'class Array' is a proxy (or wrapper) for a native language array of
//  arithmetic types (arithmetic types are those, for which
//  std::numeric_limits<T>::is_specialized is true, this includes
//  integers and characters of various length, bool, float, double and
//  long double).
//
//  The proxy provides:
//    - Rich methods to achieve common tasks involving arrays,
//      for example set all elements of the array to the same value.
//    - Ability to specify an address alignment for the array.
//      This is implemented relying only on standard C++ language
//      features, so it works in all contexts, including where
//      compiler specific alignment attributes might not be
//      available (the prime example being alignments of object
//      members).
//
//  An instance of Array behaves as a native language array in most
//  contexts. Notable exceptions to this are listed below. Storage for
//  the wrapped array is contained directly within the instance of Array
//  itself.
//
//  Example:
//    To define an array of 5 integers, aligned at a 16 byte address
//    boundary, one can use:
//
//      Array<int[5], 16> arr;
//
//    After this, 'arr' should in most contexts behave as a native
//    language array (as if it was declared as 'int arr[5]':
//
//      arr[1] = 5;
//      arr[2] = *(arr + 1) * 3;  /* Same as: arr[2] = a[1] * 3 */
//
//      int *p = &arr[-1];
//      p++;
//      if (p == arr) { /* This will be executed */ }
//
//      assert(&arr[0] == arr);
//
//    It is guaranteed that &arr[0] is aligned at least at the
//    specified address boundary. Omitting the alignment parameter
//    will imply no alignment constraint, therefore if only the richer
//    interface is required, it is possible to declare an instance as:
//
//      Array<int[3]> arr;        /* Unknown alignment, but rich methods */
//
//    Multidimensional arrays work as expected:
//
//      Array<char[7][9]> arr;    /* 7 rows and 9 columns worth of 'char's */
//
//      arr[0][0] = 1;
//
//  Notable differences from native arrays:
//    - Initialiser lists cannot be used:
//
//        Array<int[3]> arr = { 1, 2, 3 };  /* Won't compile */
//
//    - The unary '&' (address of) operator will not return a pointer
//      to the underlying array, but a pointer to the wrapper itself.
//      While we could overload '&' to return a pointer to the underlying
//      type, this can cause unexpected behaviour, and is against the
//      coding style. To get a pointer to the underlying array, use the
//      'AddrOf' method instead.
//
//        Array<int[3]> arr;
//        Array<int[3]> *pArr = &arr;       // has type 'Array<int[3]>*'
//        int (*pUnder)[3] = arr.AddrOf();  // has type 'int(*)[3]'
//
//    - The sizeof operator will not return the size of the underlying
//      array if invoked on the type or instance directly:
//
//        assert(sizeof(Array<int[3]>) != sizeof(int[3]));
//
//        Array<int[3]> arr;
//        assert(sizeof(arr) != sizeof(int[3]));
//
//    - You can use the 'SizeOf' method to get the desired value:
//
//        assert(arr.SizeOf() == sizeof(int[3]));
//
//  Declaring an 'Array' which does not wrap an array of primitive
//  arithmetic types will fail with a compile time assertion.
//

// Generic template to catch and fail at compile time with non-array types.
template<typename T, int Align = 1>
class Array {
  // Template expanded with a non-array type.
  GTEST_COMPILE_ASSERT_(
      ArrayTypeUtil<T>::is_array,
      class_Array_must_wrap_a_native_array_type);
};

// Specialisation for actual array types
template<typename E, size_t n, int Align>
class Array<E[n], Align> : public ArrayBase {
 private:
  typedef ArrayTypeUtil<E[n]> TypeUtil;

  // Expanded expanded with an array of non-numeric types.
  GTEST_COMPILE_ASSERT_(
      std::numeric_limits<typename TypeUtil::Element>::is_specialized,
      class_Array_must_wrap_a_native_array_of_numeric_type);

  // Alignment must be a power of 2
  GTEST_COMPILE_ASSERT_(
      IS_POWER_OF_TWO(Align),
      alignment_of_class_Array_must_be_a_power_of_2);

 public:
  // Type of the primitive element type. If wrapping a multi-dimensional
  // array, this type always resolves to the final non-array element type.
  // E.g.: 'Array<int[1][2]...[n]>::Element' will be 'int'.
  typedef typename TypeUtil::Element Element;

  Array() : addr_(AlignedStorage()) {}

  ~Array() {}

  // Alignment of array
  static int Alignment() { return Align; }

  // 'sizeof' of underlying array
  static size_t SizeOf() { return sizeof(E[n]); }

  // Number of array dimensions
  static size_t Dim() { return TypeUtil::Dim(); }

  // Size of array in specific dimension. Dimension 0 corresponds to
  // the leftmost index.
  static size_t Size(size_t dim = 0) {
    assert(dim < Dim());
    return TypeUtil::Size(dim);
  }

  // Pointer to underlying type
  E(*AddrOf() const)[n] { return addr_; }

  // Conversion to pointer to first element
  operator E*() const { return &(*addr_)[0]; }

  // Pointer to first primitive element
  Element *First() const { return TypeUtil::First(addr_); }

  // Pointer to last primitive element
  Element *Last() const { return TypeUtil::Last(addr_); }

  // Set all elements to 'value'
  template<typename T>
  void Set(const T &value) {
    for (Element *e = First() ; e <= Last() ; ++e)
      *e = value;
  }

 private:
  Array(const Array &);
  void operator=(const Array &);

  E(*AlignedStorage() const)[n] {
    const intptr_t A = Align;
    const intptr_t a = (reinterpret_cast<intptr_t>(storage_) + A - 1) & -A;
    return reinterpret_cast<E(*)[n]>(a);
  }

  E (*const addr_)[n];

  char storage_[sizeof(E[n]) + Align - 1];
};

}   // namespace libvpx_test

#endif  // TEST_ALIGNED_H_
