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
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
extern "C" {
#include "./md5_utils.h"
#include "vpx_mem/vpx_mem.h"
}

namespace {
class TileIndependenceTest : public ::libvpx_test::EncoderTest,
    public ::libvpx_test::CodecTestWithParam<int> {
 protected:
  TileIndependenceTest() : EncoderTest(GET_PARAM(0)), n_tiles_(GET_PARAM(1)) {
    init_flags_ = VPX_CODEC_USE_PSNR;
    MD5Init(&md5_fw_order_);
    MD5Init(&md5_inv_order_);
    vpx_codec_dec_cfg_t cfg;
    cfg.w = 352;
    cfg.h = 288;
    cfg.threads = 1;
    cfg.inv_tile_order = 0;
    fw_dec_ = ::libvpx_test::VP9CodecFactory().CreateDecoder(cfg, 0);
    cfg.inv_tile_order = 1;
    inv_dec_ = ::libvpx_test::VP9CodecFactory().CreateDecoder(cfg, 0);
  }

  virtual ~TileIndependenceTest() {
    delete fw_dec_;
    delete inv_dec_;
  }

  virtual void SetUp() {
    InitializeConfig();
    SetMode(libvpx_test::kTwoPassGood);
  }

  virtual void PreEncodeFrameHook(libvpx_test::VideoSource *video,
                                  libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(VP9E_SET_TILE_COLUMNS, n_tiles_);
    }
  }

  void UpdateMD5(::libvpx_test::Decoder *dec, const vpx_codec_cx_pkt_t *pkt,
                 MD5Context *md5) {
    dec->DecodeFrame((uint8_t *) pkt->data.frame.buf, pkt->data.frame.sz);

    const vpx_image_t *img = dec->GetDxData().Next();
    // Compute and update md5 for each raw in decompressed data.
    for (int plane = 0; plane < 3; ++plane) {
      uint8_t *buf = img->planes[plane];
      const int h = plane ? (img->d_h + 1) >> 1 : img->d_h;
      const int w = plane ? (img->d_w + 1) >> 1 : img->d_w;

      for (int y = 0; y < h; ++y) {
        MD5Update(md5, buf, w);
        buf += img->stride[plane];
      }
    }
  }

  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
    UpdateMD5(fw_dec_, pkt, &md5_fw_order_);
    UpdateMD5(inv_dec_, pkt, &md5_inv_order_);
  }

 private:
  int n_tiles_;
 protected:
  MD5Context md5_fw_order_, md5_inv_order_;
  ::libvpx_test::Decoder *fw_dec_, *inv_dec_;
};

// run an encode with 2 or 4 tiles, and do the decode both in normal and
// inverted tile ordering. Ensure that the MD5 of the output in both cases
// is identical. If so, tiles are considered independent and the test passes.
TEST_P(TileIndependenceTest, MD5Match) {
  const vpx_rational timebase = { 33333333, 1000000000 };
  cfg_.g_timebase = timebase;
  cfg_.rc_target_bitrate = 500;
  cfg_.g_lag_in_frames = 25;
  cfg_.rc_end_usage = VPX_VBR;

  libvpx_test::I420VideoSource video("hantro_collage_w352h288.yuv", 352, 288,
                                     timebase.den, timebase.num, 0, 30);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));

  uint8_t md5_fw[16], md5_inv[16];
  MD5Final(md5_fw,  &md5_fw_order_);
  MD5Final(md5_inv, &md5_inv_order_);
  static const char hex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  };
  char md5_fw_str[33], md5_inv_str[33];
  for (int i = 0; i < 16; i++) {
    md5_fw_str[i * 2 + 0]  = hex[md5_fw[i]  >> 4];
    md5_fw_str[i * 2 + 1]  = hex[md5_fw[i]  & 0xf];
    md5_inv_str[i * 2 + 0] = hex[md5_inv[i] >> 4];
    md5_inv_str[i * 2 + 1] = hex[md5_inv[i] & 0xf];
  }
  md5_fw_str[32] = md5_inv_str[32] = 0;

  // could use ASSERT_EQ(!memcmp(.., .., 16) here, but this gives nicer
  // output if it fails. Not sure if it's helpful since it's really just
  // a MD5...
  ASSERT_STREQ(md5_fw_str, md5_inv_str);
}

VP9_INSTANTIATE_TEST_CASE(TileIndependenceTest,
                          ::testing::Values(VP8_TWO_TILE_COLUMNS,
                                            VP8_FOUR_TILE_COLUMNS));

}  // namespace
