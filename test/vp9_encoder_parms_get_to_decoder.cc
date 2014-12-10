/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/y4m_video_source.h"
#include "test/yuv_video_source.h"
#include "test/util.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "vpx/internal/vpx_codec_internal.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_decoder.h"

#include "vp9/common/vp9_frame_buffers.h"

#include "vp9/decoder/vp9_decoder.h"
#include "vp9/decoder/vp9_decodeframe.h"
#include "vp9/decoder/vp9_read_bit_buffer.h"

#include "vp9/vp9_iface_common.h"
typedef vpx_codec_stream_info_t vp9_stream_info_t;
struct vpx_codec_alg_priv {
  vpx_codec_priv_t        base;
  vpx_codec_dec_cfg_t     cfg;
  vp9_stream_info_t       si;
  struct VP9Decoder *pbi;
  int                     postproc_cfg_set;
  vp8_postproc_cfg_t      postproc_cfg;
  vpx_decrypt_cb          decrypt_cb;
  void                   *decrypt_state;
  vpx_image_t             img;
  int                     img_avail;
  int                     flushed;
  int                     invert_tile_order;
  int                     frame_parallel_decode;  // frame-based threading.

  // External frame buffer info to save for VP9 common.
  void *ext_priv;  // Private data associated with the external frame buffers.
  vpx_get_frame_buffer_cb_fn_t get_ext_fb_cb;
  vpx_release_frame_buffer_cb_fn_t release_ext_fb_cb;
};
typedef       struct vpx_codec_alg_priv  vp9_codec_priv_t;

namespace {


const unsigned int kFramerate = 50;
const int kCpuUsed = 2;

struct EncodePerfTestVideo {
  const char *name;
  uint32_t width;
  uint32_t height;
  uint32_t bitrate;
  int frames;
};


const EncodePerfTestVideo kVP9EncodePerfTestVectors[] = {
    {"niklas_1280_720_30.yuv", 1280, 720, 600, 10},
};

struct EncodeParameters {
  uint32_t profile;
  uint32_t tile_rows;
  uint32_t tile_cols;
  uint32_t lossless;
  uint32_t error_resilient;
  uint32_t frame_parallel;
  // TODO(JBB): quantizers / bitrate
};
const EncodeParameters kVP9EncodeParameterSet[] = {
    {0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 2, 2, 0, 0, 1},
    {0, 0, 0, 1, 0, 0},
    // TODO(JBB): Test profiles ( requires more work).
};

int is_extension_y4m(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return 0;
  else
    return !strcmp(dot, ".y4m");
}

class Vp9EncoderParmsGetToDecoder
    : public ::libvpx_test::EncoderTest,
      public ::libvpx_test::CodecTestWith2Params<EncodeParameters, \
                                                 EncodePerfTestVideo> {
 protected:
  Vp9EncoderParmsGetToDecoder()
      : EncoderTest(GET_PARAM(0)),
        psnr_(0.0),
        nframes_(0),
        encode_parms(GET_PARAM(1)) {
  }

  virtual ~Vp9EncoderParmsGetToDecoder() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(::libvpx_test::kTwoPassGood);
    cfg_.g_lag_in_frames = 25;
    cfg_.g_error_resilient = encode_parms.error_resilient;
    cfg_.g_profile = encode_parms.profile;
    dec_cfg_.threads = 4;
    test_video_ = GET_PARAM(2);
    cfg_.rc_target_bitrate = test_video_.bitrate;
  }

  virtual void BeginPassHook(unsigned int) {
    psnr_ = 0.0;
    nframes_ = 0;
  }

  virtual void PSNRPktHook(const vpx_codec_cx_pkt_t *pkt) {
    psnr_ += pkt->data.psnr.psnr[0];
    nframes_++;
  }

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(VP9E_SET_LOSSLESS, encode_parms.lossless);
      encoder->Control(VP9E_SET_FRAME_PARALLEL_DECODING,
                       encode_parms.frame_parallel);
      encoder->Control(VP9E_SET_TILE_ROWS, encode_parms.tile_rows);
      encoder->Control(VP9E_SET_TILE_COLUMNS, encode_parms.tile_cols);
      encoder->Control(VP8E_SET_CPUUSED, kCpuUsed);
      encoder->Control(VP8E_SET_ENABLEAUTOALTREF, 1);
      encoder->Control(VP8E_SET_ARNR_MAXFRAMES, 7);
      encoder->Control(VP8E_SET_ARNR_STRENGTH, 5);
      encoder->Control(VP8E_SET_ARNR_TYPE, 3);
    }
  }

  virtual bool HandleDecodeResult(const vpx_codec_err_t res_dec,
                                  const libvpx_test::VideoSource& video,
                                  libvpx_test::Decoder *decoder) {
    vpx_codec_ctx_t* vp9_decoder = decoder->GetDecoder();
    vp9_codec_priv_t* priv = (vp9_codec_priv_t*) vp9_decoder->priv;
    VP9Decoder* pbi = priv->pbi;
    VP9_COMMON* common = &pbi->common;
    if (encode_parms.lossless) {
      EXPECT_EQ(common->base_qindex, 0);
      EXPECT_EQ(common->y_dc_delta_q, 0);
      EXPECT_EQ(common->uv_dc_delta_q, 0);
      EXPECT_EQ(common->uv_ac_delta_q, 0);
      EXPECT_EQ(common->tx_mode, ONLY_4X4);
    }
    EXPECT_EQ(common->error_resilient_mode, encode_parms.error_resilient);
    if (encode_parms.error_resilient)
      EXPECT_EQ(common->frame_parallel_decoding_mode, 1);
    else
      EXPECT_EQ(common->frame_parallel_decoding_mode,
                encode_parms.frame_parallel);

    EXPECT_EQ(common->log2_tile_cols, encode_parms.tile_cols);
    EXPECT_EQ(common->log2_tile_rows, encode_parms.tile_rows);
    EXPECT_EQ(common->profile, encode_parms.profile);

    EXPECT_EQ(VPX_CODEC_OK, res_dec) << decoder->DecodeError();
    return VPX_CODEC_OK == res_dec;
  }
  double GetAveragePsnr() const {
    if (nframes_)
      return psnr_ / nframes_;
    return 0.0;
  }

  EncodePerfTestVideo test_video_;

 private:
  double psnr_;
  unsigned int nframes_;
  EncodeParameters encode_parms;
};

TEST_P(Vp9EncoderParmsGetToDecoder, EndtoEndPSNRTest) {
  init_flags_ = VPX_CODEC_USE_PSNR;

  libvpx_test::VideoSource *video;
  if (is_extension_y4m(test_video_.name)) {
    video = new libvpx_test::Y4mVideoSource(test_video_.name,
                                            0, test_video_.frames);
  } else {
    video = new libvpx_test::YUVVideoSource(test_video_.name,
                                            VPX_IMG_FMT_I420,
                                            test_video_.width,
                                            test_video_.height,
                                            kFramerate, 1, 0,
                                            test_video_.frames);
  }

  ASSERT_NO_FATAL_FAILURE(RunLoop(video));
  delete(video);
}

VP9_INSTANTIATE_TEST_CASE(
    Vp9EncoderParmsGetToDecoder,
    ::testing::ValuesIn(kVP9EncodeParameterSet),
    ::testing::ValuesIn(kVP9EncodePerfTestVectors));

}  // namespace
