/*
 Copyright (c) 2013 The WebM project authors. All Rights Reserved.

 Use of this source code is governed by a BSD-style license
 that can be found in the LICENSE file in the root of the source
 tree. An additional intellectual property rights grant can be found
 in the file PATENTS.  All contributing project authors may
 be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_VECTORS_H_
#define TEST_VECTORS_H_

#include "./vpx_config.h"

#if CONFIG_VP8_DECODER
extern const char *kVP8TestVectors[62];
#endif
#if CONFIG_VP9_DECODER
#if CONFIG_NON420
extern const char *kVP9TestVectors[214];
#else
extern const char *kVP9TestVectors[213];
#endif
#endif

#endif  // TEST_VECTORS_H_
