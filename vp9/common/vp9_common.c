/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "vp9/common/vp9_common.h"

const VP9_LEVEL_SPEC vp9_level_defs[VP9_LEVELS] = {
  {LEVEL_1,   829440,      36864,    200,    400,   2, 1,  4,  8},
  {LEVEL_1_1, 2764800,     73728,    800,    1000,  2, 1,  4,  8},
  {LEVEL_2,   4608000,     122880,   1800,   1500,  2, 1,  4,  8},
  {LEVEL_2_1, 9216000,     245760,   3600,   2800,  2, 2,  4,  8},
  {LEVEL_3,   20736000,    552960,   7200,   6000,  2, 4,  4,  8},
  {LEVEL_3_1, 36864000,    983040,   12000,  10000, 2, 4,  4,  8},
  {LEVEL_4,   83558400,    2228224,  18000,  16000, 4, 4,  4,  8},
  {LEVEL_4_1, 160432128,   2228224,  30000,  18000, 4, 4,  5,  6},
  {LEVEL_5,   311951360,   8912896,  60000,  36000, 6, 8,  6,  4},
  {LEVEL_5_1, 588251136,   8912896,  120000, 46000, 8, 8,  10, 4},
  // TODO(huisu): update max_cpb_size for level 5_2 ~ 6_2 when
  // they are finalized (currently TBD).
  {LEVEL_5_2, 1176502272,  8912896,  180000, 0,     8, 8,  10, 4},
  {LEVEL_6,   1176502272,  35651584, 180000, 0,     8, 16, 10, 4},
  {LEVEL_6_1, 2353004544u, 35651584, 240000, 0,     8, 16, 10, 4},
  {LEVEL_6_2, 4706009088u, 35651584, 480000, 0,     8, 16, 10, 4},
};

VP9_LEVEL get_vp9_level(VP9_LEVEL_SPEC *level_spec) {
  int i;
  const VP9_LEVEL_SPEC *this_level;
  for (i = 0; i < VP9_LEVELS; ++i) {
    this_level = &vp9_level_defs[i];
    if (level_spec->max_luma_sample_rate > this_level->max_luma_sample_rate ||
        level_spec->max_luma_picture_size > this_level->max_luma_picture_size ||
        level_spec->average_bitrate > this_level->average_bitrate ||
        level_spec->max_cpb_size > this_level->max_cpb_size ||
        level_spec->compression_ratio < this_level->compression_ratio ||
        level_spec->max_col_tiles > this_level->max_col_tiles ||
        level_spec->min_altref_distance < this_level->min_altref_distance ||
        level_spec->max_ref_frame_buffers > this_level->max_ref_frame_buffers)
      continue;
    else
      break;
  }
  if (i == VP9_LEVELS)
    return LEVEL_UNKNOWN;
  return vp9_level_defs[i].level;
}
