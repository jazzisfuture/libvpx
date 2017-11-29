/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_COMPAT_AVX_H_
#define VPX_PORTS_COMPAT_AVX_H_

#define MM256_SET_M128I(a, b) _mm256_insertf128_si256(_mm256_castsi128_si256(b), (a), 1)
#define MM256_SETR_M128I(a, b) MM256_SET_M128I((b), (a))

#endif  // VPX_PORTS_COMPAT_AVX_H_
