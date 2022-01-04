/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_RATECTRL_RTC_H_
#define VPX_VP9_RATECTRL_RTC_H_

#include <cstdint>
#include <memory>

#include "vp9/common/vp9_common.h"
#include "vpx/internal/vpx_ratectrl_rtc.h"

struct VP9_COMP;

namespace libvpx {

struct VP9RateControlRtcConfig : public VpxRateControlRtcConfig {
 public:
  VP9RateControlRtcConfig();

  // Number of spatial layers
  int ss_number_layers;
  // Number of temporal layers
  int ts_number_layers;
  int max_quantizers[VPX_MAX_LAYERS];
  int min_quantizers[VPX_MAX_LAYERS];
  int scaling_factor_num[VPX_SS_MAX_LAYERS];
  int scaling_factor_den[VPX_SS_MAX_LAYERS];
};

struct VP9FrameParamsQpRTC {
  FRAME_TYPE frame_type;
  int spatial_layer_id;
  int temporal_layer_id;
};

// This interface allows using VP9 real-time rate control without initializing
// the encoder. To use this interface, you need to link with libvpxrc.a.
//
// #include "vp9/ratectrl_rtc.h"
// VP9RateControlRTC rc_api;
// VP9RateControlRtcConfig cfg;
// VP9FrameParamsQpRTC frame_params;
//
// YourFunctionToInitializeConfig(cfg);
// rc_api.InitRateControl(cfg);
// // start encoding
// while (frame_to_encode) {
//   if (config_changed)
//     rc_api.UpdateRateControl(cfg);
//   YourFunctionToFillFrameParams(frame_params);
//   rc_api.ComputeQP(frame_params);
//   YourFunctionToUseQP(rc_api.GetQP());
//   YourFunctionToUseLoopfilter(rc_api.GetLoopfilterLevel());
//   // After encoding
//   rc_api.PostEncode(encoded_frame_size);
// }
class VP9RateControlRTC {
 public:
  static std::unique_ptr<VP9RateControlRTC> Create(
      const VP9RateControlRtcConfig &cfg);
  ~VP9RateControlRTC();

  void UpdateRateControl(const VP9RateControlRtcConfig &rc_cfg);
  // GetQP() needs to be called after ComputeQP() to get the latest QP
  int GetQP() const;
  int GetLoopfilterLevel() const;
  signed char *GetCyclicRefreshMap() const;
  int *GetDeltaQ() const;
  void ComputeQP(const VP9FrameParamsQpRTC &frame_params);
  // Feedback to rate control with the size of current encoded frame
  void PostEncodeUpdate(uint64_t encoded_frame_size);

 private:
  VP9RateControlRTC() {}
  void InitRateControl(const VP9RateControlRtcConfig &cfg);
  VP9_COMP *cpi_;
};

}  // namespace libvpx

#endif  // VPX_VP9_RATECTRL_RTC_H_
