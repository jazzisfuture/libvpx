/*
 *  Copyright (c) 2023 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "vpx/vpx_codec.h"
#include "vpx/vpx_tpl.h"

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
      fprintf(tpl_file, "%ld %ld %hd %hd %ld %ld %d\n", block_stats.inter_cost,
              block_stats.intra_cost, block_stats.mv_c, block_stats.mv_r,
              block_stats.recrf_dist, block_stats.recrf_rate,
              block_stats.ref_frame_index);
    }
  }

  return VPX_CODEC_OK;
}

vpx_codec_err_t vpx_read_tpl_stats(FILE *tpl_file,
                                   VpxTplGopStats *tpl_gop_stats) {
  int i, frame_list_size;
  if (tpl_file == NULL || tpl_gop_stats == NULL) return VPX_CODEC_INVALID_PARAM;
  fscanf(tpl_file, "%d\n", &frame_list_size);
  tpl_gop_stats->size = frame_list_size;
  tpl_gop_stats->frame_stats_list =
      (VpxTplFrameStats *)calloc(frame_list_size, sizeof(VpxTplFrameStats));
  for (i = 0; i < frame_list_size; i++) {
    VpxTplFrameStats *frame_stats = &tpl_gop_stats->frame_stats_list[i];
    int num_blocks, width, height, block;
    fscanf(tpl_file, "%d %d %d\n", &width, &height, &num_blocks);
    frame_stats->num_blocks = num_blocks;
    frame_stats->frame_width = width;
    frame_stats->frame_height = height;
    frame_stats->block_stats_list =
        (VpxTplBlockStats *)calloc(num_blocks, sizeof(VpxTplBlockStats));
    for (block = 0; block < num_blocks; block++) {
      VpxTplBlockStats *block_stats = &frame_stats->block_stats_list[block];
      fscanf(tpl_file, "%ld %ld %hd %hd %ld %ld %d\n", &block_stats->inter_cost,
             &block_stats->intra_cost, &block_stats->mv_c, &block_stats->mv_r,
             &block_stats->recrf_dist, &block_stats->recrf_rate,
             &block_stats->ref_frame_index);
    }
  }

  return VPX_CODEC_OK;
}
