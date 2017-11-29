/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_COMPAT_AVX2_H_
#define VPX_PORTS_COMPAT_AVX2_H_

#include "vpx_ports/compat_avx.h"

#if defined(__clang__)
#if (__clang_major__ > 0 && __clang_major__ < 3) ||            \
    (__clang_major__ == 3 && __clang_minor__ <= 3) ||          \
    (defined(__APPLE__) && defined(__apple_build_version__) && \
     ((__clang_major__ == 4 && __clang_minor__ <= 2) ||        \
      (__clang_major__ == 5 && __clang_minor__ == 0)))
#define MM256_BROADCASTSI128_SI256(x) \
  _mm_broadcastsi128_si256((__m128i const *)&(x))
#else  // clang > 3.3, and not 5.0 on macosx.
#define MM256_BROADCASTSI128_SI256(x) _mm256_broadcastsi128_si256(x)
#endif  // clang <= 3.3
#elif defined(__GNUC__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 6)
#define MM256_BROADCASTSI128_SI256(x) \
  _mm_broadcastsi128_si256((__m128i const *)&(x))
#elif __GNUC__ == 4 && __GNUC_MINOR__ == 7
#define MM256_BROADCASTSI128_SI256(x) _mm_broadcastsi128_si256(x)
#else  // gcc > 4.7
#define MM256_BROADCASTSI128_SI256(x) _mm256_broadcastsi128_si256(x)
#endif  // gcc <= 4.6
#else   // !(gcc || clang)
#define MM256_BROADCASTSI128_SI256(x) _mm256_broadcastsi128_si256(x)
#endif  // __clang__

#endif  // VPX_PORTS_COMPAT_AVX2_H_
