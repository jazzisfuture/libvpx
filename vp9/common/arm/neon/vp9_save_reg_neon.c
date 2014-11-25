/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "vpx_ports/mem.h"

// Unused functions
void vp9_push_neon(int64_t *store) {
  // Fix compiling error reported by '-Wunused-parameter' only
  store = store;

  return;
}

// Unused functions
void vp9_pop_neon(int64_t *store) {
  // Fix compiling error reported by '-Wunused-parameter' only
  store = store;

  return;
}
