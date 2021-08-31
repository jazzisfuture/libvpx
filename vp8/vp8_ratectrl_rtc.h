/*
 *  Copyright (c) 2021 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP8_RATECTRL_RTC_H_
#define VPX_VP8_RATECTRL_RTC_H_

#include <cstdint>
#include <memory>

#include "vp8/encoder/onyx_int.h"
#include "vp8/common/common.h"
#include "vpx/internal/vpx_ratectrl_rtc.h"

namespace libvpx {
/* Quant MOD */
static const int kQTrans[] = {
  0,  1,  2,  3,  4,  5,  7,   8,   9,   10,  12,  13,  15,  17,  18,  19,
  20, 21, 23, 24, 25, 26, 27,  28,  29,  30,  31,  33,  35,  37,  39,  41,
  43, 45, 47, 49, 51, 53, 55,  57,  59,  61,  64,  67,  70,  73,  76,  79,
  82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127,
};

struct VP8RateControlRtcConfig : public VpxRateControlRtcConfig {
 public:
  VP8RateControlRtcConfig() {
    vp8_zero(layer_target_bitrate);
    vp8_zero(ts_rate_decimator);
  }
};

struct VP8FrameParamsQpRTC {
  FRAME_TYPE frame_type;
};

class VP8RateControlRTC {
 public:
  static std::unique_ptr<VP8RateControlRTC> Create(
      const VP8RateControlRtcConfig &cfg);
  ~VP8RateControlRTC() {
    if (cpi_) {
      vpx_free(cpi_->gf_active_flags);
      vpx_free(cpi_);
    }
  }

  void UpdateRateControl(const VP8RateControlRtcConfig &rc_cfg);
  // GetQP() needs to be called after ComputeQP() to get the latest QP
  int GetQP() const;
  // int GetLoopfilterLevel() const;
  void ComputeQP(const VP8FrameParamsQpRTC &frame_params);
  // Feedback to rate control with the size of current encoded frame
  void PostEncodeUpdate(uint64_t encoded_frame_size);

 private:
  VP8RateControlRTC() {}
  void InitRateControl(const VP8RateControlRtcConfig &cfg);
  VP8_COMP *cpi_;
  int q_;
  int frame_over_shoot_limit_;
  int frame_under_shoot_limit_;
};

}  // namespace libvpx

#endif  // VPX_VP8_RATECTRL_RTC_H_
