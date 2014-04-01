/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_LOOPFILTER_H_
#define VP9_COMMON_VP9_LOOPFILTER_H_

#include <stdint.h>
#include "mem.h"
#include "vp9_enums.h"
#include "vpx_integer.h"

#include <string.h>
#define vpx_memcpy  memcpy
#define vpx_memset  memset
#define vpx_memmove memmove
#define vp9_zero(dest) vpx_memset(&dest, 0, sizeof(dest))

enum { MAX_MB_PLANE = 3 };

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))

#define SEGMENT_DELTADATA   0
#define SEGMENT_ABSDATA     1

#define MAX_SEGMENTS     8
#define SEG_TREE_PROBS   (MAX_SEGMENTS-1)

#define PREDICTION_PROBS 3

#define MAX_LOOP_FILTER 63
#define MAX_SHARPNESS 7

#define SIMD_WIDTH 16
#define MAX_SEGMENTS     8

#define MAX_REF_LF_DELTAS       4
#define MAX_MODE_LF_DELTAS      2

#define MINQ 0
#define MAXQ 255
#define QINDEX_RANGE (MAXQ - MINQ + 1)
#define QINDEX_BITS 8

#define ALLOWED_REFS_PER_FRAME 3

#define NUM_REF_FRAMES_LOG2 3
#define NUM_REF_FRAMES (1 << NUM_REF_FRAMES_LOG2)

// 1 scratch frame for the new frame, 3 for scaled references on the encoder
// TODO(jkoleszar): These 3 extra references could probably come from the
// normal reference pool.
#define NUM_YV12_BUFFERS (NUM_REF_FRAMES + 4)

#define NUM_FRAME_CONTEXTS_LOG2 2
#define NUM_FRAME_CONTEXTS (1 << NUM_FRAME_CONTEXTS_LOG2)

typedef enum {
  NONE = -1,
  INTRA_FRAME = 0,
  LAST_FRAME = 1,
  GOLDEN_FRAME = 2,
  ALTREF_FRAME = 3,
  MAX_REF_FRAMES = 4,
} MV_REFERENCE_FRAME;

// Segment level features.
typedef enum {
  SEG_LVL_ALT_Q = 0,               // Use alternate Quantizer ....
  SEG_LVL_ALT_LF = 1,              // Use alternate loop filter value...
  SEG_LVL_REF_FRAME = 2,           // Optional Segment reference frame
  SEG_LVL_SKIP = 3,                // Optional Segment (0,0) + skip mode
  SEG_LVL_MAX = 4                  // Number of features supported
} SEG_LVL_FEATURES;

typedef enum {
  DC_PRED,         // Average of above and left pixels
  V_PRED,          // Vertical
  H_PRED,          // Horizontal
  D45_PRED,        // Directional 45  deg = round(arctan(1/1) * 180/pi)
  D135_PRED,       // Directional 135 deg = 180 - 45
  D117_PRED,       // Directional 117 deg = 180 - 63
  D153_PRED,       // Directional 153 deg = 180 - 27
  D207_PRED,       // Directional 207 deg = 180 + 27
  D63_PRED,        // Directional 63  deg = round(arctan(2/1) * 180/pi)
  TM_PRED,         // True-motion
  NEARESTMV,
  NEARMV,
  ZEROMV,
  NEWMV,
  MB_MODE_COUNT
} MB_PREDICTION_MODE;

typedef enum {
  KEY_FRAME = 0,
  INTER_FRAME = 1,
  FRAME_TYPES,
} FRAME_TYPE;

#define BLOCK_SIZE_GROUPS   4
#define MBSKIP_CONTEXTS 3

/* Segment Feature Masks */
#define MAX_MV_REF_CANDIDATES 2

#define INTRA_INTER_CONTEXTS 4
#define COMP_INTER_CONTEXTS 5
#define REF_CONTEXTS 5

typedef enum {
  PLANE_TYPE_Y_WITH_DC,
  PLANE_TYPE_UV,
} PLANE_TYPE;

typedef char ENTROPY_CONTEXT;

typedef char PARTITION_CONTEXT;


struct loopfilter {
  int filter_level;

  int sharpness_level;
  int last_sharpness_level;

  uint8_t mode_ref_delta_enabled;
  uint8_t mode_ref_delta_update;

  // 0 = Intra, Last, GF, ARF
  signed char ref_deltas[MAX_REF_LF_DELTAS];
  signed char last_ref_deltas[MAX_REF_LF_DELTAS];

  // 0 = ZERO_MV, MV
  signed char mode_deltas[MAX_MODE_LF_DELTAS];
  signed char last_mode_deltas[MAX_MODE_LF_DELTAS];
};

typedef struct {
  int16_t row;
  int16_t col;
} MV;

typedef union int_mv {
  uint32_t as_int;
  MV as_mv;
} int_mv; /* facilitates faster equality tests and copies */

typedef struct {
  int32_t row;
  int32_t col;
} MV32;

typedef void (*convolve_fn_t)(const uint8_t *src, ptrdiff_t src_stride,
                              uint8_t *dst, ptrdiff_t dst_stride,
                              const int16_t *filter_x, int x_step_q4,
                              const int16_t *filter_y, int y_step_q4,
                              int w, int h);

struct scale_factors;
struct scale_factors_common {
  int x_scale_fp;   // horizontal fixed point scale factor
  int y_scale_fp;   // vertical fixed point scale factor
  int x_step_q4;
  int y_step_q4;

  int (*scale_value_x)(int val, const struct scale_factors_common *sfc);
  int (*scale_value_y)(int val, const struct scale_factors_common *sfc);
  void (*set_scaled_offsets)(struct scale_factors *scale, int row, int col);
  MV32 (*scale_mv)(const MV *mv, const struct scale_factors *scale);

  convolve_fn_t predict[2][2][2];  // horiz, vert, avg
};


struct scale_factors {
  int x_offset_q4;
  int y_offset_q4;
  const struct scale_factors_common *sfc;
};

// Need to align this structure so when it is declared and
// passed it can be loaded into vector registers.
typedef struct {
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, mblim[SIMD_WIDTH]);
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, lim[SIMD_WIDTH]);
  DECLARE_ALIGNED(SIMD_WIDTH, uint8_t, hev_thr[SIMD_WIDTH]);
} loop_filter_thresh;

typedef struct {
  loop_filter_thresh lfthr[MAX_LOOP_FILTER + 1];
  uint8_t lvl[MAX_SEGMENTS][MAX_REF_FRAMES][MAX_MODE_LF_DELTAS];
  uint8_t mode_lf_lut[MB_MODE_COUNT];
} loop_filter_info_n;

typedef struct yv12_buffer_config {
  int   y_width;
  int   y_height;
  int   y_crop_width;
  int   y_crop_height;
  int   y_stride;
  /*    int   yinternal_width; */

  int   uv_width;
  int   uv_height;
  int   uv_crop_width;
  int   uv_crop_height;
  int   uv_stride;
  /*    int   uvinternal_width; */

  int   alpha_width;
  int   alpha_height;
  int   alpha_stride;

  uint8_t *y_buffer;
  uint8_t *u_buffer;
  uint8_t *v_buffer;
  uint8_t *alpha_buffer;

  uint8_t *buffer_alloc;
  int buffer_alloc_sz;
  int border;
  int frame_size;

  int corrupted;
  int flags;
} YV12_BUFFER_CONFIG;

typedef struct {
  MB_PREDICTION_MODE as_mode;
  int_mv as_mv[2];  // first, second inter predictor motion vectors
} b_mode_info;

typedef enum {
  EIGHTTAP = 0,
  EIGHTTAP_SMOOTH = 1,
  EIGHTTAP_SHARP = 2,
  BILINEAR = 3,
  SWITCHABLE = 4  /* should be the last one */
} INTERPOLATION_TYPE;

typedef enum {
  SINGLE_REFERENCE      = 0,
  COMPOUND_REFERENCE    = 1,
  REFERENCE_MODE_SELECT = 2,
  REFERENCE_MODES       = 3,
} REFERENCE_MODE;

// This structure now relates to 8x8 block regions.
typedef struct {
  MB_PREDICTION_MODE mode, uv_mode;
  MV_REFERENCE_FRAME ref_frame[2];
  TX_SIZE tx_size;
  int_mv mv[2];                // for each reference frame used
  int_mv ref_mvs[MAX_REF_FRAMES][MAX_MV_REF_CANDIDATES];
  int_mv best_mv[2];

  uint8_t mode_context[MAX_REF_FRAMES];

  unsigned char skip_coeff;    // 0=need to decode coeffs, 1=no coefficients
  unsigned char segment_id;    // Segment id for this block.

  // Flags used for prediction status of various bit-stream signals
  unsigned char seg_id_predicted;

  INTERPOLATION_TYPE interp_filter;

  BLOCK_SIZE sb_type;
} MB_MODE_INFO;

typedef struct {
  MB_MODE_INFO mbmi;
  b_mode_info bmi[4];
} MODE_INFO;

struct buf_2d {
  uint8_t *buf;
  int stride;
};

struct macroblockd_plane {
  int16_t *qcoeff;
  int16_t *dqcoeff;
  uint16_t *eobs;
  PLANE_TYPE plane_type;
  int subsampling_x;
  int subsampling_y;
  struct buf_2d dst;
  struct buf_2d pre[2];
  int16_t *dequant;
  ENTROPY_CONTEXT *above_context;
  ENTROPY_CONTEXT *left_context;
};

typedef struct macroblockd {
  struct macroblockd_plane plane[MAX_MB_PLANE];

  //struct scale_factors scale_factor[2];

  MODE_INFO *last_mi;
  int mode_info_stride;

  // A NULL indicates that the 8x8 is not part of the image
  MODE_INFO **mi_8x8;
  MODE_INFO **prev_mi_8x8;
  MODE_INFO *mi_stream;

  int up_available;
  int left_available;

  /* Distance of MB away from frame edges */
  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  int lossless;
  /* Inverse transform function pointers. */
  void (*itxm_add)(const int16_t *input, uint8_t *dest, int stride, int eob);

  //struct subpix_fn_table  subpix;

  int corrupted;

  /* Y,U,V,(A) */
  ENTROPY_CONTEXT *above_context[MAX_MB_PLANE];
  ENTROPY_CONTEXT left_context[MAX_MB_PLANE][16];

  PARTITION_CONTEXT *above_seg_context;
  PARTITION_CONTEXT left_seg_context[8];
} MACROBLOCKD;

struct segmentation {
  uint8_t enabled;
  uint8_t update_map;
  uint8_t update_data;
  uint8_t abs_delta;
  uint8_t temporal_update;

//  vp9_prob tree_probs[SEG_TREE_PROBS];
//  vp9_prob pred_probs[PREDICTION_PROBS];

  int16_t feature_data[MAX_SEGMENTS][SEG_LVL_MAX];
  unsigned int feature_mask[MAX_SEGMENTS];
};

typedef struct VP9Common {
  //struct vpx_internal_error_info  error;

  DECLARE_ALIGNED(16, int16_t, y_dequant[QINDEX_RANGE][8]);
  DECLARE_ALIGNED(16, int16_t, uv_dequant[QINDEX_RANGE][8]);
#if CONFIG_ALPHA
  DECLARE_ALIGNED(16, int16_t, a_dequant[QINDEX_RANGE][8]);
#endif

  COLOR_SPACE color_space;

  int width;
  int height;
  int display_width;
  int display_height;
  int last_width;
  int last_height;

  // TODO(jkoleszar): this implies chroma ss right now, but could vary per
  // plane. Revisit as part of the future change to YV12_BUFFER_CONFIG to
  // support additional planes.
  int subsampling_x;
  int subsampling_y;

  YV12_BUFFER_CONFIG *frame_to_show;

  YV12_BUFFER_CONFIG yv12_fb[NUM_YV12_BUFFERS];
  int fb_idx_ref_cnt[NUM_YV12_BUFFERS]; /* reference counts */
  int ref_frame_map[NUM_REF_FRAMES]; /* maps fb_idx to reference slot */

  // TODO(jkoleszar): could expand active_ref_idx to 4, with 0 as intra, and
  // roll new_fb_idx into it.

  // Each frame can reference ALLOWED_REFS_PER_FRAME buffers
  int active_ref_idx[ALLOWED_REFS_PER_FRAME];
  //struct scale_factors active_ref_scale[ALLOWED_REFS_PER_FRAME];
  //struct scale_factors_common active_ref_scale_comm[ALLOWED_REFS_PER_FRAME];
  int new_fb_idx;

  YV12_BUFFER_CONFIG post_proc_buffer;

  FRAME_TYPE last_frame_type;  /* last frame's frame type for motion search.*/
  FRAME_TYPE frame_type;

  int show_frame;
  int last_show_frame;

  // Flag signaling that the frame is encoded using only INTRA modes.
  int intra_only;

  int allow_high_precision_mv;

  // Flag signaling that the frame context should be reset to default values.
  // 0 or 1 implies don't reset, 2 reset just the context specified in the
  // frame header, 3 reset all contexts.
  int reset_frame_context;

  int frame_flags;
  // MBs, mb_rows/cols is in 16-pixel units; mi_rows/cols is in
  // MODE_INFO (8-pixel) units.
  int MBs;
  int mb_rows, mi_rows;
  int mb_cols, mi_cols;
  int mode_info_stride;

  /* profile settings */
  TX_MODE tx_mode;

  int base_qindex;
  int y_dc_delta_q;
  int uv_dc_delta_q;
  int uv_ac_delta_q;
#if CONFIG_ALPHA
  int a_dc_delta_q;
  int a_ac_delta_q;
#endif

  /* We allocate a MODE_INFO struct for each macroblock, together with
     an extra row on top and column on the left to simplify prediction. */

  MODE_INFO *mip; /* Base of allocated array */
  MODE_INFO *mi;  /* Corresponds to upper left visible macroblock */
  MODE_INFO *prev_mip; /* MODE_INFO array 'mip' from last decoded frame */
  MODE_INFO *prev_mi;  /* 'mi' from last frame (points into prev_mip) */

  MODE_INFO **mi_grid_base;
  MODE_INFO **mi_grid_visible;
  MODE_INFO **prev_mi_grid_base;
  MODE_INFO **prev_mi_grid_visible;

  // Persistent mb segment id map used in prediction.
  unsigned char *last_frame_seg_map;

  INTERPOLATION_TYPE mcomp_filter_type;

  loop_filter_info_n lf_info;

  int refresh_frame_context;    /* Two state 0 = NO, 1 = YES */

  int ref_frame_sign_bias[MAX_REF_FRAMES];    /* Two state 0, 1 */

  struct loopfilter lf;
  struct segmentation seg;

  // Context probabilities for reference frame prediction
  int allow_comp_inter_inter;
  MV_REFERENCE_FRAME comp_fixed_ref;
  MV_REFERENCE_FRAME comp_var_ref[2];
  REFERENCE_MODE comp_pred_mode;

//  FRAME_CONTEXT fc;  /* this frame entropy */
//  FRAME_CONTEXT frame_contexts[NUM_FRAME_CONTEXTS];
//  unsigned int  frame_context_idx; /* Context to use/update */
//  FRAME_COUNTS counts;

  unsigned int current_video_frame;
  int version;

#if CONFIG_VP9_POSTPROC
  struct postproc_state  postproc_state;
#endif

  int error_resilient_mode;
  int frame_parallel_decoding_mode;

  int log2_tile_cols, log2_tile_rows;
} VP9_COMMON;

/* assorted loopfilter functions which get used elsewhere */
struct VP9Common;
struct macroblockd;

extern const TX_SIZE max_uv_txsize_lookup[BLOCK_SIZES];

static INLINE TX_SIZE get_uv_tx_size(const MB_MODE_INFO *mbmi) {
  return MIN(mbmi->tx_size, max_uv_txsize_lookup[mbmi->sb_type]);
}

void vp9_loop_filter_init(struct VP9Common *cm);

// Update the loop filter for the current frame.
// This should be called before vp9_loop_filter_rows(), vp9_loop_filter_frame()
// calls this function directly.
void vp9_loop_filter_frame_init(struct VP9Common *cm, int default_filt_lvl);

void vp9_loop_filter_frame(struct VP9Common *cm,
                           struct macroblockd *mbd,
                           int filter_level,
                           int y_only, int partial);

// Apply the loop filter to [start, stop) macro block rows in frame_buffer.
void vp9_loop_filter_rows(const YV12_BUFFER_CONFIG *frame_buffer,
                          struct VP9Common *cm, struct macroblockd *xd,
                          int start, int stop, int y_only);

void vp9_loop_filter_rows_serialized(const YV12_BUFFER_CONFIG *frame_buffer,
                          VP9_COMMON *cm, MACROBLOCKD *xd,
                          int start, int stop, int y_only);

#endif  // VP9_COMMON_VP9_LOOPFILTER_H_
