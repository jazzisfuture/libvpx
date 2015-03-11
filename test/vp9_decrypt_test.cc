/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/ivf_video_source.h"

namespace {
// In a real use the 'decrypt_state' parameter will be a pointer to a struct
// with whatever internal state the decryptor uses. For testing we'll just
// xor with a constant key, and decrypt_state will point to the start of
// the original buffer.
const uint8_t test_key[16] = {
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

void encrypt_buffer(const uint8_t *src, uint8_t *dst, size_t size,
                    ptrdiff_t offset) {
  for (size_t i = 0; i < size; ++i) {
    dst[i] = src[i] ^ test_key[(offset + i) & 15];
  }
}

void test_decrypt_cb(void *decrypt_state, const uint8_t *input,
                     uint8_t *output, int count) {
  encrypt_buffer(input, output, count,
                 input - reinterpret_cast<uint8_t *>(decrypt_state));
}

}  // namespace

namespace libvpx_test {

TEST(TestDecrypt, DecryptWorksVp9) {
  libvpx_test::IVFVideoSource video("vp90-2-05-resize.ivf");
  video.Init();

  vpx_codec_dec_cfg_t dec_cfg = vpx_codec_dec_cfg_t();
  VP9Decoder decoder(dec_cfg, 0);

  video.Begin();

  // no decryption
  vpx_codec_err_t res = decoder.DecodeFrame(video.cxdata(), video.frame_size());
  ASSERT_EQ(VPX_CODEC_OK, res) << decoder.DecodeError();

  // decrypt frame
  video.Next();

  std::vector<uint8_t> encrypted(video.frame_size());
  encrypt_buffer(video.cxdata(), &encrypted[0], video.frame_size(), 0);
  vpx_decrypt_init di = { test_decrypt_cb, &encrypted[0] };
  decoder.Control(VPXD_SET_DECRYPTOR, &di);

  res = decoder.DecodeFrame(&encrypted[0], encrypted.size());
  ASSERT_EQ(VPX_CODEC_OK, res) << decoder.DecodeError();
}

TEST(TestDecrypt, DecryptWorksVp9FrameParallel) {
  libvpx_test::IVFVideoSource video("vp90-2-05-resize.ivf");
  video.Init();
  int in_frames = 0;
  int out_frames = 0;

  vpx_codec_dec_cfg_t cfg = vpx_codec_dec_cfg_t();
  cfg.threads = 4;  // Use 4 threads for parallel decode.
  const vpx_codec_flags_t flags = VPX_CODEC_USE_FRAME_THREADING;
  libvpx_test::VP9Decoder decoder(cfg, flags, 0);

  video.Begin();

  do {
    ++in_frames;
    std::vector<uint8_t> encrypted(video.frame_size());
    encrypt_buffer(video.cxdata(), &encrypted[0], video.frame_size(), 0);
    vpx_decrypt_init di = { test_decrypt_cb, &encrypted[0] };
    decoder.Control(VPXD_SET_DECRYPTOR, &di);

    const vpx_codec_err_t res = decoder.DecodeFrame(&encrypted[0], encrypted.size());
    ASSERT_EQ(VPX_CODEC_OK, res) << decoder.DecodeError();

    video.Next();
    // Flush the decoder at the end of the video.
    if (!video.cxdata())
      decoder.DecodeFrame(NULL, 0);

    libvpx_test::DxDataIterator dec_iter = decoder.GetDxData();
    const vpx_image_t *img;

    // Get decompressed data
    while ((img = dec_iter.Next())) {
      ++out_frames;
    }
  } while (video.cxdata() != NULL);

  EXPECT_EQ(in_frames, out_frames) <<
      "Input frame count does not match output frame count";
}

}  // namespace libvpx_test
