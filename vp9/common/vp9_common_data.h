/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_COMMON_DATA_H_
#define VP9_COMMON_VP9_COMMON_DATA_H_

#include "vp9/common/vp9_enums.h"
#include "vpx/vpx_integer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPB_WINDOW_SIZE 4
#define FRAME_WINDOW_SIZE 128

typedef struct {
  VP9_LEVEL level;
  uint64_t max_luma_sample_rate;
  uint32_t max_luma_picture_size;
  double average_bitrate;  // in kilobits per second
  double max_cpb_size;  // in kilobits
  double compression_ratio;
  uint8_t max_col_tiles;
  uint32_t min_altref_distance;
  uint8_t max_ref_frame_buffers;
} VP9_LEVEL_SPEC;

typedef struct {
  int64_t ts;  // timestamp
  uint32_t luma_samples;
  uint32_t size;  // in bytes
} FRAME_RECORD;

typedef struct {
  FRAME_RECORD buf[FRAME_WINDOW_SIZE];
  uint8_t start;
  uint8_t len;
} FRAME_WINDOW_BUFFER;

typedef struct {
  uint8_t seen_first_altref;
  uint32_t frames_since_last_altref;
  uint64_t total_compressed_size;  // in bytes
  uint64_t total_uncompressed_size;  // in bytes
  double time_encoded;  // in seconds
  FRAME_WINDOW_BUFFER frame_window_buffer;
  int ref_refresh_map;
} VP9_LEVEL_STATS;

typedef struct {
  VP9_LEVEL_STATS level_stats;
  VP9_LEVEL_SPEC level_spec;
} VP9_LEVEL_INFO;

extern const uint8_t b_width_log2_lookup[BLOCK_SIZES];
extern const uint8_t b_height_log2_lookup[BLOCK_SIZES];
extern const uint8_t mi_width_log2_lookup[BLOCK_SIZES];
extern const uint8_t num_8x8_blocks_wide_lookup[BLOCK_SIZES];
extern const uint8_t num_8x8_blocks_high_lookup[BLOCK_SIZES];
extern const uint8_t num_4x4_blocks_high_lookup[BLOCK_SIZES];
extern const uint8_t num_4x4_blocks_wide_lookup[BLOCK_SIZES];
extern const uint8_t size_group_lookup[BLOCK_SIZES];
extern const uint8_t num_pels_log2_lookup[BLOCK_SIZES];
extern const PARTITION_TYPE partition_lookup[][BLOCK_SIZES];
extern const BLOCK_SIZE subsize_lookup[PARTITION_TYPES][BLOCK_SIZES];
extern const TX_SIZE max_txsize_lookup[BLOCK_SIZES];
extern const BLOCK_SIZE txsize_to_bsize[TX_SIZES];
extern const TX_SIZE tx_mode_to_biggest_tx_size[TX_MODES];
extern const BLOCK_SIZE ss_size_lookup[BLOCK_SIZES][2][2];
extern const VP9_LEVEL_SPEC vp9_level_defs[VP9_LEVELS];

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_COMMON_DATA_H_
