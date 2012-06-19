/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdio.h>

#include "third_party/googletest/src/include/gtest/gtest.h"
extern "C" {
  #include "vpx_mem/vpx_mem.h"
  #include "./vpx_rtcd.h"
}

namespace {

// Test routine for the VP8 post-processing function
// vp8_post_proc_down_and_across_c.

TEST(Vp8PostProcessingFilterTest, FilterOutputCheck) {
  unsigned int i;
  unsigned int j;

  // Size of the underlying data block that will be filtered.
  const unsigned int block_width  = 16;
  const unsigned int block_height = 16;

  // 5-tap filter needs 2 padding rows above and below the block in the input.
  const unsigned int input_width = block_width;
  const unsigned int input_height = block_height + 4;
  const unsigned int input_stride = input_width;
  const unsigned int input_size = input_width * input_height;

  // Filter extends output block by 8 samples at left and right edges.
  const unsigned int output_width = block_width + 16;
  const unsigned int output_height = block_height;
  const unsigned int output_stride = output_width;
  const unsigned int output_size = output_width * output_height;

  // Allocate memory for the extended source and destination image blocks.
  unsigned char * src_image = (unsigned char *) vpx_calloc(input_size, 1);
  unsigned char * dst_image = (unsigned char *) vpx_calloc(output_size, 1);

  // Pointers to top-left pixel of block in the input and output images.
  unsigned char * src_image_ptr = src_image + (input_stride << 1);
  unsigned char * dst_image_ptr = dst_image + 8;

  // Initialize pixels in the input:
  //   block pixels to value 1,
  //   border pixels to value 10.
  (void) vpx_memset(src_image, 10, input_size);
  unsigned char * pixel_ptr = src_image_ptr;
  for (i = 0; i < block_height; ++i) {
    for (j = 0; j < block_width; ++j) {
      pixel_ptr[j] = 1;
    }
    pixel_ptr += input_stride;
  }

  // Initialize pixels in the output to 99.
  vpx_memset(dst_image, 99, output_size);

  // Call the filter.
  vp8_post_proc_down_and_across_c(src_image_ptr, dst_image_ptr, input_stride,
                                  output_stride, block_height, block_width,
                                  255);

  // Test filtered pixel values, those in:
  //   rows 0 & 15 should be 3,
  //   rows 1 & 14 should be 2,
  //   rows 2 - 13 should be 1.
  int bad_pixel_count = 0;

  pixel_ptr = dst_image;
  for (j = 0; j < block_width; ++j) {
    if (pixel_ptr[j] != (unsigned char) 3)
      ++bad_pixel_count;
  }

  pixel_ptr = dst_image + output_stride;
  for (j = 0; j < block_width; ++j) {
    if (pixel_ptr[j] != (unsigned char) 2)
      ++bad_pixel_count;
  }

  pixel_ptr = dst_image + (output_stride << 1);
  for (i = 2; i <= 13; ++i) {
    for (j = 0; j < block_width; ++j) {
      if (pixel_ptr[j] != (unsigned char) 1)
        ++bad_pixel_count;
    }
    pixel_ptr += output_stride;
  }

  pixel_ptr = dst_image + 14 * output_stride;
  for (j = 0; j < block_width; ++j) {
    if (pixel_ptr[j] != (unsigned char) 2)
      ++bad_pixel_count;
  }

  pixel_ptr = dst_image + 15 * output_stride;
  for (j = 0; j < block_width; ++j) {
    if (pixel_ptr[j] != (unsigned char) 3)
      ++bad_pixel_count;
  }

  EXPECT_EQ(0, bad_pixel_count)
          << "Vp8PostProcessingFilterTest failed with invalid filter output";

  // Free allocated memory.
  if (src_image)
    vpx_free(src_image);
  if (dst_image)
    vpx_free(dst_image);
};

}  // namespace
