/*
 Copyright (c) 2012 The WebM project authors. All Rights Reserved.

 Use of this source code is governed by a BSD-style license
 that can be found in the LICENSE file in the root of the source
 tree. An additional intellectual property rights grant can be found
 in the file PATENTS.  All contributing project authors may
 be found in the AUTHORS file in the root of the source tree.
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/decode_test_driver.h"
#include "test/img_utilities.h"
#include "test/ivf_video_source.h"

namespace libvpx_test {

const char *kTestVectors[] = {
  "vp80-00-comprehensive-001.ivf"
};

class Test1 : public libvpx_test::DecoderTest,
    public ::testing::TestWithParam<const char*> {
 protected:
  Test1() {
  }

  virtual ~Test1() {
  }
};

TEST_P(Test1, MemcpyMemcmp) {
  const std::string filename = GetParam();
  // Open compressed video file.
  libvpx_test::IVFVideoSource video(filename);

  video.Init();
  video.Begin();

  vpx_codec_dec_cfg_t dec_cfg = {0};
  Decoder decoder(dec_cfg, 0);

  // get the decoder in the correct state by decoding the first frame
  decoder.DecodeFrame(video.cxdata(), video.frame_size());

  // get the decoded image dimension info
  DxDataIterator dec_iter = decoder.GetDxData();
  const vpx_image_t *img_dec = dec_iter.Next();

  vpx_ref_frame_t ref_test;
  vpx_ref_frame_t get_test;

  vpx_img_alloc(&ref_test.img, VPX_IMG_FMT_I420, img_dec->d_w, img_dec->d_h, 1);
  vpx_img_alloc(&get_test.img, VPX_IMG_FMT_I420, img_dec->d_w, img_dec->d_h, 1);

  const vpx_ref_frame_type_t frame_type[] = { VP8_LAST_FRAME,
      VP8_GOLD_FRAME, VP8_ALTR_FRAME };
  int i;

  for (i = 0; i < 3; ++i) {
    ref_test.frame_type = get_test.frame_type = frame_type[i];

    set_constant_img(&ref_test.img, 0x5 * (i + 1));
    decoder.Control(VP8_SET_REFERENCE, &ref_test);
    set_constant_img(&get_test.img, 0x0);
    decoder.Control(VP8_COPY_REFERENCE, &get_test);

    const bool res = compare_img(&ref_test.img, &get_test.img);
    ASSERT_TRUE(res)<< "Set/Get reference mismatch found.";
  }
}

INSTANTIATE_TEST_CASE_P(GetSetReferenceTest, Test1,
                        ::testing::ValuesIn(kTestVectors));

}  // namespace
