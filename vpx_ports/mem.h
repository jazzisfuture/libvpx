/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_PORTS_MEM_H_
#define VPX_VPX_PORTS_MEM_H_

#include "vpx_config.h"
#include "vpx/vpx_integer.h"

#if (defined(__GNUC__) && __GNUC__) || defined(__SUNPRO_C)
#define DECLARE_ALIGNED(n, typ, val) typ val __attribute__((aligned(n)))
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n, typ, val) __declspec(align(n)) typ val
#else
#warning No alignment directives known for this compiler.
#define DECLARE_ALIGNED(n, typ, val) typ val
#endif

#if HAVE_NEON && defined(_MSC_VER)
#define __builtin_prefetch(x)
#endif

/* Shift down with rounding */
#define ROUND_POWER_OF_TWO(value, n) (((value) + (1 << ((n)-1))) >> (n))
#define ROUND64_POWER_OF_TWO(value, n) (((value) + (1ULL << ((n)-1))) >> (n))

#define ALIGN_POWER_OF_TWO(value, n) \
  (((value) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))

#define CONVERT_TO_SHORTPTR(x) ((uint16_t *)(((uintptr_t)(x)) << 1))
#define CAST_TO_SHORTPTR(x) ((uint16_t *)((uintptr_t)(x)))
#if CONFIG_VP9_HIGHBITDEPTH
#define CONVERT_TO_BYTEPTR(x) ((uint8_t *)(((uintptr_t)(x)) >> 1))
#define CAST_TO_BYTEPTR(x) ((uint8_t *)((uintptr_t)(x)))
#endif  // CONFIG_VP9_HIGHBITDEPTH

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif  // !defined(__has_feature)

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define VPX_WITH_ASAN 1
#else
#define VPX_WITH_ASAN 0
#endif  // __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)

#if __has_attribute(uninitialized)
// Attribute disables -ftrivial-auto-var-init=pattern for specific variables.
// -ftrivial-auto-var-init is security risk mitigation feature, so attribute
// should not be used "just in case", but only to fix real performance
// bottlenecks when other approaches do not work. In general compiler is quite
// effective eleminating unneeded initializations introduced by the flag, e.g.
// when they are followed by actual initialization by a program.
// However if compiler optimization fails and code refactoring is hard, the
// attribute can be used as a workaround.
#define VPX_UNINITIALIZED __attribute__((uninitialized))
#else
#define VPX_UNINITIALIZED
#endif  // __has_attribute(uninitialized)

#endif  // VPX_VPX_PORTS_MEM_H_
