/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vpx_ports/config.h"

#if CONFIG_NEWBESTREFMV
unsigned int vp8_sad2x16_c(
  const unsigned char *src_ptr,
  int  src_stride,
  const unsigned char *ref_ptr,
  int  ref_stride,
  int max_sad);
unsigned int vp8_sad16x2_c(
  const unsigned char *src_ptr,
  int  src_stride,
  const unsigned char *ref_ptr,
  int  ref_stride,
  int max_sad);
#endif
