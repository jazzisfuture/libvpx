/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


extern "C" {
//#include "vpx_config.h"
//#include "vpx_rtcd.h"
#include "vp8/common/blockd.h"
#include "vp8/encoder/block.h"
#include "vpx_mem/vpx_mem.h"

extern void vp8_subtract_b_c(BLOCK *be, BLOCKD *bd, int pitch);
}
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {

TEST(VP8, TestSubtract) {
  BLOCK be;
  BLOCKD bd;
  int pitch = 4;

  //Allocate... align to 16 for mmx/sse tests
  unsigned char * source = (unsigned char *)vpx_memalign(16, 4*4*sizeof(unsigned char));
  be.src_diff = (short *)vpx_memalign(16, 4*4*sizeof(short));
  bd.predictor = (unsigned char *)vpx_memalign(16, 4*4*sizeof(unsigned char));

  //start at block0
  be.src = 0;
  be.base_src = &source;

  //set difference
  short * src_diff = be.src_diff;
  for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
    	  src_diff[c] = 0xa5a5;
      }
      src_diff += pitch;
  }

  //set destination
  unsigned char * base_src = *be.base_src;
  for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
    	  base_src[c] = 0x1;
      }
      base_src += pitch;
  }

  //set predictor
  unsigned char * predictor = bd.predictor;
  for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
    	  predictor[c] = 255;
      }
      predictor += pitch;
  }

  vp8_subtract_b_c(&be, &bd, pitch);

  src_diff = be.src_diff;
  for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
          EXPECT_EQ(-254, src_diff[c]) << "r = " << r << ", c = " << c;
      }
      src_diff += pitch;
  }

  // Free allocated memory
  if(be.src_diff)
    vpx_free(be.src_diff);
  if(source)
    vpx_free(source);
  if(bd.predictor)
    vpx_free(bd.predictor);

};

} // namespace
