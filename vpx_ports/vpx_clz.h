/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_VPX_CLZ_H_
#define VPX_PORTS_VPX_CLZ_H_
#include <limits.h>

#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
         ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#if __GNUC_PREREQ(3, 4)
/* Note the casts to (int) below: this prevents VPX_CLZ{32|64}_OFFS from
 * "upgrading" the type of an entire expression to an (unsigned) size_t.
 */
# if INT_MAX >= 2147483647
#  define VPX_CLZ32_OFFS ((int)sizeof(unsigned)*CHAR_BIT)
#  define VPX_CLZ32(x) (__builtin_clz(x))
# elif LONG_MAX >= 2147483647L
#  define VPX_CLZ32_OFFS ((int)sizeof(unsigned long)*CHAR_BIT)
#  define VPX_CLZ32(x) (__builtin_clzl(x))
# endif
#elif __MSVC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
static int clz(unsigned x) {
  int lz = 0;
  _BitScanForward(&lz, x);
  return lz - 1;
}
#define VPX_CLZ32_OFFS ((int)sizeof(unsigned)*CHAR_BIT)
#define VPX_CLZ32(x) (clz(x))
#endif

#if defined(VPX_CLZ32)
# define VPX_ILOGNZ_32(v) (VPX_CLZ32_OFFS - VPX_CLZ32(v))
# define VPX_ILOG_32(v)   (VPX_ILOGNZ_32(v) & -!!(v))
#else
# error "Need __builtin_clz or equivalent."
#endif

#endif /* VPX_PORTS_VPX_CLZ_H_ */
