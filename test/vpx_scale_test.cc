/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vp9_rtcd.h"
#include "./vpx_config.h"
#include "./vpx_scale_rtcd.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/vpx_timer.h"
#include "vpx_scale/yv12config.h"

namespace {

typedef void (*ExtendFrameBorderFunc)(YV12_BUFFER_CONFIG *ybf);
typedef void (*CopyFrameFunc)(const YV12_BUFFER_CONFIG *src_ybf,
                              YV12_BUFFER_CONFIG *dst_ybf);
typedef void (*ScaleFunc)(const YV12_BUFFER_CONFIG *src,
                          YV12_BUFFER_CONFIG *dst, INTERP_FILTER filter_type,
                          int phase_scaler);

class VpxScaleBase {
 public:
  virtual ~VpxScaleBase() { libvpx_test::ClearSystemState(); }

  void ResetImage(YV12_BUFFER_CONFIG *const img, const int width,
                  const int height) {
    memset(img, 0, sizeof(YV12_BUFFER_CONFIG));
    ASSERT_EQ(
        0, vp8_yv12_alloc_frame_buffer(img, width, height, VP8BORDERINPIXELS));
    memset(img->buffer_alloc, kBufFiller, img->frame_size);
  }

  void ResetImages(const int width, const int height) {
    ResetImage(&img_, width, height);
    ResetImage(&ref_img_, width, height);
    ResetImage(&dst_img_, width, height);

    FillPlane(img_.y_buffer, img_.y_crop_width, img_.y_crop_height,
              img_.y_stride);
    FillPlane(img_.u_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);
    FillPlane(img_.v_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);
  }

  void ResetScaleImage(YV12_BUFFER_CONFIG *const img, const int width,
                       const int height) {
    memset(img, 0, sizeof(YV12_BUFFER_CONFIG));
    ASSERT_EQ(0, vpx_alloc_frame_buffer(img, width, height, 1, 1,
#if CONFIG_VP9_HIGHBITDEPTH
                                        0,
#endif
                                        VP9_ENC_BORDER_IN_PIXELS, 0));
    memset(img->buffer_alloc, kBufFiller, img->frame_size);
  }

  void ResetScaleImages(const int src_width, const int src_height,
                        const int dst_width, const int dst_height) {
    ResetScaleImage(&img_, src_width, src_height);
    ResetScaleImage(&ref_img_, dst_width, dst_height);
    ResetScaleImage(&dst_img_, dst_width, dst_height);
    FillPlane(img_.y_buffer, img_.y_crop_width, img_.y_crop_height,
              img_.y_stride);
    FillPlane(img_.u_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);
    FillPlane(img_.v_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);
  }

  void DeallocImages() {
    vp8_yv12_de_alloc_frame_buffer(&img_);
    vp8_yv12_de_alloc_frame_buffer(&ref_img_);
    vp8_yv12_de_alloc_frame_buffer(&dst_img_);
  }

  void DeallocScaleImages() {
    vpx_free_frame_buffer(&img_);
    vpx_free_frame_buffer(&ref_img_);
    vpx_free_frame_buffer(&dst_img_);
  }

 protected:
  static const int kBufFiller = 123;
  static const int kBufMax = kBufFiller - 1;

  static void FillPlane(uint8_t *buf, int width, int height, int stride) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        buf[x + (y * stride)] = (x + (width * y)) % kBufMax;
      }
    }
  }

  static void ExtendPlane(uint8_t *buf, int crop_width, int crop_height,
                          int width, int height, int stride, int padding) {
    // Copy the outermost visible pixel to a distance of at least 'padding.'
    // The buffers are allocated such that there may be excess space outside the
    // padding. As long as the minimum amount of padding is achieved it is not
    // necessary to fill this space as well.
    uint8_t *left = buf - padding;
    uint8_t *right = buf + crop_width;
    const int right_extend = padding + (width - crop_width);
    const int bottom_extend = padding + (height - crop_height);

    // Fill the border pixels from the nearest image pixel.
    for (int y = 0; y < crop_height; ++y) {
      memset(left, left[padding], padding);
      memset(right, right[-1], right_extend);
      left += stride;
      right += stride;
    }

    left = buf - padding;
    uint8_t *top = left - (stride * padding);
    // The buffer does not always extend as far as the stride.
    // Equivalent to padding + width + padding.
    const int extend_width = padding + crop_width + right_extend;

    // The first row was already extended to the left and right. Copy it up.
    for (int y = 0; y < padding; ++y) {
      memcpy(top, left, extend_width);
      top += stride;
    }

    uint8_t *bottom = left + (crop_height * stride);
    for (int y = 0; y < bottom_extend; ++y) {
      memcpy(bottom, left + (crop_height - 1) * stride, extend_width);
      bottom += stride;
    }
  }

  void ReferenceExtendBorder() {
    ExtendPlane(ref_img_.y_buffer, ref_img_.y_crop_width,
                ref_img_.y_crop_height, ref_img_.y_width, ref_img_.y_height,
                ref_img_.y_stride, ref_img_.border);
    ExtendPlane(ref_img_.u_buffer, ref_img_.uv_crop_width,
                ref_img_.uv_crop_height, ref_img_.uv_width, ref_img_.uv_height,
                ref_img_.uv_stride, ref_img_.border / 2);
    ExtendPlane(ref_img_.v_buffer, ref_img_.uv_crop_width,
                ref_img_.uv_crop_height, ref_img_.uv_width, ref_img_.uv_height,
                ref_img_.uv_stride, ref_img_.border / 2);
  }

  void ReferenceCopyFrame() {
    // Copy img_ to ref_img_ and extend frame borders. This will be used for
    // verifying extend_fn_ as well as copy_frame_fn_.
    EXPECT_EQ(ref_img_.frame_size, img_.frame_size);
    for (int y = 0; y < img_.y_crop_height; ++y) {
      for (int x = 0; x < img_.y_crop_width; ++x) {
        ref_img_.y_buffer[x + y * ref_img_.y_stride] =
            img_.y_buffer[x + y * img_.y_stride];
      }
    }

    for (int y = 0; y < img_.uv_crop_height; ++y) {
      for (int x = 0; x < img_.uv_crop_width; ++x) {
        ref_img_.u_buffer[x + y * ref_img_.uv_stride] =
            img_.u_buffer[x + y * img_.uv_stride];
        ref_img_.v_buffer[x + y * ref_img_.uv_stride] =
            img_.v_buffer[x + y * img_.uv_stride];
      }
    }

    ReferenceExtendBorder();
  }

  void ReferenceScale(INTERP_FILTER filter_type, int phase_scaler) {
    vp9_scale_and_extend_frame_c(&img_, &ref_img_, filter_type, phase_scaler);
  }

  void CompareImages(const YV12_BUFFER_CONFIG actual) {
    EXPECT_EQ(ref_img_.frame_size, actual.frame_size);
    EXPECT_EQ(0, memcmp(ref_img_.buffer_alloc, actual.buffer_alloc,
                        ref_img_.frame_size));
  }

  YV12_BUFFER_CONFIG img_;
  YV12_BUFFER_CONFIG ref_img_;
  YV12_BUFFER_CONFIG dst_img_;
};

class ExtendBorderTest
    : public VpxScaleBase,
      public ::testing::TestWithParam<ExtendFrameBorderFunc> {
 public:
  virtual ~ExtendBorderTest() {}

 protected:
  virtual void SetUp() { extend_fn_ = GetParam(); }

  void ExtendBorder() { ASM_REGISTER_STATE_CHECK(extend_fn_(&img_)); }

  void RunTest() {
#if ARCH_ARM
    // Some arm devices OOM when trying to allocate the largest buffers.
    static const int kNumSizesToTest = 6;
#else
    static const int kNumSizesToTest = 7;
#endif
    static const int kSizesToTest[] = { 1, 15, 33, 145, 512, 1025, 16383 };
    for (int h = 0; h < kNumSizesToTest; ++h) {
      for (int w = 0; w < kNumSizesToTest; ++w) {
        ASSERT_NO_FATAL_FAILURE(ResetImages(kSizesToTest[w], kSizesToTest[h]));
        ReferenceCopyFrame();
        ExtendBorder();
        CompareImages(img_);
        DeallocImages();
      }
    }
  }

  ExtendFrameBorderFunc extend_fn_;
};

TEST_P(ExtendBorderTest, ExtendBorder) { ASSERT_NO_FATAL_FAILURE(RunTest()); }

INSTANTIATE_TEST_CASE_P(C, ExtendBorderTest,
                        ::testing::Values(vp8_yv12_extend_frame_borders_c));

class CopyFrameTest : public VpxScaleBase,
                      public ::testing::TestWithParam<CopyFrameFunc> {
 public:
  virtual ~CopyFrameTest() {}

 protected:
  virtual void SetUp() { copy_frame_fn_ = GetParam(); }

  void CopyFrame() {
    ASM_REGISTER_STATE_CHECK(copy_frame_fn_(&img_, &dst_img_));
  }

  void RunTest() {
#if ARCH_ARM
    // Some arm devices OOM when trying to allocate the largest buffers.
    static const int kNumSizesToTest = 6;
#else
    static const int kNumSizesToTest = 7;
#endif
    static const int kSizesToTest[] = { 1, 15, 33, 145, 512, 1025, 16383 };
    for (int h = 0; h < kNumSizesToTest; ++h) {
      for (int w = 0; w < kNumSizesToTest; ++w) {
        ASSERT_NO_FATAL_FAILURE(ResetImages(kSizesToTest[w], kSizesToTest[h]));
        ReferenceCopyFrame();
        CopyFrame();
        CompareImages(dst_img_);
        DeallocImages();
      }
    }
  }

  CopyFrameFunc copy_frame_fn_;
};

TEST_P(CopyFrameTest, CopyFrame) { ASSERT_NO_FATAL_FAILURE(RunTest()); }

INSTANTIATE_TEST_CASE_P(C, CopyFrameTest,
                        ::testing::Values(vp8_yv12_copy_frame_c));

class ScaleTest : public VpxScaleBase,
                  public ::testing::TestWithParam<ScaleFunc> {
 public:
  virtual ~ScaleTest() {}

 protected:
  virtual void SetUp() { scale_fn_ = GetParam(); }

  void Scale(INTERP_FILTER filter_type, int phase_scaler) {
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
                ReferenceScale(filter_type, phase_scaler);
                Scale(filter_type, phase_scaler);
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

  void PrintDiff() {
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

  ScaleFunc scale_fn_;
};

TEST_P(ScaleTest, Scale) { ASSERT_NO_FATAL_FAILURE(RunTest()); }

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
          ASM_REGISTER_STATE_CHECK(ReferenceScale(filter_type, phase_scaler));

          vpx_usec_timer timer;
          vpx_usec_timer_start(&timer);
          for (int i = 0; i < kCountSpeedTestBlock; ++i) {
            Scale(filter_type, phase_scaler);
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
}  // namespace
