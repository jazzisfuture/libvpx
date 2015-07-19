/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_COMMON_MIPS_DSPR2_H_
#define VPX_COMMON_MIPS_DSPR2_H_

#include <assert.h>
#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

#ifdef __cplusplus
extern "C" {
#endif
#if HAVE_DSPR2
#define CROP_WIDTH 512

uint8_t vp9_ff_cropTbl_a[256 + 2 * CROP_WIDTH];
uint8_t *vp9_ff_cropTbl;

void vp9_dsputil_static_init(void) {
  int i;

  for (i = 0; i < 256; i++) vp9_ff_cropTbl_a[i + CROP_WIDTH] = i;

  for (i = 0; i < CROP_WIDTH; i++) {
    vp9_ff_cropTbl_a[i] = 0;
    vp9_ff_cropTbl_a[i + CROP_WIDTH + 256] = 255;
  }

  vp9_ff_cropTbl = &vp9_ff_cropTbl_a[CROP_WIDTH];
}

static INLINE void prefetch_load(const unsigned char *src) {
  __asm__ __volatile__ (
      "pref   0,  0(%[src])   \n\t"
      :
      : [src] "r" (src)
  );
}

/* prefetch data for store */
static INLINE void prefetch_store(unsigned char *dst) {
  __asm__ __volatile__ (
      "pref   1,  0(%[dst])   \n\t"
      :
      : [dst] "r" (dst)
  );
}

static INLINE void prefetch_load_streamed(const unsigned char *src) {
  __asm__ __volatile__ (
      "pref   4,  0(%[src])   \n\t"
      :
      : [src] "r" (src)
  );
}

/* prefetch data for store */
static INLINE void prefetch_store_streamed(unsigned char *dst) {
  __asm__ __volatile__ (
      "pref   5,  0(%[dst])   \n\t"
      :
      : [dst] "r" (dst)
  );
}
#endif  // #if HAVE_DSPR2
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VPX_COMMON_MIPS_DSPR2_H_
