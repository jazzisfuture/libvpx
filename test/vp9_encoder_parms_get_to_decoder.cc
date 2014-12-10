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


const unsigned int kWidth  = 160;
const unsigned int kHeight = 90;
const unsigned int kFramerate = 50;
const unsigned int kFrames = 10;
const int kBitrate = 500;
const int kCpuUsed = 2;
const double psnr_threshold = 35.0;

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

int is_extension_y4m(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return 0;
  else
    return !strcmp(dot, ".y4m");
}

class Vp9EncoderParmsGetToDecoder
    : public ::libvpx_test::EncoderTest,
      public ::libvpx_test::CodecTestWith2Params<libvpx_test::TestMode, \
                                                 EncodePerfTestVideo> {
 protected:
  Vp9EncoderParmsGetToDecoder()
      : EncoderTest(GET_PARAM(0)),
        psnr_(0.0),
        nframes_(0),
        encoding_mode_(GET_PARAM(1)) {
  }

  virtual ~Vp9EncoderParmsGetToDecoder() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(encoding_mode_);
    if (encoding_mode_ != ::libvpx_test::kRealTime) {
      cfg_.g_lag_in_frames = 5;
      cfg_.rc_end_usage = VPX_VBR;
    } else {
      cfg_.g_lag_in_frames = 0;
      cfg_.rc_end_usage = VPX_CBR;
    }
    dec_cfg_.threads = 4;
    test_video_param_ = GET_PARAM(2);
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
      encoder->Control(VP9E_SET_FRAME_PARALLEL_DECODING, 1);
      encoder->Control(VP9E_SET_TILE_COLUMNS, 4);
      encoder->Control(VP8E_SET_CPUUSED, kCpuUsed);
      if (encoding_mode_ != ::libvpx_test::kRealTime) {
        encoder->Control(VP8E_SET_ENABLEAUTOALTREF, 1);
        encoder->Control(VP8E_SET_ARNR_MAXFRAMES, 7);
        encoder->Control(VP8E_SET_ARNR_STRENGTH, 5);
        encoder->Control(VP8E_SET_ARNR_TYPE, 3);
      }
    }
  }

  virtual bool HandleDecodeResult(const vpx_codec_err_t res_dec,
                                  const libvpx_test::VideoSource& video,
                                  libvpx_test::Decoder *decoder) {

    vpx_codec_ctx_t* vp9_decoder = decoder->GetDecoder();
    vp9_codec_priv_t* priv = (vp9_codec_priv_t* ) vp9_decoder->priv;
    VP9Decoder * pbi = priv->pbi;

    EXPECT_EQ(pbi->common.frame_parallel_decoding_mode, 1);
    EXPECT_EQ(pbi->common.log2_tile_cols,2);
    EXPECT_EQ(VPX_CODEC_OK, res_dec) << decoder->DecodeError();
    return VPX_CODEC_OK == res_dec;
  }
  double GetAveragePsnr() const {
    if (nframes_)
      return psnr_ / nframes_;
    return 0.0;
  }

  EncodePerfTestVideo test_video_param_;

 private:
  double psnr_;
  unsigned int nframes_;
  libvpx_test::TestMode encoding_mode_;
};

TEST_P(Vp9EncoderParmsGetToDecoder, EndtoEndPSNRTest) {
  cfg_.rc_target_bitrate = kBitrate;
  cfg_.g_error_resilient = 0;
  init_flags_ = VPX_CODEC_USE_PSNR;

  libvpx_test::VideoSource *video;
  if (is_extension_y4m(test_video_param_.name)) {
    video = new libvpx_test::Y4mVideoSource(test_video_param_.name,
                                            0, kFrames);
  } else {
    video = new libvpx_test::YUVVideoSource(test_video_param_.name,
                                            VPX_IMG_FMT_I420,
                                            test_video_param_.width,
                                            test_video_param_.height,
                                            kFramerate, 1, 0, kFrames);
  }

  ASSERT_NO_FATAL_FAILURE(RunLoop(video));
  delete(video);
}

VP9_INSTANTIATE_TEST_CASE(
    Vp9EncoderParmsGetToDecoder,
    ::testing::Values(::libvpx_test::kTwoPassGood, ::libvpx_test::kOnePassGood),
    ::testing::ValuesIn(kVP9EncodePerfTestVectors));

}  // namespace
