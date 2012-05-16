/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TEST_ENCODER_TEST_H_
#define TEST_ENCODER_TEST_H_
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "video_source.h"
#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

enum TestMode {
  kRealTime,
  kOnePassGood,
  kOnePassBest,
  kTwoPassGood,
  kTwoPassBest
};
#define ALL_TEST_MODES ::testing::Values(kRealTime,\
    kOnePassGood,\
    kOnePassBest,\
    kTwoPassGood,\
    kTwoPassBest)


// Provides an object to handle the libvpx get_cx_data() iteration pattern
class CxDataIterator {
 public:
  explicit CxDataIterator(vpx_codec_ctx_t &encoder)
    : encoder_(encoder), iter_(NULL) {}

  const vpx_codec_cx_pkt_t *Next() {
    return vpx_codec_get_cx_data(&encoder_, &iter_);
  }

 private:
  vpx_codec_ctx_t &encoder_;
  vpx_codec_iter_t  iter_;
};


// Implements an in-memory store for libvpx twopass statistics
class TwopassStatsStore {
 public:
  TwopassStatsStore() {
    buf_.sz = 0;
    buf_.buf = NULL;
    buf_ptr_ = NULL;
    buf_alloc_sz_ = 0;
  }

  void Append(const vpx_codec_cx_pkt_t &pkt) {
    const size_t len = pkt.data.twopass_stats.sz;

    if (buf_.sz + len > buf_alloc_sz_) {
      const size_t  new_sz = buf_alloc_sz_ + 64 * 1024;
      char  *new_ptr = reinterpret_cast<char *>(realloc(buf_.buf, new_sz));

      ASSERT_TRUE(new_ptr);
      buf_ptr_ = new_ptr + (buf_ptr_ - (char *)buf_.buf);
      buf_.buf = new_ptr;
      buf_alloc_sz_ = new_sz;
    }

    memcpy(buf_ptr_, pkt.data.twopass_stats.buf, len);
    buf_.sz += len;
    buf_ptr_ += len;
  }

  vpx_fixed_buf_t buf() const {
    return buf_;
  }

 protected:
  vpx_fixed_buf_t buf_;
  char           *buf_ptr_;
  size_t          buf_alloc_sz_;
};


// Provides a simplified interface to manage one video encoding pass, given
// a configuration and video source.
//
// TODO(jkoleszar): The exact services it provides and the appropriate
// level of abstraction will be fleshed out as more tests are written.
class Encoder {
 public:
  Encoder(vpx_codec_enc_cfg_t cfg, unsigned long deadline,
          TwopassStatsStore *stats)
    : cfg_(cfg), deadline_(deadline), stats_(stats) {
    memset(&encoder_, 0, sizeof(encoder_));
  }

  ~Encoder() {
    vpx_codec_destroy(&encoder_);
  }

  CxDataIterator GetCxData() {
    return CxDataIterator(encoder_);
  }

  // This is a thin wrapper around vpx_codec_encode(), so refer to
  // vpx_encoder.h for its semantics.
  void EncodeFrame(VideoSource *video, unsigned long flags) {
    if (video->img())
      EncodeFrameInternal(video, flags);
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

  // Convenience wrapper for EncodeFrame()
  void EncodeFrame(VideoSource *video) {
    EncodeFrame(video, 0);
  }

  void set_deadline(unsigned long deadline) {
    deadline_ = deadline;
  }

 protected:
  const char *EncoderError() {
    const char *detail = vpx_codec_error_detail(&encoder_);
    return detail ? detail : vpx_codec_error(&encoder_);
  }

  // Encode an image
  void EncodeFrameInternal(VideoSource *video, unsigned long flags) {
    vpx_codec_err_t res;
    vpx_image_t *img = video->img();

    // Handle first frame initialization
    if (!encoder_.priv) {
      cfg_.g_w = img->d_w;
      cfg_.g_h = img->d_h;
      cfg_.g_timebase = video->timebase();
      cfg_.rc_twopass_stats_in = stats_->buf();
      res = vpx_codec_enc_init(&encoder_, &vpx_codec_vp8_cx_algo, &cfg_, 0);
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
    res = vpx_codec_encode(&encoder_,
                           video->img(), video->pts(), video->duration(),
                           flags, deadline_);
    ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
  }

  // Flush the encoder on EOS
  void Flush() {
    vpx_codec_err_t res;

    res = vpx_codec_encode(&encoder_, NULL, 0, 0, 0, deadline_);
    ASSERT_EQ(VPX_CODEC_OK, res) << EncoderError();
  }

  vpx_codec_ctx_t      encoder_;
  vpx_codec_enc_cfg_t  cfg_;
  unsigned long        deadline_;
  TwopassStatsStore   *stats_;
};


// Common test functionality for all Encoder tests.
//
// This class is a mixin which provides the main loop common to all
// encoder tests. It provides hooks which can be overridden by subclasses
// to implement each test's specific behavior, while centralizing the bulk
// of the boilerplate. Note that it doesn't inherit the gtest testing
// classes directly, so that tests can be parameterized differently.
class EncoderTest {
 protected:

  EncoderTest() {
    abort_ = false;
    flags_ = 0;
  }

  // Initialize the cfg_ member with the default configuration.
  void InitializeConfig() {
    vpx_codec_err_t res;

    res = vpx_codec_enc_config_default(&vpx_codec_vp8_cx_algo, &cfg_, 0);
    ASSERT_EQ(VPX_CODEC_OK, res);
  }

  // Map the TestMode enum to the deadline_ and passes_ variables.
  void SetMode(enum TestMode mode) {
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

  // Main loop.
  virtual void RunLoop(VideoSource *video) {
    for (unsigned int pass = 0; pass < passes_; pass++) {

      if (passes_ == 1)
        cfg_.g_pass = VPX_RC_ONE_PASS;
      else if (pass == 0)
        cfg_.g_pass = VPX_RC_FIRST_PASS;
      else
        cfg_.g_pass = VPX_RC_LAST_PASS;

      BeginPassHook(pass);
      Encoder encoder(cfg_, deadline_, &stats_);

      bool again;

      for (video->Begin(), again = true; again; video->Next()) {
        again = video->img() != NULL;

        PreEncodeFrameHook(video);
        encoder.EncodeFrame(video, flags_);

        CxDataIterator iter = encoder.GetCxData();

        while (const vpx_codec_cx_pkt_t *pkt = iter.Next()) {
          again = true;

          if (pkt->kind != VPX_CODEC_CX_FRAME_PKT)
            continue;

          FramePktHook(pkt);
        }

        if (!Continue())
          break;
      }

      EndPassHook();

      if (!Continue())
        break;
    }
  }

  // Hook to be called at the beginning of a pass.
  virtual void BeginPassHook(unsigned int pass) {
  }

  // Hook to be called at the end of a pass.
  virtual void EndPassHook() {
  }

  // Hook to be called before encoding a frame.
  virtual void PreEncodeFrameHook(VideoSource *video) {
  }

  // Hook to be called on every compressed data packet.
  virtual void FramePktHook(const vpx_codec_cx_pkt_t *pkt) {
  }

  // Hook to determine whether the encode loop should continue.
  virtual bool Continue() {
    return !abort_;
  }

  bool                 abort_;
  vpx_codec_enc_cfg_t  cfg_;
  unsigned int         passes_;
  unsigned long        deadline_;
  TwopassStatsStore    stats_;
  unsigned long        flags_;
};

// Macros to be used with ::testing::Combine
#define PARAMS(...) ::testing::TestWithParam< std::tr1::tuple< __VA_ARGS__ > >
#define GET_PARAM(k) std::tr1::get< k >(GetParam())

#endif //TEST_ENCODER_TEST_H_
