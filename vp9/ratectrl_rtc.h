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

#include "vp9/common/vp9_entropymode.h"
#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/vp9_iface_common.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_firstpass.h"
#include "vp9/vp9_cx_iface.h"

namespace vp9 {

struct RateControlRtcConfig {
  int width;
  int height;
  // 0-63
  int max_quantizer;
  int min_quantizer;
  int64_t target_bandwidth;
  int64_t buf_initial_sz;
  int64_t buf_optimal_sz;
  int64_t buf_sz;
  int undershoot_pct;
  int overshoot_pct;
  int max_intra_bitrate_pct;
  double framerate;
  // Number of spatial layers
  int ss_number_layers;
  // Number of temporal layers
  int ts_number_layers;
  int max_quantizers[VPX_MAX_LAYERS];
  int min_quantizers[VPX_MAX_LAYERS];
  int scaling_factor_num[VPX_SS_MAX_LAYERS];
  int scaling_factor_den[VPX_SS_MAX_LAYERS];
  int layer_target_bitrate[VPX_MAX_LAYERS];
  int ts_rate_decimator[VPX_TS_MAX_LAYERS];
};

struct FrameParamsQpRTC {
  FRAME_TYPE frame_type;
  int spatial_layer_id;
  int temporal_layer_id;
};

// This interface allows using VP9 real-time rate control without initializing
// the encoder. To use this interface, you need to link with libvp9rc.a.
//
// #include "vp9/ratectrl_rtc.h"
// VP9RateControlRTC rc_api;
// RateControlRtcConfig cfg;
// FrameParamsQpRTC frame_params;
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
  static VP9RateControlRTC *Init();
  ~VP9RateControlRTC() { vpx_free(cpi_); }

  void InitRateControl(const RateControlRtcConfig &cfg);
  void UpdateRateControl(const RateControlRtcConfig &rc_cfg);
  // GetQP() needs to be called after ComputeQP() to get the latest QP
  int GetQP() const;
  int GetLoopfilterLevel() const;
  void ComputeQP(const FrameParamsQpRTC &frame_params);
  // Feedback to rate control with the size of current encoded frame
  void PostEncodeUpdate(uint64_t encoded_frame_size);

 private:
  VP9RateControlRTC(){};

 private:
  VP9_COMP *cpi_;
};

}  // namespace vp9

#endif  // VPX_VP9_RATECTRL_RTC_H_
