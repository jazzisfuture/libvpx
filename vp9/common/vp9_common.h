/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_COMMON_H_
#define VP9_COMMON_VP9_COMMON_H_

/* Interface header for common constant data structures and lookup tables */

#include <assert.h>

#include "./vpx_config.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx_mem/vpx_mem.h"
#include "vpx/vpx_integer.h"
#include "vpx_ports/bitops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Only need this for fixed-size arrays, for structs just assign.
#define vp9_copy(dest, src) {            \
    assert(sizeof(dest) == sizeof(src)); \
    memcpy(dest, src, sizeof(src));  \
  }

// Use this for variably-sized arrays.
#define vp9_copy_array(dest, src, n) {       \
    assert(sizeof(*dest) == sizeof(*src));   \
    memcpy(dest, src, n * sizeof(*src)); \
  }

#define vp9_zero(dest) memset(&(dest), 0, sizeof(dest))
#define vp9_zero_array(dest, n) memset(dest, 0, n * sizeof(*dest))

static INLINE int get_unsigned_bits(unsigned int num_values) {
  return num_values > 0 ? get_msb(num_values) + 1 : 0;
}

#if CONFIG_DEBUG
#define CHECK_MEM_ERROR(cm, lval, expr) do { \
  lval = (expr); \
  if (!lval) \
    vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR, \
                       "Failed to allocate "#lval" at %s:%d", \
                       __FILE__, __LINE__); \
  } while (0)
#else
#define CHECK_MEM_ERROR(cm, lval, expr) do { \
  lval = (expr); \
  if (!lval) \
    vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR, \
                       "Failed to allocate "#lval); \
  } while (0)
#endif

#define VP9_SYNC_CODE_0 0x49
#define VP9_SYNC_CODE_1 0x83
#define VP9_SYNC_CODE_2 0x42

#define VP9_FRAME_MARKER 0x2

#define CPB_WINDOW_SIZE 4
#define FRAME_WINDOW_SIZE 256
#define VP9_LEVELS 14

typedef enum {
  LEVEL_UNKNOWN = 0,
  LEVEL_1 = 10,
  LEVEL_1_1 = 11,
  LEVEL_2 = 20,
  LEVEL_2_1 = 21,
  LEVEL_3 = 30,
  LEVEL_3_1 = 31,
  LEVEL_4 = 40,
  LEVEL_4_1 = 41,
  LEVEL_5 = 50,
  LEVEL_5_1 = 51,
  LEVEL_5_2 = 52,
  LEVEL_6 = 60,
  LEVEL_6_1 = 61,
  LEVEL_6_2 = 62
} VP9_LEVEL;

typedef struct {
  VP9_LEVEL level;
  int64_t max_luma_sample_rate;
  int max_luma_picture_size;
  double average_bitrate;
  double max_cpb_size;
  double compression_ratio;
  int max_col_tiles;
  int min_altref_distance;
  int max_ref_frame_buffers;
} VP9_LEVEL_SPEC;

typedef struct {
  int64_t ts;
  int luma_samples;
  int size;
  uint8_t is_altref;
} FRAME_RECORD;

typedef struct {
  FRAME_RECORD buf[FRAME_WINDOW_SIZE];
  uint8_t start;
  uint8_t len;
} FRAME_WINDOW;

typedef struct {
  uint8_t seen_first_altref;
  int frames_since_last_altref;
  int64_t total_compressed_size;
  int64_t total_uncompressed_size;
  double time_encoded;
  FRAME_WINDOW frame_window;
  int ref_refresh_map;
} VP9_LEVEL_STATS;

typedef struct {
  VP9_LEVEL_STATS level_stats;
  VP9_LEVEL_SPEC level_spec;
} VP9_LEVEL_INFO;

extern const VP9_LEVEL_SPEC vp9_level_defs[VP9_LEVELS];

VP9_LEVEL get_vp9_level(VP9_LEVEL_SPEC *level_spec);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP9_COMMON_VP9_COMMON_H_
