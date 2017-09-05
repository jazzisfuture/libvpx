/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "./vpx_scale_rtcd.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/vpx_scale_test.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/vpx_timer.h"
#include "vpx_scale/yv12config.h"

namespace libvpx_test {

typedef void (*ScaleFrameFunc)(const YV12_BUFFER_CONFIG *src,
                               YV12_BUFFER_CONFIG *dst,
                               INTERP_FILTER filter_type, int phase_scaler);

class ScaleTest : public VpxScaleBase,
                  public ::testing::TestWithParam<ScaleFrameFunc> {
 public:
  virtual ~ScaleTest() {}

 protected:
  virtual void SetUp() { scale_fn_ = GetParam(); }

  void ReferenceScaleFrame(INTERP_FILTER filter_type, int phase_scaler) {
    vp9_scale_and_extend_frame_c(&img_, &ref_img_, filter_type, phase_scaler);
  }

  void ScaleFrame(INTERP_FILTER filter_type, int phase_scaler) {
    ASM_REGISTER_STATE_CHECK(
        scale_fn_(&img_, &dst_img_, filter_type, phase_scaler));
  }

  void RunTest() {
    static const int kNumSizesToTest = 4;
    static const int kNumScaleFactorsToTest = 2;
    static const int kWidthsToTest[] = { 16, 32, 48, 64 };
    static const int kHeightsToTest[] = { 16, 20, 24, 28 };
    static const int kScaleFactor[] = { 1, 2 };
    for (INTERP_FILTER filter_type = 0; filter_type < 4; ++filter_type) {
      for (int phase_scaler = 0; phase_scaler < 16; ++phase_scaler) {
        for (int h = 0; h < kNumSizesToTest; ++h) {
          const int src_height = kHeightsToTest[h];
          for (int w = 0; w < kNumSizesToTest; ++w) {
            const int src_width = kWidthsToTest[w];
            for (int sf_up_idx = 0; sf_up_idx < kNumScaleFactorsToTest;
                 ++sf_up_idx) {
              const int sf_up = kScaleFactor[sf_up_idx];
              for (int sf_down_idx = 0; sf_down_idx < kNumScaleFactorsToTest;
                   ++sf_down_idx) {
                const int sf_down = kScaleFactor[sf_down_idx];
                const int dst_width = src_width * sf_up / sf_down;
                const int dst_height = src_height * sf_up / sf_down;
                if (sf_up == sf_down && sf_up != 1) {
                  continue;
                }
                ASSERT_NO_FATAL_FAILURE(ResetScaleImages(
                    src_width, src_height, dst_width, dst_height));
                ReferenceScaleFrame(filter_type, phase_scaler);
                ScaleFrame(filter_type, phase_scaler);
                PrintDiff();
                CompareImages(dst_img_);
                DeallocScaleImages();
              }
            }
          }
        }
      }
    }
  }

  void PrintDiff() const {
    if (memcmp(dst_img_.buffer_alloc, ref_img_.buffer_alloc,
               ref_img_.frame_size)) {
      for (int y = 0; y < ref_img_.y_stride; y++) {
        for (int x = 0; x < ref_img_.y_stride; x++) {
          if (ref_img_.buffer_alloc[y * ref_img_.y_stride + x] !=
              dst_img_.buffer_alloc[y * ref_img_.y_stride + x]) {
            printf("dst_img_[%d][%d] diff:%6d (ref),%6d (opt)\n", y, x,
                   ref_img_.buffer_alloc[y * ref_img_.y_stride + x],
                   dst_img_.buffer_alloc[y * ref_img_.y_stride + x]);
            break;
          }
        }
      }
    }
  }

  ScaleFrameFunc scale_fn_;
};

TEST_P(ScaleTest, ScaleFrame) { ASSERT_NO_FATAL_FAILURE(RunTest()); }

TEST_P(ScaleTest, DISABLED_Speed) {
  static const int kCountSpeedTestBlock = 100;
  static const int kNumScaleFactorsToTest = 2;
  static const int kScaleFactor[] = { 1, 2 };
  const int src_height = 1280;
  const int src_width = 720;
  for (INTERP_FILTER filter_type = 2; filter_type < 4; ++filter_type) {
    for (int phase_scaler = 0; phase_scaler < 2; ++phase_scaler) {
      for (int sf_up_idx = 0; sf_up_idx < kNumScaleFactorsToTest; ++sf_up_idx) {
        const int sf_up = kScaleFactor[sf_up_idx];
        for (int sf_down_idx = 0; sf_down_idx < kNumScaleFactorsToTest;
             ++sf_down_idx) {
          const int sf_down = kScaleFactor[sf_down_idx];
          const int dst_width = src_width * sf_up / sf_down;
          const int dst_height = src_height * sf_up / sf_down;
          if (sf_up == sf_down && sf_up != 1) {
            continue;
          }
          ASSERT_NO_FATAL_FAILURE(
              ResetScaleImages(src_width, src_height, dst_width, dst_height));
          ASM_REGISTER_STATE_CHECK(
              ReferenceScaleFrame(filter_type, phase_scaler));

          vpx_usec_timer timer;
          vpx_usec_timer_start(&timer);
          for (int i = 0; i < kCountSpeedTestBlock; ++i) {
            ScaleFrame(filter_type, phase_scaler);
          }
          libvpx_test::ClearSystemState();
          vpx_usec_timer_mark(&timer);
          const int elapsed_time =
              static_cast<int>(vpx_usec_timer_elapsed(&timer) / 1000);
          CompareImages(dst_img_);
          DeallocScaleImages();

          printf(
              "filter_type = %d, phase_scaler = %d, src_width = %4d, "
              "src_height = %4d, dst_width = %4d, dst_height = %4d, "
              "scale factor = %d:%d, scale time: %5d ms\n",
              filter_type, phase_scaler, src_width, src_height, dst_width,
              dst_height, sf_down, sf_up, elapsed_time);
        }
      }
    }
  }
}

#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, ScaleTest,
                        ::testing::Values(vp9_scale_and_extend_frame_ssse3));
#endif  // HAVE_SSSE3

}  // namespace libvpx_test
