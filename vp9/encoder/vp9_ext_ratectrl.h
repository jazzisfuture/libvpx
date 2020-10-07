/*
 *  Copyright (c) 2020 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_EXT_RATECTRL_H_
#define VPX_VP9_ENCODER_VP9_EXT_RATECTRL_H_
typedef void *vp9_extrc_model_t;
typedef enum {
  vp9_exrc_status_ok,
  vp9_exrc_status_error,
} vp9_exrc_status_t;

typedef struct vp9_extrc_encodeframe_decision {
  int q_index;
} vp9_extrc_encodeframe_decision_t;

typedef struct vp9_extrc_encodeframe_info {
  int frame_type;
  int show_index;
  int coding_index;
} vp9_extrc_encodeframe_info_t;

typedef struct vp9_extrc_encodeframe_result {
  long long sse;
  long long bit_count;
  long long pixel_count;
} vp9_extrc_encodeframe_result_t;

typedef struct vp9_extrc_firstpass_stats {
  double (*frame_stats)[26];
  int num_frames;
} vp9_extrc_firstpass_stats_t;

typedef struct vp9_extrc_config {
  int frame_width;
  int frame_height;
  int show_frame_count;
  int target_bitrate_kbps;
  int frame_rate_num;
  int frame_rate_den;
} vp9_extrc_config_t;

typedef vp9_exrc_status_t (*vp9_extrc_create_model_fn_t)(
    const char *rc_model_config, const vp9_extrc_config_t *encode_config,
    vp9_extrc_model_t *output_model_pt);
typedef vp9_exrc_status_t (*vp9_extrc_send_firstpass_stats_fn_t)(
    vp9_extrc_model_t rate_ctrl_model,
    vp9_extrc_firstpass_stats_t *first_pass_stats);
typedef vp9_exrc_status_t (*vp9_extrc_get_encodeframe_decision_fn_t)(
    vp9_extrc_model_t rate_ctrl_model,
    const vp9_extrc_encodeframe_info_t *encode_frame_info,
    vp9_extrc_encodeframe_decision_t *frame_decision);
typedef vp9_exrc_status_t (*vp9_extrc_update_encodeframe_result_fn_t)(
    vp9_extrc_model_t rate_ctrl_model,
    vp9_extrc_encodeframe_result_t *encode_frame_result);
typedef vp9_exrc_status_t (*vp9_extrc_delete_model_fn_t)(
    vp9_extrc_model_t rate_ctrl_model);

typedef struct vp9_extrc_funcs {
  vp9_extrc_create_model_fn_t create_model;
  vp9_extrc_send_firstpass_stats_fn_t send_firstpass_stats;
  vp9_extrc_get_encodeframe_decision_fn_t get_encodeframe_decision;
  vp9_extrc_update_encodeframe_result_fn_t update_encodeframe_result;
  vp9_extrc_delete_model_fn_t delete_model;
  char *model_config;
} vp9_extrc_funcs_t;

typedef struct vp9_extrc_info {
  int ready;
  vp9_extrc_model_t ratectrl_model;
  vp9_extrc_funcs_t funcs;
} vp9_extrc_info_t;
#endif  // VPX_VP9_ENCODER_VP9_EXT_RATECTRL_H_
