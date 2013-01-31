/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef LIBVPX_TEST_MD5_HELPER_H_
#define LIBVPX_TEST_MD5_HELPER_H_

extern "C" {
#include "./md5_utils.h"
#include "vpx/vpx_decoder.h"
}

namespace libvpx_test {
class MD5 {
 public:
  MD5() {
    MD5Init(&md5_);
  }

  void Add(const vpx_image_t *img) {
    for (int plane = 0; plane < 3; ++plane) {
      uint8_t *buf = img->planes[plane];
      const int h = plane ? (img->d_h + 1) >> 1 : img->d_h;
      const int w = plane ? (img->d_w + 1) >> 1 : img->d_w;

      for (int y = 0; y < h; ++y) {
        MD5Update(&md5_, buf, w);
        buf += img->stride[plane];
      }
    }
  }

  void Get(char str[33]) {
    static const char hex[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };
    uint8_t tmp[16];

    MD5Final(tmp, &md5_);
    for (int i = 0; i < 16; i++) {
      str[i * 2 + 0]  = hex[tmp[i] >> 4];
      str[i * 2 + 1]  = hex[tmp[i] & 0xf];
    }
    str[32] = 0;
  }

 protected:
  MD5Context md5_;
};

}  // namespace libvpx_test

#endif  // LIBVPX_TEST_MD5_HELPER_H_
