/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <string.h>
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./vpx_config.h"
#include "./vpx_scale_rtcd.h"
#include "vpx_scale/yv12config.h"
#include "vpx_mem/vpx_mem.h"

namespace {

typedef void (*extend_frame_border_fn_t)(YV12_BUFFER_CONFIG *ybf);
typedef void (*copy_frame_fn_t)(const YV12_BUFFER_CONFIG *src_ybf,
                                 YV12_BUFFER_CONFIG *dst_ybf);

class VpxScaleBase {
 public:
  virtual ~VpxScaleBase() { libvpx_test::ClearSystemState(); }

  void SetupImage(int width,
                  int height) {
    width_ = width;
    height_ = height;
    vpx_memset(&img_, 0, sizeof(YV12_BUFFER_CONFIG));
    ASSERT_EQ(0,
              vp8_yv12_alloc_frame_buffer(&img_, width_, height_,
                                          VP8BORDERINPIXELS));

    vpx_memset(img_.buffer_alloc, 123, img_.frame_size);
    FillPlane(img_.y_buffer, img_.y_width, img_.y_height,
              img_.y_stride);
    FillPlane(img_.u_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);
    FillPlane(img_.v_buffer, img_.uv_crop_width, img_.uv_crop_height,
              img_.uv_stride);

    vpx_memset(&ref_img_, 0, sizeof(YV12_BUFFER_CONFIG));
    ASSERT_EQ(0,
              vp8_yv12_alloc_frame_buffer(&ref_img_, width_, height_,
                                          VP8BORDERINPIXELS));

    vpx_memset(ref_img_.buffer_alloc, 123, ref_img_.frame_size);

    vpx_memset(&cpy_img_, 0, sizeof(YV12_BUFFER_CONFIG));
    ASSERT_EQ(0,
              vp8_yv12_alloc_frame_buffer(&cpy_img_, width_, height_,
                                          VP8BORDERINPIXELS));

    vpx_memset(cpy_img_.buffer_alloc, 123, cpy_img_.frame_size);
    ReferenceCopyFrame();
  }

  void DeallocImage() {
    vp8_yv12_de_alloc_frame_buffer(&img_);
    vp8_yv12_de_alloc_frame_buffer(&ref_img_);
  }

 protected:
#if ARCH_ARM
  // Some arm devices OOM when trying to allocate the largest bufers
  static const int kNumSizesToTest = 6;
#else
  static const int kNumSizesToTest = 7;
#endif

  void FillPlane(uint8_t *buf, int w, int h, int s) {
    for (int height = 0; height < h; ++height) {
      for (int width = 0; width < w; ++width) {
        buf[width + (height * s)] =
          (width + (w * height)) % 122;
      }
    }
  }

  void ReferenceCopyFrame() {
    // Copy img_ to ref_img_ and extend frame borders. This will be used for
    // verifying extend_fn_ as well as copy_frame_fn_.
    EXPECT_EQ(ref_img_.frame_size, img_.frame_size);
    for (int h = 0; h < img_.y_crop_height; ++h) {
      for (int w = 0; w < img_.y_crop_width; ++w) {
        ref_img_.y_buffer[w + h * ref_img_.y_stride] =
          img_.y_buffer[w + h * img_.y_stride];
      }
    }

    for (int h = 0; h < img_.uv_crop_height; ++h) {
      for (int w = 0; w < img_.uv_crop_width; ++w) {
        ref_img_.u_buffer[w + h * ref_img_.uv_stride] =
          img_.u_buffer[w + h * img_.uv_stride];
      }
    }

    for (int h = 0; h < img_.uv_crop_height; ++h) {
      for (int w = 0; w < img_.uv_crop_width; ++w) {
        ref_img_.v_buffer[w + h * ref_img_.uv_stride] =
          img_.v_buffer[w + h * img_.uv_stride];
      }
    }

    ReferenceExtendBorder();
  }

  void ReferenceExtendBorder() {
    // Ensure consistent border extension by rounding uv_crop_* at image
    // creation time. When it is rounded here, problems arise with the right
    // and bottom extensions.
    // When c_w = 15, w = 16, b = 32:
    //  (b + w - c_w + 1) / 2
    //  32 + 16 - 15 + 1 should equal 32 *but*
    //  32 + 1 + 1 equals 34 giving a right buffer of 17 instead of 16
    ExtendPlane(ref_img_.y_buffer,
                ref_img_.y_crop_width, ref_img_.y_crop_height,
                ref_img_.y_width, ref_img_.y_height,
                ref_img_.y_stride,
                ref_img_.border);
    ExtendPlane(ref_img_.u_buffer,
                ref_img_.uv_crop_width, ref_img_.uv_crop_height,
                ref_img_.uv_width, ref_img_.uv_height,
                ref_img_.uv_stride,
                ref_img_.border / 2);
    ExtendPlane(ref_img_.v_buffer,
                ref_img_.uv_crop_width, ref_img_.uv_crop_height,
                ref_img_.uv_width, ref_img_.uv_height,
                ref_img_.uv_stride,
                ref_img_.border / 2);
  }

  void ExtendPlane(uint8_t *buf, int c_w, int c_h, int w, int h, int s, int b) {
    uint8_t *l = buf - b;  // Leftmost border pixel in the image row.
    uint8_t *r = buf + c_w;  // Border pixel just right of the image row.
    int right_extend = b + (w - c_w);  // Fill the extra alignment pixels.

    // Fill the border pixels from the nearest image pixel.
    for (int height = 0; height < c_h; ++height) {
      vpx_memset(l, *(l + b), b);
      vpx_memset(r, *(r - 1), right_extend);
      l += s;
      r += s;
    }

    l = buf - b;  // Reset to the leftmost border pixel in the home row.
    uint8_t *t = l - (s * b);  // Rewind to the first pixel in the buffer.
    int extend_width = b + c_w + right_extend;  // The buffer does not always
                                                // extend as far as the
                                                // stride.

    // The first row was already extended to the left and right. Copy it up.
    for (int border = 0; border < b; ++border) {
      vpx_memcpy(t, l, extend_width);
      t += s;
    }

    uint8_t *e = l + (c_h * s);  // Move down to the last image row.
    // Only extend pixels in the visible image.
    for (int bot_extend = 0; bot_extend < b + (h - c_h); ++bot_extend) {
      vpx_memcpy(e, l + (c_h - 1) * s, extend_width);
      e += s;
    }
  }

  void CompareImages(YV12_BUFFER_CONFIG exp, YV12_BUFFER_CONFIG ref) {
    EXPECT_EQ(ref.frame_size, exp.frame_size);
    EXPECT_EQ(0, memcmp(ref.buffer_alloc, exp.buffer_alloc,
        ref.frame_size));
  }

  YV12_BUFFER_CONFIG img_;
  YV12_BUFFER_CONFIG ref_img_;
  YV12_BUFFER_CONFIG cpy_img_;
  int width_;
  int height_;
};

class ExtendBorderTest
    : public VpxScaleBase,
      public ::testing::TestWithParam<extend_frame_border_fn_t> {
 public:
 protected:
    virtual void SetUp() {
      extend_fn_ = GetParam();
    }

    void ExtendBorder() {
      ASM_REGISTER_STATE_CHECK(extend_fn_(&img_));
    }

    void RunTest() {
      int sides[] = {1, 15, 33, 145, 512, 1025, 16383};
      for (int h = 0; h < kNumSizesToTest; ++h) {
        for (int w = 0; w < kNumSizesToTest; ++w) {
          SetupImage(sides[w], sides[h]);
          ExtendBorder();
          ReferenceExtendBorder();
          CompareImages(img_, ref_img_);
          DeallocImage();
        }
      }
    }

    extend_frame_border_fn_t extend_fn_;
};

TEST_P(ExtendBorderTest, ExtendBorder) {
  RunTest();
}

INSTANTIATE_TEST_CASE_P(C, ExtendBorderTest,
                        ::testing::Values(
                            vp8_yv12_extend_frame_borders_c));
class CopyFrameTest
    : public VpxScaleBase,
      public ::testing::TestWithParam<copy_frame_fn_t> {
 public:
 protected:
    virtual void SetUp() {
      copy_frame_fn_ = GetParam();
    }

    void CopyFrame() {
      ASM_REGISTER_STATE_CHECK(copy_frame_fn_(&img_, &cpy_img_));
    }

    void RunTest() {
      int sides[] = {1, 15, 33, 145, 512, 1025, 16383};
      for (int h = 0; h < kNumSizesToTest; ++h) {
        for (int w = 0; w < kNumSizesToTest; ++w) {
          SetupImage(sides[w], sides[h]);
          ReferenceCopyFrame();
          CopyFrame();
          CompareImages(cpy_img_, ref_img_);
          DeallocImage();
        }
      }
    }

    copy_frame_fn_t copy_frame_fn_;
};

TEST_P(CopyFrameTest, CopyFrame) {
  RunTest();
}

INSTANTIATE_TEST_CASE_P(C, CopyFrameTest,
                        ::testing::Values(
                            vp8_yv12_copy_frame_c));

}  // namespace
