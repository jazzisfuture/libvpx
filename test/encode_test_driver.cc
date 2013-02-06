/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "vpx_config.h"
#include "test/encode_test_driver.h"
#if CONFIG_VP8_DECODER
#include "test/decode_test_driver.h"
#endif
#include "test/img_utilities.h"
#include "test/register_state_check.h"
#include "test/video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace libvpx_test {
void Encoder::EncodeFrame(VideoSource *video, const unsigned long frame_flags) {
  if (video->img())
    EncodeFrameInternal(*video, frame_flags);
  else
    Flush();

  // Handle twopass stats
  CxDataIterator iter = GetCxData();

  while (const vpx_codec_cx_pkt_t *pkt = iter.Next()) {
    if (pkt->kind != VPX_CODEC_STATS_PKT)
      continue;

    stats_->Append(*pkt);
  }
}

void Encoder::EncodeFrameInternal(const VideoSource &video,
                                  const unsigned long frame_flags) {
  vpx_codec_err_t res;
  const vpx_image_t *img = video.img();

  // Handle first frame initialization
  if (!encoder_.priv) {
    cfg_.g_w = img->d_w;
    cfg_.g_h = img->d_h;
    cfg_.g_timebase = video.timebase();
    cfg_.rc_twopass_stats_in = stats_->buf();
    res = vpx_codec_enc_init(&encoder_, &vpx_codec_vp8_cx_algo, &cfg_,
                             init_flags_);
    ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
  }

  // Handle frame resizing
  if (cfg_.g_w != img->d_w || cfg_.g_h != img->d_h) {
    cfg_.g_w = img->d_w;
    cfg_.g_h = img->d_h;
    res = vpx_codec_enc_config_set(&encoder_, &cfg_);
    ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
  }

  // Encode the frame
  REGISTER_STATE_CHECK(
      res = vpx_codec_encode(&encoder_,
                             video.img(), video.pts(), video.duration(),
                             frame_flags, deadline_));
  ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
}

void Encoder::Flush() {
  const vpx_codec_err_t res = vpx_codec_encode(&encoder_, NULL, 0, 0, 0,
                                               deadline_);
  ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
}

void EncoderTest::SetMode(TestMode mode) {
  switch (mode) {
    case kRealTime:
      deadline_ = VPX_DL_REALTIME;
      break;

    case kOnePassGood:
    case kTwoPassGood:
      deadline_ = VPX_DL_GOOD_QUALITY;
      break;

    case kOnePassBest:
    case kTwoPassBest:
      deadline_ = VPX_DL_BEST_QUALITY;
      break;

    default:
      ASSERT_TRUE(false) << "Unexpected mode " << mode;
  }

  if (mode == kTwoPassGood || mode == kTwoPassBest)
    passes_ = 2;
  else
    passes_ = 1;
}

void EncoderTest::RunLoop(VideoSource *video) {
#if CONFIG_VP8_DECODER
  vpx_codec_dec_cfg_t dec_cfg = {0};
#endif

  stats_.Reset();

  for (unsigned int pass = 0; pass < passes_; pass++) {
    last_pts_ = 0;

    if (passes_ == 1)
      cfg_.g_pass = VPX_RC_ONE_PASS;
    else if (pass == 0)
      cfg_.g_pass = VPX_RC_FIRST_PASS;
    else
      cfg_.g_pass = VPX_RC_LAST_PASS;

    BeginPassHook(pass);
    Encoder encoder(cfg_, deadline_, init_flags_, &stats_);
#if CONFIG_VP8_DECODER
    Decoder decoder(dec_cfg, 0);
    bool has_cxdata = false;
#endif
    bool again;
    for (again = true, video->Begin(); again; video->Next()) {
      again = video->img() != NULL;

      PreEncodeFrameHook(video);
      PreEncodeFrameHook(video, &encoder);
      encoder.EncodeFrame(video, frame_flags_);

      CxDataIterator iter = encoder.GetCxData();

      while (const vpx_codec_cx_pkt_t *pkt = iter.Next()) {
        again = true;

        switch (pkt->kind) {
          case VPX_CODEC_CX_FRAME_PKT:
#if CONFIG_VP8_DECODER
            has_cxdata = true;
            decoder.DecodeFrame((const uint8_t*)pkt->data.frame.buf,
                                pkt->data.frame.sz);
#endif
            ASSERT_GE(pkt->data.frame.pts, last_pts_);
            last_pts_ = pkt->data.frame.pts;
            FramePktHook(pkt);
            break;

          case VPX_CODEC_PSNR_PKT:
            PSNRPktHook(pkt);
            break;

          default:
            break;
        }
      }

#if CONFIG_VP8_DECODER
      if (has_cxdata) {
        const vpx_image_t *img_enc = encoder.GetPreviewFrame();
        DxDataIterator dec_iter = decoder.GetDxData();
        const vpx_image_t *img_dec = dec_iter.Next();
        if(img_enc && img_dec) {
          const bool res = compare_img(img_enc, img_dec);
          ASSERT_TRUE(res)<< "Encoder/Decoder mismatch found.";
        }
      }
#endif
      if (!Continue())
        break;
    }

    EndPassHook();

    if (!Continue())
      break;
  }
}
}  // namespace libvpx_test
