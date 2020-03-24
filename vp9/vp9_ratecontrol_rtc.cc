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
#include "vp9/encoder/vp9_picklpf.h"
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
  cm->bit_depth = VPX_BITS_8;
  cm->show_frame = 1;
  oxcf->rc_mode = VPX_CBR;
  oxcf->pass = 0;
  oxcf->aq_mode = NO_AQ;
  oxcf->content = VP9E_CONTENT_DEFAULT;
  oxcf->drop_frames_water_mark = 0;

  cpi_->use_svc = (cpi_->svc.number_spatial_layers > 1 ||
                   cpi_->svc.number_temporal_layers > 1)
                      ? 1
                      : 0;

  UpdateRateControl(rc_cfg);

  rc->rc_1_frame = 0;
  rc->rc_2_frame = 0;
  vp9_rc_init(oxcf, 0, rc);
  cpi_->sf.use_nonrd_pick_mode = 1;
  rc->compute_frame_motion_pass0 = 0;
  cm->current_video_frame = 0;
}

void VP9RateControlRTC::UpdateRateControl(const RateControlRTCConfig &rc_cfg) {
  VP9_COMMON *cm = &cpi_->common;
  VP9EncoderConfig *oxcf = &cpi_->oxcf;
  RATE_CONTROL *const rc = &cpi_->rc;

  cm->width = rc_cfg.width;
  cm->height = rc_cfg.height;
  oxcf->width = rc_cfg.width;
  oxcf->width = rc_cfg.height;
  oxcf->worst_allowed_q = vp9_quantizer_to_qindex(rc_cfg.max_quantizer);
  oxcf->best_allowed_q = vp9_quantizer_to_qindex(rc_cfg.min_quantizer);
  rc->worst_quality = oxcf->worst_allowed_q;
  rc->best_quality = oxcf->best_allowed_q;
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

  for (int sl = 0; sl < cpi_->svc.number_spatial_layers; ++sl) {
    for (int tl = 0; tl < cpi_->svc.number_temporal_layers; ++tl) {
      const int layer =
          LAYER_IDS_TO_IDX(sl, tl, cpi_->svc.number_temporal_layers);
      LAYER_CONTEXT *lc = &cpi_->svc.layer_context[layer];
      RATE_CONTROL *const lrc = &lc->rc;
      oxcf->layer_target_bitrate[layer] =
          1000 * rc_cfg.layer_target_bitrate[layer];
      lrc->worst_quality =
          vp9_quantizer_to_qindex(rc_cfg.max_quantizers[layer]);
      lrc->best_quality = vp9_quantizer_to_qindex(rc_cfg.min_quantizers[layer]);
      lc->scaling_factor_num = rc_cfg.scaling_factor_num[sl];
      lc->scaling_factor_den = rc_cfg.scaling_factor_den[sl];
    }
  }
  vp9_set_rc_buffer_sizes(cpi_);
  vp9_new_framerate(cpi_, cpi_->framerate);
  if (cpi_->svc.number_temporal_layers > 1) {
    if (cm->current_video_frame == 0) vp9_init_layer_context(cpi_);
    vp9_update_layer_context_change_config(cpi_,
                                           (int)cpi_->oxcf.target_bandwidth);
  }
  vp9_check_reset_rc_flag(cpi_);
}

void VP9RateControlRTC::ComputeQP(const FrameParamsQPRTC &frame_params) {
  VP9_COMMON *const cm = &cpi_->common;
  int width, height;
  cpi_->svc.spatial_layer_id = frame_params.spatial_layer_id;
  cpi_->svc.temporal_layer_id = frame_params.temporal_layer_id;
  if (cpi_->svc.number_spatial_layers > 1) {
    const int layer = LAYER_IDS_TO_IDX(cpi_->svc.spatial_layer_id,
                                       cpi_->svc.temporal_layer_id,
                                       cpi_->svc.number_temporal_layers);
    LAYER_CONTEXT *lc = &cpi_->svc.layer_context[layer];
    get_layer_resolution(cpi_->oxcf.width, cpi_->oxcf.height,
                         lc->scaling_factor_num, lc->scaling_factor_den, &width,
                         &height);
    cm->width = width;
    cm->height = height;
  }
  vp9_set_mb_mi(cm, cm->width, cm->height);
  cm->frame_type = frame_params.frame_type;
  cpi_->refresh_golden_frame = (cm->frame_type == KEY_FRAME) ? 1 : 0;
  cpi_->sf.use_nonrd_pick_mode = 1;
  if (cpi_->svc.number_spatial_layers == 1 &&
      cpi_->svc.number_temporal_layers == 1) {
    vp9_rc_get_one_pass_cbr_params(cpi_);
  } else {
    vp9_update_temporal_layer_framerate(cpi_);
    vp9_restore_layer_context(cpi_);
    vp9_rc_get_svc_params(cpi_);
  }
  int bottom_index, top_index;
  cpi_->common.base_qindex =
      vp9_rc_pick_q_and_bounds(cpi_, &bottom_index, &top_index);
}

int VP9RateControlRTC::GetQP() { return cpi_->common.base_qindex; }

int VP9RateControlRTC::GetLoopfilterLevel() {
  struct loopfilter *const lf = &cpi_->common.lf;
  vp9_pick_filter_level(NULL, cpi_, LPF_PICK_FROM_Q);
  return lf->filter_level;
}

void VP9RateControlRTC::PostEncode(uint64_t encoded_frame_size) {
  cpi_->rc.compute_frame_motion_pass0 = 0;
  vp9_rc_postencode_update(cpi_, encoded_frame_size);
  if (cpi_->svc.number_spatial_layers > 1 ||
      cpi_->svc.number_temporal_layers > 1)
    vp9_save_layer_context(cpi_);
  cpi_->common.current_video_frame++;
}

}  // namespace vp9
