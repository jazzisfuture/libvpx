/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef LIBVPX_IMG_UTILITIES_H_
#define LIBVPX_IMG_UTILITIES_H_

// The function should return "true" most of the time, therefore no early
// break-out is implemented within the match checking process.
static bool compare_img(const vpx_image_t *img1,
                        const vpx_image_t *img2) {
  bool match = (img1->fmt == img2->fmt) &&
               (img1->d_w == img2->d_w) &&
               (img1->d_h == img2->d_h);

  const unsigned int width_y  = img1->d_w;
  const unsigned int height_y = img1->d_h;
  unsigned int i;
  for (i = 0; i < height_y; ++i)
    match = (memcmp(img1->planes[VPX_PLANE_Y] + i * img1->stride[VPX_PLANE_Y],
                    img2->planes[VPX_PLANE_Y] + i * img2->stride[VPX_PLANE_Y],
                    width_y) == 0) && match;
  const unsigned int width_uv  = (img1->d_w + 1) >> 1;
  const unsigned int height_uv = (img1->d_h + 1) >> 1;
  for (i = 0; i <  height_uv; ++i)
    match = (memcmp(img1->planes[VPX_PLANE_U] + i * img1->stride[VPX_PLANE_U],
                    img2->planes[VPX_PLANE_U] + i * img2->stride[VPX_PLANE_U],
                    width_uv) == 0) && match;
  for (i = 0; i < height_uv; ++i)
    match = (memcmp(img1->planes[VPX_PLANE_V] + i * img1->stride[VPX_PLANE_V],
                    img2->planes[VPX_PLANE_V] + i * img2->stride[VPX_PLANE_V],
                    width_uv) == 0) && match;
  return match;
}

// set the img with a constant value
static void set_constant_img(vpx_image_t *img1, const uint8_t set_val) {
  const unsigned int width_y  = img1->d_w;
  const unsigned int height_y = img1->d_h;
  unsigned int i;
  for (i = 0; i < height_y; ++i)
    memset(img1->planes[VPX_PLANE_Y] + i * img1->stride[VPX_PLANE_Y],
                     set_val, width_y);
  const unsigned int width_uv  = (img1->d_w + 1) >> 1;
  const unsigned int height_uv = (img1->d_h + 1) >> 1;
  for (i = 0; i <  height_uv; ++i)
    memset(img1->planes[VPX_PLANE_U] + i * img1->stride[VPX_PLANE_U],
                     set_val + 1, width_uv);
  for (i = 0; i < height_uv; ++i)
    memset(img1->planes[VPX_PLANE_V] + i * img1->stride[VPX_PLANE_V],
                     set_val + 2, width_uv);
}

#endif  // LIBVPX_IMG_UTILITIES_H_
