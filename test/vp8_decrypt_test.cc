#include "third_party/googletest/src/include/gtest/gtest.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/decode_test_driver.h"
#include "test/ivf_video_source.h"

#if CONFIG_DECRYPT

namespace {

const uint8_t decrypt_key[32] = {
  255, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

}  // namespace

namespace libvpx_test {

TEST(TestDecrypt, DecryptWorks) {
  libvpx_test::IVFVideoSource video("vp80-00-comprehensive-001.ivf");
  video.Init();

  vpx_codec_dec_cfg_t dec_cfg = {0};
  Decoder decoder(dec_cfg, 0);
  decoder.Init();

  // Zero decrypt key (by default)
  video.Begin();
  vpx_codec_err_t res = decoder.DecodeFrameRaw(video.cxdata(),
                                                video.frame_size());
  ASSERT_EQ(VPX_CODEC_OK, res);

  // Non-zero decrypt key
  video.Next();
  decoder.ControlData(VP8_SET_DECRYPT_KEY, decrypt_key);
  res = decoder.DecodeFrameRaw(video.cxdata(), video.frame_size());
  ASSERT_NE(VPX_CODEC_OK, res);
}

}  // namespace libvpx_test

#endif
