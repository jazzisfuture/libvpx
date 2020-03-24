/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/vp9_ratecontrol_rtc.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vpx/vp8cx.h"
#include "vpx/vpx_codec.h"

namespace vp9 {

VP9RateControlRTC::VP9RateControlRTC() {
  cpi_ = (VP9_COMP *)vpx_memalign(32, sizeof(VP9_COMP));
}

void VP9RateControlRTC::InitRateControl(const RateControlRTCConfig &rc_cfg) {
  VP9_COMMON *cm = &cpi_->common;
  VP9EncoderConfig *oxcf = &cpi_->oxcf;
  RATE_CONTROL *const rc = &cpi_->rc;
  cm->profile = PROFILE_0;
  cm->bit_depth = 8;
  cm->show_frame = 1;
  oxcf->rc_mode = VPX_CBR;
  oxcf->pass = 0;
  oxcf->aq_mode = NO_AQ;
  oxcf->content = VP9E_CONTENT_DEFAULT;
  oxcf->drop_frames_water_mark = 0;

  cm->width = rc_cfg.width;
  cm->height = rc_cfg.height;
  oxcf->width = rc_cfg.width;
  oxcf->width = rc_cfg.height;
  oxcf->worst_allowed_q = vp9_quantizer_to_qindex(rc_cfg.max_quantizer);
  oxcf->best_allowed_q = vp9_quantizer_to_qindex(rc_cfg.min_quantizer);
  oxcf->target_bandwidth = 1000 * rc_cfg.target_bandwidth;
  oxcf->starting_buffer_level_ms = rc_cfg.buf_initial_sz;
  oxcf->optimal_buffer_level_ms = rc_cfg.buf_optimal_sz;
  oxcf->maximum_buffer_size_ms = rc_cfg.buf_sz;
  oxcf->under_shoot_pct = rc_cfg.undershoot_pct;
  oxcf->over_shoot_pct = rc_cfg.overshoot_pct;
  cpi_->oxcf.rc_max_intra_bitrate_pct = rc_cfg.max_intra_bitrate_pct;
  cpi_->framerate = rc_cfg.framerate;
  cpi_->svc.number_spatial_layers = rc_cfg.ss_number_layers;
  cpi_->svc.number_temporal_layers = rc_cfg.ts_number_layers;
  cpi_->svc.temporal_layering_mode =
      (rc_cfg.ts_number_layers > 1)
          ? (VP9E_TEMPORAL_LAYERING_MODE)rc_cfg.ts_number_layers
          : VP9E_TEMPORAL_LAYERING_MODE_NOLAYERING;
  rc->worst_quality = oxcf->worst_allowed_q;
  rc->best_quality = oxcf->best_allowed_q;
  vp9_new_framerate(cpi_, cpi_->framerate);
  vp9_set_rc_buffer_sizes(rc, oxcf);
  if (cpi_->svc.number_temporal_layers > 1) {
    if (cm->current_video_frame == 0) vp9_init_layer_context(cpi_);
    vp9_update_layer_context_change_config(cpi_,
                                           (int)cpi_->oxcf.target_bandwidth);
  }

  cpi_->use_svc = (cpi_->svc.number_spatial_layers > 1 ||
                   cpi_->svc.number_temporal_layers > 1)
                      ? 1
                      : 0;
  rc->rc_1_frame = 0;
  rc->rc_2_frame = 0;
  vp9_rc_init(oxcf, 0, rc);
  cpi_->sf.use_nonrd_pick_mode = 1;
  rc->compute_frame_motion_pass0 = 0;
  cm->current_video_frame = 0;
}

}  // namespace vp9
