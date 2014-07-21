/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "./vpx_config.h"
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "vpx_ports/vpx_timer.h"
#include "test/y4m_video_source.h"
#include "./vpx_version.h"

namespace {

using std::tr1::make_tuple;
using std::tr1::get;
const int kMaxPsnr = 100;
const double kUsecsInSec = 1000000.0;

/*file name, width, height, and bitrate*/
#define VIDEO_NAME 0
#define VIDEO_WIDTH 1
#define VIDEO_HEIGHT 2
#define TARGET_BITRATE 3
#define VIDEO_FRAMES 4
typedef std::tr1::tuple<const char *, unsigned, unsigned, unsigned, unsigned> encode_perf_test_t;

const encode_perf_test_t kVP9EncodePerfTestVectors[] = {
  make_tuple("desktop_640_360_30.yuv", 640, 360, 200, 2484),
  /*make_tuple("kirland_640_480_30.yuv", 640, 480, 200, 300),
  make_tuple("macmarcomoving_640_480_30.yuv", 640, 480, 200, 987),
  make_tuple("macmarcostationary_640_480_30.yuv", 640, 480, 200, 718),
  make_tuple("niklas_640_480_30.yuv", 640, 480, 200, 471),
  make_tuple("tacomanarrows_640_480_30.yuv", 640, 480, 200, 300),
  make_tuple("tacomasmallcameramovement_640_480_30.yuv", 640, 480, 200, 300),
  make_tuple("thaloundeskmtg_640_480_30.yuv", 640, 480, 200, 300),
  make_tuple("niklas_1280_720_30.yuv", 1280, 720, 600, 470),*/
};

#define LENGTH(x) sizeof((x)) / sizeof((x)[0])

const int encode_perf_test_speeds[] = { 5, 6, 7, 12 };

class VP9EncodePerfTest : public ::libvpx_test::EncoderTest,
    public ::libvpx_test::CodecTestWithParam<libvpx_test::TestMode> {
 protected:
  VP9EncodePerfTest()
      : EncoderTest(GET_PARAM(0)),
        psnr_(kMaxPsnr),
        nframes_(0),
        encoding_mode_(GET_PARAM(1)),
        speed_(0) {
  }

  virtual ~VP9EncodePerfTest() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(encoding_mode_);
  }

  virtual void PreEncodeFrameHook(::libvpx_test::VideoSource *video,
                                  ::libvpx_test::Encoder *encoder) {
    if (video->frame() == 1) {
        encoder->Control(VP8E_SET_CPUUSED, speed_);
    }
  }


  virtual void BeginPassHook(unsigned int /*pass*/) {
    psnr_ = kMaxPsnr;
    nframes_ = 0;
  }

  virtual void PSNRPktHook(const vpx_codec_cx_pkt_t *pkt) {
    if (pkt->data.psnr.psnr[0] < psnr_)
      psnr_= pkt->data.psnr.psnr[0];
  }

  // for performance reasons don't decode
  virtual bool DoDecode() { return 0; }

  double GetMinPsnr() const {
      return psnr_;
  }

  void SetSpeed(unsigned int speed) {
    speed_ = speed;
  }

 private:
  double psnr_;
  unsigned int nframes_;
  libvpx_test::TestMode encoding_mode_;
  unsigned speed_;
};

TEST_P(VP9EncodePerfTest, PerfTest) {
  for (size_t i = 0; i < LENGTH(kVP9EncodePerfTestVectors); i++) {
    for (size_t j = 0; j < LENGTH(encode_perf_test_speeds); j++) {
      SetUp();
      const vpx_rational timebase = { 33333333, 1000000000 };
      cfg_.g_timebase = timebase;
      cfg_.rc_target_bitrate = get<TARGET_BITRATE>(kVP9EncodePerfTestVectors[i]);
      cfg_.g_lag_in_frames = 0;
      cfg_.rc_min_quantizer = 2;
      cfg_.rc_max_quantizer = 56;
      cfg_.rc_dropframe_thresh = 0;
      cfg_.rc_undershoot_pct = 50;
      cfg_.rc_overshoot_pct = 50;
      cfg_.rc_buf_sz = 1000;
      cfg_.rc_buf_initial_sz = 500;
      cfg_.rc_buf_optimal_sz = 600;
      cfg_.rc_resize_allowed = 0;
      cfg_.rc_end_usage = VPX_CBR;

      init_flags_ = VPX_CODEC_USE_PSNR;

      const unsigned frames = get<VIDEO_FRAMES>(kVP9EncodePerfTestVectors[i]);
      const char *video_name = get<VIDEO_NAME>(kVP9EncodePerfTestVectors[i]);
      libvpx_test::I420VideoSource video(
          video_name,
          get<VIDEO_WIDTH>(kVP9EncodePerfTestVectors[i]),
          get<VIDEO_HEIGHT>(kVP9EncodePerfTestVectors[i]),
          timebase.den, timebase.num, 0,
          get<VIDEO_FRAMES>(kVP9EncodePerfTestVectors[i]));
      SetSpeed(encode_perf_test_speeds[j]);

      vpx_usec_timer t;
      vpx_usec_timer_start(&t);

      ASSERT_NO_FATAL_FAILURE(RunLoop(&video));

      vpx_usec_timer_mark(&t);
      const double elapsed_secs = double(vpx_usec_timer_elapsed(&t))
                                  / kUsecsInSec;
      const double fps = double(frames) / elapsed_secs;
      const double psnr = GetMinPsnr();

      printf("{\n");
      printf("\t\"type\" : \"encode_perf_test\",\n");
      printf("\t\"version\" : \"%s\",\n", VERSION_STRING_NOSP);
      printf("\t\"videoName\" : \"%s\",\n", video_name);
      printf("\t\"encodeTimeSecs\" : %f,\n", elapsed_secs);
      printf("\t\"totalFrames\" : %u,\n", frames);
      printf("\t\"framesPerSecond\" : %f,\n", fps);
      printf("\t\"psnr\" : %f,\n", psnr);
      printf("\t\"speed\" : %d\n", encode_perf_test_speeds[j]);
      printf("}\n");
    }
  }
}

VP9_INSTANTIATE_TEST_CASE(VP9EncodePerfTest, ::testing::Values(::libvpx_test::kRealTime));
}  // namespace
