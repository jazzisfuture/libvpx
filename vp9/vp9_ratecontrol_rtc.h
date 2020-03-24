/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_RATE_CONTROL_RTC_H_
#define VPX_VP9_RATE_CONTROL_RTC_H_

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include "vp9/common/vp9_entropymode.h"
#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/vp9_iface_common.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_firstpass.h"
#include "vp9/vp9_cx_iface.h"

namespace vp9 {

struct RateControlRTCConfig {
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
  int scaling_factor_num[VPX_MAX_LAYERS];
  int scaling_factor_den[VPX_MAX_LAYERS];
  int layer_target_bitrate[VPX_MAX_LAYERS];
};

struct FrameParamsQPRTC {
  FRAME_TYPE frame_type;
  int spatial_layer_id;
  int temporal_layer_id;
};

class VP9RateControlRTC {
 public:
  VP9RateControlRTC();
  ~VP9RateControlRTC();

  void InitRateControl(const RateControlRTCConfig &cfg);
  void UpdateRateControl(const RateControlRTCConfig &rc_cfg);
  int GetQP();
  int GetLoopfilterLevel();
  void ComputeQP(const FrameParamsQPRTC &frame_params);
  void PostEncode(const FrameParamsQPRTC &frame_params);

 protected:
  VP9_COMP *cpi_;
};

}  // namespace vp9

#endif  // VPX_VP9_RATE_CONTROL_RTC_H_
