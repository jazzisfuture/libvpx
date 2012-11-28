/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef __INC_TEMPORAL_FILTER_H
#define __INC_TEMPORAL_FILTER_H

#if ARCH_X86 || ARCH_X86_64
#include "x86/vp9_temporal_filter_x86.h"
#endif

#ifndef vp9_temporal_filter_apply
#define vp9_temporal_filter_apply vp9_temporal_filter_apply_c
#endif

struct VP9_COMP;

extern void vp9_temporal_filter_prepare_c(struct VP9_COMP *cpi, int distance);

#endif // __INC_TEMPORAL_FILTER_H
