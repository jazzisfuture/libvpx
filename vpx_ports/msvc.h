/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_PORTS_MSVC_H_
#define VPX_PORTS_MSVC_H_

#include "./vpx_config.h"

#ifdef _MSC_VER
# include <math.h>  // the ceil() definition must precede intrin.h
# if _MSC_VER > 1310 && (defined(_M_X64) || defined(_M_IX86))
#  include <intrin.h>
#  define USE_MSC_INTRI
# endif  // _MSC_VER > 1310 && (defined(_M_X64) || defined(_M_IX86)
# if _MSC_VER < 1900  // VS2015 provides snprint
#  define snprintf _snprintf
# endif  // _MSC_VER < 1900
#endif  // _MSC_VER

#endif  // VPX_PORTS_MSVC_H_
