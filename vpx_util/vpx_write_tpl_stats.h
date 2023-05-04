/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_
#define VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_

#include <stdio.h>
#include "vpx/vpx_encoder.h"

vpx_codec_err_t vpx_write_tpl_stats(FILE *tpl_file,
                                    VpxTplGopStats *tpl_gop_stats) {
  int i;
  if (tpl_file == NULL) return VPX_CODEC_INVALID_PARAM;
  fprintf(tpl_file, "%d\n", tpl_gop_stats->size);

  for (i = 0; i < tpl_gop_stats->size; i++) {
    VpxTplFrameStats frame_stats = tpl_gop_stats->frame_stats_list[i];
    const int num_blocks = frame_stats.num_blocks;
    int block;
    fprintf(tpl_file, "%d %d %d\n", frame_stats.frame_width,
            frame_stats.frame_height, num_blocks);
    for (block = 0; block < num_blocks; block++) {
      VpxTplBlockStats block_stats = frame_stats.block_stats_list[block];
      fprintf(tpl_file, "%d %d %d %d %d %d %d\n", block_stats.inter_cost,
              block_stats.intra_cost, block_stats.mv_c, block_stats.mv_r,
              block_stats.recrf_dist, block_stats.recrf_rate,
              block_stats.ref_frame_index);
    }
  }

  return VPX_CODEC_OK;
}

#endif  // VPX_VPX_UTIL_VPX_WRITE_TPL_STATS_H_
