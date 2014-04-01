/*
 * Copyright (C) 2013 MultiCoreWare, Inc. All rights reserved.
 * XinSu <xin@multicorewareinc.com>
 */

#ifndef VP9_COMMON_CONVOLVE_RS_C_H_
#define VP9_COMMON_CONVOLVE_RS_C_H_

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"

#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/kernel/vp9_inter_pred_rs.h"

void build_inter_pred_calcu_rs_c(
         VP9_COMMON *const cm,
         uint8_t *new_buffer,
         const int fri_block_count,
         const int sec_block_count,
         const INTER_PRED_PARAM_CPU_RS *pred_param_fri,
         const INTER_PRED_PARAM_CPU_RS *pred_param_sec,
         convolve_fn_t *switch_convolve_t);

#endif  // VP9_COMMON_CONVOLVE_RS_C_H_
