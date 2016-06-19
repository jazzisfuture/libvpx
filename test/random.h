/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_RANDOM_H_
#define TEST_RANDOM_H_

#include <stdint.h>

#include <limits>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "test/acm_random.h"
#include "test/array.h"

namespace libvpx_test {

namespace internal {

// Convert to unsigned type (should use std::make_unsigned in C++11)
template<typename T>
struct MakeUnsigned {
  typedef T type;
};

template<typename T>
struct MakeUnsigned<const T> {
  typedef const typename MakeUnsigned<T>::type type;
};

template<> struct MakeUnsigned<int8_t> { typedef uint8_t type; };
template<> struct MakeUnsigned<int16_t> { typedef uint16_t type; };
template<> struct MakeUnsigned<int32_t> { typedef uint32_t type; };
template<> struct MakeUnsigned<int64_t> { typedef uint64_t type; };

}  // namespace internal

// Deterministic random number generator with a higher level interface,
// providing various convenience methods.
class Random {
 public:
  Random() {
    rng_[0].Reset(ACMRandom::DeterministicSeed());
    rng_[1].Reset(ACMRandom::DeterministicSeed() + 1000);
    rng_[2].Reset(ACMRandom::DeterministicSeed() + 2000);
    rng_[3].Reset(ACMRandom::DeterministicSeed() + 3000);
  }

  virtual ~Random() {}

  ////////////////////////////////////////////////////////////////////////////
  // Scalar random numbers of various types
  ////////////////////////////////////////////////////////////////////////////

  // Uniformly distributed random number from the range
  // [std::numeric_limits<R>::min(), and std::numeric_limits<R>::max()]
  // R must be an integral type.
  template<typename R>
  R Uniform();

  // Uniformly distributed random number from the range
  // [0, hi]
  template<typename R>
  R Uniform(R hi) {
    typedef typename internal::MakeUnsigned<R>::type UR;
    assert(hi > 0);
    if (std::numeric_limits<R>::is_signed) {
      return Uniform<UR>() % (static_cast<UR>(hi) + 1);
    } else if (hi == std::numeric_limits<R>::max()) {
      return Uniform<R>();
    } else {
      return Uniform<UR>() % (hi + 1);
    }
  }

  // Uniformly distributed random number from the range
  // [lo, hi]
  template<typename R>
  R Uniform(R lo, R hi) {
    typedef typename internal::MakeUnsigned<R>::type UR;

    if (std::numeric_limits<R>::is_signed) {
      return Uniform<UR>(static_cast<UR>(hi - lo)) + lo;
    } else {
      return Uniform<UR>(hi - lo) + lo;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Random choice (with uniform probability) of one of the arguments
  ////////////////////////////////////////////////////////////////////////////

  // Note: Provide further versions of choice as required
  template<typename T>
  T Choice(T v0, T v1) {
    return Uniform<bool>() ? v1 : v0;
  }

  template<typename T>
  T Choice(T v0, T v1, T v2) {
    switch (Uniform<int>(3)) {
      case 0: return v0;
      case 1: return v1;
      default: return v2;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Filling native arrays with random values
  ////////////////////////////////////////////////////////////////////////////

  // These methods behave like the scalar random number methods, except
  // they do not return a value, but instead take a pointer to an array
  // as the first argument. Each element of this array is then filled
  // with random numbers using the scalar versions of the function.
  // Bound types are the types of the primitive array elements

  template<typename T, size_t n>
  void Uniform(T (*const array)[n]) {
    typedef typename ArrayTypeUtil<T[n]>::Element E;
    E *const first = ArrayTypeUtil<T[n]>::First(array);
    E *const last = ArrayTypeUtil<T[n]>::Last(array);
    for (E *e = first ; e <= last ; ++e)
      *e = Uniform<E>();
  }

  template<typename T, size_t n>
  void Uniform(T (*const array)[n], typename ArrayTypeUtil<T[n]>::Element hi) {
    typedef typename ArrayTypeUtil<T[n]>::Element E;
    E *const first = ArrayTypeUtil<T[n]>::First(array);
    E *const last = ArrayTypeUtil<T[n]>::Last(array);
    for (E *e = first ; e <= last ; ++e)
      *e = Uniform<E>(hi);
  }

  template<typename T, size_t n>
  void Uniform(T (*const array)[n],
               typename ArrayTypeUtil<T[n]>::Element lo,
               typename ArrayTypeUtil<T[n]>::Element hi) {
    typedef typename ArrayTypeUtil<T[n]>::Element E;
    E *const first = ArrayTypeUtil<T[n]>::First(array);
    E *const last = ArrayTypeUtil<T[n]>::Last(array);
    for (E *e = first ; e <= last ; ++e)
      *e = Uniform<E>(lo, hi);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Filling instances of 'Array'
  ////////////////////////////////////////////////////////////////////////////

  template<typename T, int A>
  void Uniform(Array<T, A> *const array) {
    typedef typename Array<T, A>::Element E;
    for (E *e = array->First() ; e <= array->Last() ; ++e)
      *e = Uniform<E>();
  }

  template<typename T, int A>
  void Uniform(Array<T, A> *const array, typename Array<T, A>::Element hi) {
    typedef typename Array<T, A>::Element E;
    for (E *e = array->First() ; e <= array->Last() ; ++e)
      *e = Uniform<E>(hi);
  }

  template<typename T, int A>
  void Uniform(Array<T, A> *const array,
               typename Array<T, A>::Element lo,
               typename Array<T, A>::Element hi) {
    typedef typename Array<T, A>::Element E;
    for (E *e = array->First() ; e <= array->Last() ; ++e)
      *e = Uniform<E>(lo, hi);
  }

 private:
  libvpx_test::ACMRandom rng_[4];
};

// Add further specialisations as necessary

template<>
inline bool Random::Uniform<bool>() {
  return rng_[0].Rand8() & 1 ? true : false;
}

template<>
inline uint8_t Random::Uniform<uint8_t>() {
  return rng_[0].Rand8();
}

template<>
inline uint16_t Random::Uniform<uint16_t>() {
  return rng_[0].Rand16();
}

template<>
inline uint32_t Random::Uniform<uint32_t>() {
  const uint32_t a = rng_[0].Rand16();
  const uint32_t b = rng_[1].Rand16();
  return b << 16 | a;
}

template<>
inline uint64_t Random::Uniform<uint64_t>() {
  const uint64_t a = rng_[0].Rand16();
  const uint64_t b = rng_[1].Rand16();
  const uint64_t c = rng_[2].Rand16();
  const uint64_t d = rng_[3].Rand16();
  return d << 48 | c << 32 | b << 16 | a;
}

template<>
inline int8_t Random::Uniform<int8_t>() { return Uniform<uint8_t>(); }

template<>
inline int16_t Random::Uniform<int16_t>() { return Uniform<uint16_t>(); }

template<>
inline int32_t Random::Uniform<int32_t>() { return Uniform<uint32_t>(); }

template<>
inline int64_t Random::Uniform<int64_t>() { return Uniform<uint64_t>(); }

}  // namespace libvpx_test

#endif  // TEST_RANDOM_H_
